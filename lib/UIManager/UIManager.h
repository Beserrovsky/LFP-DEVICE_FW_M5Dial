#pragma once

#include <stdint.h>
#include <stddef.h>

enum UIState {
    UI_STATE_HOME,
    UI_STATE_TUTORIAL,
    UI_STATE_QUESTION,
    UI_STATE_CONFIRM,
    UI_STATE_WAIT_OR_CLICK
};

class UIManager {
public:
    UIManager();
    ~UIManager();

    bool init();

    void showHome();

    void showQuestion(const char* type, const char* numeration,
                      const char* instruction, int progress,
                      const char* current, const char* prev, const char* next);

    void showConfirm();

    void showWaitOrClick(const char* message);

    void setWaitOrClickProgress(int percent);

    void setHomeLabelText(const char* text);

    UIState getCurrentState() const;

private:
    UIState currentState;
};

extern UIManager g_uiManager;
