#include <core/kernel.h>
#include <one_wire/one_wire_host.h>
#include <one_wire/maxim_crc.h>
#include "tm01x.h"

// Commands for TM01x
#define TM01X_CMD_WRITE_FLAG 0xC1
#define TM01X_CMD_WRITE_ROM  0xC5
#define TM01X_CMD_READ_ROM   0x33

#define TM01X_CMD_FINALIZE_CYFRAL  0xCA
#define TM01X_CMD_FINALIZE_METAKOM 0xCB

static void tm01x_write_byte(OneWireHost* host, uint8_t value) {
    for(uint8_t bitMask = 0x01; bitMask; bitMask <<= 1) {
        onewire_host_write_bit(host, (bool)(bitMask & value));
        furi_delay_us(5000); // 5ms pause after each bit
    }
}

// Helper function to read and verify written data
static bool tm01x_read_and_verify(OneWireHost* host, const uint8_t* data, size_t data_size) {
    bool success = false;

    if(onewire_host_reset(host)) {
        success = true;
        onewire_host_write(host, TM01X_CMD_READ_ROM);

        for(size_t i = 0; i < data_size; ++i) {
            if(data[i] != onewire_host_read(host)) {
                success = false;
                break;
            }
        }
    }

    return success;
}

bool tm01x_write_dallas(OneWireHost* host, const uint8_t* data, size_t data_size) {
    // Set TM01x specific timings
    onewire_host_set_timings_tm01x(host);

    // Write sequence
    onewire_host_reset(host);
    onewire_host_write(host, TM01X_CMD_WRITE_FLAG);
    onewire_host_write_bit(host, true);
    furi_delay_us(5000);

    onewire_host_reset(host);
    onewire_host_write(host, TM01X_CMD_WRITE_ROM);

    for(size_t i = 0; i < data_size; ++i) {
        tm01x_write_byte(host, data[i]);
    }

    return tm01x_read_and_verify(host, data, data_size);
}
