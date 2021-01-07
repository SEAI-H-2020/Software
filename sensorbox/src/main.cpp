#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h> 
#include <DHT.h>
#include "soc/timer_group_reg.h"
#include "esp_attr.h"
#include "esp_intr_alloc.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/frc_timer_reg.h"
#include "rom/ets_sys.h"
#include "ssbsettings.h"
#include "ssbsensors.h"
#include "ssbdeepsleep.h"
#include "esp_settings.h"

#define US_TO_S_FACTOR 1e6

/* frequency of RTC slow clock, Hz */
#define RTC_CTNL_SLOWCLK_FREQ   150000

//Wifi connection and AP settings
WiFiManager wm;
const char* APname = "SmartSensorBox";
const char* APpass = "softwareH";

//Box Configs
String box_id = "1";

//Server Settings
String POST_url = "http://smartsensorbox.ddns.net:5000/measurements_tstamp/multiple";
String GET_url =  "http://smartsensorbox.ddns.net:5000/usersettings/" + box_id;
String OTA_url =  "http://smartsensorbox.ddns.net:5000/update";

//RTC Variables
RTC_DATA_ATTR uint64_t lastsleep_time;
RTC_DATA_ATTR settings_t usersettings;
RTC_DATA_ATTR bool is_initialized = false;
RTC_DATA_ATTR uint32_t wake_up_counter = 0;
RTC_DATA_ATTR measurement_t measurements[MAX_MEASUREMENTS];
RTC_DATA_ATTR dssettings_t dss;
RTC_DATA_ATTR uint8_t num_measurements = 0;
RTC_DATA_ATTR uint8_t overflow_count = 0;
RTC_DATA_ATTR boolean buffer_overflow = false;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;


// Based on https://github.com/pycom/esp-idf-2.0/blob/master/components/newlib/time.c
uint64_t get_rtc_time_us(){
    SET_PERI_REG_MASK(RTC_CNTL_TIME_UPDATE_REG, RTC_CNTL_TIME_UPDATE_M);
    while (GET_PERI_REG_MASK(RTC_CNTL_TIME_UPDATE_REG, RTC_CNTL_TIME_VALID_M) == 0) {
        ;
    }
    CLEAR_PERI_REG_MASK(RTC_CNTL_TIME_UPDATE_REG, RTC_CNTL_TIME_UPDATE_M);
    uint64_t low = READ_PERI_REG(RTC_CNTL_TIME0_REG);
    uint64_t high = READ_PERI_REG(RTC_CNTL_TIME1_REG);
    uint64_t ticks = (high << 32) | low;
    return ticks * 100 / (RTC_CTNL_SLOWCLK_FREQ/ 10000);    // scale RTC_CTNL_SLOWCLK_FREQ to avoid overflow
}


void IRAM_ATTR isr(){
  clear_counter();
  overflow_count += 1;
  delay(10); //to prevent bouncing remove later
}

//This function is called after the esp wakes up
void RTC_IRAM_ATTR esp_wake_deep_sleep(void) {
    esp_default_wake_deep_sleep();
    wake_up_counter++;
}

