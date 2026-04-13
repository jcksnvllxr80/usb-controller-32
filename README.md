# USB Controller 32

A USB HID gamepad controller built on the **PIC32MX250F128B** microcontroller. The firmware reads button inputs from GPIO pins, two potentiometers on the PIC32 ADC, and an **MCP23017** I2C I/O expander, then reports them to a host PC as a standard USB HID gamepad. The board-level serial path uses a **MAX3232** level shifter, and the MCU clock is provided by a **20 MHz crystal**. Built with **MPLAB Harmony 3**, **FreeRTOS**, and **C++**.

![Block Diagram](docs/img/bd-usb-controller-32.png)

## Features

- USB HID gamepad with 18 button inputs (2 direct GPIO + 16 via MCP23017)
- 8-byte HID input report with 18 buttons, hat switch, and two live analog axes
- MCP23017 I2C I/O expander with interrupt-driven input detection
- MAX3232-backed debug serial / RS-232 path
- `POT_1` and `POT_2` sampled on `AN4` / `AN5` for `Z` and `Rz`
- Send-on-change HID reports for buttons and potentiometer axes
- Rate-limited serial logging for potentiometer changes with `POT_1` / `POT_2` identification
- FreeRTOS-based task scheduling
- UART debug logging and USB/HID diagnostics
- Green and red status LEDs

## Hardware Used

Main ICs and timing parts on this design:

- `PIC32MX250F128B` - main microcontroller, USB device controller, ADC, GPIO, I2C, and UART
- `MCP23017` - 16-bit I2C GPIO expander for the additional button inputs
- `MAX3232` - UART level shifter for the board's serial / RS-232 debug path
- `20 MHz crystal` - external clock source for the PIC32MX250F128B
- `2x linear potentiometers` - analog controls for `POT_1` and `POT_2`, mapped to HID `Z` and `Rz`
- `Mini USB breakout` - USB connector breakout for the device port, such as the small Adafruit mini USB breakout the board is using

## Architecture

![MCP23017 Pinout](docs/img/mcp23017%20pinout.jpg)

### UML Class Diagram

![UML Diagram](docs/img/uml-diagram.png)

### State Machine

![State Diagram](docs/img/state-diagram.png)

## Pin Assignment Table

### PIC32MX250F128B Direct I/O

| Signal | Pin | Port | Direction | Description |
|---|---|---|---|---|
| MCLR | 1 | — | Input | Master clear (active-low reset); 10k pull-up to VDD; PICkit 5 pin 1 (VPP) |
| PGED1 | 2 | RA0 | Programming | ICSP data (PICkit 5 pin 4) |
| PGEC1 | 3 | RA1 | Programming | ICSP clock (PICkit 5 pin 5) |
| GREEN_LED | 4 | RB0 | Output | Green status LED |
| RED_LED | 5 | RB1 | Output | Red status LED |
| POT_1 | 6 | RB2 / AN4 | Input | Analog potentiometer mapped to HID Z axis |
| POT_2 | 7 | RB3 / AN5 | Input | Analog potentiometer mapped to HID Rz axis |
| VSS | 8 | — | Power | Ground |
| OSC1 | 9 | RA2 | Input | 20 MHz crystal input |
| OSC2 | 10 | RA3 | Output | 20 MHz crystal output |
| U1TX | 11 | RB4 | Output | UART1 transmit (debug) |
| U1RX | 12 | RA4 | Input | UART1 receive (debug) |
| VDD | 13 | — | Power | 5V supply |
| INTERRUPT_A | 14 | RB5 | Input | MCP23017 Port A interrupt (active-low) |
| VBUS | 15 | — | Power | USB 5V bus voltage; connected to VCC |
| INTERRUPT_B | 16 | RB7 | Input | MCP23017 Port B interrupt (active-low) |
| SCL1 | 17 | RB8 | Output | I2C1 clock to MCP23017 |
| SDA1 | 18 | RB9 | Bidirectional | I2C1 data to MCP23017 |
| VSS | 19 | — | Power | Ground |
| VCAP | 20 | — | Power | Internal core regulator output; 0.1µF cap to VSS |
| D+ | 21 | — | Bidirectional | USB data plus |
| D− | 22 | — | Bidirectional | USB data minus |
| VUSB3V3 | 23 | — | Power | USB 3.3V supply; decoupling capacitors to GND |
| RB13 | 24 | RB13 | NC | Not connected |
| BUTTON_1 | 25 | RB14 | Input | Direct button input 1 |
| BUTTON_2 | 26 | RB15 | Input | Direct button input 2 |
| AVSS | 27 | — | Power | Analog ground |
| AVDD | 28 | — | Power | 5V analog supply |

### MCP23017 I/O Expander (I2C address 0x20)

| Port | Pins | Direction | Pull-ups | Interrupt | Description |
|---|---|---|---|---|---|
| Port A | GPA0–GPA7 | Input | Enabled | Enabled (interrupt-on-change) | 8 button inputs |
| Port B | GPB0–GPB7 | Input | Enabled | Enabled (interrupt-on-change) | 8 button inputs |

### Input/Output Summary

| Type | Count | Source | Details |
|---|---|---|---|
| Button inputs (direct) | 2 | PIC32 GPIO | RB14, RB15 |
| Button inputs (expanded) | 16 | MCP23017 Port A & B | GPA0–7, GPB0–7 |
| Analog inputs | 2 | PIC32 ADC | POT_1 on AN4 drives HID Z, POT_2 on AN5 drives HID Rz |
| LED outputs | 2 | PIC32 GPIO | Green (RB0), Red (RB1) |
| USB HID | 1 | USB peripheral | 18 buttons, hat switch, X/Y centered, live Z/Rz in an 8-byte report |
| Debug UART | 1 | UART1 via MAX3232 | 115200 baud debug/serial path (TX: RB4, RX: RA4) |
| I2C bus | 1 | I2C1 | MCP23017 at 0x20 |

