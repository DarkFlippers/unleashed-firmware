#include <furi.h>

#include "one_wire_host.h"
#include "one_wire_host_timing.h"

struct OneWireHost {
    const GpioPin* gpio_pin;
    unsigned char saved_rom[8]; /** < global search state */
    uint8_t last_discrepancy;
    uint8_t last_family_discrepancy;
    bool last_device_flag;
};

OneWireHost* onewire_host_alloc(const GpioPin* gpio_pin) {
    OneWireHost* host = malloc(sizeof(OneWireHost));
    host->gpio_pin = gpio_pin;
    onewire_host_reset_search(host);
    return host;
}

void onewire_host_free(OneWireHost* host) {
    onewire_host_stop(host);
    free(host);
}

bool onewire_host_reset(OneWireHost* host) {
    uint8_t r;
    uint8_t retries = 125;

    // wait until the gpio is high
    furi_hal_gpio_write(host->gpio_pin, true);
    do {
        if(--retries == 0) return 0;
        furi_delay_us(2);
    } while(!furi_hal_gpio_read(host->gpio_pin));

    // pre delay
    furi_delay_us(OWH_RESET_DELAY_PRE);

    // drive low
    furi_hal_gpio_write(host->gpio_pin, false);
    furi_delay_us(OWH_RESET_DRIVE);

    // release
    furi_hal_gpio_write(host->gpio_pin, true);
    furi_delay_us(OWH_RESET_RELEASE);

    // read and post delay
    r = !furi_hal_gpio_read(host->gpio_pin);
    furi_delay_us(OWH_RESET_DELAY_POST);

    return r;
}

bool onewire_host_read_bit(OneWireHost* host) {
    bool result;

    // drive low
    furi_hal_gpio_write(host->gpio_pin, false);
    furi_delay_us(OWH_READ_DRIVE);

    // release
    furi_hal_gpio_write(host->gpio_pin, true);
    furi_delay_us(OWH_READ_RELEASE);

    // read and post delay
    result = furi_hal_gpio_read(host->gpio_pin);
    furi_delay_us(OWH_READ_DELAY_POST);

    return result;
}

uint8_t onewire_host_read(OneWireHost* host) {
    uint8_t result = 0;

    for(uint8_t bitMask = 0x01; bitMask; bitMask <<= 1) {
        if(onewire_host_read_bit(host)) {
            result |= bitMask;
        }
    }

    return result;
}

void onewire_host_read_bytes(OneWireHost* host, uint8_t* buffer, uint16_t count) {
    for(uint16_t i = 0; i < count; i++) {
        buffer[i] = onewire_host_read(host);
    }
}

void onewire_host_write_bit(OneWireHost* host, bool value) {
    if(value) {
        // drive low
        furi_hal_gpio_write(host->gpio_pin, false);
        furi_delay_us(OWH_WRITE_1_DRIVE);

        // release
        furi_hal_gpio_write(host->gpio_pin, true);
        furi_delay_us(OWH_WRITE_1_RELEASE);
    } else {
        // drive low
        furi_hal_gpio_write(host->gpio_pin, false);
        furi_delay_us(OWH_WRITE_0_DRIVE);

        // release
        furi_hal_gpio_write(host->gpio_pin, true);
        furi_delay_us(OWH_WRITE_0_RELEASE);
    }
}

void onewire_host_write(OneWireHost* host, uint8_t value) {
    uint8_t bitMask;

    for(bitMask = 0x01; bitMask; bitMask <<= 1) {
        onewire_host_write_bit(host, (bitMask & value) ? 1 : 0);
    }
}

void onewire_host_write_bytes(OneWireHost* host, const uint8_t* buffer, uint16_t count) {
    for(uint16_t i = 0; i < count; ++i) {
        onewire_host_write(host, buffer[i]);
    }
}

void onewire_host_skip(OneWireHost* host) {
    onewire_host_write(host, 0xCC);
}

