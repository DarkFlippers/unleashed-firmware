#include "dtmf_dolphin_dialer.h"

#include <gui/elements.h>

typedef struct DTMFDolphinDialer {
    View* view;
    DTMFDolphinDialerOkCallback callback;
    void* context;
} DTMFDolphinDialer;

typedef struct {
    DTMFDolphinToneSection section;
    uint8_t row;
    uint8_t col;
    float freq1;
    float freq2;
    bool playing;
    uint16_t pulses;
    uint16_t pulse_ms;
    uint16_t gap_ms;
} DTMFDolphinDialerModel;

static bool dtmf_dolphin_dialer_process_up(DTMFDolphinDialer* dtmf_dolphin_dialer);
static bool dtmf_dolphin_dialer_process_down(DTMFDolphinDialer* dtmf_dolphin_dialer);
static bool dtmf_dolphin_dialer_process_left(DTMFDolphinDialer* dtmf_dolphin_dialer);
static bool dtmf_dolphin_dialer_process_right(DTMFDolphinDialer* dtmf_dolphin_dialer);
static bool
    dtmf_dolphin_dialer_process_ok(DTMFDolphinDialer* dtmf_dolphin_dialer, InputEvent* event);

void draw_button(Canvas* canvas, uint8_t row, uint8_t col, bool invert) {
    uint8_t left = DTMF_DOLPHIN_NUMPAD_X + // ((col + 1) * DTMF_DOLPHIN_BUTTON_PADDING) +
                   (col * DTMF_DOLPHIN_BUTTON_WIDTH);
    // (col * DTMF_DOLPHIN_BUTTON_PADDING);
    uint8_t top = DTMF_DOLPHIN_NUMPAD_Y + // ((row + 1) * DTMF_DOLPHIN_BUTTON_PADDING) +
                  (row * DTMF_DOLPHIN_BUTTON_HEIGHT);
    // (row * DTMF_DOLPHIN_BUTTON_PADDING);

    uint8_t span = dtmf_dolphin_get_tone_span(row, col);

    if(span == 0) {
        return;
    }

    canvas_set_color(canvas, ColorBlack);

    if(invert)
        canvas_draw_rbox(
            canvas,
            left,
            top,
            (DTMF_DOLPHIN_BUTTON_WIDTH * span) - (DTMF_DOLPHIN_BUTTON_PADDING * 2),
            DTMF_DOLPHIN_BUTTON_HEIGHT - (DTMF_DOLPHIN_BUTTON_PADDING * 2),
            2);
    else
        canvas_draw_rframe(
            canvas,
            left,
            top,
            (DTMF_DOLPHIN_BUTTON_WIDTH * span) - (DTMF_DOLPHIN_BUTTON_PADDING * 2),
            DTMF_DOLPHIN_BUTTON_HEIGHT - (DTMF_DOLPHIN_BUTTON_PADDING * 2),
            2);

    if(invert) canvas_invert_color(canvas);

    canvas_set_font(canvas, FontSecondary);
    // canvas_set_color(canvas, invert ? ColorWhite : ColorBlack);
    canvas_draw_str_aligned(
        canvas,
        left - 1 + (int)((DTMF_DOLPHIN_BUTTON_WIDTH * span) / 2),
        top + (int)(DTMF_DOLPHIN_BUTTON_HEIGHT / 2),
        AlignCenter,
        AlignCenter,
        dtmf_dolphin_data_get_tone_name(row, col));

    if(invert) canvas_invert_color(canvas);
}

void draw_dialer(Canvas* canvas, void* _model) {
    DTMFDolphinDialerModel* model = _model;
    uint8_t max_rows;
    uint8_t max_cols;
    uint8_t max_span;
    dtmf_dolphin_tone_get_max_pos(&max_rows, &max_cols, &max_span);

    canvas_set_font(canvas, FontSecondary);

    for(int r = 0; r < max_rows; r++) {
        for(int c = 0; c < max_cols; c++) {
            if(model->row == r && model->col == c)
                draw_button(canvas, r, c, true);
            else
                draw_button(canvas, r, c, false);
        }
    }
}

void update_frequencies(DTMFDolphinDialerModel* model) {
    dtmf_dolphin_data_get_tone_frequencies(&model->freq1, &model->freq2, model->row, model->col);
    dtmf_dolphin_data_get_filter_data(
        &model->pulses, &model->pulse_ms, &model->gap_ms, model->row, model->col);
}

