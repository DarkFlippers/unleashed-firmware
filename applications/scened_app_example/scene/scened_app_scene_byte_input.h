#pragma once
#include "../scened_app.h"

class ScenedAppSceneByteInput : public GenericScene<ScenedApp> {
public:
    void on_enter(ScenedApp* app, bool need_restore) final;
    bool on_event(ScenedApp* app, ScenedApp::Event* event) final;
    void on_exit(ScenedApp* app) final;

private:
    void result_callback(void* context);

    uint8_t data[4] = {
        0x01,
        0xA2,
        0xF4,
        0xD3,
    };
};