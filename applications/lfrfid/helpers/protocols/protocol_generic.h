#pragma once
#include "stdint.h"
#include "stdbool.h"

class ProtocolGeneric {
public:
    /**
     * @brief Get the encoded data size
     * 
     * @return uint8_t size of encoded data in bytes
     */
    virtual uint8_t get_encoded_data_size() = 0;

    /**
     * @brief Get the decoded data size
     * 
     * @return uint8_t size of decoded data in bytes
     */
    virtual uint8_t get_decoded_data_size() = 0;

    /**
     * @brief encode decoded data
     * 
     * @param decoded_data 
     * @param decoded_data_size 
     * @param encoded_data 
     * @param encoded_data_size 
     */
    virtual void encode(
        const uint8_t* decoded_data,
        const uint8_t decoded_data_size,
        uint8_t* encoded_data,
        const uint8_t encoded_data_size) = 0;

    /**
     * @brief decode encoded data
     * 
     * @param encoded_data 
     * @param encoded_data_size 
     * @param decoded_data 
     * @param decoded_data_size 
     */
    virtual void decode(
        const uint8_t* encoded_data,
        const uint8_t encoded_data_size,
        uint8_t* decoded_data,
        const uint8_t decoded_data_size) = 0;

    /**
     * @brief fast check that data can be correctly decoded
     * 
     * @param encoded_data 
     * @param encoded_data_size 
     * @return true - can be correctly decoded
     * @return false - cannot be correctly decoded
     */
    virtual bool can_be_decoded(const uint8_t* encoded_data, const uint8_t encoded_data_size) = 0;

    virtual ~ProtocolGeneric(){};
};
