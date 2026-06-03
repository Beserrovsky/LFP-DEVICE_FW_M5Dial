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

    // ── Fix invisible labels ──────────────────────────────────────────────────
    // The generated screens use the default LVGL light theme, which sets text
    // color to near-black (#333). On black screen backgrounds this is invisible.
    // Labels with explicit gray/color in the generated .c are fine; these four
    // were left without any explicit text color.
    lv_color_t white = lv_color_white();
    lv_obj_set_style_text_color(ui_lblQuestionsCurrentOption, white, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblQuestionsInstruction,   white, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblConfirmInstruction,     white, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblConfirmSend,            white, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblWaitOrClick,            white, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ── Fix invisible transform-zoomed labels ─────────────────────────────────
    // LV_COLOR_SCREEN_TRANSP is 0, so LVGL cannot composite transformed widgets
    // through an alpha channel. Widgets with transform_zoom != 256 get rendered
    // into an opaque black layer that covers the underlying screen content,
    // making the text invisible. Reset zoom to 256 (= 1×, no scaling).
    lv_obj_set_style_transform_zoom(ui_lblQuestionsCurrentOption, 256, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_zoom(ui_lblConfirmSend,            256, LV_PART_MAIN | LV_STATE_DEFAULT);

    return true;
}

void UIManager::showHome() {
    currentState = UI_STATE_HOME;
    lv_disp_load_scr(ui_Home);
    lv_label_set_text(ui_lblHomeNFC, "Id with NFC\nor\nClick 5 times");
}

void UIManager::showQuestion(const char* type, const char* numeration,
                              const char* instruction, int progress,
                              const char* current, const char* prev, const char* next) {
    currentState = (strcmp(type, "Tutorial") == 0) ? UI_STATE_TUTORIAL : UI_STATE_QUESTION;

    lv_label_set_text(ui_lblQuestionsType, type);
    lv_label_set_text(ui_lblQuestionsNumeration, numeration);
    lv_label_set_text(ui_lblQuestionsInstruction, instruction);

    // Arc range is dynamic: 0-1 for 2-step Tutorial (so value=1 fills fully),
    // 0-5 for the 5-step Question flow.
    if (strcmp(type, "Tutorial") == 0) {
        lv_arc_set_range(ui_arcQuestionsProgress, 0, 1);
    } else {
        lv_arc_set_range(ui_arcQuestionsProgress, 0, 5);
    }
    lv_arc_set_value(ui_arcQuestionsProgress, progress);

    lv_label_set_text(ui_lblQuestionsCurrentOption, current);
    lv_label_set_text(ui_lblQuestionsPrevOption, prev);
    lv_label_set_text(ui_lblQuestionsNextOption, next);

    if (lv_scr_act() != ui_Questions) {
        lv_disp_load_scr(ui_Questions);
    }
}

void UIManager::showConfirm() {
    currentState = UI_STATE_CONFIRM;
    lv_disp_load_scr(ui_Confirm);
}

void UIManager::showWaitOrClick(const char* message) {
    currentState = UI_STATE_WAIT_OR_CLICK;
    lv_label_set_text(ui_lblWaitOrClick, message);
    lv_arc_set_value(ui_arcWaitOrClick, 0);
    lv_disp_load_scr(ui_WaitOrClick);
}

void UIManager::setWaitOrClickProgress(int percent) {
    if (percent < 0)   percent = 0;
    if (percent > 100) percent = 100;
    lv_arc_set_value(ui_arcWaitOrClick, percent);
}

void UIManager::setHomeLabelText(const char* text) {
    lv_label_set_text(ui_lblHomeNFC, text);
}

UIState UIManager::getCurrentState() const {
    return currentState;
}
