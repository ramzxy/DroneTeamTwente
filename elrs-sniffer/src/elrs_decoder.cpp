/**
 * ELRS Packet Sniffer - ELRS/CRSF Packet Decoder Implementation
 */

#include "elrs_decoder.h"

ELRSDecoder::ELRSDecoder() {}

uint8_t ELRSDecoder::calculateCRC8(const uint8_t *data, uint8_t length) {
  // CRC8 with polynomial 0xD5
  uint8_t crc = 0;
  for (uint8_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0xD5;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

bool ELRSDecoder::validateCRC(const uint8_t *packet, uint8_t length) {
  if (length < 3) {
    return false; // Too short to have CRC
  }

  // CRSF packet format: [sync] [len] [type] [payload...] [crc8]
  // CRC is calculated over [type] [payload...]
  uint8_t crcLength = length - 2; // Exclude sync and length bytes
  uint8_t calculatedCRC = calculateCRC8(&packet[2], crcLength - 1);
  uint8_t packetCRC = packet[length - 1];

  return (calculatedCRC == packetCRC);
}

bool ELRSDecoder::decodePacket(const uint8_t *packet, uint8_t length,
                               ChannelData &channels) {
  channels.valid = false;

  // Minimum packet: [sync] [len] [type] [crc] = 4 bytes
  if (length < 4) {
    return false;
  }

  // Check sync byte (may vary, so we'll be lenient)
  // uint8_t sync = packet[0];

  // Get length field
  uint8_t payloadLength = packet[1];

  // Get frame type
  uint8_t frameType = packet[2];

  // Validate CRC
  if (!validateCRC(packet, length)) {
    DEBUG_PRINTLN("CRC validation failed");
    return false;
  }

  // Check if this is a channel data packet
  if (frameType == CRSF_FRAMETYPE_RC_CHANNELS_PACKED) {
    // Extract channel data from payload
    // Payload starts at packet[3]
    if (extractChannels(&packet[3], channels)) {
      channels.valid = true;
      return true;
    }
  }

  return false;
}

bool ELRSDecoder::extractChannels(const uint8_t *payload,
                                  ChannelData &channels) {
  // CRSF RC channels are packed as 11-bit values
  // 16 channels * 11 bits = 176 bits = 22 bytes

  // Unpack 11-bit values from byte array
  uint16_t bitIndex = 0;
  for (uint8_t ch = 0; ch < CRSF_NUM_CHANNELS; ch++) {
    uint16_t value = 0;

    // Extract 11 bits starting at bitIndex
    uint8_t byteIndex = bitIndex / 8;
    uint8_t bitOffset = bitIndex % 8;

    // Read bits across byte boundaries
    value = payload[byteIndex] >> bitOffset;
    if (bitOffset > 5) { // Need bits from next byte
      value |= (uint16_t)payload[byteIndex + 1] << (8 - bitOffset);
    }
    if (bitOffset > 5 ||
        (bitOffset == 5 && ch < 15)) { // Need bits from third byte
      value |= (uint16_t)payload[byteIndex + 2] << (16 - bitOffset);
    }

    // Mask to 11 bits
    value &= 0x07FF;

    channels.channels[ch] = value;
    bitIndex += 11;
  }

  return true;
}

uint16_t ELRSDecoder::channelToMicroseconds(uint16_t channelValue) {
  // Convert 11-bit CRSF value (0-2047) to microseconds (988-2012)
  // CRSF range: 0-2047, center at 1024
  // PWM range: 988-2012 μs, center at 1500 μs
  return map(channelValue, 0, 2047, 988, 2012);
}

void ELRSDecoder::printPacketHex(const uint8_t *packet, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    if (packet[i] < 0x10)
      Serial.print("0");
    Serial.print(packet[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
