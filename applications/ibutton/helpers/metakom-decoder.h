#pragma once
#include <stdint.h>
#include <atomic>

class MetakomDecoder {
public:
    bool read(uint8_t* data, uint8_t data_size);
    void process_front(bool polarity, uint32_t time);

    MetakomDecoder();

private:
    enum class BitState : uint8_t {
        WAIT_FRONT_HIGH,
        WAIT_FRONT_LOW,
    };

    BitState bit_state;

    enum class State : uint8_t {
        WAIT_PERIOD_SYNC,
        WAIT_START_BIT,
        WAIT_START_WORD,
        READ_WORD,
        READ_STOP_WORD,
    };

    State state;

    // high + low period time
    uint32_t period_time;
    uint32_t low_time_storage;

    static const uint8_t period_sample_count = 10;
    uint8_t period_sample_index;
    uint32_t period_sample_data[period_sample_count];

    // ready flag, key is readed and valid
    std::atomic<bool> ready;

    // max period, 230us x clock per us
    uint32_t max_period;

    uint8_t tmp_data;
    uint8_t tmp_counter;

    uint32_t key_data;
    uint8_t key_data_index;

    void reset_state();
    bool parity_check(uint8_t data);

    bool process_bit(bool polarity, uint32_t time, uint32_t* high_time, uint32_t* low_time);
};
