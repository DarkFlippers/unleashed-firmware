#pragma once
#include <api-hal.h>
#include "key-info.h"
#include "encoder-generic.h"
#include "encoder-emmarine.h"
#include "encoder-hid.h"
#include "encoder-indala.h"
#include "pulse-joiner.h"
#include <map>

class RfidTimerEmulator {
public:
    enum class Type : uint8_t {
        EM,
        HID,
        Indala,
    };

    RfidTimerEmulator();
    ~RfidTimerEmulator();
    void start(Type type);
    void stop();
    void emulate();

private:
    EncoderGeneric* current_encoder = nullptr;

    std::map<Type, EncoderGeneric*> encoders = {
        {Type::EM, new EncoderEM()},
        {Type::HID, new EncoderHID()},
        {Type::Indala, new EncoderIndala()},
    };

    PulseJoiner pulse_joiner;
    static void timer_update_callback(void* _hw, void* ctx);
};