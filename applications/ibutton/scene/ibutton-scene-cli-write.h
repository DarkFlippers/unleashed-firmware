#pragma once
#include "ibutton-scene-generic.h"

class iButtonSceneCliWrite : public iButtonScene {
public:
    void on_enter(iButtonApp* app) final;
    bool on_event(iButtonApp* app, iButtonEvent* event) final;
    void on_exit(iButtonApp* app) final;

private:
    uint16_t timeout;
};