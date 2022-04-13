#include "encoder_metakom.h"
#include <furi_hal.h>

#define METAKOM_DATA_SIZE sizeof(uint32_t)
#define METAKOM_PERIOD (125 * furi_hal_delay_instructions_per_microsecond())
#define METAKOM_0_LOW (METAKOM_PERIOD * 0.33f)
#define METAKOM_0_HI (METAKOM_PERIOD * 0.66f)
#define METAKOM_1_LOW (METAKOM_PERIOD * 0.66f)
#define METAKOM_1_HI (METAKOM_PERIOD * 0.33f)

#define METAKOM_SET_DATA(level, len) \
    *polarity = !level;              \
    *length = len;

struct EncoderMetakom {
    uint32_t data;
    uint32_t index;
};

EncoderMetakom* encoder_metakom_alloc() {
    EncoderMetakom* metakom = malloc(sizeof(EncoderMetakom));
    encoder_metakom_reset(metakom);
    return metakom;
}

void encoder_metakom_free(EncoderMetakom* metakom) {
    free(metakom);
}

void encoder_metakom_reset(EncoderMetakom* metakom) {
    metakom->data = 0;
    metakom->index = 0;
}

void encoder_metakom_set_data(EncoderMetakom* metakom, const uint8_t* data, size_t data_size) {
    furi_assert(metakom);
    furi_check(data_size >= METAKOM_DATA_SIZE);
    memcpy(&metakom->data, data, METAKOM_DATA_SIZE);
}

void encoder_metakom_get_pulse(EncoderMetakom* metakom, bool* polarity, uint32_t* length) {
    if(metakom->index == 0) {
        // sync bit
        METAKOM_SET_DATA(true, METAKOM_PERIOD);
    } else if(metakom->index >= 1 && metakom->index <= 6) {
        // start word (0b010)
        switch(metakom->index) {
        case 1:
            METAKOM_SET_DATA(false, METAKOM_0_LOW);
            break;
        case 2:
            METAKOM_SET_DATA(true, METAKOM_0_HI);
            break;
        case 3:
            METAKOM_SET_DATA(false, METAKOM_1_LOW);
            break;
        case 4:
            METAKOM_SET_DATA(true, METAKOM_1_HI);
            break;
        case 5:
            METAKOM_SET_DATA(false, METAKOM_0_LOW);
            break;
        case 6:
            METAKOM_SET_DATA(true, METAKOM_0_HI);
            break;
        }
    } else {
        // data
        uint8_t data_start_index = metakom->index - 7;
        bool clock_polarity = (data_start_index) % 2;
        uint8_t bit_index = (data_start_index) / 2;
        bool bit_value = (metakom->data >> (32 - 1 - bit_index)) & 1;

        if(!clock_polarity) {
            if(bit_value) {
                METAKOM_SET_DATA(false, METAKOM_1_LOW);
            } else {
                METAKOM_SET_DATA(false, METAKOM_0_LOW);
            }
        } else {
            if(bit_value) {
                METAKOM_SET_DATA(true, METAKOM_1_HI);
            } else {
                METAKOM_SET_DATA(true, METAKOM_0_HI);
            }
        }
    }

    metakom->index++;
    if(metakom->index >= (1 + 3 * 2 + 32 * 2)) {
        metakom->index = 0;
    }
}
