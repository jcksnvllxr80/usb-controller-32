#!/usr/bin/env python
"""
PCB Layout Script for USB Controller 32
Repositions all components, adds board outline, GND zone, and routes traces.
"""

import re
import uuid as uuid_mod
import math

PCB_FILE = 'usb-controller-32.kicad_pcb'

def uid():
    return str(uuid_mod.uuid4())

def pad_pos(cx, cy, angle, rx, ry):
    """Absolute pad position from component pos + angle + pad relative pos."""
    a = math.radians(angle)
    return (round(cx + rx*math.cos(a) - ry*math.sin(a), 2),
            round(cy + rx*math.sin(a) + ry*math.cos(a), 2))

def dip28(cx, cy, a, pin):
    if 1 <= pin <= 14:
        return pad_pos(cx, cy, a, 0, (pin-1)*2.54)
    else:
        return pad_pos(cx, cy, a, 7.62, (28-pin)*2.54)

def dip16(cx, cy, a, pin):
    if 1 <= pin <= 8:
        return pad_pos(cx, cy, a, 0, (pin-1)*2.54)
    else:
        return pad_pos(cx, cy, a, 7.62, (16-pin)*2.54)

def res_pad(cx, cy, a, pad):
    """Resistor pad: pad 1 at (0,0), pad 2 at (10.16, 0)."""
    return pad_pos(cx, cy, a, 0 if pad==1 else 10.16, 0)

def cap_pad(cx, cy, a, pad):
    """Cap pad: pad 1 at (0,0), pad 2 at (2.5, 0)."""
    return pad_pos(cx, cy, a, 0 if pad==1 else 2.5, 0)

def sw_pad(cx, cy, a, pad_num, idx=0):
    """Omron B3F switch pads: pad 1 at (0,0)/(6.5,0), pad 2 at (0,4.5)/(6.5,4.5)."""
    offsets = {(1,0): (0,0), (1,1): (6.5,0), (2,0): (0,4.5), (2,1): (6.5,4.5)}
    rx, ry = offsets[(pad_num, idx)]
    return pad_pos(cx, cy, a, rx, ry)

def pot_pad(cx, cy, a, pad):
    """Bourns 3386P: pad 1 at (0,0), pad 2 at (2.54,-2.54), pad 3 at (0,-5.08)."""
    offsets = {1: (0,0), 2: (2.54,-2.54), 3: (0,-5.08)}
    return pad_pos(cx, cy, a, *offsets[pad])

def led_pad(cx, cy, a, pad):
    """LED D5.0mm-3: pads at (0,0), (2.54,0), (5.08,0)."""
    return pad_pos(cx, cy, a, (pad-1)*2.54, 0)

def xtal_pad(cx, cy, a, pad):
    """Crystal HC49-U: pad 1 at (-2.44,0), pad 2 at (2.44,0)."""
    return pad_pos(cx, cy, a, -2.44 if pad==1 else 2.44, 0)

def usb_pad(cx, cy, a, pad):
    """USB MiniB Adafruit: pads 1-5 at (-1.6..1.6, 0), 0.8mm pitch."""
    offsets = {1: (-1.6,0), 2: (-0.8,0), 3: (0,0), 4: (0.8,0), 5: (1.6,0)}
    return pad_pos(cx, cy, a, *offsets[pad])

def header_pad(cx, cy, a, pad):
    """Pin header: pad N at (0, (N-1)*2.54)."""
    return pad_pos(cx, cy, a, 0, (pad-1)*2.54)

# ======================================================================
# COMPONENT POSITIONS: {ref: (x, y, angle)}
# ======================================================================
P = {}

# ICs
P['U1'] = (110, 48, 0)    # PIC32 MCU
P['U3'] = (142, 48, 0)    # MCP23017 I/O Expander
P['U2'] = (100, 92, 0)    # MAX232 RS232 Level Shifter

# Connectors
P['J1'] = (128, 30, 90)   # USB Mini-B at top edge
P['J2'] = (90, 40, 0)     # Programming header (1x8)
P['J3'] = (88, 98, 0)     # RS232 header (1x3)

# Crystal
P['Y1'] = (100, 68, 0)    # 20MHz crystal near U1 pins 9/10

# LED
P['D1'] = (178, 34, 0)    # Dual color LED

# Potentiometers
P['RV1'] = (165, 88, 0)
P['RV2'] = (165, 100, 0)

