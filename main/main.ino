/*
 * Lectura de humedad de suelo (FC-28) + control de bomba
 * SIN Wi-Fi
 * Placa: ESP32, sensor AO en GPIO34, relé 2 en GPIO27
 */

const int SENSOR_PIN = 34;    // FC-28 AO
const int RELAY_PIN  = 27;    // IN2 del segundo relé

// Calibración humedad (ajústalo con tus lecturas)
const int ADC_HUMEDO = 1700;
const int ADC_SECO   = 3200;
const int UMBRAL_HUMEDAD = 40;  // (%) Por debajo de este valor se activa la bomba

// Intervalo entre lecturas
const unsigned long INTERVALO_MS = 2000;
unsigned long t0 = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Bomba apagada al inicio

  Serial.println("==== Sistema de riego automático ====");
  Serial.println("ADC  | %Humedad");
  Serial.println("-----------------------");
}

void loop() {
  if (millis() - t0 >= INTERVALO_MS) {
    t0 = millis();

    // Leer humedad
    int adc = analogRead(SENSOR_PIN);
    int humedad = map(adc, ADC_HUMEDO, ADC_SECO, 100, 0);
    humedad = constrain(humedad, 0, 100);

    Serial.printf("%4d | %3d %%\n", adc, humedad);

    // Control de bomba
    if (humedad < UMBRAL_HUMEDAD) {
      Serial.println("🌵 Suelo seco → Activando bomba");
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      Serial.println("💧 Suelo húmedo → Apagando bomba");
      digitalWrite(RELAY_PIN, LOW);
    }
  }
}
