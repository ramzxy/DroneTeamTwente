/**
 * ELRS Packet Sniffer - Configuration Header
 * 
 * Hardware: RadioMaster RP2 V2 Nano Receiver
 * MCU: ESP8285
 * RF Chip: SX1280/SX1281
 * 
 * WARNING: Educational use only. Do not use for unauthorized monitoring.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// HARDWARE PIN DEFINITIONS (RP2 V2 - ESP8285 to SX1280)
// ============================================================================
// Note: These are standard ESP8285 SPI pins. If your RP2 V2 uses different
// pins, update these definitions accordingly.

// SPI Pins
#define PIN_SPI_MISO    12  // GPIO12 - Master In Slave Out
#define PIN_SPI_MOSI    13  // GPIO13 - Master Out Slave In
#define PIN_SPI_SCK     14  // GPIO14 - SPI Clock
#define PIN_SPI_NSS     15  // GPIO15 - Chip Select (NSS/CS)

// SX1280 Control Pins
#define PIN_RADIO_RST   4   // GPIO4  - Radio Reset
#define PIN_RADIO_BUSY  5   // GPIO5  - Radio Busy
#define PIN_RADIO_DIO1  2   // GPIO2  - Digital I/O 1 (Interrupt)

// Status LED
#define PIN_LED         16  // GPIO16 - Status LED (if available)

// ============================================================================
// WI-FI ACCESS POINT CONFIGURATION
// ============================================================================
#define WIFI_AP_SSID        "ELRS_Sniffer"
#define WIFI_AP_PASSWORD    "sniffer123"
#define WIFI_AP_CHANNEL     6
#define WIFI_AP_HIDDEN      false
#define WIFI_AP_MAX_CONN    4

// Static IP configuration
#define WIFI_AP_IP          IPAddress(192, 168, 4, 1)
#define WIFI_AP_GATEWAY     IPAddress(192, 168, 4, 1)
#define WIFI_AP_SUBNET      IPAddress(255, 255, 255, 0)

// ============================================================================
// ELRS/LORA RADIO CONFIGURATION
// ============================================================================
// Configure these to match the ELRS system you want to monitor

// Frequency (2.4 GHz band)
#define ELRS_FREQUENCY      2440000000UL  // 2440 MHz (common ELRS frequency)

// LoRa Parameters for 250Hz mode (adjust as needed)
#define LORA_SPREADING_FACTOR   6         // SF6
#define LORA_BANDWIDTH          LORA_BW_0800  // 800 kHz
#define LORA_CODING_RATE        LORA_CR_LI_4_6  // 4/6
#define LORA_PREAMBLE_LENGTH    12

// Packet Rate (for reference - affects timing)
#define ELRS_PACKET_RATE        250       // 250 Hz

// ============================================================================
// PACKET BUFFER CONFIGURATION
// ============================================================================
#define PACKET_BUFFER_SIZE      256       // Number of packets to store
#define MAX_PACKET_SIZE         64        // Maximum ELRS packet size (bytes)

// ============================================================================
// WEB SERVER CONFIGURATION
// ============================================================================
#define WEB_SERVER_PORT         80
#define WEBSOCKET_PATH          "/ws"

// ============================================================================
// DEBUGGING
// ============================================================================
#define DEBUG_SERIAL            true
#define DEBUG_BAUD_RATE         115200

// Debug macros
#if DEBUG_SERIAL
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

// ============================================================================
// SX1280 REGISTER DEFINITIONS (subset needed for sniffer)
// ============================================================================
// LoRa Bandwidth values
#define LORA_BW_0200    0x34
#define LORA_BW_0400    0x26
#define LORA_BW_0800    0x18
#define LORA_BW_1600    0x0A

// LoRa Coding Rate values
#define LORA_CR_4_5     0x01
#define LORA_CR_4_6     0x02
#define LORA_CR_4_7     0x03
#define LORA_CR_4_8     0x04
#define LORA_CR_LI_4_5  0x05
#define LORA_CR_LI_4_6  0x06
#define LORA_CR_LI_4_7  0x07
#define LORA_CR_LI_4_8  0x08

// ============================================================================
// EDUCATIONAL DISCLAIMER
// ============================================================================
#define DISCLAIMER_TEXT \
  "EDUCATIONAL USE ONLY - This device is for learning about radio protocols. " \
  "Do not use for unauthorized monitoring or transmission."

#endif // CONFIG_H
