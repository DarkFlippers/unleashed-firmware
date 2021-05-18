#pragma once
#include "lf-rfid-scene-generic.h"
#include "../helpers/key-info.h"
#include "../helpers/rfid-writer.h"

class LfrfidSceneWrite : public LfrfidScene {
public:
    void on_enter(LfrfidApp* app) final;
    bool on_event(LfrfidApp* app, LfrfidEvent* event) final;
    void on_exit(LfrfidApp* app) final;

private:
    uint8_t timing_index;
};