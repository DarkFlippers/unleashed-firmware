#pragma once
#include "ibutton-scene-generic.h"
#include "../helpers/key-emulator.h"

class iButtonSceneEmulate : public iButtonScene {
public:
    void on_enter(iButtonApp* app) final;
    bool on_event(iButtonApp* app, iButtonEvent* event) final;
    void on_exit(iButtonApp* app) final;
};