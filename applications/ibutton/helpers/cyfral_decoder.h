#pragma once
#include <stdint.h>
#include <atomic>

class CyfralDecoder {
public:
    bool read(uint8_t* data, uint8_t data_size);
    void process_front(bool polarity, uint32_t time);

    CyfralDecoder();

private:
    enum class BitState : uint8_t {
        WAIT_FRONT_HIGH,
        WAIT_FRONT_LOW,
    };

    enum class State : uint8_t {
        WAIT_START_NIBBLE,
        READ_NIBBLE,
        READ_STOP_NIBBLE,
    };

    State state;
    BitState bit_state;

    bool process_bit(bool polarity, uint32_t time, bool* readed, bool* readed_value);
    void reset_state();
    bool nibble_valid(uint8_t data);

    // high + low period time
    uint32_t period_time;

    // ready flag, key is readed and valid
    std::atomic<bool> ready;

    // key data storage
    uint16_t key_data;

    // temporary nibble storage
    uint8_t readed_nibble;

    // data valid flag
    // MUST be checked only in READ_STOP_NIBBLE state
    bool data_valid;

    // nibble index, we expect 8 nibbles
    uint8_t index;

    // bit index in nibble, 4 bit per nibble
    uint8_t bit_index;

    // max period, 230us x clock per us
    uint32_t max_period;
};
