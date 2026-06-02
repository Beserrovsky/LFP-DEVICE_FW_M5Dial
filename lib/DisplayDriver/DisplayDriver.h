#pragma once

#include <stdint.h>
#include <M5Unified.h>

class DisplayDriver {
public:
    DisplayDriver();
    ~DisplayDriver();

    bool init();
    void update();
    void flush();
    void setMirrorEnabled(bool enabled);
    bool getMirrorEnabled() const;

    uint16_t getWidth() const;
    uint16_t getHeight() const;

private:
    bool mirrorEnabled;
    uint16_t width;
    uint16_t height;
};

extern DisplayDriver g_displayDriver;
