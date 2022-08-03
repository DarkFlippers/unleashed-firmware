#include "protocol_metakom.h"
#include <stdlib.h>
#include <string.h>
#include <core/check.h>

#define METAKOM_DATA_SIZE 4
#define METAKOM_PERIOD_SAMPLE_COUNT 10

typedef enum {
    METAKOM_WAIT_PERIOD_SYNC,
    METAKOM_WAIT_START_BIT,
    METAKOM_WAIT_START_WORD,
    METAKOM_READ_WORD,
    METAKOM_READ_STOP_WORD,
} MetakomState;

typedef enum {
    METAKOM_BIT_WAIT_FRONT_HIGH,
    METAKOM_BIT_WAIT_FRONT_LOW,
} MetakomBitState;

struct ProtocolMetakom {
    PulseProtocol* protocol;

    // high + low period time
    uint32_t period_time;
    uint32_t low_time_storage;
    uint8_t period_sample_index;
    uint32_t period_sample_data[METAKOM_PERIOD_SAMPLE_COUNT];

    // ready flag
    // TODO: atomic access
    bool ready;

    uint8_t tmp_data;
    uint8_t tmp_counter;

    uint32_t key_data;
    uint8_t key_data_index;

    MetakomBitState bit_state;
    MetakomState state;
};

static void metakom_pulse(void* context, bool polarity, uint32_t length);
static void metakom_reset(void* context);
static void metakom_get_data(void* context, uint8_t* data, size_t length);
static bool metakom_decoded(void* context);

ProtocolMetakom* protocol_metakom_alloc() {
    ProtocolMetakom* metakom = malloc(sizeof(ProtocolMetakom));
    metakom_reset(metakom);

    metakom->protocol = pulse_protocol_alloc();

    pulse_protocol_set_context(metakom->protocol, metakom);
    pulse_protocol_set_pulse_cb(metakom->protocol, metakom_pulse);
    pulse_protocol_set_reset_cb(metakom->protocol, metakom_reset);
    pulse_protocol_set_get_data_cb(metakom->protocol, metakom_get_data);
    pulse_protocol_set_decoded_cb(metakom->protocol, metakom_decoded);

    return metakom;
}

void protocol_metakom_free(ProtocolMetakom* metakom) {
    furi_assert(metakom);
    pulse_protocol_free(metakom->protocol);
    free(metakom);
}

PulseProtocol* protocol_metakom_get_protocol(ProtocolMetakom* metakom) {
    furi_assert(metakom);
    return metakom->protocol;
}

static void metakom_get_data(void* context, uint8_t* data, size_t length) {
    furi_assert(context);
    furi_check(length >= METAKOM_DATA_SIZE);
    ProtocolMetakom* metakom = context;
    memcpy(data, &metakom->key_data, METAKOM_DATA_SIZE);
}

static bool metakom_decoded(void* context) {
    furi_assert(context);
    ProtocolMetakom* metakom = context;
    bool decoded = metakom->ready;
    return decoded;
}

static void metakom_reset(void* context) {
    furi_assert(context);
    ProtocolMetakom* metakom = context;

    metakom->ready = false;
    metakom->period_sample_index = 0;
    metakom->period_time = 0;
    metakom->tmp_counter = 0;
    metakom->tmp_data = 0;
    for(uint8_t i = 0; i < METAKOM_PERIOD_SAMPLE_COUNT; i++) {
        metakom->period_sample_data[i] = 0;
    };
    metakom->state = METAKOM_WAIT_PERIOD_SYNC;
    metakom->bit_state = METAKOM_BIT_WAIT_FRONT_LOW;
    metakom->key_data = 0;
    metakom->key_data_index = 0;
    metakom->low_time_storage = 0;
}

static bool metakom_parity_check(uint8_t data) {
    uint8_t ones_count = 0;
    bool result;

    for(uint8_t i = 0; i < 8; i++) {
        if((data >> i) & 0b00000001) {
            ones_count++;
        }
    }

    result = (ones_count % 2 == 0);

    return result;
}

