#pragma once
// =============================================================================
// Config.h — System Configuration & NVS Persistence Manager
// =============================================================================
// Manages all user-configurable parameters stored in ESP32 Non-Volatile Storage
// (Preferences / NVS Flash). Values survive power cycles and reboots.
// =============================================================================

#include <Arduino.h>
#include <Preferences.h>

namespace AppConfig {

  // ---- Runtime Configuration Variables (loaded from NVS on boot) ----
  extern String adminUser;
  extern String adminPass;
  extern String staSSID;
  extern String staPass;
  extern String apSSID;
  extern String apPass;
  extern String wsHost;
  extern int    wsPort;
  extern String wsPath;

  // ---- Lifecycle ----
  void load();            // Read all config from NVS Flash (call once in setup())
  void resetDefaults();   // Erase NVS partition and reboot

  // ---- Persistence Helpers (write individual sections to NVS) ----
  void saveAdmin(const String& user, const String& pass);
  void saveWiFi(const String& ssid, const String& pass);
  void saveAP(const String& ssid, const String& pass);
  void saveServer(const String& host, int port);
}
