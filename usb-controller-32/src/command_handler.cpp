#include "command_handler.h"
#include "logger.h"
#include <cstring>
#include <xc.h>
extern "C" {
#include "usb_debug.h"
}

extern USB_DEVICE_HANDLE usbDevHandle;
extern bool usbConfigured;
extern volatile int lastUsbEvent;

/* USB enumeration diagnostic counters (defined in usb_device.c) */
extern "C" {
extern volatile uint32_t usbDiag_setupCount;
extern volatile uint32_t usbDiag_getDescDev;
extern volatile uint32_t usbDiag_getDescCfg;
extern volatile uint32_t usbDiag_getDescStr;
extern volatile uint32_t usbDiag_setAddress;
extern volatile uint32_t usbDiag_setConfig;
extern volatile uint32_t usbDiag_ctrlSendOk;
extern volatile uint32_t usbDiag_ctrlSendFail;
extern volatile uint32_t usbDiag_ctrlSendBytes;
extern volatile uint32_t usbDiag_resetCount;
extern volatile uint32_t usbDiag_lastSetupReq;
extern volatile uint32_t usbDiag_lastSetupVal;
extern volatile uint32_t usbDiag_lastSetupLen;
extern volatile uint32_t usbDiag_irpSubmitCount;
extern volatile uint32_t usbDiag_errorCount;
extern volatile uint32_t usbDiag_errorFlags;
extern volatile bool hidTxBusy;
extern volatile bool hidReady;
extern volatile uint32_t hidSendCount;
extern volatile uint32_t hidSendFail;
extern volatile int hidLastSendResult;
extern uint8_t gamepadReport[8];

#define USB_LOG_SIZE 32
typedef struct {
    uint8_t  bmRequestType;
    uint8_t  bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
    uint8_t  action;
    uint8_t  extra;
} usb_log_entry_t;

extern volatile usb_log_entry_t usbLog[USB_LOG_SIZE];
extern volatile uint32_t usbLogHead;
}

CommandHandler& CommandHandler::getInstance() {
    static CommandHandler instance;
    return instance;
}

CommandHandler::CommandHandler()
    : rxDone_(false)
    , initialized_(false)
    , rxByte_(0)
    , cmdLen_(0)
{
    cmdBuf_[0] = '\0';
}

void CommandHandler::rxCallback(uintptr_t context) {
    auto* self = reinterpret_cast<CommandHandler*>(context);
    self->rxDone_ = true;
}

void CommandHandler::init() {
    if (initialized_) return;
    initialized_ = true;

    UART1_ReadCallbackRegister(rxCallback, reinterpret_cast<uintptr_t>(this));
    UART1_Read(&rxByte_, 1);
}

void CommandHandler::poll() {
    if (!initialized_ || !rxDone_) return;

    rxDone_ = false;

    if (rxByte_ == '\r' || rxByte_ == '\n') {
        if (cmdLen_ > 0) {
            cmdBuf_[cmdLen_] = '\0';
            processCommand();
            cmdLen_ = 0;
        }
    } else if (cmdLen_ < sizeof(cmdBuf_) - 1) {
        cmdBuf_[cmdLen_++] = static_cast<char>(rxByte_);
    }

    UART1_Read(&rxByte_, 1);
}

static const char* usbStateStr(int state) {
    switch (state) {
        case 0: return "DETACHED";
        case 1: return "ATTACHED";
        case 2: return "POWERED";
        case 3: return "DEFAULT";
        case 4: return "ADDRESSED";
        case 5: return "CONFIGURED";
        default: return "UNKNOWN";
    }
}

