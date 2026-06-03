# Receiver Protocol — SurveyPacket Field Reference

This document describes the ESP-NOW packet sent by the M5Dial survey device and how the receiver should interpret each field.

---

## Packet Structure

```c
typedef struct __attribute__((packed)) {
    uint8_t packetType;       // Always 2 (PACKET_SUBMISSION) for survey data
    uint8_t device_id;        // Hardcoded: 3
    uint32_t counter;         // millis() & 0xFFFF — wraps, used for deduplication
    uint32_t timestamp_ms;    // millis() at submission time

    int innovation;           // Q1 answer (see below)
    int satisfactionIndex;    // Q2 answer (see below)
    int nps;                  // Q3 answer (see below)

    char fishType[16];        // Q4 answer: selected employee name, null-terminated
    char fishColour[16];      // Reserved — always empty string ""
    char name[12];            // NFC-derived respondent name, null-terminated
                              // Empty string "" when debug 5-click path is used
} SurveyPacket;
```

**Total size:** 4 + 4 + 4 + 4 + 4 + 16 + 16 + 12 = **64 bytes** (packed, no padding)

---

## Field Reference

### `packetType` — `uint8_t`

Always `2` (`PACKET_SUBMISSION`). Ignore packets with any other value.

| Value | Meaning |
|---|---|
| `1` | PACKET_QUESTION_SET (not sent by M5Dial) |
| `2` | PACKET_SUBMISSION (survey response) |
| `3` | PACKET_ACK (sent by receiver back to M5Dial) |

---

### `device_id` — `uint8_t`

Always `3`. Can be used to identify which device sent the packet in a multi-device deployment.

---

### `counter` — `uint32_t`

`millis() & 0xFFFF` at send time. Use to detect duplicate transmissions (the device retries up to 4 times if no ACK is received). Discard packets with the same `device_id` + `counter` seen within a short window (~5 seconds).

---

### `timestamp_ms` — `uint32_t`

Device uptime in milliseconds at the moment of submission. Not wall-clock time.

---

### `innovation` — `int`  
**Survey Q1: "How would you rate J&J on innovation?"**

| Raw value | Displayed option |
|---|---|
| `1` | 1 (lowest) |
| `2` | 2 |
| `3` | 3 |
| `4` | 4 |
| `5` | 5 (highest) |

> Note: stored as `selectedOptionIndex + 1` (1-based).

---

### `satisfactionIndex` — `int`  
**Survey Q2: "Happy with J&J interactions?"**

| Raw value | Displayed option |
|---|---|
| `0` | Yes |
| `1` | No |

> Note: stored as `selectedOptionIndex` (0-based).

---

### `nps` — `int`  
**Survey Q3: "Recommend J&J to a colleague?"**

| Raw value | Displayed option |
|---|---|
| `0` | 0 |
| `1` | 1 |
| `2` | 2 |
| `3` | 3 |
| `4` | 4 |
| `5` | 5 |
| `6` | 6 |
| `7` | 7 |
| `8` | 8 |
| `9` | 9 |
| `10` | 10 |

> Note: stored as `selectedOptionIndex` (0-based), which equals the displayed number directly.

---

### `fishType[16]` — `char[]`  
**Survey Q4: "Best J&J Employee"**

Null-terminated string. One of the following:

| Value | Meaning |
|---|---|
| `"Junior"` | Junior selected |
| `"Vitoria"` | Vitoria selected |
| `"Sergio"` | Sergio selected |

---

### `fishColour[16]` — `char[]`

Always empty string `""`. Reserved for future use.

---

### `name[12]` — `char[]`

Null-terminated string containing the respondent's name extracted from the NFC tag NDEF payload.

- Populated when the session started via NFC scan (normal flow).
- Empty string `""` when the session started via 5-click debug path.
- Maximum length: 11 characters + null terminator.

**Expected NFC tag format:** NTAG213/215/216 with an NDEF text record containing a JSON string:
```json
{"name":"Felipe"}
```

---

## ACK Response

The receiver must send back a `DialAckPacket` to confirm reception:

```c
typedef struct __attribute__((packed)) {
    uint8_t packetType;      // 3 (PACKET_ACK)
    char session_id[16];     // Reserved — can be empty
    uint8_t device_id;       // Echo back the device_id from the survey packet
    uint32_t counter;        // Echo back the counter from the survey packet
    bool accepted;           // true = accepted, false = rejected
} DialAckPacket;
```

- If `accepted = true`: M5Dial shows success message and returns to Home after timeout.
- If `accepted = false`: M5Dial shows error message and allows resubmission.
- If no ACK is received within 500ms, the device retries up to 4 times, then reports error.
