#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>


#define WIFI_SSID "AndroidAP"
#define WIFI_PASSWORD "rnq20cck"

#define MQTT_HOST IPAddress(192, 168, 43, 224)
#define MQTT_PORT 1883

#define TRIGGER_PIN 16 //D0
#define ECHO_PIN 5 //D1

#define LED_RED_PIN D5
#define LED_GREEN_PIN D6
#define LED_BLUE_PIN D3

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

  sendPresenceData();
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

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);

  pinMode(TRIGGER_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an Input

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}

void sendPresenceData() {
  long duration;
  float distance;
  
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);
  
  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);

  if(distance > 50)
  {
    analogWrite(LED_GREEN_PIN,250);
    analogWrite(LED_RED_PIN,0);
    analogWrite(LED_BLUE_PIN,0);
  }
  else 
  {
      analogWrite(LED_GREEN_PIN,0);
    analogWrite(LED_RED_PIN,250);
    analogWrite(LED_BLUE_PIN,0);
  }
  
  uint16_t packetIdPub2 = mqttClient.publish("presence_detection", 1, true, String(distance).c_str());
  Serial.println(packetIdPub2);
}

void loop() {  
  delay(5000);
  sendPresenceData();
}
