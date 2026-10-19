// =============================================================================
// WebServerManager.cpp — Embedded HTTP Server & REST API Implementation
// =============================================================================

#include "WebServerManager.h"
#include "Config.h"
#include "DeviceManager.h"
#include "ParkingManager.h"
#include "WiFiManager.h"
#include "WebSocketManager.h"
#include "WebPage/IndexPage.h"
#include "WebPage/AdminPage.h"
#include <WebServer.h>
#include <WiFi.h>

namespace AppWebServer {

  // ---- Internal server instance ----
  static WebServer server(80);

  // ---- Forward declarations for route handlers ----
  static void handleIndex();
  static void handleAdmin();
  static void handleControl();
  static void handleEmergency();
  static void handleApiState();
  static void handleApiParking();
  static void handleApiParkingControl();
  static void handleApiAdminLogin();
  static void handleApiAdminStatus();
  static void handleApiWifiScan();
  static void handleApiWifiSave();
  static void handleApiApSave();
  static void handleApiPasswordSave();
  static void handleApiServerSave();
  static void handleApiReboot();
  static void handleApiReset();

  // ---------------------------------------------------------------------------
  // init() — Register all routes and start the HTTP server
  // ---------------------------------------------------------------------------
  void init() {
    // ---- Page routes ----
    server.on("/",         HTTP_GET, handleIndex);
    server.on("/admin",    HTTP_GET, handleAdmin);
    server.on("/webadmin", HTTP_GET, handleAdmin);  // alias

    // ---- Device control ----
    server.on("/control",        HTTP_GET, handleControl);
    server.on("/api/control",    HTTP_GET, handleControl);
    server.on("/api/emergency",  HTTP_GET, handleEmergency);
    server.on("/api/state",      HTTP_GET, handleApiState);

    // ---- Parking Telemetry & Control ----
    server.on("/api/parking",         HTTP_GET, handleApiParking);
    server.on("/api/parking/control", HTTP_GET, handleApiParkingControl);

    // ---- Admin APIs ----
    server.on("/api/admin/login",     HTTP_GET, handleApiAdminLogin);
    server.on("/api/admin/status",    HTTP_GET, handleApiAdminStatus);
    server.on("/api/admin/wifi/scan", HTTP_GET, handleApiWifiScan);
    server.on("/api/admin/wifi/save", HTTP_GET, handleApiWifiSave);
    server.on("/api/admin/ap/save",   HTTP_GET, handleApiApSave);
    server.on("/api/admin/password",  HTTP_GET, handleApiPasswordSave);
    server.on("/api/admin/server",    HTTP_GET, handleApiServerSave);
    server.on("/api/admin/reboot",    HTTP_GET, handleApiReboot);
    server.on("/api/admin/reset",     HTTP_GET, handleApiReset);

    server.begin();
    Serial.println("[HTTP] Web server started on port 80");
  }

  // ---------------------------------------------------------------------------
  // update() — Process pending HTTP requests (call every loop)
  // ---------------------------------------------------------------------------
  void update() {
    server.handleClient();
  }

  // ===========================================================================
  // Route Handlers
  // ===========================================================================

  // ---- Serve Operator Dashboard HTML ----
  static void handleIndex() {
    server.send_P(200, "text/html", PAGE_INDEX);
  }

  // ---- Serve Admin Portal HTML ----
  static void handleAdmin() {
    server.send_P(200, "text/html", PAGE_ADMIN);
  }

