#include "subghz_receiver.h"
#include "../subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi-hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification-messages.h>

#include <assets_icons.h>

struct SubghzReceiver {
    View* view;
    SubghzReceiverCallback callback;
    void* context;
};

typedef struct {
    string_t text;
    uint16_t scene;
    SubGhzProtocolCommon* protocol;
} SubghzReceiverModel;

void subghz_receiver_set_callback(
    SubghzReceiver* subghz_receiver,
    SubghzReceiverCallback callback,
    void* context) {
    furi_assert(subghz_receiver);
    furi_assert(callback);
    subghz_receiver->callback = callback;
    subghz_receiver->context = context;
}

void subghz_receiver_set_protocol(SubghzReceiver* subghz_receiver, SubGhzProtocolCommon* protocol) {
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            model->protocol = protocol;
            return true;
        });
}

void subghz_receiver_draw(Canvas* canvas, SubghzReceiverModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text(canvas, 0, 10, string_get_cstr(model->text));

    elements_button_left(canvas, "Back");
    if(model->protocol && model->protocol->to_save_string) {
        elements_button_right(canvas, "Save");
    }
}

bool subghz_receiver_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;

    if(event->type != InputTypeShort) return false;

    bool can_be_saved = false;
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            can_be_saved = (model->protocol && model->protocol->to_save_string);
            return false;
        });

    if(event->key == InputKeyBack) {
        return false;
    } else if(event->key == InputKeyLeft) {
        subghz_receiver->callback(SubghzReceverEventBack, subghz_receiver->context);
    } else if(can_be_saved && event->key == InputKeyRight) {
        subghz_receiver->callback(SubghzReceverEventSave, subghz_receiver->context);
    }

    return true;
}

void subghz_receiver_text_callback(string_t text, void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;

    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            string_set(model->text, text);
            model->scene = 0;
            return true;
        });
}

void subghz_receiver_enter(void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            model->protocol->to_string(model->protocol, model->text);
            return true;
        });
}

void subghz_receiver_exit(void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            string_clean(model->text);
            return true;
        });
}

SubghzReceiver* subghz_receiver_alloc() {
    SubghzReceiver* subghz_receiver = furi_alloc(sizeof(SubghzReceiver));

    // View allocation and configuration
    subghz_receiver->view = view_alloc();
    view_allocate_model(subghz_receiver->view, ViewModelTypeLocking, sizeof(SubghzReceiverModel));
    view_set_context(subghz_receiver->view, subghz_receiver);
    view_set_draw_callback(subghz_receiver->view, (ViewDrawCallback)subghz_receiver_draw);
    view_set_input_callback(subghz_receiver->view, subghz_receiver_input);
    view_set_enter_callback(subghz_receiver->view, subghz_receiver_enter);
    view_set_exit_callback(subghz_receiver->view, subghz_receiver_exit);

    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            string_init(model->text);
            return true;
        });
    return subghz_receiver;
}

void subghz_receiver_free(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);

    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            string_clear(model->text);
            return true;
        });
    view_free(subghz_receiver->view);
    free(subghz_receiver);
}

View* subghz_receiver_get_view(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);
    return subghz_receiver->view;
}
