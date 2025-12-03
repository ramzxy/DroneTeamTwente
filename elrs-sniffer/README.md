# ELRS Packet Sniffer

**Educational tool for analyzing ExpressLRS over-the-air packets**

⚠️ **WARNING: EDUCATIONAL USE ONLY**

This firmware transforms a RadioMaster RP2 V2 Nano Receiver into a packet sniffer for learning about ExpressLRS radio protocols. It is designed strictly for educational purposes.

**DO NOT USE FOR:**
- Unauthorized monitoring of third-party communications
- Transmitting on ELRS frequencies
- Any activity that violates local radio regulations

## Hardware Requirements

- **RadioMaster RP2 V2 Nano Receiver**
  - ESP8285 MCU
  - SX1280/SX1281 RF chip
- **USB-to-Serial adapter** (for flashing firmware)
- **5V power source**

## Features

- 📡 **Promiscuous Mode**: Captures all ELRS packets on configured frequency
- 🌐 **Web Interface**: Real-time packet display via Wi-Fi
- 📊 **Live Statistics**: RSSI, SNR, packet rate monitoring
- 🔍 **CRSF Decoder**: Automatic channel data extraction
- 💾 **CSV Export**: Download captured packets for offline analysis
- 📱 **WebSocket**: Real-time packet streaming to browser

## Quick Start

### 1. Install PlatformIO

Install [PlatformIO IDE](https://platformio.org/install) or PlatformIO Core CLI.

### 2. Clone/Download Project

```bash
cd /path/to/elrs-sniffer
```

### 3. Configure Settings (Optional)

Edit `src/config.h` to customize:
- Wi-Fi credentials (default: SSID="ELRS_Sniffer", password="sniffer123")
- ELRS frequency and packet rate
- Buffer sizes

### 4. Build Firmware

```bash
pio run
```

### 5. Flash to RP2 V2

Connect your RP2 V2 via USB-to-serial adapter:
- **TX** (RP2) → **RX** (adapter)
- **RX** (RP2) → **TX** (adapter)
- **GND** → **GND**
- **5V** → **5V**

Put RP2 V2 in bootloader mode (short GPIO0 to GND during power-on), then:

```bash
pio run --target upload
```

### 6. Use the Sniffer

1. Power on the RP2 V2 (5V)
2. Connect to Wi-Fi network: **ELRS_Sniffer** (password: **sniffer123**)
3. Open browser to: **http://192.168.4.1**
4. Power on an ELRS transmitter
5. Watch packets appear in real-time!

## Web Interface

The web interface displays:
- **Timestamp**: Milliseconds since boot
- **RSSI**: Received Signal Strength Indicator (dBm)
- **SNR**: Signal-to-Noise Ratio (dB)
- **Length**: Packet length in bytes
- **Raw Hex**: Complete packet data in hexadecimal
- **Channels 1-4**: Decoded RC channel values (if valid CRSF packet)

### Controls
- **Clear Display**: Clear the packet table
- **Download CSV**: Export all captured packets
- **Reload Page**: Refresh the interface

## Configuration

### ELRS Parameters (`src/config.h`)

Match these to your ELRS system:

```cpp
#define ELRS_FREQUENCY      2440000000UL  // 2440 MHz
#define LORA_SPREADING_FACTOR   6         // SF6
#define LORA_BANDWIDTH          LORA_BW_0800  // 800 kHz
#define LORA_CODING_RATE        LORA_CR_LI_4_6  // 4/6
#define ELRS_PACKET_RATE        250       // 250 Hz
```

Common ELRS modes:
- **250Hz**: SF6, BW800, CR4/6
- **500Hz**: SF5, BW800, CR4/6
- **150Hz**: SF7, BW800, CR4/7

### Pin Definitions

If your RP2 V2 uses different GPIO pins, update in `src/config.h`:

```cpp
#define PIN_SPI_MISO    12
#define PIN_SPI_MOSI    13
#define PIN_SPI_SCK     14
#define PIN_SPI_NSS     15
#define PIN_RADIO_RST   4
#define PIN_RADIO_BUSY  5
#define PIN_RADIO_DIO1  2
```

## Troubleshooting

### No packets appearing

1. **Check frequency**: Ensure `ELRS_FREQUENCY` matches your transmitter
2. **Check packet rate**: Ensure LoRa parameters match your ELRS mode
3. **Check range**: Move transmitter closer to sniffer
4. **Check serial output**: Connect to serial monitor (115200 baud) for debug info

### Can't connect to Wi-Fi

1. **Check SSID**: Look for "ELRS_Sniffer" network
2. **Check password**: Default is "sniffer123"
3. **Restart device**: Power cycle the RP2 V2
4. **Check serial output**: Verify AP started successfully

### Upload fails

1. **Bootloader mode**: Ensure GPIO0 is grounded during power-on
2. **Check connections**: Verify TX/RX are crossed correctly
3. **Try slower speed**: Edit `platformio.ini`, set `upload_speed = 115200`

## Technical Details

### Architecture

```
┌─────────────────┐
│   ESP8285 MCU   │
│                 │
│  ┌───────────┐  │
│  │ Main Loop │  │
│  └─────┬─────┘  │
│        │        │
│  ┌─────▼──────┐ │     ┌──────────────┐
│  │ SX1280     │◄├─────┤ SX1280/SX1281│
│  │ Driver     │ │ SPI │   RF Chip    │
│  └─────┬──────┘ │     └──────────────┘
│        │        │
│  ┌─────▼──────┐ │
│  │ Packet     │ │
│  │ Capture    │ │
│  └─────┬──────┘ │
│        │        │
│  ┌─────▼──────┐ │
│  │ ELRS       │ │
│  │ Decoder    │ │
│  └─────┬──────┘ │
│        │        │
│  ┌─────▼──────┐ │
│  │ Web Server │◄├─── Wi-Fi ───► Browser
│  │ WebSocket  │ │
│  └────────────┘ │
└─────────────────┘
```

### Packet Flow

1. SX1280 receives LoRa packet → triggers DIO1 interrupt
2. Main loop reads packet from SX1280 FIFO buffer
3. Packet stored in circular buffer with metadata (RSSI, SNR, timestamp)
4. ELRS decoder attempts to parse CRSF structure and extract channels
5. Web server broadcasts packet to all connected WebSocket clients
6. Browser displays packet in real-time table

## Legal & Regulatory

- **Educational Use Only**: This tool is for learning purposes
- **No Transmission**: This firmware only receives, never transmits
- **Local Regulations**: Ensure compliance with your local radio regulations
- **Privacy**: Do not monitor communications without authorization

## License

MIT License (Educational Use Only)

## Credits

- ExpressLRS Team for the open-source protocol
- RadioMaster for the RP2 V2 hardware
- Semtech for the SX1280 RF chip

## Support

This is an educational project. For issues:
1. Check serial debug output (115200 baud)
2. Verify hardware connections
3. Review configuration in `src/config.h`

---

**Remember: Use responsibly and ethically. Happy learning! 📚🔬**
