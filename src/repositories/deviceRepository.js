const { getPool } = require('../config/database');

class DeviceRepository {
  constructor(pool) {
    this.pool = pool || getPool();
  }

  async upsert(deviceId, name) {
    await this.pool.execute(
      `INSERT INTO devices (device_id, name, status, last_seen) 
       VALUES (?, ?, 'online', NOW())
       ON DUPLICATE KEY UPDATE status='online', last_seen=NOW()`,
      [deviceId, name]
    );
    return this.findByDeviceId(deviceId);
  }

  async findByDeviceId(deviceId) {
    const [rows] = await this.pool.execute(
      'SELECT * FROM devices WHERE device_id = ?',
      [deviceId]
    );
    return rows[0] || null;
  }

  async findAll() {
    const [rows] = await this.pool.execute('SELECT * FROM devices ORDER BY last_seen DESC');
    return rows;
  }

  async markOffline() {
    await this.pool.execute(
      "UPDATE devices SET status='offline' WHERE status='online' AND (last_seen < DATE_SUB(NOW(), INTERVAL 5 MINUTE) OR last_seen IS NULL)"
    );
  }
}

module.exports = DeviceRepository;
