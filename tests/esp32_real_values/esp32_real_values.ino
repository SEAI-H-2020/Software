#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <dht11.h>
#define DHT11PIN GPIO_NUM_22  //é igual a int DHT11PIN = GPIO_NUM_22;

dht11 DHT11;

#define DEBUG 0
#define SLEEP_TIME 10 //In seconds
#define uS_TO_S_FACTOR 1000000

WiFiClient client;
const char* wifi_ssid ="MEO-641C10"; //const char* wifi_ssid ="NETWORK_SSID";
const char* wifi_pwd = "9fa7582c76"; //const char* wifi_pwd = "PASSWORD";

HTTPClient http;
String server_host = "http://750a6a4cc3a3.ngrok.io/measurements";

#define USE_TCP
#define FORCE_MODE 3

//Real Data
float temp = 0, hum = 0;

// Fake Data
  String api_key = "SeaiHRules";
  //float temp = 24.0;
  //float hum = 44;
  float noise = 77;
  float wind_speed = 30;
  float voltage = 3.2;

//Timestamps
  uint32_t t0, t1, t2, t3, t4;

void setup(){
  // D0 needs to be connected to RST in order to wakeup the ESP!
  //pinMode(33, INPUT_PULLUP); //WAKEUP PELO BOTÃO (TESTE)
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0); //WAKEUP PELO BOTÃO (TESTE)
  esp_sleep_enable_timer_wakeup(SLEEP_TIME* uS_TO_S_FACTOR);    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0);
  
  pinMode(22, INPUT); //  DHT11 PIN
  
  //Setup Serial
  Serial.begin(19200);
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
    int chk = DHT11.read(DHT11PIN);
    hum = (float)DHT11.humidity;
    temp = (float)DHT11.temperature;
    Serial.print("Humidity (%): ");
    Serial.println(hum, 2);//Serial.println((float)DHT11.humidity, 2);
    Serial.print("Temperature (C): ");
    Serial.println(temp, 2);//Serial.println((float)DHT11.temperature, 2);

    http.begin(client, server_host);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    // Add values in the document
    //
    doc["temperature"] = String(temp);
    doc["humidity"] = String(hum);
    doc["wind"] = String(wind_speed);
    doc["noise_level"] = String(noise);
    doc["voltage"] = String(voltage);

    String requestBody;
    serializeJson(doc, requestBody);
     
    int httpResponseCode = http.POST(requestBody);
/*
    // Sensor data in JSON format
    String httpRequestData = "{\"temperature\":\"" + String(temp) + "\",\"humidity\":\"" + String(hum)+ "\",\"wind\":\"" + String(wind_speed)+ "\",\"noise_level\":\"" + String(noise)+ "\",\"voltage\":\"" + String(voltage) + "}"; 

    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);
    String httpPayload = http.getString();
*/    
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
