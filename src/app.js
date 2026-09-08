const express = require('express');
const path = require('path');
const cors = require('cors');
const helmet = require('helmet');
const sensorRoutes = require('./routes/sensorRoutes');
const { errorHandler, notFound } = require('./middlewares/errorMiddleware');
const env = require('./config/env');

const app = express();

// Security middleware
app.use(helmet());

// CORS
app.use(cors({ origin: env.allowedOrigins }));

// Body parser
app.use(express.json({ limit: '10kb' }));
app.use(express.urlencoded({ extended: true }));

// Request logger (development)
if (env.nodeEnv === 'development') {
  app.use((req, res, next) => {
    console.log(`[${new Date().toISOString()}] ${req.method} ${req.originalUrl}`);
    next();
  });
}

// Serve static dashboard files
app.use(express.static(path.join(__dirname, '..', 'public')));

// API routes
app.use('/api/v1/sensors', sensorRoutes);

// Health check
app.get('/api/health', (req, res) => {
  res.json({ status: 'ok', uptime: process.uptime(), timestamp: new Date() });
});

// Not found handler
app.use(notFound);

// Error handler
app.use(errorHandler);

module.exports = app;
