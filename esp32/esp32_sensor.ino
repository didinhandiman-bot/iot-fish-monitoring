#include <WiFi.h>             // Library WiFi ESP32
#include <WiFiClientSecure.h> // 👈 TAMBAHKAN BARIS INI
#include <HTTPClient.h>       // Library Kirim Data Web

// 🔑 KONFIGURASI JARINGAN ANDA
const char* WIFI_SSID     = "OPPO A11k";      // Ganti Nama WiFi Anda
const char* WIFI_PASSWORD = "43ed78eb1c43";   // Ganti Password WiFi Anda

// SERVER CONFIGURATION - Menggunakan Domain Resmi
const char* SERVER_URL  = "https://iot.bppmhkp.online/api/sensor/data";

#define SENSOR_INTERVAL   10000   // Kirim data setiap 10 detik                                                                                 
#define LED_PIN           2       // LED bawaan board ESP32
#define DEVICE_ID_PREFIX  "FISH-"

unsigned long lastSendTime = 0;
bool wifiConnected = false;                                                                                                                     
bool connectWifiSuccess = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);                                                                                                                       
                         
  Serial.println("\n⚡️ [BOOT] Fish Monitoring System Starting...");
  connectToWifi();

  if (!connectWifiSuccess) {
    Serial.println("❌ Gagal koneksi WiFi. Restart otomatis...");
    delay(5000);
    ESP.restart();
  }                                                                                                                                              
                         
  Serial.println("✅ Sistem Siap. Mulai kirim data.");
}

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
    float simulatedTemp = getSimulatedTemp();
    float simulatedPH   = getSimulatedPH();
    sendSensorData(simulatedTemp, simulatedPH);
  }

  delay(500); // Sedikit jeda sistem                                                                                                            
}                

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

/**                                                                                                                                              
 * Fungsi Push Data ke Domain dengan Sertifikat Aman
 */
void sendSensorData(float temp, float ph) {
  digitalWrite(LED_PIN, HIGH);

  // Gunakan klien aman (Secure Client)
  WiFiClientSecure client;

  // Bypass validasi sertifikat SSL/TLS
  client.setInsecure();

  client.setTimeout(5000);
  HTTPClient http;

  // Hubungkan ke server domain
  if (http.begin(client, SERVER_URL)) {
    http.addHeader("Content-Type", "application/json");
    
    // Ambil MAC Address Unik
    byte mac[6];
    WiFi.macAddress(mac);
                                                                                                                                                
    String deviceId = String(DEVICE_ID_PREFIX) + String(mac[4]) + String(mac[5]);
    
    // Susun JSON Payload
    String jsonString("{\"deviceId\":\"" + deviceId + "\",\"temperature\":" + String(temp, 1) + ",\"ph\":" + String(ph, 2) + "}");
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

float getSimulatedTemp() {
  return (25.0 + (random(0, 50)) / 10.0);
}

float getSimulatedPH() {
  return (6.8 + (random(0, 6)) / 100.0);
}
