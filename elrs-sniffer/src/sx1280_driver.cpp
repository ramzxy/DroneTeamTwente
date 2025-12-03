/**
 * ELRS Packet Sniffer - SX1280 Radio Driver Implementation
 */

#include "sx1280_driver.h"

SX1280Driver::SX1280Driver() : _initialized(false) {}

bool SX1280Driver::begin() {
  // Initialize SPI
  SPI.begin();
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));

  // Setup pins
  pinMode(PIN_SPI_NSS, OUTPUT);
  pinMode(PIN_RADIO_RST, OUTPUT);
  pinMode(PIN_RADIO_BUSY, INPUT);
  pinMode(PIN_RADIO_DIO1, INPUT);

  chipDeselect();

  // Reset the radio
  reset();
  delay(50);

  // Set standby mode
  if (!setStandby(SX1280_STANDBY_RC)) {
    DEBUG_PRINTLN("Failed to set standby mode");
    return false;
  }

  _initialized = true;
  DEBUG_PRINTLN("SX1280 initialized successfully");
  return true;
}

void SX1280Driver::reset() {
  digitalWrite(PIN_RADIO_RST, LOW);
  delay(10);
  digitalWrite(PIN_RADIO_RST, HIGH);
  delay(20);
  waitOnBusy();
}

bool SX1280Driver::setStandby(uint8_t mode) {
  waitOnBusy();
  chipSelect();
  uint8_t cmd[] = {SX1280_CMD_SET_STANDBY, mode};
  spiWrite(cmd, 2);
  chipDeselect();
  waitOnBusy();
  return true;
}

bool SX1280Driver::setPacketType(uint8_t packetType) {
  waitOnBusy();
  chipSelect();
  uint8_t cmd[] = {SX1280_CMD_SET_PACKET_TYPE, packetType};
  spiWrite(cmd, 2);
  chipDeselect();
  waitOnBusy();
  return true;
}

bool SX1280Driver::setRfFrequency(uint32_t frequency) {
  // Convert frequency to register value
  // freq_reg = (freq_hz * 2^18) / 52000000
  uint32_t freqReg = ((uint64_t)frequency << 18) / 52000000UL;

  waitOnBusy();
  chipSelect();
  uint8_t cmd[] = {SX1280_CMD_SET_RF_FREQUENCY,
                   (uint8_t)((freqReg >> 16) & 0xFF),
                   (uint8_t)((freqReg >> 8) & 0xFF), (uint8_t)(freqReg & 0xFF)};
  spiWrite(cmd, 4);
  chipDeselect();
  waitOnBusy();
  return true;
}

bool SX1280Driver::setModulationParams(uint8_t sf, uint8_t bw, uint8_t cr) {
  waitOnBusy();
  chipSelect();
  uint8_t cmd[] = {
      SX1280_CMD_SET_MODULATION_PARAMS,
      sf, // Spreading Factor
      bw, // Bandwidth
      cr  // Coding Rate
  };
  spiWrite(cmd, 4);
  chipDeselect();
  waitOnBusy();
  return true;
}

bool SX1280Driver::setPacketParams(uint16_t preambleLength, uint8_t headerType,
                                   uint8_t payloadLength, uint8_t crc,
                                   uint8_t invertIQ) {
  waitOnBusy();
  chipSelect();
  uint8_t cmd[] = {
      SX1280_CMD_SET_PACKET_PARAMS,
      (uint8_t)((preambleLength >> 8) & 0xFF),
      (uint8_t)(preambleLength & 0xFF),
      headerType,
      payloadLength,
      crc,
      invertIQ,
      0x00,
      0x00,
      0x00 // Reserved
  };
  spiWrite(cmd, 10);
  chipDeselect();
  waitOnBusy();
  return true;
}

bool SX1280Driver::setDioIrqParams(uint16_t irqMask, uint16_t dio1Mask,
                                   uint16_t dio2Mask, uint16_t dio3Mask) {
  waitOnBusy();
  chipSelect();
  uint8_t cmd[] = {
      SX1280_CMD_SET_DIO_IRQ_PARAMS, (uint8_t)((irqMask >> 8) & 0xFF),
      (uint8_t)(irqMask & 0xFF),     (uint8_t)((dio1Mask >> 8) & 0xFF),
      (uint8_t)(dio1Mask & 0xFF),    (uint8_t)((dio2Mask >> 8) & 0xFF),
      (uint8_t)(dio2Mask & 0xFF),    (uint8_t)((dio3Mask >> 8) & 0xFF),
      (uint8_t)(dio3Mask & 0xFF)};
  spiWrite(cmd, 9);
  chipDeselect();
  waitOnBusy();
  return true;
}

bool SX1280Driver::setBufferBaseAddress(uint8_t txBaseAddress,
                                        uint8_t rxBaseAddress) {
  waitOnBusy();
  chipSelect();
  uint8_t cmd[] = {SX1280_CMD_SET_BUFFER_BASE_ADDRESS, txBaseAddress,
                   rxBaseAddress};
  spiWrite(cmd, 3);
  chipDeselect();
  waitOnBusy();
  return true;
}

