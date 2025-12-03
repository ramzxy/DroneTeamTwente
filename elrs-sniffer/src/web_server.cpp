/**
 * ELRS Packet Sniffer - Web Server Implementation
 */

#include "web_server.h"

// External references
extern PacketCapture packetCapture;
extern ELRSDecoder elrsDecoder;

// Static instance for callbacks
static WebServerManager *webServerInstance = nullptr;

WebServerManager::WebServerManager() {
  _server = nullptr;
  _ws = nullptr;
  webServerInstance = this;
}

bool WebServerManager::begin() {
  // Configure Wi-Fi Access Point
  DEBUG_PRINTLN("Starting Wi-Fi Access Point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(WIFI_AP_IP, WIFI_AP_GATEWAY, WIFI_AP_SUBNET);

  bool apStarted = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL,
                               WIFI_AP_HIDDEN, WIFI_AP_MAX_CONN);

  if (!apStarted) {
    DEBUG_PRINTLN("Failed to start Access Point");
    return false;
  }

  DEBUG_PRINT("AP IP address: ");
  DEBUG_PRINTLN(WiFi.softAPIP());

  // Create web server
  _server = new AsyncWebServer(WEB_SERVER_PORT);
  _ws = new AsyncWebSocket(WEBSOCKET_PATH);

  // Attach WebSocket to server
  _ws->onEvent(onWebSocketEvent);
  _server->addHandler(_ws);

  // Setup HTTP routes
  _server->on("/", HTTP_GET, handleRoot);
  _server->on("/log", HTTP_GET, handleLog);
  _server->on("/stats", HTTP_GET, handleStats);
  _server->onNotFound(handleNotFound);

  // Start server
  _server->begin();
  DEBUG_PRINTLN("Web server started");

  return true;
}

void WebServerManager::handleClient() { _ws->cleanupClients(); }

void WebServerManager::broadcastPacket(const PacketData &packet,
                                       const ChannelData &channels) {
  if (_ws->count() == 0)
    return; // No clients connected

  // Create JSON packet
  String json = "{";
  json += "\"ts\":" + String(packet.timestamp) + ",";
  json += "\"rssi\":" + String(packet.rssi) + ",";
  json += "\"snr\":" + String(packet.snr) + ",";
  json += "\"len\":" + String(packet.length) + ",";

  // Raw hex data
  json += "\"hex\":\"";
  for (uint8_t i = 0; i < packet.length; i++) {
    if (packet.data[i] < 0x10)
      json += "0";
    json += String(packet.data[i], HEX);
  }
  json += "\",";

  // Decoded channels (if valid)
  if (channels.valid) {
    json += "\"ch\":[";
    for (uint8_t i = 0; i < 8; i++) { // Send first 8 channels
      json += String(channels.channels[i]);
      if (i < 7)
        json += ",";
    }
    json += "]";
  } else {
    json += "\"ch\":null";
  }

  json += "}";

  _ws->textAll(json);
}

uint16_t WebServerManager::getConnectedClients() {
  return _ws ? _ws->count() : 0;
}

// ============================================================================
// HTTP Handlers
// ============================================================================

