/*
 * Author: Syly LRDM
 * Description: This file serves as the entry point for the NebulaTrace application.
 *              It contains the main function responsible for initializing and starting
 *              the program. The purpose of this application is to handle web traffic
 *              scanning and tracing functionalities.
 * File Path: /nebulaTrace/src/main.cpp
 * Note: 
*/
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <filesystem>
#include "lanScanner.cpp"
#include "sqlDb.cpp"
#include "../include/logs.h"
#include "../include/stream.h"
#include "../include/json.h"
#include <stdexcept>
#include <fstream>

// Declare static/global variables
std::atomic<bool> running(true); // To control the threads
std::thread arpBroadcastThread;
std::thread silentIPScanThread;
Logs* logger = nullptr; // Global pointer to the logger
// Signal handler function
void signalHandler(int signal) {
    if (signal == SIGINT) {
        if (logger) {
            logger->writeLog("NebulaTrace Application Ended");
        }
        running = false; // Signal the threads to stop
    }
}

class Main {
public:
    // Constructor
    Main() {
        std::cout << "NebulaTrace Application Initialized." << std::endl;
    }

    struct deviceInfo {
        std::string macAddress;
        std::string ipv4;
        std::string ipv6;
        std::string hostName;
        std::string dhcpStartDate;
        std::string dhcpEndDate;
    };

    // Method to start the application
    void start() {
        std::cout << "Starting NebulaTrace Application..." << std::endl;
        //Decode ./parametre.JSON
        // Check if the JSON file exists and is readable
        // Configure the signal handler
        std::signal(SIGINT, signalHandler);

        try {
            Stream* stream = Stream::getInstance(); // Get the singleton instance of Stream
            LanScanner lanScanner; // Create an instance of LanScanner
            deviceInfo serverDeviceInfo; // Create an instance of serverDeviceInfo
              // Initialize the database connection
            // Read database configuration from parametre.JSON
            std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
            std::ifstream jsonFile("./parametre.JSON");
            if (!jsonFile.is_open()) {
                std::cerr << "Error: Could not open JSON file." << std::endl;
                return;
            }
            auto jsonData = JsonHandler::readJsonFromFile("./parametre.JSON");
            if (!jsonData) {
                std::cerr << "Error: Failed to parse JSON data." << std::endl;
                return;
            }            
            // Exemple d'accès aux données
            auto root = std::get<JsonObject>(jsonData->value); // Accéder à la valeur de jsonData
            auto nebulaTrace = std::get<JsonObject>(root["nebulaTrace"]->value);
            auto database = std::get<JsonObject>(nebulaTrace["database"]->value);
            std::string dbHost = std::get<std::string>(database["host"]->value);
            std::string dbUser = std::get<std::string>(database["username"]->value);
            std::string dbPassword = std::get<std::string>(database["password"]->value);
            std::string dbName = std::get<std::string>(database["databaseName"]->value);

            serverDeviceInfo.ipv4 = lanScanner.getPersonnalIPAddress("eth0"); // Get device info for eth0
            // Initialize the logger
            logger = Logs::getInstance(Stream::getInstance()->logFilePath);
            logger->writeLog("NebulaTrace Application Started");

            std::cout << "Server IP Address: " << serverDeviceInfo.ipv4 << std::endl;
            // Start the ARP broadcast thread
            arpBroadcastThread = std::thread([&lanScanner]() {
                auto nextBroadcastTime = std::chrono::steady_clock::now();
                while (running) {
                    auto now = std::chrono::steady_clock::now();
                    if (now >= nextBroadcastTime) {
                        lanScanner.sendArpBroadcast("eth0"); // Envoi du broadcast
                        nextBroadcastTime = now + std::chrono::seconds(30); // Planifie le prochain envoi
                    }
                    // Petite pause active pour éviter de consommer 100% CPU
                    std::this_thread::yield(); 
                }
            });
            // Start the silent IP scan thread
            silentIPScanThread = std::thread([&lanScanner]() {
                auto nextScanTime = std::chrono::steady_clock::now();
                while (running) {
                    auto now = std::chrono::steady_clock::now();
                    if (now >= nextScanTime) {
                        auto activeIp = lanScanner.scanSilentIPs("eth0"); // Lancer le scan
                        nextScanTime = now + std::chrono::minutes(10); // Planifie dans 10 minutes
                    }
                    std::this_thread::yield(); // Laisse un peu respirer le CPU
                }
            });
            // get arp table  
            std::vector<LanScanner::DeviceInfo> devices = lanScanner.getDevice();
            // instert into the database
            SqlDb db(dbHost, dbUser, dbPassword, dbName);
            std::vector<std::string> fieldNames = {"mac_Address", "ipv4", "ipv6", "hostName", "dhcpStartDate", "dhcpEndDate"};
            // Insert each device into the database
            for (const auto& device : devices) {
                // Check if the device already exists in the database
                std::string selectQuery = "SELECT * FROM arp_table WHERE mac_Address = '" + device.macAddress + "';";
                std::vector<std::vector<std::string>> existingDevices = db.selectData(selectQuery);
                if (!existingDevices.empty()) {
                    //std::cout << "Device already exists in the database - MAC: " << device.macAddress << std::endl;
                    continue; // Skip inserting this device
                }
                // Insert the device into the database
                std::vector<std::string> values = {device.macAddress, device.ipv4, device.ipv6, device.hostName, device.dhcpStartDate, device.dhcpEndDate};
                db.insertData(fieldNames, values, "arp_table");
                std::cout << "Inserted Device - MAC: " << device.macAddress << ", IP: " << device.ipv4 << std::endl;
            }
            // Keep the main thread running
            while (running) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            // Wait for threads to finish
            if (arpBroadcastThread.joinable()) {
                arpBroadcastThread.join();
            }
            if (silentIPScanThread.joinable()) {
                silentIPScanThread.join();
            }
            jsonFile.close(); // Close the JSON file

        } catch (const std::exception& ex) {
            if (logger) {
                logger->writeError("Exception: " + std::string(ex.what()));
            }
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    }

    // Destructor
    ~Main() {
        std::cout << "NebulaTrace Application Terminated." << std::endl;
    }
};

int main() {
    Main app; // Create an instance of the Main class
    app.start(); // Start the application
    return 0;
}
