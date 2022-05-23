#pragma once
#include <stdint.h>
#include <atomic>
#include "protocols/protocol_ioprox.h"

class DecoderIoProx {
public:
    bool read(uint8_t* data, uint8_t data_size);
    void process_front(bool polarity, uint32_t time);
    DecoderIoProx();

private:
    uint32_t current_period_duration = 0;
    uint32_t demodulation_sample_duration = 0;

    bool current_demodulated_value = false;
    bool demodulated_value_invalid = false;

    uint8_t raw_data[8] = {0};
    void store_data(bool data);

    std::atomic<bool> ready;

    void reset_state();
    ProtocolIoProx ioprox;
};
