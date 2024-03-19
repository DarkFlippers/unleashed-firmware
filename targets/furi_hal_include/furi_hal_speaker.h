/**
 * @file furi_hal_speaker.h
 * Speaker HAL
 */
#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Init speaker */
void furi_hal_speaker_init(void);

/** Deinit speaker */
void furi_hal_speaker_deinit(void);

/** Acquire speaker ownership
 *
 * @warning    You must acquire speaker ownership before use
 *
 * @param      timeout  Timeout during which speaker ownership must be acquired
 *
 * @return     bool  returns true on success
 */
FURI_WARN_UNUSED bool furi_hal_speaker_acquire(uint32_t timeout);

/** Release speaker ownership
 *
 * @warning    You must release speaker ownership after use
 */
void furi_hal_speaker_release(void);

/** Check current process speaker ownership
 *
 * @warning    always returns true if called from ISR
 *
 * @return     bool returns true if process owns speaker
 */
bool furi_hal_speaker_is_mine(void);

/** Play a note
 *
 * @warning    no ownership check if called from ISR
 *
 * @param      frequency  The frequency
 * @param      volume     The volume
 */
void furi_hal_speaker_start(float frequency, float volume);

/** Set volume
 *
 * @warning    no ownership check if called from ISR
 *
 * @param      volume  The volume
 */
void furi_hal_speaker_set_volume(float volume);

/** Stop playback
 *
 * @warning    no ownership check if called from ISR
 */
void furi_hal_speaker_stop(void);

#ifdef __cplusplus
}
#endif
