#pragma once
#include "../lfrfid_debug_app.h"

class LfRfidDebugAppSceneStart : public GenericScene<LfRfidDebugApp> {
public:
    void on_enter(LfRfidDebugApp* app, bool need_restore) final;
    bool on_event(LfRfidDebugApp* app, LfRfidDebugApp::Event* event) final;
    void on_exit(LfRfidDebugApp* app) final;

private:
    void submenu_callback(void* context, uint32_t index);
    uint32_t submenu_item_selected = 0;
};
