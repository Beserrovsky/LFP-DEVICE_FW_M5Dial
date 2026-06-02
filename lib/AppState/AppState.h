#pragma once

#include <stdint.h>
#include <stddef.h>

class AppState {
public:
    AppState();
    ~AppState();

    bool init();

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

    // Reset state
    void reset();

private:
    static const uint8_t MAX_QUESTIONS = 5;
    static const uint8_t MAX_NAME_LENGTH = 12;

    uint8_t currentQuestion;
    uint8_t currentOption;
    uint8_t savedOptions[MAX_QUESTIONS];
    char nameBuffer[MAX_NAME_LENGTH];
    uint8_t nameLength;
};

extern AppState g_appState;
