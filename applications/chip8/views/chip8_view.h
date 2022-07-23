#pragma once

#include <gui/view.h>
#include "../chip8.h"

typedef struct Chip8View Chip8View;
typedef void (*Chip8ViewCallback)(InputType type, void* context);
typedef void (*Chip8ViewKeyBackCallback)(Chip8View* view, InputType type, void* context);
typedef void (*Chip8ViewKeyUpCallback)(InputType type, void* context);
typedef void (*Chip8ViewKeyDownCallback)(InputType type, void* context);
typedef void (*Chip8ViewReleaseCallback)(InputType type, void* context);

Chip8View* chip8_alloc();

void chip8_free(Chip8View* chip8);

View* chip8_get_view(Chip8View* chip8);

void chip8_set_ok_callback(Chip8View* chip8, Chip8ViewCallback callback, void* context);
void chip8_set_back_callback(Chip8View* chip8, Chip8ViewKeyBackCallback callback, void* context);
void chip8_set_up_callback(Chip8View* chip8, Chip8ViewKeyUpCallback callback, void* context);
void chip8_set_down_callback(Chip8View* chip8, Chip8ViewKeyDownCallback callback, void* context);
void chip8_set_release_callback(Chip8View* chip8, Chip8ViewReleaseCallback callback, void* context);

void chip8_set_backup_screen(Chip8View* chip8, uint8_t** screen);

void chip8_set_file_name(Chip8View* chip8, string_t name);

void chip8_set_state(Chip8View* chip8, Chip8State* st);
