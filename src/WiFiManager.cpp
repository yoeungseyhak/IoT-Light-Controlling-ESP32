// =============================================================================
// WiFiManager.cpp — Dual-Mode WiFi Implementation
// =============================================================================

#include "WiFiManager.h"
#include "Config.h"
#include <WiFi.h>

namespace AppWiFi {

  // ---- Internal state ----
  static bool wasConnected = false;

  // ---------------------------------------------------------------------------
  // init() — Start SoftAP hotspot immediately, then attempt Station connection
  // ---------------------------------------------------------------------------
  void init() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);

    // SoftAP starts first so admin portal is always reachable even offline
    bool apOk = WiFi.softAP(AppConfig::apSSID.c_str(), AppConfig::apPass.c_str());
    if (apOk) {
      Serial.printf("[WIFI-AP] Hotspot: %s  (Pass: %s)\n",
                    AppConfig::apSSID.c_str(), AppConfig::apPass.c_str());
      Serial.printf("[WIFI-AP] Portal : http://%s/\n",
                    WiFi.softAPIP().toString().c_str());
    } else {
      Serial.println("[WIFI-AP] Failed to start Access Point ❌");
    }

    // Attempt Station connection if SSID is configured
    if (AppConfig::staSSID.length() > 0) {
      Serial.printf("[WIFI-STA] Connecting to %s...\n", AppConfig::staSSID.c_str());
      WiFi.begin(AppConfig::staSSID.c_str(), AppConfig::staPass.c_str());
    }
  }

  // ---------------------------------------------------------------------------
  // update() — Non-blocking connection monitor & auto-reconnect (called every loop)
  // ---------------------------------------------------------------------------
  static unsigned long lastStaRetryMs = 0;
  constexpr unsigned long STA_RETRY_INTERVAL_MS = 10000; // Retry every 10s non-blocking

  void update() {
    bool connected = (WiFi.status() == WL_CONNECTED);
    unsigned long now = millis();

    if (connected && !wasConnected) {
      wasConnected = true;
      Serial.printf("\n[WIFI-STA] Connected ✅  IP: %s\n",
                    WiFi.localIP().toString().c_str());
    }
    else if (!connected && wasConnected) {
      wasConnected = false;
      lastStaRetryMs = now;
      Serial.println("\n[WIFI-STA] Connection dropped. Auto-reconnecting in background...");
    }
    else if (!connected && AppConfig::staSSID.length() > 0) {
      // Periodic non-blocking background retry when offline
      if (now - lastStaRetryMs >= STA_RETRY_INTERVAL_MS) {
        lastStaRetryMs = now;
        Serial.printf("[WIFI-STA] Background reconnecting to %s...\n", AppConfig::staSSID.c_str());
        WiFi.disconnect();
        WiFi.begin(AppConfig::staSSID.c_str(), AppConfig::staPass.c_str());
      }
    }
  }

  // ---------------------------------------------------------------------------
  // connectStation() — Apply new STA credentials and reconnect
  // ---------------------------------------------------------------------------
  void connectStation(const String& ssid, const String& pass) {
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid.c_str(), pass.c_str());
    wasConnected = false;
    Serial.printf("[WIFI-STA] Reconnecting to %s...\n", ssid.c_str());
  }

  // ---------------------------------------------------------------------------
  // setSoftAP() — Apply new AP credentials (takes effect immediately)
  // ---------------------------------------------------------------------------
  void setSoftAP(const String& ssid, const String& pass) {
    WiFi.softAPdisconnect(true);
    delay(100);
    WiFi.softAP(ssid.c_str(), pass.c_str());
    Serial.printf("[WIFI-AP] Updated: %s\n", ssid.c_str());
  }

  // ---------------------------------------------------------------------------
  // isStationConnected() — Quick status check
  // ---------------------------------------------------------------------------
  bool isStationConnected() {
    return (WiFi.status() == WL_CONNECTED);
  }

  // ---------------------------------------------------------------------------
  // scanNetworksJson() — Scan nearby 2.4 GHz networks, return JSON
  // ---------------------------------------------------------------------------
  String scanNetworksJson() {
    int n = WiFi.scanNetworks();
    String json = "{\"networks\":[";
    for (int i = 0; i < n; i++) {
      if (i > 0) json += ",";
      json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",";
      json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
      json += "\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false") + "}";
    }
    json += "]}";
    WiFi.scanDelete();  // Free scan results memory
    return json;
  }
}
