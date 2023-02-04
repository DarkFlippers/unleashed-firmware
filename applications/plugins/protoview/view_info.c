/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"
#include <gui/view.h>
#include <lib/toolbox/random_name.h>

/* This view has subviews accessible navigating up/down. This
 * enumaration is used to track the currently active subview. */
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
    uint8_t cur_info_page; // Info page to display. Useful when there are
        // too many fields populated by the decoder that
        // a single page is not enough.
} InfoViewPrivData;

/* Draw the text label and value of the specified info field at x,y. */
static void render_info_field(Canvas* const canvas, ProtoViewField* f, uint8_t x, uint8_t y) {
    char buf[64];
    char strval[32];

    field_to_string(strval, sizeof(strval), f);
    snprintf(buf, sizeof(buf), "%s: %s", f->name, strval);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, x, y, buf);
}

/* Render the view with the detected message information. */
#define INFO_LINES_PER_PAGE 5
static void render_subview_main(Canvas* const canvas, ProtoViewApp* app) {
    InfoViewPrivData* privdata = app->view_privdata;
    uint8_t pages =
        (app->msg_info->fieldset->numfields + (INFO_LINES_PER_PAGE - 1)) / INFO_LINES_PER_PAGE;
    privdata->cur_info_page %= pages;
    uint8_t current_page = privdata->cur_info_page;
    char buf[32];

    /* Protocol name as title. */
    canvas_set_font(canvas, FontPrimary);
    uint8_t y = 8, lineheight = 10;

    if(pages > 1) {
        snprintf(
            buf, sizeof(buf), "%s %u/%u", app->msg_info->decoder->name, current_page + 1, pages);
        canvas_draw_str(canvas, 0, y, buf);
    } else {
        canvas_draw_str(canvas, 0, y, app->msg_info->decoder->name);
    }
    y += lineheight;

    /* Draw the info fields. */
    uint8_t max_lines = INFO_LINES_PER_PAGE;
    uint32_t j = current_page * max_lines;
    while(j < app->msg_info->fieldset->numfields) {
        render_info_field(canvas, app->msg_info->fieldset->fields[j++], 0, y);
        y += lineheight;
        if(--max_lines == 0) break;
    }

    /* Draw a vertical "save" label. Temporary solution, to switch to
     * something better ASAP. */
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
    canvas_draw_str(canvas, 0, 6, "ok: send, long ok: save");
}

/* Render the selected subview of this view. */
void render_view_info(Canvas* const canvas, ProtoViewApp* app) {
    if(app->signal_decoded == false) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 30, 36, "No signal decoded");
        return;
    }

    ui_show_available_subviews(canvas, app, SubViewInfoLast);
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
static void text_input_done_callback(void* context) {
    ProtoViewApp* app = context;
    InfoViewPrivData* privdata = app->view_privdata;

    FuriString* save_path =
        furi_string_alloc_printf("%s/%s.sub", EXT_PATH("subghz"), privdata->filename);
    save_signal(app, furi_string_get_cstr(save_path));
    furi_string_free(save_path);

    free(privdata->filename);
    privdata->filename = NULL; // Don't free it again on view exit
    ui_dismiss_keyboard(app);
    ui_show_alert(app, "Signal saved", 1500);
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
    snprintf(buf, buflen, "%.10s-%s-%d", app->msg_info->decoder->name, suffix, rand() % 1000);
    str_replace(buf, ' ', '_');
    str_replace(buf, '-', '_');
    str_replace(buf, '/', '_');
}

/* ========================== Signal transmission =========================== */

/* This is the context we pass to the data yield callback for
 * asynchronous tx. */
typedef enum {
    SendSignalSendStartGap,
    SendSignalSendBits,
    SendSignalSendEndGap,
    SendSignalEndTransmission
} SendSignalState;

#define PROTOVIEW_SENDSIGNAL_START_GAP 10000 /* microseconds. */
#define PROTOVIEW_SENDSIGNAL_END_GAP 10000 /* microseconds. */

typedef struct {
    SendSignalState state; // Current state.
    uint32_t curpos; // Current bit position of data to send.
    ProtoViewApp* app; // App reference.
    uint32_t start_gap_dur; // Gap to send at the start.
    uint32_t end_gap_dur; // Gap to send at the end.
} SendSignalCtx;

/* Setup the state context for the callback responsible to feed data
 * to the subghz async tx system. */
static void send_signal_init(SendSignalCtx* ss, ProtoViewApp* app) {
    ss->state = SendSignalSendStartGap;
    ss->curpos = 0;
    ss->app = app;
    ss->start_gap_dur = PROTOVIEW_SENDSIGNAL_START_GAP;
    ss->end_gap_dur = PROTOVIEW_SENDSIGNAL_END_GAP;
}

