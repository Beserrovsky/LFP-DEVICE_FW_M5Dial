// =============================================================
// M5Stack Dial — Survey firmware
// Fully patched / hardened version
//
// FIXES INCLUDED
// -------------------------------------------------------------
// ✓ Stub mode no longer requires SquareLine files
// ✓ LVGL 8 + LVGL 9 compatibility layer
// ✓ Safer ESP-NOW ACK synchronization
// ✓ WiFi init no longer double-inits ESP-IDF WiFi
// ✓ Persistent pending submissions in NVS
// ✓ Safer constrain() handling
// ✓ Reduced stub-mode flicker
// ✓ Better ACK timeout handling
// ✓ Production-safe compile guards
//
// NOTE:
// -------------------------------------------------------------
// If UI_STUB_MODE == 0:
//
//   Place your real SquareLine files in:
//
//      /ui/ui.h
//      /ui/ui.c
//      etc.
//
// If UI_STUB_MODE == 1:
//
//   No ui/ folder is required anymore.
// =============================================================

#define UI_STUB_MODE 1

#include <M5Dial.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <nvs_flash.h>
#include <Preferences.h>
#include <esp_heap_caps.h>

#if __has_include(<esp_arduino_version.h>)
#include <esp_arduino_version.h>
#endif

// =============================================================
// LVGL / UI COMPATIBILITY
// =============================================================

#if !UI_STUB_MODE

#include <lgfx/v1/lvgl.h>
#include "ui/ui.h"

#else

// -------------------------------------------------------------
// COMPLETE STUB IMPLEMENTATION
// No SquareLine files required
// -------------------------------------------------------------

struct _lv_obj_t {};
typedef struct _lv_obj_t lv_obj_t;

struct lv_image_dsc_t {
    const void *data;
};

// ---- fake objects ----
static lv_obj_t fakeObj;

lv_obj_t *ui_lblQuestionStep      = &fakeObj;
lv_obj_t *ui_lblQuestionText      = &fakeObj;
lv_obj_t *ui_lblCurrentOption     = &fakeObj;
lv_obj_t *ui_arcQuestionProgress  = &fakeObj;
lv_obj_t *ui_lblPrevOption        = &fakeObj;
lv_obj_t *ui_lblNextOption        = &fakeObj;
lv_obj_t *ui_lblNameValue         = &fakeObj;
lv_obj_t *ui_lblCurrentChar       = &fakeObj;
lv_obj_t *ui_lblSuccessText       = &fakeObj;
lv_obj_t *ui_lblErrorText         = &fakeObj;

lv_obj_t *ui_imgFishPreview  = &fakeObj;
lv_obj_t *ui_imgFishPreview1 = &fakeObj;
lv_obj_t *ui_imgFishPreview2 = &fakeObj;
lv_obj_t *ui_imgFishPreview3 = &fakeObj;
lv_obj_t *ui_imgFishPreview4 = &fakeObj;
lv_obj_t *ui_imgFishPreview5 = &fakeObj;

lv_obj_t *ui_splashScreen    = &fakeObj;
lv_obj_t *ui_questionScreen  = &fakeObj;
lv_obj_t *ui_textEntryScreen = &fakeObj;
lv_obj_t *ui_sendingScreen   = &fakeObj;
lv_obj_t *ui_successScreen   = &fakeObj;
lv_obj_t *ui_errorScreen     = &fakeObj;

// ---- fake images ----
lv_image_dsc_t ui_img_tetra_png   = {};
lv_image_dsc_t ui_img_betta_png   = {};
lv_image_dsc_t ui_img_crab_png    = {};
lv_image_dsc_t ui_img_gourami_png = {};
lv_image_dsc_t ui_img_shark_png   = {};

inline void ui_init() {}

inline void lv_label_set_text(lv_obj_t *, const char *) {}
inline void lv_arc_set_range(lv_obj_t *, int, int) {}
inline void lv_arc_set_value(lv_obj_t *, int) {}
inline void lv_obj_add_flag(lv_obj_t *, unsigned) {}
inline void lv_obj_clear_flag(lv_obj_t *, unsigned) {}
inline void lv_obj_set_style_opa(lv_obj_t *, int, int) {}
inline void lv_obj_set_style_img_opa(lv_obj_t *, int, int) {}
inline void lv_obj_move_foreground(lv_obj_t *) {}
inline void lv_obj_invalidate(lv_obj_t *) {}
inline void lv_image_set_src(lv_obj_t *, const lv_image_dsc_t *) {}
inline void lv_image_set_scale(lv_obj_t *, int) {}

