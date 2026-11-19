#include "sim800_call_view.h"

#include <gui/elements.h>
#include <furi.h>

#define SIM800_DETAIL_MAX 32

struct Sim800CallView {
    View* view;
    Sim800CallViewCallback callback;
    void* context;
};

typedef struct {
    Sim800CallState state;
    char number[SIM800_NUMBER_MAX];
    char operator_name[SIM800_DETAIL_MAX];
    char detail[SIM800_DETAIL_MAX];
    uint32_t duration;
    uint8_t rssi;
    bool saved;
} Sim800CallViewModel;

static const char* sim800_call_state_text(Sim800CallState state) {
    switch(state) {
    case Sim800CallStateDialing:
        return "Dialing...";
    case Sim800CallStateRinging:
        return "Ringing...";
    case Sim800CallStateActive:
        return "In call";
    case Sim800CallStateIncoming:
        return "Incoming call";
    case Sim800CallStateEnded:
        return "Call ended";
    case Sim800CallStateFailed:
    default:
        return "Call failed";
    }
}

/** AT+CSQ 0..31 as five bars, 99 as "--". */
static void sim800_call_draw_signal(Canvas* canvas, uint8_t rssi, int32_t x, int32_t y) {
    if(rssi > 31) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, x, y, "--");
        return;
    }

    uint8_t bars = (uint8_t)((rssi * 5) / 31);
    if(rssi > 0 && bars == 0) bars = 1;

    for(uint8_t i = 0; i < 5; i++) {
        int32_t height = 2 + i * 2;
        int32_t bx = x + i * 3;
        int32_t by = y - height;
        if(i < bars) {
            canvas_draw_box(canvas, bx, by, 2, height);
        } else {
            canvas_draw_dot(canvas, bx, y - 1);
        }
    }
}

static void sim800_call_view_draw(Canvas* canvas, void* model) {
    Sim800CallViewModel* m = model;

    canvas_clear(canvas);

    canvas_set_font(canvas, FontSecondary);
    if(m->operator_name[0] != '\0') {
        canvas_draw_str(canvas, 2, 9, m->operator_name);
    } else {
        canvas_draw_str(canvas, 2, 9, "No network");
    }
    sim800_call_draw_signal(canvas, m->rssi, 110, 9);
    canvas_draw_line(canvas, 0, 11, 127, 11);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 23, AlignCenter, AlignBottom, m->number);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas, 64, 34, AlignCenter, AlignBottom, sim800_call_state_text(m->state));

    if(m->state == Sim800CallStateActive) {
        char timer[16];
        snprintf(
            timer,
            sizeof(timer),
            "%02lu:%02lu",
            (unsigned long)(m->duration / 60),
            (unsigned long)(m->duration % 60));
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 46, AlignCenter, AlignBottom, timer);
    } else if(m->detail[0] != '\0') {
        canvas_draw_str_aligned(canvas, 64, 45, AlignCenter, AlignBottom, m->detail);
    }

    if(m->state == Sim800CallStateIncoming) {
        elements_button_left(canvas, "Reject");
        elements_button_center(canvas, "Answer");
    } else {
        if(!m->saved) elements_button_left(canvas, "Save");
        if(m->state == Sim800CallStateEnded || m->state == Sim800CallStateFailed) {
            elements_button_center(canvas, "Re-dial");
        } else {
            elements_button_center(canvas, "End");
        }
        if(m->state == Sim800CallStateActive) elements_button_right(canvas, "Tones");
    }
}

static bool sim800_call_view_input(InputEvent* event, void* context) {
    Sim800CallView* instance = context;

    if(event->type != InputTypeShort) return false;
    if(instance->callback == NULL) return false;

    bool consumed = false;
    bool saved = false;
    Sim800CallState state = Sim800CallStateEnded;

    with_view_model(
        instance->view,
        Sim800CallViewModel * model,
        {
            saved = model->saved;
            state = model->state;
        },
        false);

    if(event->key == InputKeyOk) {
        instance->callback(Sim800CallButtonCenter, instance->context);
        consumed = true;
    } else if(event->key == InputKeyLeft) {
        // Save is hidden once the number is saved
        if(state == Sim800CallStateIncoming || !saved) {
            instance->callback(Sim800CallButtonLeft, instance->context);
        }
        consumed = true;
    } else if(event->key == InputKeyRight) {
        // Tones only mean anything once the far end has picked up
        if(state == Sim800CallStateActive) {
            instance->callback(Sim800CallButtonRight, instance->context);
        }
        consumed = true;
    }

    return consumed;
}

Sim800CallView* sim800_call_view_alloc(void) {
    Sim800CallView* instance = malloc(sizeof(Sim800CallView));

    instance->view = view_alloc();
    instance->callback = NULL;
    instance->context = NULL;

    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(Sim800CallViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, sim800_call_view_draw);
    view_set_input_callback(instance->view, sim800_call_view_input);

    with_view_model(
        instance->view,
        Sim800CallViewModel * model,
        {
            model->state = Sim800CallStateDialing;
            model->rssi = 99;
            model->duration = 0;
            model->saved = false;
            model->number[0] = '\0';
            model->operator_name[0] = '\0';
            model->detail[0] = '\0';
        },
        false);

    return instance;
}

void sim800_call_view_free(Sim800CallView* instance) {
    furi_assert(instance);
    view_free(instance->view);
    free(instance);
}

View* sim800_call_view_get_view(Sim800CallView* instance) {
    furi_assert(instance);
    return instance->view;
}

void sim800_call_view_set_callback(
    Sim800CallView* instance,
    Sim800CallViewCallback callback,
    void* context) {
    furi_assert(instance);
    instance->context = context;
    instance->callback = callback;
}

void sim800_call_view_set_number(Sim800CallView* instance, const char* number) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        Sim800CallViewModel * model,
        { strlcpy(model->number, number, sizeof(model->number)); },
        true);
}

void sim800_call_view_set_state(Sim800CallView* instance, Sim800CallState state) {
    furi_assert(instance);
    with_view_model(instance->view, Sim800CallViewModel * model, { model->state = state; }, true);
}

void sim800_call_view_set_detail(Sim800CallView* instance, const char* detail) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        Sim800CallViewModel * model,
        { strlcpy(model->detail, detail, sizeof(model->detail)); },
        true);
}

void sim800_call_view_set_operator(Sim800CallView* instance, const char* name) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        Sim800CallViewModel * model,
        { strlcpy(model->operator_name, name, sizeof(model->operator_name)); },
        true);
}

void sim800_call_view_set_duration(Sim800CallView* instance, uint32_t seconds) {
    furi_assert(instance);
    with_view_model(
        instance->view, Sim800CallViewModel * model, { model->duration = seconds; }, true);
}

void sim800_call_view_set_rssi(Sim800CallView* instance, uint8_t rssi) {
    furi_assert(instance);
    with_view_model(instance->view, Sim800CallViewModel * model, { model->rssi = rssi; }, true);
}

void sim800_call_view_set_saved(Sim800CallView* instance, bool saved) {
    furi_assert(instance);
    with_view_model(instance->view, Sim800CallViewModel * model, { model->saved = saved; }, true);
}

Sim800CallState sim800_call_view_get_state(Sim800CallView* instance) {
    furi_assert(instance);
    Sim800CallState state = Sim800CallStateEnded;
    with_view_model(instance->view, Sim800CallViewModel * model, { state = model->state; }, false);
    return state;
}
