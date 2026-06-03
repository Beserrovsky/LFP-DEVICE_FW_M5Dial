#include <M5Dial.h>
#include <lvgl.h>
#include "DisplayDriver.h"
#include "UIManager.h"
#include "AppState.h"
#include "ESPNowManager.h"

// Global modules
extern DisplayDriver g_displayDriver;
extern UIManager g_uiManager;
extern AppState g_appState;
extern ESPNowManager g_espNowManager;

void sendCurrentSurvey() {
    SurveyPacket packet = {};

    packet.packetType = PACKET_SUBMISSION;
    packet.device_id = 3;
    packet.counter = millis() & 0xFFFF;
    packet.timestamp_ms = millis();

    /*
    packet.innovation = g_appState.getInnovation();
    packet.satisfactionIndex = g_appState.getSatisfaction();
    packet.nps = g_appState.getNPS();

    strncpy(packet.fishType,
            g_appState.getFishType().c_str(),
            sizeof(packet.fishType) - 1);

    strncpy(packet.fishColour,
            g_appState.getFishColor().c_str(),
            sizeof(packet.fishColour) - 1);

    */
   
    strncpy(packet.name, "Test", sizeof(packet.name) - 1);

    bool ok = g_espNowManager.sendSurvey(packet);

    Serial.printf("\n[ESP-NOW] Survey send: %s\n", ok ? "SUCCESS" : "FAILED");

    // placeholder UI feedback hook
    //g_uiManager.showSendResult(ok);
}

void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);
    M5Dial.Display.setBrightness(128);

    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n=== M5Dial Survey Device ===\n");

    g_displayDriver.init();
    g_uiManager.init();
    g_appState.init();
    g_espNowManager.begin();

    delay(500);
}

void loop() {
    static uint32_t lastTick = millis();
    static long encoderPosition = 0;
    static bool wasPressed = false;

    uint32_t now = millis();
    uint32_t elapsed = now - lastTick;

    // LVGL / display update
    if (elapsed > 0) {
        g_displayDriver.update(elapsed);
        lastTick = now;
    }

    M5Dial.update();

    // ----------------------------
    // LONG PRESS → MIRROR TOGGLE
    // ----------------------------
    if (M5Dial.BtnA.pressedFor(1000)) {
        g_displayDriver.setMirrorEnabled(!g_displayDriver.getMirrorEnabled());
        lv_obj_invalidate(lv_scr_act());
        M5Dial.Speaker.tone(4000, 100);

        while (M5Dial.BtnA.isPressed()) {
            M5Dial.update();
            delay(10);
        }

        wasPressed = false;
        return;
    }

    // ----------------------------
    // SHORT CLICK → SEND SURVEY
    // ----------------------------
    bool isPressed = M5Dial.BtnA.isPressed();

    if (isPressed && !wasPressed) {
        Serial.println("\n[UI] Confirm pressed → sending survey");
        sendCurrentSurvey();
    }

    wasPressed = isPressed;

    // ----------------------------
    // ENCODER → OPTION CHANGE
    // ----------------------------
    long newPosition = M5Dial.Encoder.read();

    if (newPosition != encoderPosition) {
        long delta = newPosition - encoderPosition;

        if (delta > 0) {
            g_appState.incrementOption(10);
        } else {
            g_appState.decrementOption();
        }

        M5Dial.Speaker.tone(8000, 20);

        encoderPosition = newPosition;
    }

    delay(5);
}