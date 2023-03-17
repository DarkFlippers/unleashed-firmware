#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define DTMF_DOLPHIN_MAX_TONE_COUNT 16

typedef enum {
    DTMF_DOLPHIN_TONE_BLOCK_DIALER,
    DTMF_DOLPHIN_TONE_BLOCK_BLUEBOX,
    DTMF_DOLPHIN_TONE_BLOCK_REDBOX_US,
    DTMF_DOLPHIN_TONE_BLOCK_REDBOX_UK,
    DTMF_DOLPHIN_TONE_BLOCK_REDBOX_CA,
    DTMF_DOLPHIN_TONE_BLOCK_MISC,
} DTMFDolphinToneSection;

void dtmf_dolphin_data_set_current_section(DTMFDolphinToneSection section);

DTMFDolphinToneSection dtmf_dolphin_data_get_current_section();

bool dtmf_dolphin_data_get_tone_frequencies(float* freq1, float* freq2, uint8_t row, uint8_t col);

bool dtmf_dolphin_data_get_filter_data(
    uint16_t* pulses,
    uint16_t* pulse_ms,
    uint16_t* gap_ms,
    uint8_t row,
    uint8_t col);

const char* dtmf_dolphin_data_get_tone_name(uint8_t row, uint8_t col);

const char* dtmf_dolphin_data_get_current_section_name();

void dtmf_dolphin_tone_get_max_pos(uint8_t* max_rows, uint8_t* max_cols, uint8_t* max_span);

uint8_t dtmf_dolphin_get_tone_span(uint8_t row, uint8_t col);