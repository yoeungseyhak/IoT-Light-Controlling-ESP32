// =============================================================================
// ParkingManager.cpp — Smart Parking Slot Sensor Telemetry Implementation
// =============================================================================

#include "ParkingManager.h"
#include "WebSocketManager.h"

namespace ParkingManager {

  enum SensorMethod {
    METHOD_SWITCH,      // Mechanical key switch / microswitch / button (INPUT_PULLUP, LOW = occupied)
    METHOD_ULTRASONIC,  // Ultrasonic HC-SR04 (Trig, Echo)
    METHOD_IR           // IR obstacle sensor (digital)
  };

  struct SlotConfig {
    char id[32];
    char name[32];
    SensorMethod method;
    int trigPin;             // -1 if not ultrasonic
    int echoPin;             // -1 if not ultrasonic
    int irPin;               // -1 if not IR
    int switchPin;           // GPIO for mechanical switch key
    bool occupied;
    bool lastReportedState;
    unsigned long lastMeasureMs;
    unsigned long lastDebounceMs;
  };

  constexpr int MAX_SLOTS = 16;
  static int numSlots = 4;

  // Pre-configured parking slots (Virtual / Managed via Cloud & Web)
  static SlotConfig slots[MAX_SLOTS] = {
    { "slot-1", "Slot A1", METHOD_SWITCH, -1, -1, -1, -1, false, false, 0, 0 },
    { "slot-2", "Slot A2", METHOD_SWITCH, -1, -1, -1, -1, false, false, 0, 0 },
    { "slot-3", "Slot A3", METHOD_SWITCH, -1, -1, -1, -1, false, false, 0, 0 },
    { "slot-4", "Slot A4", METHOD_SWITCH, -1, -1, -1, -1, false, false, 0, 0 }
  };

  static const unsigned long POLL_INTERVAL_MS = 1500; // Ultrasonic check interval
  static unsigned long lastUltrasonicPollMs = 0;

