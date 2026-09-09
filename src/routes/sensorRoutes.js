const { Router } = require('express');
const sensorController = require('../controllers/sensorController');
const sensorValidator = require('../validators/sensorValidator');
const asyncHandler = require('../middlewares/asyncHandler');

const router = Router();

// POST /api/v1/sensors - Terima data dari ESP32
router.post('/',
  sensorValidator.validatePayload,
  asyncHandler(sensorController.createSensorData)
);

// Alias route: terima POST dari ESP32 (compatible route)
router.post('/data',
  sensorValidator.validatePayload,
  asyncHandler(sensorController.createSensorData)
);

// Alias route: terima POST dari ESP32 format lama (compatible route)
router.post('/sensor/data',
  sensorValidator.validatePayload,
  asyncHandler(sensorController.createSensorData)
);

// GET /api/v1/sensors/latest/:deviceId - Data terakhir per device
router.get('/latest/:deviceId', asyncHandler(sensorController.getLatest));

// GET /api/v1/sensors/history/:deviceId - Histori
router.get('/history/:deviceId', asyncHandler(sensorController.getHistory));

// GET /api/v1/sensors/all - Semua device terakhir
router.get('/all', asyncHandler(sensorController.getAllLatest));

module.exports = router;
