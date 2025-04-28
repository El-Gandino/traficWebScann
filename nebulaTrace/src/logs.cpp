/**
 * @file logs.cpp
 * @brief This file is the LOGs class Writer for the nebulaTrace application.
 * 
 * This file contains the implementation of the LOGs class, which is responsible
 * for writing logs to a file in the nebulaTrace application.
 * 
 * @author Syly LRDM
 * @note File Path: /nebulaTrace/src/main.cpp
*/
#include "../include/logs.h"

// Initialize static members
Logs* Logs::instance = nullptr;
std::mutex Logs::instanceMutex;

// Private constructor for singleton
Logs::Logs(const std::string& filePath) {
    // Ensure the log directory exists
    std::string directoryPath = filePath.substr(0, filePath.find_last_of('/'));
    ensureLogDirectory(directoryPath);

    logFile.open(filePath, std::ios::app);
    if (!logFile.is_open()) {
        throw std::runtime_error("Failed to open log file: " + filePath);
    }
}

// Method to ensure the log directory exists with proper permissions
void Logs::ensureLogDirectory(const std::string& directoryPath) {
    struct stat st;
    if (stat(directoryPath.c_str(), &st) != 0) {
        // Directory does not exist, attempt to create it
        if (mkdir(directoryPath.c_str(), 0755) != 0) {
            throw std::runtime_error("Failed to create directory: " + directoryPath);
        }
    } else if (!S_ISDIR(st.st_mode)) {
        throw std::runtime_error(directoryPath + " exists but is not a directory.");
    }
}

// Destructor to close the log file
Logs::~Logs() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

// Method to get the singleton instance
Logs* Logs::getInstance(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (instance == nullptr) {
        instance = new Logs(filePath);
    }
    return instance;
}

// Method to write a log entry
void Logs::writeLog(const std::string& logEntry) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        logFile << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " : " << logEntry << ";" << std::endl;
    } else {
        throw std::runtime_error("Log file is not open.");
    }
}

// Method to write an error log entry in red color
void Logs::writeError(const std::string& errorEntry) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        logFile << "\033[31m" // ANSI escape code for red text
                << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " [ERROR] : " << errorEntry << ";" 
                << "\033[0m" // Reset color
                << std::endl;
    } else {
        throw std::runtime_error("Log file is not open.");
    }
}

// Method to write a warning log entry in yellow color
void Logs::writeWarning(const std::string& warningEntry) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        logFile << "\033[33m" // ANSI escape code for yellow text
                << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " [WARNING] : " << warningEntry << ";" 
                << "\033[0m" // Reset color
                << std::endl;
    } else {
        throw std::runtime_error("Log file is not open.");
    }
}

// Method to write an informational log entry
void Logs::writeInfo(const std::string& infoEntry) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        logFile << "\033[32m" // ANSI escape code for green text
                << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " [INFO] : " << infoEntry << ";"
                << "\033[0m" // Reset color
                << std::endl;
    } else {
        throw std::runtime_error("Log file is not open.");
    }
}