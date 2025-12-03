/**
 * ELRS Packet Sniffer - Packet Capture System Implementation
 */

#include "packet_capture.h"

PacketCapture::PacketCapture()
    : _writeIndex(0), _readIndex(0), _count(0), _totalReceived(0),
      _totalDropped(0), _bufferFull(false) {}

void PacketCapture::begin() {
  clearBuffer();
  DEBUG_PRINTLN("Packet capture system initialized");
}

bool PacketCapture::addPacket(const PacketData &packet) {
  // Check if buffer is full
  if (_count >= PACKET_BUFFER_SIZE) {
    _totalDropped++;
    _bufferFull = true;

    // Overwrite oldest packet (circular buffer behavior)
    _readIndex = (_readIndex + 1) % PACKET_BUFFER_SIZE;
    _count--;
  }

  // Add packet to buffer
  memcpy(&_buffer[_writeIndex], &packet, sizeof(PacketData));
  _writeIndex = (_writeIndex + 1) % PACKET_BUFFER_SIZE;
  _count++;
  _totalReceived++;

  return true;
}

bool PacketCapture::getPacket(uint16_t index, PacketData &packet) {
  if (index >= _count) {
    return false;
  }

  // Calculate actual buffer position
  uint16_t bufferIndex = (_readIndex + index) % PACKET_BUFFER_SIZE;
  memcpy(&packet, &_buffer[bufferIndex], sizeof(PacketData));

  return true;
}

uint16_t PacketCapture::getPacketCount() { return _count; }

void PacketCapture::clearBuffer() {
  _writeIndex = 0;
  _readIndex = 0;
  _count = 0;
  _bufferFull = false;
  // Note: We don't reset total counters
}

uint32_t PacketCapture::getTotalPacketsReceived() { return _totalReceived; }

uint32_t PacketCapture::getTotalPacketsDropped() { return _totalDropped; }

void IRAM_ATTR PacketCapture::handlePacketReceived() {
  // This is called from ISR - keep it minimal
  // Actual packet reading happens in main loop
}