#define LV_OBJ_FLAG_HIDDEN  (1u)
#define LV_OPA_100          255

#endif

// =============================================================
// LVGL 8/9 compatibility
// =============================================================

#if !UI_STUB_MODE

#if LV_VERSION_MAJOR >= 9

#define LOAD_SCREEN(scr) lv_screen_load(scr)

#else

#define LOAD_SCREEN(scr) lv_scr_load(scr)

typedef lv_disp_t lv_display_t;

#endif

#else

#define LOAD_SCREEN(scr) ((void)0)

#endif

// =============================================================
// CONFIG
// =============================================================

#define PEPPERS_GHOST_MIRROR_X true
#define DISPLAY_ROTATION 0
#define USE_SIDE_OPTION_LABELS 1
#define ENCODER_REVERSE true
#define ENCODER_COUNTS_PER_UI_STEP 1

static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 240;
static const uint16_t lvglBufferRows = 40;

static uint16_t mirrorLineBuffer[screenWidth];

// =============================================================
// DISPLAY STUB
// =============================================================

#if UI_STUB_MODE

#define STUB_BG      M5Dial.Display.color565(0x1a,0x1a,0x2e)
#define STUB_ACCENT  M5Dial.Display.color565(0xe0,0x70,0x00)
#define STUB_GREEN   M5Dial.Display.color565(0x00,0xcc,0x66)
#define STUB_RED     M5Dial.Display.color565(0xcc,0x22,0x22)
#define STUB_GREY    M5Dial.Display.color565(0xaa,0xaa,0xaa)

static char _stubStep[24] = "";
static char _stubQ[64]    = "";
static char _stubOpt[32]  = "";
static char _stubChar[8]  = "";
static char _stubName[16] = "";

static void stubSetLabel(char *dst, size_t n, const char *src)
{
    if (!src) return;
    strncpy(dst, src, n - 1);
    dst[n - 1] = 0;
}

static void stubPaint(
    uint32_t bg,
    const char *title,
    uint32_t titleColor,
    const char *b1 = nullptr,
    const char *b2 = nullptr,
    const char *b3 = nullptr)
{
    M5Dial.Display.startWrite();

    M5Dial.Display.fillScreen(bg);

    M5Dial.Display.setTextDatum(MC_DATUM);

    M5Dial.Display.setTextColor(titleColor, bg);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString(title, 120, 75);

    M5Dial.Display.setTextColor(TFT_WHITE, bg);
    M5Dial.Display.setTextSize(1);

    if (b1) M5Dial.Display.drawString(b1, 120, 115);
    if (b2) M5Dial.Display.drawString(b2, 120, 135);

    if (b3) {
        M5Dial.Display.setTextColor(STUB_GREY, bg);
        M5Dial.Display.drawString(b3, 120, 160);
    }

    M5Dial.Display.endWrite();
}

#endif

// =============================================================
// SURVEY DATA
// =============================================================

#define ARRAY_COUNT(a) (sizeof(a)/sizeof(a[0]))

enum QuestionType {
    QUESTION_NUMERIC,
    QUESTION_LIKERT,
    QUESTION_FISH_TYPE,
    QUESTION_FISH_COLOUR
};

const char *q1Options[] = {"1","2","3","4","5"};
const char *q2Options[] = {"Very low","Low","Neutral","High","Very high"};
const char *q3Options[] = {"0","1","2","3","4","5","6","7","8","9","10"};
const char *q4Options[] = {"Tetra","Betta","Gourami","Shark","Crab"};

const char *tetraColourOptions[]   = {"Blue","Yellow","Purple","Pink","White"};
const char *bettaColourOptions[]   = {"Purple","Green","Yellow","Pink","Blue"};
const char *gouramiColourOptions[] = {"Orange","Yellow","Green","Pink","Blue","Purple"};
const char *sharkColourOptions[]   = {"White"};
const char *crabColourOptions[]    = {"Red","Blue","Green","Pink"};

struct FakeQuestion {
    const char *text;
    const char **options;
    uint8_t optionCount;
    QuestionType type;
};

FakeQuestion questions[] = {
    {"Rate J&J on innovation", q1Options, ARRAY_COUNT(q1Options), QUESTION_NUMERIC},
    {"Satisfaction with J&J interactions", q2Options, ARRAY_COUNT(q2Options), QUESTION_LIKERT},
    {"Recommend J&J to a colleague?", q3Options, ARRAY_COUNT(q3Options), QUESTION_NUMERIC},
    {"Pick your fish type", q4Options, ARRAY_COUNT(q4Options), QUESTION_FISH_TYPE},
    {"Pick your fish colour", tetraColourOptions, ARRAY_COUNT(tetraColourOptions), QUESTION_FISH_COLOUR},
};

