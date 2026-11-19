#include "sim800_entry_view.h"
#include "../helpers/sim800_types.h"

#include <gui/elements.h>
#include <furi.h>

struct Sim800EntryView {
    View* view;
    Sim800EntryViewCallback callback;
    void* context;
};

typedef struct {
    char name[SIM800_NAME_MAX];
    char number[SIM800_NUMBER_MAX];
    char operator_name[28];
    uint8_t rssi;
    bool redial;
} Sim800EntryViewModel;

static void sim800_entry_draw_bars(Canvas* canvas, uint8_t rssi, int32_t x, int32_t base) {
    if(rssi > 31) {
        canvas_draw_str(canvas, x, base, "--");
        return;
    }

    uint8_t bars = (uint8_t)((rssi * 5) / 31);
    if(rssi > 0 && bars == 0) bars = 1;

    for(uint8_t i = 0; i < 5; i++) {
        int32_t h = 2 + i * 2;
        int32_t bx = x + i * 3;
        if(i < bars) {
            canvas_draw_box(canvas, bx, base - h, 2, h);
        } else {
            canvas_draw_dot(canvas, bx, base - 1);
        }
    }
}

static void sim800_entry_view_draw(Canvas* canvas, void* ctx) {
    Sim800EntryViewModel* m = ctx;

    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 13, AlignCenter, AlignBottom, m->name);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 26, AlignCenter, AlignBottom, m->number);

    canvas_draw_line(canvas, 8, 31, 119, 31);

    canvas_draw_str(canvas, 4, 42, m->operator_name);
    sim800_entry_draw_bars(canvas, m->rssi, 110, 42);

    elements_button_left(canvas, "SMS");
    elements_button_center(canvas, m->redial ? "Re-dial" : "Call");
    elements_button_right(canvas, "Edit");
}

static bool sim800_entry_view_input(InputEvent* event, void* ctx) {
    Sim800EntryView* instance = ctx;

    if(event->type != InputTypeShort || instance->callback == NULL) return false;

    if(event->key == InputKeyOk) {
        instance->callback(Sim800EntryButtonCall, instance->context);
    } else if(event->key == InputKeyLeft) {
        instance->callback(Sim800EntryButtonSms, instance->context);
    } else if(event->key == InputKeyRight) {
        instance->callback(Sim800EntryButtonEdit, instance->context);
    } else {
        return false;
    }

    return true;
}

Sim800EntryView* sim800_entry_view_alloc(void) {
    Sim800EntryView* instance = malloc(sizeof(Sim800EntryView));

    instance->view = view_alloc();
    instance->callback = NULL;
    instance->context = NULL;

    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(Sim800EntryViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, sim800_entry_view_draw);
    view_set_input_callback(instance->view, sim800_entry_view_input);

    with_view_model(
        instance->view,
        Sim800EntryViewModel * model,
        {
            model->rssi = 99;
            model->redial = false;
            model->name[0] = '\0';
            model->number[0] = '\0';
            strlcpy(model->operator_name, "--", sizeof(model->operator_name));
        },
        false);

    return instance;
}

void sim800_entry_view_free(Sim800EntryView* instance) {
    furi_assert(instance);
    view_free(instance->view);
    free(instance);
}

View* sim800_entry_view_get_view(Sim800EntryView* instance) {
    furi_assert(instance);
    return instance->view;
}

void sim800_entry_view_set_callback(
    Sim800EntryView* instance,
    Sim800EntryViewCallback callback,
    void* context) {
    furi_assert(instance);
    instance->context = context;
    instance->callback = callback;
}

void sim800_entry_view_set_entry(Sim800EntryView* instance, const char* name, const char* number) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        Sim800EntryViewModel * model,
        {
            strlcpy(model->name, name, sizeof(model->name));
            strlcpy(model->number, number, sizeof(model->number));
        },
        true);
}

void sim800_entry_view_set_operator(Sim800EntryView* instance, const char* name) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        Sim800EntryViewModel * model,
        { strlcpy(model->operator_name, name, sizeof(model->operator_name)); },
        true);
}

void sim800_entry_view_set_rssi(Sim800EntryView* instance, uint8_t rssi) {
    furi_assert(instance);
    with_view_model(instance->view, Sim800EntryViewModel * model, { model->rssi = rssi; }, true);
}

void sim800_entry_view_set_redial(Sim800EntryView* instance, bool redial) {
    furi_assert(instance);
    with_view_model(
        instance->view, Sim800EntryViewModel * model, { model->redial = redial; }, true);
}
