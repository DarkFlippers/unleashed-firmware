#pragma once
#include "../lfrfid_app.h"

class LfRfidAppSceneSaveSuccess : public GenericScene<LfRfidApp> {
public:
    void on_enter(LfRfidApp* app, bool need_restore) final;
    bool on_event(LfRfidApp* app, LfRfidApp::Event* event) final;
    void on_exit(LfRfidApp* app) final;

private:
    static void timeout_callback(void* context);
};
