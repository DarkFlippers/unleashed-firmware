#include <furi.h>
#include <furi_hal.h>

#include "protocol_cyfral.h"

#define CYFRAL_DATA_SIZE sizeof(uint16_t)
#define CYFRAL_PERIOD (125 * furi_hal_cortex_instructions_per_microsecond())
#define CYFRAL_0_LOW (CYFRAL_PERIOD * 0.66f)
#define CYFRAL_0_HI (CYFRAL_PERIOD * 0.33f)
#define CYFRAL_1_LOW (CYFRAL_PERIOD * 0.33f)
#define CYFRAL_1_HI (CYFRAL_PERIOD * 0.66f)

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

typedef struct {
    CyfralState state;
    CyfralBitState bit_state;

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
} ProtocolCyfralDecoder;

typedef struct {
    uint32_t data;
    uint32_t index;
} ProtocolCyfralEncoder;

typedef struct {
    uint16_t data;

    ProtocolCyfralDecoder decoder;
    ProtocolCyfralEncoder encoder;
} ProtocolCyfral;

static void* protocol_cyfral_alloc(void) {
    ProtocolCyfral* proto = malloc(sizeof(ProtocolCyfral));
    return (void*)proto;
}

static void protocol_cyfral_free(ProtocolCyfral* proto) {
    free(proto);
}

static uint8_t* protocol_cyfral_get_data(ProtocolCyfral* proto) {
    return (uint8_t*)&proto->data;
}

static void protocol_cyfral_decoder_start(ProtocolCyfral* proto) {
    ProtocolCyfralDecoder* cyfral = &proto->decoder;

    cyfral->state = CYFRAL_WAIT_START_NIBBLE;
    cyfral->bit_state = CYFRAL_BIT_WAIT_FRONT_LOW;
    cyfral->period_time = 0;
    cyfral->bit_index = 0;
    cyfral->index = 0;
    cyfral->nibble = 0;
    cyfral->data_valid = true;
    cyfral->max_period = CYFRAL_MAX_PERIOD_US * furi_hal_cortex_instructions_per_microsecond();

    proto->data = 0;
}

static bool protocol_cyfral_decoder_process_bit(
    ProtocolCyfralDecoder* cyfral,
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

static bool protocol_cyfral_decoder_feed(ProtocolCyfral* proto, bool level, uint32_t duration) {
    ProtocolCyfralDecoder* cyfral = &proto->decoder;

    bool bit_ready;
    bool bit_value;
    bool decoded = false;

    switch(cyfral->state) {
    case CYFRAL_WAIT_START_NIBBLE:
        // wait for start word
        if(protocol_cyfral_decoder_process_bit(cyfral, level, duration, &bit_ready, &bit_value)) {
            if(bit_ready) {
                cyfral->nibble = ((cyfral->nibble << 1) | bit_value) & 0x0F;
                if(cyfral->nibble == 0b0001) {
                    cyfral->nibble = 0;
                    cyfral->state = CYFRAL_READ_NIBBLE;
                }
            }
        } else {
            protocol_cyfral_decoder_start(proto);
        }

        break;
    case CYFRAL_READ_NIBBLE:
        // read nibbles
        if(protocol_cyfral_decoder_process_bit(cyfral, level, duration, &bit_ready, &bit_value)) {
            if(bit_ready) {
                cyfral->nibble = (cyfral->nibble << 1) | bit_value;

                cyfral->bit_index++;

                //convert every nibble to 2-bit index
                if(cyfral->bit_index == 4) {
                    switch(cyfral->nibble) {
                    case 0b1110:
                        proto->data = (proto->data << 2) | 0b11;
                        break;
                    case 0b1101:
                        proto->data = (proto->data << 2) | 0b10;
                        break;
                    case 0b1011:
                        proto->data = (proto->data << 2) | 0b01;
                        break;
                    case 0b0111:
                        proto->data = (proto->data << 2) | 0b00;
                        break;
                    default:
                        cyfral->data_valid = false;
                        break;
                    }

                    cyfral->nibble = 0;
                    cyfral->bit_index = 0;
                    cyfral->index++;
                }

                // successfully read 8 nibbles
                if(cyfral->index == 8) {
                    cyfral->state = CYFRAL_READ_STOP_NIBBLE;
                }
            }
        } else {
            protocol_cyfral_decoder_start(proto);
        }
        break;
    case CYFRAL_READ_STOP_NIBBLE:
        // read stop nibble
        if(protocol_cyfral_decoder_process_bit(cyfral, level, duration, &bit_ready, &bit_value)) {
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
                            decoded = true;
                        } else {
                            protocol_cyfral_decoder_start(proto);
                        }
                    } else {
                        protocol_cyfral_decoder_start(proto);
                    }
                    break;
                default:
                    protocol_cyfral_decoder_start(proto);
                    break;
                }
            }
        } else {
            protocol_cyfral_decoder_start(proto);
        }
        break;
    }

    return decoded;
}

