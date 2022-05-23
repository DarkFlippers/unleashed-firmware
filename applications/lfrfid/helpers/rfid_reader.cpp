#include "rfid_reader.h"
#include <furi.h>
#include <furi_hal.h>
#include <stm32wbxx_ll_cortex.h>

/**
 * @brief private violation assistant for RfidReader
 */
struct RfidReaderAccessor {
    static void decode(RfidReader& rfid_reader, bool polarity) {
        rfid_reader.decode(polarity);
    }
};

void RfidReader::decode(bool polarity) {
    uint32_t current_dwt_value = DWT->CYCCNT;
    uint32_t period = current_dwt_value - last_dwt_value;
    last_dwt_value = current_dwt_value;

#ifdef RFID_GPIO_DEBUG
    decoder_gpio_out.process_front(polarity, period);
#endif

    switch(type) {
    case Type::Normal:
        decoder_em.process_front(polarity, period);
        decoder_hid26.process_front(polarity, period);
        decoder_ioprox.process_front(polarity, period);
        break;
    case Type::Indala:
        decoder_em.process_front(polarity, period);
        decoder_hid26.process_front(polarity, period);
        decoder_ioprox.process_front(polarity, period);
        decoder_indala.process_front(polarity, period);
        break;
    }

    detect_ticks++;
}

bool RfidReader::switch_timer_elapsed() {
    const uint32_t seconds_to_switch = osKernelGetTickFreq() * 2.0f;
    return (osKernelGetTickCount() - switch_os_tick_last) > seconds_to_switch;
}

void RfidReader::switch_timer_reset() {
    switch_os_tick_last = osKernelGetTickCount();
}

void RfidReader::switch_mode() {
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
    RfidReader* _this = static_cast<RfidReader*>(comp_ctx);

    RfidReaderAccessor::decode(*_this, !level);
}

RfidReader::RfidReader() {
}

void RfidReader::start() {
    type = Type::Normal;

    furi_hal_rfid_pins_read();
    furi_hal_rfid_tim_read(125000, 0.5);
    furi_hal_rfid_tim_read_start();
    start_comparator();

    switch_timer_reset();
    last_read_count = 0;
}

void RfidReader::start_forced(RfidReader::Type _type) {
    start();
    if(_type == Type::Indala) {
        switch_mode();
    }
}

void RfidReader::stop() {
    furi_hal_rfid_pins_reset();
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_tim_reset();
    stop_comparator();
}

bool RfidReader::read(LfrfidKeyType* _type, uint8_t* data, uint8_t data_size, bool switch_enable) {
    bool result = false;
    bool something_read = false;

    // reading
    if(decoder_em.read(data, data_size)) {
        *_type = LfrfidKeyType::KeyEM4100;
        something_read = true;
    }

    if(decoder_hid26.read(data, data_size)) {
        *_type = LfrfidKeyType::KeyH10301;
        something_read = true;
    }

    if(decoder_ioprox.read(data, data_size)) {
        *_type = LfrfidKeyType::KeyIoProxXSF;
        something_read = true;
    }

    if(decoder_indala.read(data, data_size)) {
        *_type = LfrfidKeyType::KeyI40134;
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

bool RfidReader::detect() {
    bool detected = false;
    if(detect_ticks > 10) {
        detected = true;
    }
    detect_ticks = 0;

    return detected;
}

bool RfidReader::any_read() {
    return last_read_count > 0;
}

void RfidReader::start_comparator(void) {
    furi_hal_rfid_comp_set_callback(comparator_trigger_callback, this);
    last_dwt_value = DWT->CYCCNT;

    furi_hal_rfid_comp_start();
}

void RfidReader::stop_comparator(void) {
    furi_hal_rfid_comp_stop();
    furi_hal_rfid_comp_set_callback(NULL, NULL);
}
