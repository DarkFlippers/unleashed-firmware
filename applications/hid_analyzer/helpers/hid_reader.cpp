#include "hid_reader.h"
#include <furi.h>
#include <furi_hal.h>
#include <stm32wbxx_ll_cortex.h>

/**
 * @brief private violation assistant for HIDReader
 */
struct HIDReaderAccessor {
    static void decode(HIDReader& hid_reader, bool polarity) {
        hid_reader.decode(polarity);
    }
};

void HIDReader::decode(bool polarity) {
    uint32_t current_dwt_value = DWT->CYCCNT;
    uint32_t period = current_dwt_value - last_dwt_value;
    last_dwt_value = current_dwt_value;

    decoder_hid.process_front(polarity, period);

    detect_ticks++;
}

bool HIDReader::switch_timer_elapsed() {
    const uint32_t seconds_to_switch = furi_kernel_get_tick_frequency() * 2.0f;
    return (furi_get_tick() - switch_os_tick_last) > seconds_to_switch;
}

void HIDReader::switch_timer_reset() {
    switch_os_tick_last = furi_get_tick();
}

void HIDReader::switch_mode() {
    switch(type) {
    case Type::Normal:
        type = Type::Indala;
        furi_hal_rfid_change_read_config(62500.0f, 0.25f);
        break;
    case Type::Indala:
        type = Type::Normal;
        furi_hal_rfid_change_read_config(125000.0f, 0.5f);
        break;
    }

    switch_timer_reset();
}

static void comparator_trigger_callback(bool level, void* comp_ctx) {
    HIDReader* _this = static_cast<HIDReader*>(comp_ctx);

    HIDReaderAccessor::decode(*_this, !level);
}

HIDReader::HIDReader() {
}

void HIDReader::start() {
    type = Type::Normal;

    furi_hal_rfid_pins_read();
    furi_hal_rfid_tim_read(125000, 0.5);
    furi_hal_rfid_tim_read_start();
    start_comparator();

    switch_timer_reset();
    last_read_count = 0;
}

void HIDReader::start_forced(HIDReader::Type _type) {
    start();
    if(_type == Type::Indala) {
        switch_mode();
    }
}

void HIDReader::stop() {
    furi_hal_rfid_pins_reset();
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_tim_reset();
    stop_comparator();
}

bool HIDReader::read(LfrfidKeyType* _type, uint8_t* data, uint8_t data_size, bool switch_enable) {
    bool result = false;
    bool something_read = false;

    if(decoder_hid.read(data, data_size)) {
        *_type = LfrfidKeyType::KeyH10301; // should be an OK temp
        something_read = true;
    }

    // validation
    if(something_read) {
        switch_timer_reset();

        if(last_read_type == *_type && memcmp(last_read_data, data, data_size) == 0) {
            last_read_count = last_read_count + 1;

            if(last_read_count > 2) {
                result = true;
            }
        } else {
            last_read_type = *_type;
            memcpy(last_read_data, data, data_size);
            last_read_count = 0;
        }
    }

    // mode switching
    if(switch_enable && switch_timer_elapsed()) {
        switch_mode();
        last_read_count = 0;
    }

    return result;
}

bool HIDReader::detect() {
    bool detected = false;
    if(detect_ticks > 10) {
        detected = true;
    }
    detect_ticks = 0;

    return detected;
}

bool HIDReader::any_read() {
    return last_read_count > 0;
}

void HIDReader::start_comparator(void) {
    furi_hal_rfid_comp_set_callback(comparator_trigger_callback, this);
    last_dwt_value = DWT->CYCCNT;

    furi_hal_rfid_comp_start();
}

void HIDReader::stop_comparator(void) {
    furi_hal_rfid_comp_stop();
    furi_hal_rfid_comp_set_callback(NULL, NULL);
}
