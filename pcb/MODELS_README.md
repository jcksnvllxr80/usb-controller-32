# KiCad Component Library — PIC32 USB Device Project

---

## 1. Adafruit Mini-B USB Breakout

| File | Purpose |
|------|---------|
| `USB_MiniB_Adafruit_Breakout.kicad_sym` | Schematic symbol (KiCad 7+) |
| `USB_MiniB_Adafruit_Breakout.kicad_mod` | PCB footprint (KiCad 7+) |

### Installation
1. **Symbol Editor** → **File > Add Library** → select `USB_MiniB_Adafruit_Breakout.kicad_sym`
2. **Footprint Editor** → **File > Add Library** → select the folder containing `USB_MiniB_Adafruit_Breakout.kicad_mod`

### Pin Map
| Pin | Signal | Notes |
|-----|--------|-------|
| 1 | VBUS | +5 V from USB host |
| 2 | D− | USB data − |
| 3 | D+ | USB data + |
| 4 | ID | OTG ID — leave NC for device mode |
| 5 | GND | Ground |
| MP1/MP2 | Shell | Mechanical tabs — tie to GND pour |

### Footprint Dimensions (datasheet File No. 719)
- Signal pad pitch: **0.8 mm**, size: **0.5 × 1.6 mm**
- Shell pad size: **1.8 × 3.0 mm** at ±3.40 mm from centre
- Courtyard: **9.90 × 5.50 mm**
- Layout tolerance: ±0.05 mm

---

## 2. Crystal — HC-49/U Vertical, 20 MHz

| Property | Value |
|----------|-------|
| Symbol | `Device:Crystal` |
| Footprint | `Crystal_HC49-U_Vertical_20MHz:Crystal_HC49-U_Vertical_20MHz` |

### Installation
KiCad built-in library — no additional installation needed.

### Pin Map
| Pin | Signal | Notes |
|-----|--------|-------|
| 1 | XIN | To OSC1 / CLKI on PIC32 |
| 2 | XOUT | To OSC2 / CLKO on PIC32 |

### Footprint Dimensions
- Package: **HC-49/U** through-hole vertical
- Pin pitch: **4.88 mm**
- Lead diameter: **0.46 mm** (drill **0.6 mm**)
- Body: **3.68 mm W × 10.16 mm H** (above board)
- Courtyard: **6.2 × 13.0 mm**

---

## 3. PIC32MX250F128B

| Property | Value |
|----------|-------|
| Symbol | `MCU_Microchip_PIC32:PIC32MX250F128B-xSP` |
| Footprint | `Package_DIP:DIP-28_W7.62mm` |

### Installation
KiCad built-in library — no additional installation needed.

### Pin Map (DIP-28)
| Pin | Signal | Notes |
|-----|--------|-------|
| 1 | MCLR | Pull to 3.3 V via 10 kΩ; program header |
| 2–3 | AN0/AN1 | Analog or GPIO |
| 4–5 | OSC1/OSC2 | Crystal connections |
| 6 | SOSCI | Secondary osc in — leave NC if unused |
| 7 | SOSCO | Secondary osc out — leave NC if unused |
| 9 | VBUS | USB VBUS sense |
| 10 | D− | USB D− (direct — no series resistor needed on MX250) |
| 11 | D+ | USB D+ |
| 13 | VCAP | LDO filter cap — 10 µF to GND, **no other load** |
| 14 | AVDD | Analog supply — 3.3 V, decouple 100 nF |
| 15 | AVSS | Analog ground |
| 19 | SCL1 | I2C clock (4.7 kΩ pull-up to 3.3 V) |
| 18 | SDA1 | I2C data (4.7 kΩ pull-up to 3.3 V) |
| 22 | U1TX | UART TX → MAX3232 T1IN |
| 21 | U1RX | UART RX ← MAX3232 R1OUT |
| 8, 17, 28 | VDD | 3.3 V — decouple 100 nF per pin |
| 8, 20, 27 | VSS | Ground |

### Footprint Dimensions
- Package: **DIP-28**, row spacing **7.62 mm**
- Pin pitch: **2.54 mm**
- Drill: **0.8 mm**, pad: **1.6 mm**
- Courtyard: **33.0 × 10.16 mm**

---

## 4. MAX3232 RS232 Level Shifter

| Property | Value |
|----------|-------|
| Symbol | `Interface_UART:MAX3232` |
| Footprint | `Package_DIP:DIP-16_W7.62mm` |

### Installation
KiCad built-in library — no additional installation needed.

### Pin Map (DIP-16)
| Pin | Signal | Notes |
|-----|--------|-------|
| 1 | C1+ | Charge pump cap — 100 nF to C1− (pin 3) |
| 2 | V+ | Charge pump output — 100 nF to GND |
| 3 | C1− | Charge pump cap — see pin 1 |
| 4 | C2+ | Charge pump cap — 100 nF to C2− (pin 5) |
| 5 | C2− | Charge pump cap — see pin 4 |
| 6 | V− | Charge pump output — 100 nF to GND |
| 7 | T2OUT | RS232 TX channel 2 out |
| 8 | R2IN | RS232 RX channel 2 in |
| 9 | R2OUT | TTL RX channel 2 out |
| 10 | T2IN | TTL TX channel 2 in |
| 11 | T1IN | TTL TX in ← PIC32 U1TX |
| 12 | R1OUT | TTL RX out → PIC32 U1RX |
| 13 | R1IN | RS232 RX in ← cable TX |
| 14 | T1OUT | RS232 TX out → cable RX |
| 15 | GND | Ground |
| 16 | VCC | 3.3 V supply — decouple 100 nF to GND |

### Footprint Dimensions
- Package: **DIP-16**, row spacing **7.62 mm**
- Pin pitch: **2.54 mm**
- Drill: **0.8 mm**, pad: **1.6 mm**
- Courtyard: **20.32 × 10.16 mm**