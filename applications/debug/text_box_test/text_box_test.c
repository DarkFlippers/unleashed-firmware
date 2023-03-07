#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <gui/elements.h>

#define TAG "TextBoxTest"

static void text_box_center_top_secondary_128x22(Canvas* canvas) {
    canvas_draw_frame(canvas, 0, 0, 128, 22);
    elements_text_box(canvas, 0, 0, 128, 22, AlignCenter, AlignTop, "secondary font test", false);
}

static void text_box_right_bottom_bold_128x22(Canvas* canvas) {
    canvas_draw_frame(canvas, 0, 0, 128, 22);
    elements_text_box(
        canvas, 0, 0, 128, 22, AlignRight, AlignBottom, "\e#Bold font test\e#", false);
}

static void text_box_left_center_mixed_80x50(Canvas* canvas) {
    canvas_draw_frame(canvas, 0, 0, 80, 50);
    elements_text_box(
        canvas,
        0,
        0,
        80,
        50,
        AlignLeft,
        AlignCenter,
        "\e#Never\e# gonna give you up\n\e!Never\e! gonna let you down",
        false);
}

static void text_box_center_center_secondary_110x44(Canvas* canvas) {
    canvas_draw_frame(canvas, 4, 20, 110, 30);
    elements_text_box(
        canvas,
        4,
        20,
        110,
        30,
        AlignCenter,
        AlignCenter,
        "Loooooooooooooo0000000ooong file name from happy 100500 Flipper 0wners",
        true);
}

static void (*text_box_test_render[])(Canvas* canvas) = {
    text_box_center_top_secondary_128x22,
    text_box_right_bottom_bold_128x22,
    text_box_left_center_mixed_80x50,
    text_box_center_center_secondary_110x44,
};

typedef struct {
    uint32_t idx;
    FuriMutex* mutex;
} TextBoxTestState;

static void text_box_test_render_callback(Canvas* canvas, void* ctx) {
    TextBoxTestState* state = ctx;
    furi_mutex_acquire(state->mutex, FuriWaitForever);
    canvas_clear(canvas);

    text_box_test_render[state->idx](canvas);

    furi_mutex_release(state->mutex);
}

static void text_box_test_input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t text_box_test_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(32, sizeof(InputEvent));
    furi_check(event_queue);

    TextBoxTestState state = {.idx = 0, .mutex = NULL};
    state.mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    if(!state.mutex) {
        FURI_LOG_E(TAG, "Cannot create mutex");
        return 0;
    }

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, text_box_test_render_callback, &state);
    view_port_input_callback_set(view_port, text_box_test_input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    uint32_t test_renders_num = COUNT_OF(text_box_test_render);
    InputEvent event;
    while(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk) {
        furi_mutex_acquire(state.mutex, FuriWaitForever);

        if(event.type == InputTypeShort) {
            if(event.key == InputKeyRight) {
                if(state.idx < test_renders_num - 1) {
                    state.idx++;
                }
            } else if(event.key == InputKeyLeft) {
                if(state.idx > 0) {
                    state.idx--;
                }
            } else if(event.key == InputKeyBack) {
                furi_mutex_release(state.mutex);
                break;
            }
        }

        furi_mutex_release(state.mutex);
        view_port_update(view_port);
    }

    // remove & free all stuff created by app
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(state.mutex);

    furi_record_close(RECORD_GUI);

    return 0;
}
