#pragma once

#include <stdint.h>
#include <M5Dial.h>
#include <lvgl.h>

class DisplayDriver {
public:
    DisplayDriver();
    ~DisplayDriver();

    bool init();
    void update(uint32_t elapsed_ms);
    void setMirrorEnabled(bool enabled);
    bool getMirrorEnabled() const;

    uint16_t getWidth() const;
    uint16_t getHeight() const;

private:
    bool mirrorEnabled;
    uint16_t width;
    uint16_t height;

    static void flushCallback(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
};

extern DisplayDriver g_displayDriver;
