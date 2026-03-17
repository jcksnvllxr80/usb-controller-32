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

static APP_DATA appData;
static Gamepad gamepad;
static MCP23017 mcp(MCP23017::DEFAULT_ADDRESS);

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
            // Read buttons directly connected to PIC32
            bool btn1 = (BUTTON_1_Get() != 0);
            bool btn2 = (BUTTON_2_Get() != 0);

            // Drive LEDs: green for button 1, red for button 2
            if (btn1) { GREEN_LED_Set(); } else { GREEN_LED_Clear(); }
            if (btn2) { RED_LED_Set(); }   else { RED_LED_Clear(); }

            // Map to gamepad buttons
            gamepad.setButton(0, btn1);
            gamepad.setButton(1, btn2);

            // Send USB HID report when configured
            if (gamepad.isConfigured()) {
                gamepad.sendReport();
            }

            // Service MCP23017 interrupt pins
            mcp.handleInterrupts();

            vTaskDelay(pdMS_TO_TICKS(1));
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
