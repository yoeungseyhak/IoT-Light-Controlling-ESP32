#pragma once
// =============================================================================
// ParkingManager.h — Smart Parking Slot Sensor Telemetry & Telemetry Sync
// =============================================================================
// Tracks parking slot bay occupancy using Ultrasonic sensors (HC-SR04) or
// Infrared (IR) obstacle sensors. When a vehicle arrives or departs, sends
// real-time telemetry to the WebSocket cloud server & local web dashboard:
//   - Slot 1 (Slot A1) : Ultrasonic (Trig: GPIO 32, Echo: GPIO 33)
//   - Slot 2 (Slot A2) : Ultrasonic (Trig: GPIO 25, Echo: GPIO 26)
//   - Slot 3 (Slot A3) : IR Sensor  (GPIO 34)
//   - Slot 4 (Slot A4) : IR Sensor  (GPIO 35)
// =============================================================================

#include <Arduino.h>

namespace ParkingManager {

  // ---- Lifecycle ----
  void init();                                          // Setup sensor GPIOs
  void update();                                        // Poll sensors periodically (non-blocking)

  // ---- Dynamic Slot & State Control ----
  void registerSlot(const char* slotId, const char* name, const char* sensorType, const char* sensorPin);
  void removeSlot(const char* slotId);
  void setSlotState(const char* slotId, bool occupied); // Update slot & report to WebSocket
  bool isOccupied(const char* slotId);                  // Read current occupancy state
  int  getAvailableCount();                             // Number of currently free slots
  int  getTotalCount();                                 // Total registered slots

  // ---- JSON Reporting & Cloud Sync ----
  String getJsonSlots();                                // JSON representation for local WebServer
  void   syncAllToCloud();                              // Report all physical slot states to cloud WebSocket
}
