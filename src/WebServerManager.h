#pragma once
// =============================================================================
// WebServerManager.h — Embedded HTTP Server & REST API Manager
// =============================================================================
// Hosts the web dashboard and admin portal on port 80. Provides a complete
// REST API for device control, WiFi scanning, credential management,
// system telemetry, reboot, and factory reset.
// =============================================================================

#include <Arduino.h>

namespace AppWebServer {

  // ---- Lifecycle ----
  void init();     // Register all routes and start server (call once in setup())
  void update();   // Process incoming HTTP requests (call every loop())
}
