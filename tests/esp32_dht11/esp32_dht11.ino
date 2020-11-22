#include <dht11.h>
#define DHT11PIN GPIO_NUM_22  //é igual a int DHT11PIN = GPIO_NUM_22;

dht11 DHT11;

void setup()
{
  Serial.begin(921600);
 pinMode(22, INPUT);
}

void loop()
{
  Serial.println();

  int chk = DHT11.read(DHT11PIN);

  Serial.print("Humidity (%): ");
  Serial.println((float)DHT11.humidity, 2);

  Serial.print("Temperature (C): ");
  Serial.println((float)DHT11.temperature, 2);

  delay(2000);

}