static uint32_t protocol_cyfral_encoder_encode(const uint16_t data) {
    uint32_t value = 0;
    for(int8_t i = 0; i <= 7; i++) {
        switch((data >> (i * 2)) & 0b00000011) {
        case 0b11:
            value = value << 4;
            value += 0b00000111;
            break;
        case 0b10:
            value = value << 4;
            value += 0b00001011;
            break;
        case 0b01:
            value = value << 4;
            value += 0b00001101;
            break;
        case 0b00:
            value = value << 4;
            value += 0b00001110;
            break;
        default:
            break;
        }
    }

    return value;
}

static bool protocol_cyfral_encoder_start(ProtocolCyfral* proto) {
    proto->encoder.index = 0;
    proto->encoder.data = protocol_cyfral_encoder_encode(proto->data);
    return true;
}

static LevelDuration protocol_cyfral_encoder_yield(ProtocolCyfral* proto) {
    LevelDuration result;

    if(proto->encoder.index < 8) {
        // start word (0b0001)
        switch(proto->encoder.index) {
        case 0:
            result = level_duration_make(false, CYFRAL_0_LOW); //-V1037
            break;
        case 1:
            result = level_duration_make(true, CYFRAL_0_HI); //-V1037
            break;
        case 2:
            result = level_duration_make(false, CYFRAL_0_LOW);
            break;
        case 3:
            result = level_duration_make(true, CYFRAL_0_HI);
            break;
        case 4:
            result = level_duration_make(false, CYFRAL_0_LOW);
            break;
        case 5:
            result = level_duration_make(true, CYFRAL_0_HI);
            break;
        case 6:
            result = level_duration_make(false, CYFRAL_1_LOW);
            break;
        case 7:
            result = level_duration_make(true, CYFRAL_1_HI);
            break;
        }
    } else {
        // data
        uint8_t data_start_index = proto->encoder.index - 8;
        bool clock_polarity = (data_start_index) % 2;
        uint8_t bit_index = (data_start_index) / 2;
        bool bit_value = ((proto->encoder.data >> bit_index) & 1);

        if(!clock_polarity) {
            if(bit_value) {
                result = level_duration_make(false, CYFRAL_1_LOW);
            } else {
                result = level_duration_make(false, CYFRAL_0_LOW);
            }
        } else {
            if(bit_value) {
                result = level_duration_make(true, CYFRAL_1_HI);
            } else {
                result = level_duration_make(true, CYFRAL_0_HI);
            }
        }
    }

    proto->encoder.index++;
    if(proto->encoder.index >= (9 * 4 * 2)) {
        proto->encoder.index = 0;
    }

    return result;
}

static void protocol_cyfral_render_brief_data(ProtocolCyfral* proto, FuriString* result) {
    for(size_t i = 0; i < CYFRAL_DATA_SIZE; ++i) {
        furi_string_cat_printf(result, "%02X ", ((uint8_t*)&proto->data)[i]);
    }
}

const ProtocolBase ibutton_protocol_misc_cyfral = {
    .name = "Cyfral",
    .manufacturer = "Cyfral",
    .data_size = CYFRAL_DATA_SIZE,
    .alloc = (ProtocolAlloc)protocol_cyfral_alloc,
    .free = (ProtocolFree)protocol_cyfral_free,
    .get_data = (ProtocolGetData)protocol_cyfral_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_cyfral_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_cyfral_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_cyfral_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_cyfral_encoder_yield,
        },
    .render_brief_data = (ProtocolRenderData)protocol_cyfral_render_brief_data,
};
