const mysql = require('mysql2/promise');

let pool;

function getPool() {
  if (!pool) {
    pool = mysql.createPool({
      host: process.env.DB_HOST || '192.168.9.151',
      user: process.env.DB_USER || 'iot_user',
      password: process.env.DB_PASS || 'IotSecurePass@123',
      database: process.env.DB_NAME || 'fish_monitoring',
      waitForConnections: true,
      connectionLimit: 10,
      queueLimit: 0,
      timezone: '+07:00'
    });
  }
  return pool;
}

module.exports = { getPool };
