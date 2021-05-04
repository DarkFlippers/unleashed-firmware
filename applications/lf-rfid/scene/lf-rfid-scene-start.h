#pragma once
#include "lf-rfid-scene-generic.h"
#include "../helpers/rfid-timer-emulator.h"

class LfrfidSceneStart : public LfrfidScene {
public:
    void on_enter(LfrfidApp* app) final;
    bool on_event(LfrfidApp* app, LfrfidEvent* event) final;
    void on_exit(LfrfidApp* app) final;

private:
    void submenu_callback(void* context, uint32_t index);
};