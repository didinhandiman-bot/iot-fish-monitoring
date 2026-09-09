class SensorValidator {
  static validatePayload(req, res, next) {
    const body = req.body;
    const { deviceId, timestamp } = body;

    if (!deviceId || typeof deviceId !== 'string') {
      return res.status(400).json({ success: false, message: 'deviceId wajib diisi' });
    }

    // Support dua format payload
    let sensors;
    if (body.sensors && typeof body.sensors === 'object') {
      // Format baru: { deviceId, sensors: { temperature, ph } }
      sensors = body.sensors;
    } else {
      // Format lama: { deviceId, temperature, ph }
      sensors = { temperature: body.temperature, ph: body.ph };
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
