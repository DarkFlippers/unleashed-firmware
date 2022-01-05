#include "encoder_emmarin.h"
#include "protocols/protocol_emmarin.h"
#include <furi.h>

void EncoderEM::init(const uint8_t* data, const uint8_t data_size) {
    ProtocolEMMarin em_marin;
    em_marin.encode(data, data_size, reinterpret_cast<uint8_t*>(&card_data), sizeof(uint64_t));

    card_data_index = 0;
}

// data transmitted as manchester encoding
// 0 - high2low
// 1 - low2high
void EncoderEM::get_next(bool* polarity, uint16_t* period, uint16_t* pulse) {
    *period = clocks_per_bit;
    *pulse = clocks_per_bit / 2;
    *polarity = (card_data >> (63 - card_data_index)) & 1;

    card_data_index++;
    if(card_data_index >= 64) {
        card_data_index = 0;
    }
}
