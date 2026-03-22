#include "definitions.h"
#include "logger.h"
#include "mcp23017.h"

static MCP23017 mcp;

static volatile bool btn1Changed = false;
static volatile bool btn1Pressed = false;
static volatile bool btn2Changed = false;
static volatile bool btn2Pressed = false;
static volatile bool mcpIntAFired = false;
static volatile bool mcpIntBFired = false;

static void pinCallback(GPIO_PIN pin, uintptr_t context) {
    (void)context;
    if (pin == BUTTON_1_PIN) {
        btn1Pressed = (BUTTON_1_Get() == 0);
        if (btn1Pressed) { GREEN_LED_Set(); } else { GREEN_LED_Clear(); }
        btn1Changed = true;
    } else if (pin == BUTTON_2_PIN) {
        btn2Pressed = (BUTTON_2_Get() == 0);
        if (btn2Pressed) { RED_LED_Set(); } else { RED_LED_Clear(); }
        btn2Changed = true;
    } else if (pin == INTERRUPT_A_PIN) {
        mcpIntAFired = true;
    } else if (pin == INTERRUPT_B_PIN) {
        mcpIntBFired = true;
    }
}

extern "C" void APP_Initialize(void) {
    Logger::getInstance().init();
    Logger::getInstance().log("APP", "System starting...");

    GPIO_PinInterruptCallbackRegister(BUTTON_1_PIN,    pinCallback, 0);
    GPIO_PinInterruptCallbackRegister(BUTTON_2_PIN,    pinCallback, 0);
    GPIO_PinInterruptCallbackRegister(INTERRUPT_A_PIN, pinCallback, 0);
    GPIO_PinInterruptCallbackRegister(INTERRUPT_B_PIN, pinCallback, 0);

    BUTTON_1_InterruptEnable();
    BUTTON_2_InterruptEnable();
    INTERRUPT_A_InterruptEnable();
    INTERRUPT_B_InterruptEnable();

    if (!mcp.init()) {
        Logger::getInstance().log("APP", "MCP23017 init failed");
    }
}

extern "C" void APP_Tasks(void) {
    if (btn1Changed) {
        btn1Changed = false;
        Logger::getInstance().logf("[BTN1] %s", btn1Pressed ? "PRESSED" : "RELEASED");
    }
    if (btn2Changed) {
        btn2Changed = false;
        Logger::getInstance().logf("[BTN2] %s", btn2Pressed ? "PRESSED" : "RELEASED");
    }
    if (mcpIntAFired) {
        mcpIntAFired = false;
        mcp.handleInterruptA();
    }
    if (mcpIntBFired) {
        mcpIntBFired = false;
        mcp.handleInterruptB();
    }
}
