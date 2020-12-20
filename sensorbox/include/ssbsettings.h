#ifndef SSBSETTINGS_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_settings.h"

typedef struct {
    uint16_t sync_period;
    uint16_t sample_period;
} settings_t;

int get_user_settings(settings_t *usersettings, String get_url, String ota_url);

#define SSBSETTINGS_H
#endif