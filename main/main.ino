#include <WiFi.h>

//const int sensorPin = 34;

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin();

  Serial.println("==== Información del dispositivo ====");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("Iniciando lectura de humedad...");
}

void loop() {
  /*
  int sensorValue = analogRead(sensorPin);
  
  int humedad = map(sensorValue, 0, 4095, 100, 0);
  
  Serial.print("Valor bruto: ");
  Serial.print(sensorValue);
  Serial.print(" | Humedad: ");
  Serial.print(humedad);
  Serial.println("%");

  delay(2000);
  */
}
