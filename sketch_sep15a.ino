// =============================================================================
// Floor Management System (ESP32 Entry Point)
// =============================================================================
//   src/Config.h           → NVS Flash persistence (credentials, WiFi, server)
//   src/DeviceManager.h    → GPIO pins, LED control, hardware state
//   src/WiFiManager.h      → Dual AP + Station WiFi, network scanner
//   src/WebSocketManager.h → Cloud WebSocket client, heartbeat, commands
//   src/WebServerManager.h → HTTP server, REST APIs, web dashboard
// =============================================================================

#include "src/Config.h"
#include "src/DeviceManager.h"
#include "src/ParkingManager.h"
#include "src/WiFiManager.h"
#include "src/WebSocketManager.h"
#include "src/WebServerManager.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n========================================");
  Serial.println(" Floor Management System — ESP32");
  Serial.println("========================================");

  AppConfig::load();         // 1. Load config from NVS Flash
  DeviceManager::init();     // 2. Initialize GPIO pins (Lights 1 to 5 + Party)
  ParkingManager::init();    // 3. Initialize Parking sensors (Ultrasonic + IR)
  AppWiFi::init();           // 4. Start WiFi (AP hotspot + Station)
  AppWebServer::init();      // 5. Start HTTP server & REST APIs
  AppWebSocket::init();      // 6. Connect to cloud WebSocket

  Serial.println("========================================");
  Serial.println(" System Ready ✅");
  Serial.println("========================================\n");
}

void loop() {
  // 1. Physical Hardware & Switches First (100% offline responsive, 0ms lag)
  DeviceManager::update();   // Rocker switches (GPIO 19, 21, 22), LDR, Relays
  ParkingManager::update();  // Parking slot sensors

  // 2. Local HTTP Server (Instant local web access on AP 192.168.4.1 & LAN)
  AppWebServer::update();

  // 3. Network Background Sync (Non-blocking auto-reconnect)
  AppWiFi::update();         // Background WiFi reconnect manager
  AppWebSocket::update();    // Cloud WebSocket (only active when WiFi is connected)
}