// =============================================================================
// DeviceManager.cpp — Hardware GPIO & Actuator Implementation
// =============================================================================

#include "DeviceManager.h"
#include "WebSocketManager.h"
#include <Preferences.h>
#include <esp_arduino_version.h>

namespace DeviceManager {

  // ---- Storage handle for Actuator NVS persistence ----
  static Preferences devPrefs;

  // ---- Internal state tracking ----
  static bool stateLight1 = false;
  static bool stateLight2 = false;
  static bool stateLight3 = false;
  static bool stateLight4 = false;
  static bool stateLight5 = false;
  static bool stateParty  = false;
  static bool stateRelayLdr = false;

  // ---- Boom Gate Servo State & Timing (SG90 180° restricted to 0° - 90°) ----
  static int servoPin = PIN_BOOM_GATE_SERVO;
  static bool stateGateOpen = false;
  static int currentGateAngle = GATE_ANGLE_CLOSED;
  static int targetGateAngle  = GATE_ANGLE_CLOSED;
  static unsigned long lastServoStepMs = 0;
  constexpr unsigned long SERVO_STEP_INTERVAL_MS = 15; // 15ms per degree for smooth non-blocking motion
  static bool servoInitialized = false;

  // ---------------------------------------------------------------------------
  // initServoPwm() — Configure ESP32 LEDC PWM timer for 50Hz Servo control
  // Compatible with both ESP32 Arduino Core 3.x and 2.x
  // ---------------------------------------------------------------------------
  static void initServoPwm(int pin) {
#if defined(ESP_ARDUINO_VERSION) && (ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0))
    ledcAttach(pin, 50, 14); // 50 Hz, 14-bit resolution (0 - 16383)
#else
    ledcSetup(7, 50, 14);
    ledcAttachPin(pin, 7);
#endif
    servoInitialized = true;
    servoPin = pin;
  }

  // ---------------------------------------------------------------------------
  // writeServoDuty() — Calculate pulse width and set PWM duty cycle
  // 0° -> ~500us pulse, 90° -> ~1500us pulse (14-bit = 16384 counts / 20000us)
  // ---------------------------------------------------------------------------
  static void writeServoDuty(int angle) {
    if (!servoInitialized) initServoPwm(servoPin);
    if (angle < GATE_ANGLE_CLOSED) angle = GATE_ANGLE_CLOSED;
    if (angle > GATE_ANGLE_OPEN)   angle = GATE_ANGLE_OPEN;

    uint32_t pulseUs = 500 + ((uint32_t)angle * 1000) / 90;
    uint32_t duty = (pulseUs * 16384) / 20000;

#if defined(ESP_ARDUINO_VERSION) && (ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0))
    ledcWrite(servoPin, duty);
#else
    ledcWrite(7, duty);
#endif
  }

  // ---- LDR Hardware Timing (Pure real-time sensing, NO storage) ----
  static unsigned long lastLdrCheckMs = 0;
  constexpr unsigned long LDR_CHECK_INTERVAL_MS = 250; // Check every 250ms for responsive light sensing

  // ---- Switch Button Debounce State (GPIO 19, 21, 22) ----
  static int lastSwState1 = HIGH;
  static int lastSwState2 = HIGH;
  static int lastSwState3 = HIGH;
  static unsigned long lastDebounceMs1 = 0;
  static unsigned long lastDebounceMs2 = 0;
  static unsigned long lastDebounceMs3 = 0;
  constexpr unsigned long SW_DEBOUNCE_DELAY_MS = 25; // 25ms snappy debounce

  // ---- Asynchronous / Debounced NVS Flash Persistence ----
  static bool pendingSave = false;
  static unsigned long lastSaveTriggerMs = 0;
  constexpr unsigned long NVS_SAVE_DEBOUNCE_MS = 250; // Save to flash 250ms after switches settle

