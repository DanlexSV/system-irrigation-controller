/*
 * Control cíclico de bomba de agua
 * Placa: ESP32, relé conectado al GPIO27
 * Encendido: 3 segundos, Apagado: 7 segundos (ciclo total: 10 segundos)
 */

const int RELAY_PIN = 27;  // IN2 del segundo relé

// Duraciones (en milisegundos)
const unsigned long TIEMPO_ENCENDIDO = 3000;
const unsigned long TIEMPO_APAGADO = 7000;
const unsigned long CICLO_TOTAL = TIEMPO_ENCENDIDO + TIEMPO_APAGADO;

unsigned long t0 = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Apagar bomba al inicio

  Serial.println("==== Ciclo automático de bomba ====");
}

void loop() {
  unsigned long tiempoActual = millis();
  unsigned long tiempoEnCiclo = (tiempoActual - t0) % CICLO_TOTAL;

  if (tiempoEnCiclo < TIEMPO_ENCENDIDO) {
    Serial.println("💦 Bomba ENCENDIDA");
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    Serial.println("🔇 Bomba APAGADA");
    digitalWrite(RELAY_PIN, LOW);
  }

  delay(500); // Evita mensajes excesivos por segundo
}
