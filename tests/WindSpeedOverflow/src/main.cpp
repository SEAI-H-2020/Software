#include <Arduino.h>
#include "windsensor.h"

RTC_DATA_ATTR uint16_t wake_up_counter = 0;
RTC_DATA_ATTR uint16_t overflow_count = 0;


void go_sleep(){
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, HIGH);
  esp_sleep_enable_timer_wakeup(1); //For testing
  //esp_sleep_enable_timer_wakeup(dss.wake_up_time*US_TO_S_FACTOR*60);
  gpio_hold_en(GPIO_NUM_15);
  esp_deep_sleep_start();
}

uint8_t check_wakeup_reason(){
	esp_sleep_wakeup_cause_t wakeup_reason;
	wakeup_reason = esp_sleep_get_wakeup_cause();
  	switch(wakeup_reason){
    	case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    	case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    	case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    	case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    	case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    	default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  	}
 	return wakeup_reason;
}

void setup() {
	// put your setup code here, to run once:
	Serial.begin(9600);
	setup_ws_sensor();
	digitalWrite(13, LOW);
}

void loop() {
	timer_t dt = 1000; //1s
	if (wake_up_counter == 0){
		clear_counter();
	}
	wake_up_counter++;
	clear_counter();
	uint8_t wakeup_reason = check_wakeup_reason();
  	if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
  	  wake_up_counter--;
  	  overflow_count++;
  	  clear_counter();
  	  Serial.println("Overflowwwwwwww!!!!");
  	  /*
  	    NOTA! ta-se a descartar os valores entre o overflow e o tempo q micro acorda
  	    sera melhor medir o sensor???
  	  */
  	}

	uint16_t wscounter = read_windspeed_raw();
	float wspeed = calculate_windspeed(wscounter, msec2sec(dt));

	Serial.println("######### MEASUREMENTS ###########");
	Serial.print("BIN: ");
	print_counter_state_bin(wscounter);
	Serial.print("DEC: ");
	Serial.println(wscounter);
	Serial.print("WS: ");
	Serial.println(wspeed);
	Serial.print("NUM Overflow: ");
	Serial.println(overflow_count);

	go_sleep();

}
