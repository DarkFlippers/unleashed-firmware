#pragma once
#include <stdbool.h>
#include <stdint.h>

class EncoderGeneric {
public:
    /**
     * @brief init encoder
     * 
     * @param data data array
     * @param data_size data array size
     */
    virtual void init(const uint8_t* data, const uint8_t data_size) = 0;

    /**
     * @brief Get the next timer pulse
     * 
     * @param polarity pulse polarity true = high2low, false = low2high
     * @param period overall period time in timer clicks
     * @param pulse pulse time in timer clicks
     */
    virtual void get_next(bool* polarity, uint16_t* period, uint16_t* pulse) = 0;

    virtual ~EncoderGeneric(){};

private:
};
