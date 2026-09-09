require('dotenv').config();

const http = require('http');
const { Server } = require('socket.io');
const path = require('path');
const app = require('./app');
const env = require('./config/env');
const sensorService = require('./services/sensorService');

// HTTP Server (Nginx handling SSL)
const httpServer = http.createServer(app);

// Socket.IO setup - fix CORS untuk localhost
const io = new Server(httpServer, {
  cors: {
    origin: true, // Allow all origins for development
    methods: ['GET', 'POST'],
    credentials: true
  },
  transports: ['websocket', 'polling']
});

// In-memory cache for realtime (singleton)
const latestData = {};

io.on('connection', (socket) => {
  console.log(`[WS] Client connected: ${socket.id}`);
  
  // Kirim semua data terakhir ke client baru
  socket.emit('devices-list', Object.keys(latestData));
  
  for (const [deviceId, data] of Object.entries(latestData)) {
    socket.emit('sensor-data', data);
  }
  
  socket.on('disconnect', () => {
    console.log(`[WS] Client disconnected: ${socket.id}`);
  });
});

// Override handler untuk broadcast via Socket.IO
const originalHandleSensorData = sensorService.handleSensorData;
sensorService.handleSensorData = async function(payload) {
  const result = await originalHandleSensorData.call(this, payload);
  
  // Cache di memory untuk Socket.IO push
  latestData[payload.deviceId] = result;
  
  console.log(`[WS] Broadcasting: ${payload.deviceId} -> temp:${result.temperature}°C, pH:${result.ph}`);
  
  // Broadcast ke semua browser
  io.emit('sensor-data', result);
  
  return result;
};

// Periodic cleanup offline devices
setInterval(() => {
  sensorService.cleanupOfflineDevices()
    .catch(err => console.error('[Server] Cleanup error:', err.message));
}, 5 * 60 * 1000); // Setiap 5 menit

// Start server
const PORT = env.port || 3000;
httpServer.listen(PORT, '0.0.0.0', () => {
  console.log('\n' + '='.repeat(50));
  console.log(`🚀 IoT Fish Monitoring Server`);
  console.log(`📡 Dashboard: http://localhost:${PORT}/`);
  console.log(`🔌 API Base:   http://localhost:${PORT}/api/`);
  console.log(`📊 Node Env:   ${env.nodeEnv}`);
  console.log('='.repeat(50) + '\n');
});

module.exports = { httpServer, io };
