#pragma once
#include <stdint.h>
#include <atomic>
#include <lib/toolbox/manchester_decoder.h>
#include "protocols/protocol_emmarin.h"
class DecoderEMMarin {
public:
    bool read(uint8_t* data, uint8_t data_size);
    void process_front(bool polarity, uint32_t time);

    DecoderEMMarin();

private:
    void reset_state();

    uint64_t read_data = 0;
    std::atomic<bool> ready;

    ManchesterState manchester_saved_state;
    ProtocolEMMarin em_marin;
};
