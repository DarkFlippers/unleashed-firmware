#pragma once
#include "subghz-scene-generic.h"

class SubghzSceneStart : public SubghzScene {
public:
    void on_enter(SubghzApp* app) final;
    bool on_event(SubghzApp* app, SubghzEvent* event) final;
    void on_exit(SubghzApp* app) final;

private:
    void submenu_callback(void* context, uint32_t index);
};