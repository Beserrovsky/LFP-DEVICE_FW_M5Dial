#include "AppState.h"
#include <string.h>

AppState g_appState;

AppState::AppState()
    : currentQuestion(0), currentOption(0), nameLength(0) {
    for (int i = 0; i < 5; i++) {
        savedOptions[i] = 0;
    }
    nameBuffer[0] = 0;
}

AppState::~AppState() {
}

bool AppState::init() {
    return true;
}

void AppState::update() {
}

uint8_t AppState::getCurrentQuestion() const {
    return currentQuestion;
}

void AppState::setCurrentQuestion(uint8_t index) {
    currentQuestion = index;
}

uint8_t AppState::getCurrentOption() const {
    return currentOption;
}

void AppState::setCurrentOption(uint8_t index) {
    currentOption = index;
}

const uint8_t* AppState::getSavedOptions() const {
    return savedOptions;
}

void AppState::setSavedOption(uint8_t questionIndex, uint8_t optionIndex) {
    if (questionIndex < 5) {
        savedOptions[questionIndex] = optionIndex;
    }
}

const char* AppState::getNameBuffer() const {
    return nameBuffer;
}

void AppState::setNameBuffer(const char* name) {
    if (name) {
        strncpy(nameBuffer, name, sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = 0;
        nameLength = strlen(nameBuffer);
    }
}

uint8_t AppState::getNameLength() const {
    return nameLength;
}

bool AppState::loadFromNVS() {
    return true;
}

bool AppState::saveToNVS() {
    return true;
}

void AppState::reset() {
    currentQuestion = 0;
    currentOption = 0;
    for (int i = 0; i < 5; i++) {
        savedOptions[i] = 0;
    }
    nameBuffer[0] = 0;
    nameLength = 0;
}
