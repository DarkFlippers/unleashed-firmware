/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"
#include <gui/view_i.h>
#include <lib/toolbox/random_name.h>

enum {
    SubViewInfoMain,
    SubViewInfoSave,
    SubViewInfoLast, /* Just a sentinel. */
};

/* Our view private data. */
#define SAVE_FILENAME_LEN 64
typedef struct {
    /* Our save view displays an oscilloscope-alike resampled signal,
     * so that the user can see what they are saving. With left/right
     * you can move to next rows. Here we store where we are. */
    uint32_t signal_display_start_row;
    char* filename;
} InfoViewPrivData;

/* Render the view with the detected message information. */
static void render_subview_main(Canvas* const canvas, ProtoViewApp* app) {
    /* Protocol name as title. */
    canvas_set_font(canvas, FontPrimary);
    uint8_t y = 8, lineheight = 10;
    canvas_draw_str(canvas, 0, y, app->msg_info->name);
    y += lineheight;

    /* Info fields. */
    char buf[128];
    canvas_set_font(canvas, FontSecondary);
    if(app->msg_info->raw[0]) {
        snprintf(buf, sizeof(buf), "Raw: %s", app->msg_info->raw);
        canvas_draw_str(canvas, 0, y, buf);
        y += lineheight;
    }
    canvas_draw_str(canvas, 0, y, app->msg_info->info1);
    y += lineheight;
    canvas_draw_str(canvas, 0, y, app->msg_info->info2);
    y += lineheight;
    canvas_draw_str(canvas, 0, y, app->msg_info->info3);
    y += lineheight;
    canvas_draw_str(canvas, 0, y, app->msg_info->info4);
    y += lineheight;

    y = 37;
    lineheight = 7;
    canvas_draw_str(canvas, 119, y, "s");
    y += lineheight;
    canvas_draw_str(canvas, 119, y, "a");
    y += lineheight;
    canvas_draw_str(canvas, 119, y, "v");
    y += lineheight;
    canvas_draw_str(canvas, 119, y, "e");
    y += lineheight;
}

/* Render view with save option. */
static void render_subview_save(Canvas* const canvas, ProtoViewApp* app) {
    InfoViewPrivData* privdata = app->view_privdata;

    /* Display our signal in digital form: here we don't show the
     * signal with the exact timing of the received samples, but as it
     * is in its logic form, in exact multiples of the short pulse length. */
    uint8_t rows = 6;
    uint8_t rowheight = 11;
    uint8_t bitwidth = 4;
    uint8_t bitheight = 5;
    uint32_t idx = privdata->signal_display_start_row * (128 / 4);
    bool prevbit = false;
    for(uint8_t y = bitheight + 12; y <= rows * rowheight; y += rowheight) {
        for(uint8_t x = 0; x < 128; x += 4) {
            bool bit = bitmap_get(app->msg_info->bits, app->msg_info->bits_bytes, idx);
            uint8_t prevy = y + prevbit * (bitheight * -1) - 1;
            uint8_t thisy = y + bit * (bitheight * -1) - 1;
            canvas_draw_line(canvas, x, prevy, x, thisy);
            canvas_draw_line(canvas, x, thisy, x + bitwidth - 1, thisy);
            prevbit = bit;
            if(idx >= app->msg_info->pulses_count) {
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_dot(canvas, x + 1, thisy);
                canvas_draw_dot(canvas, x + 3, thisy);
                canvas_set_color(canvas, ColorBlack);
            }
            idx++; // Draw next bit
        }
    }

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 6, "ok: save, < >: slide rows");
}

/* Render the selected subview of this view. */
void render_view_info(Canvas* const canvas, ProtoViewApp* app) {
    if(app->signal_decoded == false) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 30, 36, "No signal decoded");
        return;
    }

    show_available_subviews(canvas, app, SubViewInfoLast);
    switch(app->current_subview[app->current_view]) {
    case SubViewInfoMain:
        render_subview_main(canvas, app);
        break;
    case SubViewInfoSave:
        render_subview_save(canvas, app);
        break;
    }
}

/* The user typed the file name. Let's save it and remove the keyboard
 * view. */
void text_input_done_callback(void* context) {
    ProtoViewApp* app = context;
    InfoViewPrivData* privdata = app->view_privdata;

    FuriString* save_path =
        furi_string_alloc_printf("%s/%s.sub", EXT_PATH("subghz"), privdata->filename);
    save_signal(app, furi_string_get_cstr(save_path));
    furi_string_free(save_path);

    free(privdata->filename);
    dismiss_keyboard(app);
}

/* Replace all the occurrences of character c1 with c2 in the specified
 * string. */
void str_replace(char* buf, char c1, char c2) {
    char* p = buf;
    while(*p) {
        if(*p == c1) *p = c2;
        p++;
    }
}

/* Set a random filename the user can edit. */
void set_signal_random_filename(ProtoViewApp* app, char* buf, size_t buflen) {
    char suffix[6];
    set_random_name(suffix, sizeof(suffix));
    snprintf(buf, buflen, "%.10s-%s-%d", app->msg_info->name, suffix, rand() % 1000);
    str_replace(buf, ' ', '_');
    str_replace(buf, '-', '_');
    str_replace(buf, '/', '_');
}

/* Handle input for the info view. */
void process_input_info(ProtoViewApp* app, InputEvent input) {
    if(process_subview_updown(app, input, SubViewInfoLast)) return;
    InfoViewPrivData* privdata = app->view_privdata;
    int subview = get_current_subview(app);

    /* Main subview. */
    if(subview == SubViewInfoMain) {
        if(input.type == InputTypeShort && input.key == InputKeyOk) {
            /* Reset the current sample to capture the next. */
            reset_current_signal(app);
        }
    } else if(subview == SubViewInfoSave) {
        /* Save subview. */
        if(input.type == InputTypePress && input.key == InputKeyRight) {
            privdata->signal_display_start_row++;
        } else if(input.type == InputTypePress && input.key == InputKeyLeft) {
            if(privdata->signal_display_start_row != 0) privdata->signal_display_start_row--;
        } else if(input.type == InputTypePress && input.key == InputKeyOk) {
            privdata->filename = malloc(SAVE_FILENAME_LEN);
            set_signal_random_filename(app, privdata->filename, SAVE_FILENAME_LEN);
            show_keyboard(app, privdata->filename, SAVE_FILENAME_LEN, text_input_done_callback);
        }
    }
}