# Crystal load caps (rotated 90 so pads go vertical)
P['C17'] = (107, 63, 90)  # XTAL_OUT to GND
P['C18'] = (96, 63, 90)   # XTAL_IN to GND

# VCAP/VUSB decoupling caps near U1
P['C19'] = (120, 68, 0)   # VCAP (near pin 20)
P['C20'] = (120, 57, 0)   # VUSB3V3
P['C21'] = (124, 57, 0)   # VUSB3V3
P['C22'] = (120, 54, 0)   # VUSB3V3
P['C23'] = (124, 54, 0)   # VUSB3V3

# MAX232 charge pump caps (rotated 90)
P['C27'] = (93, 91, 90)   # C1-/C1+ (near pins 1,3)
P['C24'] = (93, 94, 90)   # GND/V+ (near pin 2)
P['C26'] = (93, 98, 90)   # C2-/C2+ (near pins 4,5)
P['C25'] = (93, 102, 90)  # GND/V- (near pin 6)

# Pull-up resistors (special)
P['R35'] = (96, 37, 0)    # MCLR pull-up
P['R36'] = (128, 69, 0)   # SDA pull-up
P['R37'] = (128, 73, 0)   # SCL pull-up
P['R38'] = (160, 73, 0)   # U3 RESET pull-up

# LED resistors
P['R33'] = (165, 36, 0)   # Red LED resistor
P['R34'] = (165, 33, 0)   # Green LED resistor

# ---- BUTTON GRID: 3 cols x 6 rows ----
btn_cols = [188, 208, 228]
btn_rows = [34, 48, 62, 76, 90, 104]

# Col 1: SW1-SW6, Col 2: SW7-SW12, Col 3: SW13-SW18
sw_grid = [
    ['SW1','SW7','SW13'],
    ['SW2','SW8','SW14'],
    ['SW3','SW9','SW15'],
    ['SW4','SW10','SW16'],
    ['SW5','SW11','SW17'],
    ['SW6','SW12','SW18'],
]

for ri, row in enumerate(sw_grid):
    for ci, sw in enumerate(row):
        P[sw] = (btn_cols[ci], btn_rows[ri], 0)

# ---- BUTTON ASSOCIATED COMPONENTS ----
# Mapping: SW -> (R_gpio, R_vcc, Cap_ref)
# R_gpio connects GPIO to junction, R_vcc connects +5V to junction
btn_assoc = {
    'SW3':  ('R2','R1','C1'),
    'SW4':  ('R3','R4','C2'),
    'SW5':  ('R5','R6','C3'),
    'SW6':  ('R7','R8','C4'),
    'SW7':  ('R9','R10','C5'),
    'SW8':  ('R11','R12','C6'),
    'SW9':  ('R13','R14','C7'),
    'SW10': ('R15','R16','C8'),
    'SW11': ('R17','R18','C9'),
    'SW12': ('R19','R20','C10'),
    'SW13': ('R21','R22','C11'),
    'SW14': ('R23','R24','C12'),
    'SW15': ('R25','R26','C13'),
    'SW16': ('R27','R28','C14'),
    'SW17': ('R29','R30','C15'),
    'SW18': ('R31','R32','C16'),
}

# Place each button's associated resistors and cap
for sw, (r_gpio, r_vcc, cap) in btn_assoc.items():
    sx, sy, _ = P[sw]
    # Resistors vertical (90deg) to the right of switch
    P[r_gpio] = (sx + 9, sy - 2, 90)
    P[r_vcc]  = (sx + 12, sy - 2, 90)
    # Debounce cap horizontal near top of cell
    P[cap]    = (sx + 8, sy - 5, 0)

# Verify all components
expected = set()
for prefix, nums in [('C',range(1,28)), ('R',range(1,39)), ('SW',range(1,19)),
                      ('U',[1,2,3]), ('J',[1,2,3]), ('RV',[1,2]), ('D',[1]), ('Y',[1])]:
    for n in nums:
        expected.add(f'{prefix}{n}')
missing = expected - set(P.keys())
if missing:
    print(f"WARNING: Missing positions for: {sorted(missing)}")

# ======================================================================
# READ AND MODIFY PCB FILE
# ======================================================================
with open(PCB_FILE, 'r') as f:
    lines = f.readlines()

