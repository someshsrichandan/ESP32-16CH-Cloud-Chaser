# Hardware Wiring Guide
## ESP32 + UCTRONICS 16-Channel Relay Module

> **v2.0 — No MQTT required.** The ESP32 connects to your Wi-Fi router and communicates with Vercel directly over HTTPS. No broker, no hub, no additional hardware needed for cloud connectivity.

---

## Table of Contents
1. [Components Required](#components-required)
2. [UCTRONICS 16-Channel Relay Module Pinout](#uctronics-16-channel-relay-module-pinout)
3. [ESP32 to Relay Board Wiring](#esp32-to-relay-board-wiring)
4. [Power Wiring](#power-wiring)
5. [Bootstrapping Pin Warnings](#bootstrapping-pin-warnings)
6. [External Pull-Up Recommendations](#external-pull-up-recommendations)
7. [Wiring Diagram](#wiring-diagram)
8. [Relay Specifications](#relay-specifications)
9. [Safety Warnings](#safety-warnings)

---

## Components Required

| Component | Quantity | Notes |
|---|---|---|
| ESP32 Dev Board (38-pin) | 1 | Any ESP32 dev board works |
| UCTRONICS 16-Channel Relay Module | 1 | 5V or 12V coil version |
| 12V DC Power Supply | 1 | Min. 2A recommended |
| 3.3V–5V Logic Level Converter | 1 | Optional but recommended |
| Jumper Wires (Male-to-Female) | 20+ | For control signals |
| Jumper Wires (Male-to-Male) | 10+ | For power rails |
| Breadboard or terminal blocks | 1 | For power distribution |

---

## UCTRONICS 16-Channel Relay Module Pinout

The UCTRONICS 16-channel relay board exposes the following header pins along its control edge:

```
 ┌──────────────────────────────────────────────────────────┐
 │  UCTRONICS 16-Channel Relay Board (Control Header)       │
 │                                                          │
 │  [GND] [VCC] [IN1] [IN2] [IN3] [IN4] ... [IN15] [IN16]  │
 └──────────────────────────────────────────────────────────┘
```

| Pin Label | Description |
|---|---|
| `GND` | Ground — connect to ESP32 GND and power supply negative |
| `VCC` | Relay coil supply — **5V or 12V** depending on your board version |
| `IN1`–`IN16` | Control signal inputs — **active LOW** (relay energizes when pin is driven LOW) |

> [!IMPORTANT]
> The UCTRONICS relay board uses **optocoupler-isolated active-LOW inputs**. A LOW signal (0V) on an `INx` pin energizes the relay. A HIGH signal (3.3V) de-energizes it. This is the default for most opto-isolated relay boards.

---

## ESP32 to Relay Board Wiring

All 16 relay channels map to specific ESP32 GPIO pins as shown below.

| Relay Channel | Board Label | ESP32 GPIO | Notes |
|:---:|:---:|:---:|---|
| Relay 1 | IN1 | **GPIO 25** | Safe for general use |
| Relay 2 | IN2 | **GPIO 4** | ⚠️ Bootstrapping pin — see warning below |
| Relay 3 | IN3 | **GPIO 5** | ⚠️ Bootstrapping pin — see warning below |
| Relay 4 | IN4 | **GPIO 26** | Safe for general use |
| Relay 5 | IN5 | **GPIO 13** | ⚠️ Must be LOW at boot (SD card pin) |
| Relay 6 | IN6 | **GPIO 27** | Safe for general use |
| Relay 7 | IN7 | **GPIO 15** | ⚠️ Bootstrapping pin — see warning below |
| Relay 8 | IN8 | **GPIO 32** | Safe for general use |
| Relay 9 | IN9 | **GPIO 16** | Safe; also used for UART2 RX |
| Relay 10 | IN10 | **GPIO 17** | Safe; also used for UART2 TX |
| Relay 11 | IN11 | **GPIO 18** | Safe; also SPI SCK |
| Relay 12 | IN12 | **GPIO 19** | Safe; also SPI MISO |
| Relay 13 | IN13 | **GPIO 21** | Safe; also I2C SDA |
| Relay 14 | IN14 | **GPIO 22** | Safe; also I2C SCL |
| Relay 15 | IN15 | **GPIO 23** | Safe; also SPI MOSI |
| Relay 16 | IN16 | **GPIO 33** | Safe for general use |

### Wiring Connection Table

```
ESP32 Pin          →   Relay Board Pin
─────────────────────────────────────────
GPIO 25            →   IN1
GPIO 4             →   IN2
GPIO 5             →   IN3
GPIO 26            →   IN4
GPIO 13            →   IN5
GPIO 27            →   IN6
GPIO 15            →   IN7
GPIO 32            →   IN8
GPIO 16            →   IN9
GPIO 17            →   IN10
GPIO 18            →   IN11
GPIO 19            →   IN12
GPIO 21            →   IN13
GPIO 22            →   IN14
GPIO 23            →   IN15
GPIO 33            →   IN16
GND                →   GND
3.3V               →   JD-VCC (if optocoupler powered separately)
```

---

## Power Wiring

The relay board requires two separate power rails:

### Relay Coil Power (12V DC)
- Connect your **12V external power supply positive** to the relay board `VCC` terminal.
- Connect your **12V power supply negative/GND** to the relay board `GND` terminal.
- **Minimum current rating:** 2A for all 16 relays simultaneously (each relay draws ~70–100mA).

### ESP32 Logic Power (3.3V)
- Power the ESP32 via USB during development, or via the `VIN` pin (5V) in production.
- The ESP32 outputs **3.3V logic** on all GPIO pins. Most UCTRONICS relay boards with optocouplers accept 3.3V input signals directly — verify with your board's datasheet.

### Ground Common Reference
- **All GND pins must be tied together:** ESP32 GND, relay board GND, and power supply negative.
- Failure to share a common ground will result in floating control signals and unpredictable behavior.

```
                    ┌─────────────────────┐
   12V PSU (+) ─────┤ VCC    Relay Board  │
   12V PSU (-) ─────┤ GND                 │
                    │                     │
   ESP32 GND   ─────┤ GND (signal GND)    │
   ESP32 GPIO  ─────┤ IN1 … IN16          │
                    └─────────────────────┘
```

> [!NOTE]
> If your UCTRONICS board has a **JD-VCC** jumper, you can optionally power the optocoupler side separately at 3.3V from the ESP32 for better isolation. Remove the JD-VCC shorting jumper and wire 3.3V to JD-VCC and GND to the optocoupler GND.

---

## Bootstrapping Pin Warnings

The ESP32 has several "strapping pins" that are sampled at boot to configure its boot mode. Driving these pins to an unexpected state during power-on **can prevent the ESP32 from booting correctly** or can cause relays to momentarily fire.

| GPIO | Bootstrapping Behavior | Risk |
|---|---|---|
| **GPIO 4** | Sampled at boot — no fixed requirement, but connected to Flash QSPI on some boards | Relay 2 may fire briefly at boot |
| **GPIO 5** | Must be HIGH at boot for SPI Flash mode. Internally pulled HIGH. | Relay 3 (active LOW board) is safe, but verify |
| **GPIO 15** | Must be HIGH at boot for SPI Flash mode. | Relay 7 may fire briefly at boot |
| **GPIO 13** | Used for SD card / JTAG. Should be LOW at boot on some configs. | Relay 5 may fire briefly at boot |

### Mitigation Strategies

1. **Initialize GPIO as OUTPUT HIGH in firmware early in `setup()`** before `delay()` calls — this minimizes the window where relays may fire.
2. **Add 10kΩ pull-up resistors** from `IN2`, `IN3`, `IN7` (GPIO 4, 5, 15) to 3.3V. Since the relay board is active-LOW, pulling the input HIGH keeps the relay off during boot.
3. **Use a relay board with enable/disable control** and keep relays disabled until the ESP32 has fully initialized.

---

## External Pull-Up Recommendations

For the three bootstrapping pins (GPIO 4, 5, 15) and GPIO 13, add **10kΩ pull-up resistors** between the ESP32 GPIO pin and 3.3V:

```
3.3V ──┬── 10kΩ ──┬── ESP32 GPIO 4 (IN2)
       │          └── Relay Board IN2
       │
3.3V ──┬── 10kΩ ──┬── ESP32 GPIO 5 (IN3)
       │          └── Relay Board IN3
       │
3.3V ──┬── 10kΩ ──┬── ESP32 GPIO 15 (IN7)
       │          └── Relay Board IN7
```

This ensures the relay inputs are held HIGH (relay OFF) during the brief moment the ESP32 configures its GPIO direction at startup.

---

## Wiring Diagram

```
                             ESP32 Dev Board
                          ┌─────────────────┐
                     3V3  │ 3V3         GND │──────────────────────┐
                     GND  │ GND         D23 │── GPIO 23 → IN15     │
                          │ D22         D22 │── GPIO 22 → IN14     │
                          │ D21         D21 │── GPIO 21 → IN13     │
                          │ D19         D19 │── GPIO 19 → IN12     │
                          │ D18         D18 │── GPIO 18 → IN11     │
                          │ D17         D17 │── GPIO 17 → IN10     │
                          │ D16         D16 │── GPIO 16 → IN9      │
                          │ D15         D15 │── GPIO 15 → IN7      │
                          │ D13         D13 │── GPIO 13 → IN5      │
                          │ D27         D27 │── GPIO 27 → IN6      │
                          │ D26         D26 │── GPIO 26 → IN4      │
                          │ D25         D25 │── GPIO 25 → IN1      │
                          │ D33         D33 │── GPIO 33 → IN16     │
                          │ D32         D32 │── GPIO 32 → IN8      │
                          │ D4           D4 │── GPIO 4  → IN2      │
                          │ D5           D5 │── GPIO 5  → IN3      │
                          └─────────────────┘
                                                        │
                             UCTRONICS 16-Ch Relay Board│
                          ┌──────────────────────────── ┘
                          │  GND ←────── ESP32 GND / PSU (-)
                          │  VCC ←────── 12V PSU (+)
                          │  IN1 ←────── GPIO 25
                          │  IN2 ←────── GPIO 4
                          │  IN3 ←────── GPIO 5
                          │  IN4 ←────── GPIO 26
                          │  IN5 ←────── GPIO 13
                          │  IN6 ←────── GPIO 27
                          │  IN7 ←────── GPIO 15
                          │  IN8 ←────── GPIO 32
                          │  IN9 ←────── GPIO 16
                          │  IN10 ←───── GPIO 17
                          │  IN11 ←───── GPIO 18
                          │  IN12 ←───── GPIO 19
                          │  IN13 ←───── GPIO 21
                          │  IN14 ←───── GPIO 22
                          │  IN15 ←───── GPIO 23
                          │  IN16 ←───── GPIO 33
                          └─────────────────────────────
```

---

## Relay Specifications

The UCTRONICS 16-channel relay board uses standard **SRD-05VDC** or **SRD-12VDC** single-pole double-throw (SPDT) relays.

| Parameter | Value |
|---|---|
| Contact Configuration | SPDT (NO / COM / NC) |
| Max AC Load | **250V AC / 10A** |
| Max DC Load | **30V DC / 10A** |
| Coil Voltage | 5V DC or 12V DC (board dependent) |
| Coil Current | ~70–90 mA per relay |
| Switching Time | ~10ms |
| Mechanical Life | 10,000,000 operations |
| Electrical Life | 100,000 operations at rated load |
| Terminal Types | Screw terminals (NO, COM, NC) |

**Terminal definitions:**
- **COM** — Common. Always connect your load circuit here.
- **NO** — Normally Open. Circuit is OPEN when relay is de-energized; CLOSED when energized.
- **NC** — Normally Closed. Circuit is CLOSED when relay is de-energized; OPEN when energized.

---

## Safety Warnings

> [!CAUTION]
> **MAINS VOLTAGE (110V/230V AC) IS LETHAL.** If you are switching mains-voltage loads, follow all of these rules without exception:

1. **Use an enclosure.** All mains wiring must be inside a properly rated electrical enclosure. Never leave live AC terminals exposed.
2. **Use appropriately rated wire.** 14 AWG minimum for 10A loads. Use properly insulated wire rated for mains voltage.
3. **Fuse every channel.** Place an appropriate fuse between the mains supply and each relay's COM terminal.
4. **Maintain creepage distance.** Keep mains-side wiring physically separated from the low-voltage control side.
5. **Never work live.** Always disconnect mains power before touching any wiring on the load side of the relays.
6. **Verify local codes.** In many jurisdictions, mains wiring must be performed by a licensed electrician.
7. **Ground metal enclosures.** Bond all metal enclosure parts to protective earth (PE).
8. **Use strain relief.** Secure all cables entering the enclosure with appropriate cable glands.

> [!WARNING]
> The relay board does **not** provide galvanic isolation between the control electronics and mains-side loads when the JD-VCC jumper is installed. For full isolation, remove the JD-VCC jumper and power the optocoupler side separately.

> [!NOTE]
> For low-voltage DC loads (LED strips, 12V devices, etc.) these risks are greatly reduced. Still fuse each channel and verify polarity before connecting loads.
