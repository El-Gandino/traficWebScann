#include "../include/stream.h"

// Initialize static members
Stream* Stream::instance = nullptr;
std::mutex Stream::instanceMutex;