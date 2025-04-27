-- Créer la base de données
CREATE DATABASE IF NOT EXISTS LANMonitor;
-- Utiliser la base
USE LANMonitor;
-- Créer une seule table ARP avec IPv4 et IPv6
CREATE TABLE IF NOT EXISTS arp_table (
    id INT AUTO_INCREMENT PRIMARY KEY,
    mac_address VARCHAR(17) NOT NULL,          -- MAC format : AA:BB:CC:DD:EE:FF
    ipv4_address VARCHAR(15) DEFAULT NULL,      -- IPv4 optionnel
    ipv6_address VARCHAR(45) DEFAULT NULL,      -- IPv6 optionnel
    start_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,  -- début attribution
    end_time DATETIME NULL,                     -- fin attribution
    UNIQUE KEY unique_mac_ips (mac_address, ipv6_address, start_time)
);