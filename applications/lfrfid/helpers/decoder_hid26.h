#pragma once
#include <stdint.h>
#include <atomic>
#include "protocols/protocol_hid_h10301.h"

class DecoderHID26 {
public:
    bool read(uint8_t* data, uint8_t data_size);
    void process_front(bool polarity, uint32_t time);
    DecoderHID26();

private:
    uint32_t last_pulse_time = 0;
    bool last_pulse;
    uint8_t pulse_count;

    uint32_t stored_data[3] = {0, 0, 0};
    void store_data(bool data);

    std::atomic<bool> ready;

    void reset_state();
    ProtocolHID10301 hid;
};
