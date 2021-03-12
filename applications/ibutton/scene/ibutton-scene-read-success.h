#pragma once
#include "ibutton-scene-generic.h"
#include <gui/modules/dialog_ex.h>

class iButtonSceneReadSuccess : public iButtonScene {
public:
    void on_enter(iButtonApp* app) final;
    bool on_event(iButtonApp* app, iButtonEvent* event) final;
    void on_exit(iButtonApp* app) final;

private:
    void dialog_ex_callback(DialogExResult result, void* context);
};