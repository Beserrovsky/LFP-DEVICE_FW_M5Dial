#include <M5Dial.h>
#include <lvgl.h>
#include "DisplayDriver.h"
#include "UIManager.h"
#include "AppState.h"

void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);
    M5Dial.Display.setBrightness(128);

    Serial.begin(115200);
    delay(500);

    g_displayDriver.init();
    g_uiManager.init();
    g_appState.init();

    
}

void loop() {
    static uint32_t lastTick = millis();
    static long encoderPosition = 0;

    uint32_t now = millis();
    uint32_t elapsed = now - lastTick;

    if (elapsed > 0) {
        g_displayDriver.update(elapsed);
        lastTick = now;
    }

    M5Dial.update();

    if (M5Dial.BtnA.pressedFor(1000)) {
        g_displayDriver.setMirrorEnabled(!g_displayDriver.getMirrorEnabled());
        lv_obj_invalidate(lv_scr_act());
        M5Dial.Speaker.tone(4000, 100);

        while (M5Dial.BtnA.isPressed()) {
            M5Dial.update();
            delay(10);
        }
    }

    long newPosition = M5Dial.Encoder.read();
    if (newPosition != encoderPosition) {
        long delta = newPosition - encoderPosition;

        if (delta > 0) {
            g_appState.incrementOption(10);
        } else {
            g_appState.decrementOption();
        }

        g_uiManager.updateOptionsDisplay(g_appState.getCurrentOption());
        M5Dial.Speaker.tone(8000, 20);
        encoderPosition = newPosition;
    }

    delay(5);
}