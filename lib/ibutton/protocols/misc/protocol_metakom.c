#include <furi.h>
#include <furi_hal.h>

#include "protocol_metakom.h"

#define METAKOM_DATA_SIZE sizeof(uint32_t)
#define METAKOM_PERIOD    (125 * furi_hal_cortex_instructions_per_microsecond())
#define METAKOM_0_LOW     (METAKOM_PERIOD * 0.33f)
#define METAKOM_0_HI      (METAKOM_PERIOD * 0.66f)
#define METAKOM_1_LOW     (METAKOM_PERIOD * 0.66f)
#define METAKOM_1_HI      (METAKOM_PERIOD * 0.33f)

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

typedef struct {
    // high + low period time
    uint32_t period_time;
    uint32_t low_time_storage;
    uint8_t period_sample_index;
    uint32_t period_sample_data[METAKOM_PERIOD_SAMPLE_COUNT];

    uint8_t tmp_data;
    uint8_t tmp_counter;

    uint8_t key_data_index;

    MetakomBitState bit_state;
    MetakomState state;
} ProtocolMetakomDecoder;

typedef struct {
    uint32_t index;
} ProtocolMetakomEncoder;

typedef struct {
    uint32_t data;

    ProtocolMetakomDecoder decoder;
    ProtocolMetakomEncoder encoder;
} ProtocolMetakom;

static ProtocolMetakom* protocol_metakom_alloc(void) {
    ProtocolMetakom* proto = malloc(sizeof(ProtocolMetakom));
    return (void*)proto;
}

static void protocol_metakom_free(ProtocolMetakom* proto) {
    free(proto);
}

static uint8_t* protocol_metakom_get_data(ProtocolMetakom* proto) {
    return (uint8_t*)&proto->data;
}

static void protocol_metakom_decoder_start(ProtocolMetakom* proto) {
    ProtocolMetakomDecoder* metakom = &proto->decoder;

    metakom->period_sample_index = 0;
    metakom->period_time = 0;
    metakom->tmp_counter = 0;
    metakom->tmp_data = 0;
    for(uint8_t i = 0; i < METAKOM_PERIOD_SAMPLE_COUNT; i++) {
        metakom->period_sample_data[i] = 0;
    };
    metakom->state = METAKOM_WAIT_PERIOD_SYNC;
    metakom->bit_state = METAKOM_BIT_WAIT_FRONT_LOW;
    metakom->key_data_index = 0;
    metakom->low_time_storage = 0;

    proto->data = 0;
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
    ProtocolMetakomDecoder* metakom,
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

static bool protocol_metakom_decoder_feed(ProtocolMetakom* proto, bool level, uint32_t duration) {
    ProtocolMetakomDecoder* metakom = &proto->decoder;

    bool ready = false;

    uint32_t high_time = 0;
    uint32_t low_time = 0;

    switch(metakom->state) {
    case METAKOM_WAIT_PERIOD_SYNC:
        if(metakom_process_bit(metakom, level, duration, &high_time, &low_time)) {
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
        if(metakom_process_bit(metakom, level, duration, &high_time, &low_time)) {
            metakom->tmp_counter++;
            if(high_time > metakom->period_time) {
                metakom->tmp_counter = 0;
                metakom->state = METAKOM_WAIT_START_WORD;
            }

            if(metakom->tmp_counter > 40) {
                protocol_metakom_decoder_start(proto);
            }
        }

        break;
    case METAKOM_WAIT_START_WORD:
        if(metakom_process_bit(metakom, level, duration, &high_time, &low_time)) {
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
                    protocol_metakom_decoder_start(proto);
                }
            }
        }
        break;
    case METAKOM_READ_WORD:
        if(metakom_process_bit(metakom, level, duration, &high_time, &low_time)) {
            if(low_time < (metakom->period_time / 2)) {
                metakom->tmp_data = (metakom->tmp_data << 1) | 0b0;
            } else {
                metakom->tmp_data = (metakom->tmp_data << 1) | 0b1;
            }
            metakom->tmp_counter++;

            if(metakom->tmp_counter == 8) {
                if(metakom_parity_check(metakom->tmp_data)) {
                    proto->data = (proto->data << 8) | metakom->tmp_data;
                    metakom->key_data_index++;
                    metakom->tmp_data = 0;
                    metakom->tmp_counter = 0;

                    if(metakom->key_data_index == 4) {
                        // check for stop bit
                        if(high_time > metakom->period_time) {
                            metakom->state = METAKOM_READ_STOP_WORD;
                        } else {
                            protocol_metakom_decoder_start(proto);
                        }
                    }
                } else {
                    protocol_metakom_decoder_start(proto);
                }
            }
        }
        break;
    case METAKOM_READ_STOP_WORD:
        if(metakom_process_bit(metakom, level, duration, &high_time, &low_time)) {
            if(low_time < (metakom->period_time / 2)) {
                metakom->tmp_data = (metakom->tmp_data << 1) | 0b0;
            } else {
                metakom->tmp_data = (metakom->tmp_data << 1) | 0b1;
            }
            metakom->tmp_counter++;

            if(metakom->tmp_counter == 3) {
                if(metakom->tmp_data == 0b010) {
                    ready = true;
                } else {
                    protocol_metakom_decoder_start(proto);
                }
            }
        }
        break;
    }

    return ready;
}