  // ---------------------------------------------------------------------------
  // saveStates() — Persist light and boom gate states to NVS Flash
  // ---------------------------------------------------------------------------
  static void saveStates() {
    devPrefs.begin("dev_state", false);
    devPrefs.putBool("l1", stateLight1);
    devPrefs.putBool("l2", stateLight2);
    devPrefs.putBool("l3", stateLight3);
    devPrefs.putBool("l4", stateLight4);
    devPrefs.putBool("l5", stateLight5);
    devPrefs.putBool("pty", stateParty);
    devPrefs.putBool("gate", stateGateOpen);
    devPrefs.end();
  }

  // Request a flash save without blocking the main loop or delaying physical switches
  static void scheduleSave() {
    pendingSave = true;
    lastSaveTriggerMs = millis();
  }

  // ---- Sound Sensor State & Double Clap Detection Timing ----
  static int lastSoundState = HIGH;
  static int clapCount = 0;
  static unsigned long firstClapTimeMs = 0;
  static unsigned long lastClapActionMs = 0;
  constexpr unsigned long CLAP_MIN_INTERVAL_MS = 150; // Echo lockout after 1st clap
  constexpr unsigned long CLAP_MAX_INTERVAL_MS = 650; // Max time window for 2nd clap
  constexpr unsigned long CLAP_COOLDOWN_MS     = 600; // Cooldown after double clap trigger

  // ---------------------------------------------------------------------------
  // loadStates() — Restore states from NVS Flash across reboots
  // ---------------------------------------------------------------------------
  static void loadStates() {
    devPrefs.begin("dev_state", true); // read-only
    stateLight1 = devPrefs.getBool("l1", false);
    stateLight2 = devPrefs.getBool("l2", false);
    stateLight3 = devPrefs.getBool("l3", false);
    stateLight4 = devPrefs.getBool("l4", false);
    stateLight5 = devPrefs.getBool("l5", false);
    stateParty  = devPrefs.getBool("pty", false);
    stateGateOpen = devPrefs.getBool("gate", false);
    currentGateAngle = stateGateOpen ? GATE_ANGLE_OPEN : GATE_ANGLE_CLOSED;
    targetGateAngle  = currentGateAngle;
    devPrefs.end();

    Serial.println("[DEVICE] Restored actuator states from NVS Flash:");
    Serial.printf("  Light 1: %s | Light 2: %s | Light 3: %s\n",
                  stateLight1 ? "ON" : "OFF", stateLight2 ? "ON" : "OFF", stateLight3 ? "ON" : "OFF");
    Serial.printf("  Light 4: %s | Light 5: %s | Party: %s | Gate: %s (%d°)\n",
                  stateLight4 ? "ON" : "OFF", stateLight5 ? "ON" : "OFF",
                  stateParty ? "ON" : "OFF", stateGateOpen ? "OPEN" : "CLOSED", currentGateAngle);
  }

  // ---------------------------------------------------------------------------
  // parsePinString() — Extract GPIO pin integer from strings like "GPIO 12", "12"
  // ---------------------------------------------------------------------------
  int parsePinString(const char* pinStr) {
    if (!pinStr || strlen(pinStr) == 0) return -1;
    const char* ptr = pinStr;
    while (*ptr) {
      if (isdigit(*ptr)) {
        return atoi(ptr);
      }
      ptr++;
    }
    return -1;
  }

