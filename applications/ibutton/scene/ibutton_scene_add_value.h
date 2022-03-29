#pragma once
#include "ibutton_scene_generic.h"
#include <one_wire/ibutton/ibutton_key.h>

class iButtonSceneAddValue : public iButtonScene {
public:
    void on_enter(iButtonApp* app) final;
    bool on_event(iButtonApp* app, iButtonEvent* event) final;
    void on_exit(iButtonApp* app) final;

private:
    uint8_t new_key_data[IBUTTON_KEY_DATA_SIZE] = {};
};