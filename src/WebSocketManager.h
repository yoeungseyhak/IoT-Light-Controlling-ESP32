#pragma once
// =============================================================================
// WebSocketManager.h — Cloud WebSocket Client Manager
// =============================================================================
// Maintains a persistent bi-directional WebSocket link to the cloud server
// (e.g. iot.floormanagement.site). Handles auto-reconnect, periodic heartbeats,
// incoming device-update commands, and outgoing state reports.
// =============================================================================

#include <Arduino.h>

namespace AppWebSocket {

  // ---- Lifecycle ----
  void init();     // Connect to WS server using AppConfig settings (call in setup())
  void update();   // Process WS events + heartbeat timer (call every loop())

  // ---- Reconnect to a new host:port at runtime ----
  void reconnect(const String& host, int port, const String& path);

  // ---- Outgoing Messages ----
  void sendStateReport(const char* deviceId, const char* stateJson);
  void sendSlotReport(const char* slotId, bool occupied);
  void sendHeartbeat();
}