  // ---------------------------------------------------------------------------
  // init() — Configure GPIO pins and restore last saved state from NVS Flash
  // ---------------------------------------------------------------------------
  void init() {
    loadStates(); // Read previous states from NVS Flash

    pinMode(PIN_LIGHT1, OUTPUT);
    digitalWrite(PIN_LIGHT1, stateLight1 ? HIGH : LOW);

    pinMode(PIN_LIGHT2, OUTPUT);
    digitalWrite(PIN_LIGHT2, stateLight2 ? HIGH : LOW);

    pinMode(PIN_LIGHT3, OUTPUT);
    digitalWrite(PIN_LIGHT3, stateLight3 ? HIGH : LOW);

    pinMode(PIN_LIGHT4, OUTPUT);
    digitalWrite(PIN_LIGHT4, stateLight4 ? HIGH : LOW);

    pinMode(PIN_LIGHT5, OUTPUT);
    digitalWrite(PIN_LIGHT5, stateLight5 ? HIGH : LOW);

    pinMode(PIN_PARTY_LIGHT, OUTPUT);
    digitalWrite(PIN_PARTY_LIGHT, stateParty ? HIGH : LOW);

    // Configure Dedicated LDR Sensor (GPIO 34, ADC1_CH6) and Relay (GPIO 23)
    // Pure real-time sensing: NO storage / NVS flash used for LDR data
    pinMode(PIN_LDR, INPUT);
    pinMode(PIN_RELAY_LDR, OUTPUT);

    digitalWrite(PIN_RELAY_LDR, LOW);

    // Configure Physical Rocker Switches on GPIO 19, 21, 22
    pinMode(PIN_SW_LIGHT1, INPUT_PULLUP);
    pinMode(PIN_SW_LIGHT2, INPUT_PULLUP);
    pinMode(PIN_SW_LIGHT3, INPUT_PULLUP);

    delay(10); // Settle pull-ups
    lastSwState1 = digitalRead(PIN_SW_LIGHT1);
    lastSwState2 = digitalRead(PIN_SW_LIGHT2);
    lastSwState3 = digitalRead(PIN_SW_LIGHT3);

    // Follow physical rocker switch position directly on boot:
    // 'O' position -> ON, '-' position -> OFF
    stateLight1 = SWITCH_O_IS_HIGH ? (lastSwState1 == HIGH) : (lastSwState1 == LOW);
    stateLight2 = SWITCH_O_IS_HIGH ? (lastSwState2 == HIGH) : (lastSwState2 == LOW);
    stateLight3 = SWITCH_O_IS_HIGH ? (lastSwState3 == HIGH) : (lastSwState3 == LOW);

    digitalWrite(PIN_LIGHT1, stateLight1 ? HIGH : LOW);
    digitalWrite(PIN_LIGHT2, stateLight2 ? HIGH : LOW);
    digitalWrite(PIN_LIGHT3, stateLight3 ? HIGH : LOW);
    saveStates();

    // Configure Boom Gate SG90 Servo (GPIO 27, 0° to 90°)
    initServoPwm(servoPin);
    writeServoDuty(currentGateAngle);

    // Configure Sound Sensor (GPIO 35) for Double Clap Detection
    pinMode(PIN_SOUND_SENSOR, INPUT);
    lastSoundState = digitalRead(PIN_SOUND_SENSOR);

    Serial.println("[DEVICE] All lights & rocker switches GPIO initialized.");
    Serial.printf("[BOOM GATE] Servo initialized on GPIO %d -> %s (%d°)\n",
                  servoPin, stateGateOpen ? "OPEN" : "CLOSED", currentGateAngle);
    Serial.printf("[SWITCH] Boot positions: Light1=%s, Light2=%s, Light3=%s\n",
                  stateLight1 ? "ON (O)" : "OFF (-)",
                  stateLight2 ? "ON (O)" : "OFF (-)",
                  stateLight3 ? "ON (O)" : "OFF (-)");
    Serial.printf("[SOUND] Initialized on GPIO %d -> Double Clap targets '%s' (%s)\n",
                  PIN_SOUND_SENSOR, SOUND_TARGET, SOUND_TOGGLE_MODE ? "Toggle" : "Turn ON");
  }

