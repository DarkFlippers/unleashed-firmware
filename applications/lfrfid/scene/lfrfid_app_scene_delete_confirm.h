#pragma once
#include "../lfrfid_app.h"

class LfRfidAppSceneDeleteConfirm : public GenericScene<LfRfidApp> {
public:
    void on_enter(LfRfidApp* app, bool need_restore) final;
    bool on_event(LfRfidApp* app, LfRfidApp::Event* event) final;
    void on_exit(LfRfidApp* app) final;

private:
    static void back_callback(void* context);
    static void delete_callback(void* context);

    string_t string_header;
    string_t string_data;
    string_t string_decrypted;
};
