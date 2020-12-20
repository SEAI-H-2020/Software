#include "ssbsettings.h"


int get_user_settings(settings_t *usersettings, String endpoint){
    HTTPClient http;
    String payload;
    int sync_period_updated, sample_period_updated;

    http.begin(endpoint);
    // get json string
    int httpCode = http.GET();
    if (httpCode == 200) {
        payload = http.getString();
        //Serial.println(httpCode);
        //Serial.println(payload);
    }
    else {
        Serial.println("Error on HTTP request");
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
    
    Serial.println("sync_period = ");
    Serial.print(sync_period_updated);
    Serial.println();
    Serial.print("sample_period = ");
    Serial.print(sample_period_updated);
    Serial.println();
    http.end();

    usersettings->sync_period = sync_period_updated;
    usersettings->sample_period = sample_period_updated;
    
    /*
    I guess q pode-se sempre ir buscar as settings

    if (sync_period_updated != usersettings->sync_period 
        || sample_period_updated != usersettings->sample_period){  //estas 4 variaveis existem para verificar se os valores mudaram
      //wake_up_count = 0;                                                              //e se é necessário reiniciar o wake_up_count
      usersettings->sync_period = sync_period_updated;
      usersettings->sample_period = sample_period_updated;
      //Serial.print("wake_up_count = ");
      //Serial.print(wake_up_count);
      //Serial.println();
    }
    else{
      //wake_up_count++; Isto n devia tar aqui pq isto so corre no sync!
      //Serial.print("wake_up_count = ");
      //Serial.print(wake_up_count);
      //Serial.println();
    }
    */

    return 0;
}