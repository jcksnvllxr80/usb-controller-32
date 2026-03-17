#include "mcp23017.h"
#include "logger.h"

MCP23017::MCP23017(uint16_t address)
    : address_(address)
    , i2cComplete_(nullptr)
    , i2cMutex_(nullptr)
{
    txBuf_[0] = 0;
    txBuf_[1] = 0;
    rxBuf_[0] = 0;
}

void MCP23017::i2cCallback(uintptr_t context) {
    auto sem = reinterpret_cast<SemaphoreHandle_t>(context);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken);
}

bool MCP23017::waitForI2C() {
    if (xSemaphoreTake(i2cComplete_, pdMS_TO_TICKS(100)) != pdTRUE) {
        Logger::getInstance().log("MCP23017", "I2C timeout");
        return false;
    }
    if (I2C1_ErrorGet() != I2C_ERROR_NONE) {
        Logger::getInstance().log("MCP23017", "I2C error");
        return false;
    }
    return true;
}

bool MCP23017::writeRegister(Register reg, uint8_t value) {
    xSemaphoreTake(i2cMutex_, portMAX_DELAY);

    txBuf_[0] = static_cast<uint8_t>(reg);
    txBuf_[1] = value;

    if (!I2C1_Write(address_, txBuf_, 2)) {
        xSemaphoreGive(i2cMutex_);
        return false;
    }

    bool ok = waitForI2C();
    xSemaphoreGive(i2cMutex_);
    return ok;
}

bool MCP23017::readRegister(Register reg, uint8_t& value) {
    xSemaphoreTake(i2cMutex_, portMAX_DELAY);

    txBuf_[0] = static_cast<uint8_t>(reg);

    if (!I2C1_WriteRead(address_, txBuf_, 1, rxBuf_, 1)) {
        xSemaphoreGive(i2cMutex_);
        return false;
    }

    bool ok = waitForI2C();
    if (ok) {
        value = rxBuf_[0];
    }
    xSemaphoreGive(i2cMutex_);
    return ok;
}

bool MCP23017::readPortA(uint8_t& value) {
    return readRegister(GPIOA, value);
}

bool MCP23017::readPortB(uint8_t& value) {
    return readRegister(GPIOB, value);
}

bool MCP23017::init() {
    i2cComplete_ = xSemaphoreCreateBinary();
    i2cMutex_ = xSemaphoreCreateMutex();

    I2C1_CallbackRegister(i2cCallback, reinterpret_cast<uintptr_t>(i2cComplete_));

    Logger::getInstance().log("MCP23017", "Initializing...");

    // IOCON: BANK=0, MIRROR=0, SEQOP=0, INTPOL=0 (active-low), ODR=0
    if (!writeRegister(IOCON, 0x00)) return false;

    // All pins as inputs
    if (!writeRegister(IODIRA, 0xFF)) return false;
    if (!writeRegister(IODIRB, 0xFF)) return false;

    // Enable internal pull-ups on all pins
    if (!writeRegister(GPPUA, 0xFF)) return false;
    if (!writeRegister(GPPUB, 0xFF)) return false;

    // Non-inverted input polarity
    if (!writeRegister(IPOLA, 0x00)) return false;
    if (!writeRegister(IPOLB, 0x00)) return false;

    // Interrupt-on-change for all pins
    if (!writeRegister(GPINTENA, 0xFF)) return false;
    if (!writeRegister(GPINTENB, 0xFF)) return false;

    // Compare against previous pin value (not DEFVAL)
    if (!writeRegister(INTCONA, 0x00)) return false;
    if (!writeRegister(INTCONB, 0x00)) return false;

    // Read GPIO ports to clear any pending interrupts
    uint8_t dummy;
    (void)readRegister(GPIOA, dummy);
    (void)readRegister(GPIOB, dummy);

    Logger::getInstance().log("MCP23017", "Initialized successfully");
    return true;
}

bool MCP23017::handleInterrupts() {
    bool handled = false;
    auto& logger = Logger::getInstance();

    // INTERRUPT_A is active-low: 0 means port A has a pending interrupt
    if (INTERRUPT_A_Get() == 0) {
        uint8_t flags = 0;
        uint8_t capture = 0;

        if (readRegister(INTFA, flags) && readRegister(INTCAPA, capture)) {
            for (uint8_t i = 0; i < 8; i++) {
                if (flags & (1u << i)) {
                    bool pinState = (capture >> i) & 0x01;
                    logger.logf("[MCP23017] Port A pin %u interrupt - state: %s",
                                (unsigned)i, pinState ? "HIGH" : "LOW");
                }
            }
            handled = true;
        }
    }

    // INTERRUPT_B is active-low: 0 means port B has a pending interrupt
    if (INTERRUPT_B_Get() == 0) {
        uint8_t flags = 0;
        uint8_t capture = 0;

        if (readRegister(INTFB, flags) && readRegister(INTCAPB, capture)) {
            for (uint8_t i = 0; i < 8; i++) {
                if (flags & (1u << i)) {
                    bool pinState = (capture >> i) & 0x01;
                    logger.logf("[MCP23017] Port B pin %u interrupt - state: %s",
                                (unsigned)i, pinState ? "HIGH" : "LOW");
                }
            }
            handled = true;
        }
    }

    return handled;
}
