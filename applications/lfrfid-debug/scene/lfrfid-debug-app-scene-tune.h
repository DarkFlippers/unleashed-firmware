#pragma once
#include "../lfrfid-debug-app.h"

class LfRfidDebugAppSceneTune : public GenericScene<LfRfidDebugApp> {
public:
    void on_enter(LfRfidDebugApp* app, bool need_restore) final;
    bool on_event(LfRfidDebugApp* app, LfRfidDebugApp::Event* event) final;
    void on_exit(LfRfidDebugApp* app) final;
};