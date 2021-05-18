#include "rfid-timer-emulator.h"

extern TIM_HandleTypeDef htim1;

RfidTimerEmulator::RfidTimerEmulator() {
}

RfidTimerEmulator::~RfidTimerEmulator() {
    std::map<Type, EncoderGeneric*>::iterator it;

    for(it = encoders.begin(); it != encoders.end(); ++it) {
        delete it->second;
        encoders.erase(it);
    }
}

void RfidTimerEmulator::start(Type type) {
    if(encoders.count(type)) {
        current_encoder = encoders.find(type)->second;
        uint8_t em_data[5] = {0x53, 0x00, 0x5F, 0xB3, 0xC2};
        uint8_t hid_data[3] = {0xED, 0x87, 0x70};
        uint8_t indala_data[3] = {0x1F, 0x2E, 0x3D};

        switch(type) {
        case Type::EM:
            current_encoder->init(em_data, 5);
            break;
        case Type::HID_H10301:
            current_encoder->init(hid_data, 3);
            break;
        case Type::Indala_40134:
            current_encoder->init(indala_data, 3);
            break;
        }

        api_hal_rfid_tim_emulate(125000);
        api_hal_rfid_pins_emulate();

        api_interrupt_add(timer_update_callback, InterruptTypeTimerUpdate, this);

        for(size_t i = WWDG_IRQn; i <= DMAMUX1_OVR_IRQn; i++) {
            HAL_NVIC_SetPriority(static_cast<IRQn_Type>(i), 15, 0);
        }

        HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

        api_hal_rfid_tim_emulate_start();
    } else {
        // not found
    }
}

void RfidTimerEmulator::stop() {
    api_hal_rfid_tim_emulate_stop();
    api_interrupt_remove(timer_update_callback, InterruptTypeTimerUpdate);

    api_hal_rfid_tim_reset();
    api_hal_rfid_pins_reset();
}

void RfidTimerEmulator::timer_update_callback(void* _hw, void* ctx) {
    RfidTimerEmulator* _this = static_cast<RfidTimerEmulator*>(ctx);
    TIM_HandleTypeDef* hw = static_cast<TIM_HandleTypeDef*>(_hw);

    if(api_hal_rfid_is_tim_emulate(hw)) {
        bool result;
        bool polarity;
        uint16_t period;
        uint16_t pulse;

        do {
            _this->current_encoder->get_next(&polarity, &period, &pulse);
            result = _this->pulse_joiner.push_pulse(polarity, period, pulse);
        } while(result == false);

        _this->pulse_joiner.pop_pulse(&period, &pulse);

        api_hal_rfid_set_emulate_period(period - 1);
        api_hal_rfid_set_emulate_pulse(pulse);
    }
}
