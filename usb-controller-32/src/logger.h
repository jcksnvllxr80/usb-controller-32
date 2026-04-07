#pragma once

#include "definitions.h"

class Logger {
public:
    static Logger& getInstance();

    void init();
    void log(const char* message);
    void log(const char* tag, const char* message);
    void logf(const char* format, ...);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger() = default;

    static void txCallback(uintptr_t context);

    volatile bool txDone_;
    bool initialized_;
    char buffer_[256];
};
