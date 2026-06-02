#include "UIManager.h"

UIManager g_uiManager;

UIManager::UIManager()
    : currentState(UI_STATE_SPLASH), stateEnteredAt(0) {
}

UIManager::~UIManager() {
}

bool UIManager::init() {
    return true;
}

void UIManager::update() {
}

void UIManager::showSplash() {
    currentState = UI_STATE_SPLASH;
    stateEnteredAt = 0;
}

void UIManager::showQuestion(uint8_t questionIndex, uint8_t optionCount,
                              const char* questionText, const char* currentOption) {
    (void)questionIndex;
    (void)optionCount;
    (void)questionText;
    (void)currentOption;
    currentState = UI_STATE_QUESTION;
    stateEnteredAt = 0;
}

void UIManager::showTextEntry(const char* currentName) {
    (void)currentName;
    currentState = UI_STATE_TEXT_ENTRY;
    stateEnteredAt = 0;
}

void UIManager::showSending() {
    currentState = UI_STATE_SENDING;
    stateEnteredAt = 0;
}

void UIManager::showSuccess() {
    currentState = UI_STATE_SUCCESS;
    stateEnteredAt = 0;
}

void UIManager::showError(const char* errorMessage) {
    (void)errorMessage;
    currentState = UI_STATE_ERROR;
    stateEnteredAt = 0;
}

UIState UIManager::getCurrentState() const {
    return currentState;
}

uint32_t UIManager::getStateEnteredTime() const {
    return stateEnteredAt;
}
