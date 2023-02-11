#include "one_wire_slave.h"
#include "one_wire_slave_i.h"
#include "one_wire_device.h"
#include <furi.h>
#include <furi_hal.h>

#define OWS_RESET_MIN 270
#define OWS_RESET_MAX 960
#define OWS_PRESENCE_TIMEOUT 20
#define OWS_PRESENCE_MIN 100
#define OWS_PRESENCE_MAX 480
#define OWS_MSG_HIGH_TIMEOUT 15000
#define OWS_SLOT_MAX 135
#define OWS_READ_MIN 20
#define OWS_READ_MAX 60
#define OWS_WRITE_ZERO 30

typedef enum {
    NO_ERROR = 0,
    VERY_LONG_RESET,
    VERY_SHORT_RESET,
    PRESENCE_LOW_ON_LINE,
    AWAIT_TIMESLOT_TIMEOUT_HIGH,
    INCORRECT_ONEWIRE_CMD,
    FIRST_BIT_OF_BYTE_TIMEOUT,
    RESET_IN_PROGRESS
} OneWireSlaveError;

struct OneWireSlave {
    const GpioPin* gpio_pin;
    OneWireSlaveError error;
    OneWireDevice* device;
    OneWireSlaveResultCallback result_cb;
    void* result_cb_ctx;
};

/*********************** PRIVATE ***********************/

static uint32_t
    onewire_slave_wait_while_gpio_is(OneWireSlave* bus, uint32_t time, const bool pin_value) {
    uint32_t start = DWT->CYCCNT;
    uint32_t time_ticks = time * furi_hal_cortex_instructions_per_microsecond();
    uint32_t time_captured;

    do { //-V1044
        time_captured = DWT->CYCCNT;
        if(furi_hal_gpio_read(bus->gpio_pin) != pin_value) {
            uint32_t remaining_time = time_ticks - (time_captured - start);
            remaining_time /= furi_hal_cortex_instructions_per_microsecond();
            return remaining_time;
        }
    } while((time_captured - start) < time_ticks);

    return 0;
}

static bool onewire_slave_show_presence(OneWireSlave* bus) {
    // wait while master delay presence check
    onewire_slave_wait_while_gpio_is(bus, OWS_PRESENCE_TIMEOUT, true);

    // show presence
    furi_hal_gpio_write(bus->gpio_pin, false);
    furi_delay_us(OWS_PRESENCE_MIN);
    furi_hal_gpio_write(bus->gpio_pin, true);

    // somebody also can show presence
    const uint32_t wait_low_time = OWS_PRESENCE_MAX - OWS_PRESENCE_MIN;

    // so we will wait
    if(onewire_slave_wait_while_gpio_is(bus, wait_low_time, false) == 0) {
        bus->error = PRESENCE_LOW_ON_LINE;
        return false;
    }

    return true;
}

static bool onewire_slave_receive_bit(OneWireSlave* bus) {
    // wait while bus is low
    uint32_t time = OWS_SLOT_MAX;
    time = onewire_slave_wait_while_gpio_is(bus, time, false);
    if(time == 0) {
        bus->error = RESET_IN_PROGRESS;
        return false;
    }

    // wait while bus is high
    time = OWS_MSG_HIGH_TIMEOUT;
    time = onewire_slave_wait_while_gpio_is(bus, time, true);
    if(time == 0) {
        bus->error = AWAIT_TIMESLOT_TIMEOUT_HIGH;
        return false;
    }

    // wait a time of zero
    time = OWS_READ_MIN;
    time = onewire_slave_wait_while_gpio_is(bus, time, false);

    return (time > 0);
}

static bool onewire_slave_send_bit(OneWireSlave* bus, bool value) {
    const bool write_zero = !value;

    // wait while bus is low
    uint32_t time = OWS_SLOT_MAX;
    time = onewire_slave_wait_while_gpio_is(bus, time, false);
    if(time == 0) {
        bus->error = RESET_IN_PROGRESS;
        return false;
    }

    // wait while bus is high
    time = OWS_MSG_HIGH_TIMEOUT;
    time = onewire_slave_wait_while_gpio_is(bus, time, true);
    if(time == 0) {
        bus->error = AWAIT_TIMESLOT_TIMEOUT_HIGH;
        return false;
    }

    // choose write time
    if(write_zero) {
        furi_hal_gpio_write(bus->gpio_pin, false);
        time = OWS_WRITE_ZERO;
    } else {
        time = OWS_READ_MAX;
    }

    // hold line for ZERO or ONE time
    furi_delay_us(time);
    furi_hal_gpio_write(bus->gpio_pin, true);

    return true;
}

static void onewire_slave_cmd_search_rom(OneWireSlave* bus) {
    const uint8_t key_bytes = 8;
    uint8_t* key = onewire_device_get_id_p(bus->device);

    for(uint8_t i = 0; i < key_bytes; i++) {
        uint8_t key_byte = key[i];

        for(uint8_t j = 0; j < 8; j++) {
            bool bit = (key_byte >> j) & 0x01;

            if(!onewire_slave_send_bit(bus, bit)) return;
            if(!onewire_slave_send_bit(bus, !bit)) return;

            onewire_slave_receive_bit(bus);
            if(bus->error != NO_ERROR) return;
        }
    }
}

