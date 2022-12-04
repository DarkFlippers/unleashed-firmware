#pragma once

#include <input/input.h>
#include "tamalib/tamalib.h"

#define TAG "TamaP1"
#define TAMA_ROM_PATH EXT_PATH("tama_p1/rom.bin")
#define TAMA_SCREEN_SCALE_FACTOR 2
#define TAMA_LCD_ICON_SIZE 14
#define TAMA_LCD_ICON_MARGIN 1

typedef struct {
    FuriThread* thread;
    hal_t hal;
    uint8_t* rom;
    // 32x16 screen, perfectly represented through uint32_t
    uint32_t framebuffer[16];
    uint8_t icons;
    bool halted;
    bool fast_forward_done;
    bool buzzer_on;
    float frequency;
} TamaApp;

typedef enum {
    EventTypeInput,
    EventTypeTick,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} TamaEvent;

extern TamaApp* g_ctx;
extern FuriMutex* g_state_mutex;

void tama_p1_hal_init(hal_t* hal);
