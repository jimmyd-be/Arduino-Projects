#include <Adafruit_NeoPixel.h>
#include <ArduinoHA.h>
#include <WiFiNINA.h>
#include <ArduinoOTA.h>
#include "arduino_secrets.h"

enum Mode {
  AUTO,
  MANUAL,
  WAVE,
  FAST_WAVE,
  ALL
};


///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;  // the Wifi radio's status
IPAddress brokerIp = BROKER_IP;

const unsigned long REFRESH_INTERVAL = 1000;  // ms
unsigned long lastRefreshTime = 0;

Mode mode = AUTO;
int hue = 0;
int wavePosition = 0;

#define PIN 5
#define NUMPIXELS 60

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

WiFiClient client;

HADevice device;
HAMqtt mqtt(client, device);

HALight light("tvLeds", HALight::RGBFeature);
HASelect modeSelect("modeSelect");


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

  ArduinoOTA.begin(WiFi.localIP(), "Arduino led bar", "password", InternalStorage);
}

void onSelectCommand(int8_t index, HASelect* sender) {
  Serial.print("State: ");
  Serial.println(index);

  switch (index) {
    case 0:
      // Option "Auto" was selected
      mode = AUTO;
      break;

    case 1:
      // Option "Manual" was selected
      mode = MANUAL;
      break;

    case 2:
      // Option "Wave" was selected
      mode = WAVE;
      break;

    case 3:
      mode = FAST_WAVE;
      break;

    case 4:
      mode = ALL;
      break;

    default:
      mode = AUTO;
      return;
  }

  sender->setState(index);
}

void onStateCommand(bool state, HALight* sender) {
  Serial.print("State: ");
  Serial.println(state);

  if (state == false) {
    pixels.clear();
    pixels.show();
    mode = MANUAL;
    modeSelect.setState(1);
  }
  sender->setState(state);
}


void onRGBColorCommand(HALight::RGBColor color, HALight* sender) {
  Serial.print("Red: ");
  Serial.println(color.red);
  Serial.print("Green: ");
  Serial.println(color.green);
  Serial.print("Blue: ");
  Serial.println(color.blue);

  mode = MANUAL;

  pixels.clear();

  for (int i = 0; i < NUMPIXELS; i++) {

    pixels.setPixelColor(i, pixels.Color(color.red, color.green, color.blue));
  }
  pixels.show();

  sender->setRGBColor(color);
  modeSelect.setState(1);
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

  light.setName("Tv");
  light.setCurrentState(true);

  modeSelect.setName("Mode");
  modeSelect.setCurrentState(0);
  modeSelect.setOptions("Auto;Manual;Wave;Fast Wave;All");

  light.onStateCommand(onStateCommand);
  light.onRGBColorCommand(onRGBColorCommand);
  modeSelect.onCommand(onSelectCommand);

  mqtt.begin(brokerIp);
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void calculateColor() {

  hue += 1;

  if (hue >= 256) {
    hue = 0;
  }

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, Wheel(hue));
  }
  pixels.show();

  uint8_t red = (pixels.getPixelColor(0) >> 16);
  uint8_t green = (pixels.getPixelColor(0) >> 8);
  uint8_t blue = (pixels.getPixelColor(0));

  HALight::RGBColor color(red, green, blue);
  light.setRGBColor(color);

  // Print the updated color values
  Serial.print("Hue: ");
  Serial.print(hue);
  Serial.print(", Red: ");
  Serial.print(red);
  Serial.print(", Green: ");
  Serial.print(green);
  Serial.print(", Blue: ");
  Serial.print(blue);
  Serial.println();
}


void setup() {
  Serial.begin(9600);

  connectToWifi();

  configureHass();

  pixels.begin();
}

void loop() {

  ArduinoOTA.poll();

  mqtt.loop();

  if (light.getCurrentState()) {

    if (mode == AUTO) {

      Serial.println("Auto mode");

      if (millis() - lastRefreshTime >= REFRESH_INTERVAL) {
        lastRefreshTime += REFRESH_INTERVAL;

        calculateColor();
      }
    } else if (mode == WAVE) {

      Serial.println("Wave mode");

      int rotationPhase = (millis() / 5000) % NUMPIXELS;

      for (int i = 0; i < NUMPIXELS; i++) {
        int hue = 255 * ((i + rotationPhase) % NUMPIXELS) / NUMPIXELS;

        pixels.setPixelColor(i, Wheel(hue));
      }
      pixels.show();

      // Delay to control the speed of the wave
      delay(50);
    } else if (mode == FAST_WAVE) {

      Serial.println("Fast Wave mode");

      int rotationPhase = (millis() / 50) % NUMPIXELS;

      for (int i = 0; i < NUMPIXELS; i++) {
        int hue = 255 * ((i + rotationPhase) % NUMPIXELS) / NUMPIXELS;

        pixels.setPixelColor(i, Wheel(hue));
      }
      pixels.show();

      // Delay to control the speed of the wave
      delay(50);
    } else if (mode == ALL) {

      Serial.println("All mode");

      // Create a moving wave effect
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, Wheel((255 / NUMPIXELS) * i));
      }
      pixels.show();

      // Delay to control the speed of the wave
      delay(50);
    }
  }
}