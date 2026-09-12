#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ============================================================
// 🔑 KONFIGURASI JARINGAN & SERVER
// ============================================================
const char* WIFI_SSID     = "HW CHANNEL";
const char* WIFI_PASSWORD = "@cc_hw#123";
const char* SERVER_URL    = "https://iot.bppmhkp.online/api/sensor/data";

// ============================================================
// 🔧 PIN & PARAMETER SISTEM
// ============================================================
#define I2C_SDA_PIN       21     // Pin P21 -> SDA LCD
#define I2C_SCL_PIN       22     // Pin R22 -> SCL LCD
#define DS18B20_PIN       4      // Pin P4  -> DATA Sensor Suhu
#define LED_PIN           2      // LED Indikator Board
#define SENSOR_INTERVAL   10000  // Interval kirim data (ms)
#define LCD_ADDR          0x27   // Alamat I2C umum (ganti 0x3F jika gelap)

// ============================================================
// 🏗 OBJEK HARDWARE
// ============================================================
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

unsigned long lastSendTime = 0;
String deviceId = "";

// ============================================================
// 📺 FUNGSI BANTU LCD (Format pas 16 karakter per baris)
// ============================================================
void showLcdLines(const char* line1, const char* line2) {
  char buf1[17], buf2[17];
  snprintf(buf1, sizeof(buf1), "%-16s", line1);
  snprintf(buf2, sizeof(buf2), "%-16s", line2);

  lcd.setCursor(0, 0);
  lcd.print(buf1);
  lcd.setCursor(0, 1);
  lcd.print(buf2);
}

void updateLcdReadings(float temp, float ph, bool isOnline) {
  char line1[17];
  char line2[17];

  // Baris 1: Suhu
  if (temp == -127.00) {
    snprintf(line1, sizeof(line1), "Temp: ERR       ");
  } else {
    snprintf(line1, sizeof(line1), "Temp: %4.1f %cC   ", temp, (char)223);
  }

  // Baris 2: pH dan status jaringan
  snprintf(line2, sizeof(line2), "pH:%-4.2f  %s", ph, isOnline ? "WiFi:OK" : "WiFi:NC");

  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ============================================================
// 📶 MANAJEMEN WIFI
// ============================================================
bool connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;

  showLcdLines("WiFi Connecting", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Terhubung!");
    digitalWrite(LED_PIN, HIGH);
    return true;
  }

  Serial.println("\n[WiFi] Gagal terhubung.");
  digitalWrite(LED_PIN, LOW);
  return false;
}

// ============================================================
// 🌡 BACA SENSOR & DATA SIMULASI
// ============================================================
float readTemperature() {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  if (temp == -127.00) {
    Serial.println("[SENSOR] DS18B20 tidak terdeteksi!");
    return -127.00;
  }
  return temp;
}

float readSimulatedPH() {
  return 6.8 + (random(0, 6) / 100.0);
}

// ============================================================
// 📤 PUSH HTTP POST DATA
// ============================================================
void sendTelemetry(float temp, float ph) {
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(5000);

  HTTPClient http;
  if (!http.begin(client, SERVER_URL)) {
    Serial.println("[HTTP] Gagal menghubungkan ke host");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  // Fallback suhu jika sensor lepas saat kirim
  float reportTemp = (temp == -127.00) ? 27.0 : temp;
  String payload = "{\"deviceId\":\"" + deviceId +
                   "\",\"temperature\":" + String(reportTemp, 1) +
                   ",\"ph\":" + String(ph, 2) + "}";

  Serial.printf("[HTTP] Mengirim: %s\n", payload.c_str());
  int responseCode = http.POST(payload);

  if (responseCode > 0) {
    Serial.printf("[HTTP] Sukses, Kode: %d\n", responseCode);
  } else {
    Serial.printf("[HTTP] Error, Kode: %d\n", responseCode);
  }

  http.end();
}

// ============================================================
// ⚙️ SETUP UTAMA
// ============================================================
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 1. Inisialisasi Bus I2C & LCD dengan proteksi timing
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000); // 100kHz standard-mode (mencegah karakter aneh)
  
  lcd.init();
  lcd.backlight();
  showLcdLines(" Fish Monitoring", "   System Boot  ");
  delay(1500);

  // 2. Inisialisasi DS18B20
  sensors.begin();
  sensors.setResolution(12);

  // 3. Susun Device ID Unik dari MAC
  byte mac[6];
  WiFi.macAddress(mac);
  deviceId = "FISH-" + String(mac[4], HEX) + String(mac[5], HEX);
  deviceId.toUpperCase();

  // 4. Sambungkan ke Jaringan
  connectWiFi();
}

// ============================================================
// 🔁 LOOP UTAMA
// ============================================================
void loop() {
  bool isOnline = (WiFi.status() == WL_CONNECTED);
  digitalWrite(LED_PIN, isOnline ? HIGH : LOW);

  // Reconnect otomatis jika koneksi putus
  if (!isOnline && (millis() - lastSendTime > 5000)) {
    connectWiFi();
    isOnline = (WiFi.status() == WL_CONNECTED);
  }

  // Siklus kirim data berkala
  unsigned long now = millis();
  if (now - lastSendTime >= SENSOR_INTERVAL) {
    lastSendTime = now;

    float currentTemp = readTemperature();
    float currentPH   = readSimulatedPH();

    // Perbarui display layar
    updateLcdReadings(currentTemp, currentPH, isOnline);

    // Kirim data ke API jika WiFi terhubung
    if (isOnline) {
      sendTelemetry(currentTemp, currentPH);
    }
  }

  delay(200);
}
