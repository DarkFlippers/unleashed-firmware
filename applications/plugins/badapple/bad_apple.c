#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <furi.h>
#include <furi_hal.h>
#include <stm32wbxx_ll_tim.h>

#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include "bad_apple.h"
#include "video_player.h"

#define TAG "badapple"

typedef enum {
    BadAppleEventTypeInput,
    BadAppleEventTypeTick,
} BadAppleEventType;

typedef struct {
    BadAppleEventType type;
    InputEvent* input;
} BadAppleEvent;

// Screen is 128x64 px
static void app_draw_callback(Canvas* canvas, void* ctx) {
    BadAppleCtx* inst = ctx;

    canvas_clear(canvas);
    canvas_draw_xbm(canvas, VIDEO_X, VIDEO_Y, VIDEO_WIDTH, SCREEN_HEIGHT, inst->framebuffer);
    canvas_draw_box(canvas, 0, 0, VIDEO_X, SCREEN_HEIGHT);
    canvas_draw_box(
        canvas, VIDEO_X + VIDEO_WIDTH, 0, SCREEN_WIDTH - VIDEO_WIDTH - VIDEO_X, SCREEN_HEIGHT);
}

static void app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    FuriMessageQueue* event_queue = ctx;
    BadAppleEvent event = {.type = BadAppleEventTypeInput, .input = input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

BadAppleCtx* bad_apple_ctx_alloc(void) {
    BadAppleCtx* inst = malloc(sizeof(BadAppleCtx));
    memset(inst, 0, sizeof(BadAppleCtx));

    if(inst) {
        inst->storage = furi_record_open(RECORD_STORAGE);
        inst->video_file = storage_file_alloc(inst->storage);
        inst->file_buffer_offset = sizeof(inst->file_buffer);
    }

    return inst;
}

void bad_apple_ctx_free(BadAppleCtx* inst) {
    if(inst) {
        storage_file_free(inst->video_file);
        furi_record_close(RECORD_STORAGE);
        free(inst);
    }
}

void bad_apple_load_next_video_chunk(BadAppleCtx* inst) {
    size_t bytes_to_read = sizeof(inst->file_buffer);
    uint8_t* buf_ptr = inst->file_buffer;
    while(bytes_to_read > 0) {
        uint16_t curr_bytes_to_read = bytes_to_read > (UINT16_MAX / 2 + 1) ? (UINT16_MAX / 2 + 1) :
                                                                             bytes_to_read;
        uint16_t read = storage_file_read(inst->video_file, buf_ptr, curr_bytes_to_read);
        bytes_to_read -= read;
        buf_ptr += read;
        if(read == 0) break;
    }
    inst->file_buffer_offset = 0;
}

uint8_t bad_apple_read_byte(BadAppleCtx* inst) {
    if(inst->file_buffer_offset >= sizeof(inst->file_buffer)) {
        bad_apple_load_next_video_chunk(inst);
    }
    return inst->file_buffer[inst->file_buffer_offset++];
}

void bad_apple_timer_isr(void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    BadAppleEvent event = {.type = BadAppleEventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
    LL_TIM_ClearFlag_UPDATE(TIM2);
}

void bad_apple_timer_setup(BadAppleCtx* inst, void* ctx) {
    UNUSED(inst);

    LL_TIM_InitTypeDef tim_init = {
        .Prescaler = 63999,
        .CounterMode = LL_TIM_COUNTERMODE_UP,
        .Autoreload = 30,
    };

    LL_TIM_Init(TIM2, &tim_init);
    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_SetCounter(TIM2, 0);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, bad_apple_timer_isr, ctx);
    LL_TIM_EnableIT_UPDATE(TIM2);
}

void bad_apple_timer_deinit(void) {
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_DisableIT_UPDATE(TIM2);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, NULL, NULL);
    LL_TIM_DeInit(TIM2);
}

int32_t bad_apple_main(void* p) {
    UNUSED(p);
    BadAppleCtx* ctx = bad_apple_ctx_alloc();
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(BadAppleEvent));

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, app_draw_callback, ctx);
    view_port_input_callback_set(view_port, app_input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);

    // Frame rate: 32 FPS
    bad_apple_timer_setup(ctx, event_queue);

    bool is_opened = storage_file_open(ctx->video_file, VIDEO_PATH, FSAM_READ, FSOM_OPEN_EXISTING);
    if(is_opened) {
        BadAppleEvent event;
        notification_message(notification, &sequence_display_backlight_enforce_on);

        bool running = true;
        LL_TIM_EnableCounter(TIM2);
        while(running) {
            if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
                if(event.type == BadAppleEventTypeInput) {
                    InputEvent* input_event = event.input;
                    if(input_event->type == InputTypeLong) {
                        if(input_event->key == InputKeyBack) {
                            running = false;
                        }
                    }
                } else if(event.type == BadAppleEventTypeTick) {
                    // FURI_LOG_D(TAG, "Update frame");
                    if(!vp_play_frame(ctx)) {
                        running = false;
                    }
                }
            }
            view_port_update(view_port);
        }
        LL_TIM_DisableCounter(TIM2);
        notification_message(notification, &sequence_display_backlight_enforce_auto);
        storage_file_close(ctx->video_file);
    }

    furi_record_close(RECORD_NOTIFICATION);

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_GUI);
    bad_apple_timer_deinit();
    bad_apple_ctx_free(ctx);

    return 0;
}
