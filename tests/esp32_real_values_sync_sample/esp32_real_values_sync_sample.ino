#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <dht11.h>
#include "DHT.h"
#define DHTPIN GPIO_NUM_22  //é igual a int DHT11PIN = GPIO_NUM_22;
#define DHTTYPE DHT22
#define BatteryPin GPIO_NUM_32 
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino
#define DEBUG 0

//sync and sample variables
RTC_DATA_ATTR int sync_period;
RTC_DATA_ATTR int sample_period;
RTC_DATA_ATTR int sync_counter;
RTC_DATA_ATTR int sample_counter;

//#define SLEEP_TIME 10 //In seconds
#define uS_TO_S_FACTOR 1000000
WiFiClient client;
const char* wifi_ssid ="MEO-641C10"; //const char* wifi_ssid ="NETWORK_SSID";
const char* wifi_pwd = "9fa7582c76"; //const char* wifi_pwd = "PASSWORD";

HTTPClient http;
String POST_url = "http://smartsensorbox.ddns.net:5000/measurements";
String GET_url = "http://smartsensorbox.ddns.net:5000/usersettings/1";

#define USE_TCP
#define FORCE_MODE 3
int wake_up_count = 0;
//unsigned int sync_period = 20;
//unsigned int sample_time = 10;
int sync_period_updated;
int sample_period_updated;
int bat = 0;    //Variável que guarda o valor digital da tensao na bateria

String payload; // for the GET request
//Real Data
float temp = 0, hum = 0;
// Fake Data
  String api_key = "SeaiHRules";
  //float temp = 24.0;
  //float hum = 44;
  float noise = 77;
  float wind_speed = 30;
  float voltage = 0;

//Timestamps
  uint32_t t0, t1, t2, t3, t4;

void setup(){
  
  pinMode(32, INPUT); //  BATTERY PIN
  pinMode(22, INPUT); //  DHT11 PIN
  
  //Setup Serial
  Serial.begin(19200);
  while(!Serial){};
  Serial.flush();
  
  //Serial.println("I'm Awake");
  
  //Timestamp start of wifi connection
  t1 = millis();
  
  ////Wait for WiFi Connection
  //WiFi.begin(wifi_ssid, wifi_pwd);
  //Serial.println("Connecting..");
  //while (WiFi.status() != WL_CONNECTED){ //check for the connection
  //  delay(10);
  //}
  //--------------------------------WIFI CONNECTION--------------------------------
  WiFi.begin(wifi_ssid, wifi_pwd);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  //--------------------------------OTA CODE--------------------------------
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  dht.begin();
  //Serial.println("Ready");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
  
  //Timestamp connection succesful
  t2 = millis();
  
  //Serial.print("Connected, IP address: ");
  //Serial.println(WiFi.localIP());
}
  
