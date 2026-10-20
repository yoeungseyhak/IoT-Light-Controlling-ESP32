#pragma once
// =============================================================================
// DeviceManager.h — Hardware GPIO & Actuator Control
// =============================================================================
// Abstracts all hardware pin definitions, LED states, and physical I/O from
// the networking layer. Controls all zonal lights matching the web dashboard:
//   - Light 1 (Zone A) : GPIO 2 (Board LED)
//   - Light 2 (Zone B) : GPIO 4
//   - Light 3 (Zone C) : GPIO 5
//   - Party / Beacon   : GPIO 23
// =============================================================================

#include <Arduino.h>

namespace DeviceManager {

  // ---- Pin Assignments (Matching Cloud & Web Dashboard) ----
  // Building Lights (3 Lights Only: Light 1, 2, 3)
  constexpr int PIN_LIGHT1          = 4;    // Building Light 1 (Zone A)
  constexpr int PIN_LIGHT2          = 5;    // Building Light 2 (Zone B)
  constexpr int PIN_LIGHT3          = 18;   // Building Light 3 (Zone C)
  constexpr int PIN_PARTY_LIGHT     = 2;    // Party Light ONLY (No other component shares Pin 2)

  // ---- Dedicated LDR Sensor & Relay Control (No storage, pure real-time) ----
  constexpr int PIN_LDR                        = 34;   // LDR Sensor Input (GPIO 34, ADC1_CH6)
  constexpr int PIN_RELAY_LDR                  = 23;   // Relay / Light Output Pin (GPIO 23)
  // LDR Dark / Night Threshold (0 - 4095):
  // With 10k resistor to GND & LDR to 3.3V, dark reading is low (< 300)
  constexpr int LDR_DARK_THRESHOLD             = 300;  // Readings below this value trigger Night (Light ON)
  constexpr bool LDR_DARK_IS_LOW               = true; // true: Dark = Low ADC (LDR to 3.3V, 10k to GND); false: Dark = High ADC

  // ---- Light Relay on Pin 14 & 5 Switch Buttons ----
  constexpr int PIN_RELAY_SWITCH_HOLD          = 14;   // Light Relay on Pin 14 (ON while ANY of 5 switches held, OFF when released)
  constexpr int PIN_SW_HOLD1                   = 13;   // Switch Button 1 for Pin 14 (D13)
  constexpr int PIN_SW_HOLD2                   = 25;   // Switch Button 2 for Pin 14 (D25)
  constexpr int PIN_SW_HOLD3                   = 26;   // Switch Button 3 for Pin 14 (D26)
  constexpr int PIN_SW_HOLD4                   = 32;   // Switch Button 4 for Pin 14 (D32)
  constexpr int PIN_SW_HOLD5                   = 33;   // Switch Button 5 for Pin 14 (D33)

  // ---- Added Relay 2: Sound Sensor Relay (GPIO 15) ----
  constexpr int PIN_RELAY_SOUND                = 15;   // Toggles ON/OFF each time sound/clap is detected

  // ---- 3 Building Light Switches (Strictly for Building Lights 1, 2, 3) ----
  constexpr int PIN_SW_LIGHT1       = 19;   // Rocker Switch 1 (GPIO 19 -> Building Light 1)
  constexpr int PIN_SW_LIGHT2       = 21;   // Rocker Switch 2 (GPIO 21 -> Building Light 2)
  constexpr int PIN_SW_LIGHT3       = 22;   // Rocker Switch 3 (GPIO 22 -> Building Light 3)
  // Switch Polarity with INPUT_PULLUP:
  // Set to true if 'O' position is open (HIGH) and '-' is GND (LOW) [Standard]
  // Set to false if 'O' position is GND (LOW) and '-' is open (HIGH)
  constexpr bool SWITCH_O_IS_HIGH   = true;


  // ---- Boom Gate (SG90 Servo 180° restricted to 0° - 90°) ----
  constexpr int PIN_BOOM_GATE_SERVO = 27;  // SG90 Servo Signal (PWM GPIO 27)
  constexpr int GATE_ANGLE_CLOSED   = 0;   // 0° = Barrier horizontal (Closed)
  constexpr int GATE_ANGLE_OPEN     = 90;  // 90° = Barrier vertical (Open)
  constexpr bool GATE_INVERT_DIR    = false; // Set to true if physical mounting is inverted (90° Closed, 0° Open)
  constexpr unsigned long GATE_AUTO_CLOSE_MS = 10UL * 60UL * 1000UL; // 10 minutes auto-close countdown

  // ---- Sound Sensor (Supports both AO Analog and DO Digital) ----
  constexpr int PIN_SOUND_SENSOR            = 35;          // Sound Sensor connected to GPIO 35 (ADC1_CH7)
  constexpr bool SOUND_IS_ANALOG            = true;        // Set to true when connected to AO; false if connected to DO
  constexpr int SOUND_ANALOG_THRESHOLD      = 400;         // Sensitivity for AO: sound spike deviation above baseline (0 - 4095)
  constexpr bool SOUND_ACTIVE_LOW           = true;        // For DO pin only: standard LM393 sensor pulls DO LOW on sound
  constexpr const char* SOUND_TARGET        = "light-1";   // Target device ("light-1", "all_lights", etc.)
  constexpr bool SOUND_TOGGLE_MODE          = true;        // true: toggle ON/OFF, false: turn ON only
  constexpr bool SOUND_REQUIRE_DOUBLE_CLAP  = false;       // false: toggle on any single sound/clap; true: require 2 claps



  // ---- Lifecycle ----
  void init();                                    // Configure all GPIO pins & LDR sensor
  void update();                                  // Periodic tasks (LDR, Switches, Smooth Servo stepping)

  // ---- Dynamic Pin Parser ----
  int parsePinString(const char* pinStr);         // Extracts GPIO pin number from "GPIO 12", "12", etc.

  // ---- Device Control ----
  void setLight(const char* deviceId, bool on, int explicitPin = -1); // Toggle light by device ID or dynamic pin
  void setAllLights(bool on);                     // Batch toggle all zonal lights (1 to 5)
  bool getLightState(const char* deviceId);        // Query current state by device ID

  // ---- Boom Gate Control ----
  void setGate(bool open, int explicitPin = -1);  // Open (90°) or Close (0°) Boom Gate
  bool isGateOpen();                              // Returns true if gate is currently opened
  int  getGateAngle();                            // Returns current servo angle (0 to 90)
  void setGatePin(int pin);                       // Reconfigure servo GPIO pin dynamically
  unsigned long getGateAutoCloseRemainingSec();   // Remaining seconds before auto-close (0 if closed)

  // ---- Status Reporting & Cloud Sync ----
  String getJsonState();                          // Returns full device state as JSON
  void   syncAllToCloud();                        // Report all restored device states to cloud
}
