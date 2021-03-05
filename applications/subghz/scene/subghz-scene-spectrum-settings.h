#pragma once
#include "subghz-scene-generic.h"

class SubghzSceneSpectrumSettings : public SubghzScene {
public:
    void on_enter(SubghzApp* app) final;
    bool on_event(SubghzApp* app, SubghzEvent* event) final;
    void on_exit(SubghzApp* app) final;

private:
    void ok_callback(void* context);
};