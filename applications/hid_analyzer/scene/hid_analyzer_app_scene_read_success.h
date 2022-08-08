#pragma once
#include "../hid_analyzer_app.h"

class HIDAppSceneReadSuccess : public GenericScene<HIDApp> {
public:
    void on_enter(HIDApp* app, bool need_restore) final;
    bool on_event(HIDApp* app, HIDApp::Event* event) final;
    void on_exit(HIDApp* app) final;

private:
    static void back_callback(void* context);

    string_t string[3];
};
