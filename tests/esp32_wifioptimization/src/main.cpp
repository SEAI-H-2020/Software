#include <WiFi.h>
#include <HTTPClient.h>

#define DEBUG 0
#define SLEEP_TIME 10 //In seconds
#define SLEEP_TIME_IN_US SLEEP_TIME * 1e6

WiFiClient client;
const char* wifi_ssid ="MEO-775ED0"; //const char* wifi_ssid ="NETWORK_SSID";
const char* wifi_pwd = "b1a5cf47fc"; //const char* wifi_pwd = "PASSWORD";


HTTPClient http;
String server_host = "http://httpbin.org/post";

// Fake Data
String api_key = "SeaiHRules";
float temp = 20.0;
float hum = 5; //No clue 
float noise = 60;
float wind_speed = 20;
float voltage = 3.2;

//Timestamps
uint32_t t0, t1, t2, t3, t4;

// Data stored in RTC memory
RTC_DATA_ATTR unsigned int wake_up_counter = 0;
static RTC_DATA_ATTR struct{
  uint8_t bssid [6];
  int32_t channel;
} wifi_cfg;

// Function override
// First thing to run after the esp wakes up
void RTC_IRAM_ATTR esp_wake_deep_sleep(void){
  esp_default_wake_deep_sleep();
  wake_up_counter++;
} 

void setup(){
  //Setup Serial
  Serial.begin(9600);
  Serial.print("I'm Awake ");
  Serial.println(String(wake_up_counter));

  t1 = millis();
  WiFi.mode(WIFI_STA);
  
  if (wake_up_counter == 0){
    Serial.println("Attempting to connect for the first time..");
    WiFi.begin(wifi_ssid, wifi_pwd);
    while (WiFi.status() != WL_CONNECTED){ //check for the connection
      delay(10);
    }
    uint8_t* mac = WiFi.BSSID();
    //depois trocar por um memcpy
    for (uint8_t i = 0; i < sizeof(wifi_cfg.bssid); i++) wifi_cfg.bssid[i] = mac[i];
    wifi_cfg.channel = WiFi.channel();
  } 
  else {
    Serial.println("Attempting to connect..");
    WiFi.begin(wifi_ssid, wifi_pwd, wifi_cfg.channel, wifi_cfg.bssid);
    while (WiFi.status() != WL_CONNECTED){ //check for the connection
      delay(10);
      Serial.print("...");
    }
  }
  
  t2 = millis();
  Serial.println("Connection time" + String(t2-t1));
  Serial.println("Going to sleep");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_IN_US);
  esp_deep_sleep_start();
}

void loop() {

}