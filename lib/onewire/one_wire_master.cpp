#include "one_wire_master.h"
#include "one_wire_timings.h"

OneWireMaster::OneWireMaster(const GpioPin* one_wire_gpio) {
    gpio = one_wire_gpio;
    reset_search();
}

OneWireMaster::~OneWireMaster() {
    stop();
}

void OneWireMaster::start(void) {
    gpio_init(gpio, GpioModeOutputOpenDrain);
}

void OneWireMaster::stop(void) {
    gpio_init(gpio, GpioModeAnalog);
}

void OneWireMaster::reset_search() {
    // reset the search state
    last_discrepancy = 0;
    last_device_flag = false;
    last_family_discrepancy = 0;
    for(int i = 7;; i--) {
        saved_rom[i] = 0;
        if(i == 0) break;
    }
}

void OneWireMaster::target_search(uint8_t family_code) {
    // set the search state to find SearchFamily type devices
    saved_rom[0] = family_code;
    for(uint8_t i = 1; i < 8; i++) saved_rom[i] = 0;
    last_discrepancy = 64;
    last_family_discrepancy = 0;
    last_device_flag = false;
}

uint8_t OneWireMaster::search(uint8_t* newAddr, bool search_mode) {
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
    if(!last_device_flag) {
        // 1-Wire reset
        if(!reset()) {
            // reset the search
            last_discrepancy = 0;
            last_device_flag = false;
            last_family_discrepancy = 0;
            return false;
        }

        // issue the search command
        if(search_mode == true) {
            write(0xF0); // NORMAL SEARCH
        } else {
            write(0xEC); // CONDITIONAL SEARCH
        }

        // loop to do the search
        do {
            // read a bit and its complement
            id_bit = read_bit();
            cmp_id_bit = read_bit();

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
                    if(id_bit_number < last_discrepancy)
                        search_direction = ((saved_rom[rom_byte_number] & rom_byte_mask) > 0);
                    else
                        // if equal to last pick 1, if not then pick 0
                        search_direction = (id_bit_number == last_discrepancy);

                    // if 0 was picked then record its position in LastZero
                    if(search_direction == 0) {
                        last_zero = id_bit_number;

                        // check for Last discrepancy in family
                        if(last_zero < 9) last_family_discrepancy = last_zero;
                    }
                }

                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                if(search_direction == 1)
                    saved_rom[rom_byte_number] |= rom_byte_mask;
                else
                    saved_rom[rom_byte_number] &= ~rom_byte_mask;

                // serial number search direction write bit
                write_bit(search_direction);

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
            last_discrepancy = last_zero;

            // check for last device
            if(last_discrepancy == 0) last_device_flag = true;

            search_result = true;
        }
    }

    // if no device found then reset counters so next 'search' will be like a first
    if(!search_result || !saved_rom[0]) {
        last_discrepancy = 0;
        last_device_flag = false;
        last_family_discrepancy = 0;
        search_result = false;
    } else {
        for(int i = 0; i < 8; i++) newAddr[i] = saved_rom[i];
    }

    return search_result;
}

bool OneWireMaster::reset(void) {
    uint8_t r;
    uint8_t retries = 125;

    // wait until the gpio is high
    gpio_write(gpio, true);
    do {
        if(--retries == 0) return 0;
        delay_us(2);
    } while(!gpio_read(gpio));

    // pre delay
    delay_us(OneWireTiming::RESET_DELAY_PRE);

    // drive low
    gpio_write(gpio, false);
    delay_us(OneWireTiming::RESET_DRIVE);

    // release
    gpio_write(gpio, true);
    delay_us(OneWireTiming::RESET_RELEASE);

    // read and post delay
    r = !gpio_read(gpio);
    delay_us(OneWireTiming::RESET_DELAY_POST);

    return r;
}

bool OneWireMaster::read_bit(void) {
    bool result;

    // drive low
    gpio_write(gpio, false);
    delay_us(OneWireTiming::READ_DRIVE);

    // release
    gpio_write(gpio, true);
    delay_us(OneWireTiming::READ_RELEASE);

    // read and post delay
    result = gpio_read(gpio);
    delay_us(OneWireTiming::READ_DELAY_POST);

    return result;
}

void OneWireMaster::write_bit(bool value) {
    if(value) {
        // drive low
        gpio_write(gpio, false);
        delay_us(OneWireTiming::WRITE_1_DRIVE);

        // release
        gpio_write(gpio, true);
        delay_us(OneWireTiming::WRITE_1_RELEASE);
    } else {
        // drive low
        gpio_write(gpio, false);
        delay_us(OneWireTiming::WRITE_0_DRIVE);

        // release
        gpio_write(gpio, true);
        delay_us(OneWireTiming::WRITE_0_RELEASE);
    }
}

uint8_t OneWireMaster::read(void) {
    uint8_t result = 0;

    for(uint8_t bitMask = 0x01; bitMask; bitMask <<= 1) {
        if(read_bit()) {
            result |= bitMask;
        }
    }

    return result;
}

void OneWireMaster::read_bytes(uint8_t* buffer, uint16_t count) {
    for(uint16_t i = 0; i < count; i++) {
        buffer[i] = read();
    }
}

void OneWireMaster::write(uint8_t value) {
    uint8_t bitMask;

    for(bitMask = 0x01; bitMask; bitMask <<= 1) {
        write_bit((bitMask & value) ? 1 : 0);
    }
}

void OneWireMaster::skip(void) {
    write(0xCC);
}
