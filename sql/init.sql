-- ============================================
-- IoT Fish Monitoring - Database Schema
-- ============================================

CREATE DATABASE IF NOT EXISTS fish_monitoring;
USE fish_monitoring;

-- Tabel device (tracking status)
CREATE TABLE IF NOT EXISTS devices (
    id INT AUTO_INCREMENT PRIMARY KEY,
    device_id VARCHAR(50) UNIQUE NOT NULL,
    name VARCHAR(100) DEFAULT NULL COMMENT 'Nama deskriptif device',
    status ENUM('online', 'offline') DEFAULT 'offline',
    last_seen TIMESTAMP NULL DEFAULT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_device_id (device_id),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabel sensor logs (histori lengkap)
CREATE TABLE IF NOT EXISTS sensor_logs (
    id INT AUTO_INCREMENT PRIMARY KEY,
    device_id VARCHAR(50) NOT NULL,
    temperature DECIMAL(5,2) NOT NULL COMMENT 'Suhu (°C)',
    ph DECIMAL(4,2) NOT NULL COMMENT 'pH cairan',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_device (device_id),
    INDEX idx_created (created_at),
    INDEX idx_date (DATE(created_at))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
