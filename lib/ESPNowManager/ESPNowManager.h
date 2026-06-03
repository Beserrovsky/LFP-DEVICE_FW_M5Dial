#pragma once

#include <stdint.h>
#include <stddef.h>

enum PacketType : uint8_t {
    PACKET_QUESTION_SET = 1,
    PACKET_SUBMISSION   = 2,
    PACKET_ACK          = 3
};

typedef struct __attribute__((packed)) {
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
} SurveyPacket;

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