# Parse footprints: find line index of (at ...) and reference for each footprint
footprints = []  # list of (at_line_idx, reference)
i = 0
while i < len(lines):
    if lines[i].startswith('\t(footprint ') and not lines[i].startswith('\t\t'):
        at_idx = i + 3  # (at X Y) is always 3 lines after footprint start
        # Find reference
        ref = None
        for j in range(i, min(i + 20, len(lines))):
            m = re.search(r'"Reference" "([^"]+)"', lines[j])
            if m:
                ref = m.group(1)
                break
        if ref and ref in P:
            footprints.append((at_idx, ref))
    i += 1

# Update positions
for at_idx, ref in footprints:
    x, y, angle = P[ref]
    if angle != 0:
        new_at = f'\t\t(at {x} {y} {angle})\n'
    else:
        new_at = f'\t\t(at {x} {y})\n'
    lines[at_idx] = new_at

print(f"Updated positions for {len(footprints)} components")

# ======================================================================
# COMPUTE ABSOLUTE PAD POSITIONS FOR ALL COMPONENTS
# ======================================================================
# We need these for trace routing.

def get_all_pad_positions():
    """Returns dict: {(ref, pad_str): (x, y)}"""
    pads = {}

    # U1 (DIP-28)
    cx, cy, a = P['U1']
    for pin in range(1, 29):
        pads[('U1', str(pin))] = dip28(cx, cy, a, pin)

    # U3 (DIP-28)
    cx, cy, a = P['U3']
    for pin in range(1, 29):
        pads[('U3', str(pin))] = dip28(cx, cy, a, pin)

    # U2 (DIP-16)
    cx, cy, a = P['U2']
    for pin in range(1, 17):
        pads[('U2', str(pin))] = dip16(cx, cy, a, pin)

    # J1 (USB Mini-B)
    cx, cy, a = P['J1']
    for p in range(1, 6):
        pads[('J1', str(p))] = usb_pad(cx, cy, a, p)

    # J2 (1x08 header)
    cx, cy, a = P['J2']
    for p in range(1, 9):
        pads[('J2', str(p))] = header_pad(cx, cy, a, p)

    # J3 (1x03 header)
    cx, cy, a = P['J3']
    for p in range(1, 4):
        pads[('J3', str(p))] = header_pad(cx, cy, a, p)

    # Y1 (Crystal)
    cx, cy, a = P['Y1']
    for p in [1, 2]:
        pads[('Y1', str(p))] = xtal_pad(cx, cy, a, p)

    # D1 (LED)
    cx, cy, a = P['D1']
    for p in [1, 2, 3]:
        pads[('D1', str(p))] = led_pad(cx, cy, a, p)

    # RV1, RV2 (Pots)
    for rv in ['RV1', 'RV2']:
        cx, cy, a = P[rv]
        for p in [1, 2, 3]:
            pads[(rv, str(p))] = pot_pad(cx, cy, a, p)

    # All resistors
    for r_num in range(1, 39):
        ref = f'R{r_num}'
        cx, cy, a = P[ref]
        pads[(ref, '1')] = res_pad(cx, cy, a, 1)
        pads[(ref, '2')] = res_pad(cx, cy, a, 2)

    # All caps
    for c_num in range(1, 28):
        ref = f'C{c_num}'
        cx, cy, a = P[ref]
        pads[(ref, '1')] = cap_pad(cx, cy, a, 1)
        pads[(ref, '2')] = cap_pad(cx, cy, a, 2)

    # All switches
    for s_num in range(1, 19):
        ref = f'SW{s_num}'
        cx, cy, a = P[ref]
        pads[(ref, '1a')] = sw_pad(cx, cy, a, 1, 0)
        pads[(ref, '1b')] = sw_pad(cx, cy, a, 1, 1)
        pads[(ref, '2a')] = sw_pad(cx, cy, a, 2, 0)
        pads[(ref, '2b')] = sw_pad(cx, cy, a, 2, 1)

    return pads

all_pads = get_all_pad_positions()

# ======================================================================
# NETLIST (net -> list of (ref, pad) tuples)
# ======================================================================
# Only define non-GND, non-unconnected nets that need traces.
# GND is handled by the copper zone.
# +5V is handled as a special bus.

nets = {}

def add_net(net_name, pad_key):
    nets.setdefault(net_name, []).append(pad_key)

# USB
add_net('/USB_DP', ('U1','21')); add_net('/USB_DP', ('J1','3'))
add_net('/USB_DM', ('U1','22')); add_net('/USB_DM', ('J1','4'))

