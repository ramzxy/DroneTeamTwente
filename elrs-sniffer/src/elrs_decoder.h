/**
 * ELRS Packet Sniffer - ELRS/CRSF Packet Decoder Header
 *
 * Decodes CRSF packet structure and extracts channel data
 */

#ifndef ELRS_DECODER_H
#define ELRS_DECODER_H

#include "config.h"
#include <Arduino.h>

// CRSF Frame Types
#define CRSF_FRAMETYPE_GPS 0x02
#define CRSF_FRAMETYPE_BATTERY_SENSOR 0x08
#define CRSF_FRAMETYPE_LINK_STATISTICS 0x14
#define CRSF_FRAMETYPE_RC_CHANNELS_PACKED 0x16
#define CRSF_FRAMETYPE_ATTITUDE 0x1E
#define CRSF_FRAMETYPE_FLIGHT_MODE 0x21

// CRSF Constants
#define CRSF_SYNC_BYTE 0xC8
#define CRSF_MAX_PACKET_SIZE 64
#define CRSF_NUM_CHANNELS 16

// Decoded channel data structure
struct ChannelData {
  uint16_t channels[CRSF_NUM_CHANNELS]; // 11-bit channel values (0-2047)
  bool valid;
};

class ELRSDecoder {
public:
  ELRSDecoder();

  // Packet validation
  bool validateCRC(const uint8_t *packet, uint8_t length);

  // Packet decoding
  bool decodePacket(const uint8_t *packet, uint8_t length,
                    ChannelData &channels);

  // Channel extraction
  bool extractChannels(const uint8_t *payload, ChannelData &channels);

  // Utility functions
  static uint16_t channelToMicroseconds(uint16_t channelValue);
  static void printPacketHex(const uint8_t *packet, uint8_t length);

private:
  uint8_t calculateCRC8(const uint8_t *data, uint8_t length);
};

#endif // ELRS_DECODER_H
