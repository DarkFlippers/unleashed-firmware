#pragma once
#include "ibutton-scene-generic.h"

class iButtonSceneSaveName : public iButtonScene {
public:
    void on_enter(iButtonApp* app) final;
    bool on_event(iButtonApp* app, iButtonEvent* event) final;
    void on_exit(iButtonApp* app) final;

private:
    void text_input_callback(void* context, char* text);
};