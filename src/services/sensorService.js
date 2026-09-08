const mysql = require('mysql2/promise');
const env = require('../config/env');

let dbPool;

async function getDbPool() {
  if (!dbPool) {
    dbPool = mysql.createPool({
      host: process.env.DB_HOST || '192.168.9.151',
      user: process.env.DB_USER || 'iot_user',
      password: process.env.DB_PASS || 'IotSecurePass@123',
      database: process.env.DB_NAME || 'fish_monitoring',
      waitForConnections: true,
      connectionLimit: 10,
      queueLimit: 0,
      timezone: '+07:00'
    });
  }
  return dbPool;
}

const sensorService = {
  inMemoryData: {}
};

sensorService.handleSensorData = async function(payload) {
  const { deviceId, sensors } = payload;
  const { temperature, ph } = sensors;
  const now = new Date();
  
  const pool = await getDbPool();
  
  // 1. Upsert device
  await pool.execute(
    `INSERT INTO devices (device_id, name, status, last_seen) 
     VALUES (?, ?, 'online', NOW())
     ON DUPLICATE KEY UPDATE status='online', last_seen=NOW()`,
    [deviceId, `Sensor ${deviceId}`]
  );
  
  // 2. Cache di memory untuk realtime instant
  this.inMemoryData[deviceId] = {
    deviceId,
    temperature,
    ph,
    timestamp: now.toISOString(),
    updatedAt: now
  };
  
  // 3. Simpan ke database (async)
  pool.execute(
    'INSERT INTO sensor_logs (device_id, temperature, ph) VALUES (?, ?, ?)',
    [deviceId, parseFloat(temperature), parseFloat(ph)]
  ).catch(err => console.error('[SensorService] DB write failed:', err.message));
  
  return this.inMemoryData[deviceId];
};

sensorService.getLatest = function(deviceId) {
  return this.inMemoryData[deviceId] || null;
};

sensorService.getAllLatest = function() {
  return Object.values(this.inMemoryData);
};

sensorService.getHistory = async function(deviceId, limit = 100) {
  const pool = await getDbPool();
  const [rows] = await pool.execute(
    'SELECT * FROM sensor_logs WHERE device_id = ? ORDER BY created_at DESC LIMIT ?',
    [deviceId, parseInt(limit)]
  );
  return rows;
};

sensorService.getDevices = async function() {
  const pool = await getDbPool();
  const [rows] = await pool.execute('SELECT * FROM devices ORDER BY last_seen DESC');
  return rows;
};

sensorService.cleanupOfflineDevices = async function() {
  const pool = await getDbPool();
  await pool.execute(
    "UPDATE devices SET status='offline' WHERE status='online' AND (last_seen < DATE_SUB(NOW(), INTERVAL 5 MINUTE) OR last_seen IS NULL)"
  );
};

module.exports = sensorService;
