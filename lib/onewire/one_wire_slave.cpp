#include "one_wire_slave.h"
#include "callback-connector.h"
#include "main.h"
#include "one_wire_device.h"

#define OWET OneWireEmulateTiming

void OneWireSlave::start(void) {
    // add exti interrupt
    hal_gpio_add_int_callback(one_wire_pin_record, exti_cb, this);

    // init gpio
    hal_gpio_init(one_wire_pin_record, GpioModeInterruptRiseFall, GpioPullNo, GpioSpeedLow);
    pin_set_float();

    // init instructions per us count
    __instructions_per_us = (SystemCoreClock / 1000000.0f);
}

void OneWireSlave::stop(void) {
    // deinit gpio
    hal_gpio_init(one_wire_pin_record, GpioModeInput, GpioPullNo, GpioSpeedLow);
    // remove exti interrupt
    hal_gpio_remove_int_callback(one_wire_pin_record);

    // deattach devices
    deattach();
}

OneWireSlave::OneWireSlave(const GpioPin* pin) {
    one_wire_pin_record = pin;
    exti_cb = cbc::obtain_connector(this, &OneWireSlave::exti_callback);
}

OneWireSlave::~OneWireSlave() {
    stop();
}

void OneWireSlave::attach(OneWireDevice* attached_device) {
    device = attached_device;
    device->attach(this);
}

void OneWireSlave::deattach(void) {
    if(device != nullptr) {
        device->deattach();
    }
    device = nullptr;
}

void OneWireSlave::set_result_callback(OneWireSlaveResultCallback result_cb, void* ctx) {
    this->result_cb = result_cb;
    this->result_cb_ctx = ctx;
}

void OneWireSlave::pin_set_float() {
    hal_gpio_write(one_wire_pin_record, true);
}

void OneWireSlave::pin_set_low() {
    hal_gpio_write(one_wire_pin_record, false);
}

void OneWireSlave::pin_init_interrupt_in_isr_ctx(void) {
    hal_gpio_init(one_wire_pin_record, GpioModeInterruptRiseFall, GpioPullNo, GpioSpeedLow);
    __HAL_GPIO_EXTI_CLEAR_IT(one_wire_pin_record->pin);
}

