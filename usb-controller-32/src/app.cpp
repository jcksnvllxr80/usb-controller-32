#include "definitions.h"
#include "logger.h"
#include "mcp23017.h"
#include "command_handler.h"
#include <cstring>
extern "C" {
#include "usb_debug.h"
}


static MCP23017 mcp;
static bool appInitialized = false;
USB_DEVICE_HANDLE usbDevHandle = USB_DEVICE_HANDLE_INVALID;
bool usbConfigured = false;
volatile int lastUsbEvent = -1;

static volatile bool btn1Changed = false;
static volatile bool btn1Pressed = false;
static volatile bool btn2Changed = false;
static volatile bool btn2Pressed = false;
static volatile bool mcpIntAFired = false;
static volatile bool mcpIntBFired = false;
static bool hidHandlerRegistered = false;
static bool usbAttachPending = false;
static TickType_t usbAttachStartTick = 0;
static const TickType_t kUsbAttachDelayTicks = pdMS_TO_TICKS(250);
static const TickType_t kHidStartupQuietTicks = pdMS_TO_TICKS(1000);
static volatile bool hidHostReady = false;
static volatile TickType_t usbConfiguredTick = 0;

/* ---- HID gamepad report ---- */
USB_DEVICE_HID_TRANSFER_HANDLE hidTxHandle = USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;
volatile bool hidTxBusy = false;
volatile bool hidReady = false;
volatile uint32_t hidSendCount = 0;
volatile uint32_t hidSendFail = 0;
volatile int hidLastSendResult = 0;
static volatile bool hidReportDirty = true;
static bool hidReportSynced = false;
static uint8_t hidIdleRate = 0;
static uint8_t hidActiveProtocol = 1;
static constexpr uint8_t kDefaultHidReportId = 0;

/* 8-byte gamepad report matching hid_rpt0 descriptor:
 *   [0]    buttons 1-8   (bits 0-7)
 *   [1]    buttons 9-16  (bits 0-7)
 *   [2]    buttons 17-18 (bits 0-1), padding (bits 2-7)
 *   [3]    hat switch (bits 0-3), padding (bits 4-7)
 *   [4]    X  axis (0-255, 128=center)
 *   [5]    Y  axis
 *   [6]    Z  axis
 *   [7]    Rz axis
 */
uint8_t gamepadReport[8] = { 0, 0, 0, 0x0F, 128, 128, 128, 128 };

static void buildGamepadReport(uint8_t report[8]) {
    uint8_t portA = 0xFF;
    uint8_t portB = 0xFF;

    (void)mcp.readPortA(portA);
    (void)mcp.readPortB(portB);

    uint32_t buttons = 0;

    /* MCP23017 buttons are active-low. */
    buttons |= (uint32_t)(~portA & 0xFF);
    buttons |= (uint32_t)(~portB & 0xFF) << 8;

    if (btn1Pressed) {
        buttons |= (1u << 16);
    }
    if (btn2Pressed) {
        buttons |= (1u << 17);
    }

    report[0] = (uint8_t)(buttons & 0xFF);
    report[1] = (uint8_t)((buttons >> 8) & 0xFF);
    report[2] = (uint8_t)((buttons >> 16) & 0x03);
    report[3] = 0x0F;
    report[4] = 128;
    report[5] = 128;
    report[6] = 128;
    report[7] = 128;
}

