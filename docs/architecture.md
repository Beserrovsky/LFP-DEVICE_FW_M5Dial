# M5Dial NFC Survey Architecture

## Overview

The project is organized into five modular libraries, each responsible for a specific subsystem. All modules currently provide **compile-ready skeletons** and are designed to be integrated incrementally using the existing reference implementations.

---

## DisplayDriver

**Purpose**

Manages the M5Stack Dial display and LVGL rendering pipeline.

**Responsibilities**

- Initialize LVGL
- Configure display buffers and drivers
- Handle screen updates
- Perform horizontal mirroring for the Pepper's Ghost effect
- Manage display refresh callbacks

**Interface**

```cpp
init();
update();
flush();
setMirrorEnabled(bool enabled);
```

---

## NFCManager

**Purpose**

Provides a high-level interface for the PN532 NFC reader.

**Responsibilities**

- Initialize PN532 communication
- Detect NFC tags
- Store and expose detected UID information
- Manage NFC polling

**Interface**

```cpp
init();
update();

tagDetected();

getTagUID();
getTagLength();

clearDetection();
```

---

## ESPNowManager

**Purpose**

Handles wireless communication between the M5Dial and the receiver using ESP-NOW.

**Responsibilities**

- Initialize ESP-NOW
- Register peers
- Send survey data packets
- Handle acknowledgements (ACK)
- Synchronize packet delivery status

**Interface**

```cpp
init();

sendPacket();
sendPacketWithAck();

ackReceived();
ackAccepted();
```

**Data Structures**

Includes a `SurveyPacket` structure matching the reference implementation.

---

## UIManager

**Purpose**

Acts as the abstraction layer between application logic and the generated SquareLine/LVGL UI.

**Responsibilities**

- Manage screen transitions
- Update UI elements
- Encapsulate generated UI objects
- Prevent application logic from being placed inside generated files

**Screens**

- Splash
- Question
- Text Entry
- Sending
- Success
- Error

**Interface**

```cpp
showSplash();

showQuestion();

showSuccess();

showError();
```

---

## AppState

**Purpose**

Maintains persistent application state and survey progress.

**Responsibilities**

- Store survey answers
- Track current question
- Store current selection
- Manage user name entry
- Save and restore state from NVS

**Persistence**

```cpp
loadFromNVS();
saveToNVS();
```

**Interface**

Provides getters and setters for all application state fields.

Examples:

```cpp
getCurrentQuestion();
setCurrentQuestion();

getCurrentOption();
setCurrentOption();

getNameBuffer();
setNameBuffer();
```

---

## Global Instances

Each module exports a singleton instance that can be accessed throughout the application.

Example:

```cpp
g_displayDriver
g_nfcManager
g_espNowManager
g_uiManager
g_appState
```

These instances are intended to be initialized during startup and accessed from `main.cpp`.

---

## Application Startup Flow

```text
main.cpp
│
├── DisplayDriver.init()
├── UIManager.init()
├── NFCManager.init()
├── ESPNowManager.init()
└── AppState.loadFromNVS()
```

---

## High-Level Architecture

```text
                ┌─────────────┐
                │  AppState   │
                └──────┬──────┘
                       │
                       ▼
                ┌─────────────┐
                │ UIManager   │
                └──────┬──────┘
                       │
        ┌──────────────┼──────────────┐
        ▼                             ▼
┌─────────────┐               ┌─────────────┐
│ NFCManager  │               │ESPNowManager│
└─────────────┘               └─────────────┘
                       ▲
                       │
                ┌─────────────┐
                │DisplayDriver│
                └─────────────┘
```

---

## Design Principles

- Keep generated SquareLine files isolated in `/ui`
- Do not place application logic inside generated files
- Maintain strict separation between UI, NFC, communication, and state
- Preserve behavior from the reference sketches during migration
- Keep modules independently testable
- Commit after each successful subsystem migration