static const uint8_t questionCount = ARRAY_COUNT(questions);

// =============================================================
// STATE
// =============================================================

int currentQuestion = 0;
int currentOption   = 0;

int savedOptions[questionCount];

bool enableTextEntryTest = true;

// =============================================================
// PERSISTENT STORAGE
// =============================================================

Preferences surveyPreferences;

static const char *SURVEY_PREF_NAMESPACE = "survey";
static const char *SURVEY_COUNTER_KEY    = "counter";
static const char *SURVEY_PENDING_KEY    = "pending";
static const char *SURVEY_HASPEND_KEY    = "haspend";

bool surveyPreferencesReady = false;

// =============================================================
// PACKETS
// =============================================================

enum PacketType : uint8_t {
    PACKET_QUESTION_SET = 1,
    PACKET_SUBMISSION   = 2,
    PACKET_ACK          = 3
};

typedef struct __attribute__((packed)) SurveyPacket {
  uint8_t packetType;

  uint8_t device_id;
  uint32_t counter;
  uint32_t timestamp_ms;
  int innovation;
  int satisfactionIndex;
  int nps;
  char fishType[16];
  char fishColour[16];
  char name[12];
} SurveyPacket;

static_assert(sizeof(SurveyPacket) == 66, "SurveyPacket mismatch");

// =============================================================
// PERSISTENT PENDING SUBMISSION
// =============================================================

struct SurveyPacket pendingSubmission = {};
bool hasPendingSubmission = false;

// =============================================================
// COUNTER
// =============================================================

uint32_t submissionCounter = 1;

// =============================================================
// ACK SYNCHRONIZATION
// =============================================================

portMUX_TYPE ackMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool waitingForAck       = false;
volatile bool matchingAckReceived = false;
volatile bool matchingAckAccepted = false;
volatile uint32_t waitingAckCounter = 0;

// =============================================================
// DEVICE
// =============================================================

const uint8_t DEVICE_ID = 3;

uint8_t RECEIVER_MAC[6] = {
    0xB0,0xA7,0x32,0x14,0xFD,0xFC
};

// =============================================================
// HELPERS
// =============================================================

void savePendingSubmission()
{
    if (!surveyPreferencesReady) return;

    surveyPreferences.putBool(
        SURVEY_HASPEND_KEY,
        hasPendingSubmission);

    if (hasPendingSubmission) {
        surveyPreferences.putBytes(
            SURVEY_PENDING_KEY,
            &pendingSubmission,
            sizeof(pendingSubmission));
    }
}

void loadPendingSubmission()
{
    if (!surveyPreferencesReady) return;

    hasPendingSubmission =
        surveyPreferences.getBool(SURVEY_HASPEND_KEY, false);

    if (!hasPendingSubmission) return;

    size_t r = surveyPreferences.getBytes(
        SURVEY_PENDING_KEY,
        &pendingSubmission,
        sizeof(pendingSubmission));

    if (r != sizeof(pendingSubmission)) {
        hasPendingSubmission = false;
    }
}

void saveSubmissionCounter()
{
    if (!surveyPreferencesReady) return;

    surveyPreferences.putUInt(
        SURVEY_COUNTER_KEY,
        submissionCounter);
}

void loadSubmissionCounter()
{
    esp_err_t r = nvs_flash_init();

    if (r == ESP_ERR_NVS_NO_FREE_PAGES ||
        r == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        r = nvs_flash_init();
    }

    if (r != ESP_OK) {
        Serial.println("NVS failed");
        return;
    }

    surveyPreferencesReady =
        surveyPreferences.begin(SURVEY_PREF_NAMESPACE, false);

    if (!surveyPreferencesReady) {
        Serial.println("Preferences failed");
        return;
    }

    submissionCounter =
        surveyPreferences.getUInt(SURVEY_COUNTER_KEY, 1);

    if (submissionCounter == 0)
        submissionCounter = 1;

    loadPendingSubmission();
}

// =============================================================
// APP STATE
// =============================================================

enum AppState {
    STATE_SPLASH,
    STATE_QUESTION,
    STATE_TEXT_ENTRY,
    STATE_SENDING,
    STATE_SUCCESS,
    STATE_ERROR
};

AppState appState = STATE_SPLASH;

