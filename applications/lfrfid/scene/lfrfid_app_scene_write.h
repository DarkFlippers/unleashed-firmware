#pragma once
#include "../lfrfid_app.h"

class LfRfidAppSceneWrite : public GenericScene<LfRfidApp> {
public:
    void on_enter(LfRfidApp* app, bool need_restore) final;
    bool on_event(LfRfidApp* app, LfRfidApp::Event* event) final;
    void on_exit(LfRfidApp* app) final;

private:
    string_t data_string;
    bool card_not_supported;
};
