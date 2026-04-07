#pragma once

#include "definitions.h"

class CommandHandler {
public:
    static CommandHandler& getInstance();

    void init();
    void poll();

    CommandHandler(const CommandHandler&) = delete;
    CommandHandler& operator=(const CommandHandler&) = delete;

private:
    CommandHandler();
    ~CommandHandler() = default;

    static void rxCallback(uintptr_t context);
    void processCommand();

    volatile bool rxDone_;
    bool initialized_;
    uint8_t rxByte_;
    char cmdBuf_[64];
    uint8_t cmdLen_;
};
