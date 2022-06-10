#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define WIFI_SSID "AndroidAP"
#define WIFI_PASSWORD "rnq20cck"

#define MQTT_HOST IPAddress(192, 168, 43, 224)
#define MQTT_PORT 1883

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
 
#include "DHTesp.h"  //DHT11 Library for ESP
  
#define LED 2        //On board LED
#define DHTpin 14    //D5 of NodeMCU is GPIO14

#include <OneWire.h>
#include <DallasTemperature.h>

// Der PIN D2 (GPIO 4) wird als BUS-Pin verwendet
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

float vref = 3.3;
float resolution = vref/1023;



// In dieser Variable wird die Temperatur gespeichert
float temperature;

DHTesp dht;

float humidity, temperature2;

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
   // DS18B20 initialisieren
  DS18B20.begin();

    dht.setup(DHTpin, DHTesp::DHT11); //for DHT11 Connect DHT sensor to GPIO 17
  //dht.setup(DHTpin, DHTesp::DHT22); //for DHT22 Connect DHT sensor to GPIO 17
 handleADC();
 
}


void handleADC() {
 int rain = analogRead(A0);
 
 digitalWrite(LED,!digitalRead(LED)); //Toggle LED on data request ajax

 //Get Humidity temperatue data after request is complete
 //Give enough time to handle client to avoid problems
  delay(dht.getMinimumSamplingPeriod());

  humidity = dht.getHumidity();
  temperature2 = dht.getTemperature();

  Serial.print("H:");
  Serial.println(humidity);
  Serial.print("T:");
  Serial.println(temperature2); //dht.toFahrenheit(temperature));
  Serial.print("R:");
  Serial.println(rain);


     DS18B20.requestTemperatures();
  temperature = DS18B20.getTempCByIndex(0);

  // Ausgabe im seriellen Monitor
  Serial.println(String(temperature) + " °C");



  float temperature3 = analogRead(A0);
 temperature3 = (temperature3*resolution);
 temperature3 = temperature3*1023;
 temperature3 = (temperature3 - 32) * 5/9;
 Serial.println(temperature3);

   // 5 Sekunden warten
  delay(5000);
  
  uint16_t packetIdPub2 = mqttClient.publish("temperature_detection/1", 1, true, String(temperature).c_str());
  Serial.println(packetIdPub2);

    uint16_t packetIdPub3 = mqttClient.publish("humidity", 1, true, String(humidity).c_str());
  Serial.println(packetIdPub3);

      uint16_t packetIdPub4 = mqttClient.publish("temperature_detection/2", 1, true, String(temperature3).c_str());
  Serial.println(packetIdPub4);
}


void loop() {  
  delay(1000);
  handleADC();

}