void OneWireSlave::pin_init_opendrain_in_isr_ctx(void) {
    hal_gpio_init(one_wire_pin_record, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
    __HAL_GPIO_EXTI_CLEAR_IT(one_wire_pin_record->pin);
}

OneWiteTimeType OneWireSlave::wait_while_gpio_is(OneWiteTimeType time, const bool pin_value) {
    uint32_t start = DWT->CYCCNT;
    uint32_t time_ticks = time * __instructions_per_us;
    uint32_t time_captured;

    do {
        time_captured = DWT->CYCCNT;
        if(hal_gpio_read(one_wire_pin_record) != pin_value) {
            OneWiteTimeType remaining_time = time_ticks - (time_captured - start);
            remaining_time /= __instructions_per_us;
            return remaining_time;
        }
    } while((time_captured - start) < time_ticks);

    return 0;
}

bool OneWireSlave::show_presence(void) {
    // wait while master delay presence check
    wait_while_gpio_is(OWET::PRESENCE_TIMEOUT, true);

    // show presence
    pin_set_low();
    delay_us(OWET::PRESENCE_MIN);
    pin_set_float();

    // somebody also can show presence
    const OneWiteTimeType wait_low_time = OWET::PRESENCE_MAX - OWET::PRESENCE_MIN;

    // so we will wait
    if(wait_while_gpio_is(wait_low_time, false) == 0) {
        error = OneWireSlaveError::PRESENCE_LOW_ON_LINE;
        return false;
    }

    return true;
}

bool OneWireSlave::receive_bit(void) {
    // wait while bus is low
    OneWiteTimeType time = OWET::SLOT_MAX;
    time = wait_while_gpio_is(time, false);
    if(time == 0) {
        error = OneWireSlaveError::RESET_IN_PROGRESS;
        return false;
    }

    // wait while bus is high
    time = OWET::MSG_HIGH_TIMEOUT;
    time = wait_while_gpio_is(time, true);
    if(time == 0) {
        error = OneWireSlaveError::AWAIT_TIMESLOT_TIMEOUT_HIGH;
        return false;
    }

    // wait a time of zero
    time = OWET::READ_MIN;
    time = wait_while_gpio_is(time, false);

    return (time > 0);
}

bool OneWireSlave::send_bit(bool value) {
    const bool write_zero = !value;

    // wait while bus is low
    OneWiteTimeType time = OWET::SLOT_MAX;
    time = wait_while_gpio_is(time, false);
    if(time == 0) {
        error = OneWireSlaveError::RESET_IN_PROGRESS;
        return false;
    }

    // wait while bus is high
    time = OWET::MSG_HIGH_TIMEOUT;
    time = wait_while_gpio_is(time, true);
    if(time == 0) {
        error = OneWireSlaveError::AWAIT_TIMESLOT_TIMEOUT_HIGH;
        return false;
    }

    // choose write time
    if(write_zero) {
        pin_set_low();
        time = OWET::WRITE_ZERO;
    } else {
        time = OWET::READ_MAX;
    }

    // hold line for ZERO or ONE time
    delay_us(time);
    pin_set_float();

    return true;
}

bool OneWireSlave::send(const uint8_t* address, const uint8_t data_length) {
    uint8_t bytes_sent = 0;

    pin_set_float();

    // bytes loop
    for(; bytes_sent < data_length; ++bytes_sent) {
        const uint8_t data_byte = address[bytes_sent];

        // bit loop
        for(uint8_t bit_mask = 0x01; bit_mask != 0; bit_mask <<= 1) {
            if(!send_bit(static_cast<bool>(bit_mask & data_byte))) {
                // if we cannot send first bit
                if((bit_mask == 0x01) && (error == OneWireSlaveError::AWAIT_TIMESLOT_TIMEOUT_HIGH))
                    error = OneWireSlaveError::FIRST_BIT_OF_BYTE_TIMEOUT;
                return false;
            }
        }
    }
    return true;
}

bool OneWireSlave::receive(uint8_t* data, const uint8_t data_length) {
    uint8_t bytes_received = 0;

    pin_set_float();

    for(; bytes_received < data_length; ++bytes_received) {
        uint8_t value = 0;

        for(uint8_t bit_mask = 0x01; bit_mask != 0; bit_mask <<= 1) {
            if(receive_bit()) value |= bit_mask;
        }

        data[bytes_received] = value;
    }
    return (bytes_received != data_length);
}

void OneWireSlave::cmd_search_rom(void) {
    const uint8_t key_bytes = 8;
    uint8_t* key = device->id_storage;

    for(uint8_t i = 0; i < key_bytes; i++) {
        uint8_t key_byte = key[i];

        for(uint8_t j = 0; j < 8; j++) {
            bool bit = (key_byte >> j) & 0x01;

            if(!send_bit(bit)) return;
            if(!send_bit(!bit)) return;

            receive_bit();
            if(error != OneWireSlaveError::NO_ERROR) return;
        }
    }
}

bool OneWireSlave::receive_and_process_cmd(void) {
    uint8_t cmd;
    receive(&cmd, 1);

    if(error == OneWireSlaveError::RESET_IN_PROGRESS) return true;
    if(error != OneWireSlaveError::NO_ERROR) return false;

    switch(cmd) {
    case 0xF0:
        // SEARCH ROM
        cmd_search_rom();
        return true;

    case 0x0F:
    case 0x33:
        // READ ROM
        device->send_id();
        return true;

    default: // Unknown command
        error = OneWireSlaveError::INCORRECT_ONEWIRE_CMD;
    }

    if(error == OneWireSlaveError::RESET_IN_PROGRESS) return true;
    return (error == OneWireSlaveError::NO_ERROR);
}

bool OneWireSlave::bus_start(void) {
    bool result = true;

    if(device == nullptr) {
        result = false;
    } else {
        FURI_CRITICAL_ENTER();
        pin_init_opendrain_in_isr_ctx();
        error = OneWireSlaveError::NO_ERROR;

        if(show_presence()) {
            // TODO think about multiple command cycles
            receive_and_process_cmd();
            result =
                (error == OneWireSlaveError::NO_ERROR ||
                 error == OneWireSlaveError::INCORRECT_ONEWIRE_CMD);

        } else {
            result = false;
        }

        pin_init_interrupt_in_isr_ctx();
        FURI_CRITICAL_EXIT();
    }

    return result;
}

void OneWireSlave::exti_callback(void* _ctx) {
    OneWireSlave* _this = static_cast<OneWireSlave*>(_ctx);

    volatile bool input_state = hal_gpio_read(_this->one_wire_pin_record);
    static uint32_t pulse_start = 0;

    if(input_state) {
        uint32_t pulse_length = (DWT->CYCCNT - pulse_start) / __instructions_per_us;
        if(pulse_length >= OWET::RESET_MIN) {
            if(pulse_length <= OWET::RESET_MAX) {
                // reset cycle ok
                bool result = _this->bus_start();
                if(result && _this->result_cb != nullptr) {
                    _this->result_cb(result, _this->result_cb_ctx);
                }
            } else {
                error = OneWireSlaveError::VERY_LONG_RESET;
            }
        } else {
            error = OneWireSlaveError::VERY_SHORT_RESET;
        }
    } else {
        //FALL event
        pulse_start = DWT->CYCCNT;
    }
}
