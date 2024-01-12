#include "t5577.h"
#include <furi.h>
#include <furi_hal_rfid.h>

#define T5577_TIMING_WAIT_TIME 400
#define T5577_TIMING_START_GAP 30
#define T5577_TIMING_WRITE_GAP 18
#define T5577_TIMING_DATA_0 24
#define T5577_TIMING_DATA_1 56
#define T5577_TIMING_PROGRAM 700

#define T5577_OPCODE_PAGE_0 0b10
#define T5577_OPCODE_PAGE_1 0b11
#define T5577_OPCODE_RESET 0b00

#define T5577_BLOCKS_IN_PAGE_0 8
#define T5577_BLOCKS_IN_PAGE_1 4

//TODO: use .txt file in resources for passwords.
const uint32_t default_passwords[] = {
    0x51243648, 0x000D8787, 0x19920427, 0x50524F58, 0xF9DCEBA0, 0x65857569, 0x05D73B9F, 0x89A69E60,
    0x314159E0, 0xAA55BBBB, 0xA5B4C3D2, 0x1C0B5848, 0x00434343, 0x444E4752, 0x4E457854, 0x44B44CAE,
    0x88661858, 0xE9920427, 0x575F4F4B, 0x50520901, 0x20206666, 0x65857569, 0x5469616E, 0x7686962A,
    0xC0F5009A, 0x07CEE75D, 0xfeedbeef, 0xdeadc0de, 0x00000000, 0x11111111, 0x22222222, 0x33333333,
    0x44444444, 0x55555555, 0x66666666, 0x77777777, 0x88888888, 0x99999999, 0xAAAAAAAA, 0xBBBBBBBB,
    0xCCCCCCCC, 0xDDDDDDDD, 0xEEEEEEEE, 0xFFFFFFFF, 0xa0a1a2a3, 0xb0b1b2b3, 0x50415353, 0x00000001,
    0x00000002, 0x0000000a, 0x0000000b, 0x01020304, 0x02030405, 0x03040506, 0x04050607, 0x05060708,
    0x06070809, 0x0708090A, 0x08090A0B, 0x090A0B0C, 0x0A0B0C0D, 0x0B0C0D0E, 0x0C0D0E0F, 0x01234567,
    0x12345678, 0x10000000, 0x20000000, 0x30000000, 0x40000000, 0x50000000, 0x60000000, 0x70000000,
    0x80000000, 0x90000000, 0xA0000000, 0xB0000000, 0xC0000000, 0xD0000000, 0xE0000000, 0xF0000000,
    0x10101010, 0x01010101, 0x11223344, 0x22334455, 0x33445566, 0x44556677, 0x55667788, 0x66778899,
    0x778899AA, 0x8899AABB, 0x99AABBCC, 0xAABBCCDD, 0xBBCCDDEE, 0xCCDDEEFF, 0x0CB7E7FC, 0xFABADA11,
    0x87654321, 0x12341234, 0x69696969, 0x12121212, 0x12344321, 0x1234ABCD, 0x11112222, 0x13131313,
    0x10041004, 0x31415926, 0xabcd1234, 0x20002000, 0x19721972, 0xaa55aa55, 0x55aa55aa, 0x4f271149,
    0x07d7bb0b, 0x9636ef8f, 0xb5f44686, 0x9E3779B9, 0xC6EF3720, 0x7854794A, 0xF1EA5EED, 0x69314718,
    0x57721566, 0x93C467E3, 0x27182818, 0x50415353};

const uint32_t* t5577_get_default_passwords(uint8_t* len) {
    *len = sizeof(default_passwords) / sizeof(uint32_t);
    return default_passwords;
}

static void t5577_start() {
    furi_hal_rfid_tim_read_start(125000, 0.5);

    // do not ground the antenna
    furi_hal_rfid_pin_pull_release();
}

static void t5577_stop() {
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

static void t5577_write_reset() {
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

void t5577_write_with_mask(LFRFIDT5577* data, uint8_t page, uint32_t password) {
    t5577_start();
    FURI_CRITICAL_ENTER();

    uint8_t mask = data->mask;

    size_t pages_total = (page == 0) ? T5577_BLOCKS_IN_PAGE_0 : T5577_BLOCKS_IN_PAGE_1;

    for(size_t i = 0; i < pages_total; i++) {
        bool need_to_write = mask & 1;
        mask >>= 1;
        if(!need_to_write) continue;
        t5577_write_block_pass(page, i, false, data->block[i], true, password);
    }
    t5577_write_reset();
    FURI_CRITICAL_EXIT();
    t5577_stop();
}