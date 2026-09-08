const { getPool } = require('../config/database');
const { SensorLog } = require('../models');

class SensorLogRepository {
  constructor(pool) {
    this.pool = pool || getPool();
  }

  async create(sensorData) {
    const log = new SensorLog(
      sensorData.deviceId,
      sensorData.temperature,
      sensorData.ph
    );

    const [result] = await this.pool.execute(
      'INSERT INTO sensor_logs (device_id, temperature, ph, created_at) VALUES (?, ?, ?, ?)',
      [log.deviceId, log.temperature, log.ph, log.createdAt]
    );

    return { ...log.toJSON(), id: result.insertId };
  }

  async findByDevice(deviceId, limit = 100) {
    const [rows] = await this.pool.execute(
      'SELECT * FROM sensor_logs WHERE device_id = ? ORDER BY created_at DESC LIMIT ?',
      [deviceId, parseInt(limit)]
    );
    return rows;
  }

  async findAll(limit = 50) {
    const [rows] = await this.pool.execute(
      'SELECT sl.*, d.name as device_name FROM sensor_logs sl JOIN devices d ON sl.device_id = d.device_id ORDER BY sl.created_at DESC LIMIT ?',
      [parseInt(limit)]
    );
    return rows;
  }
}

module.exports = SensorLogRepository;
