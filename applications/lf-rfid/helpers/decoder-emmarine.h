#pragma once
#include <stdint.h>
#include <atomic>
#include "manchester-decoder.h"
#include "protocols/protocol-emmarin.h"
class DecoderEMMarine {
public:
    bool read(uint8_t* data, uint8_t data_size);
    void process_front(bool polarity, uint32_t time);

    DecoderEMMarine();

private:
    void reset_state();

    uint64_t readed_data = 0;
    std::atomic<bool> ready;

    ManchesterState manchester_saved_state;
    ProtocolEMMarin em_marine;
};
