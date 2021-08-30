#include "subghz_transmitter.h"
#include "../subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi-hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification-messages.h>

struct SubghzTransmitter {
    View* view;
    SubghzTransmitterCallback callback;
    void* context;
};

typedef struct {
    string_t text;
    uint16_t scene;
    uint32_t real_frequency;
    FuriHalSubGhzPreset preset;
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

void subghz_transmitter_set_frequency_preset(
    SubghzTransmitter* subghz_transmitter,
    uint32_t frequency,
    FuriHalSubGhzPreset preset) {
    with_view_model(
        subghz_transmitter->view, (SubghzTransmitterModel * model) {
            model->real_frequency = frequency;
            model->preset = preset;
            return true;
        });
}

static void subghz_transmitter_button_right(Canvas* canvas, const char* str) {
    const uint8_t button_height = 13;
    const uint8_t vertical_offset = 3;
    const uint8_t horizontal_offset = 1;
    const uint8_t string_width = canvas_string_width(canvas, str);
    const Icon* icon = &I_ButtonCenter_7x7;
    const uint8_t icon_offset = 3;
    const uint8_t icon_width_with_offset = icon_get_width(icon) + icon_offset;
    const uint8_t button_width = string_width + horizontal_offset * 2 + icon_width_with_offset;

    const uint8_t x = (canvas_width(canvas) - button_width) / 2 + 40;
    const uint8_t y = canvas_height(canvas);

    canvas_draw_box(canvas, x, y - button_height, button_width, button_height);

    canvas_draw_line(canvas, x - 1, y, x - 1, y - button_height + 0);
    canvas_draw_line(canvas, x - 2, y, x - 2, y - button_height + 1);
    canvas_draw_line(canvas, x - 3, y, x - 3, y - button_height + 2);

    canvas_draw_line(canvas, x + button_width + 0, y, x + button_width + 0, y - button_height + 0);
    canvas_draw_line(canvas, x + button_width + 1, y, x + button_width + 1, y - button_height + 1);
    canvas_draw_line(canvas, x + button_width + 2, y, x + button_width + 2, y - button_height + 2);

    canvas_invert_color(canvas);
    canvas_draw_icon(
        canvas, x + horizontal_offset, y - button_height + vertical_offset, &I_ButtonCenter_7x7);
    canvas_draw_str(
        canvas, x + horizontal_offset + icon_width_with_offset, y - vertical_offset, str);
    canvas_invert_color(canvas);
}

void subghz_transmitter_draw(Canvas* canvas, SubghzTransmitterModel* model) {
    char buffer[64];
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text(canvas, 0, 8, string_get_cstr(model->text));
    snprintf(
        buffer,
        sizeof(buffer),
        "%03ld.%03ld",
        model->real_frequency / 1000000 % 1000,
        model->real_frequency / 1000 % 1000);
    canvas_draw_str(canvas, 90, 8, buffer);

    if(model->protocol && model->protocol->get_upload_protocol) {
        subghz_transmitter_button_right(canvas, "Send");
    }
}

bool subghz_transmitter_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubghzTransmitter* subghz_transmitter = context;
    bool can_be_send = false;
    with_view_model(
        subghz_transmitter->view, (SubghzTransmitterModel * model) {
            can_be_send = (model->protocol && model->protocol->get_upload_protocol);
            string_clean(model->text);
            model->protocol->to_string(model->protocol, model->text);
            return true;
        });
    //if(event->type != InputTypeShort) return false;

    if(event->key == InputKeyBack) {
        return false;
    } else if(can_be_send && event->key == InputKeyOk && event->type == InputTypePress) {
        subghz_transmitter->callback(SubghzTransmitterEventSendStart, subghz_transmitter->context);
        return true;
    } else if(can_be_send && event->key == InputKeyOk && event->type == InputTypeRelease) {
        subghz_transmitter->callback(SubghzTransmitterEventSendStop, subghz_transmitter->context);
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
