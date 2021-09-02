#pragma once
#include "ibutton-scene-generic.h"
#include "../ibutton-key.h"

class iButtonSceneAddValue : public iButtonScene {
public:
    void on_enter(iButtonApp* app) final;
    bool on_event(iButtonApp* app, iButtonEvent* event) final;
    void on_exit(iButtonApp* app) final;

private:
    void byte_input_callback(void* context);
    uint8_t new_key_data[IBUTTON_KEY_DATA_SIZE] = {};
};