#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "irda.h"


typedef struct {
    uint32_t bit1_mark;
    uint32_t bit1_space;
    uint32_t bit0_mark;
    uint32_t bit0_space;
    float duty_cycle;
    uint32_t carrier_frequency;
} IrdaEncoderTimings;


void irda_encode_byte(const IrdaEncoderTimings *timings, uint8_t data);
void irda_encode_bit(const IrdaEncoderTimings *timings, bool bit);
void irda_encode_space(const IrdaEncoderTimings *timings, uint32_t duration);
void irda_encode_mark(const IrdaEncoderTimings *timings, uint32_t duration);

