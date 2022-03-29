#include "pulse_protocol.h"
#include <stdlib.h>
#include <string.h>

struct PulseProtocol {
    void* context;
    PulseProtocolPulseCallback pulse_cb;
    PulseProtocolResetCallback reset_cb;
    PulseProtocolGetDataCallback get_data_cb;
    PulseProtocolDecodedCallback decoded_cb;
};

PulseProtocol* pulse_protocol_alloc() {
    PulseProtocol* protocol = malloc(sizeof(PulseProtocol));
    memset(protocol, 0, sizeof(PulseProtocol));
    return protocol;
}

void pulse_protocol_set_context(PulseProtocol* protocol, void* context) {
    protocol->context = context;
}

void pulse_protocol_set_pulse_cb(PulseProtocol* protocol, PulseProtocolPulseCallback callback) {
    protocol->pulse_cb = callback;
}

void pulse_protocol_set_reset_cb(PulseProtocol* protocol, PulseProtocolResetCallback callback) {
    protocol->reset_cb = callback;
}

void pulse_protocol_set_get_data_cb(PulseProtocol* protocol, PulseProtocolGetDataCallback callback) {
    protocol->get_data_cb = callback;
}

void pulse_protocol_set_decoded_cb(PulseProtocol* protocol, PulseProtocolDecodedCallback callback) {
    protocol->decoded_cb = callback;
}

void pulse_protocol_free(PulseProtocol* protocol) {
    free(protocol);
}

void pulse_protocol_process_pulse(PulseProtocol* protocol, bool polarity, uint32_t length) {
    if(protocol->pulse_cb != NULL) {
        protocol->pulse_cb(protocol->context, polarity, length);
    }
}

void pulse_protocol_reset(PulseProtocol* protocol) {
    if(protocol->reset_cb != NULL) {
        protocol->reset_cb(protocol->context);
    }
}

bool pulse_protocol_decoded(PulseProtocol* protocol) {
    bool result = false;
    if(protocol->decoded_cb != NULL) {
        result = protocol->decoded_cb(protocol->context);
    }
    return result;
}

void pulse_protocol_get_data(PulseProtocol* protocol, uint8_t* data, size_t length) {
    if(protocol->get_data_cb != NULL) {
        protocol->get_data_cb(protocol->context, data, length);
    }
}