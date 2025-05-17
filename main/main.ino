#define SENSOR_PIN 35        // A0 del sensor FC-28 al pin GPIO 35
#define RELAY_PIN 26         // IN1 del relé al pin GPIO 26
#define HUMEDAD_UMBRAL 3000  // Ajusta según tu sensor y pruebas
#define DURACION_RIEGO 9000 // 9 segundos en milisegundos

bool bombaEncendida = false;
unsigned long tiempoInicioRiego = 0;

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);     // Pin del sensor como entrada
  pinMode(RELAY_PIN, OUTPUT);     // Pin del relé como salida
  digitalWrite(RELAY_PIN, HIGH);  // Bomba apagada (relé inactivo)
  Serial.println("Sistema de riego automático listo.");
}

void loop() {
  int humedad = analogRead(SENSOR_PIN);  // Leer valor analógico del sensor
  Serial.print("Humedad del suelo (ADC): ");
  Serial.println(humedad);

  unsigned long ahora = millis();

  // Si el suelo está seco y la bomba está apagada
  if (!bombaEncendida && humedad > HUMEDAD_UMBRAL) {
    Serial.println("Humedad baja. Activando bomba...");
    digitalWrite(RELAY_PIN, LOW);  // Relé activo (bomba encendida)
    tiempoInicioRiego = ahora;
    bombaEncendida = true;
  }

  // Si la bomba ya está encendida y pasó el tiempo de riego
  if (bombaEncendida && ahora - tiempoInicioRiego >= DURACION_RIEGO) {
    Serial.println("Tiempo cumplido. Apagando bomba.");
    digitalWrite(RELAY_PIN, HIGH); // Relé inactivo (bomba apagada)
    bombaEncendida = false;
  }

  delay(1000);  // Espera 1 segundo antes de la próxima lectura
}
