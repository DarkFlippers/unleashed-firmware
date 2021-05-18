#include "rfid-writer.h"
#include <api-hal.h>
#include "protocols/protocol-emmarin.h"

extern COMP_HandleTypeDef hcomp1;

/**
 * @brief all timings are specified in field clocks (field clock = 125 kHz, 8 us)
 * 
 */
class T55xxTiming {
public:
    constexpr static const uint16_t wait_time = 400;
    constexpr static const uint8_t start_gap = 15;
    constexpr static const uint8_t write_gap = 10;
    constexpr static const uint8_t data_0 = 24;
    constexpr static const uint8_t data_1 = 56;
    constexpr static const uint16_t program = 700;
};

class T55xxCmd {
public:
    constexpr static const uint8_t opcode_page_0 = 0b10;
    constexpr static const uint8_t opcode_page_1 = 0b11;
    constexpr static const uint8_t opcode_reset = 0b00;
};

RfidWriter::RfidWriter() {
}

RfidWriter::~RfidWriter() {
}

void RfidWriter::start() {
    api_hal_rfid_tim_read(125000, 0.5);
    api_hal_rfid_pins_read();
}

void RfidWriter::stop() {
    api_hal_rfid_tim_read_stop();
    api_hal_rfid_tim_reset();
    api_hal_rfid_pins_reset();
}

void RfidWriter::write_gap(uint32_t gap_time) {
    api_hal_rfid_tim_read_stop();
    delay_us(gap_time * 8);
    api_hal_rfid_tim_read_start();
}

void RfidWriter::write_bit(bool value) {
    if(value) {
        delay_us(T55xxTiming::data_1 * 8);
    } else {
        delay_us(T55xxTiming::data_0 * 8);
    }
    write_gap(T55xxTiming::write_gap);
}

void RfidWriter::write_byte(uint8_t value) {
    for(uint8_t i = 0; i < 8; i++) {
        write_bit((value >> i) & 1);
    }
}

void RfidWriter::write_block(uint8_t page, uint8_t block, bool lock_bit, uint32_t data) {
    // wait to power card
    api_hal_rfid_tim_read_start();
    delay_us(T55xxTiming::wait_time * 8);

    // start gap
    write_gap(T55xxTiming::start_gap);

    // opcode
    switch(page) {
    case 0:
        write_bit(1);
        write_bit(0);
        break;
    case 1:
        write_bit(1);
        write_bit(1);
        break;
    default:
        furi_check(false);
        break;
    }

    // lock bit
    write_bit(lock_bit);

    // data
    for(uint8_t i = 0; i < 32; i++) {
        write_bit((data >> (31 - i)) & 1);
    }

    // block address
    write_bit((block >> 2) & 1);
    write_bit((block >> 1) & 1);
    write_bit((block >> 0) & 1);

    delay_us(T55xxTiming::program * 8);

    api_hal_rfid_tim_read_stop();
}

void RfidWriter::write_em(uint8_t em_data[5]) {
    ProtocolEMMarin em_card;
    uint64_t em_encoded_data;
    em_card.encode(em_data, 5, reinterpret_cast<uint8_t*>(&em_encoded_data), sizeof(uint64_t));
    uint32_t em_config_block_data = 0b01100000000101001000000001000000;

    __disable_irq();
    write_block(0, 0, false, em_config_block_data);
    write_block(0, 1, false, em_encoded_data);
    write_block(0, 2, false, em_encoded_data >> 32);
    __enable_irq();
}
