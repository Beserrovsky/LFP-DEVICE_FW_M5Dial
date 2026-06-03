# M5Dial Embedded System — Project Context for Claude


### NFC

Must read UID and NDEF JSON object, have an interface to translate it.



Claude:
- Implementar biblioteca de NFC e testar
- Implementar Maquina de estados alinhada com a experiência total, wrappers da UI nova (completa)

Final:
- Debug ESP-NOW com sistema completo

## 🎯 Goal

This project is a modular embedded firmware for the **M5Stack Dial (ESP32-S3)**.

It combines:

- LVGL 8.4 UI (SquareLine-generated screens)
- ESP-NOW communication (survey submission + ACK protocol)
- PN532 NFC tag detection (I2C)
- Rotary encoder input (M5Dial hardware)
- Modular C++ architecture (PlatformIO)

The system is still under active development and must be extended WITHOUT breaking existing working subsystems.

---

# 🧠 Current Architecture Status

## ✅ Already working

### 1. LVGL Display System
- LVGL 8.4 integrated via PlatformIO
- Custom lv_conf.h is correctly loaded
- M5Dial display driver working
- UI rendered from SquareLine export (`ui/GeneratedUI`)
- Encoder updates a single question screen

### 2. ESP-NOW (reference implementation exists)
- Sender + ACK system already proven working in standalone sketch
- Packet structure is defined and stable
- Receiver MAC is fixed
- ACK contains:
  - device_id
  - counter
  - accepted flag

### 3. M5Dial Hardware
- Encoder works
- Button works
- Display works
- Basic audio feedback works

---

## ⚠️ Partially integrated

### ESP-NOW
- Needs modularization into `ESPNowManager`
- Must preserve:
  - retry logic
  - ACK sync
  - packet structure
  - peer registration

### UI system
- Only QUESTION screen is actively used
- Splash / success / error screens are placeholders (stub or not implemented yet)
- UIManager not fully integrated

---

## ❌ Not yet integrated

### NFC (PN532)
- Working standalone in test sketch
- Must be moved into `NFCManager`
- Must NOT change logic during migration

Requirements:
- `Wire.begin(13, 15)` MUST be preserved
- Must expose:
  - `bool hasNewTag()`
  - `String getLastUID()`

---

### AppState system
- Not implemented yet
- Intended for:
  - storing survey progress
  - selected options
  - name buffer
  - persistence (NVS later)

---

# 📦 Existing Modules (target architecture)

## DisplayDriver
- Handles LVGL flush callback
- Handles mirror mode (Pepper’s Ghost effect)
- Wraps M5Dial.Display

Interface:
- init()
- flush()
- setMirrorEnabled()

---

## ESPNowManager (TO BE COMPLETED)

Responsible for:
- ESP-NOW initialization
- peer registration
- sending SurveyPacket
- ACK handling
- retry logic

Must expose:

- bool begin()
- bool sendSurvey(const SurveyPacket&)
- bool isAckReceived()
- bool isAckAccepted()

Must be headless (NO UI DEPENDENCIES)

---

## NFCManager (TO BE CREATED / REFACTORED)

Wraps PN532 (Adafruit library)

Constraints:
- Keep Wire.begin(13,15)
- Do NOT modify PN532 logic

Must expose:
- bool hasNewTag()
- String getLastUID()

---

## UIManager (NOT COMPLETE YET)

Will later handle:
- switching screens
- updating LVGL labels
- mapping AppState → UI

DO NOT implement ESP-NOW or NFC logic here.

---

## AppState (NOT IMPLEMENTED YET)

Will store:
- currentQuestion index
- selectedOptions[]
- nameBuffer
- survey state
- persistence (NVS later)

---

# 📊 Data Model

## SurveyPacket (DO NOT MODIFY STRUCTURE)

```cpp
struct SurveyPacket {
  uint8_t packetType;
  uint8_t device_id;
  uint32_t counter;
  uint32_t timestamp_ms;

  int innovation;
  int satisfactionIndex;
  int nps;

  char fishType[16];
  char fishColour[16];
  char name[12];
};