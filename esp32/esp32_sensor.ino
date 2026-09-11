#include <WiFi.h>              // Library WiFi ESP32
#include <WiFiClientSecure.h>  // 🔒 Client Aman untuk HTTPS
#include <HTTPClient.h>        // Library Kirim Data Web
#include <OneWire.h>           // 🌡️ Protokol komunikasi DS18B20
#include <DallasTemperature.h> // 🌡️ Library sensor suhu DS18B20

// ============================================================
// 🔑 KONFIGURASI JARINGAN ANDA
// ============================================================
const char* WIFI_SSID     = "OPPO A11k";      // Ganti Nama WiFi Anda
const char* WIFI_PASSWORD = "43ed78eb1c43";   // Ganti Password WiFi Anda

// ============================================================
// 📡 SERVER CONFIGURATION - Menggunakan Domain Resmi
// ============================================================
const char* SERVER_URL  = "https://iot.bppmhkp.online/api/sensor/data";

// ============================================================
// 🔧 PIN & KONFIGURASI
// ============================================================
#define SENSOR_INTERVAL   10000    // Kirim data setiap 10 detik
#define LED_PIN           2        // LED bawaan board ESP32
#define DEVICE_ID_PREFIX  "FISH-"
#define DS18B20_PIN       4        // GPIO4 -> DATA pin DS18B20

// ============================================================
// 🏗️ INISIALISASI OBJEK
// ============================================================
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

unsigned long lastSendTime = 0;
bool wifiConnected = false;
bool connectWifiSuccess = false;

// ============================================================
// ⚙️ SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Mulai modul sensor DS18B20
  sensors.begin();
  sensors.setResolution(12); // Presisi maksimal (bisa 9-12 bit)

  Serial.println("\n⚡ [BOOT] Fish Monitoring System Starting...");
  
  // Tunggu sensor siap
  delay(500);

  // Connect WiFi
  connectToWifi();

  if (!connectWifiSuccess) {
    Serial.println("❌ Gagal koneksi WiFi. Restart otomatis...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("✅ Sistem Siap. Mulai kirim data.");
}

// ============================================================
// 🔁 MAIN LOOP
// ============================================================
void loop() {
  // Cek status WiFi
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, LOW);
    if (millis() - lastSendTime > 5000) {
      Serial.println("[WARN] WiFi putus. Mencoba ulang...");
      connectToWifi();
    }
    return;
  }

  // Jika baru connect, nyalakan LED solid
  if (!wifiConnected) {
    wifiConnected = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.printf("[OK] Terhubung ke %s\n", WiFi.SSID().c_str());
  }

  unsigned long now = millis();

  // Kirim data sesuai interval
  if (now - lastSendTime >= SENSOR_INTERVAL) {
    lastSendTime = now;
    
    // Baca dari sensor DS18B20 asli
    float realTemp = getRealTemperature();
    float simulatedPH = getSimulatedPH();

    // Log suhu ke serial monitor
    Serial.printf("[TEMP] %.1f°C | Sending to server...\n", realTemp);

    sendSensorData(realTemp, simulatedPH);
  }

  delay(500); // Sedikit jeda sistem
}

// ============================================================
// 📶 FUNGSI CONNECT WIFI
// ============================================================
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

// ============================================================
// 🌡️ BACA SUHU DARI DS18B20 ASLI
// ============================================================
float getRealTemperature() {
  sensors.requestTemperatures(); // Perintah baca suhu
  
  float temp = sensors.getTempCByIndex(0); // Baca device pertama
  
  // Handle error jika sensor tidak terdeteksi (-127.00)
  if (temp == -127.00) {
    Serial.println("⚠️ Sensor DS18B20 tidak terdeteksi! Pakai fallback.");
    return 27.0; // Fallback value kalau sensor belum nyambung
  }
  
  return temp;
}

// ============================================================
// 📤 PUSH DATA KE SERVER
// ============================================================
void sendSensorData(float temp, float ph) {
  digitalWrite(LED_PIN, HIGH);

  // Gunakan klien aman (Secure Client)
  WiFiClientSecure client;

  // Bypass validasi sertifikat SSL/TLS (self-signed cert)
  client.setInsecure();
  client.setTimeout(5000);

  HTTPClient http;

  if (http.begin(client, SERVER_URL)) {
    http.addHeader("Content-Type", "application/json");

    // Ambil MAC Address Unik sebagai Device ID
    byte mac[6];
    WiFi.macAddress(mac);
    String deviceId = String(DEVICE_ID_PREFIX) + String(mac[4]) + String(mac[5]);

    // Susun JSON Payload
    String jsonString = "{\"deviceId\":\"" + deviceId + 
                        "\",\"temperature\":" + String(temp, 1) + 
                        ",\"ph\":" + String(ph, 2) + "}";

    Serial.printf("[SEND] %s\n", jsonString.c_str());

    int httpResponse = http.POST(jsonString);

    if (httpResponse > 0) {
      Serial.printf("📤 OK (%d)\n", httpResponse);
      digitalWrite(LED_PIN, LOW);
    } else {
      Serial.printf("❌ FAIL HTTP Code: %d\n", httpResponse);
      digitalWrite(LED_PIN, HIGH);
    }

    http.end();
  } else {
    Serial.println("❌ Error Host tidak ditemukan");
  }
}

// ============================================================
// 💧 SIMULASI pH (NANTI DIGANTI SENSOR pH ASLI)
// ============================================================
float getSimulatedPH() {
  return (6.8 + (random(0, 6)) / 100.0);
}
