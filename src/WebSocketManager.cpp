// =============================================================================
// WebSocketManager.cpp — Cloud WebSocket Client Implementation
// =============================================================================

#include "WebSocketManager.h"
#include "Config.h"
#include "DeviceManager.h"
#include "ParkingManager.h"
#include "WiFiManager.h"
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

namespace AppWebSocket {

  // ---- Internal objects ----
  static WebSocketsClient ws;
  static unsigned long lastHeartbeatMs = 0;
  static const unsigned long HEARTBEAT_INTERVAL = 5000;  // 5 seconds
  static bool wasStationConnected = false;

  // ---- Forward declaration ----
  static void onEvent(WStype_t type, uint8_t* payload, size_t length);
  static void handleServerMessage(uint8_t* payload, size_t length);

  // ---------------------------------------------------------------------------
  // Helper: decide ws:// vs wss:// based on port
  // Port 443 or 80 (behind reverse proxy like Nginx/Cloudflare) → use SSL
  // Other ports (5050, 8080, etc.) → plain ws://
  // ---------------------------------------------------------------------------
  static void beginConnection(const String& host, int port, const String& path) {
    if (port == 443) {
      // SSL WebSocket (wss://) when port is explicitly set to 443
      ws.beginSSL(host.c_str(), 443, path.c_str());
      Serial.printf("[WS] Connecting wss://%s:443%s (SSL)\n", host.c_str(), path.c_str());
    } else {
      // Plain WebSocket (ws://) for port 80 (HTTP) or port 5050 (Local Node.js)
      ws.begin(host.c_str(), port, path.c_str());
      Serial.printf("[WS] Connecting ws://%s:%d%s\n", host.c_str(), port, path.c_str());
    }
  }

  // ---------------------------------------------------------------------------
  // init() — Register event handler (connection starts when WiFi connects)
  // ---------------------------------------------------------------------------
  void init() {
    ws.onEvent(onEvent);
    ws.setReconnectInterval(10000); // 10s interval avoids aggressive DNS stalls
  }

  // ---------------------------------------------------------------------------
  // update() — Process WS events and heartbeats (strictly non-blocking & offline-safe)
  // ---------------------------------------------------------------------------
  void update() {
    bool staConnected = AppWiFi::isStationConnected();

    // OFFLINE CHECK: If WiFi is not connected to router, skip ws.loop() completely!
    // This prevents DNS lookup timeouts from freezing buttons & local hardware!
    if (!staConnected) {
      if (wasStationConnected) {
        wasStationConnected = false;
        ws.disconnect();
        Serial.println("[WS] Station offline -> Paused WebSocket (100% responsive offline mode).");
      }
      return; // Zero CPU blocking when offline
    }

    // WiFi Station just became connected -> establish WebSocket!
    if (!wasStationConnected) {
      wasStationConnected = true;
      Serial.println("[WS] Station connected -> Connecting to cloud WebSocket...");
      beginConnection(AppConfig::wsHost, AppConfig::wsPort, AppConfig::wsPath);
      ws.setReconnectInterval(1000);
    }

    // Only process loop when WiFi is confirmed connected
    ws.loop();

    unsigned long now = millis();
    if (now - lastHeartbeatMs >= HEARTBEAT_INTERVAL) {
      lastHeartbeatMs = now;
      sendHeartbeat();
    }
  }

  // ---------------------------------------------------------------------------
  // reconnect() — Switch to a different server at runtime
  // ---------------------------------------------------------------------------
  void reconnect(const String& host, int port, const String& path) {
    ws.disconnect();
    delay(100);
    beginConnection(host, port, path);
    ws.onEvent(onEvent);
    ws.setReconnectInterval(3000);
  }

  // ---------------------------------------------------------------------------
  // sendHeartbeat() — Keep-alive ping to the server
  // ---------------------------------------------------------------------------
  void sendHeartbeat() {
    if (!ws.isConnected()) return;
    ws.sendTXT("{\"type\":\"heartbeat\"}");
  }

  // ---------------------------------------------------------------------------
  // sendStateReport() — Report a device state change to the cloud
  // ---------------------------------------------------------------------------
  void sendStateReport(const char* deviceId, const char* stateJson) {
    if (!ws.isConnected()) return;

    StaticJsonDocument<256> doc;
    doc["type"]     = "state-report";
    doc["deviceId"] = deviceId;

    StaticJsonDocument<128> stateDoc;
    deserializeJson(stateDoc, stateJson);
    doc["state"] = stateDoc;

    String output;
    serializeJson(doc, output);
    ws.sendTXT(output);
  }

  // ---------------------------------------------------------------------------
  // sendSlotReport() — Report parking space telemetry to the cloud
  // ---------------------------------------------------------------------------
  void sendSlotReport(const char* slotId, bool occupied) {
    if (!ws.isConnected()) return;

    StaticJsonDocument<256> doc;
    doc["type"]     = "slot-update";
    doc["slotId"]   = slotId;
    doc["occupied"] = occupied;

    String output;
    serializeJson(doc, output);
    ws.sendTXT(output);
    Serial.printf("[WS] Parking Slot Report: %s -> %s\n", slotId, occupied ? "OCCUPIED" : "AVAILABLE");
  }

