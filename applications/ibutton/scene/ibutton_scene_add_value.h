#pragma once
#include "ibutton_scene_generic.h"
#include "../ibutton_key.h"

class iButtonSceneAddValue : public iButtonScene {
public:
    void on_enter(iButtonApp* app) final;
    bool on_event(iButtonApp* app, iButtonEvent* event) final;
    void on_exit(iButtonApp* app) final;

private:
    void byte_input_callback(void* context);
    uint8_t new_key_data[IBUTTON_KEY_DATA_SIZE] = {};
};