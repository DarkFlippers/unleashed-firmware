#include <furi.h>
#include <furi_hal.h>
#include <stdlib.h>
#include <stm32wbxx_ll_tim.h>
#include "tama.h"

#define TAG_HAL "TamaLIB"

static void* tama_p1_hal_malloc(u32_t size) {
    return malloc(size);
}

static void tama_p1_hal_free(void* ptr) {
    free(ptr);
}

static void tama_p1_hal_halt(void) {
    g_ctx->halted = true;
}

static bool_t tama_p1_hal_is_log_enabled(log_level_t level) {
    switch(level) {
    case LOG_ERROR:
        return true;
    case LOG_INFO:
        return true;
    case LOG_MEMORY:
        return false;
    case LOG_CPU:
        return false;
    default:
        return false;
    }
}

static void tama_p1_hal_log(log_level_t level, char* buff, ...) {
    if(!tama_p1_hal_is_log_enabled(level)) return;

    FuriString* string;
    va_list args;
    va_start(args, buff);
    furi_string_cat_vprintf(string, buff, args);
    va_end(args);

    switch(level) {
    case LOG_ERROR:
        FURI_LOG_E(TAG_HAL, "%s", furi_string_get_cstr(string));
        break;
    case LOG_INFO:
        FURI_LOG_I(TAG_HAL, "%s", furi_string_get_cstr(string));
        break;
    case LOG_MEMORY:
    case LOG_CPU:
    default:
        FURI_LOG_D(TAG_HAL, "%s", furi_string_get_cstr(string));
        break;
    }

    furi_string_free(string);
}

static void tama_p1_hal_sleep_until(timestamp_t ts) {
    while(true) {
        uint32_t count = LL_TIM_GetCounter(TIM2);
        uint32_t delay = ts - count;
        // FURI_LOG_D(TAG, "delay: %x", delay);
        // Stolen from furi_delay_until_tick
        if(delay != 0 && 0 == (delay >> (8 * sizeof(uint32_t) - 1))) {
            // Not the best place to release mutex, but this is the only place we know whether
            // we're ahead or behind, otherwise around the step call we'll always have to
            // delay a tick and run more and more behind.
            furi_mutex_release(g_state_mutex);
            furi_delay_tick(1);
            while(furi_mutex_acquire(g_state_mutex, FuriWaitForever) != FuriStatusOk)
                furi_delay_tick(1);
        } else {
            break;
        }
    }
}

static timestamp_t tama_p1_hal_get_timestamp(void) {
    return LL_TIM_GetCounter(TIM2);
}

static void tama_p1_hal_update_screen(void) {
    // Do nothing, covered by main loop
}

static void tama_p1_hal_set_lcd_matrix(u8_t x, u8_t y, bool_t val) {
    if(val)
        g_ctx->framebuffer[y] |= 1 << x;
    else
        g_ctx->framebuffer[y] &= ~(1 << x);
}

static void tama_p1_hal_set_lcd_icon(u8_t icon, bool_t val) {
    if(val)
        g_ctx->icons |= 1 << icon;
    else
        g_ctx->icons &= ~(1 << icon);
}

static void tama_p1_hal_play_frequency(bool_t en) {
    if(en) {
        if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
            furi_hal_speaker_start(g_ctx->frequency, 0.5f);
        }
    } else {
        if(furi_hal_speaker_is_mine()) {
            furi_hal_speaker_stop();
            furi_hal_speaker_release();
        }
    }

    g_ctx->buzzer_on = en;
}

static void tama_p1_hal_set_frequency(u32_t freq) {
    g_ctx->frequency = freq / 10.0F;
    if(g_ctx->buzzer_on) tama_p1_hal_play_frequency(true);
}

static int tama_p1_hal_handler(void) {
    // Do nothing
    return 0;
}

void tama_p1_hal_init(hal_t* hal) {
    hal->malloc = tama_p1_hal_malloc;
    hal->free = tama_p1_hal_free;
    hal->halt = tama_p1_hal_halt;
    hal->is_log_enabled = tama_p1_hal_is_log_enabled;
    hal->log = tama_p1_hal_log;
    hal->sleep_until = tama_p1_hal_sleep_until;
    hal->get_timestamp = tama_p1_hal_get_timestamp;
    hal->update_screen = tama_p1_hal_update_screen;
    hal->set_lcd_matrix = tama_p1_hal_set_lcd_matrix;
    hal->set_lcd_icon = tama_p1_hal_set_lcd_icon;
    hal->set_frequency = tama_p1_hal_set_frequency;
    hal->play_frequency = tama_p1_hal_play_frequency;
    hal->handler = tama_p1_hal_handler;
}