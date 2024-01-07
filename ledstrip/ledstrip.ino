#include <Adafruit_NeoPixel.h>
#include <ArduinoHA.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

#ifdef __AVR__
#include <avr/power.h>
#endif

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;  // the Wifi radio's status
IPAddress brokerIp = BROKER_IP;

#define PIN 5
#define NUMPIXELS 60
#define BROKER_ADDR IPAddress(192, 168, 0, 250)

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

WiFiClient client;

HADevice device;
HAMqtt mqtt(client, device);

HALight light("tvLeds", HALight::BrightnessFeature | HALight::RGBFeature);

void connectToWifi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
}

void onStateCommand(bool state, HALight* sender) {
  Serial.print("State: ");
  Serial.println(state);

  if (state == 0) {
    pixels.clear();
    pixels.show();
  }

  sender->setState(state);  // report state back to the Home Assistant
}

void onBrightnessCommand(uint8_t brightness, HALight* sender) {
  Serial.print("Brightness: ");
  Serial.println(brightness);

  pixels.setBrightness(brightness);

  sender->setBrightness(brightness);  // report brightness back to the Home Assistant
}

void onRGBColorCommand(HALight::RGBColor color, HALight* sender) {
  Serial.print("Red: ");
  Serial.println(color.red);
  Serial.print("Green: ");
  Serial.println(color.green);
  Serial.print("Blue: ");
  Serial.println(color.blue);

  pixels.clear();

  for (int i = 0; i < NUMPIXELS; i++) {

    pixels.setPixelColor(i, pixels.Color(color.red, color.green, color.blue));
  }
  pixels.show();

  sender->setRGBColor(color);  // report color back to the Home Assistant
}


void configureHass() {

  // Unique ID must be set!
  byte mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));

  device.setName("TV led bar");
  device.setSoftwareVersion("1.0.0");
  device.setManufacturer("NeoPixel");
  device.setModel("WS2812b");

  // configure light (optional)
  light.setName("Tv");

  // Optionally you can set retain flag for the HA commands
  // light.setRetain(true);

  // Maximum brightness level can be changed as follows:
  // light.setBrightnessScale(50);

  // Optionally you can enable optimistic mode for the HALight.
  // In this mode you won't need to report state back to the HA when commands are executed.
  // light.setOptimistic(true);

  // Color temperature range (optional)
  // light.setMinMireds(50);
  // light.setMaxMireds(200);

  // handle light states
  light.onStateCommand(onStateCommand);
  light.onBrightnessCommand(onBrightnessCommand);  // optional
  light.onRGBColorCommand(onRGBColorCommand);      // optional

  mqtt.begin(brokerIp);

  Serial.println("MQTT begin");
}


void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  Serial.begin(9600);

  connectToWifi();

  configureHass();

  pixels.begin();
}

void loop() {

  mqtt.loop();
}