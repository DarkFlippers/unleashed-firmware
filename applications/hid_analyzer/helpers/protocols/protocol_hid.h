#pragma once
#include "protocol_generic.h"

class ProtocolHID : public ProtocolGeneric {
public:
    uint8_t get_encoded_data_size() final;
    uint8_t get_decoded_data_size() final;

    void encode(
        const uint8_t* decoded_data,
        const uint8_t decoded_data_size,
        uint8_t* encoded_data,
        const uint8_t encoded_data_size) final;

    void decode(
        const uint8_t* encoded_data,
        const uint8_t encoded_data_size,
        uint8_t* decoded_data,
        const uint8_t decoded_data_size) final;

    bool can_be_decoded(const uint8_t* encoded_data, const uint8_t encoded_data_size) final;

private:
    int16_t find_second_1(const uint8_t* encoded_data);
};