## Getting Started

### Prerequisites

- [MPLAB X IDE](https://www.microchip.com/mplab/mplab-x-ide) or the **MPLAB Extension for VS Code**
- [XC32 Compiler](https://www.microchip.com/xc32) (v4.x or later)
- [PIC32MX DFP](https://packs.download.microchip.com/) v1.6.369
- **PICkit 5** (or compatible Microchip programmer/debugger)
- Git

### Install MPLAB for VS Code

1. Open **VS Code** (or VS Code Insiders).
2. Go to the **Extensions** view (`Ctrl+Shift+X`).
3. Search for **"MPLAB"** and install the **Microchip MPLAB** extension pack.
4. The extension will prompt you to install the **XC32 compiler** and required device packs if they are not found. Follow the prompts.
5. Restart VS Code after installation.

![Harmony Configurators](docs/img/Harmony%20Configurators.png)

### Clone and Build

```bash
git clone https://github.com/jcksnvllxr80/usb-controller-32.git
cd usb-controller-32
```

1. Open the folder in VS Code.
2. The MPLAB extension should detect the project automatically via the CMake configuration in the `cmake/` directory.
3. Select the **usb-controller-32** project and the **default** configuration from the MPLAB project view.
4. Click **Build** (or press `Ctrl+Shift+B`).

### Program the Device

1. Connect the **PICkit 5** to the ICSP header on your board (ensure correct pin 1 orientation).
2. In the MPLAB extension, select **PICkit 5** as the hardware tool.
3. Set the programming speed to **Low** if you encounter connection issues.
4. Click **Program** to flash the `.hex` file to the PIC32MX250F128B.

> Warning: Use a native **USB 2.0** port when connecting this controller to the PC. On some machines, USB 3.x ports/controllers can fail enumeration or produce a Windows Code 10 error with this hardware. If your machine does not have a USB 2.0 port, connect the controller through a **USB 2.0 hub**.

### Verify Operation

1. After programming, the device should enumerate as a USB HID gamepad on the host PC.
2. Open **Set up USB game controllers** (Windows) or a gamepad tester to confirm button presses and `Z` / `Rz` potentiometer movement are registered.
3. Connect a serial terminal (115200 baud, 8N1) to UART1 to view debug logs, potentiometer change logs, and issue commands.

## UART Diagnostics

Helper scripts:

- `powershell -ExecutionPolicy Bypass -File .\tools\uart_query.ps1`
- `powershell -ExecutionPolicy Bypass -File .\tools\uart_cmd.ps1 -cmd "status"`

Useful commands:

- `status` - GPIO, USB, and LED summary
- `usb` - USB device state and register dump
- `enum` - enumeration counters and last setup request
- `hid` - HID readiness, send counters, and current report bytes
- `usblog` - recent USB transaction log
- `usbdbg` - toggle live USB debug logging
- `usbreset` - soft detach/reattach to force re-enumeration

Automatic serial logs:

- Potentiometer changes are logged automatically as `[POT] POT_1 axis=...` and/or `[POT] POT_2 axis=...`
- Logs are only emitted when the scaled axis value changes
- Pot logs are rate-limited to 5 lines per second total

## Firmware Notes

- GPIO and MCP23017 interrupts only latch state. HID reports are built and sent from `APP_Tasks()`.
- `POT_1` and `POT_2` are polled in `APP_Tasks()`, scaled from 10-bit ADC samples to 8-bit HID axis values, and mapped to HID `Z` and `Rz`.
- Potentiometer state is cached, so a new HID report is only queued when a button changes or when the last-sent `Z` / `Rz` byte changes.
- Potentiometer serial logs identify which input changed (`POT_1`, `POT_2`, or both), skip repeated values, and are rate-limited to 5 lines per second.
- HID reports are only queued when button or axis state changes, which keeps traffic and debug noise down.

## Project Structure

```
usb-controller-32/
├── cmake/                      # CMake build configurations
├── docs/                       # Datasheets, block diagram, images
├── tools/                      # Flash and UART helper scripts
├── usb-controller-32/
│   ├── mcc/                    # MPLAB Code Configurator files
│   └── src/
│       ├── main.cpp            # Entry point (SYS_Initialize + SYS_Tasks)
│       ├── app.cpp / app.h     # App task, GPIO/MCP state, USB HID send path
│       ├── command_handler.cpp / .h # UART command parser and diagnostics
│       ├── gamepad.cpp / .h    # USB HID gamepad driver
│       ├── mcp23017.cpp / .h   # MCP23017 I2C I/O expander driver
│       ├── logger.cpp / .h     # UART debug logger (singleton)
│       ├── usb_debug.c / .h    # USB debug logging helpers
│       ├── config/             # Harmony-generated peripheral config
│       └── third_party/rtos/   # FreeRTOS source
└── README.md
```

## Sponsor

PCB prototyping for this project is sponsored by [PCBWay](https://www.pcbway.com/).

PCBWay makes it easy to go from design files to manufactured boards. Upload your Gerber files, pick your board specs (layers, thickness, surface finish, color, etc.), and get an instant quote. Orders typically ship within a few days. Their online ordering process is straightforward — just create an account, upload your files, review the specs, and check out.

## License

See Microchip's license terms in the source file headers.
