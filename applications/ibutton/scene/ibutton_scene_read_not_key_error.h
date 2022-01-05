#pragma once
#include "ibutton_scene_generic.h"
#include <gui/modules/dialog_ex.h>

class iButtonSceneReadNotKeyError : public iButtonScene {
public:
    void on_enter(iButtonApp* app) final;
    bool on_event(iButtonApp* app, iButtonEvent* event) final;
    void on_exit(iButtonApp* app) final;

private:
    void dialog_ex_callback(DialogExResult result, void* context);
};