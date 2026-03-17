#include "logger.h"
#include <cstdio>
#include <cstring>
#include <cstdarg>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger()
    : txComplete_(nullptr)
    , mutex_(nullptr)
    , initialized_(false)
{
    buffer_[0] = '\0';
}

void Logger::txCallback(uintptr_t context) {
    auto sem = reinterpret_cast<SemaphoreHandle_t>(context);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken);
}

void Logger::init() {
    if (initialized_) return;

    txComplete_ = xSemaphoreCreateBinary();
    mutex_ = xSemaphoreCreateMutex();

    UART1_WriteCallbackRegister(txCallback, reinterpret_cast<uintptr_t>(txComplete_));

    initialized_ = true;
}

void Logger::log(const char* message) {
    if (!initialized_) return;

    xSemaphoreTake(mutex_, portMAX_DELAY);

    size_t len = strlen(message);
    if (len > sizeof(buffer_) - 3) {
        len = sizeof(buffer_) - 3;
    }
    memcpy(buffer_, message, len);
    buffer_[len] = '\r';
    buffer_[len + 1] = '\n';

    UART1_Write(buffer_, len + 2);
    xSemaphoreTake(txComplete_, portMAX_DELAY);

    xSemaphoreGive(mutex_);
}

void Logger::log(const char* tag, const char* message) {
    if (!initialized_) return;

    xSemaphoreTake(mutex_, portMAX_DELAY);

    int len = snprintf(buffer_, sizeof(buffer_) - 2, "[%s] %s", tag, message);
    if (len < 0) len = 0;
    if (len > static_cast<int>(sizeof(buffer_) - 3)) len = static_cast<int>(sizeof(buffer_) - 3);
    buffer_[len] = '\r';
    buffer_[len + 1] = '\n';

    UART1_Write(buffer_, len + 2);
    xSemaphoreTake(txComplete_, portMAX_DELAY);

    xSemaphoreGive(mutex_);
}

void Logger::logf(const char* format, ...) {
    if (!initialized_) return;

    xSemaphoreTake(mutex_, portMAX_DELAY);

    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer_, sizeof(buffer_) - 2, format, args);
    va_end(args);

    if (len < 0) len = 0;
    if (len > static_cast<int>(sizeof(buffer_) - 3)) len = static_cast<int>(sizeof(buffer_) - 3);
    buffer_[len] = '\r';
    buffer_[len + 1] = '\n';

    UART1_Write(buffer_, len + 2);
    xSemaphoreTake(txComplete_, portMAX_DELAY);

    xSemaphoreGive(mutex_);
}
