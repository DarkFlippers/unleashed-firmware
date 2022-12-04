#include "tracker_view.h"
#include <gui/elements.h>
#include <furi.h>

typedef struct {
    const Song* song;
    uint8_t order_list_index;
    uint8_t row;
} TrackerViewModel;

struct TrackerView {
    View* view;
    void* back_context;
    TrackerViewCallback back_callback;
};

static Channel* get_current_channel(TrackerViewModel* model) {
    uint8_t channel_id = 0;
    uint8_t pattern_id = model->song->order_list[model->order_list_index];
    Pattern* pattern = &model->song->patterns[pattern_id];
    return &pattern->channels[channel_id];
}

static const char* get_note_from_id(uint8_t note) {
#define NOTE_COUNT 12
    const char* notes[NOTE_COUNT] = {
        "C ",
        "C#",
        "D ",
        "D#",
        "E ",
        "F ",
        "F#",
        "G ",
        "G#",
        "A ",
        "A#",
        "B ",
    };
    return notes[(note) % NOTE_COUNT];
#undef NOTE_COUNT
}

static uint8_t get_octave_from_id(uint8_t note) {
    return ((note) / 12) + 2;
}

static uint8_t get_first_row_id(uint8_t row) {
    return (row / 10) * 10;
}

static void
    draw_row(Canvas* canvas, uint8_t i, Channel* channel, uint8_t row, FuriString* buffer) {
    uint8_t x = 12 * (i + 1);
    uint8_t first_row_id = get_first_row_id(row);
    uint8_t current_row_id = first_row_id + i;

    if((current_row_id) >= 64) {
        return;
    }

    Row current_row = channel->rows[current_row_id];
    uint8_t note = current_row & ROW_NOTE_MASK;
    uint8_t effect = (current_row >> 6) & ROW_EFFECT_MASK;
    uint8_t data = (current_row >> 10) & ROW_EFFECT_DATA_MASK;

    if(current_row_id == row) {
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_line(canvas, x - 9, 1, x - 9, 62);
        canvas_draw_box(canvas, x - 8, 0, 9, 64);
        canvas_draw_line(canvas, x + 1, 1, x + 1, 62);
        canvas_set_color(canvas, ColorWhite);
    }

    furi_string_printf(buffer, "%02X", current_row_id);
    canvas_draw_str(canvas, x, 61, furi_string_get_cstr(buffer));

    if(note > 0 && note < NOTE_OFF) {
        furi_string_printf(
            buffer, "%s%d", get_note_from_id(note - 1), get_octave_from_id(note - 1));
        canvas_draw_str(canvas, x, 44, furi_string_get_cstr(buffer));
    } else if(note == NOTE_OFF) {
        canvas_draw_str(canvas, x, 44, "OFF");
    } else {
        canvas_draw_str(canvas, x, 44, "---");
    }

    if(effect == 0 && data == 0) {
        canvas_draw_str(canvas, x, 21, "-");
        canvas_draw_str(canvas, x, 12, "--");
    } else {
        furi_string_printf(buffer, "%X", effect);
        canvas_draw_str(canvas, x, 21, furi_string_get_cstr(buffer));

        if(effect == EffectArpeggio || effect == EffectVibrato) {
            uint8_t data_x = EFFECT_DATA_GET_X(data);
            uint8_t data_y = EFFECT_DATA_GET_Y(data);
            furi_string_printf(buffer, "%d%d", data_x, data_y);
            canvas_draw_str(canvas, x, 12, furi_string_get_cstr(buffer));
        } else {
            furi_string_printf(buffer, "%02X", data);
            canvas_draw_str(canvas, x, 12, furi_string_get_cstr(buffer));
        }
    }

    if(current_row_id == row) {
        canvas_set_color(canvas, ColorBlack);
    }
}

static void tracker_view_draw_callback(Canvas* canvas, void* _model) {
    TrackerViewModel* model = _model;
    if(model->song == NULL) {
        return;
    }

    canvas_set_font_direction(canvas, CanvasDirectionBottomToTop);
    canvas_set_font(canvas, FontKeyboard);

    Channel* channel = get_current_channel(model);
    FuriString* buffer = furi_string_alloc();

    for(uint8_t i = 0; i < 10; i++) {
        draw_row(canvas, i, channel, model->row, buffer);
    }
    furi_string_free(buffer);
}

static bool tracker_view_input_callback(InputEvent* event, void* context) {
    TrackerView* tracker_view = context;

    if(tracker_view->back_callback) {
        if(event->type == InputTypeShort && event->key == InputKeyBack) {
            tracker_view->back_callback(tracker_view->back_context);
            return true;
        }
    }
    return false;
}

TrackerView* tracker_view_alloc() {
    TrackerView* tracker_view = malloc(sizeof(TrackerView));
    tracker_view->view = view_alloc();
    view_allocate_model(tracker_view->view, ViewModelTypeLocking, sizeof(TrackerViewModel));
    view_set_context(tracker_view->view, tracker_view);
    view_set_draw_callback(tracker_view->view, (ViewDrawCallback)tracker_view_draw_callback);
    view_set_input_callback(tracker_view->view, (ViewInputCallback)tracker_view_input_callback);
    return tracker_view;
}

void tracker_view_free(TrackerView* tracker_view) {
    view_free(tracker_view->view);
    free(tracker_view);
}

View* tracker_view_get_view(TrackerView* tracker_view) {
    return tracker_view->view;
}

void tracker_view_set_back_callback(
    TrackerView* tracker_view,
    TrackerViewCallback callback,
    void* context) {
    tracker_view->back_callback = callback;
    tracker_view->back_context = context;
}

void tracker_view_set_song(TrackerView* tracker_view, const Song* song) {
    with_view_model(
        tracker_view->view, TrackerViewModel * model, { model->song = song; }, true);
}

void tracker_view_set_position(TrackerView* tracker_view, uint8_t order_list_index, uint8_t row) {
    with_view_model(
        tracker_view->view,
        TrackerViewModel * model,
        {
            model->order_list_index = order_list_index;
            model->row = row;
        },
        true);
}