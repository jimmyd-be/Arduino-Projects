#include <SPI.h>
#include <WiFiNINA.h>
#include <MQTT.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
char server[] = SERVER_IP;
char channel[] = MQTT_CHANNEL;
char mqttUsername[] = MQTT_USERNAME;
char mqttPassword[] = MQTT_PASSWORD;
char mqttClient[] = DEVICE_NAME;


int status = WL_IDLE_STATUS;     // the Wifi radio's status

int buttonState = LOW;         // variable for reading the pushbutton status

static const uint8_t analog_pins[] = {A0, A1, A2, A3, A4, A5, A6};
static const String analog_pin_names[] = {"A0", "A1", "A2", "A3", "A4", "A5", "A6"};
static const int maxPorts = 14;

unsigned long lastMillis = 0;


WiFiClient net;
MQTTClient client;

void setup() {
  Serial.begin(9600);

  client.begin(server, net);

  Serial.println("Start initializing pins");


  for (int i = 0; i <= 6; i++) {

    pinMode(analog_pins[i], OUTPUT);
    digitalWrite(i, LOW);
  }

  for (int i = 0; i <= maxPorts; i++) {

    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);

  }

  Serial.println("Initializing pins done");

  initializeWifi();
  connectToMQTT();
}

void initializeWifi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  connectToWifi();
}

void connectToWifi() {
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

void connectToMQTT() {

  Serial.println("Connect to MQTT server");
  
  while (!client.connect(channel, mqttUsername, mqttPassword)) {
    Serial.println(".");
  }

}

void loop() {

  for (int i = 0; i <= 6; i++) {

    buttonState = digitalRead(analog_pins[i]);

    if (buttonState  == HIGH) {
      sendMessage(analog_pin_names[i]);
    }
  }

  for (int i = 0; i <= maxPorts; i++) {
    buttonState = digitalRead(i);

    if (buttonState == HIGH) {
      sendMessage(String(i));
    }
  }
}

void sendMessage(String pin) {

  client.loop();

  if (millis() - lastMillis > 1000) {
    lastMillis = millis();

    Serial.println("Pin " + pin + " got a signal");

    if (WiFi.status() != WL_CONNECTED) {
      connectToWifi();
    }

    if (!client.connected()) {
      connectToMQTT();
    }

    Serial.println("Send message to MQTT");

    client.publish(channel, createMessage(pin));
  }
}

String createMessage(String pin) {
  String message = "{\"pin\": \"%pin%\", \"device\": \"%device%\"}";
  message.replace("%pin%", pin);
  message.replace("%device%", mqttClient);

  return message;
}
