#include <M5Dial.h>
#include <lvgl.h>
#include "DisplayDriver.h"
#include "UIManager.h"
#include "AppState.h"
#include "ESPNowManager.h"
#include "SurveyConfig.h"

extern DisplayDriver g_displayDriver;
extern UIManager g_uiManager;
extern AppState g_appState;
extern ESPNowManager g_espNowManager;

// ── Helper: option label lookup with boundary blanks ─────────────────────────

static const char* optionLabel(const char* const* options, uint8_t numOptions, int idx) {
    if (idx < 0 || idx >= (int)numOptions) return "";
    return options[idx];
}

// ── Tutorial UI refresh ───────────────────────────────────────────────────────

static void refreshTutorialStep1(const char* instruction, uint8_t optIdx) {
    g_uiManager.showQuestion(
        "Tutorial", "1/2", instruction, 0,
        optionLabel(TUTORIAL_STEP1_OPTIONS, 3, (int)optIdx),
        optionLabel(TUTORIAL_STEP1_OPTIONS, 3, (int)optIdx - 1),
        optionLabel(TUTORIAL_STEP1_OPTIONS, 3, (int)optIdx + 1)
    );
}

// ── Screen transition helpers ─────────────────────────────────────────────────

static void goHome() {
    g_appState.setCurrentScreen(SCREEN_HOME);
    g_uiManager.showHome();
    Serial.println("[STATE] → Home");
}

static void startTutorial() {
    g_appState.setTutorialStep(0);
    g_appState.setCurrentOption(TUTORIAL_STEP1_START);
    g_appState.setCurrentScreen(SCREEN_TUTORIAL);
    refreshTutorialStep1("rotate the dial until OK", TUTORIAL_STEP1_START);
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
        optionLabel(q.options, q.numOptions, 1)
    );
    Serial.printf("[STATE] → Question %d\n", qIdx + 1);
}

static void goConfirm() {
    g_appState.setCurrentScreen(SCREEN_CONFIRM);
    g_uiManager.showConfirm();
    Serial.println("[STATE] → Confirm");
}

static uint32_t waitOrClickEnterTime = 0;

static void goWaitOrClick(bool success, const char* message) {
    waitOrClickEnterTime = millis();
    g_appState.setWaitOrClickIsSuccess(success);
    g_appState.setWaitOrClickMessage(message);
    g_appState.setCurrentScreen(SCREEN_WAIT_OR_CLICK);
    g_uiManager.showWaitOrClick(message);
    Serial.printf("[STATE] → WaitOrClick (%s)\n", success ? "success" : "error");
}

// ── Screen handlers ───────────────────────────────────────────────────────────

static uint8_t homeClickCount = 0;

static void handleHome(uint8_t clicks, int /*encoderDelta*/) {
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
                refreshTutorialStep1("click once to advance", opt);
                Serial.println("[Tutorial] reached OK");
            } else {
                refreshTutorialStep1("rotate the dial until OK", opt);
            }
        }
    } else if (step == 1) {
        // T1_rotated: on OK, wait for 1 click
        if (encoderDelta < 0) {
            // scroll back off OK
            g_appState.setCurrentOption(TUTORIAL_STEP1_START);
            g_appState.setTutorialStep(0);
            refreshTutorialStep1("rotate the dial until OK", TUTORIAL_STEP1_START);
        }
        if (clicks == 1) {
            // advance to Tutorial step 2
            g_appState.setTutorialStep(2);
            g_appState.setCurrentOption(2); // middle of 5 X's
            g_uiManager.showQuestion(
                "Tutorial", "2/2", "click twice to go back", 1,
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
                "Tutorial", "2/2", "click twice to go back", 1,
                optionLabel(TUTORIAL_STEP2_OPTIONS, TUTORIAL_STEP2_NUM, (int)opt),
                optionLabel(TUTORIAL_STEP2_OPTIONS, TUTORIAL_STEP2_NUM, (int)opt - 1),
                optionLabel(TUTORIAL_STEP2_OPTIONS, TUTORIAL_STEP2_NUM, (int)opt + 1)
            );
        }
        if (clicks == 2) {
            // return to T1 confirmed
            g_appState.setTutorialStep(3);
            g_appState.setCurrentOption(TUTORIAL_STEP1_OK_IDX);
            g_uiManager.showQuestion(
                "Tutorial", "1/2", "click once again to end tutorial", 0,
                "OK", "", ""
            );
            Serial.println("[Tutorial] → step 3 (T1 confirmed)");
        }
    } else if (step == 3) {
        // T1_confirmed: 1 click ends tutorial
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
            optionLabel(q.options, q.numOptions, (int)opt),
            optionLabel(q.options, q.numOptions, (int)opt - 1),
            optionLabel(q.options, q.numOptions, (int)opt + 1)
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
    }
}

static void handleConfirm(uint8_t clicks, int /*encoderDelta*/) {
    if (clicks == 1) {
        Serial.println("[Confirm] single click → submit");
        // Phase 3: real ESP-NOW send goes here
        goWaitOrClick(true, MSG_SUCCESS);
    } else if (clicks == 2) {
        Serial.println("[Confirm] double click → back to Q4");
        startQuestion(NUM_QUESTIONS - 1);
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
