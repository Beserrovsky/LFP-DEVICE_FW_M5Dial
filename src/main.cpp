#include <M5Dial.h>
#include <lvgl.h>
#include "DisplayDriver.h"
#include "UIManager.h"
#include "AppState.h"
#include "NFCManager.h"
#include "ESPNowManager.h"
#include "SurveyConfig.h"

extern DisplayDriver g_displayDriver;
extern UIManager g_uiManager;
extern AppState g_appState;
extern NFCManager g_nfcManager;
extern ESPNowManager g_espNowManager;

// ── Home screen state (declared early — referenced by goHome()) ───────────────
static uint8_t  homeClickCount = 0;
static bool     nfcTagShowing  = false;
static uint32_t nfcTagShowTime = 0;
static const uint32_t NFC_SHOW_MS = 1500;

// ── Helper: option label lookup with boundary blanks ─────────────────────────

static const char* optionLabel(const char* const* options, uint8_t numOptions, int idx) {
    if (idx < 0 || idx >= (int)numOptions) return "";
    return options[idx];
}

// Side option label: shows "..." when the slot is occupied but the question
// uses long names that would overflow the side slots (e.g. the last question).
static const char* sideOptionLabel(uint8_t qIdx, const char* const* options, uint8_t numOptions, int idx) {
    if (idx < 0 || idx >= (int)numOptions) return "";
    if (qIdx == NUM_QUESTIONS - 1) return "...";
    return options[idx];
}

// ── Tutorial UI refresh ───────────────────────────────────────────────────────

static const uint8_t TUTORIAL_STEP1_NUM = 2; // ["X", "OK"]

// Step 3 (T1_reversed): OK is to the LEFT — user must rotate left to reach it.
// List: ["OK", "X"]  index 0 = OK (left), index 1 = X (start)
static const char* const TUTORIAL_STEP3_OPTIONS[] = { "OK", "X" };
static const uint8_t TUTORIAL_STEP3_NUM     = 2;
static const uint8_t TUTORIAL_STEP3_START   = 1;   // start on X
static const uint8_t TUTORIAL_STEP3_OK_IDX  = 0;   // OK is at index 0 (left)

static void refreshTutorialStep1(const char* instruction, uint8_t optIdx) {
    g_uiManager.showQuestion(
        "Tutorial", "1/2", instruction, 0,
        optionLabel(TUTORIAL_STEP1_OPTIONS, TUTORIAL_STEP1_NUM, (int)optIdx),
        optionLabel(TUTORIAL_STEP1_OPTIONS, TUTORIAL_STEP1_NUM, (int)optIdx - 1),
        optionLabel(TUTORIAL_STEP1_OPTIONS, TUTORIAL_STEP1_NUM, (int)optIdx + 1)
    );
}

// ── Screen transition helpers ─────────────────────────────────────────────────

static void goHome() {
    homeClickCount = 0;
    nfcTagShowing  = false;
    g_appState.setCurrentScreen(SCREEN_HOME);
    g_uiManager.showHome();
    Serial.println("[STATE] → Home");
}

static void startTutorial() {
    g_appState.setTutorialStep(0);
    g_appState.setCurrentOption(TUTORIAL_STEP1_START);
    g_appState.setCurrentScreen(SCREEN_TUTORIAL);
    refreshTutorialStep1("Rotate the dial until OK.", TUTORIAL_STEP1_START);
    Serial.println("[STATE] → Tutorial step 0");
}

static void startQuestion(uint8_t qIdx) {
    g_appState.setCurrentQuestion(qIdx);
    g_appState.setCurrentOption(0);
    g_appState.setCurrentScreen(SCREEN_QUESTION);

    const QuestionDef& q = QUESTIONS[qIdx];
    char num[8];
    snprintf(num, sizeof(num), "%d/5", qIdx + 1);

    g_uiManager.showQuestion(
        "Question", num, q.text, (int)(qIdx + 1),
        optionLabel(q.options, q.numOptions, 0),
        "",
        sideOptionLabel(qIdx, q.options, q.numOptions, 1)
    );
    Serial.printf("[STATE] → Question %d\n", qIdx + 1);
}

