#include "DHT.h"
#include <Wire.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;  // the Wifi radio's status

char servername[]="google.com";  // remote server we will connect to

WiFiClient client;
MqttClient mqttClient(client);

const char broker[] = "192.168.0.250";
int        port     = 1883;
const char topic[]  = "arduino/simple";

const long interval = 1000;
unsigned long previousMillis = 0;

DHT dht;

#define DHT22_PIN 2

String stateTopic = "home/temperature_sensor_living_room/state";


void sendMQTTTemperatureDiscoveryMsg() {
  String discoveryTopic = "homeassistant/sensor/temperature_sensor_living_room/temperature/config";
  String json;

  StaticJsonDocument<256> doc;

  doc["name"] = "Living room Temperature";
  doc["stat_t"]   = stateTopic;
  doc["unit_of_meas"] = "°C";
  doc["dev_cla"] = "temperature";
  doc["frc_upd"] = true;
  doc["val_tpl"] = "{{ value_json.temperature|default(0)| round(2) }}";

  serializeJson(doc, json);

  mqttClient.beginMessage(discoveryTopic);
  mqttClient.print(json);
  mqttClient.endMessage();
}

void sendMQTTHumidityDiscoveryMsg() {
  String discoveryTopic = "homeassistant/sensor/temperature_sensor_living_room/humidity/config";
  String json;

  StaticJsonDocument<256> doc;

  doc["name"] = "Living room Humidity";
  doc["stat_t"]   = stateTopic;
  doc["unit_of_meas"] = "%";
  doc["dev_cla"] = "humidity";
  doc["frc_upd"] = true;
  doc["val_tpl"] = "{{ value_json.humidity|default(0)| round(2) }}";

  serializeJson(doc, json);

  mqttClient.beginMessage(discoveryTopic);
  mqttClient.print(json);
  mqttClient.endMessage();
}

void connectWifi() {

  if (status != WL_CONNECTED) {
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
  Serial.print("You're connected to the network");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Attempting to connect to WPA network...");
  Serial.print("SSID: ");
  Serial.println(ssid);

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

  connectWifi();

  sendMQTTHumidityDiscoveryMsg();
  sendMQTTTemperatureDiscoveryMsg();

  // initialize the DHT sensor
  dht.setup(DHT22_PIN);

}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  connectWifi();

  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();

  Serial.println(String(temperature));
  Serial.println(String(humidity));

  StaticJsonDocument<256> doc;
  String json;

  doc["humidity"] = humidity;
  doc["temperature"]   = temperature;

  serializeJson(doc, json);

  mqttClient.beginMessage(stateTopic);
  mqttClient.print(json);
  mqttClient.endMessage();

  delay(60000);
}
