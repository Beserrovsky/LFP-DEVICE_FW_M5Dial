# M5Dial NFC Survey Firmware

## Project Goal

Firmware for the M5Stack Dial (ESP32-S3) used to collect survey responses through a rotary encoder interface.

Main subsystems:

- LVGL 8.4
- SquareLine Studio generated UI
- ESP-NOW communication
- PN532 NFC reader
- M5Dial encoder + button
- Pepper's Ghost mirrored display mode

The project follows a strict modular architecture.

Generated files must remain isolated inside:

```text
/lib/GeneratedUI
```

Application logic must never be added to generated files.

---

# Current Development Priority

## Phase 1 (Current)

Implement the complete UI flow described in:

```text
/docs/ux-flow.md
```

using the latest SquareLine export already available in:

```text
/lib/GeneratedUI
```

Focus ONLY on:

- UIManager
- AppState
- Input handling
- State machine

NFC and ESP-NOW may be temporarily mocked if necessary.

### Goal

Deliver a fully functional survey flow including:

- Encoder navigation
- Button interactions
- Screen transitions
- Tutorial flow
- Question flow
- Confirmation flow
- Success/Error flow

before integrating NFC.

---

## Phase 2

Implement `NFCManager` based on:

```text
/reference/pn532_demo.ino
```

### Requirements

Preserve existing PN532 behavior.

Preserve I2C configuration:

```cpp
Wire.begin(13, 15);
```

Required capabilities:

- Read UID
- Read NDEF payload
- Extract JSON object from NDEF
- Validate identifier format
- Provide translation interface

Suggested API:

```cpp
bool hasNewTag();

String getUID();

String getNDEFPayload();

bool isValidTag();

String getValidationError();

void clearDetection();
```

---

## Phase 3

Integrate ESP-NOW with the completed application flow.

Use the existing proven implementation as reference.

Must preserve:

- Packet structure
- Retry logic
- ACK handling
- Peer registration

---

# Architecture

The application is composed of five modules:

```text
DisplayDriver
UIManager
AppState
NFCManager
ESPNowManager
```

Dependencies:

```text
AppState
    ↓
UIManager

NFCManager ─┐
            ├──> AppState
ESPNowManager┘

DisplayDriver
    ↓
UIManager
```

## Rules

- UIManager cannot contain NFC logic.
- UIManager cannot contain ESP-NOW logic.
- NFCManager cannot manipulate LVGL objects.
- ESPNowManager cannot manipulate LVGL objects.
- GeneratedUI must never be edited manually.
- Business logic belongs in AppState and application controllers.

---

# UI Flow

Source of truth:

```text
/docs/ux-flow.md
```

When implementing screens, always follow `/docs/ux-flow.md` exactly.

---

## Home Screen

Wait for:

- NFC tag
- OR 5 button clicks (debug mode)

Invalid NFC:

```text
Error!

{message}
```

Navigate to WaitOrClick screen.

---

## Tutorial

Uses the Questions screen.

Two-step tutorial:

### Step 1

```text
Type: Tutorial
Progress: 0
Numeration: 1/2
Instruction:
rotate the dial until OK
```

Available options:

```text
X -> OK
```

Once user rotates:

```text
click once to advance
```

If user rotates back:

```text
rotate the dial until OK
```

### Step 2

```text
Type: Tutorial
Progress: 1
Numeration: 2/2
Instruction:
click twice to go back
```

Available options:

```text
X X X X X
```

After double click:

```text
Progress: 0
Numeration: 1/2
Instruction:
click once again to end tutorial
```

Available options:

```text
OK
```

---

## Questions

After tutorial completion:

```text
Type: Question
```

There are 4 survey questions.

Questions use the same screen and controls as the tutorial.

### Progress

```text
Question 1 -> 1/5
Question 2 -> 2/5
Question 3 -> 3/5
Question 4 -> 4/5
Confirmation -> 5/5
```

Question text is displayed in:

```text
lblQuestionsInstruction
```

Maximum length:

```text
37 characters
```

Example:

```text
How would you rate J&J on innovation?
```

---

## Confirm Screen

Reached after question 4.

### Single click

Submit survey.

Navigate to WaitOrClick while waiting for response.

### Double click

Return to question 4.

---

## WaitOrClick Screen

### Success

```text
Thanks!

Find yourself at the bigger screen.
```

Timer expiration OR click:

```text
Home
```

### Error

```text
Error!

{message}
```

Message length:

```text
Maximum 37 characters
```

Timer expiration OR click:

```text
Confirm Screen
```

---

# Long Press Actions

Available globally from any screen.

---

## 2 Seconds

Behavior:

- Double beep
- Immediately toggle display mirror mode

No button release required.

Equivalent action:

```cpp
g_displayDriver.setMirrorEnabled(...);
```

---

## 5 Seconds

Behavior:

- Triple beep
- Immediately restart device

No button release required.

Equivalent action:

```cpp
ESP.restart();
```

---

# Current UI Objects

The latest SquareLine export is considered authoritative.

Claude should inspect the generated UI and create wrappers inside UIManager.

Do NOT access generated LVGL objects directly throughout the application.

Preferred pattern:

```cpp
g_uiManager.showHome();

g_uiManager.showQuestion();

g_uiManager.showConfirm();

g_uiManager.showWaitOrClick();

g_uiManager.updateQuestion(
    title,
    instruction,
    option,
    progress
);
```

Only UIManager should know generated object names.

---

# AppState Responsibilities

AppState is the single source of truth.

Suggested stored state:

```cpp
currentScreen

currentQuestion

currentOption

tutorialStep

surveyAnswers[4]

nfcUID

nfcPayload

submissionPending

submissionSuccess
```

UI should always render from AppState.

Avoid duplicated state.

---

# SurveyPacket

DO NOT MODIFY.

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
```

---

# Expectations for Claude

When implementing:

1. Prefer small commits.
2. Build after every subsystem migration.
3. Keep modules independent.
4. Preserve existing working code whenever possible.
5. Do not edit files inside `/lib/GeneratedUI`.
6. Follow `/docs/ux-flow.md` over assumptions.
7. Focus first on the complete UI state machine.
8. NFC integration comes only after the UI flow is fully operational.
9. ESP-NOW integration comes after NFC integration.
10. If a generated UI object is needed, wrap it inside UIManager rather than exposing it globally.

---

# Immediate Task

Current objective:

1. Inspect the latest SquareLine export in `/lib/GeneratedUI`.
2. Create UIManager wrappers for all required screens and controls.
3. Implement AppState.
4. Implement the full UX flow from `/docs/ux-flow.md`.
5. Implement encoder navigation and button interactions.
6. Verify the complete survey flow without NFC.
7. Only after the UI flow is working, begin NFCManager implementation based on `/reference/pn532_demo.ino`.