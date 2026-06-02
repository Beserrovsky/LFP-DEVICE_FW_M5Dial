#include <lvgl.h>
#include "UIManager.h"
#include <ui.h>
#include <stdio.h>
#include <string.h>

UIManager g_uiManager;

UIManager::UIManager()
    : currentState(UI_STATE_SPLASH) {
}

UIManager::~UIManager() {
}

bool UIManager::init() {
    currentState = UI_STATE_SPLASH;
    ui_init();
    return true;
}

void UIManager::showSplash() {
    currentState = UI_STATE_SPLASH;
}

void UIManager::showQuestion(uint8_t questionNum, uint8_t totalQuestions,
                              const char* text, const char* currentOption,
                              const char* prevOption, const char* nextOption) {
    currentState = UI_STATE_QUESTION;

    char stepBuf[32];
    snprintf(stepBuf, sizeof(stepBuf), "Q %d/%d", questionNum, totalQuestions);

    lv_label_set_text(ui_lblQuestionStep1, stepBuf);
    lv_label_set_text(ui_lblQuestionText1, text);
    lv_label_set_text(ui_lblCurrentOption1, currentOption);
    lv_label_set_text(ui_lblPrevOption1, prevOption);
    lv_label_set_text(ui_lblNextOption1, nextOption);

    lv_disp_load_scr(ui_questionScreen1);
}

void UIManager::showTextEntry(const char* currentName) {
    currentState = UI_STATE_TEXT_ENTRY;
}

void UIManager::showSending() {
    currentState = UI_STATE_SENDING;
}

void UIManager::showSuccess() {
    currentState = UI_STATE_SUCCESS;
}

void UIManager::showError(const char* errorMessage) {
    currentState = UI_STATE_ERROR;
    (void)errorMessage;
}

void UIManager::setCurrentOption(int value) {
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", value);
    lv_label_set_text(ui_lblCurrentOption1, buf);
}

void UIManager::setPreviousOption(int value) {
    char buf[4];
    if (value == 0) {
        strcpy(buf, " ");
    } else {
        snprintf(buf, sizeof(buf), "%d", value);
    }
    lv_label_set_text(ui_lblPrevOption1, buf);
}

void UIManager::setNextOption(int value) {
    char buf[4];
    if (value == 10) {
        strcpy(buf, " ");
    } else {
        snprintf(buf, sizeof(buf), "%d", value);
    }
    lv_label_set_text(ui_lblNextOption1, buf);
}

UIState UIManager::getCurrentState() const {
    return currentState;
}

