#pragma once
#include <stdint.h>
#include <atomic>

class DecoderAnalyzer {
public:
    bool read(uint8_t* data, uint8_t data_size);
    void process_front(bool polarity, uint32_t time);

    DecoderAnalyzer();
    ~DecoderAnalyzer();

private:
    void reset_state();

    std::atomic<bool> ready;

    static const uint32_t data_size = 2048;
    uint32_t data_index = 0;
    uint32_t* data;
};
