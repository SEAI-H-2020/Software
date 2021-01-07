#include <Arduino.h>
#include "windsensor.h"
void setup_ws_sensor(){
	pinMode(COUNTERQ1 , INPUT);	
	pinMode(COUNTERQ4 , INPUT);
	pinMode(COUNTERQ5 , INPUT);
	pinMode(COUNTERQ6 , INPUT);
	pinMode(COUNTERQ7 , INPUT);
	pinMode(COUNTERQ8 , INPUT);
	pinMode(COUNTERQ9 , INPUT);
	pinMode(COUNTERQ10 , INPUT);
	pinMode(COUNTERQ11 , INPUT);
	pinMode(COUNTERQ12 , INPUT);
	pinMode(COUNTERQ13 , INPUT);
	pinMode(COUNTERQ14 , INPUT);
	pinMode(COUNTER_RESET, OUTPUT);	
	digitalWrite(COUNTER_RESET, LOW);
}

void print_counter_state_bin(uint16_t wscounter){
	for(int cbit = 15; cbit >= 0; cbit--){
		Serial.print(bitRead(wscounter,cbit));
	}
	Serial.print("\r\n");
}

void clear_counter(){
	gpio_hold_dis(COUNTER_RESET_GPIO);
	digitalWrite(COUNTER_RESET, HIGH);
	delay(10);
	digitalWrite(COUNTER_RESET, LOW);
	gpio_hold_en(COUNTER_RESET_GPIO);
}

uint16_t read_windspeed_raw(){
	uint16_t wscounter = 0;

	wscounter += digitalRead(COUNTERQ1)  << 0;
	wscounter += digitalRead(COUNTERQ4)  << 3;
	wscounter += digitalRead(COUNTERQ5)  << 4;
	wscounter += digitalRead(COUNTERQ6)  << 5;
	wscounter += digitalRead(COUNTERQ7)  << 6;
	wscounter += digitalRead(COUNTERQ8)  << 7;
	wscounter += digitalRead(COUNTERQ9)  << 8;
	wscounter += digitalRead(COUNTERQ10)  << 9;
	wscounter += digitalRead(COUNTERQ11)  << 10;
	wscounter += digitalRead(COUNTERQ12)  << 11;
	wscounter += digitalRead(COUNTERQ13)  << 12;
	wscounter += digitalRead(COUNTERQ14)  << 13;

	return wscounter;

}

/*
	dt: time period given in seconds
	returns windspeed in m/s
*/
float calculate_windspeed(uint16_t wscounter, time_t dt){
	return kph2mps(2.4) *  (float) wscounter / dt;
}


float read_windspeed(time_t dt){
	return calculate_windspeed(read_windspeed_raw(), dt);
}