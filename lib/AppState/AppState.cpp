#include "AppState.h"
#include <string.h>

AppState g_appState;

AppState::AppState()
    : currentQuestion(0), currentOption(0), nameLength(0) {
    for (int i = 0; i < MAX_QUESTIONS; i++) {
        savedOptions[i] = 0;
    }
    nameBuffer[0] = 0;
}

AppState::~AppState() {
}

bool AppState::init() {
    reset();
    return true;
}

uint8_t AppState::getCurrentQuestion() const {
    return currentQuestion;
}

void AppState::setCurrentQuestion(uint8_t index) {
    if (index < MAX_QUESTIONS) {
        currentQuestion = index;
    }
}

uint8_t AppState::getCurrentOption() const {
    return currentOption;
}

void AppState::setCurrentOption(uint8_t index) {
    currentOption = index;
}

void AppState::incrementOption(uint8_t maxValue) {
    if (currentOption < maxValue) {
        currentOption++;
    }
}

void AppState::decrementOption() {
    if (currentOption > 0) {
        currentOption--;
    }
}

const uint8_t* AppState::getSavedOptions() const {
    return savedOptions;
}

uint8_t AppState::getSavedOption(uint8_t questionIndex) const {
    if (questionIndex < MAX_QUESTIONS) {
        return savedOptions[questionIndex];
    }
    return 0;
}

void AppState::setSavedOption(uint8_t questionIndex, uint8_t optionIndex) {
    if (questionIndex < MAX_QUESTIONS) {
        savedOptions[questionIndex] = optionIndex;
    }
}

const char* AppState::getNameBuffer() const {
    return nameBuffer;
}

void AppState::setNameBuffer(const char* name) {
    if (name) {
        strncpy(nameBuffer, name, MAX_NAME_LENGTH - 1);
        nameBuffer[MAX_NAME_LENGTH - 1] = 0;
        nameLength = strlen(nameBuffer);
    } else {
        nameBuffer[0] = 0;
        nameLength = 0;
    }
}

uint8_t AppState::getNameLength() const {
    return nameLength;
}

void AppState::setNameLength(uint8_t len) {
    if (len < MAX_NAME_LENGTH) {
        nameLength = len;
        if (nameLength < strlen(nameBuffer)) {
            nameBuffer[nameLength] = 0;
        }
    }
}

void AppState::reset() {
    currentQuestion = 0;
    currentOption = 0;
    for (int i = 0; i < MAX_QUESTIONS; i++) {
        savedOptions[i] = 0;
    }
    nameBuffer[0] = 0;
    nameLength = 0;
}