static USB_DEVICE_HID_EVENT_RESPONSE hidEventCallback(
    USB_DEVICE_HID_INDEX instanceIndex,
    USB_DEVICE_HID_EVENT event,
    void* eventData,
    uintptr_t context)
{
    (void)instanceIndex;
    (void)context;
    switch (event) {
        case USB_DEVICE_HID_EVENT_REPORT_SENT:
            hidTxBusy = false;
            break;
        case USB_DEVICE_HID_EVENT_REPORT_RECEIVED:
            /* Host sent us data (e.g. output report) — ignore for now */
            break;
        case USB_DEVICE_HID_EVENT_SET_IDLE: {
            auto* data = static_cast<USB_DEVICE_HID_EVENT_DATA_SET_IDLE*>(eventData);
            hidHostReady = true;
            hidIdleRate = data->duration;
            USB_DEVICE_ControlStatus(usbDevHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;
        }
        case USB_DEVICE_HID_EVENT_GET_IDLE:
            hidHostReady = true;
            USB_DEVICE_ControlSend(usbDevHandle, &hidIdleRate, 1);
            break;
        case USB_DEVICE_HID_EVENT_SET_PROTOCOL: {
            auto* data = static_cast<USB_DEVICE_HID_EVENT_DATA_SET_PROTOCOL*>(eventData);
            hidHostReady = true;
            hidActiveProtocol = (uint8_t)data->protocolCode;
            USB_DEVICE_ControlStatus(usbDevHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;
        }
        case USB_DEVICE_HID_EVENT_GET_PROTOCOL:
            hidHostReady = true;
            USB_DEVICE_ControlSend(usbDevHandle, &hidActiveProtocol, 1);
            break;
        case USB_DEVICE_HID_EVENT_GET_REPORT: {
            auto* data = static_cast<USB_DEVICE_HID_EVENT_DATA_GET_REPORT*>(eventData);
            hidHostReady = true;
            if ((data->reportType == USB_HID_REPORT_TYPE_INPUT) &&
                (data->reportID == kDefaultHidReportId)) {
                buildGamepadReport(gamepadReport);
                const size_t length = (data->reportLength < sizeof(gamepadReport))
                    ? data->reportLength
                    : sizeof(gamepadReport);
                USB_DEVICE_ControlSend(usbDevHandle, gamepadReport, length);
            } else {
                USB_DEVICE_ControlStatus(usbDevHandle, USB_DEVICE_CONTROL_STATUS_ERROR);
            }
            break;
        }
        case USB_DEVICE_HID_EVENT_SET_REPORT: {
            auto* data = static_cast<USB_DEVICE_HID_EVENT_DATA_SET_REPORT*>(eventData);
            hidHostReady = true;
            (void)data;
            USB_DEVICE_ControlStatus(usbDevHandle, USB_DEVICE_CONTROL_STATUS_ERROR);
            break;
        }
        case USB_DEVICE_HID_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:
            USB_DEVICE_ControlStatus(usbDevHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;
        case USB_DEVICE_HID_EVENT_CONTROL_TRANSFER_DATA_SENT:
        case USB_DEVICE_HID_EVENT_CONTROL_TRANSFER_ABORTED:
            break;
        default:
            break;
    }
    return USB_DEVICE_HID_EVENT_RESPONSE_NONE;
}

static USB_DEVICE_EVENT_RESPONSE usbDeviceEventCallback(
    USB_DEVICE_EVENT event, void* eventData, uintptr_t context)
{
    (void)context;
    /* Called from ISR context — NO Logger/UART calls allowed here! */
    lastUsbEvent = (int)event;
    switch (event) {
        case USB_DEVICE_EVENT_CONFIGURED: {
            auto* configured = static_cast<USB_DEVICE_EVENT_DATA_CONFIGURED*>(eventData);
            usbConfigured = (configured != nullptr) && (configured->configurationValue == 1u);
            usbConfiguredTick = xTaskGetTickCountFromISR();
            hidHostReady = false;
            hidReady = usbConfigured && hidHandlerRegistered;
            hidTxBusy = false;
            hidTxHandle = USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;
            hidReportDirty = true;
            hidReportSynced = false;
            break;
        }
        case USB_DEVICE_EVENT_RESET:
        case USB_DEVICE_EVENT_SUSPENDED:
        case USB_DEVICE_EVENT_DECONFIGURED:
        case USB_DEVICE_EVENT_POWER_REMOVED:
            usbConfigured = false;
            hidHostReady = false;
            usbConfiguredTick = 0;
            hidReady = false;
            hidTxBusy = false;
            hidTxHandle = USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;
            hidReportDirty = true;
            hidReportSynced = false;
            break;
        default:
            break;
    }
    return USB_DEVICE_EVENT_RESPONSE_NONE;
}

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
    /* Register GPIO callbacks — these don't need interrupts enabled yet */
    GPIO_PinInterruptCallbackRegister(BUTTON_1_PIN,    pinCallback, 0);
    GPIO_PinInterruptCallbackRegister(BUTTON_2_PIN,    pinCallback, 0);
    GPIO_PinInterruptCallbackRegister(INTERRUPT_A_PIN, pinCallback, 0);
    GPIO_PinInterruptCallbackRegister(INTERRUPT_B_PIN, pinCallback, 0);

    BUTTON_1_InterruptEnable();
    BUTTON_2_InterruptEnable();
    INTERRUPT_A_InterruptEnable();
    INTERRUPT_B_InterruptEnable();
}

extern "C" void APP_Tasks(void) {
    /* Deferred init — runs under RTOS with interrupts enabled */
    if (!appInitialized) {
        appInitialized = true;

        Logger::getInstance().init();
        Logger::getInstance().log("APP", "System starting...");

        CommandHandler::getInstance().init();

        btn1Pressed = (BUTTON_1_Get() == 0);
        btn2Pressed = (BUTTON_2_Get() == 0);

        if (!mcp.init()) {
            Logger::getInstance().log("APP", "MCP23017 init failed");
        }
    }

    /* Keep trying to open USB device until the device layer is ready */
    if (usbDevHandle == USB_DEVICE_HANDLE_INVALID) {
        usbDevHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);
        if (usbDevHandle != USB_DEVICE_HANDLE_INVALID) {
            USB_DEVICE_EventHandlerSet(usbDevHandle, usbDeviceEventCallback, 0);
            hidHandlerRegistered =
                (USB_DEVICE_HID_EventHandlerSet(USB_DEVICE_HID_INDEX_0, hidEventCallback, 0) == USB_DEVICE_HID_RESULT_OK);
            usbAttachStartTick = xTaskGetTickCount();
            usbAttachPending = true;
            Logger::getInstance().log("APP", "USB device opened; delaying attach for host re-detect");
        }
    }

    if ((usbDevHandle != USB_DEVICE_HANDLE_INVALID) && usbAttachPending) {
        TickType_t elapsed = xTaskGetTickCount() - usbAttachStartTick;
        if (elapsed >= kUsbAttachDelayTicks) {
            USB_DEVICE_Attach(usbDevHandle);
            usbAttachPending = false;
            Logger::getInstance().log("APP", "USB attached");
        }
    }

    CommandHandler::getInstance().poll();
    usb_debug_drain();

    if (btn1Changed) {
        btn1Changed = false;
        hidReportDirty = true;
        Logger::getInstance().logf("[BTN1] %s", btn1Pressed ? "PRESSED" : "RELEASED");
    }
    if (btn2Changed) {
        btn2Changed = false;
        hidReportDirty = true;
        Logger::getInstance().logf("[BTN2] %s", btn2Pressed ? "PRESSED" : "RELEASED");
    }
    if (mcpIntAFired) {
        mcpIntAFired = false;
        if (mcp.handleInterruptA()) {
            hidReportDirty = true;
        }
    }
    if (mcpIntBFired) {
        mcpIntBFired = false;
        if (mcp.handleInterruptB()) {
            hidReportDirty = true;
        }
    }

    /* ---- Build and send HID gamepad report ---- */
    const bool hidStartupQuietComplete =
        hidReady && ((xTaskGetTickCount() - usbConfiguredTick) >= kHidStartupQuietTicks);

    if (hidReady && (hidHostReady || hidStartupQuietComplete) && !hidTxBusy && hidReportDirty) {
        uint8_t nextReport[sizeof(gamepadReport)];
        buildGamepadReport(nextReport);

        if (hidReportSynced &&
            std::memcmp(gamepadReport, nextReport, sizeof(gamepadReport)) == 0) {
            hidReportDirty = false;
        } else {
            std::memcpy(gamepadReport, nextReport, sizeof(gamepadReport));
            hidTxBusy = true;
            USB_DEVICE_HID_RESULT res = USB_DEVICE_HID_ReportSend(
                USB_DEVICE_HID_INDEX_0, &hidTxHandle,
                gamepadReport, sizeof(gamepadReport));
            hidLastSendResult = (int)res;
            if (res == USB_DEVICE_HID_RESULT_OK) {
                hidSendCount++;
                hidReportSynced = true;
                hidReportDirty = false;
            } else {
                hidTxBusy = false;
                hidSendFail++;
            }
        }
    }
}
