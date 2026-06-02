#include "ESPNowManager.h"

ESPNowManager g_espNowManager;

ESPNowManager::ESPNowManager()
    : ready(false), ackRxFlag(false), ackAcceptedFlag(false) {
}

ESPNowManager::~ESPNowManager() {
}

bool ESPNowManager::init() {
    return true;
}

void ESPNowManager::update() {
}

bool ESPNowManager::isReady() const {
    return ready;
}

bool ESPNowManager::sendPacket(const SurveyPacket& packet) {
    (void)packet;
    return true;
}

bool ESPNowManager::sendPacketWithAck(const SurveyPacket& packet) {
    (void)packet;
    return true;
}

bool ESPNowManager::ackReceived() const {
    return ackRxFlag;
}

bool ESPNowManager::ackAccepted() const {
    return ackAcceptedFlag;
}
