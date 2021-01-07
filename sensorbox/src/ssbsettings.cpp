#include "ssbsettings.h"
#include "ssbota.h"


int get_user_settings(settings_t *usersettings, String get_url, String ota_url){
    HTTPClient http;
    String payload;
    int sync_period_updated, sample_period_updated;

    http.begin(get_url);
    // get json string
    int httpCode = http.GET();
    if (httpCode == 200) {
        payload = http.getString();
        //DEBUG_PRINT(httpCode);
        //DEBUG_PRINT(payload);
    }
    else {
        DEBUG_PRINT("Error on HTTP request");
        return -1;
    }
    // changes json string so it can be deserialized
    char json[500];
    payload.replace(" ","");
    payload.replace("\n","");
    payload.trim();
    payload.remove(0,1);
    payload.toCharArray(json,500);
    // deserializes json string to GETdoc
    StaticJsonDocument<200> GETdoc;
    deserializeJson(GETdoc, json);
    sync_period_updated = GETdoc["sync_period"];    // memória do RTC
    sample_period_updated = GETdoc["sample_time"];  // memória do RTC
    
    DEBUG_PRINT("sync_period = ");
    DEBUG_PRINT(sync_period_updated);
    DEBUG_PRINT();
    DEBUG_PRINT("sample_period = ");
    DEBUG_PRINT(sample_period_updated);
    DEBUG_PRINT();

    usersettings->sync_period = sync_period_updated;
    usersettings->sample_period = sample_period_updated;



    http.end();

    #ifdef OTA_UPDATE
    float latest_firmware = GETdoc["latest_firmware"];
      #ifdef FIRMWARE_VERSION
        DEBUG_PRINT(FIRMWARE_VERSION);
        DEBUG_PRINT(latest_firmware);
        float curr_firmware = FIRMWARE_VERSION;
        if (curr_firmware < latest_firmware){
          DEBUG_PRINT("Your firmware version is V"+String(curr_firmware) + 
            ", the latest available firmware version is V"+String(latest_firmware));
          DEBUG_PRINT("Installing the new update now...");
          update_ota(ota_url, latest_firmware);
        }
        else{
          DEBUG_PRINT("Firmware is up to date");
        }
      #else 
        DEBUG_PRINT("Firmware version is not defined");
      #endif
    #endif
    /*
    I guess q pode-se sempre ir buscar as settings

    if (sync_period_updated != usersettings->sync_period 
        || sample_period_updated != usersettings->sample_period){  //estas 4 variaveis existem para verificar se os valores mudaram
      //wake_up_count = 0;                                                              //e se é necessário reiniciar o wake_up_count
      usersettings->sync_period = sync_period_updated;
      usersettings->sample_period = sample_period_updated;
      //DEBUG_PRINT("wake_up_count = ");
      //DEBUG_PRINT(wake_up_count);
      //DEBUG_PRINT();
    }
    else{
      //wake_up_count++; Isto n devia tar aqui pq isto so corre no sync!
      //DEBUG_PRINT("wake_up_count = ");
      //DEBUG_PRINT(wake_up_count);
      //DEBUG_PRINT();
    }
    */

    return 0;
}