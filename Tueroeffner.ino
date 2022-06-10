#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define WIFI_SSID "AndroidAP"
#define WIFI_PASSWORD "rnq20cck"

#define MQTT_HOST IPAddress(192, 168, 43, 224)
#define MQTT_PORT 1883

#include <WiFiClient.h>
#include <ESP8266WebServer.h> 

const int sensorPin = D0;     // the number of the sensor pin
const int ledPin =  D1;      // the number of the LED pin
const int buttonPin = D2;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
  mqttClient.setWill("system/status", 1, true, "ende");
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(ledPin, OUTPUT);      
  pinMode(sensorPin, INPUT);
  pinMode(buttonPin, INPUT);

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}


void handleDoor() {
  int switchStatus = digitalRead(button_pin); 

  if(switchStatus == 0) {
     byte state = digitalRead(sensorPin);
      if(state == 1) {
          digitalWrite(ledPin,state);
          uint16_t packetIdPub2 = mqttClient.publish("door_lock", 1, true, String("Bewegung erkannt").c_str());
          Serial.println(packetIdPub2);
      }
      else if(state == 0) {
          digitalWrite(ledPin,state);
      } 
  }
  delay(500);
}


void loop() {  
  handleDoor();
}
