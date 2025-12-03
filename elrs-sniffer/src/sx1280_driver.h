/**
 * ELRS Packet Sniffer - SX1280 Radio Driver Header
 *
 * Provides low-level SPI communication and configuration for SX1280/SX1281
 * RF chip in promiscuous/sniffer mode.
 */

#ifndef SX1280_DRIVER_H
#define SX1280_DRIVER_H

#include "config.h"
#include <Arduino.h>
#include <SPI.h>

// SX1280 Commands
#define SX1280_CMD_GET_STATUS 0xC0
#define SX1280_CMD_WRITE_REGISTER 0x18
#define SX1280_CMD_READ_REGISTER 0x19
#define SX1280_CMD_WRITE_BUFFER 0x1A
#define SX1280_CMD_READ_BUFFER 0x1B
#define SX1280_CMD_SET_SLEEP 0x84
#define SX1280_CMD_SET_STANDBY 0x80
#define SX1280_CMD_SET_FS 0xC1
#define SX1280_CMD_SET_TX 0x83
#define SX1280_CMD_SET_RX 0x82
#define SX1280_CMD_SET_RXDUTYCYCLE 0x94
#define SX1280_CMD_SET_CAD 0xC5
#define SX1280_CMD_SET_TXCONTINUOUSWAVE 0xD1
#define SX1280_CMD_SET_TXCONTINUOUSPREAMBLE 0xD2
#define SX1280_CMD_SET_PACKET_TYPE 0x8A
#define SX1280_CMD_GET_PACKET_TYPE 0x03
#define SX1280_CMD_SET_RF_FREQUENCY 0x86
#define SX1280_CMD_SET_TX_PARAMS 0x8E
#define SX1280_CMD_SET_CADPARAMS 0x88
#define SX1280_CMD_SET_BUFFER_BASE_ADDRESS 0x8F
#define SX1280_CMD_SET_MODULATION_PARAMS 0x8B
#define SX1280_CMD_SET_PACKET_PARAMS 0x8C
#define SX1280_CMD_GET_RX_BUFFER_STATUS 0x17
#define SX1280_CMD_GET_PACKET_STATUS 0x1D
#define SX1280_CMD_GET_RSSI_INST 0x1F
#define SX1280_CMD_SET_DIO_IRQ_PARAMS 0x8D
#define SX1280_CMD_GET_IRQ_STATUS 0x15
#define SX1280_CMD_CLR_IRQ_STATUS 0x97
#define SX1280_CMD_SET_REGULATOR_MODE 0x96
#define SX1280_CMD_SET_SAVE_CONTEXT 0xD5
#define SX1280_CMD_SET_AUTO_TX 0x98
#define SX1280_CMD_SET_AUTO_FS 0x9E
#define SX1280_CMD_SET_LONGPREAMBLE 0x9B
#define SX1280_CMD_SET_RANGING_ROLE 0xA3

// Packet Types
#define SX1280_PACKET_TYPE_GFSK 0x00
#define SX1280_PACKET_TYPE_LORA 0x01
#define SX1280_PACKET_TYPE_RANGING 0x02
#define SX1280_PACKET_TYPE_FLRC 0x03
#define SX1280_PACKET_TYPE_BLE 0x04

// IRQ Masks
#define SX1280_IRQ_TX_DONE 0x0001
#define SX1280_IRQ_RX_DONE 0x0002
#define SX1280_IRQ_SYNCWORD_VALID 0x0004
#define SX1280_IRQ_SYNCWORD_ERROR 0x0008
#define SX1280_IRQ_HEADER_VALID 0x0010
#define SX1280_IRQ_HEADER_ERROR 0x0020
#define SX1280_IRQ_CRC_ERROR 0x0040
#define SX1280_IRQ_RANGING_SLAVE_RESPONSE_DONE 0x0080
#define SX1280_IRQ_RANGING_SLAVE_REQUEST_DISCARDED 0x0100
#define SX1280_IRQ_RANGING_MASTER_RESULT_VALID 0x0200
#define SX1280_IRQ_RANGING_MASTER_TIMEOUT 0x0400
#define SX1280_IRQ_RANGING_SLAVE_REQUEST_VALID 0x0800
#define SX1280_IRQ_CAD_DONE 0x1000
#define SX1280_IRQ_CAD_DETECTED 0x2000
#define SX1280_IRQ_RX_TX_TIMEOUT 0x4000
#define SX1280_IRQ_PREAMBLE_DETECTED 0x8000

// Standby Modes
#define SX1280_STANDBY_RC 0x00
#define SX1280_STANDBY_XOSC 0x01

// Regulator Modes
#define SX1280_REGULATOR_LDO 0x00
#define SX1280_REGULATOR_DC_DC 0x01

class SX1280Driver {
public:
  SX1280Driver();

  // Initialization
  bool begin();
  void reset();

  // Configuration
  bool setStandby(uint8_t mode = SX1280_STANDBY_RC);
  bool setPacketType(uint8_t packetType);
  bool setRfFrequency(uint32_t frequency);
  bool setModulationParams(uint8_t sf, uint8_t bw, uint8_t cr);
  bool setPacketParams(uint16_t preambleLength, uint8_t headerType,
                       uint8_t payloadLength, uint8_t crc, uint8_t invertIQ);
  bool setDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask,
                       uint16_t dio3Mask);
  bool setBufferBaseAddress(uint8_t txBaseAddress, uint8_t rxBaseAddress);

  // Promiscuous mode configuration
  bool enablePromiscuousMode();

  // RX Operations
  bool setRx(uint16_t timeout); // timeout in ms, 0xFFFF = continuous
  bool getRxBufferStatus(uint8_t *payloadLength, uint8_t *rxStartBufferPointer);
  bool readBuffer(uint8_t offset, uint8_t *buffer, uint8_t length);

  // Status and Metadata
  uint16_t getIrqStatus();
  bool clearIrqStatus(uint16_t irqMask);
  bool getPacketStatus(int8_t *rssi, int8_t *snr);
  uint8_t getStatus();

private:
  // SPI Communication
  void spiWrite(uint8_t *data, uint8_t length);
  void spiRead(uint8_t *data, uint8_t length);
  void spiTransfer(uint8_t *dataOut, uint8_t *dataIn, uint8_t length);

  // Helper functions
  void waitOnBusy();
  void chipSelect();
  void chipDeselect();

  // State
  bool _initialized;
};

#endif // SX1280_DRIVER_H
