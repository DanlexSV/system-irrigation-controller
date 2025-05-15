#include <WiFi.h>
#include <HTTPClient.h>

// ------------- CREDENCIALES DE TU RED -------------
const char* SSID     = "MOVISTAR_PLUS_1ADO";
const char* PASSWORD = "cEoYtcxfamN2Rgv3Bzaj";
// --------------------------------------------------

// ------------- ENDPOINT DONDE GUARDAR EL DATO -----
const char* API_URL  = "https://ejemplo.tu-servidor.com/api/humedad";
// --------------------------------------------------

// Pin analógico y calibración
const int SENSOR_PIN  = 34;
const int ADC_HUMEDO  = 1700;  // <–– Ajusta con tu lectura real en tierra muy húmeda
const int ADC_SECO    = 3200;  // <–– Ajusta con tu lectura real en tierra seca
const unsigned long INTERVALO_MS = 2000;

unsigned long t0 = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  Serial.printf("Conectando a %s ...\n", SSID);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("⚠️  No se pudo conectar. Continuamos sin Wi-Fi.");
  }

  Serial.println("\n==== Información del dispositivo ====");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("\n===== Lectura de humedad (FC-28) =====");
  Serial.println("ADC  | %H");
  Serial.println("-----------------------");
}

void loop() {
  if (millis() - t0 >= INTERVALO_MS) {
    t0 = millis();

    int adc      = analogRead(SENSOR_PIN);
    int humedad  = map(adc, ADC_HUMEDO, ADC_SECO, 100, 0);
    humedad      = constrain(humedad, 0, 100);

    Serial.printf("%4d | %3d %%\n", adc, humedad);

    if (WiFi.status() == WL_CONNECTED) {
      //sendToServer(adc, humedad);
      Serial.print("Mandando datos a la web");
    }
  }
}

void sendToServer(int adc, int humedad) {
  HTTPClient http;
  http.begin(API_URL);        // Si tu servidor usa un certificado propio: http.setInsecure();
  http.addHeader("Content-Type", "application/json");

  String payload = String("{\"adc\":") + adc + ",\"humedad\":" + humedad + "}";
  int code = http.POST(payload);

  Serial.printf("POST → %d\n", code);
  http.end();
}