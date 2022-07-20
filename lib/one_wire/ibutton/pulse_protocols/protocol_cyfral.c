#include "protocol_cyfral.h"
#include <stdlib.h>
#include <string.h>
#include <furi.h>
#include <furi_hal.h>

#define CYFRAL_DATA_SIZE 2
#define CYFRAL_MAX_PERIOD_US 230

typedef enum {
    CYFRAL_BIT_WAIT_FRONT_HIGH,
    CYFRAL_BIT_WAIT_FRONT_LOW,
} CyfralBitState;

typedef enum {
    CYFRAL_WAIT_START_NIBBLE,
    CYFRAL_READ_NIBBLE,
    CYFRAL_READ_STOP_NIBBLE,
} CyfralState;

struct ProtocolCyfral {
    PulseProtocol* protocol;

    CyfralState state;
    CyfralBitState bit_state;

    // ready flag, key is read and valid
    // TODO: atomic access
    bool ready;
    // key data storage
    uint16_t key_data;
    // high + low period time
    uint32_t period_time;
    // temporary nibble storage
    uint8_t nibble;
    // data valid flag
    // MUST be checked only in READ_STOP_NIBBLE state
    bool data_valid;
    // nibble index, we expect 8 nibbles
    uint8_t index;
    // bit index in nibble, 4 bit per nibble
    uint8_t bit_index;
    // max period, 230us x clock per us
    uint32_t max_period;
};

static void cyfral_pulse(void* context, bool polarity, uint32_t length);
static void cyfral_reset(void* context);
static void cyfral_get_data(void* context, uint8_t* data, size_t length);
static bool cyfral_decoded(void* context);

ProtocolCyfral* protocol_cyfral_alloc() {
    ProtocolCyfral* cyfral = malloc(sizeof(ProtocolCyfral));
    cyfral_reset(cyfral);

    cyfral->protocol = pulse_protocol_alloc();

    pulse_protocol_set_context(cyfral->protocol, cyfral);
    pulse_protocol_set_pulse_cb(cyfral->protocol, cyfral_pulse);
    pulse_protocol_set_reset_cb(cyfral->protocol, cyfral_reset);
    pulse_protocol_set_get_data_cb(cyfral->protocol, cyfral_get_data);
    pulse_protocol_set_decoded_cb(cyfral->protocol, cyfral_decoded);

    return cyfral;
}

void protocol_cyfral_free(ProtocolCyfral* cyfral) {
    furi_assert(cyfral);
    pulse_protocol_free(cyfral->protocol);
    free(cyfral);
}

PulseProtocol* protocol_cyfral_get_protocol(ProtocolCyfral* cyfral) {
    furi_assert(cyfral);
    return cyfral->protocol;
}

static void cyfral_get_data(void* context, uint8_t* data, size_t length) {
    furi_assert(context);
    furi_check(length >= CYFRAL_DATA_SIZE);
    ProtocolCyfral* cyfral = context;
    memcpy(data, &cyfral->key_data, CYFRAL_DATA_SIZE);
}

static bool cyfral_decoded(void* context) {
    furi_assert(context);
    ProtocolCyfral* cyfral = context;
    bool decoded = cyfral->ready;
    return decoded;
}

static void cyfral_reset(void* context) {
    furi_assert(context);
    ProtocolCyfral* cyfral = context;
    cyfral->state = CYFRAL_WAIT_START_NIBBLE;
    cyfral->bit_state = CYFRAL_BIT_WAIT_FRONT_LOW;

    cyfral->period_time = 0;
    cyfral->bit_index = 0;
    cyfral->ready = false;
    cyfral->index = 0;

    cyfral->key_data = 0;
    cyfral->nibble = 0;
    cyfral->data_valid = true;

    cyfral->max_period = CYFRAL_MAX_PERIOD_US * furi_hal_cortex_instructions_per_microsecond();
}

