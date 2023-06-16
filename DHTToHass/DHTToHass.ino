#include "DHT.h"
#include <Wire.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>
#include <utility/wifi_drv.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;  // the Wifi radio's status
String stateTopic = "home/temperature_sensor_living_room/state";
const char broker[] = BROKER_IP;
int port = 1883;

WiFiClient client;
MqttClient mqttClient(client);

DHT dht;

#define DHT22_PIN 2


void sendMQTTTemperatureDiscoveryMsg() {
  String discoveryTopic = "homeassistant/sensor/temperature_sensor_living_room/temperature/config";
  String json;

  StaticJsonDocument<256> doc;

  doc["name"] = "Living room Temperature";
  doc["stat_t"] = stateTopic;
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
  doc["stat_t"] = stateTopic;
  doc["unit_of_meas"] = "%";
  doc["dev_cla"] = "humidity";
  doc["frc_upd"] = true;
  doc["val_tpl"] = "{{ value_json.humidity|default(0)| round(2) }}";

  serializeJson(doc, json);

  mqttClient.beginMessage(discoveryTopic);
  mqttClient.print(json);
  mqttClient.endMessage();
}

void changeLedColor(String color) {

  if (color == "red") {
    WiFiDrv::analogWrite(25, 0);    //GREEN
    WiFiDrv::analogWrite(26, 100);  //RED
    WiFiDrv::analogWrite(27, 0);    //BLUE
  } else if (color == "green") {
    WiFiDrv::analogWrite(25, 0);  //GREEN
    WiFiDrv::analogWrite(26, 0);  //RED
    WiFiDrv::analogWrite(27, 0);  //BLUE
  }
}

char ScanSSIDs()
//scan SSIDs, and if my SSID is present return 1
{
  char score = 0;
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) return (0);  //error
  for (int thisNet = 0; thisNet < numSsid; thisNet++)
    if (strcmp(WiFi.SSID(thisNet), ssid) == 0) score = 1;  //if one is = to my SSID
  return (score);
}

void testWiFiConnection()
//test if always connected
{
  int StatusWiFi = WiFi.status();
  if (StatusWiFi == WL_CONNECTION_LOST || StatusWiFi == WL_DISCONNECTED || StatusWiFi == WL_SCAN_COMPLETED)  //if no connection
  {
    changeLedColor("red");
    if (ScanSSIDs()) connectWifi();  //if my SSID is present, connect
  }
}

void connectWifi()
//connect to my SSID
{
  status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    if (status == WL_CONNECTED) digitalWrite(9, HIGH);  //LED ON to show connected
    else delay(500);
  }

  changeLedColor("green");
  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");

  Serial.println("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.println("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1)
      ;
  }

  Serial.println("You're connected to the MQTT broker!");
}

void setup() {
  Serial.begin(9600);
  Serial.println("Attempting to connect to WPA network...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  //Led configuration
  WiFiDrv::pinMode(25, OUTPUT);
  WiFiDrv::pinMode(26, OUTPUT);
  WiFiDrv::pinMode(27, OUTPUT);

  changeLedColor("red");

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

void loop() {
  delay(dht.getMinimumSamplingPeriod());

  testWiFiConnection();

  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();

  Serial.println(String(temperature));
  Serial.println(String(humidity));

  StaticJsonDocument<256> doc;
  String json;

  doc["humidity"] = humidity;
  doc["temperature"] = temperature;

  serializeJson(doc, json);

  mqttClient.beginMessage(stateTopic);
  mqttClient.print(json);
  mqttClient.endMessage();

  delay(60000);
}
