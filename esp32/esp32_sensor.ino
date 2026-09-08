/* ============================================
 * IoT Fish Monitoring - ESP32 Sensor Dummy
 * Kirim data fake ke API setiap 1 detik
 * 
 * Setup:
 * 1. Install library ArduinoJson via Library Manager
 * 2. Ganti WIFI_SSID, WIFI_PASSWORD
 * 3. Ganti SERVER_IP dengan IP server backend kamu
 * 4. Upload ke ESP32
 * ============================================ */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ============================================
// KONFIGURASI WiFi (GANTI DENGAN DATA KAMU)
// ============================================
const char* WIFI_SSID = "NAMA_WIFI_KAMU";
const char* WIFI_PASS = "PASSWORD_WIFI_KAMU";

// ============================================
// KONFIGURASI SERVER (GANTI DENGAN IP SERVER)
// ============================================
const char* SERVER_HOST = "192.168.1.100"; // <- ganti ini
const int SERVER_PORT = 3000;

// ============================================
// KONFIGURASI DEVICE
// ============================================
#define DEVICE_ID "ESP32-001"

// ============================================
// GLOBAL VARIABELS
// ============================================
float simTemp = 27.5;  // Suhu awal simulasi
float simPH = 7.2;     // pH awal simulasi
unsigned long lastTime = 0;
unsigned long sendInterval = 1000; // 1 detik
int loopCount = 0;

// ============================================
// FUNGSI SIMULASI SENSOR
// ============================================
// Simulasi suhu berubah naik-turun kayak air ikan beneran
float simulateTemp() {
  static float timeVar = 0;
  timeVar += 0.02;
  
  // Noise kecil + variasi sinusoidal (sangat realistis)
  float noise = (random(-10, 11) / 100.0);
  simTemp = simTemp + noise;
  
  // Clamp antara 24-32°C (range air ikan normal)
  if (simTemp < 24.0) simTemp = 24.0;
  if (simTemp > 32.0) simTemp = 32.0;
  
  return simTemp;
}

// Simulasi pH berubah lambat (pH stabil kecuali ada perubahan besar)
float simulatePH() {
  static float timeVar = 0;
  timeVar += 0.005;
  
  // Perubahan sangat pelan + noise super kecil
  float change = (random(-1, 2) / 1000.0);
  simPH = simPH + change;
  
  // Clamp antara 6.0-9.0 (range aman untuk ikan)
  if (simPH < 6.0) simPH = 6.0;
  if (simPH > 9.0) simPH = 9.0;
  
  return simPH;
}

// ============================================
// SETUP WiFi & Serial Monitor
// ============================================
void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n==========================================");
  Serial.println("🐟 IoT FISH MONITORING SYSTEM");
  Serial.println("   Device: " DEVICE_ID);
  Serial.println("==========================================\n");
  
  // Connect WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  Serial.print("📡 Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Connected!");
    Serial.print("🌐 Local IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("🔗 Server: ");
    Serial.print(SERVER_HOST);
    Serial.print(":");
    Serial.println(SERVER_PORT);
    Serial.println("==========================================\n");
  } else {
    Serial.println("❌ WiFi FAILED!");
    Serial.println("Check SSID/Password dan restart ESP32");
    while (1) delay(1000); // Stop kalau gagal connect WiFi
  }
}

// ============================================
// KIRIM DATA KE SERVER
// ============================================
void sendData(float temperature, float ph) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi disconnected!");
    return;
  }
  
  HTTPClient http;
  
  // Build URL endpoint
  String url = "http://";
  url += SERVER_HOST;
  url += ":";
  url += SERVER_PORT;
  url += "/api/sensor/data";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  // Build JSON body
  // Format: {"deviceId":"ESP32-001","timestamp":12345,"sensors":{"temperature":28.5,"ph":7.2}}
  StaticJsonDocument<384> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["timestamp"] = millis();
  
  JsonObject sensors = doc.createNestedObject("sensors");
  sensors["temperature"] = temperature;
  sensors["ph"] = ph;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send POST request
  Serial.printf("[SEND] %s -> %.1f°C | pH %.2f\n", 
                DEVICE_ID, temperature, ph);
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    Serial.printf("✅ [OK] HTTP %d | Response: %s\n", 
                  httpResponseCode, http.getString().c_str());
  } else {
    Serial.printf("❌ [FAIL] HTTP %d\n", httpResponseCode);
  }
  
  http.end();
}

// ============================================
// MAIN LOOP
// ============================================
void loop() {
  unsigned long currentTime = millis();
  
  // Setiap 1 detik, kirim data
  if (currentTime - lastTime >= sendInterval) {
    lastTime = currentTime;
    loopCount++;
    
    // Generate sensor values (dummy tapi realistis)
    float temp = simulateTemp();
    float ph = simulatePH();
    
    // Kirim ke backend
    sendData(temp, ph);
    
    // Print status ke Serial Monitor
    Serial.printf("💧 Tank Status: %.1f°C | pH %.2f\n\n", temp, ph);
  }
  
  // Delay kecil biar ESP32 tidak overload
  delay(10);
}
