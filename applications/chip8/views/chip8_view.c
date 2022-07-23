#include <gui/elements.h>
#include "chip8_view.h"
#include "../chip8.h"
#include "../emulator_core/flipper_chip.h"

struct Chip8View {
    View* view;
    Chip8ViewCallback callback;
    void* context;
    Chip8ViewKeyBackCallback backCallback;
    Chip8ViewKeyUpCallback upCallback;
    Chip8ViewKeyDownCallback downCallback;
    Chip8ViewReleaseCallback releaseCallback;
};

typedef struct {
    string_t file_name;
    Chip8State state;
    uint8_t** backup_screen;
} Chip8Model;

static void chip8_draw_callback(Canvas* canvas, void* _model) {
    Chip8Model* model = _model;

    if(model->state.worker_state == WorkerStateLoadingRom) {
        canvas_draw_icon(canvas, 4, 22, &I_Clock_18x18);
    }

    if(model->state.worker_state == WorkerStateRomLoaded) {
        while(!model->state.t_chip8_state->go_render) {
            for(int y = 0; y < CHIP8_SCREEN_H; y++) {
                for(int x = 0; x < CHIP8_SCREEN_W; x++) {
                    if(model->backup_screen[y][x] == 0) {
                        canvas_set_color(canvas, ColorWhite);
                    } else {
                        canvas_set_color(canvas, ColorBlack);
                    }
                    canvas_draw_box(canvas, x * 2, y * 2, 2, 2);
                    //canvas_draw_dot(canvas, x, y);
                }
            }
            return;
        }

        uint8_t** screen = t_chip8_get_screen(model->state.t_chip8_state);

        for(int y = 0; y < CHIP8_SCREEN_H; y++) {
            for(int x = 0; x < CHIP8_SCREEN_W; x++) {
                if(screen[y][x] == 0) {
                    canvas_set_color(canvas, ColorWhite);
                } else {
                    canvas_set_color(canvas, ColorBlack);
                }
                canvas_draw_box(canvas, x * 2, y * 2, 2, 2);
                model->backup_screen[y][x] = screen[y][x];
                //canvas_draw_dot(canvas, x, y);
            }
        }
        model->state.t_chip8_state->go_render = false;
    }

    if(model->state.worker_state == WorkerStateRomLoadError) {
        canvas_draw_icon(canvas, 4, 22, &I_Error_18x18);
    }
}

static bool chip8_input_callback(InputEvent* event, void* context) {
    FURI_LOG_I("Chip8", "received input");
    furi_assert(context);
    Chip8View* chip8 = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyOk) {
            consumed = true;
            furi_assert(chip8->callback);
            chip8->callback(InputTypeShort, chip8->context);
        }

        if(event->key == InputKeyBack) {
            consumed = true;
            furi_assert(chip8->callback);
            chip8->backCallback(chip8, InputTypeShort, chip8->context);
        }

        if(event->key == InputKeyUp) {
            consumed = true;
            furi_assert(chip8->upCallback);
            chip8->upCallback(InputTypeShort, chip8->context);
        }
        if(event->key == InputKeyDown) {
            consumed = true;
            furi_assert(chip8->downCallback);
            chip8->downCallback(InputTypeShort, chip8->context);
        }
    }
    if(event->type == InputTypeRelease) {
        chip8->releaseCallback(InputTypeShort, chip8->context);
    }

    return consumed;
}

Chip8View* chip8_alloc() {
    Chip8View* chip8 = malloc(sizeof(Chip8View));

    chip8->view = view_alloc();
    view_allocate_model(chip8->view, ViewModelTypeLocking, sizeof(Chip8Model));
    view_set_context(chip8->view, chip8);
    view_set_draw_callback(chip8->view, chip8_draw_callback);
    view_set_input_callback(chip8->view, chip8_input_callback);

    return chip8;
}

void chip8_free(Chip8View* chip8) {
    furi_assert(chip8);
    view_free(chip8->view);
    free(chip8);
}

View* chip8_get_view(Chip8View* chip8) {
    furi_assert(chip8);
    return chip8->view;
}

void chip8_set_ok_callback(Chip8View* chip8, Chip8ViewCallback callback, void* context) {
    furi_assert(chip8);
    furi_assert(callback);
    with_view_model(
        chip8->view, (Chip8Model * model) {
            chip8->callback = callback;
            chip8->context = context;
            return false;
        });
}

void chip8_set_back_callback(Chip8View* chip8, Chip8ViewKeyBackCallback callback, void* context) {
    furi_assert(chip8);
    furi_assert(callback);
    with_view_model(
        chip8->view, (Chip8Model * model) {
            chip8->backCallback = callback;
            chip8->context = context;
            return true;
        });
}

void chip8_set_up_callback(Chip8View* chip8, Chip8ViewKeyUpCallback callback, void* context) {
    furi_assert(chip8);
    furi_assert(callback);
    with_view_model(
        chip8->view, (Chip8Model * model) {
            chip8->upCallback = callback;
            chip8->context = context;
            return true;
        });
}

void chip8_set_down_callback(Chip8View* chip8, Chip8ViewKeyDownCallback callback, void* context) {
    furi_assert(chip8);
    furi_assert(callback);
    with_view_model(
        chip8->view, (Chip8Model * model) {
            chip8->downCallback = callback;
            chip8->context = context;
            return true;
        });
}

void chip8_set_file_name(Chip8View* chip8, string_t name) {
    furi_assert(name);
    with_view_model(
        chip8->view, (Chip8Model * model) {
            *model->file_name = *name;
            return false;
        });
}

void chip8_set_backup_screen(Chip8View* chip8, uint8_t** screen) {
    furi_assert(screen);
    with_view_model(
        chip8->view, (Chip8Model * model) {
            model->backup_screen = screen;
            return false;
        });
}

void chip8_set_state(Chip8View* chip8, Chip8State* st) {
    furi_assert(st);
    with_view_model(
        chip8->view, (Chip8Model * model) {
            memcpy(&(model->state), st, sizeof(Chip8State));
            return true;
        });
}

void chip8_set_release_callback(Chip8View* chip8, Chip8ViewReleaseCallback callback, void* context) {
    furi_assert(chip8);
    furi_assert(callback);
    with_view_model(
        chip8->view, (Chip8Model * model) {
            chip8->releaseCallback = callback;
            chip8->context = context;
            return true;
        });
}
