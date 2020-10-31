#include <WiFi.h>
#include <HTTPClient.h>

#define DEBUG 0
#define SLEEP_TIME 10 //In seconds

WiFiClient client;
const char* wifi_ssid ="MEO-641C10"; //const char* wifi_ssid ="NETWORK_SSID";
const char* wifi_pwd = "9fa7582c76"; //const char* wifi_pwd = "PASSWORD";

HTTPClient http;
String server_host = "http://httpbin.org/post";

#define USE_TCP
#define FORCE_MODE 3

// Fake Data
String api_key = "SeaiHRules";
float temp = 20.0;
float hum = 5; //No clue 
float noise = 60;
float wind_speed = 20;
float voltage = 3.2;

//Timestamps
uint32_t t0, t1, t2, t3, t4;

void setup(){
  // D0 needs to be connected to RST in order to wakeup the ESP!
  pinMode(33, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0);
  //Setup Serial
  Serial.begin(9600);
  while(!Serial){};
  Serial.flush();

  Serial.println("I'm Awake");

  //Timestamp start of wifi connection
  t1 = millis();

  //Wait for WiFi Connection
  WiFi.begin(wifi_ssid, wifi_pwd);
  Serial.println("Connecting..");
  while (WiFi.status() != WL_CONNECTED){ //check for the connection
    delay(10);
  }

  //Timestamp connection succesful
  t2 = millis();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void loop(){
    t3 = millis();


    http.begin(client, server_host);
    http.addHeader("Content-Type", "application/json");

    // Sensor data in JSON format
    String httpRequestData = "{\"api_key\":\"" + api_key
                              + "\",\"temp\":\"" + String(temp) 
                              + "\",\"hum\":\"" + String(hum)
                              + "\",\"wind\":\"" + String(wind_speed)
                              + "\",\"noise\":\"" + String(noise)
                              + "\",\"voltage\":\"" + String(voltage)
                              + "\"}"; 

    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);
    String httpPayload = http.getString();
    
    #if DEBUG
    Serial.print("HTTP payload: ");
    Serial.print(httpPayload);
    #endif
   
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    // Close TCP connection 
    http.end();

    t4 = millis();

    // Print timings
    Serial.print("WiFi Connection duration: ");
    Serial.println(t2-t1);
    Serial.print("Post request duration: ");
    Serial.println(t4-t3);
    Serial.print("Full program duration: ");
    Serial.println(t4-t0);

    Serial.println("Going to sleep...");
    Serial.println();
    esp_deep_sleep_start();
}