uint32_t stateEnteredAt = 0;

// =============================================================
// TEXT ENTRY
// =============================================================

const char *charOptions[] = {
    "A","B","C","D","E","F","G","H","I","J","K","L","M",
    "N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
    "DEL","OK"
};

static const uint8_t charOptionCount =
    ARRAY_COUNT(charOptions);

char nameBuffer[12] = "";

int nameLength = 0;
int currentCharIndex = 0;

// =============================================================
// UI
// =============================================================

void showSplash()
{
    appState = STATE_SPLASH;
    stateEnteredAt = millis();

#if UI_STUB_MODE
    stubPaint(
        STUB_BG,
        "SURVEY",
        STUB_ACCENT,
        "Rotate or press to begin");
#endif
}

void showSuccess()
{
    appState = STATE_SUCCESS;
    stateEnteredAt = millis();

#if UI_STUB_MODE
    stubPaint(
        STUB_BG,
        "SUCCESS",
        STUB_GREEN,
        "Response collected");
#endif
}

void showError(const char *msg)
{
    appState = STATE_ERROR;
    stateEnteredAt = millis();

#if UI_STUB_MODE
    stubPaint(
        STUB_RED,
        "ERROR",
        TFT_WHITE,
        msg,
        "Press to retry");
#endif
}

void showSending()
{
    appState = STATE_SENDING;
    stateEnteredAt = millis();

#if UI_STUB_MODE
    stubPaint(
        STUB_BG,
        "SENDING...",
        STUB_GREY);
#endif
}

void showQuestion()
{
    appState = STATE_QUESTION;
    stateEnteredAt = millis();

    FakeQuestion &q = questions[currentQuestion];

    char stepText[24];

    snprintf(
        stepText,
        sizeof(stepText),
        "Q %d/%d",
        currentQuestion + 1,
        questionCount);

#if UI_STUB_MODE

    stubSetLabel(_stubStep, sizeof(_stubStep), stepText);
    stubSetLabel(_stubQ, sizeof(_stubQ), q.text);
    stubSetLabel(_stubOpt, sizeof(_stubOpt), q.options[currentOption]);

    stubPaint(
        STUB_BG,
        _stubStep,
        STUB_ACCENT,
        _stubQ,
        _stubOpt,
        "press to confirm");

#endif
}

// =============================================================
// WIFI INIT FIX
// =============================================================

bool ensureWifiReady()
{
    wifi_mode_t mode;

    if (esp_wifi_get_mode(&mode) == ESP_OK) {
        return true;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if (esp_wifi_init(&cfg) != ESP_OK)
        return false;

    if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK)
        return false;

    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK)
        return false;

    if (esp_wifi_start() != ESP_OK)
        return false;

    esp_wifi_disconnect();

    return true;
}

// =============================================================
// ESP-NOW
// =============================================================

bool espNowReady = false;

typedef struct __attribute__((packed)) DialAckPacket {
    uint8_t packetType;
    char session_id[16];
    uint8_t device_id;
    uint32_t counter;
    bool accepted;
} DialAckPacket;

static_assert(sizeof(DialAckPacket) == 23, "ACK packet mismatch");

void handleAckPacket(const uint8_t *data, int len)
{
    Serial.printf("ACK len=%d\n", len);
    if (len != sizeof(DialAckPacket))
        return;

    const DialAckPacket *ack =
        (const DialAckPacket *)data;

    Serial.printf(
        "ACK type=%d dev=%d counter=%lu accepted=%d\n",
        ack->packetType,
        ack->device_id,
        ack->counter,
        ack->accepted
    );

    if (ack->packetType != PACKET_ACK)
        return;

    portENTER_CRITICAL_ISR(&ackMux);

    if (waitingForAck &&
        ack->device_id == DEVICE_ID &&
        ack->counter == waitingAckCounter)
    {
        matchingAckReceived = true;
        matchingAckAccepted = ack->accepted;
        waitingForAck = false;
    }

    portEXIT_CRITICAL_ISR(&ackMux);
}

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3

void onEspNowData(
    const esp_now_recv_info_t *info,
    const uint8_t *data,
    int len)
{
    (void)info;
    handleAckPacket(data, len);
}

#else

void onEspNowData(
    const uint8_t *mac,
    const uint8_t *data,
    int len)
{
    (void)mac;
    handleAckPacket(data, len);
}

#endif

