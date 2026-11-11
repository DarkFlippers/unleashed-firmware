#include "t5577_listen_view.h"

#include <furi.h>
#include <gui/canvas.h>
#include <gui/elements.h>

/** The fullest a sample is drawn, in pixels above the floor. */
#define T5577_LISTEN_VIEW_LEVEL_MAX 9

#define T5577_LISTEN_VIEW_TITLE_MAX  16
#define T5577_LISTEN_VIEW_STATUS_MAX 32

// No picture. The one the work view uses shows a card being put ONTO a Flipper, which is the
// opposite of what this screen asks for - the Flipper goes against somebody else's reader - and
// a FAP cannot link the firmware's own icons to find a better one. Drawing a cloner would be
// replacing a wrong guess with a guess; the line at the bottom says where to hold it instead,
// and the trace gets the sixty pixels the picture was using.
#define T5577_LISTEN_VIEW_TRACE_X 0
#define T5577_LISTEN_VIEW_TRACE_Y 32
#define T5577_LISTEN_VIEW_TRACE_W 128
#define T5577_LISTEN_VIEW_TRACE_H 14

// Inside the frame, with a pixel of margin all round.
#define T5577_LISTEN_VIEW_FLOOR (T5577_LISTEN_VIEW_TRACE_Y + T5577_LISTEN_VIEW_TRACE_H - 2)
#define T5577_LISTEN_VIEW_FIRST (T5577_LISTEN_VIEW_TRACE_X + 4)
#define T5577_LISTEN_VIEW_PITCH 2

typedef struct {
    char title[T5577_LISTEN_VIEW_TITLE_MAX];
    char status[T5577_LISTEN_VIEW_STATUS_MAX];
    // Oldest to newest, so the newest is always at the right and the trace reads the way a
    // scrolling instrument does. Always full: the ticks that have not happened yet are zeroes,
    // which draw as the same floor the quiet ones do.
    uint8_t level[T5577_LISTEN_VIEW_SAMPLES];
} T5577ListenViewModel;

struct T5577ListenView {
    View* view;
};

static void t5577_listen_view_draw_callback(Canvas* canvas, void* _model) {
    const T5577ListenViewModel* model = _model;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 12, model->title);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 26, model->status);

    canvas_draw_rframe(
        canvas,
        T5577_LISTEN_VIEW_TRACE_X,
        T5577_LISTEN_VIEW_TRACE_Y,
        T5577_LISTEN_VIEW_TRACE_W,
        T5577_LISTEN_VIEW_TRACE_H,
        3);

    for(uint8_t i = 0; i < T5577_LISTEN_VIEW_SAMPLES; i++) {
        const uint8_t x = T5577_LISTEN_VIEW_FIRST + i * T5577_LISTEN_VIEW_PITCH;
        const uint8_t level = model->level[i];

        if(level == 0) {
            canvas_draw_dot(canvas, x, T5577_LISTEN_VIEW_FLOOR);
        } else {
            canvas_draw_line(
                canvas, x, T5577_LISTEN_VIEW_FLOOR, x, T5577_LISTEN_VIEW_FLOOR - level);
        }
    }

    // Drawn, not boxed: this clips rather than wraps, so it has to fit 128 px.
    canvas_draw_str(canvas, 0, 60, "Hold on the cloner");
}

T5577ListenView* t5577_listen_view_alloc(void) {
    T5577ListenView* instance = malloc(sizeof(T5577ListenView));

    instance->view = view_alloc();
    view_set_context(instance->view, instance);
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(T5577ListenViewModel));
    view_set_draw_callback(instance->view, t5577_listen_view_draw_callback);

    return instance;
}

void t5577_listen_view_free(T5577ListenView* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* t5577_listen_view_get_view(T5577ListenView* instance) {
    furi_assert(instance);

    return instance->view;
}

void t5577_listen_view_start(T5577ListenView* instance, const char* title) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        T5577ListenViewModel * model,
        {
            strlcpy(model->title, title, sizeof(model->title));
            model->status[0] = '\0';
            memset(model->level, 0, sizeof(model->level));
        },
        true);
}

void t5577_listen_view_set_status(T5577ListenView* instance, const char* status) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        T5577ListenViewModel * model,
        { strlcpy(model->status, status, sizeof(model->status)); },
        true);
}

void t5577_listen_view_push(T5577ListenView* instance, uint32_t value, uint32_t full) {
    furi_assert(instance);

    // One expression, one place: a tick at or past full is a full bar, and `full` of zero would
    // be a caller with nothing to scale against.
    const uint8_t level = (full == 0 || value >= full) ?
                              T5577_LISTEN_VIEW_LEVEL_MAX :
                              (uint8_t)((value * T5577_LISTEN_VIEW_LEVEL_MAX) / full);

    with_view_model(
        instance->view,
        T5577ListenViewModel * model,
        {
            // Shifted rather than kept in a ring with a head: the whole trace is redrawn every
            // tick anyway, and a flat array draws left to right without the reader having to
            // work out where "oldest" is.
            memmove(model->level, model->level + 1, sizeof(model->level) - 1);
            model->level[T5577_LISTEN_VIEW_SAMPLES - 1] = level;
        },
        true);
}