# I2C
add_net('/SCL', ('U1','17')); add_net('/SCL', ('U3','12')); add_net('/SCL', ('R37','2'))
add_net('/SDA', ('U1','18')); add_net('/SDA', ('U3','13')); add_net('/SDA', ('R36','2'))

# UART
add_net('/U1TX', ('U1','11')); add_net('/U1TX', ('U2','11'))
add_net('/U1RX', ('U1','12')); add_net('/U1RX', ('U2','12'))

# RS232
add_net('/RS232_RX', ('U2','13')); add_net('/RS232_RX', ('J3','2'))
add_net('/RS232_TX', ('U2','14')); add_net('/RS232_TX', ('J3','3'))

# Crystal
add_net('/XTAL_IN', ('U1','9')); add_net('/XTAL_IN', ('Y1','1')); add_net('/XTAL_IN', ('C18','1'))
add_net('/XTAL_OUT', ('U1','10')); add_net('/XTAL_OUT', ('Y1','2')); add_net('/XTAL_OUT', ('C17','1'))

# Programming
add_net('/MCLR', ('U1','1')); add_net('/MCLR', ('J2','1')); add_net('/MCLR', ('R35','2'))
add_net('/PGED1', ('U1','2')); add_net('/PGED1', ('J2','4'))
add_net('/PGEC1', ('U1','3')); add_net('/PGEC1', ('J2','5'))

# Interrupts
add_net('/INTA', ('U1','14')); add_net('/INTA', ('U3','20'))
add_net('/INTB', ('U1','16')); add_net('/INTB', ('U3','19'))

# LED
add_net('/GREEN_LED', ('U1','4')); add_net('/GREEN_LED', ('R34','2'))
add_net('/RED_LED', ('U1','5')); add_net('/RED_LED', ('R33','2'))
add_net('Net-(D1-A1)', ('D1','1')); add_net('Net-(D1-A1)', ('R34','1'))
add_net('Net-(D1-A2)', ('D1','2')); add_net('Net-(D1-A2)', ('R33','1'))

# Potentiometers
add_net('/POT_1', ('U1','6')); add_net('/POT_1', ('RV1','2'))
add_net('/POT_2', ('U1','7')); add_net('/POT_2', ('RV2','2'))

# Direct buttons
add_net('/BTN1', ('U1','25')); add_net('/BTN1', ('SW1','2a'))
add_net('/BTN2', ('U1','26')); add_net('/BTN2', ('SW2','2a'))

# VCAP / VUSB
add_net('Net-(U1-VCAP)', ('U1','20')); add_net('Net-(U1-VCAP)', ('C19','2'))
add_net('Net-(U1-VUSB3V3)', ('U1','23'))
for c in ['C20','C21','C22','C23']:
    add_net('Net-(U1-VUSB3V3)', (c, '1'))

# MAX232 charge pump
add_net('Net-(U2-C1+)', ('U2','1')); add_net('Net-(U2-C1+)', ('C27','2'))
add_net('Net-(U2-C1-)', ('U2','3')); add_net('Net-(U2-C1-)', ('C27','1'))
add_net('Net-(U2-V+)', ('U2','2')); add_net('Net-(U2-V+)', ('C24','2'))
add_net('Net-(U2-C2+)', ('U2','4')); add_net('Net-(U2-C2+)', ('C26','2'))
add_net('Net-(U2-C2-)', ('U2','5')); add_net('Net-(U2-C2-)', ('C26','1'))
add_net('Net-(U2-V-)', ('U2','6')); add_net('Net-(U2-V-)', ('C25','2'))

# U3 Reset
add_net('Net-(U3-~{RESET})', ('U3','18')); add_net('Net-(U3-~{RESET})', ('R38','2'))

# GPIO nets (U3 -> Cap -> R_gpio)
gpio_map = {
    '/GPA0': ('U3','21','C1','R2'),  '/GPA1': ('U3','22','C2','R3'),
    '/GPA2': ('U3','23','C3','R5'),  '/GPA3': ('U3','24','C4','R7'),
    '/GPA4': ('U3','25','C5','R9'),  '/GPA5': ('U3','26','C6','R11'),
    '/GPA6': ('U3','27','C7','R13'), '/GPA7': ('U3','28','C8','R15'),
    '/GPB0': ('U3','1','C9','R17'),  '/GPB1': ('U3','2','C10','R19'),
    '/GPB2': ('U3','3','C11','R21'), '/GPB3': ('U3','4','C12','R23'),
    '/GPB4': ('U3','5','C13','R25'), '/GPB5': ('U3','6','C14','R27'),
    '/GPB6': ('U3','7','C15','R29'), '/GPB7': ('U3','8','C16','R31'),
}
for gpio_net, (u3ref, u3pin, cap_ref, r_gpio_ref) in gpio_map.items():
    add_net(gpio_net, (u3ref, u3pin))
    add_net(gpio_net, (cap_ref, '2'))
    add_net(gpio_net, (r_gpio_ref, '1'))

