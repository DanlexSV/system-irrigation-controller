#define RELAY_PIN 26

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
}

void loop() {
  Serial.println("Encendiendo bomba");
  digitalWrite(RELAY_PIN, LOW);
  delay(5000);

  Serial.println("Apagando bomba");
  digitalWrite(RELAY_PIN, HIGH);
  delay(5000);
}