/**
 * ELRS Packet Sniffer - Packet Capture System Header
 *
 * Manages packet capture, circular buffer, and interrupt handling
 */

#ifndef PACKET_CAPTURE_H
#define PACKET_CAPTURE_H

#include "config.h"
#include <Arduino.h>

// Packet metadata structure
struct PacketData {
  uint32_t timestamp;            // Milliseconds since boot
  int8_t rssi;                   // RSSI in dBm
  int8_t snr;                    // SNR in dB
  uint8_t length;                // Packet length in bytes
  uint8_t data[MAX_PACKET_SIZE]; // Raw packet data
};

class PacketCapture {
public:
  PacketCapture();

  // Initialization
  void begin();

  // Packet storage
  bool addPacket(const PacketData &packet);
  bool getPacket(uint16_t index, PacketData &packet);
  uint16_t getPacketCount();
  void clearBuffer();

  // Statistics
  uint32_t getTotalPacketsReceived();
  uint32_t getTotalPacketsDropped();

  // Interrupt handler (call from ISR)
  void IRAM_ATTR handlePacketReceived();

private:
  PacketData _buffer[PACKET_BUFFER_SIZE];
  volatile uint16_t _writeIndex;
  volatile uint16_t _readIndex;
  volatile uint16_t _count;
  volatile uint32_t _totalReceived;
  volatile uint32_t _totalDropped;
  volatile bool _bufferFull;
};

// Global instance for ISR access
extern PacketCapture packetCapture;

#endif // PACKET_CAPTURE_H
