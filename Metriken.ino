#include <ESP8266WiFi.h>

//Spannung auslesen vorbereiten
ADC_MODE(ADC_VCC);
String Ubatt = "";
String ResetOrBoot = "";

void setup() {
  // Serielle Schnittstelle aktivieren für Debugging
  Serial.begin(115200);
  delay(100);
  Serial.println("Setup ...");
  // Grund für den Start erfahren -> Reset oder Stromversorgung hergestellt?
  const rst_info * resetInfo = system_get_rst_info();  // Resetgrund einlesen
  if ((resetInfo->reason) == 6) {
    ResetOrBoot = "Boot (Stromversorgung hergestellt)";
  } else {
    ResetOrBoot = "Reset";
  }
}

void loop() {
  Serial.println();
  // Warum booten wir?
  Serial.println("Auslöser: " + ResetOrBoot);
  Serial.println();
  Serial.print("Versorgungs- oder Batteriespannung:  ");
  // Betriebsspannung auslesen
  // genaue Spannung der Stromquelle, PIN A0 muss mit 3,3V Verbunden werden!
  uint16_t my_getVcc_Voltage = ESP.getVcc();
  float_t my_Voltage_calculated = ((float)my_getVcc_Voltage/1024.0f);
  Ubatt = String(my_Voltage_calculated, 3);
  Serial.println(Ubatt);

  delay(5000);
}
