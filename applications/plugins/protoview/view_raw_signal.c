/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"

/* Render the received signal.
 *
 * The screen of the flipper is 128 x 64. Even using 4 pixels per line
 * (where low level signal is one pixel high, high level is 4 pixels
 * high) and 4 pixels of spacing between the different lines, we can
 * plot comfortably 8 lines.
 *
 * The 'idx' argument is the first sample to render in the circular
 * buffer. */
void render_signal(ProtoViewApp* app, Canvas* const canvas, RawSamplesBuffer* buf, uint32_t idx) {
    canvas_set_color(canvas, ColorBlack);

    int rows = 8;
    uint32_t time_per_pixel = app->us_scale;
    uint32_t start_idx = idx;
    bool level = 0;
    uint32_t dur = 0, sample_num = 0;
    for(int row = 0; row < rows; row++) {
        for(int x = 0; x < 128; x++) {
            int y = 3 + row * 8;
            if(dur < time_per_pixel / 2) {
                /* Get more data. */
                raw_samples_get(buf, idx++, &level, &dur);
                sample_num++;
            }

            canvas_draw_line(canvas, x, y, x, y - (level * 3));

            /* Write a small triangle under the last sample detected. */
            if(app->signal_bestlen != 0 && sample_num + start_idx == app->signal_bestlen + 1) {
                canvas_draw_dot(canvas, x, y + 2);
                canvas_draw_dot(canvas, x - 1, y + 3);
                canvas_draw_dot(canvas, x, y + 3);
                canvas_draw_dot(canvas, x + 1, y + 3);
                sample_num++; /* Make sure we don't mark the next, too. */
            }

            /* Remove from the current level duration the time we
             * just plot. */
            if(dur > time_per_pixel)
                dur -= time_per_pixel;
            else
                dur = 0;
        }
    }
}

/* Raw pulses rendering. This is our default view. */
void render_view_raw_pulses(Canvas* const canvas, ProtoViewApp* app) {
    /* Show signal. */
    render_signal(app, canvas, DetectedSamples, app->signal_offset);

    /* Show signal information. */
    char buf[64];
    snprintf(buf, sizeof(buf), "%luus", (unsigned long)DetectedSamples->short_pulse_dur);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_with_border(canvas, 97, 63, buf, ColorWhite, ColorBlack);
    if(app->signal_decoded) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_with_border(
            canvas, 1, 61, app->msg_info->decoder->name, ColorWhite, ColorBlack);
    }
}

/* Handle input for the raw pulses view. */
void process_input_raw_pulses(ProtoViewApp* app, InputEvent input) {
    if(input.type == InputTypeRepeat) {
        /* Handle panning of the signal window. Long pressing
         * right will show successive samples, long pressing left
         * previous samples. */
        if(input.key == InputKeyRight)
            app->signal_offset++;
        else if(input.key == InputKeyLeft)
            app->signal_offset--;
    } else if(input.type == InputTypeLong) {
        if(input.key == InputKeyOk) {
            /* Reset the current sample to capture the next. */
            reset_current_signal(app);
        }
    } else if(input.type == InputTypeShort) {
        if(input.key == InputKeyOk) {
            app->signal_offset = 0;
            adjust_raw_view_scale(app, DetectedSamples->short_pulse_dur);
        } else if(input.key == InputKeyDown) {
            /* Rescaling. The set becomes finer under 50us per pixel. */
            uint32_t scale_step = app->us_scale >= 50 ? 50 : 10;
            if(app->us_scale < 500) app->us_scale += scale_step;
        } else if(input.key == InputKeyUp) {
            uint32_t scale_step = app->us_scale > 50 ? 50 : 10;
            if(app->us_scale > 10) app->us_scale -= scale_step;
        }
    }
}

/* Adjust raw view scale depending on short pulse duration. */
void adjust_raw_view_scale(ProtoViewApp* app, uint32_t short_pulse_dur) {
    if(short_pulse_dur == 0)
        app->us_scale = PROTOVIEW_RAW_VIEW_DEFAULT_SCALE;
    else if(short_pulse_dur < 75)
        app->us_scale = 10;
    else if(short_pulse_dur < 145)
        app->us_scale = 30;
    else if(short_pulse_dur < 400)
        app->us_scale = 100;
    else if(short_pulse_dur < 1000)
        app->us_scale = 200;
    else
        app->us_scale = PROTOVIEW_RAW_VIEW_DEFAULT_SCALE;
}