void setup() {  
  // WiFiManager stuff
  #ifdef RESET_WIFI_SETTINGS
  if (wake_up_counter == 0){
    wm.resetSettings(); // wipe settings on first cycle
  }
  #endif
  std::vector<const char *> menu = {"wifi","info","sep","restart","exit"};
  wm.setMenu(menu);

  //-----------------------------Serial Setup-------------------------------------
  Serial.begin(9600);
  Serial.setDebugOutput(true);  

  #ifdef INITIAL_DELAY
  delay(5000);
  #endif
  
  DEBUG_P("\n Starting");

    //---------------------------Mesurement buffer check----------------------------
  DEBUG_P("Measurements in buffer: ");
  DEBUG_P(num_measurements);
  if (num_measurements >= MAX_MEASUREMENTS){
      DEBUG_P("Buffer full! Overwriting measurements!");
      num_measurements = 0; //starts overwriting from index 0
      buffer_overflow = true;
  }

  //-----------------------------HARDWARE SETUP----------------------------------
  setup_ws_sensor();
  pinMode(PIN_SUPPLY, OUTPUT);
  attachInterrupt(GPIO_NUM_33, isr, RISING); 
  DHT dht(DHTPIN, DHTTYPE);
  dht.begin();

  //-----------------------------Figure out wakeup reason-----------------------
  uint8_t wakeup_reason = check_wakeup_reason();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    wake_up_counter--;
    overflow_count++;
    clear_counter();
    DEBUG_P("Overflowwwwwwww!!!!");
    uint64_t elapsed_time_us = get_rtc_time_us()-lastsleep_time;
    uint64_t remaining_time = (uint64_t)dss.wake_up_time * 60 * US_TO_S_FACTOR - elapsed_time_us;

    //Debug
    long remaining_sleep_us = (long) remaining_time;
    DEBUG_P("Remaining time:");
    DEBUG_P(remaining_sleep_us);

    //Sleep for the remaining time
    gpio_hold_en(COUNTER_RESET_GPIO);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, HIGH);
    esp_sleep_enable_timer_wakeup(remaining_time);
    esp_deep_sleep_start();
  }

  //------------------------------Sample-----------------------------------------
  debug_sample_counter(dss, wake_up_counter);
  if ((wake_up_counter % dss.sample_counter == 0) && (is_initialized == true)){
    DEBUG_P("SAMPLE");
    gpio_hold_dis(PIN_SUPPLY_GPIO);
    digitalWrite(PIN_SUPPLY, HIGH);
    uint8_t idx = num_measurements;

    #ifdef BATT_MEASURE
    //// read battery voltage level
    //bat = analogRead(BatteryPin);
    //voltage = (float)5*bat/4095;
    //measurements[idx].voltage = voltage;
    //DEBUG_P("voltage = ");
    //DEBUG_P(voltage);
    //DEBUG_P();
    #else
    measurements[idx].voltage = 3.3; //dummy
    #endif
    
    // read temperature and humidity values
    //int chk = DHT11.read(DHT11PIN);
    float hum = (float)dht.readHumidity();
    float temp = (float)dht.readTemperature();
    //// display values to serial
    DEBUG_P("Humidity (%): ");
    DEBUG_P(hum);//DEBUG_P((float)DHT11.humidity, 2);
    DEBUG_P("Temperature (C): ");
    DEBUG_P(temp);//DEBUG_P((float)DHT11.temperature, 2);
    measurements[idx].temp = temp; //dummy
    measurements[idx].hum = hum; //dummy

    // read sound sensor
    float adc = analogRead(MIC); //Read the ADC value from amplifer 
    //Serial.println (adc);//Print ADC for initial calculation 
    float dB = (adc-66.286) / 20.143; 
    measurements[idx].noise = dB; //dummy value

    //// read windspeed sensor
    uint16_t wscounter = read_windspeed_raw();
	  measurements[idx].windspeed = calculate_windspeed(wscounter,dss.wake_up_time*60);
    measurements[idx].windspeed += overflow_count * (2^13);
//
//
    clear_counter();
    DEBUG_P("######### WindSpeed MEASUREMENTS ###########");
	  DEBUG_P("BIN: ");
	  print_counter_state_bin(wscounter);
    DEBUG_P("DEC: ");
	  DEBUG_P(wscounter);
	  DEBUG_P("WS: ");
	  DEBUG_P(measurements[idx].windspeed);
    DEBUG_P("Num Overflows: ");
	  DEBUG_P(overflow_count);

    wscounter = 0;
    overflow_count = 0;
    //ToDo clear counter on first sync

    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
    }
    else {
        sprintf(measurements[idx].timestamp, "%02d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        Serial.print("Datetime: ");
        Serial.println(measurements[idx].timestamp); //2020-12-22 22:49:39'
    }

    num_measurements++;
    digitalWrite(PIN_SUPPLY, LOW);
  }

  //--------------------------------Sync-----------------------------------------
  debug_sync_counter(dss, wake_up_counter);

  if ((wake_up_counter % dss.sync_counter == 0) || (is_initialized == false)){
    DEBUG_P("SYNC");

    //-------------------------Connect to Wifi----------------------------------
    bool is_connected = wm.autoConnect(is_initialized, APname, APpass);

    if(is_connected) {
      DEBUG_P("IM connected");
      //------------------------- Clock Sync----------------------------------------
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

      //-------------------------User Settings ------------------------------------
      if (!get_user_settings(&usersettings, GET_url, OTA_url)){
        dss.wake_up_time = min(usersettings.sync_period, usersettings.sample_period);
        dss.sample_counter = usersettings.sample_period/dss.wake_up_time;
        dss.sync_counter = usersettings.sync_period/dss.wake_up_time;
        is_initialized = true;
      } 
      else{
        DEBUG_P("Failure in syncing to db");
      }

      //-------------------------Post user settingshttps://andrewsleigh.com/fab-slider/coding-better-debugging/--------------------------------
      if (num_measurements > 0 || buffer_overflow){
        // In case of overflow post all measurements -> circular buffer
        if (buffer_overflow){
          num_measurements = MAX_MEASUREMENTS;
        }
        DEBUG_P("Number of samples to post");
        DEBUG_P(num_measurements);
        post_measurements(measurements, num_measurements, POST_url);
        //ToDo: add fail check
        DEBUG_P("Posted!!");
        num_measurements = 0;
        buffer_overflow = false;
      }
    }
    else if (!is_connected && is_initialized){
        DEBUG_P("Unable to sync. Running in Offline mode.");
    } 
    else if (!is_connected && !is_initialized) {
        DEBUG_P("Failure to connect or timeout. Rebooting");
        ESP.restart();
    }
    else{
        DEBUG_P("Oops..Something went wrong. Rebooting");
        ESP.restart();
    }
  }


  float uptime = (float)esp_timer_get_time()*1e-6;
  Serial.print("Program duration:");
  Serial.print(uptime);


  //-----------------------------------Sleep----------------------------------------
  gpio_hold_en(COUNTER_RESET_GPIO);
  gpio_hold_en(PIN_SUPPLY_GPIO);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, HIGH);
  esp_sleep_enable_timer_wakeup(0.2*US_TO_S_FACTOR*60-esp_timer_get_time()); //For testing
  //esp_sleep_enable_timer_wakeup(dss.wake_up_time*US_TO_S_FACTOR*60-esp_timer_get_time());
  lastsleep_time = get_rtc_time_us(); 
  esp_deep_sleep_start();
}

void loop() {
}