static void dtmf_dolphin_dialer_draw_callback(Canvas* canvas, void* _model) {
    DTMFDolphinDialerModel* model = _model;
    if(model->playing) {
        // Leverage the prioritized draw callback to handle
        // the DMA so that it doesn't skip.
        dtmf_dolphin_audio_handle_tick();
        // Don't do any drawing if audio is playing.
        canvas_set_font(canvas, FontPrimary);
        elements_multiline_text_aligned(
            canvas,
            canvas_width(canvas) / 2,
            canvas_height(canvas) / 2,
            AlignCenter,
            AlignCenter,
            "Playing Tones");
        return;
    }
    update_frequencies(model);
    uint8_t max_rows = 0;
    uint8_t max_cols = 0;
    uint8_t max_span = 0;
    dtmf_dolphin_tone_get_max_pos(&max_rows, &max_cols, &max_span);

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text(canvas, 2, 10, dtmf_dolphin_data_get_current_section_name());
    canvas_draw_line(
        canvas,
        (max_span * DTMF_DOLPHIN_BUTTON_WIDTH) + 1,
        0,
        (max_span * DTMF_DOLPHIN_BUTTON_WIDTH) + 1,
        canvas_height(canvas));
    elements_multiline_text(canvas, (max_span * DTMF_DOLPHIN_BUTTON_WIDTH) + 4, 10, "Detail");
    canvas_draw_line(
        canvas, 0, DTMF_DOLPHIN_NUMPAD_Y - 3, canvas_width(canvas), DTMF_DOLPHIN_NUMPAD_Y - 3);
    // elements_multiline_text_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Dialer Mode");

    draw_dialer(canvas, model);

    FuriString* output = furi_string_alloc();

    if(model->freq1 && model->freq2) {
        furi_string_cat_printf(
            output,
            "Dual Tone\nF1: %u Hz\nF2: %u Hz\n",
            (unsigned int)model->freq1,
            (unsigned int)model->freq2);
    } else if(model->freq1) {
        furi_string_cat_printf(output, "Single Tone\nF: %u Hz\n", (unsigned int)model->freq1);
    }

    canvas_set_font(canvas, FontSecondary);
    canvas_set_color(canvas, ColorBlack);
    if(model->pulse_ms) {
        furi_string_cat_printf(output, "P: %u * %u ms\n", model->pulses, model->pulse_ms);
    }
    if(model->gap_ms) {
        furi_string_cat_printf(output, "Gaps: %u ms\n", model->gap_ms);
    }
    elements_multiline_text(
        canvas, (max_span * DTMF_DOLPHIN_BUTTON_WIDTH) + 4, 21, furi_string_get_cstr(output));

    furi_string_free(output);
}

static bool dtmf_dolphin_dialer_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    DTMFDolphinDialer* dtmf_dolphin_dialer = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyRight) {
            consumed = dtmf_dolphin_dialer_process_right(dtmf_dolphin_dialer);
        } else if(event->key == InputKeyLeft) {
            consumed = dtmf_dolphin_dialer_process_left(dtmf_dolphin_dialer);
        } else if(event->key == InputKeyUp) {
            consumed = dtmf_dolphin_dialer_process_up(dtmf_dolphin_dialer);
        } else if(event->key == InputKeyDown) {
            consumed = dtmf_dolphin_dialer_process_down(dtmf_dolphin_dialer);
        }

    } else if(event->key == InputKeyOk) {
        consumed = dtmf_dolphin_dialer_process_ok(dtmf_dolphin_dialer, event);
    }

    return consumed;
}

static bool dtmf_dolphin_dialer_process_up(DTMFDolphinDialer* dtmf_dolphin_dialer) {
    with_view_model(
        dtmf_dolphin_dialer->view,
        DTMFDolphinDialerModel * model,
        {
            uint8_t span = 0;
            uint8_t cursor = model->row;
            while(span == 0 && cursor > 0) {
                cursor--;
                span = dtmf_dolphin_get_tone_span(cursor, model->col);
            }
            if(span != 0) {
                model->row = cursor;
            }
        },
        true);
    return true;
}

static bool dtmf_dolphin_dialer_process_down(DTMFDolphinDialer* dtmf_dolphin_dialer) {
    uint8_t max_rows = 0;
    uint8_t max_cols = 0;
    uint8_t max_span = 0;
    dtmf_dolphin_tone_get_max_pos(&max_rows, &max_cols, &max_span);

    with_view_model(
        dtmf_dolphin_dialer->view,
        DTMFDolphinDialerModel * model,
        {
            uint8_t span = 0;
            uint8_t cursor = model->row;
            while(span == 0 && cursor < max_rows - 1) {
                cursor++;
                span = dtmf_dolphin_get_tone_span(cursor, model->col);
            }
            if(span != 0) {
                model->row = cursor;
            }
        },
        true);
    return true;
}