  // ---------------------------------------------------------------------------
  // update() — Periodic hardware tasks (LDR, Rocker Switches, Smooth Servo)
  // ---------------------------------------------------------------------------
  void update() {
    unsigned long now = millis();

    // -------------------------------------------------------------------------
    // Rocker Switches Polling: 'O' = ON | '-' = OFF
    // -------------------------------------------------------------------------
    // Rocker Switch 1 (GPIO 19 -> Light 1)
    int read1 = digitalRead(PIN_SW_LIGHT1);
    if (read1 != lastSwState1 && (now - lastDebounceMs1) > SW_DEBOUNCE_DELAY_MS) {
      lastDebounceMs1 = now;
      lastSwState1 = read1;
      bool targetState = SWITCH_O_IS_HIGH ? (read1 == HIGH) : (read1 == LOW);
      if (stateLight1 != targetState) {
        setLight("light-1", targetState);
        AppWebSocket::sendStateReport("light-1", targetState ? "{\"on\":true}" : "{\"on\":false}");
        Serial.printf("[SWITCH] Switch 1 pushed to '%s' -> Light 1 is %s & Synced to Web\n",
                      targetState ? "O" : "-", targetState ? "ON" : "OFF");
      }
    }

    // Rocker Switch 2 (GPIO 21 -> Light 2)
    int read2 = digitalRead(PIN_SW_LIGHT2);
    if (read2 != lastSwState2 && (now - lastDebounceMs2) > SW_DEBOUNCE_DELAY_MS) {
      lastDebounceMs2 = now;
      lastSwState2 = read2;
      bool targetState = SWITCH_O_IS_HIGH ? (read2 == HIGH) : (read2 == LOW);
      if (stateLight2 != targetState) {
        setLight("light-2", targetState);
        AppWebSocket::sendStateReport("light-2", targetState ? "{\"on\":true}" : "{\"on\":false}");
        Serial.printf("[SWITCH] Switch 2 pushed to '%s' -> Light 2 is %s & Synced to Web\n",
                      targetState ? "O" : "-", targetState ? "ON" : "OFF");
      }
    }

    // Rocker Switch 3 (GPIO 22 -> Light 3)
    int read3 = digitalRead(PIN_SW_LIGHT3);
    if (read3 != lastSwState3 && (now - lastDebounceMs3) > SW_DEBOUNCE_DELAY_MS) {
      lastDebounceMs3 = now;
      lastSwState3 = read3;
      bool targetState = SWITCH_O_IS_HIGH ? (read3 == HIGH) : (read3 == LOW);
      if (stateLight3 != targetState) {
        setLight("light-3", targetState);
        AppWebSocket::sendStateReport("light-3", targetState ? "{\"on\":true}" : "{\"on\":false}");
        Serial.printf("[SWITCH] Switch 3 pushed to '%s' -> Light 3 is %s & Synced to Web\n",
                      targetState ? "O" : "-", targetState ? "ON" : "OFF");
      }
    }

    // -------------------------------------------------------------------------
    // LDR Sensor (GPIO 34) Controls Light Relay (GPIO 23): Pure Real-Time (No Storage)
    // Analog == 0 -> Night (Light ON) | Analog > 0 -> Day (Light OFF)
    // -------------------------------------------------------------------------
    if (now - lastLdrCheckMs >= LDR_CHECK_INTERVAL_MS) {
      lastLdrCheckMs = now;
      int rawLdr = analogRead(PIN_LDR);

      // 0 = Night -> Turn ON light (Relay Pin 23 HIGH)
      // > 0 = Day -> Turn OFF light (Relay Pin 23 LOW)
      // Pure in-memory check, zero storage writes
      bool desiredRelay = (rawLdr == 0);

      if (desiredRelay != stateRelayLdr) {
        stateRelayLdr = desiredRelay;
        digitalWrite(PIN_RELAY_LDR, stateRelayLdr ? HIGH : LOW);
        Serial.printf("[LDR->LIGHT] Analog: %d -> %s detected. Light (GPIO %d) is %s\n",
                      rawLdr, stateRelayLdr ? "NIGHT (0)" : "DAY (>0)", PIN_RELAY_LDR, stateRelayLdr ? "ON" : "OFF");
      }
    }

    // -------------------------------------------------------------------------
    // Smooth Boom Gate Servo Motion (Non-blocking stepping 15ms/degree)
    // -------------------------------------------------------------------------
    if (currentGateAngle != targetGateAngle) {
      if (now - lastServoStepMs >= SERVO_STEP_INTERVAL_MS) {
        lastServoStepMs = now;
        if (targetGateAngle > currentGateAngle) {
          currentGateAngle++;
        } else {
          currentGateAngle--;
        }
        writeServoDuty(currentGateAngle);
      }
    }

    // -------------------------------------------------------------------------
    // Sound Sensor: Double Clap Detection (👏 👏)
    // -------------------------------------------------------------------------
    int soundVal = digitalRead(PIN_SOUND_SENSOR);
    bool soundTriggered = SOUND_ACTIVE_LOW ? (soundVal == LOW && lastSoundState == HIGH)
                                           : (soundVal == HIGH && lastSoundState == LOW);
    lastSoundState = soundVal;

    // Cooldown check (prevent immediate re-trigger after action)
    if (now - lastClapActionMs > CLAP_COOLDOWN_MS) {
      if (soundTriggered) {
        if (clapCount == 0) {
          // First clap registered
          clapCount = 1;
          firstClapTimeMs = now;
          Serial.println("[SOUND] 👏 First clap detected! Listening for 2nd clap...");
        } else if (clapCount == 1) {
          unsigned long interval = now - firstClapTimeMs;
          if (interval >= CLAP_MIN_INTERVAL_MS && interval <= CLAP_MAX_INTERVAL_MS) {
            // Second clap registered within valid window!
            clapCount = 0;
            lastClapActionMs = now;

            bool curState = getLightState(SOUND_TARGET);
            bool newState = SOUND_TOGGLE_MODE ? !curState : true;
            setLight(SOUND_TARGET, newState);
            AppWebSocket::sendStateReport(SOUND_TARGET, newState ? "{\"on\":true}" : "{\"on\":false}");

            Serial.printf("[SOUND] 👏 👏 Double Clap Confirmed! (%lums) -> %s is %s & Synced\n",
                          interval, SOUND_TARGET, newState ? "ON" : "OFF");
          }
        }
      }

      // Check for timeout of first clap (user didn't clap a 2nd time)
      if (clapCount == 1 && (now - firstClapTimeMs > CLAP_MAX_INTERVAL_MS)) {
        clapCount = 0;
        Serial.println("[SOUND] Single clap timed out (ignored).");
      }
    }

    // -------------------------------------------------------------------------
    // Deferred NVS Flash Persistence (Flush pending saves smoothly without lag)
    // -------------------------------------------------------------------------
    if (pendingSave && (now - lastSaveTriggerMs >= NVS_SAVE_DEBOUNCE_MS)) {
      pendingSave = false;
      saveStates();
    }
  }

