#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>

Preferences prefs;


const char* BASE_URL = "https://daniel.fernandosilva.es/api/devices/";
const char* ENDPOINTS[] = {
  "humidity",
  "watering/check",
  "watering/auto"
};
enum Endpoint { HUMIDITY = 0, CHECK_WATERING, AUTO_WATERING };

#define SENSOR_PIN 35
#define RELAY_PIN  26

const int   HUMEDAD_UMBRAL = 3000;
const uint32_t DURACION_RIEGO = 9000;

const float HUMIDITY_DELTA = 2.0;
float ultimoHumedadPct = -1.0;

bool bombaEncendida            = false;
unsigned long tiempoInicioRiego = 0;


bool   conectarWiFi();
bool   leerCredencialesNVS(String &outSSID, String &outPassword);
void   guardarCredencialesNVS(const String &ssid, const String &password);
void   borrarCredencialesNVS();
String leerLineaPorSerial(const char *prompt);

int    leerHumedadADC();
float  adcAporcentaje(int raw);
void   revisarRiego();
void   regarPlanta(int durationMs);
void   controlarBomba();
void   enviarHumedad(float humedadPct);
void   enviarAutoWatering(int durationSeconds);


void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println("Sistema de riego automático iniciando...");

  bool wifiOk = conectarWiFi();
  if (!wifiOk) {
    Serial.println("No se logró conectar a ninguna red WiFi. Se suspende lógica de riego.");
  } else {
    Serial.println("Continuando con lógica de riego...");
  }
}

void loop() {
  int   humedadRaw = leerHumedadADC();
  float humedadPct = adcAporcentaje(humedadRaw);

  if (ultimoHumedadPct < 0.0 || fabs(humedadPct - ultimoHumedadPct) >= HUMIDITY_DELTA) {
    enviarHumedad(humedadPct);
    ultimoHumedadPct = humedadPct;

    if (humedadPct < 40.0) {
      revisarRiego();
    }
  }

  controlarBomba();

  delay(500);
}


bool conectarWiFi() {
  String ssid, password;

  if (leerCredencialesNVS(ssid, password)) {
    Serial.println("Credenciales encontradas en NVS:");
    Serial.print("    SSID: "); Serial.println(ssid);
    Serial.print("    PASSWORD: "); Serial.println(password);
  } else {
    ssid     = "";
    password = "";
  }

  bool conectado = false;
  uint8_t intentosGlobal = 0;

  while (!conectado && intentosGlobal < 5) {
    if (ssid.length() > 0 && password.length() > 0) {
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid.c_str(), password.c_str());

      Serial.print("Conectando a WiFi: ");
      Serial.print(ssid);
      unsigned long start = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) { // timeout 10s
        Serial.print('.');
        delay(500);
      }
      Serial.println();

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Conectado al WiFi.");
        Serial.print("IP local: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC: ");
        Serial.println(WiFi.macAddress());
        conectado = true;
        break;
      } else {
        Serial.println("No se pudo conectar con esas credenciales.");
        borrarCredencialesNVS();
        ssid     = "";
        password = "";
      }
    }

    Serial.println("▶ Introduce SSID y PASSWORD por Serial:");
    ssid     = leerLineaPorSerial("   SSID: ");
    password = leerLineaPorSerial("   PASSWORD: ");

    guardarCredencialesNVS(ssid, password);

    intentosGlobal++;
  }

  return conectado;
}

bool leerCredencialesNVS(String &outSSID, String &outPassword) {
  prefs.begin("wifiCreds", true);
  if (prefs.isKey("ssid") && prefs.isKey("pass")) {
    outSSID     = prefs.getString("ssid", "");
    outPassword = prefs.getString("pass", "");
    prefs.end();
    return true;
  }
  prefs.end();
  return false;
}