bool SX1280Driver::enablePromiscuousMode() {
  DEBUG_PRINTLN("Configuring SX1280 for promiscuous mode...");

  // Set packet type to LoRa
  if (!setPacketType(SX1280_PACKET_TYPE_LORA)) {
    DEBUG_PRINTLN("Failed to set packet type");
    return false;
  }

  // Set RF frequency
  if (!setRfFrequency(ELRS_FREQUENCY)) {
    DEBUG_PRINTLN("Failed to set frequency");
    return false;
  }

  // Set modulation parameters
  if (!setModulationParams(LORA_SPREADING_FACTOR, LORA_BANDWIDTH,
                           LORA_CODING_RATE)) {
    DEBUG_PRINTLN("Failed to set modulation params");
    return false;
  }

  // Set packet parameters for promiscuous mode
  // Header type: 0x00 = explicit (variable length)
  // Payload length: 0xFF = maximum
  // CRC: 0x00 = disabled (accept packets with bad CRC for sniffer mode)
  // InvertIQ: 0x00 = standard
  if (!setPacketParams(LORA_PREAMBLE_LENGTH, 0x00, 0xFF, 0x00, 0x00)) {
    DEBUG_PRINTLN("Failed to set packet params");
    return false;
  }

  // Set buffer base addresses
  if (!setBufferBaseAddress(0x00, 0x00)) {
    DEBUG_PRINTLN("Failed to set buffer base");
    return false;
  }

  // Configure interrupts - enable RX_DONE and optionally CRC_ERROR
  uint16_t irqMask =
      SX1280_IRQ_RX_DONE | SX1280_IRQ_CRC_ERROR | SX1280_IRQ_HEADER_ERROR;
  if (!setDioIrqParams(irqMask, irqMask, 0x0000, 0x0000)) {
    DEBUG_PRINTLN("Failed to set IRQ params");
    return false;
  }

  DEBUG_PRINTLN("Promiscuous mode configured successfully");
  return true;
}

bool SX1280Driver::setRx(uint16_t timeout) {
  // timeout: 0xFFFF = continuous RX mode
  // timeout in units of 15.625 μs
  waitOnBusy();
  chipSelect();
  uint8_t cmd[] = {SX1280_CMD_SET_RX,
                   0x00, // Periodbase (not used in continuous mode)
                   (uint8_t)((timeout >> 8) & 0xFF), (uint8_t)(timeout & 0xFF)};
  spiWrite(cmd, 4);
  chipDeselect();
  waitOnBusy();
  return true;
}

bool SX1280Driver::getRxBufferStatus(uint8_t *payloadLength,
                                     uint8_t *rxStartBufferPointer) {
  waitOnBusy();
  chipSelect();
  uint8_t cmd = SX1280_CMD_GET_RX_BUFFER_STATUS;
  uint8_t response[3] = {0};
  spiWrite(&cmd, 1);
  spiRead(response, 3);
  chipDeselect();

  *payloadLength = response[1];
  *rxStartBufferPointer = response[2];
  return true;
}

bool SX1280Driver::readBuffer(uint8_t offset, uint8_t *buffer, uint8_t length) {
  waitOnBusy();
  chipSelect();
  uint8_t cmd[] = {SX1280_CMD_READ_BUFFER, offset};
  spiWrite(cmd, 2);
  spiRead(buffer, length);
  chipDeselect();
  return true;
}

uint16_t SX1280Driver::getIrqStatus() {
  waitOnBusy();
  chipSelect();
  uint8_t cmd = SX1280_CMD_GET_IRQ_STATUS;
  uint8_t response[3] = {0};
  spiWrite(&cmd, 1);
  spiRead(response, 3);
  chipDeselect();

  return ((uint16_t)response[1] << 8) | response[2];
}

bool SX1280Driver::clearIrqStatus(uint16_t irqMask) {
  waitOnBusy();
  chipSelect();
  uint8_t cmd[] = {SX1280_CMD_CLR_IRQ_STATUS, (uint8_t)((irqMask >> 8) & 0xFF),
                   (uint8_t)(irqMask & 0xFF)};
  spiWrite(cmd, 3);
  chipDeselect();
  waitOnBusy();
  return true;
}

bool SX1280Driver::getPacketStatus(int8_t *rssi, int8_t *snr) {
  waitOnBusy();
  chipSelect();
  uint8_t cmd = SX1280_CMD_GET_PACKET_STATUS;
  uint8_t response[6] = {0};
  spiWrite(&cmd, 1);
  spiRead(response, 6);
  chipDeselect();

  // For LoRa packets:
  // response[1] = RFU
  // response[2] = RSSI (raw value, needs conversion)
  // response[3] = SNR (raw value, needs conversion)

  *rssi = -response[2] / 2;       // Convert to dBm
  *snr = (int8_t)response[3] / 4; // Convert to dB

  return true;
}

uint8_t SX1280Driver::getStatus() {
  waitOnBusy();
  chipSelect();
  uint8_t cmd = SX1280_CMD_GET_STATUS;
  uint8_t status = 0;
  spiWrite(&cmd, 1);
  spiRead(&status, 1);
  chipDeselect();
  return status;
}

// ============================================================================
// Private Helper Functions
// ============================================================================

void SX1280Driver::spiWrite(uint8_t *data, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    SPI.transfer(data[i]);
  }
}

void SX1280Driver::spiRead(uint8_t *data, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    data[i] = SPI.transfer(0x00);
  }
}

void SX1280Driver::spiTransfer(uint8_t *dataOut, uint8_t *dataIn,
                               uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    dataIn[i] = SPI.transfer(dataOut[i]);
  }
}

void SX1280Driver::waitOnBusy() {
  unsigned long timeout = millis() + 1000; // 1 second timeout
  while (digitalRead(PIN_RADIO_BUSY) == HIGH) {
    if (millis() > timeout) {
      DEBUG_PRINTLN("WARNING: BUSY timeout");
      break;
    }
    delayMicroseconds(10);
  }
}

void SX1280Driver::chipSelect() {
  digitalWrite(PIN_SPI_NSS, LOW);
  delayMicroseconds(1);
}

void SX1280Driver::chipDeselect() {
  delayMicroseconds(1);
  digitalWrite(PIN_SPI_NSS, HIGH);
}
