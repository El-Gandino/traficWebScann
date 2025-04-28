#ifndef STREAM_H
#define STREAM_H

#include <string>
#include <mutex>

class Stream {
private:
    static Stream* instance; // Singleton instance
    static std::mutex instanceMutex; // Mutex for thread safety

    // Private constructor to prevent direct instantiation
    Stream() : logFilePath("./log/nebulaTrace.log") {}

public:
    std::string logFilePath; // Path to the log file

    // Deleted copy constructor and assignment operator to prevent copying
    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

    // Method to get the singleton instance
    static Stream* getInstance() {
        std::lock_guard<std::mutex> lock(instanceMutex);
        if (instance == nullptr) {
            instance = new Stream();
        }
        return instance;
    }
};

#endif // STREAM_H