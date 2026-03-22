#pragma once

#include "definitions.h"

class MCP23017 {
public:
    enum Register : uint8_t {
        IODIRA   = 0x00,
        IODIRB   = 0x01,
        IPOLA    = 0x02,
        IPOLB    = 0x03,
        GPINTENA = 0x04,
        GPINTENB = 0x05,
        DEFVALA  = 0x06,
        DEFVALB  = 0x07,
        INTCONA  = 0x08,
        INTCONB  = 0x09,
        IOCON    = 0x0A,
        GPPUA    = 0x0C,
        GPPUB    = 0x0D,
        INTFA    = 0x0E,
        INTFB    = 0x0F,
        INTCAPA  = 0x10,
        INTCAPB  = 0x11,
        GPIOA    = 0x12,
        GPIOB    = 0x13,
        OLATA    = 0x14,
        OLATB    = 0x15,
    };

    static constexpr uint16_t DEFAULT_ADDRESS = 0x20;

    explicit MCP23017(uint16_t address = DEFAULT_ADDRESS);

    bool init();

    bool writeRegister(Register reg, uint8_t value);
    bool readRegister(Register reg, uint8_t& value);

    bool readPortA(uint8_t& value);
    bool readPortB(uint8_t& value);

    // Poll interrupt pins and handle any pending MCP23017 interrupts.
    // Returns true if an interrupt was serviced.
    bool handleInterrupts();

private:
    static void i2cCallback(uintptr_t context);
    bool waitForI2C();

    uint16_t address_;
    volatile bool i2cDone_;
    uint8_t txBuf_[2];
    uint8_t rxBuf_[1];
};
