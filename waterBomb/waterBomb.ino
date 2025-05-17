// GPIO que controla el relé
constexpr uint8_t RELAY_PIN = 26;   // D26

// Ajusta a true si tu módulo de relé es activo-alto.
// La mayoría de los relés baratos son activo-bajo (0 = ON, 1 = OFF).
constexpr bool RELAY_ACTIVE_HIGH = false;

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);

  // Relé apagado al arrancar
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_HIGH ? LOW : HIGH);
  Serial.println("Envía 'Y' para encender la bomba, 'N' para apagarla.");
}

void loop() {
  if (Serial.available()) {
    char c = toupper(Serial.read());

    if (c == 'Y') {
      digitalWrite(RELAY_PIN, RELAY_ACTIVE_HIGH ? HIGH : LOW);
      Serial.println("Bomba ON");
    } else if (c == 'N') {
      digitalWrite(RELAY_PIN, RELAY_ACTIVE_HIGH ? LOW : HIGH);
      Serial.println("Bomba OFF");
    }
  }
}