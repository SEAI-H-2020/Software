#ifndef WINDSENSOR_H
#define WINDSENSOR_H

#define COUNTERQ1 16
#define COUNTERQ4 17	
#define COUNTERQ5 18
#define COUNTERQ6 19
#define COUNTERQ7 21
#define COUNTERQ8 22
#define COUNTERQ9 23
#define COUNTERQ10 25
#define COUNTERQ11 26
#define COUNTERQ12 27
#define COUNTERQ13 32
#define COUNTERQ14 33
#define COUNTER_RESET 15

// Conversion from km/h to m/s
#define BASE_SPEED 2.4 // Speed in kph for 1 pulse per second
#define KPH_TO_MPS 0.277778
#define kph2mps(x) (x*KPH_TO_MPS) 

// Time conversions
#define MSEC_TO_SEC 1e-3
#define msec2sec(x) (x*MSEC_TO_SEC)

void setup_ws_sensor();

void print_counter_state_bin(uint16_t wscounter);

void clear_counter();

uint16_t read_windspeed_raw();

/*
	dt: time period given in seconds
	returns windspeed in m/s
*/
float calculate_windspeed(uint16_t wscounter, time_t dt);

float read_windspeed(time_t dt);


#endif