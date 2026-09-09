#include <WiFi.h>             // Library utama WiFi ESP32
#include <WiFiClientSecure.h> // 👈 TAMBAHKAN BARIS INI (Kunci agar HTTPClientSecure terbaca)
#include <HTTPClient.h>       // Library untuk kirim data web

// 🔑 KONFIGURASI JARINGAN ANDA
const char* WIFI_SSID     = "NAMA-WIFI-OM";       // Ganti nama WiFi Om
const char* WIFI_PASSWORD = "PASSWORD-WIFI";      // Ganti password WiFi Om

// Server menggunakan Domain Resmi + HTTPS
const char* SERVER_URL  = "https://iot.bppmhkp.online/api/sensor/data";

#define SENSOR_INTERVAL   10000   // Interval kirim data (ms)
#define LED_PIN           2       // Pin LED bawaan ESP32
#define DEVICE_ID_PREFIX  "FISH-"

unsigned long lastSendTime = 0;
bool wifiConnected = false;
bool connectWifiSuccess = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("\n⚡ [BOOT] Fish Monitoring System Starting...");
  
  // Coba connect ke WiFi
  connectToWifi();
  
  if (!connectWifiSuccess) {
    Serial.println("❌ Gagal koneksi WiFi. Restart otomatis...");
    delay(5000);
    ESP.restart();
  }
  
  Serial.println("✅ Sistem Siap. Mulai kirim data.");
}

void loop() {
  // Cek koneksi WiFi secara berkala
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, LOW);
    if (millis() - lastSendTime > 5000) {
      Serial.println("[WARN] WiFi putus. Mencoba ulang...");
      connectToWifi();
    }
    return; // Stop loop jika belum konek
  }

  // Jika baru saja berhasil konek, nyalakan LED solid
  if (!wifiConnected) {
    wifiConnected = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.printf("[OK] Terhubung ke %s\n", WiFi.SSID().c_str());
  }

  unsigned long now = millis();
  
  // Saatnya kirim data?
  if (now - lastSendTime >= SENSOR_INTERVAL) {
    lastSendTime = now;
    float simulatedTemp = getSimulatedTemp();
    float simulatedPH   = getSimulatedPH();
    
    sendSensorData(simulatedTemp, simulatedPH);
  }
  
  delay(500); // Istirahat sebentar
}

// Fungsi untuk Connect WiFi
void connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    connectWifiSuccess = true;
    Serial.println("\n✅ WiFi Terhubung!");
  } else {
    connectWifiSuccess = false;
  }
}

// Fungsi untuk Push Data ke Server (HTTPS)
void sendSensorData(float temp, float ph) {
  digitalWrite(LED_PIN, HIGH); // Indikator sedang mengirim
  
  // Gunakan WiFiClientSecure (Wajib untuk HTTPS)
  WiFiClientSecure client;
  
  // TIPS WAJIB: Agar aman bypass sertifikat domain
  client.setInsecure(); 
  
  client.setTimeout(5000); 

  HTTPClient http;
  
  if (http.begin(client, SERVER_URL)) {
    http.addHeader("Content-Type", "application/json");

    // Ambil MAC Address unik untuk Device ID
    byte mac[6];
    WiFi.macAddress(mac);
    String deviceId = String(DEVICE_ID_PREFIX) + String(mac[4]) + String(mac[5]);

    // Susun JSON Payload
    String jsonString("{\"deviceId\":\"" + deviceId + "\",\"temperature\":" + String(temp, 1) + ",\"ph\":" + String(ph, 2) + "\"}");

    Serial.printf("[SEND] %s\n", jsonString.c_str());

    int httpResponse = http.POST(jsonString);

    if (httpResponse > 0) {
      Serial.printf("📤 OK (%d)\n", httpResponse);
      digitalWrite(LED_PIN, LOW); // LED mati setelah sukses
    } else {
      Serial.printf("❌ FAIL HTTP Code: %d\n", httpResponse);
      // Jika gagal (-1), biasanya firewall atau DNS
      digitalWrite(LED_PIN, HIGH); 
    }
    
    http.end();
  } else {
    Serial.println("❌ Error Host tidak ditemukan");
  }
}

// Fungsi pembantu angka random realistis
float getSimulatedTemp() {
  return (25.0 + (random(0, 50)) / 10.0);
}

float getSimulatedPH() {
  return (6.8 + (random(0, 6)) / 100.0);
}
