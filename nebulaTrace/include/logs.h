#ifndef LOGS_H
#define LOGS_H

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <ctime>
#include <iomanip> // For std::put_time
#include <sys/stat.h> // For stat and mkdir
#include <sys/types.h> // For POSIX types
#include <stdexcept> // For std::runtime_error

class Logs {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    static Logs* instance;
    static std::mutex instanceMutex;

    // Private constructor for singleton
    Logs(const std::string& filePath);

public:
    // Deleted copy constructor and assignment operator to prevent copying
    Logs(const Logs&) = delete;
    Logs& operator=(const Logs&) = delete;

    // Method to ensure the log directory exists with proper permissions
    static void ensureLogDirectory(const std::string& directoryPath);

    // Destructor to close the log file
    ~Logs();

    // Method to get the singleton instance
    static Logs* getInstance(const std::string& filePath);

    // Method to write a log entry
    void writeLog(const std::string& logEntry);

    // Method to write an error log entry in red color
    void writeError(const std::string& errorEntry);

    // Method to write a warning log entry in yellow color
    void writeWarning(const std::string& warningEntry);
    void writeInfo(const std::string& infoEntry);
};

#endif // LOGS_H