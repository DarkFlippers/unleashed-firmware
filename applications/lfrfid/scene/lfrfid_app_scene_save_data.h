#pragma once
#include "../lfrfid_app.h"

class LfRfidAppSceneSaveData : public GenericScene<LfRfidApp> {
public:
    void on_enter(LfRfidApp* app, bool need_restore) final;
    bool on_event(LfRfidApp* app, LfRfidApp::Event* event) final;
    void on_exit(LfRfidApp* app) final;

private:
    static void save_callback(void* context);
    uint8_t old_key_data[LFRFID_KEY_SIZE] = {
        0xAA,
        0xAA,
        0xAA,
        0xAA,
        0xAA,
        0xAA,
        0xAA,
        0xAA,
    };

    uint8_t new_key_data[LFRFID_KEY_SIZE] = {
        0xBB,
        0xBB,
        0xBB,
        0xBB,
        0xBB,
        0xBB,
        0xBB,
        0xBB,
    };
};
