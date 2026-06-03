#include <lvgl.h>
#include "UIManager.h"
#include <ui.h>
#include <stdio.h>
#include <string.h>

UIManager g_uiManager;

UIManager::UIManager()
    : currentState(UI_STATE_HOME) {
}

UIManager::~UIManager() {
}

bool UIManager::init() {
    currentState = UI_STATE_HOME;
    ui_init();
    return true;
}

void UIManager::showHome() {
    currentState = UI_STATE_HOME;
    lv_disp_load_scr(ui_Home);
}

void UIManager::showQuestion(const char* type, const char* numeration,
                              const char* instruction, int progress,
                              const char* current, const char* prev, const char* next) {
    currentState = (strcmp(type, "Tutorial") == 0) ? UI_STATE_TUTORIAL : UI_STATE_QUESTION;

    lv_label_set_text(ui_lblQuestionsType, type);
    lv_label_set_text(ui_lblQuestionsNumeration, numeration);
    lv_label_set_text(ui_lblQuestionsInstruction, instruction);
    lv_arc_set_value(ui_arcQuestionsProgress, progress);
    lv_label_set_text(ui_lblQuestionsCurrentOption, current);
    lv_label_set_text(ui_lblQuestionsPrevOption, prev);
    lv_label_set_text(ui_lblQuestionsNextOption, next);

    lv_disp_load_scr(ui_Questions);
}

void UIManager::showConfirm() {
    currentState = UI_STATE_CONFIRM;
    lv_disp_load_scr(ui_Confirm);
}

void UIManager::showWaitOrClick(const char* message) {
    currentState = UI_STATE_WAIT_OR_CLICK;
    lv_label_set_text(ui_lblWaitOrClick, message);
    lv_disp_load_scr(ui_WaitOrClick);
}

UIState UIManager::getCurrentState() const {
    return currentState;
}
