#pragma once
#include "encoder_generic.h"

class EncoderIndala_40134 : public EncoderGeneric {
public:
    /**
     * @brief init data to emulate
     * 
     * @param data indala raw data
     * @param data_size must be 5
     */
    void init(const uint8_t* data, const uint8_t data_size) final;

    void get_next(bool* polarity, uint16_t* period, uint16_t* pulse) final;

private:
    uint64_t card_data;
    uint8_t card_data_index;
    uint8_t bit_clock_index;
    bool last_bit;
    bool current_polarity;
    static const uint8_t clock_per_bit = 16;
};
