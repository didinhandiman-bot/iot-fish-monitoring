const sensorService = require('../services/sensorService');

class SensorController {
  // POST /api/v1/sensors - Terima data dari ESP32
  async createSensorData(req, res) {
    try {
      const body = req.body;
      
      // Normalisasi payload: support dua format (firmware lama & baru)
      let temperature, ph;
      if (body.sensors && typeof body.sensors === 'object') {
        // Format baru: { deviceId, sensors: { temperature, ph } }
        temperature = parseFloat(body.sensors.temperature);
        ph = parseFloat(body.sensors.ph);
      } else {
        // Format lama: { deviceId, temperature, ph }
        temperature = parseFloat(body.temperature);
        ph = parseFloat(body.ph);
      }

      const payload = {
        deviceId: body.deviceId,
        timestamp: body.timestamp || Date.now(),
        sensors: {
          temperature: temperature,
          ph: ph
        }
      };

      const result = await sensorService.handleSensorData(payload);
      
      return res.status(200).json({
        success: true,
        message: 'Sensor data berhasil disimpan',
        data: result
      });
    } catch (error) {
      console.error('[SensorController] Error:', error);
      return res.status(500).json({
        success: false,
        message: 'Gagal memproses data sensor'
      });
    }
  }

  // GET /api/v1/sensors/latest/:deviceId - Data terakhir per device
  async getLatest(req, res) {
    try {
      const { deviceId } = req.params;
      const data = sensorService.getLatest(deviceId);

      if (!data) {
        return res.status(404).json({
          success: false,
          message: `Device ${deviceId} tidak ditemukan atau belum ada data`
        });
      }

      return res.status(200).json({
        success: true,
        data
      });
    } catch (error) {
      console.error('[SensorController] Error:', error);
      return res.status(500).json({
        success: false,
        message: 'Gagal mengambil data terakhir'
      });
    }
  }

  // GET /api/v1/sensors/history/:deviceId - Histori data
  async getHistory(req, res) {
    try {
      const { deviceId } = req.params;
      const limit = parseInt(req.query.limit) || 100;
      const history = await sensorService.getHistory(deviceId, limit);

      return res.status(200).json({
        success: true,
        count: history.length,
        data: history
      });
    } catch (error) {
      console.error('[SensorController] Error:', error);
      return res.status(500).json({
        success: false,
        message: 'Gagal mengambil histori data'
      });
    }
  }

  // GET /api/v1/sensors/all - Semua data terakhir
  async getAllLatest(req, res) {
    try {
      const data = sensorService.getAllLatest();

      return res.status(200).json({
        success: true,
        count: data.length,
        data
      });
    } catch (error) {
      console.error('[SensorController] Error:', error);
      return res.status(500).json({
        success: false,
        message: 'Gagal mengambil data'
      });
    }
  }
}

module.exports = new SensorController(); // Singleton instance
