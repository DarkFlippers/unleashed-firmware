#include "sim800_connect_view.h"

#include <gui/elements.h>
#include <furi.h>

// The app ticks at 1 Hz, which is far too coarse for a bar, so this view runs its own clock
#define SIM800_CONNECT_PERIOD_MS 100

typedef struct {
    uint32_t baud;
    uint32_t total_ms;
    uint32_t elapsed_ms;
} Sim800ConnectViewModel;

struct Sim800ConnectView {
    View* view;
    FuriTimer* timer;
};

static void sim800_connect_view_draw(Canvas* canvas, void* ctx) {
    Sim800ConnectViewModel* model = ctx;
    char line[24];
    float progress = 0.0f;

    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 16, AlignCenter, AlignBottom, "Connecting");

    canvas_set_font(canvas, FontSecondary);
    snprintf(line, sizeof(line), "%lu baud", model->baud);
    canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignBottom, line);

    if(model->total_ms > 0) {
        progress = (float)model->elapsed_ms / (float)model->total_ms;
        if(progress > 1.0f) progress = 1.0f;
    }

    snprintf(
        line,
        sizeof(line),
        "%lu.%lus",
        model->elapsed_ms / 1000UL,
        (model->elapsed_ms % 1000UL) / 100UL);
    elements_progress_bar_with_text(canvas, 14, 38, 100, progress, line);
}

static void sim800_connect_view_tick(void* ctx) {
    Sim800ConnectView* instance = ctx;
    with_view_model(
        instance->view,
        Sim800ConnectViewModel * model,
        { model->elapsed_ms += SIM800_CONNECT_PERIOD_MS; },
        true);
}

static void sim800_connect_view_enter(void* ctx) {
    Sim800ConnectView* instance = ctx;
    furi_timer_start(instance->timer, furi_ms_to_ticks(SIM800_CONNECT_PERIOD_MS));
}

static void sim800_connect_view_exit(void* ctx) {
    Sim800ConnectView* instance = ctx;
    furi_timer_stop(instance->timer);
}

Sim800ConnectView* sim800_connect_view_alloc(void) {
    Sim800ConnectView* instance = malloc(sizeof(Sim800ConnectView));

    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(Sim800ConnectViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, sim800_connect_view_draw);
    view_set_enter_callback(instance->view, sim800_connect_view_enter);
    view_set_exit_callback(instance->view, sim800_connect_view_exit);

    instance->timer = furi_timer_alloc(sim800_connect_view_tick, FuriTimerTypePeriodic, instance);

    return instance;
}

void sim800_connect_view_free(Sim800ConnectView* instance) {
    furi_assert(instance);
    furi_timer_stop(instance->timer);
    furi_timer_free(instance->timer);
    view_free(instance->view);
    free(instance);
}

View* sim800_connect_view_get_view(Sim800ConnectView* instance) {
    furi_assert(instance);
    return instance->view;
}

void sim800_connect_view_start(Sim800ConnectView* instance, uint32_t baud, uint32_t total_ms) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        Sim800ConnectViewModel * model,
        {
            model->baud = baud;
            model->total_ms = total_ms;
            model->elapsed_ms = 0;
        },
        true);
}
