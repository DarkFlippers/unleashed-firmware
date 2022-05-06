#pragma once
#include "encoder_generic.h"
#include "osc_fsk.h"

class EncoderHID_H10301 : public EncoderGeneric {
public:
    /**
     * @brief init data to emulate
     * 
     * @param data 1 byte FC, next 2 byte SN
     * @param data_size must be 3
     */
    void init(const uint8_t* data, const uint8_t data_size) final;
    void get_next(bool* polarity, uint16_t* period, uint16_t* pulse) final;
    EncoderHID_H10301();
    ~EncoderHID_H10301();

private:
    static const uint8_t card_data_max = 3;
    uint32_t card_data[card_data_max];
    uint8_t card_data_index;
    void write_bit(bool bit, uint8_t position);
    void write_raw_bit(bool bit, uint8_t position);

    OscFSK* fsk;
};
