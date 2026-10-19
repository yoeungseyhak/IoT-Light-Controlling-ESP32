// =============================================================================
// Config.cpp — NVS Persistence Implementation
// =============================================================================

#include "Config.h"

namespace AppConfig {

  // ---- Storage handle ----
  static Preferences prefs;

  // ---- Runtime config variable definitions ----
  String adminUser;
  String adminPass;
  String staSSID;
  String staPass;
  String apSSID;
  String apPass;
  String wsHost;
  int    wsPort;
  String wsPath;

  // ---------------------------------------------------------------------------
  // load() — Read all parameters from NVS; use hardcoded defaults if absent
  // ---------------------------------------------------------------------------
  void load() {
    prefs.begin("sys_cfg", true);  // open read-only
    adminUser = prefs.getString("admin_user", "admin");
    adminPass = prefs.getString("admin_pass", "admin123");
    staSSID   = prefs.getString("sta_ssid",   "Hak");
    staPass   = prefs.getString("sta_pass",   "Hak12345");
    apSSID    = prefs.getString("ap_ssid",    "Cotafer_Parking");
    apPass    = prefs.getString("ap_pass",    "12345678");
    wsHost    = prefs.getString("ws_host",    "iot.floormanagement.site");
    wsPort    = prefs.getInt("ws_port",       80);
    wsPath    = prefs.getString("ws_path",    "/ws/device");
    prefs.end();

    Serial.println("[CONFIG] Loaded from NVS Flash");
    Serial.printf("  Admin  : %s / ***\n", adminUser.c_str());
    Serial.printf("  STA    : %s\n",       staSSID.c_str());
    Serial.printf("  AP     : %s\n",       apSSID.c_str());
    Serial.printf("  WS     : %s:%d%s\n",  wsHost.c_str(), wsPort, wsPath.c_str());
  }

  // ---------------------------------------------------------------------------
  // Persistence helpers — each opens NVS read-write, writes, then closes
  // ---------------------------------------------------------------------------
  void saveAdmin(const String& user, const String& pass) {
    prefs.begin("sys_cfg", false);
    prefs.putString("admin_user", user);
    prefs.putString("admin_pass", pass);
    prefs.end();
    adminUser = user;
    adminPass = pass;
    Serial.printf("[CONFIG] Admin credentials saved: %s\n", user.c_str());
  }

  void saveWiFi(const String& ssid, const String& pass) {
    prefs.begin("sys_cfg", false);
    prefs.putString("sta_ssid", ssid);
    prefs.putString("sta_pass", pass);
    prefs.end();
    staSSID = ssid;
    staPass = pass;
    Serial.printf("[CONFIG] WiFi STA saved: %s\n", ssid.c_str());
  }

  void saveAP(const String& ssid, const String& pass) {
    prefs.begin("sys_cfg", false);
    prefs.putString("ap_ssid", ssid);
    prefs.putString("ap_pass", pass);
    prefs.end();
    apSSID = ssid;
    apPass = pass;
    Serial.printf("[CONFIG] AP saved: %s\n", ssid.c_str());
  }

  void saveServer(const String& host, int port) {
    prefs.begin("sys_cfg", false);
    prefs.putString("ws_host", host);
    prefs.putInt("ws_port", port);
    prefs.end();
    wsHost = host;
    wsPort = port;
    Serial.printf("[CONFIG] WebSocket server saved: %s:%d\n", host.c_str(), port);
  }

  // ---------------------------------------------------------------------------
  // resetDefaults() — Erase entire NVS namespace and restart
  // ---------------------------------------------------------------------------
  void resetDefaults() {
    prefs.begin("sys_cfg", false);
    prefs.clear();
    prefs.end();
    Serial.println("[CONFIG] Factory reset! Rebooting...");
    delay(300);
    ESP.restart();
  }
}