static bool dtmf_dolphin_dialer_process_left(DTMFDolphinDialer* dtmf_dolphin_dialer) {
    with_view_model(
        dtmf_dolphin_dialer->view,
        DTMFDolphinDialerModel * model,
        {
            uint8_t span = 0;
            uint8_t cursor = model->col;
            while(span == 0 && cursor > 0) {
                cursor--;
                span = dtmf_dolphin_get_tone_span(model->row, cursor);
            }
            if(span != 0) {
                model->col = cursor;
            }
        },
        true);
    return true;
}

static bool dtmf_dolphin_dialer_process_right(DTMFDolphinDialer* dtmf_dolphin_dialer) {
    uint8_t max_rows = 0;
    uint8_t max_cols = 0;
    uint8_t max_span = 0;
    dtmf_dolphin_tone_get_max_pos(&max_rows, &max_cols, &max_span);

    with_view_model(
        dtmf_dolphin_dialer->view,
        DTMFDolphinDialerModel * model,
        {
            uint8_t span = 0;
            uint8_t cursor = model->col;
            while(span == 0 && cursor < max_cols - 1) {
                cursor++;
                span = dtmf_dolphin_get_tone_span(model->row, cursor);
            }
            if(span != 0) {
                model->col = cursor;
            }
        },
        true);
    return true;
}

static bool
    dtmf_dolphin_dialer_process_ok(DTMFDolphinDialer* dtmf_dolphin_dialer, InputEvent* event) {
    bool consumed = false;

    with_view_model(
        dtmf_dolphin_dialer->view,
        DTMFDolphinDialerModel * model,
        {
            if(event->type == InputTypePress) {
                model->playing = dtmf_dolphin_audio_play_tones(
                    model->freq1, model->freq2, model->pulses, model->pulse_ms, model->gap_ms);
            } else if(event->type == InputTypeRelease) {
                model->playing = !dtmf_dolphin_audio_stop_tones();
            }
        },
        true);

    return consumed;
}

static void dtmf_dolphin_dialer_enter_callback(void* context) {
    furi_assert(context);
    DTMFDolphinDialer* dtmf_dolphin_dialer = context;

    with_view_model(
        dtmf_dolphin_dialer->view,
        DTMFDolphinDialerModel * model,
        {
            model->col = 0;
            model->row = 0;
            model->section = 0;
            model->freq1 = 0.0;
            model->freq2 = 0.0;
            model->playing = false;
        },
        true);
}

DTMFDolphinDialer* dtmf_dolphin_dialer_alloc() {
    DTMFDolphinDialer* dtmf_dolphin_dialer = malloc(sizeof(DTMFDolphinDialer));

    dtmf_dolphin_dialer->view = view_alloc();
    view_allocate_model(
        dtmf_dolphin_dialer->view, ViewModelTypeLocking, sizeof(DTMFDolphinDialerModel));

    with_view_model(
        dtmf_dolphin_dialer->view,
        DTMFDolphinDialerModel * model,
        {
            model->col = 0;
            model->row = 0;
            model->section = 0;
            model->freq1 = 0.0;
            model->freq2 = 0.0;
            model->playing = false;
        },
        true);

    view_set_context(dtmf_dolphin_dialer->view, dtmf_dolphin_dialer);
    view_set_draw_callback(dtmf_dolphin_dialer->view, dtmf_dolphin_dialer_draw_callback);
    view_set_input_callback(dtmf_dolphin_dialer->view, dtmf_dolphin_dialer_input_callback);
    view_set_enter_callback(dtmf_dolphin_dialer->view, dtmf_dolphin_dialer_enter_callback);
    return dtmf_dolphin_dialer;
}

void dtmf_dolphin_dialer_free(DTMFDolphinDialer* dtmf_dolphin_dialer) {
    furi_assert(dtmf_dolphin_dialer);
    view_free(dtmf_dolphin_dialer->view);
    free(dtmf_dolphin_dialer);
}

View* dtmf_dolphin_dialer_get_view(DTMFDolphinDialer* dtmf_dolphin_dialer) {
    furi_assert(dtmf_dolphin_dialer);
    return dtmf_dolphin_dialer->view;
}
