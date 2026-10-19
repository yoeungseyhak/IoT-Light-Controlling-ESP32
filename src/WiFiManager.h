#pragma once
// =============================================================================
// WiFiManager.h — Dual-Mode WiFi (AP + Station) Manager
// =============================================================================
// Manages concurrent SoftAP hotspot (always-on admin portal access) and
// Station mode (connect to local router for internet / cloud WebSocket).
// Includes non-blocking reconnection and network scanning for the Admin UI.
// =============================================================================

#include <Arduino.h>

namespace AppWiFi {

  // ---- Lifecycle ----
  void init();     // Start AP + attempt STA connection (call once in setup())
  void update();   // Non-blocking reconnection check (call every loop())

  // ---- Actions ----
  void connectStation(const String& ssid, const String& pass);  // (Re)connect STA
  void setSoftAP(const String& ssid, const String& pass);       // Apply new AP settings

  // ---- Status ----
  bool isStationConnected();   // Is STA linked to a router?
  String scanNetworksJson();   // Scan 2.4 GHz networks, return JSON array
}
