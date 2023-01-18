/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"

#include <cc1101.h>

/* Read directly from the G0 CC1101 pin, and draw a black or white
 * dot depending on the level. */
void render_view_direct_sampling(Canvas* const canvas, ProtoViewApp* app) {
    if(!app->direct_sampling_enabled) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 9, "Direct sampling is a special");
        canvas_draw_str(canvas, 2, 18, "mode that displays the signal");
        canvas_draw_str(canvas, 2, 27, "captured in real time. Like in");
        canvas_draw_str(canvas, 2, 36, "a old CRT TV. It's very slow.");
        canvas_draw_str(canvas, 2, 45, "Can crash your Flipper.");
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 14, 60, "To enable press OK");
        return;
    }

    for(int y = 0; y < 64; y++) {
        for(int x = 0; x < 128; x++) {
            bool level = furi_hal_gpio_read(&gpio_cc1101_g0);
            if(level) canvas_draw_dot(canvas, x, y);
            /* Busy loop: this is a terrible approach as it blocks
             * everything else, but for now it's the best we can do
             * to obtain direct data with some spacing. */
            uint32_t x = 250;
            while(x--)
                ;
        }
    }
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_with_border(canvas, 36, 60, "Direct sampling", ColorWhite, ColorBlack);
}

/* Handle input */
void process_input_direct_sampling(ProtoViewApp* app, InputEvent input) {
    if(input.type == InputTypePress && input.key == InputKeyOk) {
        app->direct_sampling_enabled = !app->direct_sampling_enabled;
    }
}

/* Enter view. Stop the subghz thread to prevent access as we read
 * the CC1101 data directly. */
void view_enter_direct_sampling(ProtoViewApp* app) {
    if(app->txrx->txrx_state == TxRxStateRx && !app->txrx->debug_timer_sampling) {
        subghz_worker_stop(app->txrx->worker);
    } else {
        raw_sampling_worker_stop(app);
    }
}

/* Exit view. Restore the subghz thread. */
void view_exit_direct_sampling(ProtoViewApp* app) {
    if(app->txrx->txrx_state == TxRxStateRx && !app->txrx->debug_timer_sampling) {
        subghz_worker_start(app->txrx->worker);
    } else {
        raw_sampling_worker_start(app);
    }
    app->direct_sampling_enabled = false;
}
