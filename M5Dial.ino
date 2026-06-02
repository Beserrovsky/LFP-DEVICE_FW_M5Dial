#include "M5Dial.h"
#include <Arduino.h>
#include <lvgl.h>
#include <Wire.h>
#include <SPI.h>
#include "ui.h"
#include <Adafruit_PN532.h>

// init the tft espi
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;  // Descriptor of a display driver

#define EXAMPLE_LCD_H_RES 240
#define EXAMPLE_LCD_V_RES 240
#define LV_VER_RES_MAX 240
#define LV_HOR_RES_MAX 240
M5GFX *tft;

long oldPosition = -999;

// PN532 NFC Reader on I2C (G13=SDA, G15=SCL)
Adafruit_PN532 nfc(Wire);
char nfcJsonBuffer[145];  // Max 144 bytes + null terminator

void tft_lv_initialization() {
  lv_init();
  static lv_color_t buf1[(LV_HOR_RES_MAX * LV_VER_RES_MAX) / 10];  // Declare a buffer for 1/10 screen siz
  static lv_color_t buf2[(LV_HOR_RES_MAX * LV_VER_RES_MAX) / 10];  // second buffer is optionnal
  // Initialize `disp_buf` display buffer with the buffer(s).
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, (LV_HOR_RES_MAX * LV_VER_RES_MAX) / 10);
  tft=&M5Dial.Lcd;
}

// Display flushing
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft->startWrite();
  tft->setAddrWindow(area->x1, area->y1, w, h);
  tft->pushColors((uint16_t *)&color_p->full, w * h, true);
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

// Initialize NFC Reader
void init_nfc() {
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("PN532 not found");
    return;
  }
  Serial.print("PN532 firmware v"); 
  Serial.print((versiondata >> 24) & 0xFF, DEC);
  Serial.print('.'); 
  Serial.println((versiondata >> 16) & 0xFF, DEC);
  nfc.SAMConfig();
  nfc.setPassiveActivationRetries(0xFF);
}

// Read NTAG213 and extract NDEF JSON payload
bool readNFCTag(char* jsonBuffer, size_t bufferSize) {
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  
  if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 0)) {
    return false;  // No tag detected
  }
  
  Serial.println("Tag detected!");
  
  // Read NTAG213 (180 bytes total, 4-byte header + 176 user bytes)
  uint8_t tagData[180];
  bool success = true;
  
  // Read all blocks (0-44, 4 bytes per block)
  for (uint8_t i = 0; i < 45; i++) {
    uint8_t block[4];
    if (!nfc.mifareclassic_ReadDataBlock(i, block)) {
      Serial.print("Failed to read block ");
      Serial.println(i);
      success = false;
      break;
    }
    memcpy(&tagData[i * 4], block, 4);
  }
  
  if (!success) return false;
  
  // Parse NDEF Message
  // NDEF format: [Type Length (TL)] [Type] [Payload Length (PL)] [Payload]
  // For JSON: TL=0x01, Type='T' (text), or Type='U' (URI)
  
  uint8_t ndefStart = 4;  // NTAG213 NDEF starts at byte 4
  uint8_t recordType = tagData[ndefStart];
  
  if (recordType != 0xE1) {
    // Look for NDEF Message Begin (MB) flag
    uint8_t tnf = recordType & 0x07;
    uint8_t typeLength = tagData[ndefStart + 1];
    uint8_t payloadLength = tagData[ndefStart + 2];
    
    if (payloadLength == 0) payloadLength = tagData[ndefStart + 3];
    
    uint8_t payloadStart = ndefStart + 2 + typeLength;
    if (tagData[ndefStart + 2] == 0xFF) {
      payloadStart = ndefStart + 4 + typeLength;
    }
    
    // Extract JSON payload
    size_t copySize = (payloadLength < bufferSize - 1) ? payloadLength : bufferSize - 1;
    memcpy(jsonBuffer, &tagData[payloadStart], copySize);
    jsonBuffer[copySize] = '\0';
  } else {
    // Try alternative NDEF parsing for ISO/IEC 14443-4 Type 2
    // Capability Container at offset 12
    if (tagData[12] == 0xE1 && tagData[13] == 0x10) {
      uint8_t ndefMsgStart = 16;
      uint8_t ndefHeader = tagData[ndefMsgStart];
      uint8_t payloadLength = tagData[ndefMsgStart + 2];
      
      if (payloadLength > 0 && payloadLength <= 144) {
        size_t copySize = (payloadLength < bufferSize - 1) ? payloadLength : bufferSize - 1;
        memcpy(jsonBuffer, &tagData[ndefMsgStart + 3], copySize);
        jsonBuffer[copySize] = '\0';
      }
    }
  }
  
  Serial.print("JSON: ");
  Serial.println(jsonBuffer);
  return true;
}

// Poll for NFC tag periodically
void pollNFC() {
  static unsigned long lastPoll = 0;
  if (millis() - lastPoll > 500) {  // Poll every 500ms
    lastPoll = millis();
    readNFCTag(nfcJsonBuffer, sizeof(nfcJsonBuffer));
  }
}

void setup()
{
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setBrightness(80);
  Serial.begin(115200);
  
  tft_lv_initialization();
  init_disp_driver();
  ui_init();
  
  // Initialize NFC on I2C (G13=SDA, G15=SCL)
  Wire.begin(13, 15);  // SDA=G13, SCL=G15
  init_nfc();
}



void loop()
{
    uint32_t wait_ms = lv_timer_handler();
    M5.delay(wait_ms);  

    M5Dial.update();
    long newPosition = M5Dial.Encoder.read();
    if (newPosition != oldPosition) {
        M5Dial.Speaker.tone(8000, 20);
        oldPosition = newPosition;
    }
    
    // Poll for NFC tag
    pollNFC();
}


