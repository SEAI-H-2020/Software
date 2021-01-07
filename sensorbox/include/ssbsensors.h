#ifndef SSBSENSORS_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "windsensor.h"
#include "esp_settings.h"

#define DHTPIN GPIO_NUM_13
#define DHTTYPE DHT22
#define MIC 35
#define PIN_SUPPLY 14
#define PIN_SUPPLY_GPIO GPIO_NUM_14

typedef struct {
    float temp;
    float hum;
    float windspeed;
    float noise;
    float voltage;
    char timestamp[20];
} measurement_t;

int post_measurements(measurement_t measurements[], uint8_t num_measurements, String endpoint);

#define SSBSENSORS_H
#endif