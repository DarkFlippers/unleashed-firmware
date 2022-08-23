#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FSKDemod FSKDemod;

/**
 * @brief Allocate a new FSKDemod instance
 * FSKDemod is a demodulator that can decode FSK encoded data
 * 
 * @param low_time time between rising edges for the 0 bit
 * @param low_pulses rising edges count for the 0 bit
 * @param hi_time time between rising edges for the 1 bit
 * @param hi_pulses rising edges count for the 1 bit
 * @return FSKDemod* 
 */
FSKDemod*
    fsk_demod_alloc(uint32_t low_time, uint32_t low_pulses, uint32_t hi_time, uint32_t hi_pulses);

/**
 * @brief Free a FSKDemod instance
 * 
 * @param fsk_demod 
 */
void fsk_demod_free(FSKDemod* fsk_demod);

/**
 * @brief Feed sample to demodulator
 * 
 * @param demod FSKDemod instance
 * @param polarity sample polarity
 * @param time sample time
 * @param value demodulated bit value
 * @param count demodulated bit count
 */
void fsk_demod_feed(FSKDemod* demod, bool polarity, uint32_t time, bool* value, uint32_t* count);

#ifdef __cplusplus
}
#endif
