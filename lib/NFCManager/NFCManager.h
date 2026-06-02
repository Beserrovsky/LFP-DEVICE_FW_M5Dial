#pragma once

#include <stdint.h>
#include <stddef.h>

class NFCManager {
public:
    NFCManager();
    ~NFCManager();

    bool init();
    void update();

    bool tagDetected() const;
    const uint8_t* getTagUID() const;
    uint8_t getTagUIDLength() const;

    void clearDetection();

private:
    uint8_t tagUID[7];
    uint8_t tagUIDLength;
    bool detectionActive;
};

extern NFCManager g_nfcManager;
