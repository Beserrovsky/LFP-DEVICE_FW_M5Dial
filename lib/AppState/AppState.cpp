#include "AppState.h"
#include <string.h>

AppState g_appState;

AppState::AppState()
    : currentScreen(SCREEN_HOME), tutorialStep(0),
      currentQuestion(0), currentOption(0), nameLength(0),
      waitOrClickIsSuccess(false), submissionPending(false), submissionSuccess(false) {
    for (int i = 0; i < MAX_QUESTIONS; i++) {
        savedOptions[i] = 0;
    }
    nameBuffer[0] = 0;
    waitOrClickMessage[0] = 0;
}

AppState::~AppState() {
}

bool AppState::init() {
    reset();
    return true;
}

AppScreen AppState::getCurrentScreen() const {
    return currentScreen;
}

void AppState::setCurrentScreen(AppScreen screen) {
    currentScreen = screen;
}

uint8_t AppState::getTutorialStep() const {
    return tutorialStep;
}

void AppState::setTutorialStep(uint8_t step) {
    tutorialStep = step;
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

bool AppState::getWaitOrClickIsSuccess() const {
    return waitOrClickIsSuccess;
}

void AppState::setWaitOrClickIsSuccess(bool success) {
    waitOrClickIsSuccess = success;
}

const char* AppState::getWaitOrClickMessage() const {
    return waitOrClickMessage;
}

void AppState::setWaitOrClickMessage(const char* message) {
    if (message) {
        strncpy(waitOrClickMessage, message, MAX_ERROR_LENGTH - 1);
        waitOrClickMessage[MAX_ERROR_LENGTH - 1] = 0;
    } else {
        waitOrClickMessage[0] = 0;
    }
}

bool AppState::getSubmissionPending() const {
    return submissionPending;
}

void AppState::setSubmissionPending(bool pending) {
    submissionPending = pending;
}

bool AppState::getSubmissionSuccess() const {
    return submissionSuccess;
}

void AppState::setSubmissionSuccess(bool success) {
    submissionSuccess = success;
}

void AppState::reset() {
    currentScreen = SCREEN_HOME;
    tutorialStep = 0;
    currentQuestion = 0;
    currentOption = 0;
    for (int i = 0; i < MAX_QUESTIONS; i++) {
        savedOptions[i] = 0;
    }
    nameBuffer[0] = 0;
    nameLength = 0;
    waitOrClickIsSuccess = false;
    waitOrClickMessage[0] = 0;
    submissionPending = false;
    submissionSuccess = false;
}