  // ---- Device Control API (/control?id=light-1&state=on or /control?id=gate&action=open) ----
  static void handleControl() {
    String id     = server.arg("id");
    String state  = server.arg("state");
    String action = server.arg("action");

    if (id.length() == 0) {
      server.send(400, "application/json", "{\"error\":\"Missing 'id' parameter\"}");
      return;
    }

    // ---- Handle Boom Gate Control ----
    if (id == "gate" || id == "boom-gate" || id == "boomgate") {
      bool open = false;
      if (server.hasArg("action")) {
        open = (action == "open" || action == "opened" || action == "1" || action == "raise" || action == "true");
      } else if (server.hasArg("state")) {
        open = (state == "open" || state == "opened" || state == "on" || state == "1" || state == "raise" || state == "true");
      }
      DeviceManager::setGate(open);
      // Real-time broadcast to Cloud WebSocket
      AppWebSocket::sendStateReport("boom-gate", open ? "{\"status\":\"open\"}" : "{\"status\":\"closed\"}");
      server.send(200, "application/json", "{\"success\":true,\"gate\":" + String(open ? "true" : "false") + "}");
      return;
    }

    if (state != "on" && state != "1" && state != "off" && state != "0") {
      server.send(400, "application/json", "{\"error\":\"Invalid 'state' value\"}");
      return;
    }

    bool on = (state == "on" || state == "1");

    if (id == "all_lights" || id == "all") {
      DeviceManager::setAllLights(on);
      const char* st = on ? "{\"on\":true}" : "{\"on\":false}";
      AppWebSocket::sendStateReport("light-1", st);
      AppWebSocket::sendStateReport("light-2", st);
      AppWebSocket::sendStateReport("light-3", st);
      server.send(200, "application/json", "{\"success\":true}");
      return;
    }

    // Canonicalize IDs to match Cloud Database: light-1, light-2, light-3, party-light
    String canonId = id;
    if (id == "light1") canonId = "light-1";
    else if (id == "light2") canonId = "light-2";
    else if (id == "light3") canonId = "light-3";
    else if (id == "light4") canonId = "light-4";
    else if (id == "light5") canonId = "light-5";
    else if (id == "party")  canonId = "party-light";

    DeviceManager::setLight(canonId.c_str(), on);
    AppWebSocket::sendStateReport(canonId.c_str(), on ? "{\"on\":true}" : "{\"on\":false}");
    server.send(200, "application/json", "{\"success\":true}");
  }

  // ---- Emergency Evacuate / Lockdown Handler ----
  static void handleEmergency() {
    String mode = server.arg("mode");
    if (mode == "evacuate") {
      DeviceManager::setAllLights(true);
      DeviceManager::setLight("party-light", true);
      DeviceManager::setGate(true); // Open Boom Gate in evacuation!
      AppWebSocket::sendStateReport("party-light", "{\"on\":true}");
      AppWebSocket::sendStateReport("boom-gate", "{\"status\":\"open\"}");
      server.send(200, "application/json", "{\"success\":true,\"mode\":\"evacuate\"}");
    } else if (mode == "lockdown") {
      DeviceManager::setGate(false); // Close Boom Gate in lockdown!
      AppWebSocket::sendStateReport("boom-gate", "{\"status\":\"closed\"}");
      server.send(200, "application/json", "{\"success\":true,\"mode\":\"lockdown\"}");
    } else {
      DeviceManager::setLight("party-light", false);
      AppWebSocket::sendStateReport("party-light", "{\"on\":false}");
      server.send(200, "application/json", "{\"success\":true,\"mode\":\"clear\"}");
    }
  }

  // ---- Full Device State JSON (/api/state) ----
  static void handleApiState() {
    server.send(200, "application/json", DeviceManager::getJsonState());
  }

  // ---- Parking Telemetry JSON (/api/parking) ----
  static void handleApiParking() {
    server.send(200, "application/json", ParkingManager::getJsonSlots());
  }

  // ---- Parking Slot Manual Control (/api/parking/control?slot=slot-1&occupied=1) ----
  static void handleApiParkingControl() {
    String slot = server.arg("slot");
    String occupied = server.arg("occupied");

    if (slot.length() == 0) {
      server.send(400, "application/json", "{\"error\":\"Missing 'slot' parameter\"}");
      return;
    }

    bool occ = (occupied == "1" || occupied == "true" || occupied == "on");
    ParkingManager::setSlotState(slot.c_str(), occ);
    server.send(200, "application/json", "{\"success\":true}");
  }

  // ---- Admin Login Verification (/api/admin/login?user=...&pass=...) ----
  static void handleApiAdminLogin() {
    String u = server.arg("user");
    String p = server.arg("pass");

    if (u == AppConfig::adminUser && p == AppConfig::adminPass) {
      server.send(200, "application/json", "{\"success\":true}");
    } else {
      server.send(401, "application/json",
        "{\"success\":false,\"message\":\"Invalid Username or Password!\"}");
    }
  }

