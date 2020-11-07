#include <WiFi.h>
#include <FirebaseESP32.h>
 
 
#define FIREBASE_HOST "https://seai-database.firebaseio.com/"
#define FIREBASE_AUTH "K4FnftisHj97VFoelJ7bFtZme0Q18o7GVlPlT8FQ"
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"
 
 
//Define FirebaseESP32 data object
FirebaseData firebaseData;
FirebaseJson json;
int Vresistor = GPIO_NUM_32; //potenciometro de teste. Ignorar.
 
// Fake Data         DÁ ERRO QUANDO DECLARO COMO FLOAT... a DB NAO ACEITA FLOATS?
 String api_key = "SeaiHRules";
 int temp = 20.0;
 int hum = 5; //No clue 
 int noise = 60;
 int wind_speed = 20;
 int voltage = 3.2;
 
void setup()
{
 
  Serial.begin(921600);
 
 pinMode(32, INPUT);
 
 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
 
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
 
  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
 
  /*
  This option allows get and delete functions (PUT and DELETE HTTP requests) works for device connected behind the
  Firewall that allows only GET and POST requests.
  
  Firebase.enableClassicRequest(firebaseData, true);
  */
 
  //String path = "/data";
  
 
  Serial.println("------------------------------------");
  Serial.println("Connected...");
  
}
 
void loop()
{
 int Vrdata = analogRead(32); //Vrdata = analogRead(Vresistor);  
 int Sdata = map(Vrdata,0,4095,0,1000);
 Serial.println(Vrdata); //Serial.println(Sdata); 
delay(100); 
  json.set("/data", Sdata);
  json.set("/Temperature", temp);
  json.set("/Humidity", hum);
  json.set("/Noise", noise);
  json.set("/Wind Speed", wind_speed);
  Firebase.updateNode(firebaseData,"/Sensor",json);
}
