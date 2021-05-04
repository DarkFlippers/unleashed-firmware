#pragma once
#include "encoder-generic.h"

class EncoderHID : public EncoderGeneric {
public:
    /**
     * @brief init data to emulate
     * 
     * @param data 1 byte FC, next 2 byte SN
     * @param data_size must be 3
     */
    void init(const uint8_t* data, const uint8_t data_size) final;

    void get_next(bool* polarity, uint16_t* period, uint16_t* pulse) final;

private:
    uint64_t card_data;
    uint8_t card_data_index;
};