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
#include "lanScanner.cpp" // Include the header file for ARPTable and ARPEntry classes
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
        try {
            LanScanner lanScanner; // Create an instance of LanScanner
            deviceInfo serverDeviceInfo; // Create an instance of serverDeviceInfo
            serverDeviceInfo.ipv4 = lanScanner.getPersonnalIPAddress("eth0"); // Get device info for eth0
            
            std::cout << "Server IP Address: " << serverDeviceInfo.ipv4 << std::endl;

            std::thread arpBroadcastThread([&lanScanner]() {
                while (true) {
                    lanScanner.sendArpBroadcast("eth0"); // Call sendArpBroadcast every 30 seconds
                    std::this_thread::sleep_for(std::chrono::seconds(30)); // Wait for 30 seconds
                }
            });

            std::thread silentIPScanThread([&lanScanner]() {
                while (true) {
                    lanScanner.scanSilentIPs("eth0"); // Call scanSilentIPs every 10 minutes
                    std::this_thread::sleep_for(std::chrono::minutes(10)); // Wait for 10 minutes
                }
            });
            std::signal(SIGINT, [](int signal) {
                std::cout << "\nInterrupt signal (" << signal << ") received. Stopping Silent IP Scan..." << std::endl;
                std::exit(0); // Exit the program gracefully
            });
            while (true) {}
            arpBroadcastThread.detach(); // Detach the ARP broadcast thread
            silentIPScanThread.detach(); // Detach the silent IP scan thread
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
            return;
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
