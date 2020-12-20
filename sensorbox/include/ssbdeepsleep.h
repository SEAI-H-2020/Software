#ifndef SSBDEEPSLEEP_H

#include <Arduino.h>
#include "esp_settings.h"

typedef struct {
    uint16_t wake_up_time = 0;
    uint16_t sample_counter = 1;
    uint16_t sync_counter = 1;
} dssettings_t;

void debug_sample_counter(dssettings_t dss, uint32_t wake_up_counter);

void debug_sync_counter(dssettings_t dss, uint32_t wake_up_counter);


#define SSBDEEPSLEEP_H
#endif