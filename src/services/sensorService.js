const SensorLogRepository = require('../repositories/sensorLogRepository');
const DeviceRepository = require('../repositories/deviceRepository');

class SensorService {
  constructor() {
    this.logRepo = new SensorLogRepository();
    this.deviceRepo = new DeviceRepository();
    this.inMemoryData = {}; // Cache realtime untuk performa
  }

  async handleSensorData(payload) {
    const { deviceId, sensors } = payload;
    const { temperature, ph } = sensors;
    const now = new Date();

    // 1. Simpan device (upsert)
    await this.deviceRepo.upsert(deviceId, `Sensor ${deviceId}`);

    // 2. Cache di memory untuk realtime instant (tanpa nunggu DB)
    this.inMemoryData[deviceId] = {
      deviceId,
      temperature,
      ph,
      timestamp: now.toISOString(),
      updatedAt: now
    };

    // 3. Simpan ke database (async, non-blocking)
    this.logRepo.create({ deviceId, temperature, ph })
      .catch(err => console.error('[SensorService] DB write failed:', err.message));

    return this.inMemoryData[deviceId];
  }

  getLatest(deviceId) {
    return this.inMemoryData[deviceId] || null;
  }

  getAllLatest() {
    return Object.values(this.inMemoryData);
  }

  async getHistory(deviceId, limit = 100) {
    return this.logRepo.findByDevice(deviceId, limit);
  }

  async getDevices() {
    return this.deviceRepo.findAll();
  }

  // Cleanup device yang >5 menit tidak kirim data
  async cleanupOfflineDevices() {
    await this.deviceRepo.markOffline();
  }
}

module.exports = new SensorService(); // Singleton instance
