#include "ssbdeepsleep.h"

void debug_sample_counter(dssettings_t dss, uint32_t wake_up_counter){
  Serial.print("wake_up_counter: ");
  Serial.print(wake_up_counter);
  Serial.print("\tsample_counter: ");
  Serial.print(dss.sample_counter);
  Serial.print("\tresult: ");
  Serial.println(wake_up_counter % dss.sample_counter);
}

void debug_sync_counter(dssettings_t dss, uint32_t wake_up_counter){
  Serial.print("wake_up_counter: ");
  Serial.print(wake_up_counter);
  Serial.print("\tsync_counter: ");
  Serial.print(dss.sync_counter);
  Serial.print("\tresult: ");
  Serial.println(wake_up_counter % dss.sync_counter);
}