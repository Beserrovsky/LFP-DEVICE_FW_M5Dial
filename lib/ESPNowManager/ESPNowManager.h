#pragma once

#include <stdint.h>
#include <stddef.h>

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

class ESPNowManager {
public:
    ESPNowManager();
    ~ESPNowManager();

    bool init();
    void update();

    bool isReady() const;
    bool sendPacket(const SurveyPacket& packet);
    bool sendPacketWithAck(const SurveyPacket& packet);

    bool ackReceived() const;
    bool ackAccepted() const;

private:
    bool ready;
    bool ackRxFlag;
    bool ackAcceptedFlag;
};

extern ESPNowManager g_espNowManager;
