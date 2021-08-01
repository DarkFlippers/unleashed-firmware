#pragma once
#include <stdint.h>
#include <atomic>

class DecoderGpioOut {
public:
    void process_front(bool polarity, uint32_t time);

    DecoderGpioOut();
    ~DecoderGpioOut();

private:
    void reset_state();
};
