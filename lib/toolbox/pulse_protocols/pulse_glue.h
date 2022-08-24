/**
 * @file pulse_glue.h
 * 
 * Simple tool to glue separated pulses to corret 
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PulseGlue PulseGlue;

PulseGlue* pulse_glue_alloc();
void pulse_glue_free(PulseGlue* pulse_glue);
void pulse_glue_reset(PulseGlue* pulse_glue);

bool pulse_glue_push(PulseGlue* pulse_glue, bool polarity, uint32_t length);
void pulse_glue_pop(PulseGlue* pulse_glue, uint32_t* length, uint32_t* period);

#ifdef __cplusplus
}
#endif