# Junction nets (R_gpio pad 2 + R_vcc pad 2 + SW pad 2)
junction_map = {
    'Net-(R1-Pad2)':  ('R2','R1','SW3'),
    'Net-(R3-Pad2)':  ('R3','R4','SW4'),
    'Net-(R5-Pad2)':  ('R5','R6','SW5'),
    'Net-(R7-Pad2)':  ('R7','R8','SW6'),
    'Net-(R10-Pad2)': ('R9','R10','SW7'),
    'Net-(R11-Pad2)': ('R11','R12','SW8'),
    'Net-(R13-Pad2)': ('R13','R14','SW9'),
    'Net-(R15-Pad2)': ('R15','R16','SW10'),
    'Net-(R17-Pad2)': ('R17','R18','SW11'),
    'Net-(R19-Pad2)': ('R19','R20','SW12'),
    'Net-(R21-Pad2)': ('R21','R22','SW13'),
    'Net-(R23-Pad2)': ('R23','R24','SW14'),
    'Net-(R25-Pad2)': ('R25','R26','SW15'),
    'Net-(R27-Pad2)': ('R27','R28','SW16'),
    'Net-(R29-Pad2)': ('R29','R30','SW17'),
    'Net-(R31-Pad2)': ('R31','R32','SW18'),
}
for jnet, (r_gpio, r_vcc, sw) in junction_map.items():
    add_net(jnet, (r_gpio, '2'))
    add_net(jnet, (r_vcc, '2'))
    add_net(jnet, (sw, '2b'))  # Use pad 2b (right side, closer to resistors)

# +5V net
v5_pads = [
    ('U1','13'), ('U1','15'), ('U1','28'),
    ('U2','16'), ('U3','9'), ('J2','2'),
    ('J1','5'),  # Actually +3.3V but from VBUS
    ('R35','1'), ('R36','1'), ('R37','1'), ('R38','1'),
    ('RV1','1'), ('RV2','1'),
]
# Add all R_vcc pad 1 (+5V pull-ups)
for sw, (r_gpio, r_vcc, cap) in btn_assoc.items():
    v5_pads.append((r_vcc, '1'))
for p in v5_pads:
    add_net('+5V', p)

# +3.3V (just J1 pad 5 to something? Actually it says +3.3V in the net)
# J1 pad 5 is labeled +3.3V - this is the USB VBUS feeding the board.
# Let's keep it as part of +5V for now since the schematic connects it.
# Actually, in the netlist, J1 pad 5 net is "+3.3V" not "+5V".
# The +5V comes from J2 pad 2. Let me fix this.
# Remove J1-5 from +5V and add it to +3.3V
nets['+5V'] = [p for p in nets['+5V'] if p != ('J1','5')]
add_net('+3.3V', ('J1','5'))

# ======================================================================
# TRACE GENERATION
# ======================================================================

def segment(x1, y1, x2, y2, net, layer="F.Cu", width=0.25):
    """Generate a KiCad trace segment."""
    return (f'\t(segment\n'
            f'\t\t(start {x1} {y1})\n'
            f'\t\t(end {x2} {y2})\n'
            f'\t\t(width {width})\n'
            f'\t\t(layer "{layer}")\n'
            f'\t\t(net "{net}")\n'
            f'\t\t(uuid "{uid()}")\n'
            f'\t)\n')

def via_element(x, y, net):
    """Generate a KiCad via."""
    return (f'\t(via\n'
            f'\t\t(at {x} {y})\n'
            f'\t\t(size 0.6)\n'
            f'\t\t(drill 0.3)\n'
            f'\t\t(layers "F.Cu" "B.Cu")\n'
            f'\t\t(net "{net}")\n'
            f'\t\t(uuid "{uid()}")\n'
            f'\t)\n')

