# ESP8266 Deep Sleep and WiFi connection

Simple Arduino based to test the time needed for ESP8266 to wake from deep sleep, connect to a Wi-Fi network, send an HTTP Post request with fake sensor data in JSON format and return to deep sleep.

## Motivation

Provide Hardware team with a rough estimate of the timings to choose a battery.

## Test Conditions

This test program was developed using the PlatformIO extesion for VSCode in Ubuntu 20.04. 

## Pre-Requisites
All the needed libraries and drivers are installed by default in PlatformIO

## HTTP Server
In order to test the http POST request [httpbin.org](https://httpbin.org/#/HTTP_Methods) was used. This service provides methods to test different http requests. In the case of a POST request it returns the request's parameters.

You can also change the server by replacing the server_host with your own server's IP address:
```
HTTPClient http;
String server_host = "http://httpbin.org/post";
```

You can check the payload by making the DEBUG flag true:
```
#define DEBUG 1
```

Now if you check the serial monitor you can view the payload from the request:

```
HTTP payload: {
  "args": {}, 
  "data": "{\"api_key\":\"SeaiHRules\",\"temp\":\"20.00\",\"hum\":\"5.00\",\"wind\":\"20.00\",\"noise\":\"60.00\",\"voltage\":\"3.20\"}", 
  "files": {}, 
  "form": {}, 
  "headers": {
    "Accept-Encoding": "identity;q=1,chunked;q=0.1,*;q=0", 
    "Content-Length": "100", 
    "Content-Type": "application/json", 
    "Host": "httpbin.org", 
    "User-Agent": "ESP8266HTTPClient", 
    "X-Amzn-Trace-Id": "Root=1-5f9033b0-4a0c7da332a4f9f43787e733"
  }, 
  "json": {
    "api_key": "SeaiHRules", 
    "hum": "5.00", 
    "noise": "60.00", 
    "temp": "20.00", 
    "voltage": "3.20", 
    "wind": "20.00"
  }, 
  "origin": "188.251.123.14", 
  "url": "http://httpbin.org/post"
}
HTTP Response code: 200
```

## Running the sketch

In order to run the sketch you only need to replace the NETWORK_SSID and PASSWORD placeholders with your own network's values:

```
WiFiClient client;
const char* wifi_ssid = "NETWORK_SSID";
const char* wifi_pwd = "PASSWORD";
```
After this compile the sketch and flash the microcontroller and you should be able to see the duration time needed for the wifi connection and post request in a serial monitor of your choice (using a baudrate of 9600bps).

## Results 

The time needed for the ESP to wakeup from deep sleep is not straightforward to test due to the microcontroller's clock being disabled and the RTC is difficult to access and can be unreliable so it's better to check the reset signals using an oscilloscope. According to this [guide](http://iot-bits.com/reducing-esp8266-deep-sleep-wakeup-time-current/) an expected wakeup time is around 140-160 ms.

To put the ESP to sleep time can be saved by using the `ESP.deepSleepInstant(microseconds, mode)` method which does not wait for WiFi Shutdown and can save around 90ms according to [this](https://blog.voneicken.com/2018/lp-wifi-esp8266-2/)

The duration of the wifi connection and post request were obtained using the results of 20 wakeup cycles:

|Task  | avg(ms)  |  min(ms) |  max(ms)|
|---|---|---|---|
|  Wi-Fi Connection| 3172.7  |  2026 | 7023|
|  POST Request |  358.8 | 297  |  662  |
|  All |  3570 | 2371  |  7382  |


## References

[Sleep Mode Comparison](https://www.losant.com/blog/making-the-esp8266-low-powered-with-deep-sleep)

[Deep Sleep Wakeup Comparison](http://iot-bits.com/reducing-esp8266-deep-sleep-wakeup-time-current/)

[Low power projects blog](https://blog.voneicken.com/projects/low-power-wifi-intro/)

[Random Nerd Tutorials for ESP8266](https://blog.voneicken.com/projects/low-power-wifi-intro/)
