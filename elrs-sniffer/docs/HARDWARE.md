# RadioMaster RP2 V2 Hardware Details

## Overview

The RadioMaster RP2 V2 Nano Receiver is a compact ExpressLRS receiver featuring:
- **MCU**: ESP8285 (ESP8266 with 1MB integrated flash)
- **RF Chip**: SX1280 or SX1281 (2.4 GHz LoRa transceiver)
- **Antenna**: Built-in ceramic antenna
- **Power**: 5V input
- **Communication**: CRSF protocol (serial)

## Pinout

### External Connections (Solder Pads)

| Pin | Function | Description |
|-----|----------|-------------|
| 5V  | Power Input | Connect to 5V source (flight controller or BEC) |
| GND | Ground | Common ground |
| TX  | UART TX | Transmit data (CRSF telemetry) |
| RX  | UART RX | Receive data (CRSF commands) |

### Internal Connections (ESP8285 ↔ SX1280)

**Note**: These pin mappings are based on standard ESP8285 SPI configuration. If your RP2 V2 uses different pins, update `src/config.h`.

| ESP8285 GPIO | SX1280 Pin | Function |
|--------------|------------|----------|
| GPIO12 | MISO | SPI Master In, Slave Out |
| GPIO13 | MOSI | SPI Master Out, Slave In |
| GPIO14 | SCK | SPI Clock |
| GPIO15 | NSS | SPI Chip Select |
| GPIO4  | RST | Radio Reset |
| GPIO5  | BUSY | Radio Busy Signal |
| GPIO2  | DIO1 | Digital I/O 1 (Interrupt) |
| GPIO16 | LED | Status LED (optional) |

## SX1280/SX1281 RF Chip

### Specifications
- **Frequency Range**: 2400 - 2500 MHz
- **Modulation**: LoRa, FLRC, GFSK
- **Output Power**: Up to +12.5 dBm
- **Sensitivity**: Down to -132 dBm
- **Data Rate**: Up to 203 kbps (FLRC mode)

### LoRa Parameters (ELRS)

ExpressLRS uses LoRa modulation with various configurations:

| Packet Rate | Spreading Factor | Bandwidth | Coding Rate |
|-------------|------------------|-----------|-------------|
| 50 Hz | SF9 | 800 kHz | 4/7 |
| 150 Hz | SF7 | 800 kHz | 4/7 |
| 250 Hz | SF6 | 800 kHz | 4/6 |
| 500 Hz | SF5 | 800 kHz | 4/6 |

## Power Requirements

- **Input Voltage**: 5V ± 0.5V
- **Current Draw**: 
  - Idle: ~50 mA
  - Active RX: ~80-120 mA
  - Peak: ~150 mA

**Important**: Ensure your power source can provide stable 5V with sufficient current.

## LED Indicator

| LED State | Meaning |
|-----------|---------|
| Solid ON | Initializing / Processing packet |
| OFF | Ready / Idle |
| Fast Blink (200ms) | Error / Failed initialization |
| Slow Blink (500ms) | Web server error |

## Flashing Firmware

### Required Hardware
- USB-to-Serial adapter (3.3V or 5V logic)
- Jumper wires

### Connections

```
RP2 V2          USB-Serial
------          ----------
TX      ───────► RX
RX      ◄─────── TX
GND     ───────► GND
5V      ───────► 5V (or external power)
GPIO0   ───────► GND (during boot only)
```

### Bootloader Mode

1. Connect GPIO0 to GND
2. Apply power (or press reset if available)
3. Release GPIO0 after 1 second
4. ESP8285 is now in bootloader mode
5. Flash using PlatformIO: `pio run --target upload`

**Note**: GPIO0 location may require soldering to an internal test pad. Consult RP2 V2 documentation or use Wi-Fi flashing if already running ExpressLRS firmware.

## Wi-Fi Flashing (Alternative)

If your RP2 V2 is already running ExpressLRS firmware:

1. Power on the receiver
2. Press bind button 3 times quickly
3. Connect to "ExpressLRS RX" Wi-Fi network
4. Navigate to http://10.0.0.1
5. Upload compiled firmware binary

## Schematic Notes

The RP2 V2 schematic is not publicly available. Pin mappings in this project are based on:
- Standard ESP8285 SPI pin assignments
- Common ExpressLRS receiver designs
- Community reverse engineering

If you experience issues, the pins may differ. To find correct pins:
1. Examine the ExpressLRS source code for RP2 V2 target
2. Use a multimeter to trace PCB connections
3. Consult RadioMaster support

## Safety Warnings

⚠️ **Do not exceed 5.5V input** - This will damage the ESP8285

⚠️ **ESD Sensitive** - Handle with anti-static precautions

⚠️ **No reverse polarity protection** - Double-check power connections

## Dimensions

- **Length**: ~20mm
- **Width**: ~11mm
- **Height**: ~4mm (with antenna)
- **Weight**: ~1g

## Operating Environment

- **Temperature**: -20°C to +85°C
- **Humidity**: 5% to 95% non-condensing

---

For more information, visit [RadioMaster Official Website](https://www.radiomasterrc.com/)