void loop(){
    t3 = millis();
    ArduinoOTA.handle();
    
    //--------------------------------GET REQUEST--------------------------------
    http.begin(GET_url);
    // get json string
    int httpCode = http.GET();
    if (httpCode > 0) {
        payload = http.getString();
        //Serial.println(httpCode);
        //Serial.println(payload);
    }
    else {
        Serial.println("Error on HTTP request");
    }
    // changes json string so it can be deserialized
    char json[500];
    payload.replace(" ","");
    payload.replace("\n","");
    payload.trim();
    payload.remove(0,1);
    payload.toCharArray(json,500);
    // deserializes json string to GETdoc
    StaticJsonDocument<200> GETdoc;
    deserializeJson(GETdoc, json);
    sync_period_updated = GETdoc["sync_period"];    // memória do RTC
    sample_period_updated = GETdoc["sample_time"];  // memória do RTC
    
    Serial.println("sync_period = ");
    Serial.print(sync_period_updated);
    Serial.println();
    Serial.print("sample_period = ");
    Serial.print(sample_period_updated);
    Serial.println();
    http.end();

    if (sync_period_updated != sync_period || sample_period_updated != sample_period){  //estas 4 variaveis existem para verificar se os valores mudaram
      wake_up_count = 0;                                                              //e se é necessário reiniciar o wake_up_count
      sync_period = sync_period_updated;
      sample_period = sample_period_updated;
      Serial.print("wake_up_count = ");
      Serial.print(wake_up_count);
      Serial.println();
    }
    else{
      wake_up_count++;
      Serial.print("wake_up_count = ");
      Serial.print(wake_up_count);
      Serial.println();
    }
    
   int wake_up_time = min(sync_period, sample_period);
   sample_counter = sample_period/wake_up_time;
   sync_counter = sync_period/wake_up_time;
   
   // declarar variaveis dos sensores aqui-----------------------------------------------------------------------------
   float temp[sample_counter];
   float hum[sample_counter];
   float voltage[sample_counter];
   //float noise[sample_counter];
   // Fake Data
   float noise = 43;
   float wind_speed = 17;
   
   if (wake_up_count % sample_counter == 0){
    //rotina de sample --> guarda os valores dos sensores em vetores
    int i = 0;
    while (i < sample_counter){
      bat = analogRead(BatteryPin);
      voltage[i] = (float)5*bat/4095;
      Serial.print("voltage = ");
      Serial.print(voltage[i]);
      Serial.println();
    
      // read temperature and humidity values
      //int chk = DHT11.read(DHT11PIN);

      hum[i] = dht.readHumidity();
      temp[i] = dht.readTemperature();
      //hum[i] = (float)DHT11.humidity;
      //temp[i] = (float)DHT11.temperature;
      
      // display values to serial
      //Serial.print("Humidity (%): ");
      //Serial.println(hum[i], 2);//Serial.println((float)DHT11.humidity, 2);
      //Serial.print("Temperature (C): ");
      //Serial.println(temp[i], 2);//Serial.println((float)DHT11.temperature, 2);
      i++;
    }
    Serial.print("i = ");
    Serial.print(i);
    Serial.println();
   }
   if (wake_up_count % sync_counter == 0){
    //fazer rotina de sync --> criar vetores para guardar e enviar todos os valores de uma vez num while
        //--------------------------------POST REQUEST--------------------------------
    http.begin(client, POST_url);
    http.addHeader("Content-Type", "application/json");
    StaticJsonDocument<200> POSTdoc;
    // Add values in the document
    int i = 0;
    while (i < sample_counter){
      POSTdoc["temperature"] = String(temp[i]);
      POSTdoc["humidity"] = String(hum[i]);
      POSTdoc["wind"] = String(wind_speed);
      POSTdoc["noise_level"] = String(noise);
      POSTdoc["voltage"] = String(voltage[i]);
      i++;
    }
      String requestBody;
      // build Json string
      serializeJson(POSTdoc, requestBody);
      // post Json string
      int httpResponseCode = http.POST(requestBody);

      #if DEBUG
      Serial.print("HTTP payload: ");
      Serial.print(httpPayload);
      #endif
      // POST test
      //Serial.print("HTTP Response code: ");
      //Serial.println(httpResponseCode);
      
    
    // Close TCP connection 
    http.end();
   }

  //--------------------------------WAKE UP ROUTINES--------------------------------
  // D0 needs to be connected to RST in order to wakeup the ESP!
  //pinMode(33, INPUT_PULLUP); //WAKEUP PELO BOTÃO (TESTE)
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0); //WAKEUP PELO BOTÃO (TESTE)
  esp_sleep_enable_timer_wakeup(wake_up_time* uS_TO_S_FACTOR*60);  //counter in minutes
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0);

  /*
  //--------------------------------READ SENSOR VALUES-------------------------------- está no ciclo sample
    // read battery voltage level
    bat = analogRead(BatteryPin);
    voltage = (float)5*bat/4095;
    Serial.print("voltage = ");
    Serial.print(voltage);
    Serial.println();
    
    // read temperature and humidity values
    int chk = DHT11.read(DHT11PIN);
    hum = (float)DHT11.humidity;
    temp = (float)DHT11.temperature;
    // display values to serial
    Serial.print("Humidity (%): ");
    Serial.println(hum, 2);//Serial.println((float)DHT11.humidity, 2);
    Serial.print("Temperature (C): ");
    Serial.println(temp, 2);//Serial.println((float)DHT11.temperature, 2);
*/
    

    
    
    t4 = millis();
    /*
    // Print timings
    Serial.print("WiFi Connection duration: ");
    Serial.println(t2-t1);
    Serial.print("Post request duration: ");
    Serial.println(t4-t3);
    Serial.print("Full program duration: ");
    Serial.println(t4-t0);
    Serial.println("Going to sleep...");
    Serial.println();
    */
    //--------------------------------DEEP SLEEP START--------------------------------

    esp_deep_sleep_start();
}
