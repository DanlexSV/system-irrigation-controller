#define RELAY_PIN 26   // GPIO26

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);   // relé OFF (activo-bajo)
}

void loop() {
  Serial.println("Encendiendo bomba");
  digitalWrite(RELAY_PIN, LOW);    // relé ON
  delay(5000);

  Serial.println("Apagando bomba");
  digitalWrite(RELAY_PIN, HIGH);   // relé OFF
  delay(5000);
}