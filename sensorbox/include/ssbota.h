#ifndef SSBOTA_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32httpUpdate.h>
#include "esp_settings.h"


int update_ota(String ota_url, float latest_fw);

#define SSBOTA_H
#endif