def route_l(x1, y1, x2, y2, net, layer="F.Cu", width=0.25, h_first=True):
    """Route L-shaped trace between two points. Returns list of segment strings."""
    segs = []
    if abs(x1-x2) < 0.01 or abs(y1-y2) < 0.01:
        # Straight line
        segs.append(segment(x1, y1, x2, y2, net, layer, width))
    elif h_first:
        segs.append(segment(x1, y1, x2, y1, net, layer, width))
        segs.append(segment(x2, y1, x2, y2, net, layer, width))
    else:
        segs.append(segment(x1, y1, x1, y2, net, layer, width))
        segs.append(segment(x1, y2, x2, y2, net, layer, width))
    return segs

def route_net(net_name, pad_keys, layer="F.Cu", width=0.25):
    """Route a net by daisy-chaining pad positions."""
    segs = []
    points = []
    for pk in pad_keys:
        if pk in all_pads:
            points.append(all_pads[pk])
        else:
            print(f"WARNING: pad {pk} not found for net {net_name}")
    if len(points) < 2:
        return segs

    # Sort points by X then Y for reasonable daisy chain
    points.sort()

    for i in range(len(points)-1):
        x1, y1 = points[i]
        x2, y2 = points[i+1]
        # Decide horizontal-first vs vertical-first based on direction
        h_first = abs(x2-x1) >= abs(y2-y1)
        segs.extend(route_l(x1, y1, x2, y2, net_name, layer, width, h_first))

    return segs

# Generate all traces
all_traces = []

# Route each net
for net_name, pad_keys in nets.items():
    if net_name == 'GND':
        continue  # Handled by zone

    # Determine trace width
    if net_name in ['+5V', '+3.3V']:
        width = 0.5  # Power traces wider
    elif net_name.startswith('/USB'):
        width = 0.3  # USB traces
    else:
        width = 0.25

    # Route GPIO and junction nets on B.Cu to avoid F.Cu congestion
    if net_name.startswith('/GP') or net_name.startswith('Net-(R'):
        layer = "B.Cu"
    else:
        layer = "F.Cu"

    all_traces.extend(route_net(net_name, pad_keys, layer, width))

print(f"Generated {len(all_traces)} trace segments")

# ======================================================================
# BOARD OUTLINE (Edge.Cuts)
# ======================================================================
# Board rectangle with rounded corners
BL = 85   # Board Left
BT = 26   # Board Top
BR = 248  # Board Right
BB = 120  # Board Bottom
CR = 2    # Corner radius

outline = []
def gr_line(x1, y1, x2, y2):
    return (f'\t(gr_line\n'
            f'\t\t(start {x1} {y1})\n'
            f'\t\t(end {x2} {y2})\n'
            f'\t\t(stroke\n'
            f'\t\t\t(width 0.05)\n'
            f'\t\t\t(type solid)\n'
            f'\t\t)\n'
            f'\t\t(layer "Edge.Cuts")\n'
            f'\t\t(uuid "{uid()}")\n'
            f'\t)\n')

def gr_arc(sx, sy, mx, my, ex, ey):
    return (f'\t(gr_arc\n'
            f'\t\t(start {sx} {sy})\n'
            f'\t\t(mid {mx} {my})\n'
            f'\t\t(end {ex} {ey})\n'
            f'\t\t(stroke\n'
            f'\t\t\t(width 0.05)\n'
            f'\t\t\t(type solid)\n'
            f'\t\t)\n'
            f'\t\t(layer "Edge.Cuts")\n'
            f'\t\t(uuid "{uid()}")\n'
            f'\t)\n')

# Simple rectangle outline (no rounded corners for simplicity)
outline.append(gr_line(BL, BT, BR, BT))  # Top
outline.append(gr_line(BR, BT, BR, BB))  # Right
outline.append(gr_line(BR, BB, BL, BB))  # Bottom
outline.append(gr_line(BL, BB, BL, BT))  # Left

