#include "wav_player_view.h"

float map(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min + 1) / (in_max - in_min + 1) + out_min;
}

static void wav_player_view_draw_callback(Canvas* canvas, void* _model) {
    WavPlayerViewModel* model = _model;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    uint8_t x_pos = 0;
    uint8_t y_pos = 0;

    /*char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", model->num_channels);
    canvas_draw_str(canvas, 0, 10, buffer);
    snprintf(buffer, sizeof(buffer), "%d", model->bits_per_sample);
    canvas_draw_str(canvas, 0, 20, buffer);*/

    // volume
    x_pos = 123;
    y_pos = 0;
    const float volume = (64 / 10.0f) * model->volume;
    canvas_draw_frame(canvas, x_pos, y_pos, 4, 64);
    canvas_draw_box(canvas, x_pos, y_pos + (64 - volume), 4, volume);

    // play / pause
    x_pos = 58;
    y_pos = 55;
    if(!model->play) {
        canvas_draw_line(canvas, x_pos, y_pos, x_pos + 8, y_pos + 4);
        canvas_draw_line(canvas, x_pos, y_pos + 8, x_pos + 8, y_pos + 4);
        canvas_draw_line(canvas, x_pos, y_pos + 8, x_pos, y_pos);
    } else {
        canvas_draw_box(canvas, x_pos, y_pos, 3, 9);
        canvas_draw_box(canvas, x_pos + 4, y_pos, 3, 9);
    }

    x_pos = 78;
    y_pos = 55;
    canvas_draw_line(canvas, x_pos, y_pos, x_pos + 4, y_pos + 4);
    canvas_draw_line(canvas, x_pos, y_pos + 8, x_pos + 4, y_pos + 4);
    canvas_draw_line(canvas, x_pos, y_pos + 8, x_pos, y_pos);

    x_pos = 82;
    y_pos = 55;
    canvas_draw_line(canvas, x_pos, y_pos, x_pos + 4, y_pos + 4);
    canvas_draw_line(canvas, x_pos, y_pos + 8, x_pos + 4, y_pos + 4);
    canvas_draw_line(canvas, x_pos, y_pos + 8, x_pos, y_pos);

    x_pos = 40;
    y_pos = 55;
    canvas_draw_line(canvas, x_pos, y_pos, x_pos - 4, y_pos + 4);
    canvas_draw_line(canvas, x_pos, y_pos + 8, x_pos - 4, y_pos + 4);
    canvas_draw_line(canvas, x_pos, y_pos + 8, x_pos, y_pos);

    x_pos = 44;
    y_pos = 55;
    canvas_draw_line(canvas, x_pos, y_pos, x_pos - 4, y_pos + 4);
    canvas_draw_line(canvas, x_pos, y_pos + 8, x_pos - 4, y_pos + 4);
    canvas_draw_line(canvas, x_pos, y_pos + 8, x_pos, y_pos);

    // len
    x_pos = 4;
    y_pos = 47;
    const uint8_t play_len = 116;
    uint8_t play_pos = map(model->current, model->start, model->end, 0, play_len - 4);

    canvas_draw_frame(canvas, x_pos, y_pos, play_len, 4);
    canvas_draw_box(canvas, x_pos + play_pos, y_pos - 2, 4, 8);
    canvas_draw_box(canvas, x_pos, y_pos, play_pos, 4);

    // osc
    x_pos = 4;
    y_pos = 0;
    for(size_t i = 1; i < DATA_COUNT; i++) {
        canvas_draw_line(canvas, x_pos + i - 1, model->data[i - 1], x_pos + i, model->data[i]);
    }
}