  // ---- Live System Telemetry (/api/admin/status) ----
  static void handleApiAdminStatus() {
    unsigned long s = millis() / 1000;
    String uptime = String(s / 3600) + "h " + String((s % 3600) / 60) + "m " + String(s % 60) + "s";

    String json = "{";
    json += "\"mac\":\""    + WiFi.macAddress() + "\",";
    json += "\"ip\":\""     + (AppWiFi::isStationConnected()
                               ? WiFi.localIP().toString()
                               : WiFi.softAPIP().toString()) + "\",";
    json += "\"heap\":\""   + String(ESP.getFreeHeap() / 1024.0, 1) + " KB\",";
    json += "\"rssi\":\""   + (AppWiFi::isStationConnected()
                               ? String(WiFi.RSSI()) + " dBm"
                               : String("N/A")) + "\",";
    json += "\"uptime\":\"" + uptime + "\",";
    json += "\"staSsid\":\"" + AppConfig::staSSID + "\",";
    json += "\"apSsid\":\""  + AppConfig::apSSID + "\",";
    json += "\"wsHost\":\""  + AppConfig::wsHost + "\",";
    json += "\"wsPort\":"    + String(AppConfig::wsPort);
    json += "}";

    server.send(200, "application/json", json);
  }

  // ---- WiFi Network Scanner (/api/admin/wifi/scan) ----
  static void handleApiWifiScan() {
    server.send(200, "application/json", AppWiFi::scanNetworksJson());
  }

  // ---- Save Station WiFi Credentials (/api/admin/wifi/save?ssid=...&pass=...) ----
  static void handleApiWifiSave() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");

    if (ssid.length() == 0) {
      server.send(400, "application/json", "{\"error\":\"Missing SSID\"}");
      return;
    }

    AppConfig::saveWiFi(ssid, pass);
    AppWiFi::connectStation(ssid, pass);
    server.send(200, "application/json", "{\"success\":true}");
  }

  // ---- Save SoftAP Settings (/api/admin/ap/save?ssid=...&pass=...) ----
  static void handleApiApSave() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");

    if (ssid.length() == 0 || pass.length() < 8) {
      server.send(400, "application/json", "{\"error\":\"SSID required, password >= 8 chars\"}");
      return;
    }

    AppConfig::saveAP(ssid, pass);
    AppWiFi::setSoftAP(ssid, pass);
    server.send(200, "application/json", "{\"success\":true}");
  }

  // ---- Change Admin Credentials (/api/admin/password?user=...&pass=...) ----
  static void handleApiPasswordSave() {
    String u = server.arg("user");
    String p = server.arg("pass");

    if (u.length() == 0 || p.length() < 6) {
      server.send(400, "application/json", "{\"error\":\"Username required, password >= 6 chars\"}");
      return;
    }

    AppConfig::saveAdmin(u, p);
    server.send(200, "application/json", "{\"success\":true}");
  }

  // ---- Save Cloud WebSocket Server (/api/admin/server?host=...&port=...) ----
  static void handleApiServerSave() {
    String host = server.arg("host");
    int    port = server.arg("port").toInt();

    if (host.length() == 0 || port <= 0) {
      server.send(400, "application/json", "{\"error\":\"Invalid host or port\"}");
      return;
    }

    AppConfig::saveServer(host, port);
    AppWebSocket::reconnect(host, port, AppConfig::wsPath);
    server.send(200, "application/json", "{\"success\":true}");
  }

  // ---- Reboot ESP32 (/api/admin/reboot) ----
  static void handleApiReboot() {
    server.send(200, "application/json", "{\"success\":true}");
    delay(500);
    ESP.restart();
  }

  // ---- Factory Reset (/api/admin/reset) ----
  static void handleApiReset() {
    server.send(200, "application/json", "{\"success\":true}");
    delay(200);
    AppConfig::resetDefaults();  // Clears NVS and reboots
  }
}