void onewire_host_start(OneWireHost* host) {
    furi_hal_gpio_write(host->gpio_pin, true);
    furi_hal_gpio_init(host->gpio_pin, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
}

void onewire_host_stop(OneWireHost* host) {
    furi_hal_gpio_write(host->gpio_pin, true);
    furi_hal_gpio_init(host->gpio_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void onewire_host_reset_search(OneWireHost* host) {
    host->last_discrepancy = 0;
    host->last_device_flag = false;
    host->last_family_discrepancy = 0;
    for(int i = 7;; i--) {
        host->saved_rom[i] = 0;
        if(i == 0) break;
    }
}

void onewire_host_target_search(OneWireHost* host, uint8_t family_code) {
    host->saved_rom[0] = family_code;
    for(uint8_t i = 1; i < 8; i++) host->saved_rom[i] = 0;
    host->last_discrepancy = 64;
    host->last_family_discrepancy = 0;
    host->last_device_flag = false;
}

uint8_t onewire_host_search(OneWireHost* host, uint8_t* new_addr, OneWireHostSearchMode mode) {
    uint8_t id_bit_number;
    uint8_t last_zero, rom_byte_number, search_result;
    uint8_t id_bit, cmp_id_bit;

    unsigned char rom_byte_mask, search_direction;

    // initialize for search
    id_bit_number = 1;
    last_zero = 0;
    rom_byte_number = 0;
    rom_byte_mask = 1;
    search_result = 0;

    // if the last call was not the last one
    if(!host->last_device_flag) {
        // 1-Wire reset
        if(!onewire_host_reset(host)) {
            // reset the search
            host->last_discrepancy = 0;
            host->last_device_flag = false;
            host->last_family_discrepancy = 0;
            return false;
        }

        // issue the search command
        switch(mode) {
        case OneWireHostSearchModeConditional:
            onewire_host_write(host, 0xEC);
            break;
        case OneWireHostSearchModeNormal:
            onewire_host_write(host, 0xF0);
            break;
        }

        // loop to do the search
        do {
            // read a bit and its complement
            id_bit = onewire_host_read_bit(host);
            cmp_id_bit = onewire_host_read_bit(host);

            // check for no devices on 1-wire
            if((id_bit == 1) && (cmp_id_bit == 1))
                break;
            else {
                // all devices coupled have 0 or 1
                if(id_bit != cmp_id_bit)
                    search_direction = id_bit; // bit write value for search
                else {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if(id_bit_number < host->last_discrepancy)
                        search_direction =
                            ((host->saved_rom[rom_byte_number] & rom_byte_mask) > 0);
                    else
                        // if equal to last pick 1, if not then pick 0
                        search_direction = (id_bit_number == host->last_discrepancy);

                    // if 0 was picked then record its position in LastZero
                    if(search_direction == 0) {
                        last_zero = id_bit_number;

                        // check for Last discrepancy in family
                        if(last_zero < 9) host->last_family_discrepancy = last_zero;
                    }
                }

                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                if(search_direction == 1)
                    host->saved_rom[rom_byte_number] |= rom_byte_mask;
                else
                    host->saved_rom[rom_byte_number] &= ~rom_byte_mask;

                // serial number search direction write bit
                onewire_host_write_bit(host, search_direction);

                // increment the byte counter id_bit_number
                // and shift the mask rom_byte_mask
                id_bit_number++;
                rom_byte_mask <<= 1;

                // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                if(rom_byte_mask == 0) {
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
            }
        } while(rom_byte_number < 8); // loop until through all ROM bytes 0-7

        // if the search was successful then
        if(!(id_bit_number < 65)) {
            // search successful so set last_Discrepancy, last_device_flag, search_result
            host->last_discrepancy = last_zero;

            // check for last device
            if(host->last_discrepancy == 0) host->last_device_flag = true;

            search_result = true;
        }
    }

    // if no device found then reset counters so next 'search' will be like a first
    if(!search_result || !host->saved_rom[0]) {
        host->last_discrepancy = 0;
        host->last_device_flag = false;
        host->last_family_discrepancy = 0;
        search_result = false;
    } else {
        for(int i = 0; i < 8; i++) new_addr[i] = host->saved_rom[i];
    }

    return search_result;
}
