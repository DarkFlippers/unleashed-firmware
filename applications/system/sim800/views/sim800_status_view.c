#include "sim800_status_view.h"

#include <gui/elements.h>
#include <furi.h>

#define SIM800_STATUS_FIELD_MAX 28

struct Sim800StatusView {
    View* view;
    Sim800StatusViewCallback callback;
    void* context;
};

typedef struct {
    char operator_name[SIM800_STATUS_FIELD_MAX];
    char registration[SIM800_STATUS_FIELD_MAX];
    char sim[SIM800_STATUS_FIELD_MAX];
    char power[SIM800_STATUS_FIELD_MAX];
    uint8_t rssi;
    bool linked;
} Sim800StatusViewModel;

static void sim800_status_draw_bars(Canvas* canvas, uint8_t rssi) {
    const int32_t x = 98;
    const int32_t base = 11;

    if(rssi > 31) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, x + 6, base, "--");
        return;
    }

    uint8_t bars = (uint8_t)((rssi * 5) / 31);
    if(rssi > 0 && bars == 0) bars = 1;

    for(uint8_t i = 0; i < 5; i++) {
        int32_t h = 2 + i * 2;
        int32_t bx = x + i * 6;
        if(i < bars) {
            canvas_draw_box(canvas, bx, base - h, 4, h);
        } else {
            canvas_draw_frame(canvas, bx, base - 2, 4, 2);
        }
    }
}

static void sim800_status_row(Canvas* canvas, int32_t y, const char* label, const char* value) {
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, y, label);
    canvas_draw_str(canvas, 42, y, value);
}

static void sim800_status_view_draw(Canvas* canvas, void* ctx) {
    Sim800StatusViewModel* m = ctx;
    char signal[24];

    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Status");
    sim800_status_draw_bars(canvas, m->rssi);
    canvas_draw_line(canvas, 0, 13, 127, 13);

    if(!m->linked) {
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text_aligned(
            canvas, 64, 34, AlignCenter, AlignCenter, "No module on the UART.\nCheck Settings.");
        return;
    }

    if(m->rssi > 31) {
        snprintf(signal, sizeof(signal), "no reading");
    } else {
        // AT+CSQ: 0 is -113 dBm, each step is 2 dB
        snprintf(signal, sizeof(signal), "%d dBm (%u)", -113 + (m->rssi * 2), m->rssi);
    }

    sim800_status_row(canvas, 23, "Carrier", m->operator_name);
    sim800_status_row(canvas, 32, "Signal", signal);
    sim800_status_row(canvas, 41, "Network", m->registration);
    sim800_status_row(canvas, 50, "SIM", m->sim);
    sim800_status_row(canvas, 59, "Power", m->power);
}

static bool sim800_status_view_input(InputEvent* event, void* ctx) {
    Sim800StatusView* instance = ctx;

    if(event->type == InputTypeShort && event->key == InputKeyOk && instance->callback) {
        instance->callback(instance->context);
        return true;
    }
    return false;
}

Sim800StatusView* sim800_status_view_alloc(void) {
    Sim800StatusView* instance = malloc(sizeof(Sim800StatusView));

    instance->view = view_alloc();
    instance->callback = NULL;
    instance->context = NULL;

    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(Sim800StatusViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, sim800_status_view_draw);
    view_set_input_callback(instance->view, sim800_status_view_input);

    with_view_model(
        instance->view,
        Sim800StatusViewModel * model,
        {
            model->rssi = 99;
            model->linked = false;
            strlcpy(model->operator_name, "--", sizeof(model->operator_name));
            strlcpy(model->registration, "--", sizeof(model->registration));
            strlcpy(model->sim, "--", sizeof(model->sim));
            strlcpy(model->power, "--", sizeof(model->power));
        },
        false);

    return instance;
}

void sim800_status_view_free(Sim800StatusView* instance) {
    furi_assert(instance);
    view_free(instance->view);
    free(instance);
}

View* sim800_status_view_get_view(Sim800StatusView* instance) {
    furi_assert(instance);
    return instance->view;
}

void sim800_status_view_set_callback(
    Sim800StatusView* instance,
    Sim800StatusViewCallback callback,
    void* context) {
    furi_assert(instance);
    instance->context = context;
    instance->callback = callback;
}

#define SIM800_STATUS_SETTER(field)                             \
    with_view_model(                                            \
        instance->view,                                         \
        Sim800StatusViewModel* model,                           \
        { strlcpy(model->field, text, sizeof(model->field)); }, \
        true)

void sim800_status_view_set_operator(Sim800StatusView* instance, const char* text) {
    furi_assert(instance);
    SIM800_STATUS_SETTER(operator_name);
}

void sim800_status_view_set_registration(Sim800StatusView* instance, const char* text) {
    furi_assert(instance);
    SIM800_STATUS_SETTER(registration);
}

void sim800_status_view_set_sim(Sim800StatusView* instance, const char* text) {
    furi_assert(instance);
    SIM800_STATUS_SETTER(sim);
}

void sim800_status_view_set_power(Sim800StatusView* instance, const char* text) {
    furi_assert(instance);
    SIM800_STATUS_SETTER(power);
}

void sim800_status_view_set_signal(Sim800StatusView* instance, uint8_t rssi) {
    furi_assert(instance);
    with_view_model(instance->view, Sim800StatusViewModel * model, { model->rssi = rssi; }, true);
}

void sim800_status_view_set_linked(Sim800StatusView* instance, bool linked) {
    furi_assert(instance);
    with_view_model(
        instance->view, Sim800StatusViewModel * model, { model->linked = linked; }, true);
}
