#pragma once

#include <cstdint>

struct GamepadReport
{
    uint8_t buttonsLo;
    uint8_t buttonsHi;
    uint8_t buttonsExtra;
    uint8_t hat;
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t rz;
};

static_assert(sizeof(GamepadReport) == 8, "HID gamepad report must stay 8 bytes");

constexpr uint8_t kGamepadHatCentered = 0x0F;
constexpr uint8_t kGamepadAxisCentered = 128;

extern GamepadReport gamepadReport;
