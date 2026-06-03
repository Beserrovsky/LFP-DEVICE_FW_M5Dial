#pragma once

#include <stdint.h>
#include <stddef.h>

enum PacketType : uint8_t {
    PACKET_QUESTION_SET = 1,
    PACKET_SUBMISSION   = 2,
    PACKET_ACK          = 3
};

typedef struct __attribute__((packed)) {
    // ── Transport header ────────────────────────────────────────────────────
    uint8_t  packetType;       // Always 2 (PACKET_SUBMISSION)
    uint8_t  device_id;        // Sending device ID (currently 3)
    uint32_t counter;          // Wrapping sequence number — used for deduplication
    uint32_t timestamp_ms;     // Device uptime in ms at send time

    // ── Respondent identity (from NFC tag) ──────────────────────────────────
    char name[16];             // First name  e.g. "Felipe"
    char identifier[16];       // Unique ID: phone number or employee ID e.g. "11987654321"

    // ── Survey answers ───────────────────────────────────────────────────────
    uint8_t  innovation;       // Q1: 1–5  (1 = lowest, 5 = highest)
    uint8_t  satisfaction;     // Q2: 1 = Yes, 0 = No
    uint8_t  nps;              // Q3: 0–10 (NPS score, direct value)
    char     bestEmployee[12]; // Q4: "Junior" | "Vitoria" | "Sergio"
} SurveyPacket;
// Total: 1+1+4+4 + 16+16 + 1+1+1+12 = 57 bytes (packed)

typedef struct __attribute__((packed)) {
    uint8_t packetType;
    char session_id[16];
    uint8_t device_id;
    uint32_t counter;
    bool accepted;
} DialAckPacket;

class ESPNowManager {
public:
    ESPNowManager();
    ~ESPNowManager();

    bool begin();
    bool sendSurvey(const SurveyPacket& pkt);

    bool isAckReceived() const;
    bool isAckAccepted() const;
    bool isReady() const;

private:
    static const uint8_t DEVICE_ID = 3;
    static const uint8_t RECEIVER_MAC[6];
    static const uint8_t MAX_RETRIES = 4;
    static const uint32_t ACK_TIMEOUT_MS = 500;

    bool initialized;
    volatile bool waitingForAck;
    volatile bool ackReceived;
    volatile bool ackAccepted;
    volatile uint32_t waitingAckCounter;

    bool ensureWifiReady();
    bool ensureReceiverPeer();
    bool initEspNow();

    static void onEspNowRecv(
        const uint8_t *mac,
        const uint8_t *data,
        int len
    );

    static ESPNowManager* instance;
};

extern ESPNowManager g_espNowManager;