void WebServerManager::handleRoot(AsyncWebServerRequest *request) {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ELRS Packet Sniffer</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #fff;
            padding: 20px;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
        }
        .disclaimer {
            background: #ff6b6b;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 20px;
            text-align: center;
            font-weight: bold;
        }
        h1 {
            text-align: center;
            margin-bottom: 30px;
            font-size: 2.5em;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .stats {
            display: flex;
            gap: 20px;
            margin-bottom: 20px;
            flex-wrap: wrap;
        }
        .stat-box {
            background: rgba(255,255,255,0.1);
            backdrop-filter: blur(10px);
            padding: 20px;
            border-radius: 12px;
            flex: 1;
            min-width: 200px;
        }
        .stat-value {
            font-size: 2em;
            font-weight: bold;
            margin-top: 10px;
        }
        .controls {
            margin-bottom: 20px;
            display: flex;
            gap: 10px;
        }
        button {
            background: #4CAF50;
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 6px;
            cursor: pointer;
            font-size: 16px;
            transition: background 0.3s;
        }
        button:hover { background: #45a049; }
        button.danger { background: #f44336; }
        button.danger:hover { background: #da190b; }
        .table-container {
            background: rgba(255,255,255,0.95);
            border-radius: 12px;
            overflow: hidden;
            box-shadow: 0 8px 32px rgba(0,0,0,0.2);
        }
        table {
            width: 100%;
            border-collapse: collapse;
            color: #333;
        }
        th {
            background: #667eea;
            color: white;
            padding: 15px;
            text-align: left;
            font-weight: 600;
            position: sticky;
            top: 0;
        }
        td {
            padding: 12px 15px;
            border-bottom: 1px solid #ddd;
        }
        tr:hover { background: #f5f5f5; }
        .mono { font-family: 'Courier New', monospace; font-size: 0.9em; }
        .status {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }
        .status.connected { background: #4CAF50; }
        .status.disconnected { background: #f44336; }
    </style>
</head>
<body>
    <div class="container">
        <div class="disclaimer">
            ⚠️ EDUCATIONAL USE ONLY - This tool is for learning about radio protocols
        </div>
        
        <h1>📡 ELRS Packet Sniffer</h1>
        
        <div class="stats">
            <div class="stat-box">
                <div>WebSocket Status</div>
                <div class="stat-value">
                    <span class="status" id="wsStatus"></span>
                    <span id="wsText">Connecting...</span>
                </div>
            </div>
            <div class="stat-box">
                <div>Packets Received</div>
                <div class="stat-value" id="packetCount">0</div>
            </div>
            <div class="stat-box">
                <div>Packet Rate</div>
                <div class="stat-value" id="packetRate">0 Hz</div>
            </div>
        </div>
        
        <div class="controls">
            <button onclick="clearTable()">Clear Display</button>
            <button onclick="downloadCSV()">Download CSV</button>
            <button class="danger" onclick="location.reload()">Reload Page</button>
        </div>
        
        <div class="table-container">
            <table>
                <thead>
                    <tr>
                        <th>Time (ms)</th>
                        <th>RSSI (dBm)</th>
                        <th>SNR (dB)</th>
                        <th>Length</th>
                        <th>Raw Hex</th>
                        <th>Ch1</th>
                        <th>Ch2</th>
                        <th>Ch3</th>
                        <th>Ch4</th>
                    </tr>
                </thead>
                <tbody id="packetTable">
                    <tr><td colspan="9" style="text-align:center;">Waiting for packets...</td></tr>
                </tbody>
            </table>
        </div>
    </div>
    
    <script>
        let ws;
        let packets = [];
        let lastPacketTime = 0;
        let packetRateCounter = 0;
        
        function connectWebSocket() {
            ws = new WebSocket('ws://' + location.hostname + '/ws');
            
            ws.onopen = function() {
                document.getElementById('wsStatus').className = 'status connected';
                document.getElementById('wsText').textContent = 'Connected';
            };
            
            ws.onclose = function() {
                document.getElementById('wsStatus').className = 'status disconnected';
                document.getElementById('wsText').textContent = 'Disconnected';
                setTimeout(connectWebSocket, 2000);
            };
            
            ws.onmessage = function(event) {
                const packet = JSON.parse(event.data);
                packets.push(packet);
                addPacketToTable(packet);
                updateStats();
            };
        }
        
        function addPacketToTable(pkt) {
            const table = document.getElementById('packetTable');
            if (table.rows[0].cells[0].colSpan === 9) {
                table.innerHTML = '';
            }
            
            const row = table.insertRow(0);
            row.insertCell(0).textContent = pkt.ts;
            row.insertCell(1).textContent = pkt.rssi;
            row.insertCell(2).textContent = pkt.snr;
            row.insertCell(3).textContent = pkt.len;
            row.insertCell(4).innerHTML = '<span class="mono">' + pkt.hex + '</span>';
            
            if (pkt.ch) {
                for (let i = 0; i < 4; i++) {
                    row.insertCell(5 + i).textContent = pkt.ch[i];
                }
            } else {
                for (let i = 0; i < 4; i++) {
                    row.insertCell(5 + i).textContent = '-';
                }
            }
            
            if (table.rows.length > 100) {
                table.deleteRow(100);
            }
        }
        
        function updateStats() {
            document.getElementById('packetCount').textContent = packets.length;
            
            const now = Date.now();
            if (now - lastPacketTime < 1000) {
                packetRateCounter++;
            } else {
                document.getElementById('packetRate').textContent = packetRateCounter + ' Hz';
                packetRateCounter = 1;
                lastPacketTime = now;
            }
        }
        
        function clearTable() {
            document.getElementById('packetTable').innerHTML = 
                '<tr><td colspan="9" style="text-align:center;">Waiting for packets...</td></tr>';
            packets = [];
            updateStats();
        }
        
        function downloadCSV() {
            let csv = 'Timestamp,RSSI,SNR,Length,RawHex,Ch1,Ch2,Ch3,Ch4\n';
            packets.forEach(p => {
                csv += p.ts + ',' + p.rssi + ',' + p.snr + ',' + p.len + ',' + p.hex + ',';
                if (p.ch) {
                    csv += p.ch.slice(0, 4).join(',');
                } else {
                    csv += ',,,';
                }
                csv += '\n';
            });
            
            const blob = new Blob([csv], { type: 'text/csv' });
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'elrs_packets_' + Date.now() + '.csv';
            a.click();
        }
        
        connectWebSocket();
    </script>
</body>
</html>
)rawliteral";

  request->send(200, "text/html", html);
}

void WebServerManager::handleLog(AsyncWebServerRequest *request) {
  String csv = webServerInstance->generateCSV();
  request->send(200, "text/csv", csv);
}

void WebServerManager::handleStats(AsyncWebServerRequest *request) {
  String json = webServerInstance->generateStatsJSON();
  request->send(200, "application/json", json);
}

void WebServerManager::handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void WebServerManager::onWebSocketEvent(AsyncWebSocket *server,
                                        AsyncWebSocketClient *client,
                                        AwsEventType type, void *arg,
                                        uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    DEBUG_PRINTF("WebSocket client #%u connected\n", client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    DEBUG_PRINTF("WebSocket client #%u disconnected\n", client->id());
  }
}

// ============================================================================
// Helper Functions
// ============================================================================

String WebServerManager::generateCSV() {
  String csv = "Timestamp,RSSI,SNR,Length,RawHex\n";

  uint16_t count = packetCapture.getPacketCount();
  for (uint16_t i = 0; i < count; i++) {
    PacketData packet;
    if (packetCapture.getPacket(i, packet)) {
      csv += String(packet.timestamp) + ",";
      csv += String(packet.rssi) + ",";
      csv += String(packet.snr) + ",";
      csv += String(packet.length) + ",";

      for (uint8_t j = 0; j < packet.length; j++) {
        if (packet.data[j] < 0x10)
          csv += "0";
        csv += String(packet.data[j], HEX);
      }
      csv += "\n";
    }
  }

  return csv;
}

String WebServerManager::generateStatsJSON() {
  String json = "{";
  json +=
      "\"total_received\":" + String(packetCapture.getTotalPacketsReceived()) +
      ",";
  json +=
      "\"total_dropped\":" + String(packetCapture.getTotalPacketsDropped()) +
      ",";
  json += "\"buffer_count\":" + String(packetCapture.getPacketCount()) + ",";
  json += "\"connected_clients\":" + String(_ws->count());
  json += "}";
  return json;
}
