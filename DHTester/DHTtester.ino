#include "DHT.h"
#include <Wire.h>

DHT dht;

#define DHT22_PIN 2

void setup()
{
  Serial.begin(9600);

  // initialize the DHT sensor
  dht.setup(DHT22_PIN);

}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());
  Serial.println(String(dht.getTemperature()));
  Serial.println(String(dht.getHumidity()));
}
