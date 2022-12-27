#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>
#include <stdlib.h>
#include <stm32wbxx_ll_tim.h>
#include "tamalib/tamalib.h"
#include "tama.h"
#include "compiled/assets_icons.h"

TamaApp* g_ctx;
FuriMutex* g_state_mutex;

static const Icon* icons_list[] = {
    &I_icon_0,
    &I_icon_1,
    &I_icon_2,
    &I_icon_3,
    &I_icon_4,
    &I_icon_5,
    &I_icon_6,
    &I_icon_7,
};

static void tama_p1_draw_callback(Canvas* const canvas, void* cb_ctx) {
    furi_assert(cb_ctx);

    FuriMutex* const mutex = cb_ctx;
    if(furi_mutex_acquire(mutex, 25) != FuriStatusOk) return;

    if(g_ctx->rom == NULL) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 30, 30, "No ROM");
    } else if(g_ctx->halted) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 30, 30, "Halted");
    } else {
        // FURI_LOG_D(TAG, "Drawing frame");
        // Calculate positioning
        uint16_t canv_width = canvas_width(canvas);
        uint16_t canv_height = canvas_height(canvas);
        uint16_t lcd_matrix_scaled_width = 32 * TAMA_SCREEN_SCALE_FACTOR;
        uint16_t lcd_matrix_scaled_height = 16 * TAMA_SCREEN_SCALE_FACTOR;
        uint16_t lcd_matrix_top = (canv_height - lcd_matrix_scaled_height) / 2;
        uint16_t lcd_matrix_left = (canv_width - lcd_matrix_scaled_width) / 2;
        uint16_t lcd_icon_upper_top = lcd_matrix_top - TAMA_LCD_ICON_SIZE - TAMA_LCD_ICON_MARGIN;
        uint16_t lcd_icon_upper_left = lcd_matrix_left;
        uint16_t lcd_icon_lower_top =
            lcd_matrix_top + lcd_matrix_scaled_height + TAMA_LCD_ICON_MARGIN;
        uint16_t lcd_icon_lower_left = lcd_matrix_left;
        uint16_t lcd_icon_spacing_horiz =
            (lcd_matrix_scaled_width - (4 * TAMA_LCD_ICON_SIZE)) / 3 + TAMA_LCD_ICON_SIZE;

        // Draw pixels
        // canvas_draw_frame(
        //     canvas,
        //     lcd_matrix_left,
        //     lcd_matrix_top,
        //     lcd_matrix_scaled_width,
        //     lcd_matrix_scaled_height);

        uint16_t y = lcd_matrix_top;
        for(uint8_t row = 0; row < 16; ++row) {
            uint16_t x = lcd_matrix_left;
            uint32_t row_pixels = g_ctx->framebuffer[row];
            for(uint8_t col = 0; col < 32; ++col) {
                if(row_pixels & 1) {
                    canvas_draw_box(
                        canvas, x, y, TAMA_SCREEN_SCALE_FACTOR, TAMA_SCREEN_SCALE_FACTOR);
                }
                x += TAMA_SCREEN_SCALE_FACTOR;
                row_pixels >>= 1;
            }
            y += TAMA_SCREEN_SCALE_FACTOR;
        }

        // Draw icons
        uint8_t lcd_icons = g_ctx->icons;
        // Top
        y = lcd_icon_upper_top;
        uint16_t x_ic = lcd_icon_upper_left;
        for(uint8_t i = 0; i < 4; ++i) {
            // canvas_draw_frame(canvas, x_ic, y, TAMA_LCD_ICON_SIZE, TAMA_LCD_ICON_SIZE);
            if(lcd_icons & 1) {
                canvas_draw_icon(canvas, x_ic, y, icons_list[i]);
            }
            x_ic += lcd_icon_spacing_horiz;
            lcd_icons >>= 1;
        }

        // Bottom
        y = lcd_icon_lower_top;
        x_ic = lcd_icon_lower_left;
        for(uint8_t i = 4; i < 8; ++i) {
            // canvas_draw_frame(canvas, x_ic, y, TAMA_LCD_ICON_SIZE, TAMA_LCD_ICON_SIZE);
            if(lcd_icons & 1) {
                canvas_draw_icon(canvas, x_ic, y, icons_list[i]);
            }
            x_ic += lcd_icon_spacing_horiz;
            lcd_icons >>= 1;
        }
    }

    furi_mutex_release(mutex);
}

