#pragma once

#include <stdint.h>
#include <stddef.h>

enum AppScreen {
    SCREEN_HOME,
    SCREEN_TUTORIAL,
    SCREEN_QUESTION,
    SCREEN_CONFIRM,
    SCREEN_WAIT_OR_CLICK
};

class AppState {
public:
    AppState();
    ~AppState();

    bool init();

    // Screen tracking
    AppScreen getCurrentScreen() const;
    void setCurrentScreen(AppScreen screen);

    // Tutorial
    uint8_t getTutorialStep() const;
    void setTutorialStep(uint8_t step);

    // Question tracking
    uint8_t getCurrentQuestion() const;
    void setCurrentQuestion(uint8_t index);

    // Option selection
    uint8_t getCurrentOption() const;
    void setCurrentOption(uint8_t index);
    void incrementOption(uint8_t maxValue = 10);
    void decrementOption();

    // Answer storage
    const uint8_t* getSavedOptions() const;
    uint8_t getSavedOption(uint8_t questionIndex) const;
    void setSavedOption(uint8_t questionIndex, uint8_t optionIndex);

    // Name entry
    const char* getNameBuffer() const;
    void setNameBuffer(const char* name);
    uint8_t getNameLength() const;
    void setNameLength(uint8_t len);

    // WaitOrClick context
    bool getWaitOrClickIsSuccess() const;
    void setWaitOrClickIsSuccess(bool success);
    const char* getWaitOrClickMessage() const;
    void setWaitOrClickMessage(const char* message);

    // Submission state
    bool getSubmissionPending() const;
    void setSubmissionPending(bool pending);
    bool getSubmissionSuccess() const;
    void setSubmissionSuccess(bool success);

    // Reset state
    void reset();

private:
    static const uint8_t MAX_QUESTIONS = 5;
    static const uint8_t MAX_NAME_LENGTH = 12;
    static const uint8_t MAX_ERROR_LENGTH = 50;

    AppScreen currentScreen;
    uint8_t tutorialStep;

    uint8_t currentQuestion;
    uint8_t currentOption;
    uint8_t savedOptions[MAX_QUESTIONS];
    char nameBuffer[MAX_NAME_LENGTH];
    uint8_t nameLength;

    bool waitOrClickIsSuccess;
    char waitOrClickMessage[MAX_ERROR_LENGTH];

    bool submissionPending;
    bool submissionSuccess;
};

extern AppState g_appState;
