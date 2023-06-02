#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void wav_player_speaker_init();

void wav_player_speaker_start();

void wav_player_speaker_stop();

void wav_player_dma_init(uint32_t address, size_t size);

void wav_player_dma_start();

void wav_player_dma_stop();

void wav_player_hal_deinit();

#ifdef __cplusplus
}
#endif