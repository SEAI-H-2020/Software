#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <dht11.h>
#define DHT11PIN GPIO_NUM_22  //é igual a int DHT11PIN = GPIO_NUM_22;
#define BatteryPin GPIO_NUM_32 

dht11 DHT11;

#define DEBUG 0
//#define SLEEP_TIME 10 //In seconds
#define uS_TO_S_FACTOR 1000000

WiFiClient client;
const char* wifi_ssid ="MEO-641C10"; //const char* wifi_ssid ="NETWORK_SSID";
const char* wifi_pwd = "9fa7582c76"; //const char* wifi_pwd = "PASSWORD";

HTTPClient http;
String server_host = "http://smartsensorbox.ddns.net:5000/measurements";

#define USE_TCP
#define FORCE_MODE 3
unsigned int sync_period = 20;
unsigned int sample_time = 10;
int bat = 0;    //Variável que guarda o valor digital da tensao na bateria

String payload; // GET request
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

    //GET REQUEST
    http.begin("http://smartsensorbox.ddns.net:5000/usersettings/1");
    // get json string
    int httpCode = http.GET();
    if (httpCode > 0) {
        payload = http.getString();
        //Serial.println(httpCode);
        Serial.println(payload);
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
    int sync_period = GETdoc["sync_period"];
    int sample_time = GETdoc["sample_time"];
    //float GETtemp = GETdoc["temperature"];
    //float GEThum = GETdoc["humidity"];
    //float GETwind = GETdoc["wind"];
    //float GETnoise = GETdoc["noise_level"];
    //float GETvoltage = GETdoc["voltage"];
    Serial.println("sync_period = ");
    Serial.print(sync_period);
    Serial.println();
    Serial.print("sample_time = ");
    Serial.print(sample_time);
    Serial.println();
    http.end();
    
  // D0 needs to be connected to RST in order to wakeup the ESP!
  //pinMode(33, INPUT_PULLUP); //WAKEUP PELO BOTÃO (TESTE)
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0); //WAKEUP PELO BOTÃO (TESTE)
  esp_sleep_enable_timer_wakeup(sync_period* uS_TO_S_FACTOR);    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0);

    // read battery voltage level
    bat = analogRead(BatteryPin);
    voltage = (float)5*bat/4095;
    Serial.print("bat = ");
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
    
    //POST REQUEST
    http.begin(client, server_host);
    http.addHeader("Content-Type", "application/json");
    StaticJsonDocument<200> POSTdoc;
    // Add values in the document
    POSTdoc["temperature"] = String(temp);
    POSTdoc["humidity"] = String(hum);
    POSTdoc["wind"] = String(wind_speed);
    POSTdoc["noise_level"] = String(noise);
    POSTdoc["voltage"] = String(voltage);
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
