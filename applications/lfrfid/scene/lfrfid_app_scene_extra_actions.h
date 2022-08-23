#pragma once
#include "../lfrfid_app.h"

class LfRfidAppSceneExtraActions : public GenericScene<LfRfidApp> {
public:
    void on_enter(LfRfidApp* app, bool need_restore) final;
    bool on_event(LfRfidApp* app, LfRfidApp::Event* event) final;
    void on_exit(LfRfidApp* app) final;

private:
    static void submenu_callback(void* context, uint32_t index);
    uint32_t submenu_item_selected = 0;
};
