#include "ssbdeepsleep.h"

void debug_sample_counter(dssettings_t dss, uint32_t wake_up_counter){
  DEBUG_P("wake_up_counter: ");
  DEBUG_P(wake_up_counter);
  DEBUG_P("\tsample_counter: ");
  DEBUG_P(dss.sample_counter);
  DEBUG_P("\tresult: ");
  DEBUG_P(wake_up_counter % dss.sample_counter);
}

void debug_sync_counter(dssettings_t dss, uint32_t wake_up_counter){
  DEBUG_P("wake_up_counter: ");
  DEBUG_P(wake_up_counter);
  DEBUG_P("\tsync_counter: ");
  DEBUG_P(dss.sync_counter);
  DEBUG_P("\tresult: ");
  DEBUG_P(wake_up_counter % dss.sync_counter);
}

uint8_t check_wakeup_reason(){
	esp_sleep_wakeup_cause_t wakeup_reason;
	wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 : DEBUG_P("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : DEBUG_P("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : DEBUG_P("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : DEBUG_P("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : DEBUG_P("Wakeup caused by ULP program"); break;
    default : DEBUG_P("Wakeup was not caused by deep sleep"); break;
  }
  return wakeup_reason;
}