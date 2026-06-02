#include "M5Dial.h"
#include <Arduino.h>
#include <lvgl.h>
#include <Wire.h>
#include <SPI.h>
#include "ui.h"

// init the tft espi
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;  // Descriptor of a display driver

#define EXAMPLE_LCD_H_RES 240
#define EXAMPLE_LCD_V_RES 240
#define LV_VER_RES_MAX 240
#define LV_HOR_RES_MAX 240
M5GFX *tft;

void tft_lv_initialization() {
  lv_init();
  static lv_color_t buf1[(LV_HOR_RES_MAX * LV_VER_RES_MAX) / 10];  // Declare a buffer for 1/10 screen siz
  static lv_color_t buf2[(LV_HOR_RES_MAX * LV_VER_RES_MAX) / 10];  // second buffer is optionnal
  // Initialize `disp_buf` display buffer with the buffer(s).
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, (LV_HOR_RES_MAX * LV_VER_RES_MAX) / 10);
  tft=&M5Dial.Lcd;
}

bool mirrorEnabled = true;

// Display flushing
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    if (!mirrorEnabled)
    {
        tft->startWrite();
        tft->setAddrWindow(area->x1, area->y1, w, h);
        tft->pushColors((uint16_t *)&color_p->full, w * h, true);
        tft->endWrite();

        lv_disp_flush_ready(disp);
        return;
    }

    static lv_color_t mirror_buf[LV_HOR_RES_MAX];

    uint32_t mirror_x = LV_HOR_RES_MAX - area->x2 - 1;

    tft->startWrite();

    for(uint32_t y = 0; y < h; y++)
    {
        lv_color_t* src = color_p + (y * w);

        for(uint32_t x = 0; x < w; x++)
        {
            mirror_buf[x] = src[w - 1 - x];
        }

        tft->setAddrWindow(
            mirror_x,
            area->y1 + y,
            w,
            1
        );

        tft->pushColors(
            (uint16_t*)&mirror_buf[0].full,
            w,
            true
        );
    }

    tft->endWrite();

    lv_disp_flush_ready(disp);
}

void init_disp_driver() {
  lv_disp_drv_init(&disp_drv);  // Basic initialization
  disp_drv.flush_cb = my_disp_flush;  // Set your driver function
  disp_drv.draw_buf = &draw_buf;      // Assign the buffer to the display
  disp_drv.hor_res = LV_HOR_RES_MAX;  // Set the horizontal resolution of the display
  disp_drv.ver_res = LV_VER_RES_MAX;  // Set the vertical resolution of the display
  lv_disp_drv_register(&disp_drv);                   // Finally register the driver
  lv_disp_set_bg_color(NULL, lv_color_hex3(0x000));  // Set default background color to black
}

int currentOption = 3;
long oldPosition = 0;


void setup()
{
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setBrightness(100);
  Serial.begin(115200);
  tft_lv_initialization();
  init_disp_driver();
  ui_init();

  updateOptions();
  oldPosition = M5Dial.Encoder.read();
}


void updateOptions() // Update QuestionScreen1 Previous, Current and Next NPS values!
{
    char buf[4];

    // Atual
    sprintf(buf, "%d", currentOption);
    lv_label_set_text(ui_lblCurrentOption1, buf);

    // Anterior (wrap)
    if (currentOption == 0)
    {
      strcpy(buf, " ");
    }
    else
    {
      sprintf(buf, "%d", currentOption - 1);
    }
    lv_label_set_text(ui_lblPrevOption1, buf);

    // Próximo (wrap)
    if (currentOption == 10)
    {
      strcpy(buf, " ");
    }
    else
    {
      sprintf(buf, "%d", currentOption + 1);
    }

    lv_label_set_text(ui_lblNextOption1, buf);
}


void loop()
{
    lv_tick_inc(5);
    lv_timer_handler();

    M5.delay(5);

    M5Dial.update();

    if (M5Dial.BtnA.pressedFor(1000))
    {
        mirrorEnabled = !mirrorEnabled;

        lv_obj_invalidate(lv_scr_act());

        M5Dial.Speaker.tone(4000, 100);

        while (M5Dial.BtnA.isPressed())
        {
            M5Dial.update();
            delay(10);
        }
    }

    long newPosition = M5Dial.Encoder.read();

    if (newPosition != oldPosition)
    {
        long delta = newPosition - oldPosition;

        if (delta > 0)
        {
            currentOption++;
            if (currentOption > 10)
                currentOption = 10;
        }
        else
        {
            currentOption--;
            if (currentOption < 0)
                currentOption = 0;
        }

        updateOptions();

        M5Dial.Speaker.tone(8000, 20);

        oldPosition = newPosition;
    }
}

