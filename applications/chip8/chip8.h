#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <m-string.h>
#include <storage/storage.h>
#include "emulator_core/flipper_chip.h"

#define CHIP8_SCREEN_W 64
#define CHIP8_SCREEN_H 32

typedef struct Chip8Emulator Chip8Emulator;

typedef enum {
    WorkerStateLoadingRom,
    WorkerStateRomLoaded,
    WorkerStateRomLoadError,
    WorkerStateBackPressed,
} WorkerState;

typedef struct {
    WorkerState worker_state;
    t_chip8_state* t_chip8_state;
} Chip8State;

Chip8Emulator* chip8_make_emulator(string_t file_path);

void chip8_close_emulator(Chip8Emulator* chip8);
void chip8_set_back_pressed(Chip8Emulator* chip8);
void chip8_set_up_pressed(Chip8Emulator* chip8);
void chip8_set_down_pressed(Chip8Emulator* chip8);
void chip8_release_keyboard(Chip8Emulator* chip8);

Chip8State* chip8_get_state(Chip8Emulator* chip8);

void chip8_toggle(Chip8Emulator* chip8);

uint16_t read_rom_data(File* file, uint8_t* data);

#ifdef __cplusplus
}
#endif
