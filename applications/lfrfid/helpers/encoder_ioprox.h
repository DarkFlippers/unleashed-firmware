#pragma once
#include "encoder_generic.h"
#include "osc_fsk.h"

class EncoderIoProx : public EncoderGeneric {
public:
    /**
     * @brief init data to emulate
     * 
     * @param data 1 byte FC, 1 byte Version, 2 bytes code
     * @param data_size must be 4
     */
    void init(const uint8_t* data, const uint8_t data_size) final;
    void get_next(bool* polarity, uint16_t* period, uint16_t* pulse) final;
    EncoderIoProx();
    ~EncoderIoProx();

private:
    static const uint8_t card_data_max = 8;

    uint8_t card_data[card_data_max];
    uint8_t card_data_index;

    OscFSK* fsk;
};