static bool wav_player_view_input_callback(InputEvent* event, void* context) {
    WavPlayerView* wav_player_view = context;
    bool consumed = false;

    if(wav_player_view->callback) {
        if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
            if(event->key == InputKeyUp) {
                wav_player_view->callback(WavPlayerCtrlVolUp, wav_player_view->context);
                consumed = true;
            } else if(event->key == InputKeyDown) {
                wav_player_view->callback(WavPlayerCtrlVolDn, wav_player_view->context);
                consumed = true;
            } else if(event->key == InputKeyLeft) {
                wav_player_view->callback(WavPlayerCtrlMoveL, wav_player_view->context);
                consumed = true;
            } else if(event->key == InputKeyRight) {
                wav_player_view->callback(WavPlayerCtrlMoveR, wav_player_view->context);
                consumed = true;
            } else if(event->key == InputKeyOk) {
                wav_player_view->callback(WavPlayerCtrlOk, wav_player_view->context);
                consumed = true;
            } else if(event->key == InputKeyBack) {
                wav_player_view->callback(WavPlayerCtrlBack, wav_player_view->context);
                consumed = true;
            }
        }
    }

    return consumed;
}

WavPlayerView* wav_player_view_alloc() {
    WavPlayerView* wav_view = malloc(sizeof(WavPlayerView));
    wav_view->view = view_alloc();
    view_set_context(wav_view->view, wav_view);
    view_allocate_model(wav_view->view, ViewModelTypeLocking, sizeof(WavPlayerViewModel));
    view_set_draw_callback(wav_view->view, wav_player_view_draw_callback);
    view_set_input_callback(wav_view->view, wav_player_view_input_callback);

    return wav_view;
}

void wav_player_view_free(WavPlayerView* wav_view) {
    furi_assert(wav_view);
    view_free(wav_view->view);
    free(wav_view);
}

View* wav_player_view_get_view(WavPlayerView* wav_view) {
    furi_assert(wav_view);
    return wav_view->view;
}

void wav_player_view_set_volume(WavPlayerView* wav_view, float volume) {
    furi_assert(wav_view);
    with_view_model(
        wav_view->view, WavPlayerViewModel * model, { model->volume = volume; }, true);
}

void wav_player_view_set_start(WavPlayerView* wav_view, size_t start) {
    furi_assert(wav_view);
    with_view_model(
        wav_view->view, WavPlayerViewModel * model, { model->start = start; }, true);
}

void wav_player_view_set_end(WavPlayerView* wav_view, size_t end) {
    furi_assert(wav_view);
    with_view_model(
        wav_view->view, WavPlayerViewModel * model, { model->end = end; }, true);
}

void wav_player_view_set_current(WavPlayerView* wav_view, size_t current) {
    furi_assert(wav_view);
    with_view_model(
        wav_view->view, WavPlayerViewModel * model, { model->current = current; }, true);
}

void wav_player_view_set_play(WavPlayerView* wav_view, bool play) {
    furi_assert(wav_view);
    with_view_model(
        wav_view->view, WavPlayerViewModel * model, { model->play = play; }, true);
}

void wav_player_view_set_chans(WavPlayerView* wav_view, uint16_t chn) {
    furi_assert(wav_view);
    with_view_model(
        wav_view->view, WavPlayerViewModel * model, { model->num_channels = chn; }, true);
}

void wav_player_view_set_bits(WavPlayerView* wav_view, uint16_t bit) {
    furi_assert(wav_view);
    with_view_model(
        wav_view->view, WavPlayerViewModel * model, { model->bits_per_sample = bit; }, true);
}

void wav_player_view_set_data(WavPlayerView* wav_view, uint16_t* data, size_t data_count) {
    furi_assert(wav_view);
    with_view_model(
        wav_view->view,
        WavPlayerViewModel * model,
        {
            size_t inc = (data_count / DATA_COUNT) - 1;

            for(size_t i = 0; i < DATA_COUNT; i++) {
                model->data[i] = *data / 6;
                if(model->data[i] > 42) model->data[i] = 42;
                data += inc;
            }
        },
        true);
}

void wav_player_view_set_ctrl_callback(WavPlayerView* wav_view, WavPlayerCtrlCallback callback) {
    furi_assert(wav_view);
    wav_view->callback = callback;
}

void wav_player_view_set_context(WavPlayerView* wav_view, void* context) {
    furi_assert(wav_view);
    wav_view->context = context;
}