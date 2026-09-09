const express = require('express');
const path = require('path');
const cors = require('cors');
const helmet = require('helmet');
const sensorRoutes = require('./routes/sensorRoutes');
const { errorHandler, notFound } = require('./middlewares/errorMiddleware');
const env = require('./config/env');

const app = express();

// Security middleware - custom CSP yang allow CDN
app.use(helmet({
  contentSecurityPolicy: {
    directives: {
      defaultSrc: ["'self'"],
      scriptSrc: ["'self'", "'unsafe-inline'", "'unsafe-eval'", 
                   "https://cdn.socket.io", "https://cdnjs.cloudflare.com", 
                   "https://cdn.jsdelivr.net"],
      styleSrc: ["'self'", "'unsafe-inline'", "https://cdnjs.cloudflare.com"],
      imgSrc: ["'self'", "data:", "https:"],
      connectSrc: ["'self'", "ws:", "wss:"],
      fontSrc: ["'self'", "https://cdnjs.cloudflare.com"],
      objectSrc: ["'none'"],
      upgradeInsecureRequests: []
    }
  }
}));

// CORS
app.use(cors({ origin: true, credentials: true }));

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

// Serve static dashboard files TANPA CACHE sama sekali
app.use((req, res, next) => {
  res.set('Cache-Control', 'no-store, no-cache, must-revalidate, proxy-revalidate');
  res.set('Pragma', 'no-cache');
  res.set('Expires', '0');
  res.set('Surrogate-Control', 'no-store');
  next();
});
app.use(express.static(path.join(__dirname, '..', 'public')));

// API routes - dual mounting agar kompatibel dengan firmware lama & baru
app.use('/api/v1/sensors', sensorRoutes);
app.use('/api/sensor', sensorRoutes);

// Health check
app.get('/api/health', (req, res) => {
  res.json({ status: 'ok', uptime: process.uptime(), timestamp: new Date() });
});

// Not found handler
app.use(notFound);

// Error handler
app.use(errorHandler);

module.exports = app;
