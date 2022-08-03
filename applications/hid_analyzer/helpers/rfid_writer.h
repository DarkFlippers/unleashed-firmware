#pragma once
#include "stdint.h"

class RfidWriter {
public:
    RfidWriter();
    ~RfidWriter();
    void start();
    void stop();
    void write_em(const uint8_t em_data[5]);
    void write_hid(const uint8_t hid_data[3]);
    void write_indala(const uint8_t indala_data[3]);

private:
    void write_gap(uint32_t gap_time);
    void write_bit(bool value);
    void write_byte(uint8_t value);
    void write_block(uint8_t page, uint8_t block, bool lock_bit, uint32_t data);
    void write_reset();
};
