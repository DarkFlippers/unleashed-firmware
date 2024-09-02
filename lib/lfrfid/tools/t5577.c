#include "t5577.h"
#include <furi.h>
#include <furi_hal_rfid.h>

#define T5577_TIMING_WAIT_TIME 400
#define T5577_TIMING_START_GAP 30
#define T5577_TIMING_WRITE_GAP 18
#define T5577_TIMING_DATA_0    24
#define T5577_TIMING_DATA_1    56
#define T5577_TIMING_PROGRAM   700

#define T5577_OPCODE_PAGE_0 0b10
#define T5577_OPCODE_PAGE_1 0b11
#define T5577_OPCODE_RESET  0b00

static void t5577_start(void) {
    furi_hal_rfid_tim_read_start(125000, 0.5);

    // do not ground the antenna
    furi_hal_rfid_pin_pull_release();
}

static void t5577_stop(void) {
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
}

static void t5577_write_gap(uint32_t gap_time) {
    furi_hal_rfid_tim_read_pause();
    furi_delay_us(gap_time * 8);
    furi_hal_rfid_tim_read_continue();
}

static void t5577_write_bit(bool value) {
    if(value) {
        furi_delay_us(T5577_TIMING_DATA_1 * 8);
    } else {
        furi_delay_us(T5577_TIMING_DATA_0 * 8);
    }
    t5577_write_gap(T5577_TIMING_WRITE_GAP);
}

static void t5577_write_opcode(uint8_t value) {
    t5577_write_bit((value >> 1) & 1);
    t5577_write_bit((value >> 0) & 1);
}

static void t5577_write_reset(void) {
    t5577_write_gap(T5577_TIMING_START_GAP);
    t5577_write_bit(1);
    t5577_write_bit(0);
}

static void t5577_write_block_pass(
    uint8_t page,
    uint8_t block,
    bool lock_bit,
    uint32_t data,
    bool with_pass,
    uint32_t password) {
    furi_delay_us(T5577_TIMING_WAIT_TIME * 8);

    // start gap
    t5577_write_gap(T5577_TIMING_START_GAP);

    // opcode for page
    t5577_write_opcode((page == 1) ? T5577_OPCODE_PAGE_1 : T5577_OPCODE_PAGE_0);

    // password
    if(with_pass) {
        for(uint8_t i = 0; i < 32; i++) {
            t5577_write_bit((password >> (31 - i)) & 1);
        }
    }

    // lock bit
    t5577_write_bit(lock_bit);

    // data
    for(uint8_t i = 0; i < 32; i++) {
        t5577_write_bit((data >> (31 - i)) & 1);
    }

    // block address
    t5577_write_bit((block >> 2) & 1);
    t5577_write_bit((block >> 1) & 1);
    t5577_write_bit((block >> 0) & 1);

    furi_delay_us(T5577_TIMING_PROGRAM * 8);

    furi_delay_us(T5577_TIMING_WAIT_TIME * 8);
    t5577_write_reset();
}

static void t5577_write_block_simple(uint8_t block, bool lock_bit, uint32_t data) {
    t5577_write_block_pass(0, block, lock_bit, data, false, 0);
}

void t5577_write(LFRFIDT5577* data) {
    t5577_start();
    FURI_CRITICAL_ENTER();
    for(size_t i = 0; i < data->blocks_to_write; i++) {
        t5577_write_block_simple(i, false, data->block[i]);
    }
    t5577_write_reset();
    FURI_CRITICAL_EXIT();
    t5577_stop();
}

void t5577_write_with_pass(LFRFIDT5577* data, uint32_t password) {
    t5577_start();
    FURI_CRITICAL_ENTER();
    for(size_t i = 0; i < data->blocks_to_write; i++) {
        t5577_write_block_pass(0, i, false, data->block[i], true, password);
    }
    t5577_write_reset();
    FURI_CRITICAL_EXIT();
    t5577_stop();
}

void t5577_write_with_mask(LFRFIDT5577* data, uint8_t page, bool with_pass, uint32_t password) {
    t5577_start();
    FURI_CRITICAL_ENTER();

    uint8_t mask = data->mask;

    size_t pages_total = (page == 0) ? T5577_BLOCKS_IN_PAGE_0 : T5577_BLOCKS_IN_PAGE_1;

    for(size_t i = 0; i < pages_total; i++) {
        bool need_to_write = mask & 1;
        mask >>= 1;
        if(!need_to_write) continue;
        t5577_write_block_pass(page, i, false, data->block[i], with_pass, password);
    }
    t5577_write_reset();
    FURI_CRITICAL_EXIT();
    t5577_stop();
}
