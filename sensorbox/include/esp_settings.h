#ifndef ESP_SETTINGS_H

//#define OTA_UPDATE
//#define FIRMWARE_VERSION 0.2

#define MAX_MEASUREMENTS 80

//#define BATT_MEASURE

//#define DEBUG 
#ifdef DEBUG
// set up string messagesc
#define DEBUG_P(str)    \
   Serial.print("[DEBUG] "); \
   Serial.println(str);        
#else
    #define DEBUG_P(str) // just leaves an empty definition without serial printing
#endif

//#define RESET_WIFI_SETTINGS   
//#define INITIAL_DELAY         // initial delay of 5sec before printing out stuff

#define ESP_SETTINGS_H
#endif