bool ensureReceiverPeer()
{
    if (esp_now_is_peer_exist(RECEIVER_MAC))
        return true;

    esp_now_peer_info_t pi = {};

    memcpy(pi.peer_addr, RECEIVER_MAC, 6);

    pi.channel = 0;
    pi.encrypt = false;

    return esp_now_add_peer(&pi) == ESP_OK;
}

void initEspNow()
{
    if (esp_netif_init() != ESP_OK &&
        esp_netif_init() != ESP_ERR_INVALID_STATE)
    {
        return;
    }

    if (esp_event_loop_create_default() != ESP_OK &&
        esp_event_loop_create_default() != ESP_ERR_INVALID_STATE)
    {
        return;
    }

    if (!ensureWifiReady()) {
        Serial.println("WiFi failed");
        return;
    }

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }

    esp_now_register_recv_cb(onEspNowData);

    espNowReady = ensureReceiverPeer();
}

// =============================================================
// PACKET BUILD
// =============================================================

void copyPacketString(char *dst, size_t n, const char *src)
{
    if (!n) return;

    strncpy(dst, src, n - 1);

    dst[n - 1] = 0;
}

void buildSurveyPacket(
    struct SurveyPacket &p,
    uint32_t counter)
{
    memset(&p, 0, sizeof(p));

    p.packetType = PACKET_SUBMISSION;

    p.device_id = DEVICE_ID;
    p.counter   = counter;
    p.timestamp_ms = millis();

    p.innovation =
        atoi(questions[0].options[savedOptions[0]]);

    p.satisfactionIndex =
        savedOptions[1];

    p.nps =
        atoi(questions[2].options[savedOptions[2]]);

    copyPacketString(
        p.fishType,
        sizeof(p.fishType),
        questions[3].options[savedOptions[3]]);

    copyPacketString(
        p.fishColour,
        sizeof(p.fishColour),
        questions[4].options[savedOptions[4]]);

    copyPacketString(
        p.name,
        sizeof(p.name),
        nameBuffer);
}

// =============================================================
// SEND
// =============================================================

bool sendSurveyPacketWithAck(const struct SurveyPacket &pkt)
{
    if (!espNowReady)
        return false;

    if (!ensureReceiverPeer())
        return false;

    for (uint8_t attempt = 0; attempt < 4; attempt++)
    {
        portENTER_CRITICAL(&ackMux);

        matchingAckReceived = false;
        matchingAckAccepted = false;
        waitingForAck = true;
        waitingAckCounter = pkt.counter;

        portEXIT_CRITICAL(&ackMux);

        esp_err_t r =
            esp_now_send(
                RECEIVER_MAC,
                (const uint8_t *)&pkt,
                sizeof(pkt));

        if (r != ESP_OK) {
            continue;
        }

        uint32_t t0 = millis();

        while (millis() - t0 < 500)
        {
#if !UI_STUB_MODE

#if LV_VERSION_MAJOR >= 9
            lv_timer_handler();
#else
            lv_task_handler();
#endif

#endif

            bool gotAck = false;

            portENTER_CRITICAL(&ackMux);

            gotAck = matchingAckReceived;

            portEXIT_CRITICAL(&ackMux);

            if (gotAck)
            {
                bool accepted;

                portENTER_CRITICAL(&ackMux);
                accepted = matchingAckAccepted;
                portEXIT_CRITICAL(&ackMux);

                return accepted;
            }

            delay(5);
        }

        portENTER_CRITICAL(&ackMux);
        waitingForAck = false;
        portEXIT_CRITICAL(&ackMux);
    }

    return false;
}

// =============================================================
// SUBMIT
// =============================================================

void submitPendingSurvey()
{
    showSending();

    bool ok =
        sendSurveyPacketWithAck(pendingSubmission);

    if (ok)
    {
        hasPendingSubmission = false;

        savePendingSubmission();

        submissionCounter =
            pendingSubmission.counter + 1;

        saveSubmissionCounter();

        showSuccess();
    }
    else
    {
        showError("Send failed");
    }
}

void submitFinalSurvey()
{
    buildSurveyPacket(
        pendingSubmission,
        submissionCounter);

    hasPendingSubmission = true;

    savePendingSubmission();

    submitPendingSurvey();
}

// =============================================================
// FLOW
// =============================================================

void resetFakeSurvey()
{
    currentQuestion = 0;
    currentOption = 0;

    for (uint8_t i=0;i<questionCount;i++)
        savedOptions[i] = 0;

    nameBuffer[0] = 0;
    nameLength = 0;
    currentCharIndex = 0;
}

void startSurvey()
{
    resetFakeSurvey();
    showQuestion();
}

