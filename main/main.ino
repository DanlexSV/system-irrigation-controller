#include <WiFi.h>
#include <HTTPClient.h>

const char* SSID       = "Galaxy S24 Ultra E943";
const char* PASSWORD   = "hola12345";
const char* API_URL    = "https://daniel.fernandosilva.es/api/devices/humidity";

#define SENSOR_PIN 35
#define RELAY_PIN  26

const int   HUMEDAD_UMBRAL = 3000;
const uint32_t DURACION_RIEGO = 9000;

const float HUMIDITY_DELTA = 2.0;
float ultimoHumedadPct = -1.0;

bool bombaEncendida            = false;
unsigned long tiempoInicioRiego = 0;

void conectarWiFi();
int  leerHumedadADC();
float adcAporcentaje(int raw);
void revisarRiego(int humedadRaw);
void enviarHumedad(float humedadPct);


void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println("Sistema de riego automático listo.");

  conectarWiFi();
}

void loop() {
  int   humedadRaw = leerHumedadADC();
  float humedadPct = adcAporcentaje(humedadRaw);

  revisarRiego(humedadRaw);

  if (ultimoHumedadPct < 0.0 || fabs(humedadPct - ultimoHumedadPct) >= HUMIDITY_DELTA) {
    enviarHumedad(humedadPct);
    ultimoHumedadPct = humedadPct;
  }

  delay(1000);
}


void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  Serial.print("Conectando a WiFi");
  uint8_t intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print('.');
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("✅ Conectado al WiFi");
    Serial.print("IP local: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  } else {
    Serial.println();
    Serial.println("❌ No se pudo conectar al WiFi");
  }
}

int leerHumedadADC() {
  int lectura = analogRead(SENSOR_PIN);
  Serial.print("Humedad suelo (ADC): ");
  Serial.println(lectura);
  return lectura;
}

float adcAporcentaje(int raw) {
  float pct = 100.0 - ( (float)raw / 4095.0 ) * 100.0;
  return constrain(pct, 0, 100);
}

void revisarRiego(int humedadRaw) {
  unsigned long ahora = millis();

  if (!bombaEncendida && humedadRaw > HUMEDAD_UMBRAL) {
    Serial.println("Humedad baja. 🟢 Activando bomba...");
    digitalWrite(RELAY_PIN, LOW);
    tiempoInicioRiego = ahora;
    bombaEncendida = true;
  }

  if (bombaEncendida && (ahora - tiempoInicioRiego) >= DURACION_RIEGO) {
    Serial.println("⏰ Tiempo cumplido. 🔴 Apagando bomba.");
    digitalWrite(RELAY_PIN, HIGH);
    bombaEncendida = false;
  }
}

void enviarHumedad(float humedadPct) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi desconectado. Reconectando...");
    conectarWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("🚫 No se pudo enviar la lectura (sin WiFi)");
      return;
    }
  }

  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/json");

  String mac  = WiFi.macAddress();
  String body = String("{\"mac_address\":\"") + mac + "\",\"humidity\":" + String(humedadPct, 1) + "}";

  Serial.print("📤 POST ");
  Serial.println(body);

  int code = http.POST(body);
  if (code > 0) {
    Serial.printf("📥 HTTP %d\n", code);
    Serial.println(http.getString());
  } else {
    Serial.printf("❌ Error HTTP: %s\n", http.errorToString(code).c_str());
  }

  http.end();
}
