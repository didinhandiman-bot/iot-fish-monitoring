require('dotenv').config({override: true});

const http = require('http');

const options = {
  hostname: 'localhost',
  port: 3000,
  path: '/api/v1/sensors',
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
    'Content-Length': Buffer.byteLength(JSON.stringify({
      deviceId: 'OmDidinTestAPI',
      sensors: { temperature: 28.5, ph: 7.2 }
    }))
  }
};

const req = http.request(options, (res) => {
  let data = '';
  res.on('data', chunk => data += chunk);
  res.on('end', () => {
    console.log('STATUS:', res.statusCode);
    console.log('RESPONSE:', data);
  });
});

req.on('error', (e) => {
  console.error('REQUEST ERROR:', e.message);
});

req.write(JSON.stringify({
  deviceId: 'OmDidinTestAPI',
  sensors: { temperature: 28.5, ph: 7.2 }
}));
req.end();

setTimeout(() => {
  console.log('Timeout - no response received');
  process.exit(1);
}, 5000);
