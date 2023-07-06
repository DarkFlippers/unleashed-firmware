/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"
#include <cc1101.h>

static void direct_sampling_timer_start(ProtoViewApp* app);
static void direct_sampling_timer_stop(ProtoViewApp* app);

#define CAPTURED_BITMAP_BITS (128 * 64)
#define CAPTURED_BITMAP_BYTES (CAPTURED_BITMAP_BITS / 8)
#define DEFAULT_USEC_PER_PIXEL 50
#define USEC_PER_PIXEL_SMALL_CHANGE 5
#define USEC_PER_PIXEL_LARGE_CHANGE 25
#define USEC_PER_PIXEL_MIN 5
#define USEC_PER_PIXEL_MAX 300
typedef struct {
    uint8_t* captured; // Bitmap with the last captured screen.
    uint32_t captured_idx; // Current index to write into the bitmap
    uint32_t usec_per_pixel; // Number of useconds a pixel should represent
    bool show_usage_info;
} DirectSamplingViewPrivData;

/* Read directly from the G0 CC1101 pin, and draw a black or white
 * dot depending on the level. */
void render_view_direct_sampling(Canvas* const canvas, ProtoViewApp* app) {
    DirectSamplingViewPrivData* privdata = app->view_privdata;

    if(!app->direct_sampling_enabled && privdata->show_usage_info) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 9, "Direct sampling displays the");
        canvas_draw_str(canvas, 2, 18, "the captured signal in real");
        canvas_draw_str(canvas, 2, 27, "time, like in a CRT TV set.");
        canvas_draw_str(canvas, 2, 36, "Use UP/DOWN to change the");
        canvas_draw_str(canvas, 2, 45, "resolution (usec/pixel).");
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 5, 60, "To start/stop, press OK");
        return;
    }
    privdata->show_usage_info = false;

    /* Draw on screen. */
    int idx = 0;
    for(int y = 0; y < 64; y++) {
        for(int x = 0; x < 128; x++) {
            bool level = bitmap_get(privdata->captured, CAPTURED_BITMAP_BYTES, idx++);
            if(level) canvas_draw_dot(canvas, x, y);
        }
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%lu usec/px", privdata->usec_per_pixel);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_with_border(canvas, 1, 60, buf, ColorWhite, ColorBlack);
}

/* Handle input */
void process_input_direct_sampling(ProtoViewApp* app, InputEvent input) {
    DirectSamplingViewPrivData* privdata = app->view_privdata;

    if(input.type == InputTypePress && input.key == InputKeyOk) {
        app->direct_sampling_enabled = !app->direct_sampling_enabled;
    }

    if((input.key == InputKeyUp || input.key == InputKeyDown) &&
       (input.type == InputTypePress || input.type == InputTypeRepeat)) {
        uint32_t change = input.type == InputTypePress ? USEC_PER_PIXEL_SMALL_CHANGE :
                                                         USEC_PER_PIXEL_LARGE_CHANGE;
        if(input.key == InputKeyUp) change = -change;
        privdata->usec_per_pixel += change;
        if(privdata->usec_per_pixel < USEC_PER_PIXEL_MIN)
            privdata->usec_per_pixel = USEC_PER_PIXEL_MIN;
        else if(privdata->usec_per_pixel > USEC_PER_PIXEL_MAX)
            privdata->usec_per_pixel = USEC_PER_PIXEL_MAX;
        /* Update the timer frequency. */
        direct_sampling_timer_stop(app);
        direct_sampling_timer_start(app);
    }
}

/* Enter view. Stop the subghz thread to prevent access as we read
 * the CC1101 data directly. */
void view_enter_direct_sampling(ProtoViewApp* app) {
    /* Set view defaults. */
    DirectSamplingViewPrivData* privdata = app->view_privdata;
    privdata->usec_per_pixel = DEFAULT_USEC_PER_PIXEL;
    privdata->captured = malloc(CAPTURED_BITMAP_BYTES);
    privdata->show_usage_info = true;

    if(app->txrx->txrx_state == TxRxStateRx && !app->txrx->debug_timer_sampling) {
        subghz_devices_stop_async_rx(app->radio_device);

        /* To read data asynchronously directly from the view, we need
         * to put the CC1101 back into reception mode (the previous call
         * to stop the async RX will put it into idle) and configure the
         * G0 pin for reading. */
        subghz_devices_set_rx(app->radio_device);
        furi_hal_gpio_init(
            subghz_devices_get_data_gpio(app->radio_device),
            GpioModeInput,
            GpioPullNo,
            GpioSpeedLow);
    } else {
        raw_sampling_worker_stop(app);
    }

    // Start the timer to capture raw data
    direct_sampling_timer_start(app);
}

/* Exit view. Restore the subghz thread. */
void view_exit_direct_sampling(ProtoViewApp* app) {
    DirectSamplingViewPrivData* privdata = app->view_privdata;
    if(privdata->captured) free(privdata->captured);
    app->direct_sampling_enabled = false;

    direct_sampling_timer_stop(app);

    /* Restart normal data feeding. */
    if(app->txrx->txrx_state == TxRxStateRx && !app->txrx->debug_timer_sampling) {
        subghz_devices_start_async_rx(app->radio_device, protoview_rx_callback, NULL);
    } else {
        furi_hal_gpio_init(
            subghz_devices_get_data_gpio(app->radio_device),
            GpioModeInput,
            GpioPullNo,
            GpioSpeedLow);
        raw_sampling_worker_start(app);
    }
}

/* =========================== Timer implementation ========================= */

static void ds_timer_isr(void* ctx) {
    ProtoViewApp* app = ctx;
    DirectSamplingViewPrivData* privdata = app->view_privdata;

    if(app->direct_sampling_enabled) {
        bool level = furi_hal_gpio_read(subghz_devices_get_data_gpio(app->radio_device));
        bitmap_set(privdata->captured, CAPTURED_BITMAP_BYTES, privdata->captured_idx, level);
        privdata->captured_idx = (privdata->captured_idx + 1) % CAPTURED_BITMAP_BITS;
    }
    LL_TIM_ClearFlag_UPDATE(TIM2);
}

static void direct_sampling_timer_start(ProtoViewApp* app) {
    DirectSamplingViewPrivData* privdata = app->view_privdata;

    furi_hal_bus_enable(FuriHalBusTIM2);

    LL_TIM_InitTypeDef tim_init = {
        .Prescaler = 63, /* CPU frequency is ~64Mhz. */
        .CounterMode = LL_TIM_COUNTERMODE_UP,
        .Autoreload = privdata->usec_per_pixel};

    LL_TIM_Init(TIM2, &tim_init);
    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_SetCounter(TIM2, 0);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, ds_timer_isr, app);
    LL_TIM_EnableIT_UPDATE(TIM2);
    LL_TIM_EnableCounter(TIM2);
}

static void direct_sampling_timer_stop(ProtoViewApp* app) {
    UNUSED(app);
    FURI_CRITICAL_ENTER();
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_DisableIT_UPDATE(TIM2);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, NULL, NULL);
    furi_hal_bus_disable(FuriHalBusTIM2);
    FURI_CRITICAL_EXIT();
}
