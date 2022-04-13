#include "encoder_cyfral.h"
#include <furi_hal.h>

#define CYFRAL_DATA_SIZE sizeof(uint16_t)
#define CYFRAL_PERIOD (125 * furi_hal_delay_instructions_per_microsecond())
#define CYFRAL_0_LOW (CYFRAL_PERIOD * 0.66f)
#define CYFRAL_0_HI (CYFRAL_PERIOD * 0.33f)
#define CYFRAL_1_LOW (CYFRAL_PERIOD * 0.33f)
#define CYFRAL_1_HI (CYFRAL_PERIOD * 0.66f)

#define CYFRAL_SET_DATA(level, len) \
    *polarity = level;              \
    *length = len;

struct EncoderCyfral {
    uint32_t data;
    uint32_t index;
};

EncoderCyfral* encoder_cyfral_alloc() {
    EncoderCyfral* cyfral = malloc(sizeof(EncoderCyfral));
    encoder_cyfral_reset(cyfral);
    return cyfral;
}

void encoder_cyfral_free(EncoderCyfral* cyfral) {
    free(cyfral);
}

void encoder_cyfral_reset(EncoderCyfral* cyfral) {
    cyfral->data = 0;
    cyfral->index = 0;
}

uint32_t cyfral_encoder_encode(const uint16_t data) {
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

void encoder_cyfral_set_data(EncoderCyfral* cyfral, const uint8_t* data, size_t data_size) {
    furi_assert(cyfral);
    furi_check(data_size >= CYFRAL_DATA_SIZE);
    uint16_t intermediate;
    memcpy(&intermediate, data, CYFRAL_DATA_SIZE);
    cyfral->data = cyfral_encoder_encode(intermediate);
}

void encoder_cyfral_get_pulse(EncoderCyfral* cyfral, bool* polarity, uint32_t* length) {
    if(cyfral->index < 8) {
        // start word (0b0001)
        switch(cyfral->index) {
        case 0:
            CYFRAL_SET_DATA(false, CYFRAL_0_LOW);
            break;
        case 1:
            CYFRAL_SET_DATA(true, CYFRAL_0_HI);
            break;
        case 2:
            CYFRAL_SET_DATA(false, CYFRAL_0_LOW);
            break;
        case 3:
            CYFRAL_SET_DATA(true, CYFRAL_0_HI);
            break;
        case 4:
            CYFRAL_SET_DATA(false, CYFRAL_0_LOW);
            break;
        case 5:
            CYFRAL_SET_DATA(true, CYFRAL_0_HI);
            break;
        case 6:
            CYFRAL_SET_DATA(false, CYFRAL_1_LOW);
            break;
        case 7:
            CYFRAL_SET_DATA(true, CYFRAL_1_HI);
            break;
        }
    } else {
        // data
        uint8_t data_start_index = cyfral->index - 8;
        bool clock_polarity = (data_start_index) % 2;
        uint8_t bit_index = (data_start_index) / 2;
        bool bit_value = ((cyfral->data >> bit_index) & 1);

        if(!clock_polarity) {
            if(bit_value) {
                CYFRAL_SET_DATA(false, CYFRAL_1_LOW);
            } else {
                CYFRAL_SET_DATA(false, CYFRAL_0_LOW);
            }
        } else {
            if(bit_value) {
                CYFRAL_SET_DATA(true, CYFRAL_1_HI);
            } else {
                CYFRAL_SET_DATA(true, CYFRAL_0_HI);
            }
        }
    }

    cyfral->index++;
    if(cyfral->index >= (9 * 4 * 2)) {
        cyfral->index = 0;
    }
}
