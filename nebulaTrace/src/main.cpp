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
#include "lanScanner.cpp"
#include "../include/logs.h"
#include "../include/stream.h"

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

        // Configure the signal handler
        std::signal(SIGINT, signalHandler);

        try {
            Stream* stream = Stream::getInstance(); // Get the singleton instance of Stream
            LanScanner lanScanner; // Create an instance of LanScanner
            deviceInfo serverDeviceInfo; // Create an instance of serverDeviceInfo

            serverDeviceInfo.ipv4 = lanScanner.getPersonnalIPAddress("eth0"); // Get device info for eth0

            // Initialize the logger
            logger = Logs::getInstance(Stream::getInstance()->logFilePath);
            logger->writeLog("NebulaTrace Application Started");

            std::cout << "Server IP Address: " << serverDeviceInfo.ipv4 << std::endl;

            // Start the ARP broadcast thread
            arpBroadcastThread = std::thread([&lanScanner]() {
                while (running) {
                    lanScanner.sendArpBroadcast("eth0"); // Call sendArpBroadcast
                    //for (int i = 0; i < 30 && running; ++i) {
                        for (int i = 0; i < 30 && running; ++i) {
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                        }
                    //}
                }
            });
            // Start the silent IP scan thread
            silentIPScanThread = std::thread([&lanScanner]() {
                while (running) {
                    lanScanner.scanSilentIPs("eth0"); // Call scanSilentIPs
                    for (int i = 0; i < 600 && running; ++i) {
                        std::this_thread::sleep_for(std::chrono::seconds(1)); // Check every second
                    }
                }
            });

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
