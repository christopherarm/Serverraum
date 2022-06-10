#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define WIFI_SSID "AndroidAP"
#define WIFI_PASSWORD "rnq20cck"

#define MQTT_HOST IPAddress(192, 168, 43, 224)
#define MQTT_PORT 1883

#include <WiFiClient.h>
#include <ESP8266WebServer.h> 

int Buzzer = D2;        // used for ESP8266
int Gas_analog = A0;    // used for ESP8266
int Gas_digital = D1;   // used for ESP8266

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

  pinMode(Buzzer, OUTPUT);      
  pinMode(Gas_digital, INPUT);
  
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}


void handleGas() {
 int gassensorAnalog = analogRead(Gas_analog);
  int gassensorDigital = digitalRead(Gas_digital);

  Serial.print("Gas Sensor: ");
  Serial.print(gassensorAnalog);
  Serial.print("\t");
  Serial.print("Gas Class: ");
  Serial.print(gassensorDigital);
  Serial.print("\t");
  Serial.print("\t");
  
  if (gassensorAnalog > 100) {
    Serial.println("Gas");
    digitalWrite (Buzzer, HIGH) ; //send tone
    delay(1000);
    digitalWrite (Buzzer, LOW) ;  //no tone

    uint16_t packetIdPub2 = mqttClient.publish("presence_detection_infrared", 1, true, String("Gas erkannt").c_str());
    Serial.println(packetIdPub2);
  }
  else {
    Serial.println("No Gas");
  }
  delay(100);
}


void loop() {  
  handleGas();
}
