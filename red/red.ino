#include <WiFi.h>
#include <HTTPClient.h>

const char* SSID     = "Galaxy S24 Ultra E943";
const char* PASSWORD = "hola12345";
const char* API_URL  = "https://daniel.fernandosilva.es/api";

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  delay(100);

  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("✅ Conectado al WiFi.");
  Serial.print("IP Local: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    Serial.println("🌐 Haciendo petición GET a la API...");

    http.begin(API_URL);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("📥 Código de respuesta: %d\n", httpCode);
      String payload = http.getString();
      Serial.println("📄 Respuesta:");
      Serial.println(payload);
    } else {
      Serial.printf("❌ Error en la petición HTTP: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.println("⚠️ No conectado a WiFi");
  }

  delay(10000);
}