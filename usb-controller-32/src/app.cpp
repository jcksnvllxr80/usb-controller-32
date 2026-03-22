#include "definitions.h"
#include "logger.h"

// Flags set by ISR, consumed by APP_Tasks
static volatile bool btn1Changed = false;
static volatile bool btn1Pressed = false;
static volatile bool btn2Changed = false;
static volatile bool btn2Pressed = false;

static void buttonCallback(GPIO_PIN pin, uintptr_t context) {
    (void)context;
    if (pin == BUTTON_1_PIN) {
        btn1Pressed = (BUTTON_1_Get() == 0);
        if (btn1Pressed) { GREEN_LED_Set(); } else { GREEN_LED_Clear(); }
        btn1Changed = true;
    } else if (pin == BUTTON_2_PIN) {
        btn2Pressed = (BUTTON_2_Get() == 0);
        if (btn2Pressed) { RED_LED_Set(); } else { RED_LED_Clear(); }
        btn2Changed = true;
    }
}

extern "C" void APP_Initialize(void) {
    Logger::getInstance().init();
    Logger::getInstance().log("APP", "System starting...");
    Logger::getInstance().logf("[APP] Initial PORTB=0x%08X TRISB=0x%08X ANSELB=0x%08X",
                               PORTB, TRISB, ANSELB);

    GPIO_PinInterruptCallbackRegister(BUTTON_1_PIN, buttonCallback, 0);
    GPIO_PinInterruptCallbackRegister(BUTTON_2_PIN, buttonCallback, 0);
    BUTTON_1_InterruptEnable();
    BUTTON_2_InterruptEnable();
}

extern "C" void APP_Tasks(void) {
    if (btn1Changed) {
        btn1Changed = false;
        Logger::getInstance().logf("[BTN1] %s  PORTB=0x%04X",
                                   btn1Pressed ? "PRESSED" : "RELEASED", PORTB);
    }
    if (btn2Changed) {
        btn2Changed = false;
        Logger::getInstance().logf("[BTN2] %s  PORTB=0x%04X",
                                   btn2Pressed ? "PRESSED" : "RELEASED", PORTB);
    }
}
