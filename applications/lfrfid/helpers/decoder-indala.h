#pragma once
#include <stdint.h>
#include <limits.h>
#include <atomic>

class DecoderIndala {
public:
    bool read(uint8_t* data, uint8_t data_size);
    void process_front(bool polarity, uint32_t time);

    DecoderIndala();

private:
    void reset_state();

    void verify();
    void verify_inner();

    uint32_t last_pulse_time = 0;
    uint32_t pulse_count = 0;
    uint32_t overall_pulse_count = 0;

    uint64_t readed_data = 0;

    std::atomic<bool> ready;
    uint8_t facility = 0;
    uint16_t number = 0;
};