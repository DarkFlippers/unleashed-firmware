#include "rfid_writer.h"
#include "protocols/protocol_ioprox.h"
#include <furi_hal.h>
#include "protocols/protocol_emmarin.h"
#include "protocols/protocol_hid_h10301.h"
#include "protocols/protocol_indala_40134.h"

/**
 * @brief all timings are specified in field clocks (field clock = 125 kHz, 8 us)
 * 
 */
class T55xxTiming {
public:
    constexpr static const uint16_t wait_time = 400;
    constexpr static const uint8_t start_gap = 30;
    constexpr static const uint8_t write_gap = 18;
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
    furi_hal_rfid_tim_read(125000, 0.5);
    furi_hal_rfid_pins_read();
    furi_hal_rfid_tim_read_start();

    // do not ground the antenna
    furi_hal_rfid_pin_pull_release();
}

void RfidWriter::stop() {
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_tim_reset();
    furi_hal_rfid_pins_reset();
}

void RfidWriter::write_gap(uint32_t gap_time) {
    furi_hal_rfid_tim_read_stop();
    furi_hal_delay_us(gap_time * 8);
    furi_hal_rfid_tim_read_start();
}

void RfidWriter::write_bit(bool value) {
    if(value) {
        furi_hal_delay_us(T55xxTiming::data_1 * 8);
    } else {
        furi_hal_delay_us(T55xxTiming::data_0 * 8);
    }
    write_gap(T55xxTiming::write_gap);
}

void RfidWriter::write_byte(uint8_t value) {
    for(uint8_t i = 0; i < 8; i++) {
        write_bit((value >> i) & 1);
    }
}

void RfidWriter::write_block(uint8_t page, uint8_t block, bool lock_bit, uint32_t data) {
    furi_hal_delay_us(T55xxTiming::wait_time * 8);

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

    furi_hal_delay_us(T55xxTiming::program * 8);

    furi_hal_delay_us(T55xxTiming::wait_time * 8);
    write_reset();
}

void RfidWriter::write_reset() {
    write_gap(T55xxTiming::start_gap);
    write_bit(1);
    write_bit(0);
}

void RfidWriter::write_em(const uint8_t em_data[5]) {
    ProtocolEMMarin em_card;
    uint64_t em_encoded_data;
    em_card.encode(em_data, 5, reinterpret_cast<uint8_t*>(&em_encoded_data), sizeof(uint64_t));
    const uint32_t em_config_block_data = 0b00000000000101001000000001000000;

    FURI_CRITICAL_ENTER();
    write_block(0, 0, false, em_config_block_data);
    write_block(0, 1, false, em_encoded_data);
    write_block(0, 2, false, em_encoded_data >> 32);
    write_reset();
    FURI_CRITICAL_EXIT();
}

void RfidWriter::write_hid(const uint8_t hid_data[3]) {
    ProtocolHID10301 hid_card;
    uint32_t card_data[3];
    hid_card.encode(hid_data, 3, reinterpret_cast<uint8_t*>(&card_data), sizeof(card_data) * 3);

    const uint32_t hid_config_block_data = 0b00000000000100000111000001100000;

    FURI_CRITICAL_ENTER();
    write_block(0, 0, false, hid_config_block_data);
    write_block(0, 1, false, card_data[0]);
    write_block(0, 2, false, card_data[1]);
    write_block(0, 3, false, card_data[2]);
    write_reset();
    FURI_CRITICAL_EXIT();
}

/** Endian fixup. Translates an ioprox block into a t5577 block */
static uint32_t ioprox_encode_block(const uint8_t block_data[4]) {
    uint8_t raw_card_data[] = {block_data[3], block_data[2], block_data[1], block_data[0]};
    return *reinterpret_cast<uint32_t*>(&raw_card_data);
}

void RfidWriter::write_ioprox(const uint8_t ioprox_data[4]) {
    ProtocolIoProx ioprox_card;

    uint8_t encoded_data[8];
    ioprox_card.encode(ioprox_data, 4, encoded_data, sizeof(encoded_data));

    const uint32_t ioprox_config_block_data = 0b00000000000101000111000001000000;

    FURI_CRITICAL_ENTER();
    write_block(0, 0, false, ioprox_config_block_data);
    write_block(0, 1, false, ioprox_encode_block(&encoded_data[0]));
    write_block(0, 2, false, ioprox_encode_block(&encoded_data[4]));
    write_reset();
    FURI_CRITICAL_EXIT();
}

void RfidWriter::write_indala(const uint8_t indala_data[3]) {
    ProtocolIndala40134 indala_card;
    uint32_t card_data[2];
    indala_card.encode(
        indala_data, 3, reinterpret_cast<uint8_t*>(&card_data), sizeof(card_data) * 2);

    const uint32_t indala_config_block_data = 0b00000000000010000001000001000000;

    FURI_CRITICAL_ENTER();
    write_block(0, 0, false, indala_config_block_data);
    write_block(0, 1, false, card_data[0]);
    write_block(0, 2, false, card_data[1]);
    write_reset();
    FURI_CRITICAL_EXIT();
}
