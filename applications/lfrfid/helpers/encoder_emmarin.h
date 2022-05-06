#pragma once
#include "encoder_generic.h"

class EncoderEM : public EncoderGeneric {
public:
    /**
     * @brief init data to emulate
     * 
     * @param data 1 byte FC, next 4 byte SN
     * @param data_size must be 5
     */
    void init(const uint8_t* data, const uint8_t data_size) final;

    void get_next(bool* polarity, uint16_t* period, uint16_t* pulse) final;

private:
    // clock pulses per bit
    static const uint8_t clocks_per_bit = 64;

    uint64_t card_data;
    uint8_t card_data_index;
};
