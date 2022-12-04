#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <Counter_icons.h>

#define MAX_COUNT 99
#define BOXTIME 2
#define BOXWIDTH 30
#define MIDDLE_X 64 - BOXWIDTH / 2
#define MIDDLE_Y 32 - BOXWIDTH / 2
#define OFFSET_Y 9

typedef struct {
    FuriMessageQueue* input_queue;
    ViewPort* view_port;
    Gui* gui;
    FuriMutex** mutex;

    int count;
    bool pressed;
    int boxtimer;
} Counter;

void state_free(Counter* c) {
    gui_remove_view_port(c->gui, c->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(c->view_port);
    furi_message_queue_free(c->input_queue);
    furi_mutex_free(c->mutex);
    free(c);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    Counter* c = ctx;
    if(input_event->type == InputTypeShort) {
        furi_message_queue_put(c->input_queue, input_event, 0);
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    Counter* c = ctx;
    furi_check(furi_mutex_acquire(c->mutex, FuriWaitForever) == FuriStatusOk);
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignCenter, "Counter :)");
    canvas_set_font(canvas, FontBigNumbers);

    char scount[5];
    if(c->pressed == true || c->boxtimer > 0) {
        canvas_draw_rframe(canvas, MIDDLE_X, MIDDLE_Y + OFFSET_Y, BOXWIDTH, BOXWIDTH, 5);
        canvas_draw_rframe(
            canvas, MIDDLE_X - 1, MIDDLE_Y + OFFSET_Y - 1, BOXWIDTH + 2, BOXWIDTH + 2, 5);
        canvas_draw_rframe(
            canvas, MIDDLE_X - 2, MIDDLE_Y + OFFSET_Y - 2, BOXWIDTH + 4, BOXWIDTH + 4, 5);
        c->pressed = false;
        c->boxtimer--;
    } else {
        canvas_draw_rframe(canvas, MIDDLE_X, MIDDLE_Y + OFFSET_Y, BOXWIDTH, BOXWIDTH, 5);
    }
    snprintf(scount, sizeof(scount), "%d", c->count);
    canvas_draw_str_aligned(canvas, 64, 32 + OFFSET_Y, AlignCenter, AlignCenter, scount);
    furi_mutex_release(c->mutex);
}

Counter* state_init() {
    Counter* c = malloc(sizeof(Counter));
    c->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    c->view_port = view_port_alloc();
    c->gui = furi_record_open(RECORD_GUI);
    c->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    c->count = 0;
    c->boxtimer = 0;
    view_port_input_callback_set(c->view_port, input_callback, c);
    view_port_draw_callback_set(c->view_port, render_callback, c);
    gui_add_view_port(c->gui, c->view_port, GuiLayerFullscreen);
    return c;
}

int32_t counterapp(void) {
    Counter* c = state_init();

    while(1) {
        InputEvent input;
        while(furi_message_queue_get(c->input_queue, &input, FuriWaitForever) == FuriStatusOk) {
            furi_check(furi_mutex_acquire(c->mutex, FuriWaitForever) == FuriStatusOk);

            if(input.key == InputKeyBack) {
                furi_mutex_release(c->mutex);
                state_free(c);
                return 0;
            } else if(input.key == InputKeyUp && c->count < MAX_COUNT) {
                c->pressed = true;
                c->boxtimer = BOXTIME;
                c->count++;
            } else if(input.key == InputKeyDown && c->count != 0) {
                c->pressed = true;
                c->boxtimer = BOXTIME;
                c->count--;
            }
            furi_mutex_release(c->mutex);
            view_port_update(c->view_port);
        }
    }
    state_free(c);
    return 0;
}
