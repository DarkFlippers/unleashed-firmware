#include "music_beeper_worker.h"

#include <furi.h>
#include <furi_hal.h>

#include <Music_Beeper_icons.h>
#include <gui/gui.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

#define TAG "MusicBeeper"

#define MUSIC_BEEPER_APP_PATH_FOLDER ANY_PATH("music_player")
#define MUSIC_BEEPER_APP_EXTENSION "*"

#define MUSIC_BEEPER_SEMITONE_HISTORY_SIZE 4

typedef struct {
    uint8_t semitone_history[MUSIC_BEEPER_SEMITONE_HISTORY_SIZE];
    uint8_t duration_history[MUSIC_BEEPER_SEMITONE_HISTORY_SIZE];

    uint8_t volume;
    uint8_t semitone;
    uint8_t dots;
    uint8_t duration;
    float position;
} MusicBeeperModel;

typedef struct {
    MusicBeeperModel* model;
    FuriMutex** model_mutex;

    FuriMessageQueue* input_queue;

    ViewPort* view_port;
    Gui* gui;

    MusicBeeperWorker* worker;
} MusicBeeper;

static const float MUSIC_BEEPER_VOLUMES[] = {0, .25, .5, .75, 1};

static const char* semitone_to_note(int8_t semitone) {
    switch(semitone) {
    case 0:
        return "C";
    case 1:
        return "C#";
    case 2:
        return "D";
    case 3:
        return "D#";
    case 4:
        return "E";
    case 5:
        return "F";
    case 6:
        return "F#";
    case 7:
        return "G";
    case 8:
        return "G#";
    case 9:
        return "A";
    case 10:
        return "A#";
    case 11:
        return "B";
    default:
        return "--";
    }
}

static bool is_white_note(uint8_t semitone, uint8_t id) {
    switch(semitone) {
    case 0:
        if(id == 0) return true;
        break;
    case 2:
        if(id == 1) return true;
        break;
    case 4:
        if(id == 2) return true;
        break;
    case 5:
        if(id == 3) return true;
        break;
    case 7:
        if(id == 4) return true;
        break;
    case 9:
        if(id == 5) return true;
        break;
    case 11:
        if(id == 6) return true;
        break;
    default:
        break;
    }

    return false;
}

static bool is_black_note(uint8_t semitone, uint8_t id) {
    switch(semitone) {
    case 1:
        if(id == 0) return true;
        break;
    case 3:
        if(id == 1) return true;
        break;
    case 6:
        if(id == 3) return true;
        break;
    case 8:
        if(id == 4) return true;
        break;
    case 10:
        if(id == 5) return true;
        break;
    default:
        break;
    }

    return false;
}

static void render_callback(Canvas* canvas, void* ctx) {
    MusicBeeper* music_beeper = ctx;
    furi_check(furi_mutex_acquire(music_beeper->model_mutex, FuriWaitForever) == FuriStatusOk);

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 12, "MusicBeeper");

    uint8_t x_pos = 0;
    uint8_t y_pos = 24;
    const uint8_t white_w = 10;
    const uint8_t white_h = 40;

    const int8_t black_x = 6;
    const int8_t black_y = -5;
    const uint8_t black_w = 8;
    const uint8_t black_h = 32;

    // white keys
    for(size_t i = 0; i < 7; i++) {
        if(is_white_note(music_beeper->model->semitone, i)) {
            canvas_draw_box(canvas, x_pos + white_w * i, y_pos, white_w + 1, white_h);
        } else {
            canvas_draw_frame(canvas, x_pos + white_w * i, y_pos, white_w + 1, white_h);
        }
    }

    // black keys
    for(size_t i = 0; i < 7; i++) {
        if(i != 2 && i != 6) {
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_box(
                canvas, x_pos + white_w * i + black_x, y_pos + black_y, black_w + 1, black_h);
            canvas_set_color(canvas, ColorBlack);
            if(is_black_note(music_beeper->model->semitone, i)) {
                canvas_draw_box(
                    canvas, x_pos + white_w * i + black_x, y_pos + black_y, black_w + 1, black_h);
            } else {
                canvas_draw_frame(
                    canvas, x_pos + white_w * i + black_x, y_pos + black_y, black_w + 1, black_h);
            }
        }
    }

    // volume view_port
    x_pos = 124;
    y_pos = 0;
    const uint8_t volume_h =
        (64 / (COUNT_OF(MUSIC_BEEPER_VOLUMES) - 1)) * music_beeper->model->volume;
    canvas_draw_frame(canvas, x_pos, y_pos, 4, 64);
    canvas_draw_box(canvas, x_pos, y_pos + (64 - volume_h), 4, volume_h);

    // note stack view_port
    x_pos = 73;
    y_pos = 0;
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_frame(canvas, x_pos, y_pos, 49, 64);
    canvas_draw_line(canvas, x_pos + 28, 0, x_pos + 28, 64);

    char duration_text[16];
    for(uint8_t i = 0; i < MUSIC_BEEPER_SEMITONE_HISTORY_SIZE; i++) {
        if(music_beeper->model->duration_history[i] == 0xFF) {
            snprintf(duration_text, 15, "--");
        } else {
            snprintf(duration_text, 15, "%d", music_beeper->model->duration_history[i]);
        }

        if(i == 0) {
            canvas_draw_box(canvas, x_pos, y_pos + 48, 49, 16);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }
        canvas_draw_str(
            canvas,
            x_pos + 4,
            64 - 16 * i - 3,
            semitone_to_note(music_beeper->model->semitone_history[i]));
        canvas_draw_str(canvas, x_pos + 31, 64 - 16 * i - 3, duration_text);
        canvas_draw_line(canvas, x_pos, 64 - 16 * i, x_pos + 48, 64 - 16 * i);
    }

    furi_mutex_release(music_beeper->model_mutex);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    MusicBeeper* music_beeper = ctx;
    if(input_event->type == InputTypeShort) {
        furi_message_queue_put(music_beeper->input_queue, input_event, 0);
    }
}

