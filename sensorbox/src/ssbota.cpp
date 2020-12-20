#include "ssbota.h"

int update_ota(String ota_url, float latest_fw){
    t_httpUpdate_return ret = ESPhttpUpdate.update(ota_url, String(latest_fw));

    switch(ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("[update] Update failed (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            return -1;
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("[update] Update no Update.");
            return 0;
            break;
        case HTTP_UPDATE_OK:
            Serial.println("[update] Update ok."); // may not be called since we reboot the ESP
            return 1;
            break;
        default:
            return -1;
    }
    return -1;
}