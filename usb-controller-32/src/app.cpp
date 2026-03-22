#include "definitions.h"
#include "logger.h"

static bool lastB1 = false;
static bool lastB2 = false;

extern "C" void APP_Initialize(void) {
    Logger::getInstance().init();
    Logger::getInstance().log("APP", "System starting...");
}

extern "C" void APP_Tasks(void) {
    bool currB1 = (BUTTON_1_Get() == 0);
    if (currB1 != lastB1) {
        lastB1 = currB1;
        if (currB1) { GREEN_LED_Set(); } else { GREEN_LED_Clear(); }
    }

    bool currB2 = (BUTTON_2_Get() == 0);
    if (currB2 != lastB2) {
        lastB2 = currB2;
        if (currB2) { RED_LED_Set(); } else { RED_LED_Clear(); }
    }
}
