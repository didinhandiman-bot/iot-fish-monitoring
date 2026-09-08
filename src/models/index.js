class Device {
  constructor(deviceId, name = null) {
    this.deviceId = deviceId;
    this.name = name || `Sensor ${deviceId}`;
    this.status = 'online';
    this.lastSeen = new Date();
  }

  toJSON() {
    return {
      deviceId: this.deviceId,
      name: this.name,
      status: this.status,
      lastSeen: this.lastSeen,
      createdAt: new Date()
    };
  }
}

class SensorLog {
  constructor(deviceId, temperature, ph) {
    this.deviceId = deviceId;
    this.temperature = temperature;
    this.ph = ph;
    this.createdAt = new Date();
  }

  toJSON() {
    return {
      id: null,
      device_id: this.deviceId,
      temperature: this.temperature,
      ph: this.ph,
      created_at: this.createdAt
    };
  }
}

module.exports = { Device, SensorLog };
