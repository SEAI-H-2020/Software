#include <Arduino.h>
#include "windsensor.h"

void setup() {
	// put your setup code here, to run once:
	Serial.begin(9600);
	setup_ws_sensor();
}

void loop() {
	timer_t dt = 10000; //1s

	clear_counter();
	delay(dt);

	uint16_t wscounter = read_windspeed_raw();
	float wspeed = calculate_windspeed(wscounter, msec2sec(dt));

	Serial.println("######### MEASUREMENTS ###########");
	Serial.print("BIN: ");
	print_counter_state_bin(wscounter);
	Serial.print("WS: ");
	Serial.println(wspeed);

}
