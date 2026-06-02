#include <M5Unified.h>

void setup()
{
    auto cfg = M5.config();
    M5.begin(cfg);

    Serial.begin(115200);

    M5.Display.fillScreen(BLACK);

    M5.Display.setCursor(20,20);
    M5.Display.print("Hello M5Dial");
}

void loop()
{
    M5.update();
}