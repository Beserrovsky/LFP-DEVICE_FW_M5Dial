#pragma once

#include <stdint.h>
#include <stddef.h>

enum UIState {
    UI_STATE_SPLASH,
    UI_STATE_QUESTION,
    UI_STATE_TEXT_ENTRY,
    UI_STATE_SENDING,
    UI_STATE_SUCCESS,
    UI_STATE_ERROR
};

class UIManager {
public:
    UIManager();
    ~UIManager();

    bool init();
    void update();

    void showSplash();
    void showQuestion(uint8_t questionIndex, uint8_t optionCount,
                      const char* questionText, const char* currentOption);
    void showTextEntry(const char* currentName);
    void showSending();
    void showSuccess();
    void showError(const char* errorMessage);

    UIState getCurrentState() const;
    uint32_t getStateEnteredTime() const;

private:
    UIState currentState;
    uint32_t stateEnteredAt;
};

extern UIManager g_uiManager;