void CommandHandler::processCommand() {
    auto& log = Logger::getInstance();

    if (strcmp(cmdBuf_, "?") == 0 || strcmp(cmdBuf_, "help") == 0) {
        log.log("--- Commands ---");
        log.log("  status  - GPIO, USB, RTOS overview");
        log.log("  gpio    - PORTB/TRISB/LATB/ANSELB/CNENB/CNSTATB");
        log.log("  usb     - USB device state + registers");
        log.log("  enum    - USB enumeration trace counters");
        log.log("  hid     - HID report state");
        log.log("  usblog  - USB transaction log (last 32)");
        log.log("  usbdbg  - Toggle live USB debug logging");
        log.log("  usbreset- Soft detach/reattach USB");
        log.log("  btn     - Button pin states");
        log.log("  heap    - FreeRTOS heap info");
        log.log("  regs    - USB peripheral registers");
        log.log("  ping    - Alive check");

    } else if (strcmp(cmdBuf_, "ping") == 0) {
        log.log("pong");

    } else if (strcmp(cmdBuf_, "status") == 0) {
        const char* stateStr = "NO_HANDLE";
        if (usbDevHandle != USB_DEVICE_HANDLE_INVALID) {
            USB_DEVICE_STATE usbState = USB_DEVICE_StateGet(usbDevHandle);
            stateStr = usbStateStr(usbState);
        }
        log.logf("[STATUS] USB=%s Cfg=%d Evt=%d BTN1=%d BTN2=%d LED_G=%d LED_R=%d",
            stateStr, usbConfigured ? 1 : 0, lastUsbEvent,
            BUTTON_1_Get(), BUTTON_2_Get(),
            GREEN_LED_Get(), RED_LED_Get());
        log.logf("[STATUS] PORTB=0x%04X TRISB=0x%04X LATB=0x%04X",
            PORTB, TRISB, LATB);

    } else if (strcmp(cmdBuf_, "gpio") == 0) {
        log.logf("[GPIO] PORTB =0x%04X", PORTB);
        log.logf("[GPIO] TRISB =0x%04X", TRISB);
        log.logf("[GPIO] LATB  =0x%04X", LATB);
        log.logf("[GPIO] ANSELB=0x%04X", ANSELB);
        log.logf("[GPIO] CNENB =0x%04X", CNENB);
        log.logf("[GPIO] CNCONB=0x%04X", CNCONB);
        log.logf("[GPIO] CNPUB =0x%04X", CNPUB);
        log.logf("[GPIO] CNSTATB=0x%04X", CNSTATB);

    } else if (strcmp(cmdBuf_, "btn") == 0) {
        log.logf("[BTN] BTN1(RB14)=%d  BTN2(RB15)=%d",
            BUTTON_1_Get(), BUTTON_2_Get());
        log.logf("[BTN] CNENB bit14=%d bit15=%d",
            (CNENB >> 14) & 1, (CNENB >> 15) & 1);
        log.logf("[BTN] CNPUB bit14=%d bit15=%d",
            (CNPUB >> 14) & 1, (CNPUB >> 15) & 1);
        log.logf("[BTN] TRISB bit14=%d bit15=%d",
            (TRISB >> 14) & 1, (TRISB >> 15) & 1);
        log.logf("[BTN] ANSELB bit14=%d bit15=%d",
            (ANSELB >> 14) & 1, (ANSELB >> 15) & 1);

    } else if (strcmp(cmdBuf_, "usb") == 0) {
        bool handleValid = (usbDevHandle != USB_DEVICE_HANDLE_INVALID);
        log.logf("[USB] DevHandle=%s Configured=%d LastEvt=%d",
            handleValid ? "OK" : "INVALID", usbConfigured ? 1 : 0, lastUsbEvent);
        if (handleValid) {
            USB_DEVICE_STATE usbState = USB_DEVICE_StateGet(usbDevHandle);
            log.logf("[USB] State=%s (%d)", usbStateStr(usbState), usbState);
        } else {
            log.log("[USB] State=N/A (handle invalid, Open not yet succeeded)");
        }
        log.logf("[USB] U1CON =0x%04X", U1CON);
        log.logf("[USB] U1PWRC=0x%04X", U1PWRC);
        log.logf("[USB] U1OTGCON=0x%04X", U1OTGCON);
        log.logf("[USB] U1OTGSTAT=0x%04X VBUSVD=%d SESVD=%d SESEND=%d ID=%d",
            U1OTGSTAT, (U1OTGSTAT >> 0) & 1, (U1OTGSTAT >> 3) & 1,
            (U1OTGSTAT >> 2) & 1, (U1OTGSTAT >> 7) & 1);
        log.logf("[USB] U1IR  =0x%04X  U1IE =0x%04X", U1IR, U1IE);
        log.logf("[USB] U1OTGIR=0x%04X U1OTGIE=0x%04X", U1OTGIR, U1OTGIE);
        log.logf("[USB] U1STAT=0x%04X  U1ADDR=0x%04X", U1STAT, U1ADDR);
        log.logf("[USB] U1EIR =0x%04X  U1EIE =0x%04X", U1EIR, U1EIE);
        log.logf("[USB] IEC1  =0x%04X  IFS1  =0x%04X", IEC1, IFS1);

    } else if (strcmp(cmdBuf_, "regs") == 0) {
        log.logf("[REGS] DEVCFG2=0x%08lX", DEVCFG2);
        log.logf("[REGS] DEVCFG3=0x%08lX", DEVCFG3);
        log.logf("[REGS] U1CON  =0x%04X  USBEN=%d PPBRST=%d",
            U1CON, (U1CON >> 0) & 1, (U1CON >> 1) & 1);
        log.logf("[REGS] U1PWRC =0x%04X  USBPWR=%d USUSPEND=%d",
            U1PWRC, (U1PWRC >> 0) & 1, (U1PWRC >> 1) & 1);
        log.logf("[REGS] U1OTGCON=0x%04X DPPULUP=%d DMPULUP=%d",
            U1OTGCON, (U1OTGCON >> 7) & 1, (U1OTGCON >> 6) & 1);
        log.logf("[REGS] U1OTGSTAT=0x%04X VBUSVD=%d SESVD=%d SESEND=%d ID=%d",
            U1OTGSTAT, (U1OTGSTAT >> 0) & 1, (U1OTGSTAT >> 3) & 1,
            (U1OTGSTAT >> 2) & 1, (U1OTGSTAT >> 7) & 1);
        log.logf("[REGS] UPLLEN=%d UPLLIDIV=%d", (DEVCFG2 >> 15) & 1, DEVCFG2 & 7);

    } else if (strcmp(cmdBuf_, "enum") == 0) {
        log.logf("[ENUM] Resets=%lu SETUP=%lu",
            usbDiag_resetCount, usbDiag_setupCount);
        log.logf("[ENUM] GetDesc: Dev=%lu Cfg=%lu Str=%lu",
            usbDiag_getDescDev, usbDiag_getDescCfg, usbDiag_getDescStr);
        log.logf("[ENUM] SetAddr=%lu SetCfg=%lu",
            usbDiag_setAddress, usbDiag_setConfig);
        log.logf("[ENUM] CtrlSend: OK=%lu Fail=%lu LastBytes=%lu",
            usbDiag_ctrlSendOk, usbDiag_ctrlSendFail, usbDiag_ctrlSendBytes);
        log.logf("[ENUM] IRP submits=%lu", usbDiag_irpSubmitCount);
        log.logf("[ENUM] LastSetup: bReq=%lu wVal=0x%04lX wLen=%lu",
            usbDiag_lastSetupReq, usbDiag_lastSetupVal, usbDiag_lastSetupLen);
        log.logf("[ENUM] USB Errors: count=%lu flags=0x%02lX",
            usbDiag_errorCount, usbDiag_errorFlags);

    } else if (strcmp(cmdBuf_, "usblog") == 0) {
        static const char* actNames[] = {
            "SETUP", "SEND", "STALL", "STAT_OK", "RESET", "EVENT", "FWD_CLS"
        };
        uint32_t head = usbLogHead;
        uint32_t count = (head < USB_LOG_SIZE) ? head : USB_LOG_SIZE;
        uint32_t start = (head < USB_LOG_SIZE) ? 0 : (head - USB_LOG_SIZE);
        log.logf("[USBLOG] %lu entries (head=%lu)", count, head);
        for (uint32_t i = 0; i < count; i++) {
            uint32_t idx = (start + i) % USB_LOG_SIZE;
            volatile usb_log_entry_t *e = &usbLog[idx];
            const char* act = (e->action < 7) ? actNames[e->action] : "???";
            log.logf("  %02lu %s x=%02X bm=%02X bR=%02X wV=%04X wI=%04X wL=%u",
                (unsigned long)(start + i), act, e->extra,
                e->bmRequestType, e->bRequest,
                e->wValue, e->wIndex, e->wLength);
        }

    } else if (strcmp(cmdBuf_, "hid") == 0) {
        log.logf("[HID] ready=%d txBusy=%d sent=%lu fail=%lu last=%d",
            hidReady ? 1 : 0, hidTxBusy ? 1 : 0,
            hidSendCount, hidSendFail, hidLastSendResult);
        log.logf("[HID] report: %02X %02X %02X %02X %02X %02X %02X %02X",
            gamepadReport[0], gamepadReport[1], gamepadReport[2], gamepadReport[3],
            gamepadReport[4], gamepadReport[5], gamepadReport[6], gamepadReport[7]);

    } else if (strcmp(cmdBuf_, "usbdbg") == 0) {
        bool now = !usb_debug_enabled();
        usb_debug_enable(now);
        log.logf("[USBDBG] USB debug logging %s", now ? "ON" : "OFF");

    } else if (strcmp(cmdBuf_, "usbreset") == 0) {
        if (usbDevHandle != USB_DEVICE_HANDLE_INVALID) {
            log.log("[USBRESET] Detaching...");
            USB_DEVICE_Detach(usbDevHandle);
            /* Hold the disconnect long enough for the host to de-enumerate. */
            vTaskDelay(pdMS_TO_TICKS(250));
            log.log("[USBRESET] Re-attaching...");
            USB_DEVICE_Attach(usbDevHandle);
            log.log("[USBRESET] Done. Watch for re-enumeration.");
        } else {
            log.log("[USBRESET] No USB handle - device not opened");
        }

    } else if (strcmp(cmdBuf_, "heap") == 0) {
        log.logf("[HEAP] Free=%u", (unsigned)xPortGetFreeHeapSize());

    } else {
        log.logf("[CMD] Unknown: '%s'  (type ? for help)", cmdBuf_);
    }
}