// Go back to a question restoring the previously saved option selection.
static void goBackToQuestion(uint8_t qIdx) {
    uint8_t savedOpt = g_appState.getSavedOption(qIdx);
    g_appState.setCurrentQuestion(qIdx);
    g_appState.setCurrentOption(savedOpt);
    g_appState.setCurrentScreen(SCREEN_QUESTION);

    const QuestionDef& q = QUESTIONS[qIdx];
    char num[8];
    snprintf(num, sizeof(num), "%d/5", qIdx + 1);
    g_uiManager.showQuestion(
        "Question", num, q.text, (int)(qIdx + 1),
        optionLabel(    q.options, q.numOptions, (int)savedOpt),
        sideOptionLabel(qIdx, q.options, q.numOptions, (int)savedOpt - 1),
        sideOptionLabel(qIdx, q.options, q.numOptions, (int)savedOpt + 1)
    );
    Serial.printf("[STATE] → Back to Question %d (option %d)\n", qIdx + 1, savedOpt);
}

static void goConfirm() {
    g_appState.setCurrentScreen(SCREEN_CONFIRM);
    g_uiManager.showConfirm();
    Serial.println("[STATE] → Confirm");
}

static uint32_t  waitOrClickEnterTime    = 0;
static AppScreen waitOrClickReturnScreen = SCREEN_HOME;

static void goWaitOrClick(bool success, const char* message,
                           AppScreen returnOnError = SCREEN_CONFIRM) {
    waitOrClickEnterTime    = millis();
    waitOrClickReturnScreen = returnOnError;
    g_appState.setWaitOrClickIsSuccess(success);
    g_appState.setWaitOrClickMessage(message);
    g_appState.setCurrentScreen(SCREEN_WAIT_OR_CLICK);
    g_uiManager.showWaitOrClick(message);
    Serial.printf("[STATE] → WaitOrClick (%s)\n", success ? "success" : "error");
}

// ── Screen handlers ───────────────────────────────────────────────────────────

static void handleHome(uint8_t clicks, int /*encoderDelta*/) {
    // NFC tag briefly shown — wait for timer then transition
    if (nfcTagShowing) {
        if (millis() - nfcTagShowTime >= NFC_SHOW_MS) {
            nfcTagShowing = false;
            g_nfcManager.clearDetection();
            M5Dial.Speaker.tone(3000, 100);
            startTutorial();
        }
        return;
    }

    // Poll NFC
    g_nfcManager.update();

    if (g_nfcManager.hasNewTag()) {
        if (g_nfcManager.isValidTag()) {
            g_appState.setNameBuffer(g_nfcManager.getExtractedName());
            char msg[32];
            snprintf(msg, sizeof(msg), "Hello, %s!", g_nfcManager.getExtractedName());
            g_uiManager.setHomeLabelText(msg);
            nfcTagShowing = true;
            nfcTagShowTime = millis();
        } else {
            char errMsg[50];
            snprintf(errMsg, sizeof(errMsg), "Error!\n\n%.37s",
                     g_nfcManager.getValidationError().c_str());
            g_nfcManager.clearDetection();
            goWaitOrClick(false, errMsg, SCREEN_HOME);
        }
        return;
    }

    // Debug path: 5 clicks
    if (clicks > 0) {
        homeClickCount += clicks;
        Serial.printf("[Home] click count: %d\n", homeClickCount);
        if (homeClickCount >= 5) {
            homeClickCount = 0;
            M5Dial.Speaker.tone(3000, 100);
            startTutorial();
        }
    }
}

