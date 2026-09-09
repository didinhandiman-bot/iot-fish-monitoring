/**
 * Test Script - Simulasi Data Sensor (Ganti ESP32 Sementara)
 * 
 * Jalankan ini setelah server start:
 *   node test-sensor-dummy.js
 * 
 * Ini akan kirim dummy data ke API setiap 2 detik
 */

const fetch = (...args) => import('node-fetch').then(({default: fetch}) => fetch(...args));

const SERVER_URL = process.env.SERVER_URL || 'http://localhost:3000';
const INTERVAL = parseInt(process.env.INTERVAL_MS) || 2000;

const devices = [
    { id: 'ESP32-001', temp: 27.5, ph: 7.2 },
    { id: 'ESP32-002', temp: 26.8, ph: 7.0 },
    { id: 'ESP32-003', temp: 28.2, ph: 6.9 }
];

let loopCounts = {};
devices.forEach(d => loopCounts[d.id] = 0);

function simulateSensor(device) {
    loopCounts[device.id]++;
    
    // Noise kecil + variasi realistis
    device.temp += (Math.random() - 0.5) * 0.3;
    device.temp = Math.max(24, Math.min(32, device.temp));
    
    device.ph += (Math.random() - 0.5) * 0.02;
    device.ph = Math.max(6.0, Math.min(9.0, device.ph));
    
    return {
        deviceId: device.id,
        timestamp: Date.now(),
        sensors: {
            temperature: parseFloat(device.temp.toFixed(1)),
            ph: parseFloat(device.ph.toFixed(2))
        }
    };
}

async function sendData(payload) {
    try {
        const res = await fetch(`${SERVER_URL}/api/v1/sensors`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        
        const result = await res.json();
        if (result.success) {
            console.log(`✅ ${payload.deviceId}: ${payload.sensors.temperature}°C | pH ${payload.sensors.ph}`);
        } else {
            console.log(`❌ ${payload.deviceId}: ${result.message}`);
        }
    } catch (err) {
        console.log(`⚠️  ${payload.deviceId}: Connection error - ${err.message}`);
    }
}

async function main() {
    console.log('\n🐟 IoT FISH MONITORING - Dummy Sensor Simulator');
    console.log(`📡 Sending to: ${SERVER_URL}/api/v1/sensors`);
    console.log(`📊 Devices: ${devices.map(d => d.id).join(', ')}`);
    console.log('💡 Ctrl+C untuk berhenti\n');
    
    // Kirim data pertama
    for (const device of devices) {
        const payload = simulateSensor(device);
        await sendData(payload);
    }
    
    // Kirim interval
    setInterval(async () => {
        for (const device of devices) {
            const payload = simulateSensor(device);
            await sendData(payload);
        }
    }, INTERVAL);
}

main().catch(console.error);
