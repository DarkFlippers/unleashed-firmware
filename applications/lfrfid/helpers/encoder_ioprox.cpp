#include "encoder_ioprox.h"
#include "protocols/protocol_ioprox.h"
#include <furi.h>

void EncoderIoProx::init(const uint8_t* data, const uint8_t data_size) {
    ProtocolIoProx ioprox;
    ioprox.encode(data, data_size, card_data, sizeof(card_data));
    card_data_index = 0;
}

void EncoderIoProx::get_next(bool* polarity, uint16_t* period, uint16_t* pulse) {
    uint8_t bit = (card_data[card_data_index / 8] >> (7 - (card_data_index % 8))) & 1;

    bool advance = fsk->next(bit, period);
    if(advance) {
        card_data_index++;
        if(card_data_index >= (8 * card_data_max)) {
            card_data_index = 0;
        }
    }

    *polarity = true;
    *pulse = *period / 2;
}

EncoderIoProx::EncoderIoProx() {
    fsk = new OscFSK(8, 10, 64);
}

EncoderIoProx::~EncoderIoProx() {
    delete fsk;
}
