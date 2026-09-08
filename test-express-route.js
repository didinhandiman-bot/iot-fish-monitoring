require('dotenv').config({override: true});

const express = require('express');
const app = express();
app.use(express.json());

// Import routes & middleware
const sensorRoutes = require('./src/routes/sensorRoutes');
console.log('=== Route Config ===');
console.log('Router type:', typeof sensorRoutes);
console.log('Router stack:', sensorRoutes.stack ? sensorRoutes.stack.map(m => ({path: m.route?.path, methods: m.route?.methods})).join(', ') : '(no stack)');

app.use('/api/v1/sensors', sensorRoutes);

const server = app.listen(3999, async () => {
  console.log('Test server on port 3999');
  
  const http = require('http');
  
  const options = {
    hostname: 'localhost',
    port: 3999,
    path: '/api/v1/sensors',
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Content-Length': Buffer.byteLength(JSON.stringify({
        deviceId: 'ExpressTest',
        sensors: { temperature: 28.5, ph: 7.2 }
      }))
    }
  };
  
  const req = http.request(options, (res) => {
    let data = '';
    res.on('data', chunk => data += chunk);
    res.on('end', () => {
      console.log('\n=== EXPRESS ROUTE TEST ===');
      console.log('STATUS:', res.statusCode);
      console.log('BODY:', data);
    });
  });
  
  req.on('error', (e) => {
    console.error('REQUEST ERROR:', e.message);
  });
  
  req.write(JSON.stringify({
    deviceId: 'ExpressTest',
    sensors: { temperature: 28.5, ph: 7.2 }
  }));
  req.end();
  
  setTimeout(() => process.exit(0), 3000);
});