static void handleTutorial(uint8_t clicks, int encoderDelta) {
    uint8_t step = g_appState.getTutorialStep();
    uint8_t opt  = g_appState.getCurrentOption();

    if (step == 0) {
        // T1_init: rotate to reach OK (index 2)
        if (encoderDelta != 0) {
            int newOpt = (int)opt + (encoderDelta > 0 ? 1 : -1);
            if (newOpt < (int)TUTORIAL_STEP1_START) newOpt = TUTORIAL_STEP1_START;
            if (newOpt > (int)TUTORIAL_STEP1_OK_IDX) newOpt = TUTORIAL_STEP1_OK_IDX;
            g_appState.setCurrentOption((uint8_t)newOpt);
            opt = (uint8_t)newOpt;

            if (opt == TUTORIAL_STEP1_OK_IDX) {
                g_appState.setTutorialStep(1);
                refreshTutorialStep1("Click once to advance.", opt);
                Serial.println("[Tutorial] reached OK");
            } else {
                refreshTutorialStep1("Rotate the dial until OK.", opt);
            }
        }
    } else if (step == 1) {
        // T1_rotated: on OK, wait for 1 click
        if (encoderDelta < 0) {
            // scroll back off OK
            g_appState.setCurrentOption(TUTORIAL_STEP1_START);
            g_appState.setTutorialStep(0);
            refreshTutorialStep1("Rotate the dial until OK.", TUTORIAL_STEP1_START);
        }
        if (clicks == 1) {
            // advance to Tutorial step 2
            g_appState.setTutorialStep(2);
            g_appState.setCurrentOption(2); // middle of 5 X's
            g_uiManager.showQuestion(
                "Tutorial", "2/2", "Click twice to go back.", 1,
                "X", "X", "X"
            );
            Serial.println("[Tutorial] → step 2");
        }
    } else if (step == 2) {
        // T2: encoder scrolls freely within 5 X's; double click goes back
        if (encoderDelta != 0) {
            int newOpt = (int)opt + (encoderDelta > 0 ? 1 : -1);
            if (newOpt < 0) newOpt = 0;
            if (newOpt >= (int)TUTORIAL_STEP2_NUM) newOpt = TUTORIAL_STEP2_NUM - 1;
            g_appState.setCurrentOption((uint8_t)newOpt);
            opt = (uint8_t)newOpt;

            g_uiManager.showQuestion(
                "Tutorial", "2/2", "Click twice to go back.", 1,
                optionLabel(TUTORIAL_STEP2_OPTIONS, TUTORIAL_STEP2_NUM, (int)opt),
                optionLabel(TUTORIAL_STEP2_OPTIONS, TUTORIAL_STEP2_NUM, (int)opt - 1),
                optionLabel(TUTORIAL_STEP2_OPTIONS, TUTORIAL_STEP2_NUM, (int)opt + 1)
            );
        }
        if (clicks == 2) {
            // Return to T1 reversed: OK is now on the LEFT
            g_appState.setTutorialStep(3);
            g_appState.setCurrentOption(TUTORIAL_STEP3_START);
            g_uiManager.showQuestion(
                "Tutorial", "1/2", "Rotate the dial to OK.", 0,
                optionLabel(TUTORIAL_STEP3_OPTIONS, TUTORIAL_STEP3_NUM, TUTORIAL_STEP3_START),
                optionLabel(TUTORIAL_STEP3_OPTIONS, TUTORIAL_STEP3_NUM, (int)TUTORIAL_STEP3_START - 1),
                optionLabel(TUTORIAL_STEP3_OPTIONS, TUTORIAL_STEP3_NUM, (int)TUTORIAL_STEP3_START + 1)
            );
            Serial.println("[Tutorial] → step 3 (T1 reversed, OK on left)");
        }
    } else if (step == 3) {
        // T1_reversed: rotate LEFT to reach OK (index 0), then click to advance
        if (encoderDelta != 0) {
            int newOpt = (int)opt + (encoderDelta > 0 ? 1 : -1);
            if (newOpt < 0) newOpt = 0;
            if (newOpt >= (int)TUTORIAL_STEP3_NUM) newOpt = TUTORIAL_STEP3_NUM - 1;
            g_appState.setCurrentOption((uint8_t)newOpt);
            opt = (uint8_t)newOpt;

            if (opt == TUTORIAL_STEP3_OK_IDX) {
                g_uiManager.showQuestion(
                    "Tutorial", "1/2", "Click once to advance.", 0,
                    "OK", "", ""
                );
            } else {
                g_uiManager.showQuestion(
                    "Tutorial", "1/2", "Rotate the dial to OK.", 0,
                    optionLabel(TUTORIAL_STEP3_OPTIONS, TUTORIAL_STEP3_NUM, (int)opt),
                    optionLabel(TUTORIAL_STEP3_OPTIONS, TUTORIAL_STEP3_NUM, (int)opt - 1),
                    optionLabel(TUTORIAL_STEP3_OPTIONS, TUTORIAL_STEP3_NUM, (int)opt + 1)
                );
            }
        }
        if (clicks == 1 && opt == TUTORIAL_STEP3_OK_IDX) {
            g_appState.setTutorialStep(4);
            g_uiManager.showQuestion(
                "Tutorial", "2/2", "You are ready. Click once to proceed.", 1,
                "GO", "", ""
            );
            Serial.println("[Tutorial] → step 4 (T2 ready)");
        }
    } else if (step == 4) {
        // T2_ready: single click ends tutorial
        if (clicks == 1) {
            M5Dial.Speaker.tone(2000, 200);
            startQuestion(0);
        }
    }
}

