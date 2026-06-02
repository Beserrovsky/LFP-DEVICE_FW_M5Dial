// =============================================================
// ui_stub.cpp — implementação mínima de ui/ui.h para testes sem SquareLine
//
// Em vez de criar telas LVGL reais, este stub:
//   • Define todos os ponteiros de objetos como nullptr
//   • Sobrescreve as funções lv_label_set_text / lv_arc_set_value etc.
//     NÃO — em vez disso, deixa o sketch chamar essas funções normalmente
//     (LVGL ignora chamadas com obj == nullptr internamente desde 9.x)
//   • Usa M5Dial.Display para escrever texto diretamente no display,
//     contornando o LVGL completamente para feedback visual
//   • Imprime tudo no Serial para depuração
//
// COMO USAR:
//   No M5Dial_Survey.ino, substitua:
//       #include "ui/ui.h"
//   por:
//       #include "ui/ui.h"   ← mantém, pois ui.h agora aponta para este stub
//
//   E no topo do .ino, logo após os outros includes, adicione:
//       #define UI_STUB_MODE 1
//   Isso ativa o display direto via M5GFX nas funções showXxx().
// =============================================================

#include "ui.h"
#include <M5Dial.h>

// ── Definição dos ponteiros (todos nullptr — sem objetos reais) ───────────

lv_obj_t *ui_splashScreen     = nullptr;
lv_obj_t *ui_questionScreen   = nullptr;
lv_obj_t *ui_textEntryScreen  = nullptr;
lv_obj_t *ui_sendingScreen    = nullptr;
lv_obj_t *ui_successScreen    = nullptr;
lv_obj_t *ui_errorScreen      = nullptr;

lv_obj_t *ui_lblQuestionStep   = nullptr;
lv_obj_t *ui_lblQuestionText   = nullptr;
lv_obj_t *ui_lblCurrentOption  = nullptr;
lv_obj_t *ui_lblCurrentChar    = nullptr;
lv_obj_t *ui_lblNameValue      = nullptr;
lv_obj_t *ui_lblSuccessText    = nullptr;
lv_obj_t *ui_lblErrorText      = nullptr;
lv_obj_t *ui_lblPrevOption     = nullptr;
lv_obj_t *ui_lblNextOption     = nullptr;

lv_obj_t *ui_arcQuestionProgress = nullptr;

lv_obj_t *ui_imgFishPreview  = nullptr;
lv_obj_t *ui_imgFishPreview1 = nullptr;
lv_obj_t *ui_imgFishPreview2 = nullptr;
lv_obj_t *ui_imgFishPreview3 = nullptr;
lv_obj_t *ui_imgFishPreview4 = nullptr;
lv_obj_t *ui_imgFishPreview5 = nullptr;

// ── Image descriptors vazios ──────────────────────────────────────────────

const lv_image_dsc_t ui_img_tetra_png   = {};
const lv_image_dsc_t ui_img_betta_png   = {};
const lv_image_dsc_t ui_img_gourami_png = {};
const lv_image_dsc_t ui_img_shark_png   = {};
const lv_image_dsc_t ui_img_crab_png    = {};

// ── Helpers de display direto (M5GFX) ────────────────────────────────────

static void stubClear(uint32_t bgColor = 0x1a1a2e)
{
    M5Dial.Display.fillScreen(bgColor);
}

static void stubTitle(const char *line1, uint32_t color = TFT_WHITE)
{
    M5Dial.Display.setTextColor(color, M5Dial.Display.color565(0x1a, 0x1a, 0x2e));
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.setTextDatum(MC_DATUM);
    M5Dial.Display.drawString(line1, 120, 90);
}

static void stubBody(const char *line, int y, uint32_t color = TFT_LIGHTGREY)
{
    M5Dial.Display.setTextColor(color, M5Dial.Display.color565(0x1a, 0x1a, 0x2e));
    M5Dial.Display.setTextSize(1);
    M5Dial.Display.setTextDatum(MC_DATUM);
    M5Dial.Display.drawString(line, 120, y);
}

// ── ui_init ───────────────────────────────────────────────────────────────
//
// Chamado por initLvgl() no sketch. Em modo stub não criamos telas LVGL.
// Apenas imprime uma confirmação e mostra algo no display.

void ui_init()
{
    Serial.println("[UI STUB] ui_init() — sem SquareLine, modo de teste ativo");

    stubClear();
    stubTitle("STUB MODE", TFT_YELLOW);
    stubBody("ui_init OK", 130, TFT_GREEN);
    stubBody("Rotate or press to start", 155, TFT_LIGHTGREY);
}
