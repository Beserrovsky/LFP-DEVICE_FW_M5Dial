#include "NFCManager.h"

NFCManager g_nfcManager;

NFCManager::NFCManager()
    : tagUIDLength(0), detectionActive(false) {
    for (int i = 0; i < 7; i++) {
        tagUID[i] = 0;
    }
}

NFCManager::~NFCManager() {
}

bool NFCManager::init() {
    return true;
}

void NFCManager::update() {
}

bool NFCManager::tagDetected() const {
    return detectionActive;
}

const uint8_t* NFCManager::getTagUID() const {
    return tagUID;
}

uint8_t NFCManager::getTagUIDLength() const {
    return tagUIDLength;
}

void NFCManager::clearDetection() {
    detectionActive = false;
    tagUIDLength = 0;
}
