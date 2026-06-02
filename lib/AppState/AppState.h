#pragma once

#include <stdint.h>
#include <stddef.h>

class AppState {
public:
    AppState();
    ~AppState();

    bool init();
    void update();

    uint8_t getCurrentQuestion() const;
    void setCurrentQuestion(uint8_t index);

    uint8_t getCurrentOption() const;
    void setCurrentOption(uint8_t index);

    const uint8_t* getSavedOptions() const;
    void setSavedOption(uint8_t questionIndex, uint8_t optionIndex);

    const char* getNameBuffer() const;
    void setNameBuffer(const char* name);
    uint8_t getNameLength() const;

    bool loadFromNVS();
    bool saveToNVS();

    void reset();

private:
    uint8_t currentQuestion;
    uint8_t currentOption;
    uint8_t savedOptions[5];
    char nameBuffer[12];
    uint8_t nameLength;
};

extern AppState g_appState;
