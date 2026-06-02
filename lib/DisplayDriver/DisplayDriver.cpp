#include "DisplayDriver.h"

#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 240

static lv_disp_draw_buf_t g_drawBuf;
static lv_disp_drv_t g_dispDrv;
static lv_color_t g_mirrorBuf[DISPLAY_WIDTH];

DisplayDriver g_displayDriver;

DisplayDriver::DisplayDriver()
    : mirrorEnabled(false), width(DISPLAY_WIDTH), height(DISPLAY_HEIGHT) {
}

DisplayDriver::~DisplayDriver() {
}

void DisplayDriver::flushCallback(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    if (!g_displayDriver.mirrorEnabled) {
        M5Dial.Display.startWrite();
        M5Dial.Display.setAddrWindow(area->x1, area->y1, w, h);
        M5Dial.Display.pushColors((uint16_t *)&color_p->full, w * h, true);
        M5Dial.Display.endWrite();

        lv_disp_flush_ready(disp);
        return;
    }

    uint32_t mirror_x = DISPLAY_WIDTH - area->x2 - 1;

    M5Dial.Display.startWrite();

    for (uint32_t y = 0; y < h; y++) {
        lv_color_t* src = color_p + (y * w);

        for (uint32_t x = 0; x < w; x++) {
            g_mirrorBuf[x] = src[w - 1 - x];
        }

        M5Dial.Display.setAddrWindow(mirror_x, area->y1 + y, w, 1);
        M5Dial.Display.pushColors((uint16_t *)&g_mirrorBuf[0].full, w, true);
    }

    M5Dial.Display.endWrite();

    lv_disp_flush_ready(disp);
}

bool DisplayDriver::init() {
    lv_init();

    static lv_color_t buf1[(DISPLAY_WIDTH * DISPLAY_HEIGHT) / 10];
    static lv_color_t buf2[(DISPLAY_WIDTH * DISPLAY_HEIGHT) / 10];

    lv_disp_draw_buf_init(&g_drawBuf, buf1, buf2, (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 10);

    lv_disp_drv_init(&g_dispDrv);
    g_dispDrv.flush_cb = flushCallback;
    g_dispDrv.draw_buf = &g_drawBuf;
    g_dispDrv.hor_res = DISPLAY_WIDTH;
    g_dispDrv.ver_res = DISPLAY_HEIGHT;

    lv_disp_drv_register(&g_dispDrv);
    lv_disp_set_bg_color(NULL, lv_color_hex3(0x000));

    return true;
}

void DisplayDriver::update(uint32_t elapsed_ms) {
    if (elapsed_ms > 0) {
        lv_tick_inc(elapsed_ms);
    }
    lv_timer_handler();
}

void DisplayDriver::setMirrorEnabled(bool enabled) {
    mirrorEnabled = enabled;
}

bool DisplayDriver::getMirrorEnabled() const {
    return mirrorEnabled;
}

uint16_t DisplayDriver::getWidth() const {
    return width;
}

uint16_t DisplayDriver::getHeight() const {
    return height;
}