static bool protocol_metakom_encoder_start(ProtocolMetakom* proto) {
    proto->encoder.index = 0;
    return true;
}

static LevelDuration protocol_metakom_encoder_yield(ProtocolMetakom* proto) {
    LevelDuration result;

    if(proto->encoder.index == 0) {
        // sync bit
        result = level_duration_make(false, METAKOM_PERIOD);
    } else if(proto->encoder.index <= 6) {
        // start word (0b010)
        switch(proto->encoder.index) {
        case 1:
            result = level_duration_make(true, METAKOM_0_LOW); //-V1037
            break;
        case 2:
            result = level_duration_make(false, METAKOM_0_HI); //-V1037
            break;
        case 3:
            result = level_duration_make(true, METAKOM_1_LOW);
            break;
        case 4:
            result = level_duration_make(false, METAKOM_1_HI);
            break;
        case 5:
            result = level_duration_make(true, METAKOM_0_LOW);
            break;
        case 6:
            result = level_duration_make(false, METAKOM_0_HI);
            break;
        }
    } else {
        // data
        uint8_t data_start_index = proto->encoder.index - 7;
        bool clock_polarity = (data_start_index) % 2;
        uint8_t bit_index = (data_start_index) / 2;
        bool bit_value = (proto->data >> (32 - 1 - bit_index)) & 1;

        if(!clock_polarity) {
            if(bit_value) {
                result = level_duration_make(true, METAKOM_1_LOW);
            } else {
                result = level_duration_make(true, METAKOM_0_LOW);
            }
        } else {
            if(bit_value) {
                result = level_duration_make(false, METAKOM_1_HI);
            } else {
                result = level_duration_make(false, METAKOM_0_HI);
            }
        }
    }

    proto->encoder.index++;
    if(proto->encoder.index >= (1 + 3 * 2 + 32 * 2)) {
        proto->encoder.index = 0;
    }

    return result;
}

static void protocol_metakom_render_uid(ProtocolMetakom* proto, FuriString* result) {
    furi_string_cat_printf(result, "ID: ");
    for(size_t i = 0; i < METAKOM_DATA_SIZE; ++i) {
        furi_string_cat_printf(result, "%02X ", ((uint8_t*)&proto->data)[i]);
    }
}

static void protocol_metakom_render_brief_data(ProtocolMetakom* proto, FuriString* result) {
    protocol_metakom_render_uid(proto, result);
}

const ProtocolBase ibutton_protocol_misc_metakom = {
    .name = "Metakom",
    .manufacturer = "Metakom",
    .data_size = METAKOM_DATA_SIZE,
    .alloc = (ProtocolAlloc)protocol_metakom_alloc,
    .free = (ProtocolFree)protocol_metakom_free,
    .get_data = (ProtocolGetData)protocol_metakom_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_metakom_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_metakom_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_metakom_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_metakom_encoder_yield,
        },
    .render_uid = (ProtocolRenderData)protocol_metakom_render_uid,
    .render_brief_data = (ProtocolRenderData)protocol_metakom_render_brief_data,
};
