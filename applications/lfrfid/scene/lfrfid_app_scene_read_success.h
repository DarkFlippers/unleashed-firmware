#pragma once
#include "../lfrfid_app.h"

class LfRfidAppSceneReadSuccess : public GenericScene<LfRfidApp> {
public:
    void on_enter(LfRfidApp* app, bool need_restore) final;
    bool on_event(LfRfidApp* app, LfRfidApp::Event* event) final;
    void on_exit(LfRfidApp* app) final;

private:
    static void back_callback(void* context);
    static void more_callback(void* context);

    string_t string[3];
};
