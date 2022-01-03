#pragma once
#include "ibutton-scene-generic.h"

class iButtonSceneInfo : public iButtonScene {
public:
    void on_enter(iButtonApp* app) final;
    bool on_event(iButtonApp* app, iButtonEvent* event) final;
    void on_exit(iButtonApp* app) final;

private:
    void widget_callback(GuiButtonType result, InputType type, void* context);
};