#pragma once
// =============================================================
// ui_stub.h — substituto mínimo de ui/ui.h para testes sem SquareLine Studio
//
// Declara todos os símbolos que main.cpp referencia.
// Nenhuma tela real é criada; o display mostra texto simples via M5GFX.
// =============================================================

#include <lvgl.h>

// ── Objetos LVGL referenciados no sketch ─────────────────────
// Todos declarados como ponteiros nulos — as funções stub ignoram
// chamadas a objetos nulos de forma segura.

extern lv_obj_t *ui_splashScreen;
extern lv_obj_t *ui_questionScreen;
extern lv_obj_t *ui_textEntryScreen;
extern lv_obj_t *ui_sendingScreen;
extern lv_obj_t *ui_successScreen;
extern lv_obj_t *ui_errorScreen;

extern lv_obj_t *ui_lblQuestionStep;
extern lv_obj_t *ui_lblQuestionText;
extern lv_obj_t *ui_lblCurrentOption;
extern lv_obj_t *ui_lblCurrentChar;
extern lv_obj_t *ui_lblNameValue;
extern lv_obj_t *ui_lblSuccessText;
extern lv_obj_t *ui_lblErrorText;
extern lv_obj_t *ui_lblPrevOption;
extern lv_obj_t *ui_lblNextOption;

extern lv_obj_t *ui_arcQuestionProgress;

extern lv_obj_t *ui_imgFishPreview;
extern lv_obj_t *ui_imgFishPreview1;
extern lv_obj_t *ui_imgFishPreview2;
extern lv_obj_t *ui_imgFishPreview3;
extern lv_obj_t *ui_imgFishPreview4;
extern lv_obj_t *ui_imgFishPreview5;

// ── Image descriptors (fish previews) ────────────────────────
// Declaradas como estruturas vazias — lv_image_set_src() não é
// chamada em modo stub (os objetos são nullptr).
extern const lv_image_dsc_t ui_img_tetra_png;
extern const lv_image_dsc_t ui_img_betta_png;
extern const lv_image_dsc_t ui_img_gourami_png;
extern const lv_image_dsc_t ui_img_shark_png;
extern const lv_image_dsc_t ui_img_crab_png;

// ── Inicialização ─────────────────────────────────────────────
void ui_init();