static void handleQuestion(uint8_t clicks, int encoderDelta) {
    uint8_t qIdx = g_appState.getCurrentQuestion();
    uint8_t opt  = g_appState.getCurrentOption();
    const QuestionDef& q = QUESTIONS[qIdx];

    if (encoderDelta != 0) {
        int newOpt = (int)opt + (encoderDelta > 0 ? 1 : -1);
        if (newOpt < 0) newOpt = 0;
        if (newOpt >= (int)q.numOptions) newOpt = (int)q.numOptions - 1;
        g_appState.setCurrentOption((uint8_t)newOpt);
        opt = (uint8_t)newOpt;

        char num[8];
        snprintf(num, sizeof(num), "%d/5", qIdx + 1);
        g_uiManager.showQuestion(
            "Question", num, q.text, (int)(qIdx + 1),
            optionLabel(    q.options, q.numOptions, (int)opt),
            sideOptionLabel(qIdx, q.options, q.numOptions, (int)opt - 1),
            sideOptionLabel(qIdx, q.options, q.numOptions, (int)opt + 1)
        );
    }

    if (clicks == 1) {
        g_appState.setSavedOption(qIdx, opt);
        Serial.printf("[Q%d] saved option %d (%s)\n", qIdx + 1, opt, q.options[opt]);
        M5Dial.Speaker.tone(3000, 100);

        if (qIdx + 1 < NUM_QUESTIONS) {
            startQuestion(qIdx + 1);
        } else {
            goConfirm();
        }
    } else if (clicks == 2 && qIdx > 0) {
        Serial.printf("[Q%d] double click → back to Q%d\n", qIdx + 1, qIdx);
        goBackToQuestion(qIdx - 1);
    }
}

static void handleConfirm(uint8_t clicks, int /*encoderDelta*/) {
    if (clicks == 1) {
        Serial.println("[Confirm] single click → submit");

        SurveyPacket pkt = {};
        pkt.packetType   = PACKET_SUBMISSION;
        pkt.device_id    = 3;
        pkt.counter      = millis() & 0xFFFF;
        pkt.timestamp_ms = millis();

        pkt.innovation        = (int)g_appState.getSavedOption(0) + 1;
        pkt.satisfactionIndex = (int)g_appState.getSavedOption(1);
        pkt.nps               = (int)g_appState.getSavedOption(2);

        uint8_t fishIdx = g_appState.getSavedOption(3);
        strncpy(pkt.fishType,   QUESTIONS[3].options[fishIdx], sizeof(pkt.fishType)   - 1);
        strncpy(pkt.fishColour, "",                            sizeof(pkt.fishColour) - 1);
        strncpy(pkt.name,       g_appState.getNameBuffer(),    sizeof(pkt.name)       - 1);

        bool ok = g_espNowManager.sendSurvey(pkt);
        Serial.printf("[ESP-NOW] Survey send: %s\n", ok ? "SUCCESS" : "FAILED");

        if (ok) {
            goWaitOrClick(true, MSG_SUCCESS);
        } else {
            goWaitOrClick(false, MSG_SEND_FAILED);
        }
    } else if (clicks == 2) {
        Serial.println("[Confirm] double click → back to Q4");
        goBackToQuestion(NUM_QUESTIONS - 1);
    }
}

static const uint32_t WAIT_OR_CLICK_TIMEOUT_MS = 7000;

