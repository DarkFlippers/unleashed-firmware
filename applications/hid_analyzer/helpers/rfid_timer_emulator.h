#pragma once
#include <furi_hal.h>
#include "key_info.h"
#include "encoder_generic.h"
#include "encoder_emmarin.h"
#include "encoder_hid_h10301.h"
#include "encoder_indala_40134.h"
#include "pulse_joiner.h"
#include <map>

class RfidTimerEmulator {
public:
    RfidTimerEmulator();
    ~RfidTimerEmulator();
    void start(LfrfidKeyType type, const uint8_t* data, uint8_t data_size);
    void stop();

private:
    EncoderGeneric* current_encoder = nullptr;

    std::map<LfrfidKeyType, EncoderGeneric*> encoders = {
        {LfrfidKeyType::KeyEM4100, new EncoderEM()},
        {LfrfidKeyType::KeyH10301, new EncoderHID_H10301()},
        {LfrfidKeyType::KeyI40134, new EncoderIndala_40134()},
    };

    PulseJoiner pulse_joiner;
    static void timer_update_callback(void* ctx);
};
