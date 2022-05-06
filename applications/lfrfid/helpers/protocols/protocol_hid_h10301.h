#pragma once
#include "protocol_generic.h"

class ProtocolHID10301 : public ProtocolGeneric {
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
};
