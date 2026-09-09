/**
 * ESP32 Firmware — IoT Fish Monitoring Sensor Simulator
 * 
 * Deskripsi:
 * - Terhubung ke WiFi (SSID & PASSWORD diatur di bagian bawah)
 * - Push data random suhu (25-30°C) dan pH (6.0-8.0) ke dashboard setiap 10 detik
 * - Auto-reconnect jika WiFi atau server putus
 * - LED onboard (pin 2) berkedip saat connect, menyala solid saat kirim data
 * 
 * Setup di Arduino IDE:
 * 1. Install board ESP32: Tools > Board > ESP32 Dev Module
 * 2. Setting Partition Scheme: Large App
 * 3. Upload ke board
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ============================================================
// 🔧 KONFIGURASI — GANTI DENGAN WIFI ANDA!
// ============================================================
const char* WIFI_SSID     = "KANTOR-BPPMHKP";    // Nama WiFi Anda
const char* WIFI_PASSWORD = "PASSWORD_WIFI";      // Password WiFi Anda

// Server endpoint (pake domain biar lebih stabil)
const char* SERVER_URL   = "https://iot.bppmhkp.online/api/sensor/data";

// Device ID unik untuk tiap ESP32
const String DEVICE_ID   = "ESP32-AQUA-01";

// Interval pengiriman (milidetik) — default 10 detik
const unsigned long SEND_INTERVAL = 10000;

// ============================================================
// 📡 GLOBAL VARIABEL
// ============================================================
WiFiClientSecure client;          // HTTPS client
unsigned long lastSendTime = 0;   // Timer untuk interval kirim data
int ledPin = 2;                   // LED onboard ESP32 (pin GPIO2)
bool wifiConnected = false;       // Status koneksi WiFi

// Random data range
float minTemp = 25.0, maxTemp = 30.0;   // Suhu realistis air ikan
float minPH   = 6.0,  maxPH   = 8.0;    // pH air yang aman untuk ikan

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   🐟 IoT Fish Monitoring ESP32      ║");
  Serial.println("╚══════════════════════════════════════╝\n");
  Serial.print("[INFO] Device ID: ");
  Serial.println(DEVICE_ID);
  Serial.print("[INFO] WiFi SSID: ");
  Serial.println(WIFI_SSID);
  
  // Disable SSL verification sementara karena Nginx menggunakan self-signed di internal
  // atau sertifikat yang mungkin belum trusted oleh ESP32
  client.setInsecure();
  
  connectWiFi();
}

void loop() {
  // Cek status WiFi, reconnect jika putus
  if (!checkWiFi()) {
    connectWiFi();
    return;
  }
  
  unsigned long now = millis();
  
  // Kirim data jika interval sudah tercapai
  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;
    sendData();
  }
  
  // Jangan gunakan delay supaya WiFi tetap stabil
  delay(50);
}

/**
 * Connect ke WiFi dengan auto-reconnect
 */
void connectWiFi() {
  Serial.printf("\n[INFO] Connecting to WiFi: %s\n", WIFI_SSID);
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  const int maxAttempts = 30;  // Maksimal tunggu 30 detik
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Kedipkan LED saat proses connect
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.printf("\n✅ Connected!\n");
    Serial.print("[INFO] IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("[INFO] Signal Strength: ");
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    digitalWrite(ledPin, HIGH);  // LED solid = connected
  } else {
    wifiConnected = false;
    Serial.printf("\n❌ Failed to connect after %d attempts\n", maxAttempts);
    Serial.println("[INFO] Restarting in 5 seconds...");
    digitalWrite(ledPin, LOW);
    delay(5000);
    ESP.restart();
  }
}

/**
 * Cek status koneksi WiFi
 */
bool checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WARN] WiFi disconnected!");
    digitalWrite(ledPin, LOW);
    wifiConnected = false;
    return false;
  }
  return true;
}

/**
 * Generate data random realistis
 */
float generateRandomFloat(float min, float max, int decimals) {
  float value = (random() / (float)RAND_MAX) * (max - min) + min;
  // Round ke jumlah desimal tertentu
  float multiplier = pow(10, decimals);
  return round(value * multiplier) / multiplier;
}

/**
 * Kirim data sensor ke server
 */
void sendData() {
  // Generate data random tapi realistis (dengan variasi bertahap, bukan lompatan acak)
  static float prevTemp = 27.0;
  static float prevPH   = 7.0;
  
  // Variasi kecil dari data terakhir (lebih realistis daripada random murni)
  float tempVariation = (random(-15, 15)) / 100.0;  // +/- 0.15°C
  float phVariation   = (random(-5, 5))  / 100.0;   // +/- 0.05 pH
  
  float temperature = constrain(prevTemp + tempVariation, minTemp, maxTemp);
  float pH          = constrain(prevPH  + phVariation,   minPH,   maxPH);
  
  prevTemp = temperature;
  prevPH   = pH;
  
  // Berikan feedback LED — kedip cepat saat kirim
  digitalWrite(ledPin, HIGH);
  
  // Buat JSON payload
  StaticJsonDocument<256> doc;
  doc["deviceId"]   = DEVICE_ID;
  doc["temperature"] = temperature;
  doc["ph"]         = pH;
  doc["timestamp"]  = getTimeStamp();
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.printf("\n📤 Sending data: %s\n", payload.c_str());
  
  // Reset HTTP client
  HTTPClient https;
  https.setTimeout(5000);  // Timeout 5 detik
  
  if (https.begin(client, SERVER_URL)) {
    https.addHeader("Content-Type", "application/json");
    
    // POST request
    int statusCode = https.POST(payload);
    
    if (statusCode == 200) {
      String response = https.getString();
      Serial.printf("✅ Response: %s\n", response.c_str());
      
      // Update previous values
      prevTemp = temperature;
      prevPH   = pH;
      
      // LED solid sebentar = sukses kirim
      digitalWrite(ledPin, HIGH);
      delay(200);
      digitalWrite(ledPin, LOW);
    } else {
      Serial.printf("❌ Error! Status: %d\n", statusCode);
      Serial.printf("Response: %s\n", https.getString().c_str());
      
      // LED kedip lambat = error
      for (int i = 0; i < 3; i++) {
        digitalWrite(ledPin, HIGH);
        delay(150);
        digitalWrite(ledPin, LOW);
        delay(150);
      }
    }
    
    https.end();
  } else {
    Serial.printf("⚠️ Unable to connect to HTTPS server!\n");
  }
  
  // Balikin LED ke status connected (solid)
  if (wifiConnected) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}

/**
 * Ambil timestamp UTC format ISO 8601
 * Menggunakan NTP pool agar akurat
 */
String getTimeStamp() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // Timezone offset 0, no DST
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "1970-01-01T00:00:00Z";
  }
  
  char timeStr[32];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(timeStr);
}