static void tama_p1_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    TamaEvent event = {.type = EventTypeInput, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void tama_p1_update_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    TamaEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

static int32_t tama_p1_worker(void* context) {
    bool running = true;
    FuriMutex* mutex = context;
    while(furi_mutex_acquire(mutex, FuriWaitForever) != FuriStatusOk) furi_delay_tick(1);

    cpu_sync_ref_timestamp();
    LL_TIM_EnableCounter(TIM2);
    while(running) {
        if(furi_thread_flags_get()) {
            running = false;
        } else {
            // FURI_LOG_D(TAG, "Stepping");
            // for (int i = 0; i < 100; ++i)
            tamalib_step();
        }
    }
    LL_TIM_DisableCounter(TIM2);
    furi_mutex_release(mutex);
    return 0;
}

static void tama_p1_init(TamaApp* const ctx) {
    g_ctx = ctx;
    memset(ctx, 0, sizeof(TamaApp));
    tama_p1_hal_init(&ctx->hal);

    // Load ROM
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FileInfo fi;
    if(storage_common_stat(storage, TAMA_ROM_PATH, &fi) == FSE_OK) {
        File* rom_file = storage_file_alloc(storage);
        if(storage_file_open(rom_file, TAMA_ROM_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
            ctx->rom = malloc((size_t)fi.size);
            uint8_t* buf_ptr = ctx->rom;
            size_t read = 0;
            while(read < fi.size) {
                size_t to_read = fi.size - read;
                if(to_read > UINT16_MAX) to_read = UINT16_MAX;
                uint16_t now_read = storage_file_read(rom_file, buf_ptr, (uint16_t)to_read);
                read += now_read;
                buf_ptr += now_read;
            }

            // Reorder endianess of ROM
            for(size_t i = 0; i < fi.size; i += 2) {
                uint8_t b = ctx->rom[i];
                ctx->rom[i] = ctx->rom[i + 1];
                ctx->rom[i + 1] = b & 0xF;
            }
        }

        storage_file_close(rom_file);
        storage_file_free(rom_file);
    }
    furi_record_close(RECORD_STORAGE);

    if(ctx->rom != NULL) {
        // Init TIM2
        // 64KHz
        LL_TIM_InitTypeDef tim_init = {
            .Prescaler = 999,
            .CounterMode = LL_TIM_COUNTERMODE_UP,
            .Autoreload = 0xFFFFFFFF,
        };
        LL_TIM_Init(TIM2, &tim_init);
        LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
        LL_TIM_DisableCounter(TIM2);
        LL_TIM_SetCounter(TIM2, 0);

        // Init TamaLIB
        tamalib_register_hal(&ctx->hal);
        tamalib_init((u12_t*)ctx->rom, NULL, 64000);
        tamalib_set_speed(1);

        // TODO: implement fast forwarding
        ctx->fast_forward_done = true;

        // Start stepping thread
        ctx->thread = furi_thread_alloc();
        furi_thread_set_name(ctx->thread, "TamaLIB");
        furi_thread_set_stack_size(ctx->thread, 1024);
        furi_thread_set_callback(ctx->thread, tama_p1_worker);
        furi_thread_set_context(ctx->thread, g_state_mutex);
        furi_thread_start(ctx->thread);
    }
}

static void tama_p1_deinit(TamaApp* const ctx) {
    if(ctx->rom != NULL) {
        tamalib_release();
        furi_thread_free(ctx->thread);
        free(ctx->rom);
    }
}

int32_t tama_p1_app(void* p) {
    UNUSED(p);

    TamaApp* ctx = malloc(sizeof(TamaApp));
    g_state_mutex = furi_mutex_alloc(FuriMutexTypeRecursive);
    tama_p1_init(ctx);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(TamaEvent));

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, tama_p1_draw_callback, g_state_mutex);
    view_port_input_callback_set(view_port, tama_p1_input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    FuriTimer* timer =
        furi_timer_alloc(tama_p1_update_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 30);

    for(bool running = true; running;) {
        TamaEvent event;
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, FuriWaitForever);
        if(event_status == FuriStatusOk) {
            // Local override with acquired context
            if(furi_mutex_acquire(g_state_mutex, FuriWaitForever) != FuriStatusOk) continue;

            if(event.type == EventTypeTick) {
                // FURI_LOG_D(TAG, "EventTypeTick");
                view_port_update(view_port);
            } else if(event.type == EventTypeInput) {
                FURI_LOG_D(
                    TAG,
                    "EventTypeInput: %ld %d %d",
                    event.input.sequence,
                    event.input.key,
                    event.input.type);
                InputType input_type = event.input.type;
                if(input_type == InputTypePress || input_type == InputTypeRelease) {
                    btn_state_t tama_btn_state = 0;
                    if(input_type == InputTypePress)
                        tama_btn_state = BTN_STATE_PRESSED;
                    else if(input_type == InputTypeRelease)
                        tama_btn_state = BTN_STATE_RELEASED;

                    if(event.input.key == InputKeyLeft) {
                        tamalib_set_button(BTN_LEFT, tama_btn_state);
                    } else if(event.input.key == InputKeyOk) {
                        tamalib_set_button(BTN_MIDDLE, tama_btn_state);
                    } else if(event.input.key == InputKeyRight) {
                        tamalib_set_button(BTN_RIGHT, tama_btn_state);
                    }
                }

                if(event.input.key == InputKeyBack && event.input.type == InputTypeLong) {
                    furi_timer_stop(timer);
                    running = false;
                }
            }

            furi_mutex_release(g_state_mutex);
        } else {
            // Timeout
            // FURI_LOG_D(TAG, "Timed out");
        }
    }

    if(ctx->rom != NULL) {
        furi_thread_flags_set(furi_thread_get_id(ctx->thread), 1);
        furi_thread_join(ctx->thread);
    }

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(g_state_mutex);
    tama_p1_deinit(ctx);
    free(ctx);

    return 0;
}
