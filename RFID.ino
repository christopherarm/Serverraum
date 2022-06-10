#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>


#define WIFI_SSID "AndroidAP"
#define WIFI_PASSWORD "rnq20cck"

#define MQTT_HOST IPAddress(192, 168, 43, 224)
#define MQTT_PORT 1883

#include <SPI.h>
#include <MFRC522.h>
 
#define SS_PIN D8
#define RST_PIN D3
const int ledPin = D0;
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;



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

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();

  SPI.begin();
  rfid.PCD_Init();
   
  Serial.println("Boot RFID-Reader...");
   
  pinMode(ledPin, OUTPUT);
}


void loop() {  
  delay(1000);
  digitalWrite(ledPin,LOW);
  handleRFID();
}

void handleRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;
  Serial.println(printHex(rfid.uid.uidByte, rfid.uid.size));
   
  digitalWrite(ledPin,HIGH);
  delay(5000);
  digitalWrite(ledPin,LOW);
   
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  }
   
  String printHex(byte *buffer, byte bufferSize) {
  String id = "";
  for (byte i = 0; i < bufferSize; i++) {
  id += buffer[i] < 0x10 ? "0" : "";
  id += String(buffer[i], HEX);
  }
      uint16_t packetIdPub2 = mqttClient.publish("door_lock", 1, true, String(id).c_str());
    Serial.println(packetIdPub2);
  return id;
}
