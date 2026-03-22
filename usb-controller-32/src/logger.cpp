#include "logger.h"
#include <cstdio>
#include <cstring>
#include <cstdarg>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger()
    : txDone_(true)
    , initialized_(false)
{
    buffer_[0] = '\0';
}

void Logger::txCallback(uintptr_t context) {
    auto* flag = reinterpret_cast<volatile bool*>(context);
    *flag = true;
}

void Logger::init() {
    if (initialized_) return;

    UART1_WriteCallbackRegister(txCallback, reinterpret_cast<uintptr_t>(&txDone_));

    initialized_ = true;
}

void Logger::log(const char* message) {
    if (!initialized_) return;

    while (!txDone_) {}

    size_t len = strlen(message);
    if (len > sizeof(buffer_) - 3) {
        len = sizeof(buffer_) - 3;
    }
    memcpy(buffer_, message, len);
    buffer_[len] = '\r';
    buffer_[len + 1] = '\n';
    buffer_[len + 2] = '\0';

    txDone_ = false;
    if (!UART1_Write(buffer_, len + 2)) {
        txDone_ = true;
    }
}

void Logger::log(const char* tag, const char* message) {
    if (!initialized_) return;

    while (!txDone_) {}

    int len = snprintf(buffer_, sizeof(buffer_) - 2, "[%s] %s", tag, message);
    if (len < 0) len = 0;
    if (len > static_cast<int>(sizeof(buffer_) - 3)) len = static_cast<int>(sizeof(buffer_) - 3);
    buffer_[len] = '\r';
    buffer_[len + 1] = '\n';

    txDone_ = false;
    if (!UART1_Write(buffer_, len + 2)) {
        txDone_ = true;
    }
}

void Logger::logf(const char* format, ...) {
    if (!initialized_) return;

    while (!txDone_) {}

    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer_, sizeof(buffer_) - 2, format, args);
    va_end(args);

    if (len < 0) len = 0;
    if (len > static_cast<int>(sizeof(buffer_) - 3)) len = static_cast<int>(sizeof(buffer_) - 3);
    buffer_[len] = '\r';
    buffer_[len + 1] = '\n';

    txDone_ = false;
    if (!UART1_Write(buffer_, len + 2)) {
        txDone_ = true;
    }
}
