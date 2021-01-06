#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h> 
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

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}


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
  overflow_count += 1;
  clear_counter();
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
  
  Serial.println("\n Starting");

    //---------------------------Mesurement buffer check----------------------------
  Serial.print("Measurements in buffer: ");
  Serial.println(num_measurements);
  if (num_measurements >= MAX_MEASUREMENTS){
      Serial.println("Buffer full! Overwriting measurements!");
      num_measurements = 0; //starts overwriting from index 0
      buffer_overflow = true;
  }

  //-----------------------------HARDWARE SETUP----------------------------------
  setup_ws_sensor();
  attachInterrupt(GPIO_NUM_33, isr, RISING); 
  //-----------------------------Figure out wakeup reason-----------------------
  uint8_t wakeup_reason = check_wakeup_reason();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    wake_up_counter--;
    overflow_count++;
    clear_counter();
    Serial.println("Overflowwwwwwww!!!!");
    uint64_t elapsed_time_us = get_rtc_time_us()-lastsleep_time;
    uint64_t remaining_time = (uint64_t)dss.wake_up_time * 60 * US_TO_S_FACTOR - elapsed_time_us;

    //Debug
    long remaining_sleep_us = (long) remaining_time;
    Serial.print("Remaining time:");
    Serial.println(remaining_sleep_us);

    //Sleep for the remaining time
    gpio_hold_en(COUNTER_RESET_GPIO);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, HIGH);
    esp_sleep_enable_timer_wakeup(remaining_time);
    esp_deep_sleep_start();
  }

  //------------------------------Sample-----------------------------------------
  debug_sample_counter(dss, wake_up_counter);
  if ((wake_up_counter % dss.sample_counter == 0) && (is_initialized == true)){
    Serial.println("SAMPLE");
    uint8_t idx = num_measurements;

    //// read battery voltage level
    //bat = analogRead(BatteryPin);
    //voltage = (float)5*bat/4095;
    //Serial.print("voltage = ");
    //Serial.print(voltage);
    //Serial.println();
    measurements[idx].voltage = 3.3; //dummy
    
    // read temperature and humidity values
    //int chk = DHT11.read(DHT11PIN);
    //hum = (float)DHT11.humidity;
    //temp = (float)DHT11.temperature;
    //// display values to serial
    //Serial.print("Humidity (%): ");
    //Serial.println(hum, 2);//Serial.println((float)DHT11.humidity, 2);
    //Serial.print("Temperature (C): ");
    //Serial.println(temp, 2);//Serial.println((float)DHT11.temperature, 2);
    measurements[idx].temp = 15; //dummy
    measurements[idx].hum = 80; //dummy

    // read sound sensor
    measurements[idx].noise = 10; //dummy value

    // read windspeed sensor
    uint16_t wscounter = read_windspeed_raw();
	  measurements[idx].windspeed = calculate_windspeed(wscounter,dss.wake_up_time*60);
    measurements[idx].windspeed += overflow_count * (2^13);

    // read measurement date and time
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
    return;
    }
    else {
      sprintf(measurements[idx].timestamp, "%02d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      Serial.print("Datetime: ");
      Serial.println(measurements[idx].timestamp); //2020-12-22 22:49:39'
  }

    //clear_counter();
    //Serial.println("######### WindSpeed MEASUREMENTS ###########");
	  //Serial.print("BIN: ");
	  //print_counter_state_bin(wscounter);
    //Serial.print("DEC: ");
	  //Serial.println(wscounter);
	  //Serial.print("WS: ");
	  //Serial.println(measurements[idx].windspeed);
    //Serial.print("Num Overflows: ");
	  //Serial.println(overflow_count);

    wscounter = 0;
    overflow_count = 0;
    //ToDo clear counter on first sync

    num_measurements++;

  }

  //--------------------------------Sync-----------------------------------------
  debug_sync_counter(dss, wake_up_counter);

  if ((wake_up_counter % dss.sync_counter == 0) || (is_initialized == false)){
    Serial.println("SYNC");

    //-------------------------Connect to Wifi----------------------------------
    bool is_connected = wm.autoConnect(is_initialized, APname, APpass);

    if(is_connected) {
      //-------------------------User Settings ------------------------------------
      if (!get_user_settings(&usersettings, GET_url, OTA_url)){
        dss.wake_up_time = min(usersettings.sync_period, usersettings.sample_period);
        dss.sample_counter = usersettings.sample_period/dss.wake_up_time;
        dss.sync_counter = usersettings.sync_period/dss.wake_up_time;
        is_initialized = true;
      } 
      else{
        Serial.print("Failure in syncing to db");
      }

      //-------------------------Post user settings--------------------------------
      if (num_measurements > 0 || buffer_overflow){
        // In case of overflow post all measurements -> circular buffer
        if (buffer_overflow){
          num_measurements = MAX_MEASUREMENTS;
        }
        Serial.print("Number of samples to post");
        Serial.println(num_measurements);
        post_measurements(measurements, num_measurements, POST_url);
        //ToDo: add fail check
        Serial.println("Posted!!");
        num_measurements = 0;
        buffer_overflow = false;
      }
      //------------------------- Clock Sync----------------------------------------
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    }
    else if (!is_connected && is_initialized){
        Serial.println("Unable to sync. Running in Offline mode.");
    } 
    else if (!is_connected && !is_initialized) {
        Serial.println("Failure to connect or timeout. Rebooting");
        ESP.restart();
    }
    else{
        Serial.println("Oops..Something went wrong. Rebooting");
        ESP.restart();
    }
  }


  float uptime = (float)esp_timer_get_time()*1e-6;
  Serial.print("Program duration:");
  Serial.println(uptime);

  //-----------------------------------Sleep----------------------------------------
  gpio_hold_en(COUNTER_RESET_GPIO);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, HIGH);
  esp_sleep_enable_timer_wakeup(0.2*US_TO_S_FACTOR*60-esp_timer_get_time()); //For testing
  //esp_sleep_enable_timer_wakeup(dss.wake_up_time*US_TO_S_FACTOR*60-esp_timer_get_time());
  lastsleep_time = get_rtc_time_us(); 
  esp_deep_sleep_start();
}

void loop() {
}