void guardarCredencialesNVS(const String &ssid, const String &password) {
  prefs.begin("wifiCreds", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", password);
  prefs.end();
}

void borrarCredencialesNVS() {
  prefs.begin("wifiCreds", false);
  prefs.clear(); 
  prefs.end();
}

String leerLineaPorSerial(const char *prompt) {
  Serial.print(prompt);
  String res = "";
  while (!Serial.available()) {
    delay(10);
  }
  res = Serial.readStringUntil('\n');
  res.trim();
  return res;
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

void revisarRiego() {
  String url = String(BASE_URL) + ENDPOINTS[CHECK_WATERING];

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Reconectando...");
    if (!conectarWiFi()) {
      Serial.println("No se pudo reconectar. Abortando revisión de riego.");
      return;
    }
  }

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String mac = WiFi.macAddress();
  String body = String("{\"mac_address\":\"") + mac + "\"}";

  Serial.print("CHECK_WATERING POST: ");
  Serial.println(body);

  int code = http.POST(body);
  if (code <= 0) {
    Serial.printf("Error HTTP en CHECK_WATERING: %s\n", http.errorToString(code).c_str());
    http.end();
    return;
  }

  String payload = http.getString();
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload);
  http.end();

  if (err) {
    Serial.print("JSON inválido: ");
    Serial.println(err.c_str());
    return;
  }

  bool shouldWater = doc["should_water"];          
  int durationMsApi = doc["duration_seconds"].as<int>();
  if (durationMsApi <= 0) durationMsApi = DURACION_RIEGO;  

  Serial.printf("CHECK_WATERING → should_water=%s, durationMs=%d\n", shouldWater?"true":"false", durationMsApi);

  if (shouldWater) {
    regarPlanta(durationMsApi);
  }
}

void regarPlanta(int durationMs) {
  if (bombaEncendida) {
    Serial.println("⚠️  Bomba ya estaba encendida: ignorando nuevo comando");
    return;
  }
  bombaDuracionMs   = (durationMs > 0) ? durationMs : DURACION_RIEGO;
  tiempoInicioRiego = millis();
  bombaEncendida    = true;

  Serial.printf("🟢 Bomba ON (duración %lu ms)\n", bombaDuracionMs);
  digitalWrite(RELAY_PIN, LOW);
}

void controlarBomba() {
  if (bombaEncendida && (millis() - tiempoInicioRiego) >= bombaDuracionMs) {
    digitalWrite(RELAY_PIN, HIGH);
    bombaEncendida = false;
    Serial.println("🔴 Bomba OFF (tiempo cumplido)");

    int durationSecs = bombaDuracionMs / 1000;
    if (durationSecs <= 0) durationSecs = 1;
    enviarAutoWatering(durationSecs);
  }
}

void enviarHumedad(float humedadPct) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Reconectando...");
    if (!conectarWiFi()) {
      Serial.println("No se pudo enviar la lectura (sin WiFi)");
      return;
    }
  }

  String url = String(BASE_URL) + ENDPOINTS[HUMIDITY];
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String mac  = WiFi.macAddress();
  String body = String("{\"mac_address\":\"") + mac + "\",\"humidity\":" + String(humedadPct, 1) + "}";

  Serial.print("POST ");
  Serial.println(body);

  int code = http.POST(body);
  if (code > 0) {
    Serial.printf("HTTP %d\n", code);
    Serial.println(http.getString());
  } else {
    Serial.printf("Error HTTP: %s\n", http.errorToString(code).c_str());
  }

  http.end();
}

void enviarAutoWatering(int durationSeconds) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Reconectando antes de AUTO_WATERING...");
    if (!conectarWiFi()) {
      Serial.println("No se pudo enviar AUTO_WATERING (sin WiFi)");
      return;
    }
  }

  String url = String(BASE_URL) + ENDPOINTS[AUTO_WATERING];
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String mac  = WiFi.macAddress();
  String body = String("{\"mac_address\":\"") + mac + "\",\"duration_seconds\":" + String(durationSeconds) + "}";

  Serial.print("AUTO_WATERING POST: ");
  Serial.println(body);

  int code = http.POST(body);
  if (code > 0) {
    Serial.printf("AUTO_WATERING HTTP %d\n", code);
    Serial.println(http.getString());
  } else {
    Serial.printf("Error HTTP AUTO_WATERING: %s\n", http.errorToString(code).c_str());
  }

  http.end();
}