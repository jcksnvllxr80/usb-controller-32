#include "gamepad.h"
#include "logger.h"

Gamepad::Gamepad()
    : deviceHandle_(USB_DEVICE_HANDLE_INVALID)
    , txTransferHandle_(USB_DEVICE_HID_TRANSFER_HANDLE_INVALID)
    , configured_(false)
    , reportPending_(false)
    , idleRate_(0)
    , activeProtocol_(1) // Report protocol
{
    report_.buttons = 0;
    lastSentReport_.buttons = 0;
}

bool Gamepad::open() {
    deviceHandle_ = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);

    if (deviceHandle_ == USB_DEVICE_HANDLE_INVALID) {
        Logger::getInstance().log("Gamepad", "Failed to open USB device");
        return false;
    }

    USB_DEVICE_EventHandlerSet(deviceHandle_, usbDeviceEventHandler,
                               reinterpret_cast<uintptr_t>(this));

    USB_DEVICE_Attach(deviceHandle_);

    Logger::getInstance().log("Gamepad", "USB device opened, waiting for host...");
    return true;
}

void Gamepad::usbDeviceEventHandler(USB_DEVICE_EVENT event,
                                    void* eventData, uintptr_t context) {
    auto* self = reinterpret_cast<Gamepad*>(context);

    switch (event) {
        case USB_DEVICE_EVENT_RESET:
            self->configured_ = false;
            break;

        case USB_DEVICE_EVENT_CONFIGURED: {
            auto* cfg = static_cast<USB_DEVICE_EVENT_DATA_CONFIGURED*>(eventData);
            if (cfg->configurationValue == 1) {
                USB_DEVICE_HID_EventHandlerSet(USB_DEVICE_HID_INDEX_0,
                                               usbHIDEventHandler, context);
                self->configured_ = true;
                Logger::getInstance().log("Gamepad", "USB Configured");
            }
            break;
        }

        case USB_DEVICE_EVENT_POWER_DETECTED:
            USB_DEVICE_Attach(self->deviceHandle_);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:
            USB_DEVICE_Detach(self->deviceHandle_);
            self->configured_ = false;
            break;

        case USB_DEVICE_EVENT_SUSPENDED:
        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_DECONFIGURED:
            self->configured_ = false;
            break;

        default:
            break;
    }
}

void Gamepad::usbHIDEventHandler(USB_DEVICE_HID_INDEX instanceIndex,
                                 USB_DEVICE_HID_EVENT event,
                                 void* pData, uintptr_t context) {
    auto* self = reinterpret_cast<Gamepad*>(context);
    (void)instanceIndex;

    switch (event) {
        case USB_DEVICE_HID_EVENT_REPORT_SENT:
            self->reportPending_ = false;
            break;

        case USB_DEVICE_HID_EVENT_REPORT_RECEIVED:
            break;

        case USB_DEVICE_HID_EVENT_SET_IDLE: {
            auto* data = static_cast<USB_DEVICE_HID_EVENT_DATA_SET_IDLE*>(pData);
            self->idleRate_ = data->duration;
            USB_DEVICE_ControlStatus(self->deviceHandle_,
                                     USB_DEVICE_CONTROL_STATUS_OK);
            break;
        }

        case USB_DEVICE_HID_EVENT_GET_IDLE:
            USB_DEVICE_ControlSend(self->deviceHandle_,
                                   &self->idleRate_, 1);
            break;

        case USB_DEVICE_HID_EVENT_SET_PROTOCOL: {
            auto* data =
                static_cast<USB_DEVICE_HID_EVENT_DATA_SET_PROTOCOL*>(pData);
            self->activeProtocol_ = static_cast<uint8_t>(data->protocolCode);
            USB_DEVICE_ControlStatus(self->deviceHandle_,
                                     USB_DEVICE_CONTROL_STATUS_OK);
            break;
        }

        case USB_DEVICE_HID_EVENT_GET_PROTOCOL:
            USB_DEVICE_ControlSend(self->deviceHandle_,
                                   &self->activeProtocol_, 1);
            break;

        case USB_DEVICE_HID_EVENT_GET_REPORT:
            USB_DEVICE_ControlSend(self->deviceHandle_,
                                   &self->report_, sizeof(GamepadReport));
            break;

        case USB_DEVICE_HID_EVENT_SET_REPORT:
            USB_DEVICE_ControlStatus(self->deviceHandle_,
                                     USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_HID_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:
            USB_DEVICE_ControlStatus(self->deviceHandle_,
                                     USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_HID_EVENT_CONTROL_TRANSFER_DATA_SENT:
            break;

        default:
            break;
    }
}

void Gamepad::setButton(uint8_t buttonIndex, bool pressed) {
    if (buttonIndex >= 16) return;

    if (pressed) {
        report_.buttons |= static_cast<uint16_t>(1u << buttonIndex);
    } else {
        report_.buttons &= static_cast<uint16_t>(~(1u << buttonIndex));
    }
}

bool Gamepad::sendReport() {
    if (!configured_ || reportPending_) return false;

    // Only send when report state changes
    if (report_.buttons == lastSentReport_.buttons) return true;

    reportPending_ = true;
    lastSentReport_ = report_;

    USB_DEVICE_HID_RESULT result = USB_DEVICE_HID_ReportSend(
        USB_DEVICE_HID_INDEX_0,
        &txTransferHandle_,
        &report_,
        sizeof(GamepadReport));

    if (result != USB_DEVICE_HID_RESULT_OK) {
        reportPending_ = false;
        return false;
    }

    return true;
}
