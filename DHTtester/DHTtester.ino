#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

DHT dht;

#define DHT22_PIN 2

void setup()
{
  Serial.begin(9600);

  // initialize the DHT sensor
  dht.setup(DHT22_PIN);

  // initialize the LCD
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());
  lcd.setCursor(0, 0); //Set cursor at the begin of the first line
  lcd.print(String(dht.getHumidity()) + "%");
  lcd.setCursor(0, 1); //Set curos at the begin of the second line
  lcd.print(String(dht.getTemperature()));
  lcd.print((char)223); //Print degree icon °
  lcd.print("C");
  Serial.println(String(dht.getTemperature()));
}