static bool onewire_slave_receive_and_process_cmd(OneWireSlave* bus) {
    uint8_t cmd;
    onewire_slave_receive(bus, &cmd, 1);

    if(bus->error == RESET_IN_PROGRESS)
        return true;
    else if(bus->error != NO_ERROR)
        return false;

    switch(cmd) {
    case 0xF0:
        // SEARCH ROM
        onewire_slave_cmd_search_rom(bus);
        return true;

    case 0x0F:
    case 0x33:
        // READ ROM
        onewire_device_send_id(bus->device);
        return true;

    default: // Unknown command
        bus->error = INCORRECT_ONEWIRE_CMD;
        return false;
    }
}

static bool onewire_slave_bus_start(OneWireSlave* bus) {
    bool result = true;

    if(bus->device == NULL) {
        result = false;
    } else {
        FURI_CRITICAL_ENTER();
        furi_hal_gpio_init(bus->gpio_pin, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
        bus->error = NO_ERROR;

        if(onewire_slave_show_presence(bus)) {
            // TODO think about multiple command cycles
            onewire_slave_receive_and_process_cmd(bus);
            result = (bus->error == NO_ERROR || bus->error == INCORRECT_ONEWIRE_CMD);

        } else {
            result = false;
        }

        furi_hal_gpio_init(bus->gpio_pin, GpioModeInterruptRiseFall, GpioPullNo, GpioSpeedLow);
        FURI_CRITICAL_EXIT();
    }

    return result;
}

static void exti_cb(void* context) {
    OneWireSlave* bus = context;

    volatile bool input_state = furi_hal_gpio_read(bus->gpio_pin);
    static uint32_t pulse_start = 0;

    if(input_state) {
        uint32_t pulse_length =
            (DWT->CYCCNT - pulse_start) / furi_hal_cortex_instructions_per_microsecond();
        if(pulse_length >= OWS_RESET_MIN) {
            if(pulse_length <= OWS_RESET_MAX) {
                // reset cycle ok
                bool result = onewire_slave_bus_start(bus);
                if(result && bus->result_cb != NULL) {
                    bus->result_cb(bus->result_cb_ctx);
                }
            } else {
                bus->error = VERY_LONG_RESET;
            }
        } else {
            bus->error = VERY_SHORT_RESET;
        }
    } else {
        //FALL event
        pulse_start = DWT->CYCCNT;
    }
};

/*********************** PUBLIC ***********************/

OneWireSlave* onewire_slave_alloc(const GpioPin* gpio_pin) {
    OneWireSlave* bus = malloc(sizeof(OneWireSlave));
    bus->gpio_pin = gpio_pin;
    bus->error = NO_ERROR;
    bus->device = NULL;
    bus->result_cb = NULL;
    bus->result_cb_ctx = NULL;
    return bus;
}

void onewire_slave_free(OneWireSlave* bus) {
    onewire_slave_stop(bus);
    free(bus);
}

void onewire_slave_start(OneWireSlave* bus) {
    furi_hal_gpio_add_int_callback(bus->gpio_pin, exti_cb, bus);
    furi_hal_gpio_write(bus->gpio_pin, true);
    furi_hal_gpio_init(bus->gpio_pin, GpioModeInterruptRiseFall, GpioPullNo, GpioSpeedLow);
}

void onewire_slave_stop(OneWireSlave* bus) {
    furi_hal_gpio_write(bus->gpio_pin, true);
    furi_hal_gpio_init(bus->gpio_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_remove_int_callback(bus->gpio_pin);
}

void onewire_slave_attach(OneWireSlave* bus, OneWireDevice* device) {
    bus->device = device;
    onewire_device_attach(device, bus);
}

void onewire_slave_detach(OneWireSlave* bus) {
    if(bus->device != NULL) {
        onewire_device_detach(bus->device);
    }
    bus->device = NULL;
}

void onewire_slave_set_result_callback(
    OneWireSlave* bus,
    OneWireSlaveResultCallback result_cb,
    void* context) {
    bus->result_cb = result_cb;
    bus->result_cb_ctx = context;
}

bool onewire_slave_send(OneWireSlave* bus, const uint8_t* address, const uint8_t data_length) {
    uint8_t bytes_sent = 0;

    furi_hal_gpio_write(bus->gpio_pin, true);

    // bytes loop
    for(; bytes_sent < data_length; ++bytes_sent) {
        const uint8_t data_byte = address[bytes_sent];

        // bit loop
        for(uint8_t bit_mask = 0x01; bit_mask != 0; bit_mask <<= 1) {
            if(!onewire_slave_send_bit(bus, bit_mask & data_byte)) {
                // if we cannot send first bit
                if((bit_mask == 0x01) && (bus->error == AWAIT_TIMESLOT_TIMEOUT_HIGH))
                    bus->error = FIRST_BIT_OF_BYTE_TIMEOUT;
                return false;
            }
        }
    }
    return true;
}

bool onewire_slave_receive(OneWireSlave* bus, uint8_t* data, const uint8_t data_length) {
    uint8_t bytes_received = 0;

    furi_hal_gpio_write(bus->gpio_pin, true);

    for(; bytes_received < data_length; ++bytes_received) {
        uint8_t value = 0;

        for(uint8_t bit_mask = 0x01; bit_mask != 0; bit_mask <<= 1) {
            if(onewire_slave_receive_bit(bus)) value |= bit_mask;
        }

        data[bytes_received] = value;
    }
    return (bytes_received != data_length);
}