/* Send signal data feeder callback. When the asynchronous transmission is
 * active, this function is called to return new samples from the currently
 * decoded signal in app->msg_info. The subghz subsystem aspects this function,
 * that is the data feeder, to return LevelDuration types (that is a structure
 * with level, that is pulse or gap, and duration in microseconds).
 *
 * The position into the transmission is stored in the context 'ctx', that
 * references a SendSignalCtx structure.
 *
 * In the SendSignalCtx structure 'ss' we remember at which bit of the
 * message we are, in ss->curoff. We also send a start and end gap in order
 * to make sure the transmission is clear.
 */
LevelDuration radio_tx_feed_data(void* ctx) {
    SendSignalCtx* ss = ctx;

    /* Send start gap. */
    if(ss->state == SendSignalSendStartGap) {
        ss->state = SendSignalSendBits;
        return level_duration_make(0, ss->start_gap_dur);
    }

    /* Send data. */
    if(ss->state == SendSignalSendBits) {
        uint32_t dur = 0, j;
        uint32_t level = 0;

        /* Let's see how many consecutive bits we have with the same
         * level. */
        for(j = 0; ss->curpos + j < ss->app->msg_info->pulses_count; j++) {
            uint32_t l =
                bitmap_get(ss->app->msg_info->bits, ss->app->msg_info->bits_bytes, ss->curpos + j);
            if(j == 0) {
                /* At the first bit of this sequence, we store the
                 * level of the sequence. */
                level = l;
                dur += ss->app->msg_info->short_pulse_dur;
                continue;
            }

            /* As long as the level is the same, we update the duration.
             * Otherwise stop the loop and return this sample. */
            if(l != level) break;
            dur += ss->app->msg_info->short_pulse_dur;
        }
        ss->curpos += j;

        /* If this was the last set of bits, change the state to
         * send the final gap. */
        if(ss->curpos >= ss->app->msg_info->pulses_count) ss->state = SendSignalSendEndGap;
        return level_duration_make(level, dur);
    }

    /* Send end gap. */
    if(ss->state == SendSignalSendEndGap) {
        ss->state = SendSignalEndTransmission;
        return level_duration_make(0, ss->end_gap_dur);
    }

    /* End transmission. Here state is guaranteed
     * to be SendSignalEndTransmission */
    return level_duration_reset();
}

/* Vibrate and produce a click sound when a signal is sent. */
void notify_signal_sent(ProtoViewApp* app) {
    static const NotificationSequence sent_seq = {
        &message_blue_255,
        &message_vibro_on,
        &message_note_g1,
        &message_delay_10,
        &message_sound_off,
        &message_vibro_off,
        &message_blue_0,
        NULL};
    notification_message(app->notification, &sent_seq);
}

/* Handle input for the info view. */
void process_input_info(ProtoViewApp* app, InputEvent input) {
    /* If we don't have a decoded signal, we don't allow to go up/down
     * in the subviews: they are only useful when a loaded signal. */
    if(app->signal_decoded && ui_process_subview_updown(app, input, SubViewInfoLast)) return;

    InfoViewPrivData* privdata = app->view_privdata;
    int subview = ui_get_current_subview(app);

    /* Main subview. */
    if(subview == SubViewInfoMain) {
        if(input.type == InputTypeLong && input.key == InputKeyOk) {
            /* Reset the current sample to capture the next. */
            reset_current_signal(app);
        } else if(input.type == InputTypeShort && input.key == InputKeyOk) {
            /* Show next info page. */
            privdata->cur_info_page++;
        }
    } else if(subview == SubViewInfoSave) {
        /* Save subview. */
        if(input.type == InputTypePress && input.key == InputKeyRight) {
            privdata->signal_display_start_row++;
        } else if(input.type == InputTypePress && input.key == InputKeyLeft) {
            if(privdata->signal_display_start_row != 0) privdata->signal_display_start_row--;
        } else if(input.type == InputTypeLong && input.key == InputKeyOk) {
            // We have have the buffer already allocated, in case the
            // user aborted with BACK a previous saving.
            if(privdata->filename == NULL) privdata->filename = malloc(SAVE_FILENAME_LEN);
            set_signal_random_filename(app, privdata->filename, SAVE_FILENAME_LEN);
            ui_show_keyboard(app, privdata->filename, SAVE_FILENAME_LEN, text_input_done_callback);
        } else if(input.type == InputTypeShort && input.key == InputKeyOk) {
            SendSignalCtx send_state;
            send_signal_init(&send_state, app);
            radio_tx_signal(app, radio_tx_feed_data, &send_state);
            notify_signal_sent(app);
        }
    }
}

/* Called on view exit. */
void view_exit_info(ProtoViewApp* app) {
    InfoViewPrivData* privdata = app->view_privdata;
    // When the user aborts the keyboard input, we are left with the
    // filename buffer allocated.
    if(privdata->filename) free(privdata->filename);
}
