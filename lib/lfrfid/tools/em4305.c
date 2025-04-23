#include "em4305.h"
#include <furi.h>
#include <furi_hal_rfid.h>

#define TAG "EM4305"

#define EM4305_TIMING_1     (32)
#define EM4305_TIMING_0_OFF (23)
#define EM4305_TIMING_0_ON  (18)

#define EM4305_FIELD_STOP_OFF_CYCLES (55)
#define EM4305_FIELD_STOP_ON_CYCLES  (18)

#define EM4305_TIMING_POWER_CHECK  (1480)
#define EM4305_TIMING_EEPROM_WRITE (9340)

static bool em4305_line_parity(uint8_t data) {
    uint8_t parity = 0;
    for(uint8_t i = 0; i < 8; i++) {
        parity ^= (data >> i) & 1;
    }
    return parity;
}

static uint64_t em4305_prepare_data(uint32_t data) {
    uint8_t i, j;
    uint64_t data_with_parity = 0;

    // 4 lines of 8 bits of data
    // line even parity at bits 8 17 26 35
    // column even parity at bits 36-43
    // bit 44 is always 0
    // final table is 5 lines of 9 bits

    // line parity
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 8; j++) {
            data_with_parity = (data_with_parity << 1) | ((data >> (i * 8 + j)) & 1);
        }
        data_with_parity = (data_with_parity << 1) | (uint64_t)em4305_line_parity(data >> (i * 8));
    }

    // column parity
    for(i = 0; i < 8; i++) {
        uint8_t column_parity = 0;
        for(j = 0; j < 4; j++) {
            column_parity ^= (data >> (j * 8 + i)) & 1;
        }
        data_with_parity = (data_with_parity << 1) | column_parity;
    }

    // bit 44
    data_with_parity = (data_with_parity << 1) | 0;

    return data_with_parity;
}

static void em4305_start(void) {
    furi_hal_rfid_tim_read_start(125000, 0.5);

    // do not ground the antenna
    furi_hal_rfid_pin_pull_release();
}

static void em4305_stop(void) {
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
}

static void em4305_write_bit(bool value) {
    if(value) {
        furi_delay_us(EM4305_TIMING_1 * 8);
    } else {
        furi_hal_rfid_tim_read_pause();
        furi_delay_us(EM4305_TIMING_0_OFF * 8);
        furi_hal_rfid_tim_read_continue();
        furi_delay_us(EM4305_TIMING_0_ON * 8);
    }
}

static void em4305_write_opcode(uint8_t value) {
    // 3 bit opcode
    for(uint8_t i = 0; i < 3; i++) {
        em4305_write_bit((value >> i) & 1);
    }

    // parity
    bool parity = 0;
    for(uint8_t i = 0; i < 3; i++) {
        parity ^= (value >> i) & 1;
    }
    em4305_write_bit(parity);
}

static void em4305_field_stop() {
    furi_hal_rfid_tim_read_pause();
    furi_delay_us(EM4305_FIELD_STOP_OFF_CYCLES * 8);
    furi_hal_rfid_tim_read_continue();
    furi_delay_us(EM4305_FIELD_STOP_ON_CYCLES * 8);
}

static void em4305_write_word(uint8_t address, uint32_t data) {
    // parity
    uint64_t data_with_parity = em4305_prepare_data(data);

    // power up the tag
    furi_delay_us(8000);

    // field stop
    em4305_field_stop();

    // start bit
    em4305_write_bit(0);

    // opcode
    em4305_write_opcode(EM4x05_OPCODE_WRITE);

    // address
    bool address_parity = 0;
    for(uint8_t i = 0; i < 4; i++) {
        em4305_write_bit((address >> (i)) & 1);
        address_parity ^= (address >> (i)) & 1;
    }
    em4305_write_bit(0);
    em4305_write_bit(0);
    em4305_write_bit(address_parity);

    // data
    for(uint8_t i = 0; i < 45; i++) {
        em4305_write_bit((data_with_parity >> (44 - i)) & 1);
    }

    // wait for power check and eeprom write
    furi_delay_us(EM4305_TIMING_POWER_CHECK);
    furi_delay_us(EM4305_TIMING_EEPROM_WRITE);
}

void em4305_write(LFRFIDEM4305* data) {
    furi_check(data);

    em4305_start();
    FURI_CRITICAL_ENTER();

    for(uint8_t i = 0; i < EM4x05_WORD_COUNT; i++) {
        if(data->mask & (1 << i)) {
            em4305_write_word(i, data->word[i]);
        }
    }

    FURI_CRITICAL_EXIT();
    em4305_stop();
}
