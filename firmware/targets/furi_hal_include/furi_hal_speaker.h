/**
 * @file furi_hal_speaker.h
 * Speaker HAL
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void furi_hal_speaker_init();

void furi_hal_speaker_start(float frequency, float volume);

void furi_hal_speaker_stop();

#ifdef __cplusplus
}
#endif
