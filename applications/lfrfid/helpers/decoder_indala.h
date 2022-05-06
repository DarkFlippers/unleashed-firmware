#pragma once
#include <stdint.h>
#include <limits.h>
#include <atomic>
#include "protocols/protocol_indala_40134.h"

class DecoderIndala {
public:
    bool read(uint8_t* data, uint8_t data_size);
    void process_front(bool polarity, uint32_t time);

    void process_internal(bool polarity, uint32_t time, uint64_t* data);

    DecoderIndala();

private:
    void reset_state();

    uint64_t raw_data;
    uint64_t cursed_raw_data;

    std::atomic<bool> ready;
    std::atomic<bool> cursed_data_valid;
    ProtocolIndala40134 indala;
};