static void handleWaitOrClick(uint8_t clicks, int /*encoderDelta*/) {
    uint32_t elapsed = millis() - waitOrClickEnterTime;

    // Drive the arc fill to show time remaining
    int percent = (int)((uint64_t)elapsed * 100 / WAIT_OR_CLICK_TIMEOUT_MS);
    g_uiManager.setWaitOrClickProgress(percent);

    if (clicks >= 1 || elapsed >= WAIT_OR_CLICK_TIMEOUT_MS) {
        if (g_appState.getWaitOrClickIsSuccess()) {
            g_appState.reset();
            goHome();
        } else if (waitOrClickReturnScreen == SCREEN_HOME) {
            g_appState.reset();
            goHome();
            Serial.println("[WaitOrClick] error → back to Home");
        } else {
            g_appState.setCurrentScreen(SCREEN_CONFIRM);
            g_uiManager.showConfirm();
            Serial.println("[WaitOrClick] error → back to Confirm");
        }
    }
}

// ── Long press tracking ───────────────────────────────────────────────────────

static bool mirror2sFired = false;

static void handleLongPress() {
    if (M5Dial.BtnA.pressedFor(5000)) {
        M5Dial.Speaker.tone(1500, 100);
        delay(150);
        M5Dial.Speaker.tone(1500, 100);
        delay(150);
        M5Dial.Speaker.tone(1500, 100);
        delay(150);
        ESP.restart();
    }
    if (M5Dial.BtnA.pressedFor(2000) && !mirror2sFired) {
        g_displayDriver.setMirrorEnabled(!g_displayDriver.getMirrorEnabled());
        lv_obj_invalidate(lv_scr_act());
        M5Dial.Speaker.tone(2500, 100);
        delay(150);
        M5Dial.Speaker.tone(2500, 100);
        mirror2sFired = true;
    }
    if (!M5Dial.BtnA.isPressed()) {
        mirror2sFired = false;
    }
}

// ── Click accumulation ────────────────────────────────────────────────────────

static uint8_t  pendingClicks  = 0;
static uint32_t lastClickTime  = 0;
static bool     prevPressed    = false;
static const uint32_t CLICK_WINDOW_MS = 350;

static uint8_t pollClicks() {
    bool isPressed = M5Dial.BtnA.isPressed();

    if (isPressed && !prevPressed) {
        pendingClicks++;
        lastClickTime = millis();
        M5Dial.Speaker.tone(4000, 30);
    }
    prevPressed = isPressed;

    if (pendingClicks > 0 && !isPressed &&
        (millis() - lastClickTime) >= CLICK_WINDOW_MS) {
        uint8_t c = pendingClicks;
        pendingClicks = 0;
        return c;
    }
    return 0;
}

// ── Encoder delta ─────────────────────────────────────────────────────────────

static long lastEncoderPos = 0;

static int pollEncoder() {
    long pos = M5Dial.Encoder.read();
    int delta = (int)(pos - lastEncoderPos);
    if (delta != 0) {
        lastEncoderPos = pos;
        M5Dial.Speaker.tone(8000, 20);
    }
    return delta;
}

// ── Arduino entry points ──────────────────────────────────────────────────────

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
    if (!g_nfcManager.init()) {
        Serial.println("[NFC] Init failed — NFC disabled");
    }
    g_espNowManager.begin();

    // Start on Home screen
    goHome();

    delay(500);
}

void loop() {
    static uint32_t lastTick = millis();

    uint32_t now     = millis();
    uint32_t elapsed = now - lastTick;
    if (elapsed > 0) {
        g_displayDriver.update(elapsed);
        lastTick = now;
    }

    M5Dial.update();

    handleLongPress();

    // Long press consumed the press — don't also count it as a click
    if (mirror2sFired || M5Dial.BtnA.pressedFor(2000)) {
        prevPressed = M5Dial.BtnA.isPressed();
        delay(5);
        return;
    }

    uint8_t clicks       = pollClicks();
    int     encoderDelta = pollEncoder();

    switch (g_appState.getCurrentScreen()) {
        case SCREEN_HOME:
            handleHome(clicks, encoderDelta);
            break;
        case SCREEN_TUTORIAL:
            handleTutorial(clicks, encoderDelta);
            break;
        case SCREEN_QUESTION:
            handleQuestion(clicks, encoderDelta);
            break;
        case SCREEN_CONFIRM:
            handleConfirm(clicks, encoderDelta);
            break;
        case SCREEN_WAIT_OR_CLICK:
            handleWaitOrClick(clicks, encoderDelta);
            break;
    }

    delay(5);
}
