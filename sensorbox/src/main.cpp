#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h> 
#include "ssbsettings.h"
#include "ssbsensors.h"
#include "ssbdeepsleep.h"

#define MAX_MEASUREMENTS 20
#define RESET_WIFI_SETTINGS 0
#define INITIAL_DELAY 0         // initial delay of 5sec before printing out stuff
#define US_TO_S_FACTOR 1e6

//Wifi connection and AP settings
WiFiManager wm;
const char* APname = "SmartSensorBox";
const char* APpass = "softwareH";

//Box Configs
String box_id = "1";

//Server Settings
String POST_url = "http://smartsensorbox.ddns.net:5000/measurements/multiple";
String GET_url = "http://smartsensorbox.ddns.net:5000/usersettings/" + box_id;

//RTC Variables
RTC_DATA_ATTR settings_t usersettings;
RTC_DATA_ATTR bool is_initialized = false;
RTC_DATA_ATTR uint32_t wake_up_counter = 0;
RTC_DATA_ATTR measurement_t measurements[MAX_MEASUREMENTS];
RTC_DATA_ATTR dssettings_t dss;
RTC_DATA_ATTR uint8_t num_measurements = 0;

//This function is called after the esp wakes up
void RTC_IRAM_ATTR esp_wake_deep_sleep(void) {
    esp_default_wake_deep_sleep();
    wake_up_counter++;
}

void setup() {  
  // WiFiManager stuff
  #if RESET_WIFI_SETTINGS
  wm.resetSettings(); // wipe settings
  #endif
  std::vector<const char *> menu = {"wifi","info","sep","restart","exit"};
  wm.setMenu(menu);

  //--------------------------------Serial Setup-----------------------------------
  Serial.begin(9600);
  //Serial.setDebugOutput(true);  
  #if INITIAL_DELAY
  delay(5000);
  #endif
  
  Serial.println("\n Starting");

  //-----------------------------HARDWARE SETUP----------------------------------
  setup_ws_sensor();

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
	  measurements[idx].windspeed = calculate_windspeed(wscounter, usersettings.sample_period*60);
    Serial.println("######### WindSpeed MEASUREMENTS ###########");
	  Serial.print("BIN: ");
	  print_counter_state_bin(wscounter);
	  Serial.print("WS: ");
	  Serial.println(measurements[idx].windspeed);
    clear_counter();
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
    res = get_user_settings(&usersettings, GET_url);
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
    Serial.print("Number of samples to post");
    Serial.println(num_measurements);
    if (num_measurements > 0){
      post_measurements(measurements, num_measurements, POST_url);
      //ToDo: add fail check
      Serial.println("Posted!!");
    }
    num_measurements = 0;
  }

  //-----------------------------------Sleep----------------------------------------
  esp_sleep_enable_timer_wakeup(0.2*US_TO_S_FACTOR*60);
  esp_deep_sleep_start();

}

void loop() {
}