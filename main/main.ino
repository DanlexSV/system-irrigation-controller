#define SENSOR_PIN 35
#define RELAY_PIN 26
#define HUMEDAD_UMBRAL 3000
#define DURACION_RIEGO 9000

bool bombaEncendida = false;
unsigned long tiempoInicioRiego = 0;

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println("Sistema de riego automático listo.");
}

void loop() {
  int humedad = analogRead(SENSOR_PIN);
  Serial.print("Humedad del suelo (ADC): ");
  Serial.println(humedad);

  unsigned long ahora = millis();

  if (!bombaEncendida && humedad > HUMEDAD_UMBRAL) {
    Serial.println("Humedad baja. Activando bomba...");
    digitalWrite(RELAY_PIN, LOW);
    tiempoInicioRiego = ahora;
    bombaEncendida = true;
  }

  if (bombaEncendida && ahora - tiempoInicioRiego >= DURACION_RIEGO) {
    Serial.println("Tiempo cumplido. Apagando bomba.");
    digitalWrite(RELAY_PIN, HIGH);
    bombaEncendida = false;
  }

   delay(1000);
}