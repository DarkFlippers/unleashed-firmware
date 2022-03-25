#include "rfid_timer_emulator.h"

RfidTimerEmulator::RfidTimerEmulator() {
}

RfidTimerEmulator::~RfidTimerEmulator() {
    std::map<LfrfidKeyType, EncoderGeneric*>::iterator it;

    for(it = encoders.begin(); it != encoders.end(); ++it) {
        delete it->second;
    }

    encoders.clear();
}

void RfidTimerEmulator::start(LfrfidKeyType type, const uint8_t* data, uint8_t data_size) {
    if(encoders.count(type)) {
        current_encoder = encoders.find(type)->second;

        if(data_size >= lfrfid_key_get_type_data_count(type)) {
            current_encoder->init(data, data_size);

            furi_hal_rfid_tim_emulate(125000);
            furi_hal_rfid_pins_emulate();

            furi_hal_rfid_tim_emulate_start(RfidTimerEmulator::timer_update_callback, this);
        }
    } else {
        // not found
    }
}

void RfidTimerEmulator::stop() {
    furi_hal_rfid_tim_emulate_stop();
    furi_hal_rfid_tim_reset();
    furi_hal_rfid_pins_reset();
}

void RfidTimerEmulator::timer_update_callback(void* ctx) {
    RfidTimerEmulator* _this = static_cast<RfidTimerEmulator*>(ctx);

    bool result;
    bool polarity;
    uint16_t period;
    uint16_t pulse;

    do {
        _this->current_encoder->get_next(&polarity, &period, &pulse);
        result = _this->pulse_joiner.push_pulse(polarity, period, pulse);
    } while(result == false);

    _this->pulse_joiner.pop_pulse(&period, &pulse);

    furi_hal_rfid_set_emulate_period(period - 1);
    furi_hal_rfid_set_emulate_pulse(pulse);
}