  // ---------------------------------------------------------------------------
  // setAllLights() — Batch toggle all zonal lights (Light 1 to 5) & save to NVS
  // ---------------------------------------------------------------------------
  void setAllLights(bool on) {
    digitalWrite(PIN_LIGHT1, on ? HIGH : LOW);
    digitalWrite(PIN_LIGHT2, on ? HIGH : LOW);
    digitalWrite(PIN_LIGHT3, on ? HIGH : LOW);
    digitalWrite(PIN_LIGHT4, on ? HIGH : LOW);
    digitalWrite(PIN_LIGHT5, on ? HIGH : LOW);
    stateLight1 = on;
    stateLight2 = on;
    stateLight3 = on;
    stateLight4 = on;
    stateLight5 = on;
    scheduleSave(); // Persist smoothly without blocking
    Serial.printf("[DEVICE] All zonal lights (1-5) -> %s\n", on ? "ON" : "OFF");
  }

  // ---------------------------------------------------------------------------
  // setLight() — Toggle a light by its device ID string or dynamic GPIO pin
  // ---------------------------------------------------------------------------
  void setLight(const char* deviceId, bool on, int explicitPin) {
    if (!deviceId && explicitPin < 0) return;

    if (deviceId) {
      if (strcmp(deviceId, "light-1") == 0 || strcmp(deviceId, "light1") == 0) {
        int pin = (explicitPin >= 0) ? explicitPin : PIN_LIGHT1;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, on ? HIGH : LOW);
        stateLight1 = on;
        scheduleSave();
        Serial.printf("[DEVICE] Light 1 (GPIO %d) -> %s\n", pin, on ? "ON" : "OFF");
        return;
      }
      else if (strcmp(deviceId, "light-2") == 0 || strcmp(deviceId, "light2") == 0) {
        int pin = (explicitPin >= 0) ? explicitPin : PIN_LIGHT2;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, on ? HIGH : LOW);
        stateLight2 = on;
        scheduleSave();
        Serial.printf("[DEVICE] Light 2 (GPIO %d) -> %s\n", pin, on ? "ON" : "OFF");
        return;
      }
      else if (strcmp(deviceId, "light-3") == 0 || strcmp(deviceId, "light3") == 0) {
        int pin = (explicitPin >= 0) ? explicitPin : PIN_LIGHT3;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, on ? HIGH : LOW);
        stateLight3 = on;
        scheduleSave();
        Serial.printf("[DEVICE] Light 3 (GPIO %d) -> %s\n", pin, on ? "ON" : "OFF");
        return;
      }
      else if (strcmp(deviceId, "light-4") == 0 || strcmp(deviceId, "light4") == 0) {
        int pin = (explicitPin >= 0) ? explicitPin : PIN_LIGHT4;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, on ? HIGH : LOW);
        stateLight4 = on;
        scheduleSave();
        Serial.printf("[DEVICE] Light 4 (GPIO %d) -> %s\n", pin, on ? "ON" : "OFF");
        return;
      }
      else if (strcmp(deviceId, "light-5") == 0 || strcmp(deviceId, "light5") == 0) {
        int pin = (explicitPin >= 0) ? explicitPin : PIN_LIGHT5;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, on ? HIGH : LOW);
        stateLight5 = on;
        scheduleSave();
        Serial.printf("[DEVICE] Light 5 (GPIO %d) -> %s\n", pin, on ? "ON" : "OFF");
        return;
      }
      else if (strcmp(deviceId, "party-light") == 0 || strcmp(deviceId, "party") == 0) {
        int pin = (explicitPin >= 0) ? explicitPin : PIN_PARTY_LIGHT;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, on ? HIGH : LOW);
        stateParty = on;
        scheduleSave();
        Serial.printf("[DEVICE] Party Light (GPIO %d) -> %s\n", pin, on ? "ON" : "OFF");
        return;
      }
      else if (strcmp(deviceId, "all_lights") == 0 || strcmp(deviceId, "all") == 0) {
        setAllLights(on);
        return;
      }
    }

    // Dynamic light on custom pin (e.g. GPIO 14, 27)
    int targetPin = (explicitPin >= 0) ? explicitPin : (deviceId ? parsePinString(deviceId) : -1);
    if (targetPin >= 0 && targetPin <= 39) {
      pinMode(targetPin, OUTPUT);
      digitalWrite(targetPin, on ? HIGH : LOW);
      Serial.printf("[DEVICE] Dynamic device %s (GPIO %d) -> %s\n", deviceId ? deviceId : "custom", targetPin, on ? "ON" : "OFF");
    } else {
      Serial.printf("[DEVICE] Unknown light device: %s\n", deviceId ? deviceId : "null");
    }
  }

  // ---------------------------------------------------------------------------
  // getLightState() — Query the current on/off state of a light
  // ---------------------------------------------------------------------------
  bool getLightState(const char* deviceId) {
    if (!deviceId) return false;
    if (strcmp(deviceId, "light-1") == 0 || strcmp(deviceId, "light1") == 0) return stateLight1;
    if (strcmp(deviceId, "light-2") == 0 || strcmp(deviceId, "light2") == 0) return stateLight2;
    if (strcmp(deviceId, "light-3") == 0 || strcmp(deviceId, "light3") == 0) return stateLight3;
    if (strcmp(deviceId, "light-4") == 0 || strcmp(deviceId, "light4") == 0) return stateLight4;
    if (strcmp(deviceId, "light-5") == 0 || strcmp(deviceId, "light5") == 0) return stateLight5;
    if (strcmp(deviceId, "party-light") == 0 || strcmp(deviceId, "party") == 0) return stateParty;
    return false;
  }

  // ---------------------------------------------------------------------------
  // setGate() — Open (90°) or Close (0°) Boom Gate
  // ---------------------------------------------------------------------------
  void setGate(bool open, int explicitPin) {
    if (explicitPin >= 0 && explicitPin != servoPin) {
      setGatePin(explicitPin);
    }
    stateGateOpen = open;
    targetGateAngle = open ? GATE_ANGLE_OPEN : GATE_ANGLE_CLOSED;
    scheduleSave();
    Serial.printf("[BOOM GATE] Gate set to %s (Target %d°, GPIO %d)\n",
                  open ? "OPEN" : "CLOSED", targetGateAngle, servoPin);
  }

  // ---------------------------------------------------------------------------
  // isGateOpen() — Query current open/close status
  // ---------------------------------------------------------------------------
  bool isGateOpen() {
    return stateGateOpen;
  }

  // ---------------------------------------------------------------------------
  // getGateAngle() — Query current servo position angle (0 - 90)
  // ---------------------------------------------------------------------------
  int getGateAngle() {
    return currentGateAngle;
  }

  // ---------------------------------------------------------------------------
  // setGatePin() — Reconfigure servo GPIO pin dynamically
  // ---------------------------------------------------------------------------
  void setGatePin(int pin) {
    if (pin >= 0 && pin <= 39 && pin != servoPin) {
      servoPin = pin;
      initServoPwm(servoPin);
      writeServoDuty(currentGateAngle);
      Serial.printf("[BOOM GATE] Servo pin reconfigured to GPIO %d\n", servoPin);
    }
  }

  // ---------------------------------------------------------------------------
  // getJsonState() — Build a JSON object with all device states
  // ---------------------------------------------------------------------------
  String getJsonState() {
    String json = "{";
    json += "\"light1\":" + String(stateLight1 ? "true" : "false") + ",";
    json += "\"light2\":" + String(stateLight2 ? "true" : "false") + ",";
    json += "\"light3\":" + String(stateLight3 ? "true" : "false") + ",";
    json += "\"light4\":" + String(stateLight4 ? "true" : "false") + ",";
    json += "\"light5\":" + String(stateLight5 ? "true" : "false") + ",";
    json += "\"party\":"  + String(stateParty  ? "true" : "false") + ",";
    json += "\"gate\":"   + String(stateGateOpen ? "true" : "false") + ",";
    json += "\"boomGate\":\"" + String(stateGateOpen ? "open" : "closed") + "\",";
    json += "\"gateAngle\":" + String(currentGateAngle);
    json += "}";
    return json;
  }

  // ---------------------------------------------------------------------------
  // syncAllToCloud() — Broadcast all restored device states to cloud WebSocket
  // ---------------------------------------------------------------------------
  void syncAllToCloud() {
    AppWebSocket::sendStateReport("light-1", stateLight1 ? "{\"on\":true}" : "{\"on\":false}");
    AppWebSocket::sendStateReport("light-2", stateLight2 ? "{\"on\":true}" : "{\"on\":false}");
    AppWebSocket::sendStateReport("light-3", stateLight3 ? "{\"on\":true}" : "{\"on\":false}");
    AppWebSocket::sendStateReport("light-4", stateLight4 ? "{\"on\":true}" : "{\"on\":false}");
    AppWebSocket::sendStateReport("light-5", stateLight5 ? "{\"on\":true}" : "{\"on\":false}");
    AppWebSocket::sendStateReport("party-light", stateParty ? "{\"on\":true}" : "{\"on\":false}");
    AppWebSocket::sendStateReport("boom-gate", stateGateOpen ? "{\"status\":\"open\"}" : "{\"status\":\"closed\"}");
    Serial.println("[DEVICE] All restored actuator states (including Boom Gate) reported to cloud WebSocket.");
  }
}