  // Measure distance from Ultrasonic HC-SR04 sensor in cm
  static float readDistanceCm(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 25000); // 25ms timeout (~4m max)
    if (duration == 0) return -1.0f; // No echo
    return (duration * 0.0343f) / 2.0f;
  }

  // Parse up to two pin numbers from a string (e.g. "GPIO 32, 33" or "32")
  static void parsePins(const char* pinStr, int& p1, int& p2) {
    p1 = -1;
    p2 = -1;
    if (!pinStr) return;
    const char* p = pinStr;
    while (*p) {
      if (isdigit(*p)) {
        if (p1 == -1) {
          p1 = atoi(p);
        } else if (p2 == -1) {
          p2 = atoi(p);
          break;
        }
        while (isdigit(*p)) p++;
        continue;
      }
      p++;
    }
  }

  // ---------------------------------------------------------------------------
  // init() — Configure GPIO pins for all parking slot sensors
  // ---------------------------------------------------------------------------
  void init() {
    for (int i = 0; i < numSlots; i++) {
      if (slots[i].method == METHOD_SWITCH && slots[i].switchPin >= 0) {
        // Pins <= 33 support internal pullup; pins 34-39 are input only without pullup
        pinMode(slots[i].switchPin, (slots[i].switchPin <= 33) ? INPUT_PULLUP : INPUT);
      }
      else if (slots[i].method == METHOD_ULTRASONIC && slots[i].trigPin >= 0 && slots[i].echoPin >= 0) {
        pinMode(slots[i].trigPin, OUTPUT);
        digitalWrite(slots[i].trigPin, LOW);
        pinMode(slots[i].echoPin, INPUT);
      }
      else if (slots[i].method == METHOD_IR && slots[i].irPin >= 0) {
        pinMode(slots[i].irPin, (slots[i].irPin <= 33) ? INPUT_PULLUP : INPUT);
      }
    }

    Serial.printf("[PARKING] %d parking slot sensors configured:\n", numSlots);
    for (int i = 0; i < numSlots; i++) {
      if (slots[i].method == METHOD_SWITCH) {
        Serial.printf("  %s (%s): Mechanical Switch Key (GPIO %d, INPUT_PULLUP)\n",
                      slots[i].name, slots[i].id, slots[i].switchPin);
      } else if (slots[i].method == METHOD_ULTRASONIC) {
        Serial.printf("  %s (%s): Ultrasonic (Trig %d, Echo %d)\n",
                      slots[i].name, slots[i].id, slots[i].trigPin, slots[i].echoPin);
      } else if (slots[i].method == METHOD_IR) {
        Serial.printf("  %s (%s): IR Sensor (GPIO %d)\n",
                      slots[i].name, slots[i].id, slots[i].irPin);
      } else {
        Serial.printf("  %s (%s): Virtual / Manual Slot\n", slots[i].name, slots[i].id);
      }
    }

    // Give pull-up resistors 10ms to settle
    delay(10);

    // CRITICAL: Immediately read physical switch/button states upon boot!
    // If the user changed the switch key while unpowered, this detects it right now!
    for (int i = 0; i < numSlots; i++) {
      if (slots[i].method == METHOD_SWITCH && slots[i].switchPin >= 0) {
        bool rawPressed = (digitalRead(slots[i].switchPin) == LOW);
        slots[i].occupied = rawPressed;
        slots[i].lastReportedState = rawPressed;
        slots[i].lastDebounceMs = millis();
        Serial.printf("[PARKING] Physical check: %s (%s) is %s\n",
                      slots[i].name, slots[i].id, rawPressed ? "OCCUPIED 🚗" : "FREE 🟢");
      }
      else if (slots[i].method == METHOD_IR && slots[i].irPin >= 0) {
        bool rawDetected = (digitalRead(slots[i].irPin) == LOW);
        slots[i].occupied = rawDetected;
        slots[i].lastReportedState = rawDetected;
        slots[i].lastDebounceMs = millis();
      }
    }
  }

  // ---------------------------------------------------------------------------
  // registerSlot() — Dynamically add or update a slot registered from the cloud
  // ---------------------------------------------------------------------------
  void registerSlot(const char* slotId, const char* name, const char* sensorType, const char* sensorPin) {
    if (!slotId) return;

    int p1 = -1, p2 = -1;
    parsePins(sensorPin, p1, p2);

    SensorMethod method = METHOD_SWITCH;
    int trig = -1, echo = -1, ir = -1, swPin = -1;

    if (sensorType && (strstr(sensorType, "switch") || strstr(sensorType, "mech") || strstr(sensorType, "key") || strstr(sensorType, "button"))) {
      method = METHOD_SWITCH;
      swPin = (p1 >= 0) ? p1 : 32;
      pinMode(swPin, (swPin <= 33) ? INPUT_PULLUP : INPUT);
    } else if (sensorType && strstr(sensorType, "ir")) {
      method = METHOD_IR;
      ir = (p1 >= 0) ? p1 : 34;
      pinMode(ir, (ir <= 33) ? INPUT_PULLUP : INPUT);
    } else if (sensorType && strstr(sensorType, "ultrasonic")) {
      method = METHOD_ULTRASONIC;
      trig = p1;
      echo = p2;
      if (trig >= 0 && echo >= 0) {
        pinMode(trig, OUTPUT);
        digitalWrite(trig, LOW);
        pinMode(echo, INPUT);
      }
    } else {
      // Default to switch if single pin provided
      if (p2 < 0 && p1 >= 0) {
        method = METHOD_SWITCH;
        swPin = p1;
        pinMode(swPin, (swPin <= 33) ? INPUT_PULLUP : INPUT);
      }
    }

    // Check if slot exists
    for (int i = 0; i < numSlots; i++) {
      if (strcmp(slots[i].id, slotId) == 0) {
        if (name) strncpy(slots[i].name, name, sizeof(slots[i].name) - 1);
        slots[i].method    = method;
        slots[i].trigPin   = trig;
        slots[i].echoPin   = echo;
        slots[i].irPin     = ir;
        slots[i].switchPin = swPin;
        Serial.printf("[PARKING] Slot %s updated: method=%d pin=%d\n", slotId, (int)method, (method == METHOD_SWITCH) ? swPin : (method == METHOD_IR ? ir : trig));
        return;
      }
    }

    // If new slot
    if (numSlots < MAX_SLOTS) {
      strncpy(slots[numSlots].id, slotId, sizeof(slots[numSlots].id) - 1);
      strncpy(slots[numSlots].name, name ? name : slotId, sizeof(slots[numSlots].name) - 1);
      slots[numSlots].method    = method;
      slots[numSlots].trigPin   = trig;
      slots[numSlots].echoPin   = echo;
      slots[numSlots].irPin     = ir;
      slots[numSlots].switchPin = swPin;
      slots[numSlots].occupied  = false;
      slots[numSlots].lastReportedState = false;
      slots[numSlots].lastMeasureMs = 0;
      slots[numSlots].lastDebounceMs = 0;
      numSlots++;
      Serial.printf("[PARKING] Dynamic slot registered: %s (%s) [Pin %d]\n", name, slotId, (method == METHOD_SWITCH) ? swPin : (method == METHOD_IR ? ir : trig));
    }
  }

  // ---------------------------------------------------------------------------
  // removeSlot() — Remove a deleted slot
  // ---------------------------------------------------------------------------
  void removeSlot(const char* slotId) {
    if (!slotId) return;
    for (int i = 0; i < numSlots; i++) {
      if (strcmp(slots[i].id, slotId) == 0) {
        for (int j = i; j < numSlots - 1; j++) {
          slots[j] = slots[j + 1];
        }
        numSlots--;
        Serial.printf("[PARKING] Slot removed: %s\n", slotId);
        return;
      }
    }
  }

  // ---------------------------------------------------------------------------
  // setSlotState() — Update slot occupancy & report to cloud WebSocket
  // ---------------------------------------------------------------------------
  void setSlotState(const char* slotId, bool occupied) {
    if (!slotId) return;

    for (int i = 0; i < numSlots; i++) {
      if (strcmp(slots[i].id, slotId) == 0) {
        slots[i].occupied = occupied;
        slots[i].lastReportedState = occupied;
        AppWebSocket::sendSlotReport(slots[i].id, occupied);
        Serial.printf("[PARKING] %s manually set to %s\n", slotId, occupied ? "OCCUPIED" : "AVAILABLE");
        return;
      }
    }

    AppWebSocket::sendSlotReport(slotId, occupied);
    Serial.printf("[PARKING] Dynamic %s reported as %s\n", slotId, occupied ? "OCCUPIED" : "AVAILABLE");
  }

  // ---------------------------------------------------------------------------
  // isOccupied() — Check current occupancy
  // ---------------------------------------------------------------------------
  bool isOccupied(const char* slotId) {
    if (!slotId) return false;
    for (int i = 0; i < numSlots; i++) {
      if (strcmp(slots[i].id, slotId) == 0) {
        return slots[i].occupied;
      }
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  // getAvailableCount() — Count available bays
  // ---------------------------------------------------------------------------
  int getAvailableCount() {
    int count = 0;
    for (int i = 0; i < numSlots; i++) {
      if (!slots[i].occupied) count++;
    }
    return count;
  }

  int getTotalCount() {
    return numSlots;
  }

  // ---------------------------------------------------------------------------
  // update() — Read sensors periodically and report changes
  // ---------------------------------------------------------------------------
  void update() {
    unsigned long now = millis();

    // 1. Check all slots
    for (int i = 0; i < numSlots; i++) {
      bool currentlyOccupied = slots[i].occupied;

      // A) MECHANICAL SWITCH KEY (Active LOW when pressed/closed to GND)
      if (slots[i].method == METHOD_SWITCH && slots[i].switchPin >= 0) {
        bool rawPressed = (digitalRead(slots[i].switchPin) == LOW);
        if (rawPressed != slots[i].occupied) {
          if (now - slots[i].lastDebounceMs > 25) { // 25ms snappy debounce
            currentlyOccupied = rawPressed;
            slots[i].lastDebounceMs = now;
          }
        } else {
          slots[i].lastDebounceMs = now;
        }
      }
      // B) IR OBSTACLE SENSOR (Active LOW)
      else if (slots[i].method == METHOD_IR && slots[i].irPin >= 0) {
        bool rawDetected = (digitalRead(slots[i].irPin) == LOW);
        if (rawDetected != slots[i].occupied) {
          if (now - slots[i].lastDebounceMs > 80) {
            currentlyOccupied = rawDetected;
            slots[i].lastDebounceMs = now;
          }
        } else {
          slots[i].lastDebounceMs = now;
        }
      }
      // C) ULTRASONIC SENSOR (Polled at 1.5s interval)
      else if (slots[i].method == METHOD_ULTRASONIC && slots[i].trigPin >= 0 && slots[i].echoPin >= 0) {
        if (now - lastUltrasonicPollMs >= POLL_INTERVAL_MS) {
          float distance = readDistanceCm(slots[i].trigPin, slots[i].echoPin);
          if (distance > 2.0f && distance < 20.0f) {
            currentlyOccupied = true;
          } else if (distance >= 20.0f || distance < 0.0f) {
            currentlyOccupied = false;
          }
        }
      }

      slots[i].occupied = currentlyOccupied;

      // Report to Cloud WebSocket when state changes
      if (slots[i].occupied != slots[i].lastReportedState) {
        slots[i].lastReportedState = slots[i].occupied;
        AppWebSocket::sendSlotReport(slots[i].id, slots[i].occupied);
        Serial.printf("[PARKING] %s (%s): %s [%s]\n",
                      slots[i].name, slots[i].id,
                      slots[i].occupied ? "VEHICLE PARKED 🚗" : "AVAILABLE 🟢",
                      slots[i].method == METHOD_SWITCH ? "Mechanical Switch Key" : (slots[i].method == METHOD_IR ? "IR Sensor" : "Ultrasonic"));
      }
    }

    if (now - lastUltrasonicPollMs >= POLL_INTERVAL_MS) {
      lastUltrasonicPollMs = now;
    }
  }

  // ---------------------------------------------------------------------------
  // getJsonSlots() — JSON representation for local WebServer
  // ---------------------------------------------------------------------------
  String getJsonSlots() {
    String json = "{\"total\":" + String(numSlots) + ",\"available\":" + String(getAvailableCount()) + ",\"slots\":[";
    for (int i = 0; i < numSlots; i++) {
      if (i > 0) json += ",";
      json += "{\"id\":\"" + String(slots[i].id) + "\",\"occupied\":" + (slots[i].occupied ? "true" : "false") + "}";
    }
    json += "]}";
    return json;
  }

  // ---------------------------------------------------------------------------
  // syncAllToCloud() — Send all physical slot states to cloud WebSocket
  // ---------------------------------------------------------------------------
  void syncAllToCloud() {
    for (int i = 0; i < numSlots; i++) {
      AppWebSocket::sendSlotReport(slots[i].id, slots[i].occupied);
    }
    Serial.printf("[PARKING] Synced %d physical slot states to cloud.\n", numSlots);
  }
}
