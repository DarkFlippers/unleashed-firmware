#pragma once
#include "lf-rfid-scene-generic.h"
#include "../helpers/key-info.h"

class LfrfidSceneReadNormal : public LfrfidScene {
public:
    void on_enter(LfrfidApp* app) final;
    bool on_event(LfrfidApp* app, LfrfidEvent* event) final;
    void on_exit(LfrfidApp* app) final;

private:
    uint32_t success_reads = 0;
    static const uint8_t data_size = LFRFID_KEY_SIZE;
    uint8_t last_data[data_size] = {0};
};