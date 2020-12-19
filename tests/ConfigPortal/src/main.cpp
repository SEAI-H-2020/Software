#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

#define RESET_WIFI_SETTINGS 1

WiFiManager wm;

void setup() {
  Serial.begin(9600);
  Serial.setDebugOutput(true);  
  delay(3000);
  Serial.println("\n Starting");

  #if RESET_WIFI_SETTINGS
  wm.resetSettings(); // wipe settings
  #endif

  //Parameters could be nice to have
  //Acho q por default n são guardados!!!
  //
  // add a custom input field
  //WiFiManagerParameter sync_time("synctime", "Sync Time", "5", customFieldLength);
  //WiFiManagerParameter sample_time("synctime", "Sync Time", "5", customFieldLength);
  //wm.addParameter(&sync_time);
  //wm.addParameter(&sample_time);

  // Para customizar o menu
  // 
  // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
  // const char* menu[] = {"wifi","info","param","sep","restart","exit"}; 
  // wm.setMenu(menu,6);

  std::vector<const char *> menu = {"wifi","info","sep","restart","exit"};
  wm.setMenu(menu);
  // wm.setCountry("Pt"); // n tem pt fdps

  // set dark theme
  wm.setClass("invert");
  
  //Timeouts
  //wm.setConnectTimeout(20); // how long to try to connect for before continuing
  //wm.setConfigPortalTimeout(60); // auto close configportal after n seconds

  bool res = wm.autoConnect("SmartSensorBox", "softwareH");

  if(!res) {
    Serial.println("Failed to connect or hit timeout");
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  }
}

void loop() {
}