static void music_beeper_worker_callback(
    uint8_t semitone,
    uint8_t dots,
    uint8_t duration,
    float position,
    void* context) {
    MusicBeeper* music_beeper = context;
    furi_check(furi_mutex_acquire(music_beeper->model_mutex, FuriWaitForever) == FuriStatusOk);

    for(size_t i = 0; i < MUSIC_BEEPER_SEMITONE_HISTORY_SIZE - 1; i++) {
        size_t r = MUSIC_BEEPER_SEMITONE_HISTORY_SIZE - 1 - i;
        music_beeper->model->duration_history[r] = music_beeper->model->duration_history[r - 1];
        music_beeper->model->semitone_history[r] = music_beeper->model->semitone_history[r - 1];
    }

    semitone = (semitone == 0xFF) ? 0xFF : semitone % 12;

    music_beeper->model->semitone = semitone;
    music_beeper->model->dots = dots;
    music_beeper->model->duration = duration;
    music_beeper->model->position = position;

    music_beeper->model->semitone_history[0] = semitone;
    music_beeper->model->duration_history[0] = duration;

    furi_mutex_release(music_beeper->model_mutex);
    view_port_update(music_beeper->view_port);
}

void music_beeper_clear(MusicBeeper* instance) {
    memset(instance->model->duration_history, 0xff, MUSIC_BEEPER_SEMITONE_HISTORY_SIZE);
    memset(instance->model->semitone_history, 0xff, MUSIC_BEEPER_SEMITONE_HISTORY_SIZE);
    music_beeper_worker_clear(instance->worker);
}

MusicBeeper* music_beeper_alloc() {
    MusicBeeper* instance = malloc(sizeof(MusicBeeper));

    instance->model = malloc(sizeof(MusicBeeperModel));
    instance->model->volume = 4;

    instance->model_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    instance->worker = music_beeper_worker_alloc();
    music_beeper_worker_set_volume(
        instance->worker, MUSIC_BEEPER_VOLUMES[instance->model->volume]);
    music_beeper_worker_set_callback(instance->worker, music_beeper_worker_callback, instance);

    music_beeper_clear(instance);

    instance->view_port = view_port_alloc();
    view_port_draw_callback_set(instance->view_port, render_callback, instance);
    view_port_input_callback_set(instance->view_port, input_callback, instance);

    // Open GUI and register view_port
    instance->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(instance->gui, instance->view_port, GuiLayerFullscreen);

    return instance;
}

void music_beeper_free(MusicBeeper* instance) {
    gui_remove_view_port(instance->gui, instance->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(instance->view_port);

    music_beeper_worker_free(instance->worker);

    furi_message_queue_free(instance->input_queue);

    furi_mutex_free(instance->model_mutex);

    free(instance->model);
    free(instance);
}

int32_t music_beeper_app(void* p) {
    MusicBeeper* music_beeper = music_beeper_alloc();

    FuriString* file_path;
    file_path = furi_string_alloc();

    do {
        if(p && strlen(p)) {
            furi_string_set(file_path, (const char*)p);
        } else {
            furi_string_set(file_path, MUSIC_BEEPER_APP_PATH_FOLDER);

            DialogsFileBrowserOptions browser_options;
            dialog_file_browser_set_basic_options(
                &browser_options, MUSIC_BEEPER_APP_EXTENSION, &I_music_10px);
            browser_options.hide_ext = false;

            DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
            bool res = dialog_file_browser_show(dialogs, file_path, file_path, &browser_options);

            furi_record_close(RECORD_DIALOGS);
            if(!res) {
                FURI_LOG_E(TAG, "No file selected");
                break;
            }
        }

        if(!music_beeper_worker_load(music_beeper->worker, furi_string_get_cstr(file_path))) {
            FURI_LOG_E(TAG, "Unable to load file");
            break;
        }

        music_beeper_worker_start(music_beeper->worker);

        InputEvent input;
        while(furi_message_queue_get(music_beeper->input_queue, &input, FuriWaitForever) ==
              FuriStatusOk) {
            furi_check(
                furi_mutex_acquire(music_beeper->model_mutex, FuriWaitForever) == FuriStatusOk);

            if(input.key == InputKeyBack) {
                furi_mutex_release(music_beeper->model_mutex);
                break;
            } else if(input.key == InputKeyUp) {
                if(music_beeper->model->volume < COUNT_OF(MUSIC_BEEPER_VOLUMES) - 1)
                    music_beeper->model->volume++;
                music_beeper_worker_set_volume(
                    music_beeper->worker, MUSIC_BEEPER_VOLUMES[music_beeper->model->volume]);
            } else if(input.key == InputKeyDown) {
                if(music_beeper->model->volume > 0) music_beeper->model->volume--;
                music_beeper_worker_set_volume(
                    music_beeper->worker, MUSIC_BEEPER_VOLUMES[music_beeper->model->volume]);
            }

            furi_mutex_release(music_beeper->model_mutex);
            view_port_update(music_beeper->view_port);
        }

        music_beeper_worker_stop(music_beeper->worker);
        if(p && strlen(p)) break; // Exit instead of going to browser if launched with arg
        music_beeper_clear(music_beeper);
    } while(1);

    furi_string_free(file_path);
    music_beeper_free(music_beeper);

    return 0;
}