static bool cyfral_process_bit(
    ProtocolCyfral* cyfral,
    bool polarity,
    uint32_t length,
    bool* bit_ready,
    bool* bit_value) {
    bool result = true;
    *bit_ready = false;

    // bit start from low
    switch(cyfral->bit_state) {
    case CYFRAL_BIT_WAIT_FRONT_LOW:
        if(polarity == true) {
            cyfral->period_time += length;

            *bit_ready = true;
            if(cyfral->period_time <= cyfral->max_period) {
                if((cyfral->period_time / 2) > length) {
                    *bit_value = false;
                } else {
                    *bit_value = true;
                }
            } else {
                result = false;
            }

            cyfral->bit_state = CYFRAL_BIT_WAIT_FRONT_HIGH;
        } else {
            result = false;
        }
        break;
    case CYFRAL_BIT_WAIT_FRONT_HIGH:
        if(polarity == false) {
            cyfral->period_time = length;
            cyfral->bit_state = CYFRAL_BIT_WAIT_FRONT_LOW;
        } else {
            result = false;
        }
        break;
    }

    return result;
}

static void cyfral_pulse(void* context, bool polarity, uint32_t length) {
    furi_assert(context);
    ProtocolCyfral* cyfral = context;

    bool bit_ready;
    bool bit_value;

    if(cyfral->ready) return;

    switch(cyfral->state) {
    case CYFRAL_WAIT_START_NIBBLE:
        // wait for start word
        if(cyfral_process_bit(cyfral, polarity, length, &bit_ready, &bit_value)) {
            if(bit_ready) {
                cyfral->nibble = ((cyfral->nibble << 1) | bit_value) & 0x0F;
                if(cyfral->nibble == 0b0001) {
                    cyfral->nibble = 0;
                    cyfral->state = CYFRAL_READ_NIBBLE;
                }
            }
        } else {
            cyfral_reset(cyfral);
        }

        break;
    case CYFRAL_READ_NIBBLE:
        // read nibbles
        if(cyfral_process_bit(cyfral, polarity, length, &bit_ready, &bit_value)) {
            if(bit_ready) {
                cyfral->nibble = (cyfral->nibble << 1) | bit_value;

                cyfral->bit_index++;

                //convert every nibble to 2-bit index
                if(cyfral->bit_index == 4) {
                    switch(cyfral->nibble) {
                    case 0b1110:
                        cyfral->key_data = (cyfral->key_data << 2) | 0b11;
                        break;
                    case 0b1101:
                        cyfral->key_data = (cyfral->key_data << 2) | 0b10;
                        break;
                    case 0b1011:
                        cyfral->key_data = (cyfral->key_data << 2) | 0b01;
                        break;
                    case 0b0111:
                        cyfral->key_data = (cyfral->key_data << 2) | 0b00;
                        break;
                    default:
                        cyfral->data_valid = false;
                        break;
                    }

                    cyfral->nibble = 0;
                    cyfral->bit_index = 0;
                    cyfral->index++;
                }

                // succefully read 8 nibbles
                if(cyfral->index == 8) {
                    cyfral->state = CYFRAL_READ_STOP_NIBBLE;
                }
            }
        } else {
            cyfral_reset(cyfral);
        }
        break;
    case CYFRAL_READ_STOP_NIBBLE:
        // read stop nibble
        if(cyfral_process_bit(cyfral, polarity, length, &bit_ready, &bit_value)) {
            if(bit_ready) {
                cyfral->nibble = ((cyfral->nibble << 1) | bit_value) & 0x0F;
                cyfral->bit_index++;

                switch(cyfral->bit_index) {
                case 0:
                case 1:
                case 2:
                case 3:
                    break;
                case 4:
                    if(cyfral->nibble == 0b0001) {
                        // validate data
                        if(cyfral->data_valid) {
                            cyfral->ready = true;
                        } else {
                            cyfral_reset(cyfral);
                        }
                    } else {
                        cyfral_reset(cyfral);
                    }
                    break;
                default:
                    cyfral_reset(cyfral);
                    break;
                }
            }
        } else {
            cyfral_reset(cyfral);
        }
        break;
    }
}
