#include <M5Unified.h>
#include <DisplayDriver.h>
#include <NFCManager.h>
#include <ESPNowManager.h>
#include <UIManager.h>
#include <AppState.h>

void setup() {
    g_displayDriver.init();
    g_nfcManager.init();
    g_espNowManager.init();
    g_uiManager.init();
    g_appState.init();
}

void loop() {
    g_displayDriver.update();
    g_nfcManager.update();
    g_espNowManager.update();
    g_uiManager.update();
    g_appState.update();
}