# ======================================================================
# GND COPPER ZONE (B.Cu)
# ======================================================================
gnd_zone = (
    f'\t(zone\n'
    f'\t\t(net "GND")\n'
    f'\t\t(net_name "GND")\n'
    f'\t\t(layer "B.Cu")\n'
    f'\t\t(uuid "{uid()}")\n'
    f'\t\t(name "GND_ZONE")\n'
    f'\t\t(hatch edge 0.5)\n'
    f'\t\t(priority 0)\n'
    f'\t\t(connect_pads\n'
    f'\t\t\t(clearance 0.3)\n'
    f'\t\t)\n'
    f'\t\t(min_thickness 0.25)\n'
    f'\t\t(filled_areas_thickness no)\n'
    f'\t\t(fill yes\n'
    f'\t\t\t(thermal_gap 0.5)\n'
    f'\t\t\t(thermal_bridge_width 0.5)\n'
    f'\t\t)\n'
    f'\t\t(polygon\n'
    f'\t\t\t(pts\n'
    f'\t\t\t\t(xy {BL+0.5} {BT+0.5})\n'
    f'\t\t\t\t(xy {BR-0.5} {BT+0.5})\n'
    f'\t\t\t\t(xy {BR-0.5} {BB-0.5})\n'
    f'\t\t\t\t(xy {BL+0.5} {BB-0.5})\n'
    f'\t\t\t)\n'
    f'\t\t)\n'
    f'\t)\n'
)

# ======================================================================
# +5V COPPER ZONE (F.Cu, lower priority, inner area)
# ======================================================================
v5_zone = (
    f'\t(zone\n'
    f'\t\t(net "+5V")\n'
    f'\t\t(net_name "+5V")\n'
    f'\t\t(layer "F.Cu")\n'
    f'\t\t(uuid "{uid()}")\n'
    f'\t\t(name "VCC_ZONE")\n'
    f'\t\t(hatch edge 0.5)\n'
    f'\t\t(priority 1)\n'
    f'\t\t(connect_pads\n'
    f'\t\t\t(clearance 0.3)\n'
    f'\t\t)\n'
    f'\t\t(min_thickness 0.25)\n'
    f'\t\t(filled_areas_thickness no)\n'
    f'\t\t(fill yes\n'
    f'\t\t\t(thermal_gap 0.5)\n'
    f'\t\t\t(thermal_bridge_width 0.5)\n'
    f'\t\t)\n'
    f'\t\t(polygon\n'
    f'\t\t\t(pts\n'
    f'\t\t\t\t(xy {BL+1} {BT+1})\n'
    f'\t\t\t\t(xy {BR-1} {BT+1})\n'
    f'\t\t\t\t(xy {BR-1} {BB-1})\n'
    f'\t\t\t\t(xy {BL+1} {BB-1})\n'
    f'\t\t\t)\n'
    f'\t\t)\n'
    f'\t)\n'
)

# ======================================================================
# ASSEMBLE AND WRITE OUTPUT
# ======================================================================
# Find the last line (closing paren) and insert before it
# The file ends with:
#     (embedded_fonts no)
# )

# Find the last closing paren
last_paren_idx = len(lines) - 1
while last_paren_idx >= 0 and lines[last_paren_idx].strip() != ')':
    last_paren_idx -= 1

# Also find the (embedded_fonts no) line before it
embed_idx = last_paren_idx - 1
while embed_idx >= 0 and 'embedded_fonts' not in lines[embed_idx]:
    embed_idx -= 1

# Insert new elements before the final (embedded_fonts no) and )
insert_point = embed_idx if embed_idx > 0 else last_paren_idx

new_content = []

# Board outline
new_content.append('\n\t; ---- BOARD OUTLINE ----\n')
new_content.extend(outline)

# Zones
new_content.append('\n\t; ---- GND ZONE (B.Cu) ----\n')
new_content.append(gnd_zone)
new_content.append('\n\t; ---- +5V ZONE (F.Cu) ----\n')
new_content.append(v5_zone)

# Traces
new_content.append('\n\t; ---- TRACES ----\n')
new_content.extend(all_traces)

# Insert
lines_before = lines[:insert_point]
lines_after = lines[insert_point:]
final_lines = lines_before + new_content + lines_after

with open(PCB_FILE, 'w') as f:
    f.writelines(final_lines)

print(f"PCB file updated successfully!")
print(f"  Board outline: {BR-BL}mm x {BB-BT}mm")
print(f"  Components placed: {len(footprints)}")
print(f"  Trace segments: {len(all_traces)}")
print(f"  Zones: GND (B.Cu) + +5V (F.Cu)")
print(f"\nOpen in KiCad and run DRC. You may need to:")
print(f"  1. Edit > Fill All Zones (B shortcut) to fill the copper zones")
print(f"  2. Fine-tune component positions if any overlap")
print(f"  3. Add any remaining traces flagged by the ratsnest")
