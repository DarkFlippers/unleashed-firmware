#pragma once
#include "../hid_analyzer_app.h"

class HIDAppSceneRead : public GenericScene<HIDApp> {
public:
    void on_enter(HIDApp* app, bool need_restore) final;
    bool on_event(HIDApp* app, HIDApp::Event* event) final;
    void on_exit(HIDApp* app) final;
};