static bool metakom_process_bit(
    ProtocolMetakom* metakom,
    bool polarity,
    uint32_t time,
    uint32_t* high_time,
    uint32_t* low_time) {
    bool result = false;

    switch(metakom->bit_state) {
    case METAKOM_BIT_WAIT_FRONT_LOW:
        if(polarity == false) {
            *low_time = metakom->low_time_storage;
            *high_time = time;
            result = true;
            metakom->bit_state = METAKOM_BIT_WAIT_FRONT_HIGH;
        }
        break;
    case METAKOM_BIT_WAIT_FRONT_HIGH:
        if(polarity == true) {
            metakom->low_time_storage = time;
            metakom->bit_state = METAKOM_BIT_WAIT_FRONT_LOW;
        }
        break;
    }

    return result;
}

static void metakom_pulse(void* context, bool polarity, uint32_t time) {
    furi_assert(context);
    ProtocolMetakom* metakom = context;

    if(metakom->ready) return;

    uint32_t high_time = 0;
    uint32_t low_time = 0;

    switch(metakom->state) {
    case METAKOM_WAIT_PERIOD_SYNC:
        if(metakom_process_bit(metakom, polarity, time, &high_time, &low_time)) {
            metakom->period_sample_data[metakom->period_sample_index] = high_time + low_time;
            metakom->period_sample_index++;

            if(metakom->period_sample_index == METAKOM_PERIOD_SAMPLE_COUNT) {
                for(uint8_t i = 0; i < METAKOM_PERIOD_SAMPLE_COUNT; i++) {
                    metakom->period_time += metakom->period_sample_data[i];
                };
                metakom->period_time /= METAKOM_PERIOD_SAMPLE_COUNT;

                metakom->state = METAKOM_WAIT_START_BIT;
            }
        }

        break;
    case METAKOM_WAIT_START_BIT:
        if(metakom_process_bit(metakom, polarity, time, &high_time, &low_time)) {
            metakom->tmp_counter++;
            if(high_time > metakom->period_time) {
                metakom->tmp_counter = 0;
                metakom->state = METAKOM_WAIT_START_WORD;
            }

            if(metakom->tmp_counter > 40) {
                metakom_reset(metakom);
            }
        }

        break;
    case METAKOM_WAIT_START_WORD:
        if(metakom_process_bit(metakom, polarity, time, &high_time, &low_time)) {
            if(low_time < (metakom->period_time / 2)) {
                metakom->tmp_data = (metakom->tmp_data << 1) | 0b0;
            } else {
                metakom->tmp_data = (metakom->tmp_data << 1) | 0b1;
            }
            metakom->tmp_counter++;

            if(metakom->tmp_counter == 3) {
                if(metakom->tmp_data == 0b010) {
                    metakom->tmp_counter = 0;
                    metakom->tmp_data = 0;
                    metakom->state = METAKOM_READ_WORD;
                } else {
                    metakom_reset(metakom);
                }
            }
        }
        break;
    case METAKOM_READ_WORD:
        if(metakom_process_bit(metakom, polarity, time, &high_time, &low_time)) {
            if(low_time < (metakom->period_time / 2)) {
                metakom->tmp_data = (metakom->tmp_data << 1) | 0b0;
            } else {
                metakom->tmp_data = (metakom->tmp_data << 1) | 0b1;
            }
            metakom->tmp_counter++;

            if(metakom->tmp_counter == 8) {
                if(metakom_parity_check(metakom->tmp_data)) {
                    metakom->key_data = (metakom->key_data << 8) | metakom->tmp_data;
                    metakom->key_data_index++;
                    metakom->tmp_data = 0;
                    metakom->tmp_counter = 0;

                    if(metakom->key_data_index == 4) {
                        // check for stop bit
                        if(high_time > metakom->period_time) {
                            metakom->state = METAKOM_READ_STOP_WORD;
                        } else {
                            metakom_reset(metakom);
                        }
                    }
                } else {
                    metakom_reset(metakom);
                }
            }
        }
        break;
    case METAKOM_READ_STOP_WORD:
        if(metakom_process_bit(metakom, polarity, time, &high_time, &low_time)) {
            if(low_time < (metakom->period_time / 2)) {
                metakom->tmp_data = (metakom->tmp_data << 1) | 0b0;
            } else {
                metakom->tmp_data = (metakom->tmp_data << 1) | 0b1;
            }
            metakom->tmp_counter++;

            if(metakom->tmp_counter == 3) {
                if(metakom->tmp_data == 0b010) {
                    metakom->ready = true;
                } else {
                    metakom_reset(metakom);
                }
            }
        }
        break;
    }
}
