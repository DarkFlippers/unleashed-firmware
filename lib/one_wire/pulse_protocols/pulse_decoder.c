#include <stdlib.h>
#include "pulse_decoder.h"
#include <string.h>
#include <core/check.h>

#define MAX_PROTOCOL 5

struct PulseDecoder {
    PulseProtocol* protocols[MAX_PROTOCOL];
};

PulseDecoder* pulse_decoder_alloc() {
    PulseDecoder* decoder = malloc(sizeof(PulseDecoder));
    memset(decoder, 0, sizeof(PulseDecoder));
    return decoder;
}

void pulse_decoder_free(PulseDecoder* reader) {
    furi_assert(reader);
    free(reader);
}

void pulse_decoder_add_protocol(PulseDecoder* reader, PulseProtocol* protocol, int32_t index) {
    furi_check(index < MAX_PROTOCOL);
    furi_check(reader->protocols[index] == NULL);
    reader->protocols[index] = protocol;
}

void pulse_decoder_process_pulse(PulseDecoder* reader, bool polarity, uint32_t length) {
    furi_assert(reader);
    for(size_t index = 0; index < MAX_PROTOCOL; index++) {
        if(reader->protocols[index] != NULL) {
            pulse_protocol_process_pulse(reader->protocols[index], polarity, length);
        }
    }
}

int32_t pulse_decoder_get_decoded_index(PulseDecoder* reader) {
    furi_assert(reader);
    int32_t decoded = -1;
    for(size_t index = 0; index < MAX_PROTOCOL; index++) {
        if(reader->protocols[index] != NULL) {
            if(pulse_protocol_decoded(reader->protocols[index])) {
                decoded = index;
                break;
            }
        }
    }

    return decoded;
}

void pulse_decoder_reset(PulseDecoder* reader) {
    furi_assert(reader);
    for(size_t index = 0; index < MAX_PROTOCOL; index++) {
        if(reader->protocols[index] != NULL) {
            pulse_protocol_reset(reader->protocols[index]);
        }
    }
}

void pulse_decoder_get_data(PulseDecoder* reader, int32_t index, uint8_t* data, size_t length) {
    furi_assert(reader);
    furi_check(reader->protocols[index] != NULL);
    pulse_protocol_get_data(reader->protocols[index], data, length);
}