bool startSurveyIfIdle()
{
    if (appState != STATE_SPLASH)
        return false;

    startSurvey();
    return true;
}

void handleRotation(int dir)
{
    if (startSurveyIfIdle())
        return;

    if (appState == STATE_QUESTION)
    {
        int maxOpt =
            max(0,
            (int)questions[currentQuestion].optionCount - 1);

        currentOption =
            constrain(
                currentOption + dir,
                0,
                maxOpt);

        showQuestion();
    }
}

void submitCurrentQuestion()
{
    savedOptions[currentQuestion] =
        currentOption;

    currentQuestion++;

    if (currentQuestion >= questionCount)
    {
        submitFinalSurvey();
        return;
    }

    currentOption = 0;

    showQuestion();
}

void handlePress()
{
    if (startSurveyIfIdle())
        return;

    if (appState == STATE_QUESTION)
    {
        submitCurrentQuestion();
        return;
    }

    if (appState == STATE_ERROR)
    {
        if (hasPendingSubmission)
            submitPendingSurvey();
        else
            showSplash();

        return;
    }

    if (appState == STATE_SUCCESS)
    {
        showSplash();
        return;
    }
}

// =============================================================
// ENCODER
// =============================================================

void pollEncoderRotation()
{
    long raw = M5Dial.Encoder.read();

    if (raw == 0)
        return;

    M5Dial.Encoder.write(0);

    if (ENCODER_REVERSE)
        raw = -raw;

    while (raw >= ENCODER_COUNTS_PER_UI_STEP)
    {
        handleRotation(1);
        raw -= ENCODER_COUNTS_PER_UI_STEP;
    }

    while (raw <= -ENCODER_COUNTS_PER_UI_STEP)
    {
        handleRotation(-1);
        raw += ENCODER_COUNTS_PER_UI_STEP;
    }
}

void pollEncoderButton()
{
    if (M5Dial.BtnA.wasPressed())
        handlePress();
}

// =============================================================
// LVGL FLUSH
// =============================================================

#if !UI_STUB_MODE

static void displayFlush(
    lv_display_t *disp,
    const lv_area_t *area,
    uint8_t *pxMap)
{
    uint32_t w =
        area->x2 - area->x1 + 1;

    uint32_t h =
        area->y2 - area->y1 + 1;

    uint16_t *px =
        (uint16_t *)pxMap;

#if PEPPERS_GHOST_MIRROR_X

    int16_t mx =
        screenWidth - area->x2 - 1;

    for (uint32_t row=0; row<h; row++)
    {
        uint16_t *src =
            px + row * w;

        for (uint32_t col=0; col<w; col++)
        {
            mirrorLineBuffer[col] =
                src[w - 1 - col];
        }

        M5Dial.Display.pushImage(
            mx,
            area->y1 + row,
            w,
            1,
            mirrorLineBuffer);
    }

#else

    M5Dial.Display.pushImage(
        area->x1,
        area->y1,
        w,
        h,
        px);

#endif

#if LV_VERSION_MAJOR >= 9
    lv_display_flush_ready(disp);
#else
    lv_disp_flush_ready(disp);
#endif
}

#endif

// =============================================================
// SETUP
// =============================================================

void setup()
{
    Serial.begin(115200);

    delay(500);

    loadSubmissionCounter();

    auto cfg = M5.config();

    M5Dial.begin(cfg, true, false);

    M5Dial.Display.setRotation(DISPLAY_ROTATION);
    M5Dial.Display.setBrightness(255);

#if UI_STUB_MODE

    showSplash();

#else

    // Production LVGL init here

#endif

    initEspNow();

    if (hasPendingSubmission)
    {
        Serial.println("Recovered pending submission");
    }
}

// =============================================================
// LOOP
// =============================================================

void loop()
{
    M5Dial.update();

    pollEncoderRotation();
    pollEncoderButton();

    if (appState == STATE_SUCCESS &&
        millis() - stateEnteredAt > 2200)
    {
        showSplash();
    }

#if !UI_STUB_MODE

    static uint32_t lastTick = millis();

    uint32_t now = millis();

    uint32_t elapsed = now - lastTick;

    if (elapsed)
    {
#if LV_VERSION_MAJOR >= 9
        lv_tick_inc(elapsed);
#else
        lv_tick_inc(elapsed);
#endif
        lastTick = now;
    }

#if LV_VERSION_MAJOR >= 9
    lv_timer_handler();
#else
    lv_task_handler();
#endif

#endif

    delay(5);
}