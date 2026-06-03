#include "ESPNowManager.h"
#include <esp_wifi.h>
#include <esp_now.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <freertos/portmacro.h>

const uint8_t ESPNowManager::RECEIVER_MAC[6] = {
    0xB0, 0xA7, 0x32, 0x14, 0xFD, 0xFC
};

ESPNowManager* ESPNowManager::instance = nullptr;
portMUX_TYPE ackMux = portMUX_INITIALIZER_UNLOCKED;

ESPNowManager g_espNowManager;

ESPNowManager::ESPNowManager()
    : initialized(false),
      waitingForAck(false),
      ackReceived(false),
      ackAccepted(false),
      waitingAckCounter(0) {
    instance = this;
}

ESPNowManager::~ESPNowManager() {
}

bool ESPNowManager::ensureWifiReady() {
    wifi_mode_t mode;

    if (esp_wifi_get_mode(&mode) == ESP_OK) {
        Serial.println("WiFi already initialized");
        return true;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if (esp_wifi_init(&cfg) != ESP_OK) {
        Serial.println("WiFi init failed");
        return false;
    }

    if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK) {
        Serial.println("WiFi storage config failed");
        return false;
    }

    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
        Serial.println("WiFi STA mode failed");
        return false;
    }

    if (esp_wifi_start() != ESP_OK) {
        Serial.println("WiFi start failed");
        return false;
    }

    esp_wifi_disconnect();

    Serial.println("WiFi ready");
    return true;
}

bool ESPNowManager::ensureReceiverPeer() {
    if (esp_now_is_peer_exist(RECEIVER_MAC)) {
        Serial.println("Receiver peer already exists");
        return true;
    }

    esp_now_peer_info_t pi = {};
    memcpy(pi.peer_addr, RECEIVER_MAC, 6);
    pi.channel = 0;
    pi.encrypt = false;

    if (esp_now_add_peer(&pi) == ESP_OK) {
        Serial.println("Receiver peer registered");
        return true;
    }

    Serial.println("Failed to register receiver peer");
    return false;
}

void ESPNowManager::onEspNowRecv(
    const uint8_t *mac,
    const uint8_t *data,
    int len) {
    (void)mac;

    if (instance == nullptr) {
        return;
    }

    if (len != sizeof(DialAckPacket)) {
        Serial.printf("ACK packet size mismatch: got %d, expected %zu\n", len, sizeof(DialAckPacket));
        return;
    }

    const DialAckPacket *ack = (const DialAckPacket *)data;

    Serial.printf(
        "ACK received: type=%d dev=%d counter=%lu accepted=%d\n",
        ack->packetType,
        ack->device_id,
        ack->counter,
        ack->accepted
    );

    if (ack->packetType != PACKET_ACK) {
        Serial.println("ACK packet type mismatch");
        return;
    }

    portENTER_CRITICAL_ISR(&ackMux);

    if (instance->waitingForAck &&
        ack->device_id == DEVICE_ID &&
        ack->counter == instance->waitingAckCounter) {
        instance->ackReceived = true;
        instance->ackAccepted = ack->accepted;
        instance->waitingForAck = false;
        Serial.println("ACK matched!");
    }

    portEXIT_CRITICAL_ISR(&ackMux);
}

bool ESPNowManager::initEspNow() {
    if (esp_netif_init() != ESP_OK) {
        if (esp_netif_init() != ESP_ERR_INVALID_STATE) {
            Serial.println("esp_netif_init failed");
            return false;
        }
    }

    if (esp_event_loop_create_default() != ESP_OK) {
        if (esp_event_loop_create_default() != ESP_ERR_INVALID_STATE) {
            Serial.println("esp_event_loop_create_default failed");
            return false;
        }
    }

    if (!ensureWifiReady()) {
        Serial.println("WiFi initialization failed");
        return false;
    }

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return false;
    }

    esp_now_register_recv_cb(onEspNowRecv);

    if (!ensureReceiverPeer()) {
        Serial.println("Failed to register receiver peer");
        return false;
    }

    Serial.println("ESP-NOW initialized");
    return true;
}

bool ESPNowManager::begin() {
    if (initialized) {
        Serial.println("ESPNowManager already initialized");
        return true;
    }

    initialized = initEspNow();
    return initialized;
}

bool ESPNowManager::sendSurvey(const SurveyPacket& pkt) {
    if (!initialized) {
        Serial.println("ESPNowManager not initialized");
        return false;
    }

    if (!ensureReceiverPeer()) {
        Serial.println("Receiver peer verification failed");
        return false;
    }

    for (uint8_t attempt = 0; attempt < MAX_RETRIES; attempt++) {
        Serial.printf("Send attempt %d/%d\n", attempt + 1, MAX_RETRIES);

        portENTER_CRITICAL(&ackMux);
        ackReceived = false;
        ackAccepted = false;
        waitingForAck = true;
        waitingAckCounter = pkt.counter;
        portEXIT_CRITICAL(&ackMux);

        esp_err_t r = esp_now_send(
            RECEIVER_MAC,
            (const uint8_t *)&pkt,
            sizeof(pkt)
        );

        if (r != ESP_OK) {
            Serial.printf("esp_now_send failed with code %d\n", r);
            continue;
        }

        Serial.println("Packet sent, waiting for ACK...");

        uint32_t t0 = millis();
        while (millis() - t0 < ACK_TIMEOUT_MS) {
            bool gotAck = false;

            portENTER_CRITICAL(&ackMux);
            gotAck = ackReceived;
            portEXIT_CRITICAL(&ackMux);

            if (gotAck) {
                bool accepted;

                portENTER_CRITICAL(&ackMux);
                accepted = ackAccepted;
                portEXIT_CRITICAL(&ackMux);

                if (accepted) {
                    Serial.println("ACK accepted!");
                    return true;
                } else {
                    Serial.println("ACK rejected");
                    return false;
                }
            }

            delay(5);
        }

        portENTER_CRITICAL(&ackMux);
        waitingForAck = false;
        portEXIT_CRITICAL(&ackMux);

        Serial.println("ACK timeout, retrying...");
    }

    Serial.println("Send failed after all retries");
    return false;
}

bool ESPNowManager::isAckReceived() const {
    bool result;
    portENTER_CRITICAL(&ackMux);
    result = ackReceived;
    portEXIT_CRITICAL(&ackMux);
    return result;
}

bool ESPNowManager::isAckAccepted() const {
    bool result;
    portENTER_CRITICAL(&ackMux);
    result = ackAccepted;
    portEXIT_CRITICAL(&ackMux);
    return result;
}

bool ESPNowManager::isReady() const {
    return initialized;
}
