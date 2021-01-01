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
String POST_url = "http://smartsensorbox.ddns.net:5000/measurements/multiple";
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
  wm.resetSettings(); // wipe settings
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
    clear_counter();

    Serial.println("######### WindSpeed MEASUREMENTS ###########");
	  Serial.print("BIN: ");
	  print_counter_state_bin(wscounter);
    Serial.print("DEC: ");
	  Serial.println(wscounter);
	  Serial.print("WS: ");
	  Serial.println(measurements[idx].windspeed);
    Serial.print("Num Overflows: ");
	  Serial.println(overflow_count);

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
    bool res = wm.autoConnect(APname, APpass);

    if(!res) {
      Serial.println("Failed to connect or hit timeout");
      ESP.restart();
    } 
    else {
      Serial.println("Connected!");
    }

    //-------------------------User Settings ------------------------------------
    res = get_user_settings(&usersettings, GET_url, OTA_url);
    if (!res){
      dss.wake_up_time = min(usersettings.sync_period, usersettings.sample_period);
      dss.sample_counter = usersettings.sample_period/dss.wake_up_time;
      dss.sync_counter = usersettings.sync_period/dss.wake_up_time;
      is_initialized = true;
    } 
    else{
      Serial.print("Failure in syncing to db");
      exit(0);
    }

    //-------------------------Post user settings--------------------------------
    if (num_measurements > 0){
      Serial.print("Number of samples to post");
      Serial.println(num_measurements);
      post_measurements(measurements, num_measurements, POST_url);
      //ToDo: add fail check
      Serial.println("Posted!!");
    }
    num_measurements = 0;
  }

  float uptime = (float)esp_timer_get_time()*1e-6;
  Serial.print("Program duration:");
  Serial.println(uptime);

  //-----------------------------------Sleep----------------------------------------
  gpio_hold_en(COUNTER_RESET_GPIO);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, HIGH);
  //esp_sleep_enable_timer_wakeup(0.2*US_TO_S_FACTOR*60-esp_timer_get_time()); //For testing
  esp_sleep_enable_timer_wakeup(dss.wake_up_time*US_TO_S_FACTOR*60-esp_timer_get_time());
  lastsleep_time = get_rtc_time_us(); 
  esp_deep_sleep_start();
}

void loop() {
}