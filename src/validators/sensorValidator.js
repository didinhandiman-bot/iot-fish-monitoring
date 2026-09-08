class SensorValidator {
  static validatePayload(req, res, next) {
    const { deviceId, timestamp, sensors } = req.body;

    if (!deviceId || typeof deviceId !== 'string') {
      return res.status(400).json({ success: false, message: 'deviceId wajib diisi' });
    }

    if (!sensors || typeof sensors !== 'object') {
      return res.status(400).json({ success: false, message: 'sensors wajib berupa object' });
    }

    if (typeof sensors.temperature !== 'number' || sensors.temperature < -5 || sensors.temperature > 60) {
      return res.status(400).json({ success: false, message: 'temperature harus angka (-5 sampai 60)' });
    }

    if (typeof sensors.ph !== 'number' || sensors.ph < 0 || sensors.ph > 14) {
      return res.status(400).json({ success: false, message: 'ph harus angka (0 sampai 14)' });
    }

    next();
  }
}

module.exports = SensorValidator;
