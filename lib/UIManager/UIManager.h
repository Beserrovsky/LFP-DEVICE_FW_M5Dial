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

    void showSplash();
    void showQuestion(uint8_t questionNum, uint8_t totalQuestions,
                      const char* text, const char* currentOption,
                      const char* prevOption, const char* nextOption);
    void showTextEntry(const char* currentName);
    void showSending();
    void showSuccess();
    void showError(const char* errorMessage);

    void setCurrentOption(int value);
    void setPreviousOption(int value);
    void setNextOption(int value);
    void updateOptionsDisplay(int currentValue);

    UIState getCurrentState() const;

private:
    UIState currentState;
};

extern UIManager g_uiManager;
