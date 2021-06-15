#pragma once
#include "../scened-app.h"

class ScenedAppSceneStart : public GenericScene<ScenedApp> {
public:
    void on_enter(ScenedApp* app, bool need_restore) final;
    bool on_event(ScenedApp* app, ScenedApp::Event* event) final;
    void on_exit(ScenedApp* app) final;

private:
    void submenu_callback(void* context, uint32_t index);
    uint32_t submenu_item_selected = 0;
};