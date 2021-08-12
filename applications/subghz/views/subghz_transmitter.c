#include "subghz_transmitter.h"
#include "../subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi-hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification-messages.h>

#include <assets_icons.h>

struct SubghzTransmitter {
    View* view;
    SubghzTransmitterCallback callback;
    void* context;
};

typedef struct {
    string_t text;
    uint16_t scene;
    SubGhzProtocolCommon* protocol;
} SubghzTransmitterModel;

void subghz_transmitter_set_callback(
    SubghzTransmitter* subghz_transmitter,
    SubghzTransmitterCallback callback,
    void* context) {
    furi_assert(subghz_transmitter);

    subghz_transmitter->callback = callback;
    subghz_transmitter->context = context;
}

void subghz_transmitter_set_protocol(
    SubghzTransmitter* subghz_transmitter,
    SubGhzProtocolCommon* protocol) {
    with_view_model(
        subghz_transmitter->view, (SubghzTransmitterModel * model) {
            model->protocol = protocol;
            return true;
        });
}

void subghz_transmitter_draw(Canvas* canvas, SubghzTransmitterModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text(canvas, 0, 10, string_get_cstr(model->text));

    elements_button_center(canvas, "Send");
}

bool subghz_transmitter_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubghzTransmitter* subghz_transmitter = context;

    if(event->type != InputTypeShort) return false;

    if(event->key == InputKeyBack) {
        return false;
    } else if(event->key == InputKeyOk) {
        subghz_transmitter->callback(SubghzTransmitterEventSend, subghz_transmitter->context);
        return true;
    }

    return true;
}

void subghz_transmitter_text_callback(string_t text, void* context) {
    furi_assert(context);
    SubghzTransmitter* subghz_transmitter = context;

    with_view_model(
        subghz_transmitter->view, (SubghzTransmitterModel * model) {
            string_set(model->text, text);
            model->scene = 0;
            return true;
        });
}

void subghz_transmitter_enter(void* context) {
    furi_assert(context);
    SubghzTransmitter* subghz_transmitter = context;
    with_view_model(
        subghz_transmitter->view, (SubghzTransmitterModel * model) {
            model->protocol->to_string(model->protocol, model->text);
            return true;
        });
}

void subghz_transmitter_exit(void* context) {
    furi_assert(context);
    SubghzTransmitter* subghz_transmitter = context;
    with_view_model(
        subghz_transmitter->view, (SubghzTransmitterModel * model) {
            string_clean(model->text);
            return true;
        });
}

SubghzTransmitter* subghz_transmitter_alloc() {
    SubghzTransmitter* subghz_transmitter = furi_alloc(sizeof(SubghzTransmitter));

    // View allocation and configuration
    subghz_transmitter->view = view_alloc();
    view_allocate_model(
        subghz_transmitter->view, ViewModelTypeLocking, sizeof(SubghzTransmitterModel));
    view_set_context(subghz_transmitter->view, subghz_transmitter);
    view_set_draw_callback(subghz_transmitter->view, (ViewDrawCallback)subghz_transmitter_draw);
    view_set_input_callback(subghz_transmitter->view, subghz_transmitter_input);
    view_set_enter_callback(subghz_transmitter->view, subghz_transmitter_enter);
    view_set_exit_callback(subghz_transmitter->view, subghz_transmitter_exit);

    with_view_model(
        subghz_transmitter->view, (SubghzTransmitterModel * model) {
            string_init(model->text);
            return true;
        });
    return subghz_transmitter;
}

void subghz_transmitter_free(SubghzTransmitter* subghz_transmitter) {
    furi_assert(subghz_transmitter);

    with_view_model(
        subghz_transmitter->view, (SubghzTransmitterModel * model) {
            string_clear(model->text);
            return true;
        });
    view_free(subghz_transmitter->view);
    free(subghz_transmitter);
}

View* subghz_transmitter_get_view(SubghzTransmitter* subghz_transmitter) {
    furi_assert(subghz_transmitter);
    return subghz_transmitter->view;
}