  // ---------------------------------------------------------------------------
  // onEvent() — WebSocket lifecycle event handler
  // ---------------------------------------------------------------------------
  static void onEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
      case WStype_CONNECTED:
        Serial.printf("[WS] Connected ✅ to %s:%d%s\n",
                      AppConfig::wsHost.c_str(), AppConfig::wsPort, AppConfig::wsPath.c_str());
        sendHeartbeat();
        // Immediately push restored hardware states and physical switch positions to cloud!
        DeviceManager::syncAllToCloud();
        ParkingManager::syncAllToCloud();
        break;

      case WStype_DISCONNECTED:
        Serial.println("[WS] Disconnected. Auto-reconnecting in 3s...");
        break;

      case WStype_TEXT:
        handleServerMessage(payload, length);
        break;

      case WStype_ERROR:
        Serial.printf("[WS] Error: %s\n", payload ? (char*)payload : "unknown");
        break;

      case WStype_PING:
        Serial.println("[WS] Ping received");
        break;

      case WStype_PONG:
        Serial.println("[WS] Pong received");
        break;

      default:
        break;
    }
  }

  // ---------------------------------------------------------------------------
  // handleServerMessage() — Parse incoming JSON commands from the server
  // ---------------------------------------------------------------------------
  static void handleServerMessage(uint8_t* payload, size_t length) {
    DynamicJsonDocument doc(2048);
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) {
      Serial.printf("[WS] JSON parse error: %s\n", err.c_str());
      return;
    }

    const char* type = doc["type"];
    if (!type) return;

    // ---- 1. Handle device-update commands ----
    if (strcmp(type, "device-update") == 0) {
      const char* triggeredBy = doc["triggeredBy"];
      // Server system-init on connect: do NOT overwrite ESP32 restored NVS states!
      if (triggeredBy && strcmp(triggeredBy, "system-init") == 0) {
        Serial.println("[WS] Preserving ESP32 hardware states over system-init dump");
        return;
      }

      const char* deviceId = doc["deviceId"];
      if (!deviceId) return;

      int explicitPin = -1;
      if (doc.containsKey("pin")) {
        const char* pinStr = doc["pin"];
        explicitPin = DeviceManager::parsePinString(pinStr);
      }

      JsonObject state = doc["state"];
      Serial.printf("[WS] Command for device %s (Pin %d)\n", deviceId, explicitPin);

      // Handle Boom Gate commands (from Web UI, Admin override, or Emergency)
      if (strcmp(deviceId, "boom-gate") == 0 || strcmp(deviceId, "gate") == 0 || strcmp(deviceId, "boomgate") == 0) {
        bool openGate = false;
        if (state.containsKey("status")) {
          const char* st = state["status"];
          openGate = (strcmp(st, "open") == 0 || strcmp(st, "opened") == 0);
        } else if (state.containsKey("open")) {
          openGate = state["open"].as<bool>();
        } else if (state.containsKey("on")) {
          openGate = state["on"].as<bool>();
        }
        DeviceManager::setGate(openGate, explicitPin);
        return;
      }

      if (state.containsKey("on")) {
        DeviceManager::setLight(deviceId, state["on"].as<bool>(), explicitPin);
      }
    }
    // ---- 2. Handle dynamic device created or removed ----
    else if (strcmp(type, "devices-changed") == 0) {
      const char* action = doc["action"];
      if (action && strcmp(action, "create") == 0) {
        JsonObject dev = doc["device"];
        const char* id = dev["id"];
        const char* pinStr = dev["pin"];
        int pin = DeviceManager::parsePinString(pinStr);
        if (pin >= 0) {
          pinMode(pin, OUTPUT);
          digitalWrite(pin, LOW);
          Serial.printf("[WS] Dynamic component configured: %s on GPIO %d\n", id, pin);
        }
      }
    }
    // ---- 3. Initial sync of all parking slots on connect ----
    else if (strcmp(type, "slots-init") == 0) {
      JsonArray slots = doc["slots"];
      for (JsonObject slot : slots) {
        const char* id = slot["id"];
        const char* name = slot["name"];
        const char* sensorType = slot["sensor_type"];
        const char* sensorPin = slot["sensor_pin"];
        ParkingManager::registerSlot(id, name, sensorType, sensorPin);
      }
      Serial.printf("[WS] Synced %d parking slots from server\n", (int)slots.size());
      // Re-report live physical switch states to cloud after registration
      ParkingManager::syncAllToCloud();
    }
    // ---- 4. Handle dynamic slot created or removed ----
    else if (strcmp(type, "slots-changed") == 0) {
      const char* action = doc["action"];
      if (action && strcmp(action, "create") == 0) {
        JsonObject slot = doc["slot"];
        const char* id = slot["id"];
        const char* name = slot["name"];
        const char* sensorType = slot["sensor_type"];
        const char* sensorPin = slot["sensor_pin"];
        ParkingManager::registerSlot(id, name, sensorType, sensorPin);
      } else if (action && strcmp(action, "delete") == 0) {
        const char* id = doc["slotId"];
        ParkingManager::removeSlot(id);
      }
    }
    // ---- 5. Manual or external slot override ----
    else if (strcmp(type, "slot-command") == 0) {
      const char* slotId = doc["slotId"];
      bool occupied = doc["occupied"].as<bool>();
      ParkingManager::setSlotState(slotId, occupied);
    }
  }
}
