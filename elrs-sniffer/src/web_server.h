/**
 * ELRS Packet Sniffer - Web Server Header
 *
 * Manages Wi-Fi AP, HTTP server, and WebSocket for real-time packet streaming
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "config.h"
#include "elrs_decoder.h"
#include "packet_capture.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

class WebServerManager {
public:
  WebServerManager();

  // Initialization
  bool begin();

  // Server management
  void handleClient();
  void broadcastPacket(const PacketData &packet, const ChannelData &channels);

  // Statistics
  uint16_t getConnectedClients();

private:
  AsyncWebServer *_server;
  AsyncWebSocket *_ws;

  // HTTP handlers
  static void handleRoot(AsyncWebServerRequest *request);
  static void handleLog(AsyncWebServerRequest *request);
  static void handleStats(AsyncWebServerRequest *request);
  static void handleNotFound(AsyncWebServerRequest *request);

  // WebSocket handlers
  static void onWebSocketEvent(AsyncWebSocket *server,
                               AsyncWebSocketClient *client, AwsEventType type,
                               void *arg, uint8_t *data, size_t len);

  // Helper functions
  String generateCSV();
  String generateStatsJSON();
};

#endif // WEB_SERVER_H
