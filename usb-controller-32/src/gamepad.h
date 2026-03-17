#pragma once

#include "definitions.h"

struct __attribute__((packed)) GamepadReport {
    uint16_t buttons;   // 16 button bits, bit 0 = button 1
};

class Gamepad {
public:
    Gamepad();

    bool open();
    void setButton(uint8_t buttonIndex, bool pressed);
    bool sendReport();
    bool isConfigured() const { return configured_; }

private:
    static void usbDeviceEventHandler(USB_DEVICE_EVENT event,
                                      void* eventData, uintptr_t context);
    static void usbHIDEventHandler(USB_DEVICE_HID_INDEX instanceIndex,
                                   USB_DEVICE_HID_EVENT event,
                                   void* pData, uintptr_t context);

    USB_DEVICE_HANDLE deviceHandle_;
    USB_DEVICE_HID_TRANSFER_HANDLE txTransferHandle_;
    bool configured_;
    bool reportPending_;
    uint8_t idleRate_;
    uint8_t activeProtocol_;
    GamepadReport report_;
    GamepadReport lastSentReport_;
};
