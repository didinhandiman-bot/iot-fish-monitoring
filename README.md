# IoT Fish Quality Monitoring
Monitoring kualitas air real-time menggunakan ESP32 dengan sensor suhu dan pH meter.

## Struktur Project
```
iot-fish-monitoring/
├── sql/                    # Database schema
│   └── init.sql            # Inisialisasi database
├── src/                    # Backend server
│   ├── server.js           # Express.js + Socket.IO
│   ├── config.js           # Konfigurasi (db, port)
│   └── db.js               # MySQL connection pool
├── dashboard/              # Frontend dashboard
│   └── public/
│       └── index.html      # React-style dashboard (vanilla JS)
├── esp32/                  # Code ESP32
│   └── esp32_sensor.ino    # Sketch kirim data dummy
└── package.json
```

## Quick Start

### 1. Setup Database
```bash
mysql -u root -p < sql/init.sql
```

### 2. Install Dependencies
```bash
npm install
```

### 3. Run Server
```bash
npm start
# or development mode with auto-reload
npm run dev
```

Server akan berjalan di `http://localhost:3000`

### 4. Akses Dashboard
Buka browser: `http://localhost:3000/dashboard/`

### 5. Test dengan Curl (simulasi ESP32)
```bash
curl -X POST http://localhost:3000/api/sensor/data \
  -H "Content-Type: application/json" \
  -d '{"deviceId":"ESP32-DEMO","sensors":{"temperature":28.5,"ph":7.2}}'
```

## Arsitektur
```
ESP32 ──HTTP POST──> Express.js API ──┬──> MySQL (histori)
                                       └──> Socket.IO ──> Browser Dashboard
```

## Endpoints
- **POST** `/api/sensor/data` - Terima data dari ESP32
- **GET** `/api/sensor/latest/:deviceId` - Data terakhir per device
- **GET** `/api/sensor/history/:deviceId` - Histori data
- **GET** `/api/devices` - Daftar semua device
- **WS** `socket.io` - Realtime push ke browser
