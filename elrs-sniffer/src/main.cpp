/**
 * ELRS Packet Sniffer - Main Application
 *
 * Educational tool for analyzing ExpressLRS over-the-air packets
 *
 * Hardware: RadioMaster RP2 V2 Nano Receiver
 * MCU: ESP8285
 * RF Chip: SX1280/SX1281
 *
 * ⚠️ WARNING: EDUCATIONAL USE ONLY
 * This firmware is designed strictly for educational purposes to understand
 * radio protocols. Do NOT use for unauthorized monitoring or transmission.
 *
 * Author: Generated for educational purposes
 * License: MIT (Educational Use Only)
 */

#include "config.h"
#include "elrs_decoder.h"
#include "packet_capture.h"
#include "sx1280_driver.h"
#include "web_server.h"
#include <Arduino.h>

// Global instances
SX1280Driver radio;
PacketCapture packetCapture;
ELRSDecoder elrsDecoder;
WebServerManager webServer;

// State variables
volatile bool packetReady = false;
unsigned long lastStatusPrint = 0;
unsigned long lastPacketCheck = 0;

// ISR for packet reception
void IRAM_ATTR onPacketReceived() { packetReady = true; }

void setup() {
// Initialize serial for debugging
#if DEBUG_SERIAL
  Serial.begin(DEBUG_BAUD_RATE);
  delay(100);
  Serial.println();
  Serial.println("========================================");
  Serial.println("   ELRS Packet Sniffer v1.0");
  Serial.println("   Educational Use Only");
  Serial.println("========================================");
  Serial.println();
#endif

  // Print disclaimer
  DEBUG_PRINTLN(DISCLAIMER_TEXT);
  DEBUG_PRINTLN();

  // Initialize LED
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH); // LED on during init

  // Initialize packet capture system
  DEBUG_PRINTLN("Initializing packet capture system...");
  packetCapture.begin();

  // Initialize SX1280 radio
  DEBUG_PRINTLN("Initializing SX1280 radio...");
  if (!radio.begin()) {
    DEBUG_PRINTLN("ERROR: Failed to initialize SX1280!");
    DEBUG_PRINTLN("Check hardware connections and pin definitions.");
    while (1) {
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
      delay(200); // Fast blink = error
    }
  }

  // Configure radio for promiscuous mode
  DEBUG_PRINTLN("Configuring promiscuous mode...");
  if (!radio.enablePromiscuousMode()) {
    DEBUG_PRINTLN("ERROR: Failed to configure promiscuous mode!");
    while (1) {
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
      delay(200);
    }
  }

  // Set up interrupt for packet reception
  pinMode(PIN_RADIO_DIO1, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_RADIO_DIO1), onPacketReceived,
                  RISING);

  // Start continuous RX mode
  DEBUG_PRINTLN("Starting continuous RX mode...");
  radio.setRx(0xFFFF); // Continuous mode

  // Initialize web server
  DEBUG_PRINTLN("Starting web server...");
  if (!webServer.begin()) {
    DEBUG_PRINTLN("ERROR: Failed to start web server!");
    while (1) {
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
      delay(500);
    }
  }

  digitalWrite(PIN_LED, LOW); // LED off = ready

  DEBUG_PRINTLN();
  DEBUG_PRINTLN("========================================");
  DEBUG_PRINTLN("Sniffer ready!");
  DEBUG_PRINTLN("Connect to Wi-Fi: " WIFI_AP_SSID);
  DEBUG_PRINTLN("Password: " WIFI_AP_PASSWORD);
  DEBUG_PRINT("Web interface: http://");
  DEBUG_PRINTLN(WiFi.softAPIP());
  DEBUG_PRINTLN("========================================");
  DEBUG_PRINTLN();
}

void loop() {
  // Handle web server clients
  webServer.handleClient();

  // Check for received packets
  if (packetReady || (millis() - lastPacketCheck > 10)) {
    lastPacketCheck = millis();

    // Check IRQ status
    uint16_t irqStatus = radio.getIrqStatus();

    if (irqStatus & SX1280_IRQ_RX_DONE) {
      // Packet received!
      digitalWrite(PIN_LED, HIGH); // LED on during processing

      PacketData packet;
      packet.timestamp = millis();

      // Get RSSI and SNR
      radio.getPacketStatus(&packet.rssi, &packet.snr);

      // Get packet from buffer
      uint8_t rxStartPtr;
      radio.getRxBufferStatus(&packet.length, &rxStartPtr);

      if (packet.length > 0 && packet.length <= MAX_PACKET_SIZE) {
        radio.readBuffer(rxStartPtr, packet.data, packet.length);

        // Add to capture buffer
        packetCapture.addPacket(packet);

        // Try to decode as ELRS/CRSF packet
        ChannelData channels;
        bool decoded =
            elrsDecoder.decodePacket(packet.data, packet.length, channels);

        // Broadcast to web clients
        webServer.broadcastPacket(packet, channels);

// Debug output
#if DEBUG_SERIAL
        DEBUG_PRINTF("[%lu] RSSI: %d dBm, SNR: %d dB, Len: %d, Decoded: %s\n",
                     packet.timestamp, packet.rssi, packet.snr, packet.length,
                     decoded ? "YES" : "NO");

        if (decoded && channels.valid) {
          DEBUG_PRINTF("  Channels: [%d, %d, %d, %d]\n", channels.channels[0],
                       channels.channels[1], channels.channels[2],
                       channels.channels[3]);
        }
#endif
      }

      // Clear IRQ and restart RX
      radio.clearIrqStatus(SX1280_IRQ_RX_DONE | SX1280_IRQ_CRC_ERROR);
      radio.setRx(0xFFFF); // Restart continuous RX

      digitalWrite(PIN_LED, LOW);
      packetReady = false;
    }

    // Handle CRC errors (still capture for analysis)
    if (irqStatus & SX1280_IRQ_CRC_ERROR) {
      DEBUG_PRINTLN("CRC Error detected");
      radio.clearIrqStatus(SX1280_IRQ_CRC_ERROR);
      radio.setRx(0xFFFF);
    }
  }

  // Print status every 10 seconds
  if (millis() - lastStatusPrint > 10000) {
    lastStatusPrint = millis();

    DEBUG_PRINTLN("--- Status ---");
    DEBUG_PRINTF("Total packets: %lu\n",
                 packetCapture.getTotalPacketsReceived());
    DEBUG_PRINTF("Dropped: %lu\n", packetCapture.getTotalPacketsDropped());
    DEBUG_PRINTF("Buffer: %d/%d\n", packetCapture.getPacketCount(),
                 PACKET_BUFFER_SIZE);
    DEBUG_PRINTF("Web clients: %d\n", webServer.getConnectedClients());
    DEBUG_PRINTLN();
  }

  // Small delay to prevent watchdog issues
  delay(1);
}
