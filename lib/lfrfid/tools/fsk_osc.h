#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FSKOsc FSKOsc;

/**
 * @brief Allocate a new FSKOsc instance
 * FSKOsc is a oscillator that can provide FSK encoding
 * 
 * @param freq_low 
 * @param freq_hi 
 * @param osc_phase_max 
 * @return FSKOsc* 
 */
FSKOsc* fsk_osc_alloc(uint32_t freq_low, uint32_t freq_hi, uint32_t osc_phase_max);

/**
 * @brief Free a FSKOsc instance
 * 
 * @param osc 
 */
void fsk_osc_free(FSKOsc* osc);

/**
 * @brief Reset ocillator
 * 
 * @param osc 
 */
void fsk_osc_reset(FSKOsc* osc);

/**
 * @brief Get next duration sample from oscillator
 * 
 * @param osc 
 * @param bit 
 * @param period 
 * @return bool 
 */
bool fsk_osc_next(FSKOsc* osc, bool bit, uint32_t* period);

/**
 * @brief Get next half of sample from oscillator
 * Useful when encoding high and low levels separately.
 * 
 * @param osc 
 * @param bit 
 * @param level 
 * @param duration 
 * @return bool 
 */
bool fsk_osc_next_half(FSKOsc* osc, bool bit, bool* level, uint32_t* duration);

#ifdef __cplusplus
}
#endif
