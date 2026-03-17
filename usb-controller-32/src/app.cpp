/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "logger.h"
#include "mcp23017.h"
#include "gamepad.h"

extern "C" {
#include "semphr.h"
}

static APP_DATA appData;
static Gamepad gamepad;
static MCP23017 mcp(MCP23017::DEFAULT_ADDRESS);

// Event semaphore: signaled from ISR callbacks to wake APP_Tasks
static SemaphoreHandle_t eventSem = nullptr;

// Volatile state shared between ISR callbacks and APP_Tasks
static volatile bool button1Pressed = false;
static volatile bool button2Pressed = false;
static volatile bool buttonChanged = false;
static volatile bool mcpInterruptPending = false;

// -- ISR Callbacks (called from CN interrupt handler) --

static void onButton1Change(GPIO_PIN pin, uintptr_t context) {
    bool pressed = (BUTTON_1_Get() != 0);
    button1Pressed = pressed;
    buttonChanged = true;
    if (pressed) { GREEN_LED_Set(); } else { GREEN_LED_Clear(); }
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(eventSem, &xHigherPriorityTaskWoken);
}

static void onButton2Change(GPIO_PIN pin, uintptr_t context) {
    bool pressed = (BUTTON_2_Get() != 0);
    button2Pressed = pressed;
    buttonChanged = true;
    if (pressed) { RED_LED_Set(); } else { RED_LED_Clear(); }
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(eventSem, &xHigherPriorityTaskWoken);
}

static void onMcpInterrupt(GPIO_PIN pin, uintptr_t context) {
    mcpInterruptPending = true;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(eventSem, &xHigherPriorityTaskWoken);
}

void APP_Initialize ( void )
{
    appData.state = APP_STATE_INIT;
}

void APP_Tasks ( void )
{
    switch ( appData.state )
    {
        case APP_STATE_INIT:
        {
            Logger::getInstance().init();
            Logger::getInstance().log("APP", "System starting...");
            eventSem = xSemaphoreCreateBinary();
            appData.state = APP_STATE_INIT_PERIPHERALS;
            break;
        }

        case APP_STATE_INIT_PERIPHERALS:
        {
            if (!mcp.init()) {
                Logger::getInstance().log("APP", "MCP23017 init failed, retrying...");
                vTaskDelay(pdMS_TO_TICKS(500));
                break;
            }

            // Register CN interrupt callbacks
            GPIO_PinInterruptCallbackRegister(GPIO_PIN_RB14, onButton1Change, 0);
            GPIO_PinInterruptCallbackRegister(GPIO_PIN_RB15, onButton2Change, 0);
            GPIO_PinInterruptCallbackRegister(GPIO_PIN_RB5,  onMcpInterrupt, 0);
            GPIO_PinInterruptCallbackRegister(GPIO_PIN_RB7,  onMcpInterrupt, 0);

            // Enable CN interrupts on all 4 pins
            BUTTON_1_InterruptEnable();
            BUTTON_2_InterruptEnable();
            INTERRUPT_A_InterruptEnable();
            INTERRUPT_B_InterruptEnable();

            Logger::getInstance().log("APP", "GPIO interrupts enabled");
            appData.state = APP_STATE_USB_OPEN;
            break;
        }

        case APP_STATE_USB_OPEN:
        {
            if (!gamepad.open()) {
                Logger::getInstance().log("APP", "USB open failed, retrying...");
                vTaskDelay(pdMS_TO_TICKS(500));
                break;
            }
            appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
            break;
        }

        case APP_STATE_WAIT_FOR_CONFIGURATION:
        {
            if (gamepad.isConfigured()) {
                Logger::getInstance().log("APP", "USB configured, entering main loop");
                appData.state = APP_STATE_RUNNING;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
            break;
        }

        case APP_STATE_RUNNING:
        {
            // Sleep until a GPIO interrupt fires (or timeout for housekeeping)
            xSemaphoreTake(eventSem, pdMS_TO_TICKS(100));

            // Handle button state changes - update gamepad and send report
            if (buttonChanged) {
                buttonChanged = false;
                gamepad.setButton(0, button1Pressed);
                gamepad.setButton(1, button2Pressed);
                if (gamepad.isConfigured()) {
                    gamepad.sendReport();
                }
            }

            // Handle MCP23017 interrupts - read registers in task context
            if (mcpInterruptPending) {
                mcpInterruptPending = false;
                mcp.handleInterrupts();
            }

            break;
        }

        case APP_STATE_ERROR:
        default:
        {
            Logger::getInstance().log("APP", "Error state");
            vTaskDelay(pdMS_TO_TICKS(1000));
            break;
        }
    }
}
