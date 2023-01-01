#include "morse_code_worker.h"
#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>
#include <stdlib.h>
#include <furi_hal.h>
#include <string.h>

static const float MORSE_CODE_VOLUMES[] = {0, .25, .5, .75, 1};

typedef struct {
    FuriString* words;
    uint8_t volume;
    uint32_t dit_delta;
} MorseCodeModel;

typedef struct {
    MorseCodeModel* model;
    FuriMutex** model_mutex;

    FuriMessageQueue* input_queue;

    ViewPort* view_port;
    Gui* gui;

    MorseCodeWorker* worker;
} MorseCode;

static void render_callback(Canvas* const canvas, void* ctx) {
    MorseCode* morse_code = ctx;
    furi_check(furi_mutex_acquire(morse_code->model_mutex, FuriWaitForever) == FuriStatusOk);
    // border around the edge of the screen
    canvas_set_font(canvas, FontPrimary);

    //write words
    elements_multiline_text_aligned(
        canvas, 64, 30, AlignCenter, AlignCenter, furi_string_get_cstr(morse_code->model->words));

    // volume view_port
    uint8_t vol_bar_x_pos = 124;
    uint8_t vol_bar_y_pos = 0;
    const uint8_t volume_h = (64 / (COUNT_OF(MORSE_CODE_VOLUMES) - 1)) * morse_code->model->volume;
    canvas_draw_frame(canvas, vol_bar_x_pos, vol_bar_y_pos, 4, 64);
    canvas_draw_box(canvas, vol_bar_x_pos, vol_bar_y_pos + (64 - volume_h), 4, volume_h);

    //dit bpms
    FuriString* ditbpm = furi_string_alloc_printf("Dit: %ld ms", morse_code->model->dit_delta);
    canvas_draw_str_aligned(canvas, 0, 10, AlignLeft, AlignCenter, furi_string_get_cstr(ditbpm));
    furi_string_free(ditbpm);

    //button info
    elements_button_center(canvas, "Press/Hold");
    furi_mutex_release(morse_code->model_mutex);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    MorseCode* morse_code = ctx;
    furi_message_queue_put(morse_code->input_queue, input_event, FuriWaitForever);
}

static void morse_code_worker_callback(FuriString* words, void* context) {
    MorseCode* morse_code = context;
    furi_check(furi_mutex_acquire(morse_code->model_mutex, FuriWaitForever) == FuriStatusOk);
    furi_string_set(morse_code->model->words, words);
    furi_mutex_release(morse_code->model_mutex);
    view_port_update(morse_code->view_port);
}

MorseCode* morse_code_alloc() {
    MorseCode* instance = malloc(sizeof(MorseCode));

    instance->model = malloc(sizeof(MorseCodeModel));
    instance->model->words = furi_string_alloc_set_str("");
    instance->model->volume = 3;
    instance->model->dit_delta = 150;
    instance->model_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    instance->worker = morse_code_worker_alloc();

    morse_code_worker_set_callback(instance->worker, morse_code_worker_callback, instance);

    instance->view_port = view_port_alloc();
    view_port_draw_callback_set(instance->view_port, render_callback, instance);
    view_port_input_callback_set(instance->view_port, input_callback, instance);

    // Open GUI and register view_port
    instance->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(instance->gui, instance->view_port, GuiLayerFullscreen);

    return instance;
}

void morse_code_free(MorseCode* instance) {
    gui_remove_view_port(instance->gui, instance->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(instance->view_port);

    morse_code_worker_free(instance->worker);

    furi_message_queue_free(instance->input_queue);

    furi_mutex_free(instance->model_mutex);

    furi_string_free(instance->model->words);
    free(instance->model);
    free(instance);
}

int32_t morse_code_app() {
    MorseCode* morse_code = morse_code_alloc();
    InputEvent input;

    morse_code_worker_start(morse_code->worker);
    morse_code_worker_set_volume(
        morse_code->worker, MORSE_CODE_VOLUMES[morse_code->model->volume]);
    morse_code_worker_set_dit_delta(morse_code->worker, morse_code->model->dit_delta);

    while(furi_message_queue_get(morse_code->input_queue, &input, FuriWaitForever) ==
          FuriStatusOk) {
        furi_check(furi_mutex_acquire(morse_code->model_mutex, FuriWaitForever) == FuriStatusOk);
        if(input.key == InputKeyBack && input.type == InputTypeLong) {
            furi_mutex_release(morse_code->model_mutex);
            break;
        } else if(input.key == InputKeyBack && input.type == InputTypeShort) {
            morse_code_worker_reset_text(morse_code->worker);
            furi_string_reset(morse_code->model->words);
        } else if(input.key == InputKeyOk) {
            if(input.type == InputTypePress)
                morse_code_worker_play(morse_code->worker, true);
            else if(input.type == InputTypeRelease)
                morse_code_worker_play(morse_code->worker, false);
        } else if(input.key == InputKeyUp && input.type == InputTypePress) {
            if(morse_code->model->volume < COUNT_OF(MORSE_CODE_VOLUMES) - 1)
                morse_code->model->volume++;
            morse_code_worker_set_volume(
                morse_code->worker, MORSE_CODE_VOLUMES[morse_code->model->volume]);
        } else if(input.key == InputKeyDown && input.type == InputTypePress) {
            if(morse_code->model->volume > 0) morse_code->model->volume--;
            morse_code_worker_set_volume(
                morse_code->worker, MORSE_CODE_VOLUMES[morse_code->model->volume]);
        } else if(input.key == InputKeyLeft && input.type == InputTypePress) {
            if(morse_code->model->dit_delta > 10) morse_code->model->dit_delta -= 10;
            morse_code_worker_set_dit_delta(morse_code->worker, morse_code->model->dit_delta);
        } else if(input.key == InputKeyRight && input.type == InputTypePress) {
            if(morse_code->model->dit_delta >= 10) morse_code->model->dit_delta += 10;
            morse_code_worker_set_dit_delta(morse_code->worker, morse_code->model->dit_delta);
        }

        FURI_LOG_D(
            "Input",
            "%s %s %ld",
            input_get_key_name(input.key),
            input_get_type_name(input.type),
            input.sequence);

        furi_mutex_release(morse_code->model_mutex);
        view_port_update(morse_code->view_port);
    }

    morse_code_worker_stop(morse_code->worker);
    morse_code_free(morse_code);
    return 0;
}