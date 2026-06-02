#include <M5Dial.h>
#include "DisplayDriver.h"
#include "UIManager.h"

void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);
    M5Dial.Display.setBrightness(128);

    Serial.begin(115200);
    delay(500);

    g_displayDriver.init();
    g_uiManager.init();
}

void loop() {
    static uint32_t lastTick = millis();
    uint32_t now = millis();
    uint32_t elapsed = now - lastTick;

    if (elapsed > 0) {
        g_displayDriver.update(elapsed);
        lastTick = now;
    }

    M5Dial.update();

    delay(5);
}