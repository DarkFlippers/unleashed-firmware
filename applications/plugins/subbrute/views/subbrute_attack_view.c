#include "subbrute_attack_view.h"
#include "../subbrute_i.h"
#include "../helpers/subbrute_worker.h"

#include "assets_icons.h"
#include "../../../services/gui/icon_i.h"
#include <input/input.h>
#include <gui/elements.h>
#include <gui/icon_i.h>

struct SubBruteAttackView {
    View* view;
    SubBruteAttackViewCallback callback;
    void* context;
    SubBruteWorker* worker;
};

typedef struct {
    SubBruteAttacks index;
    uint8_t max_value;
    uint8_t current_step;
    bool is_attacking;
} SubBruteAttackViewModel;

void subbrute_attack_view_set_callback(
    SubBruteAttackView* instance,
    SubBruteAttackViewCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

bool subbrute_attack_view_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SubBruteAttackView* instance = context;

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        instance->callback(SubBruteCustomEventTypeBackPressed, instance->context);
        return true;
    }

    bool is_attacking = false;

    with_view_model(
        instance->view, (SubBruteAttackViewModel * model) {
            is_attacking = model->is_attacking;
            return false;
        });

    //    if(!is_attacking) {
    //        instance->callback(SubBruteCustomEventTypeTransmitNotStarted, instance->context);
    //    } else {
    //        instance->callback(SubBruteCustomEventTypeTransmitStarted, instance->context);
    //    }

    if(!is_attacking) {
        if(event->key == InputKeyOk) {
            with_view_model(
                instance->view, (SubBruteAttackViewModel * model) {
                    model->is_attacking = true;
                    return true;
                });
            instance->callback(SubBruteCustomEventTypeTransmitStarted, instance->context);
            //        } else if(event->key == InputKeyBack) {
            //            if(previous_scene == SubBruteSceneLoadFile) {
            //                instance->callback(SubBruteCustomEventTypeLoadFile, instance->context);
            //            } else {
            //                instance->callback(SubBruteCustomEventTypeBackPressed, instance->context);
            //            }
        } else if(event->key == InputKeyUp) {
            instance->callback(SubBruteCustomEventTypeSaveFile, instance->context);
        } else if(event->key == InputKeyDown) {
            instance->callback(SubBruteCustomEventTypeTransmitCustom, instance->context);
        } else if(event->type == InputTypePress) {
            with_view_model(
                instance->view, (SubBruteAttackViewModel * model) {
                    if(event->key == InputKeyLeft) {
                        model->current_step =
                            ((model->current_step - 100) + model->max_value) % model->max_value;
                    } else if(event->key == InputKeyRight) {
                        model->current_step = (model->current_step + 100) % model->max_value;
                    }
                    return true;
                });
            instance->callback(SubBruteCustomEventTypeChangeStep, instance->context);
        } else if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
            with_view_model(
                instance->view, (SubBruteAttackViewModel * model) {
                    if(event->key == InputKeyLeft) {
                        model->current_step =
                            ((model->current_step - 1) + model->max_value) % model->max_value;
                    } else if(event->key == InputKeyRight) {
                        model->current_step = (model->current_step + 1) % model->max_value;
                    }
                    return true;
                });
            instance->callback(SubBruteCustomEventTypeChangeStep, instance->context);
        }
    } else {
        if((event->type == InputTypeShort || event->type == InputTypeRepeat) &&
           (event->key == InputKeyOk || event->key == InputKeyBack)) {
            with_view_model(
                instance->view, (SubBruteAttackViewModel * model) {
                    model->is_attacking = false;
                    return true;
                });
            instance->callback(SubBruteCustomEventTypeTransmitNotStarted, instance->context);
        }
    }

    return true;
}

SubBruteAttackView* subbrute_attack_view_alloc() {
    SubBruteAttackView* instance = malloc(sizeof(SubBruteAttackView));

    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(SubBruteAttackViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)subbrute_attack_view_draw);
    view_set_input_callback(instance->view, subbrute_attack_view_input);
    view_set_enter_callback(instance->view, subbrute_attack_view_enter);
    view_set_exit_callback(instance->view, subbrute_attack_view_exit);

    //instance->worker = subbrute_worker_alloc();

    return instance;
}

void subbrute_attack_view_enter(void* context) {
    furi_assert(context);
}

void subbrute_attack_view_free(SubBruteAttackView* instance) {
    furi_assert(instance);

    subbrute_worker_free(instance->worker);

    view_free(instance->view);
    free(instance);
}

View* subbrute_attack_view_get_view(SubBruteAttackView* instance) {
    furi_assert(instance);
    return instance->view;
}

void subbrute_attack_view_set_current_step(SubBruteAttackView* instance, uint8_t current_step) {
    with_view_model(
        instance->view, (SubBruteAttackViewModel * model) {
            model->current_step = current_step;
            return true;
        });
}

uint8_t subbrute_attack_view_get_current_step(SubBruteAttackView* instance) {
    uint8_t current_step;
    with_view_model(
        instance->view, (SubBruteAttackViewModel * model) {
            current_step = model->current_step;
            return false;
        });

    return current_step;
}

// We need to call init every time, because not every time we calls enter
// normally, call enter only once
void subbrute_attack_view_init_values(
    SubBruteAttackView* instance,
    uint8_t index,
    uint8_t max_value,
    uint8_t current_step) {
    with_view_model(
        instance->view, (SubBruteAttackViewModel * model) {
            model->max_value = max_value;
            model->index = index;
            model->current_step = current_step;
            return true;
        });
}

void subbrute_attack_view_stop_worker(SubBruteAttackView* instance) {
    furi_assert(instance);
    subbrute_worker_stop(instance->worker);
}

bool subbrute_attack_view_can_send(SubBruteAttackView* instance) {
    furi_assert(instance);

    return subbrute_worker_can_transmit(instance->worker);
}

void subbrute_attack_view_start_worker(
    SubBruteAttackView* instance,
    uint32_t frequency,
    FuriHalSubGhzPreset preset,
    string_t protocol_name) {
    furi_assert(instance);
    if(!subbrute_worker_is_running(instance->worker)) {
        subbrute_worker_start(instance->worker, frequency, preset, protocol_name);
    }
}

bool subbrute_attack_view_transmit(SubBruteAttackView* instance, string_t payload) {
    furi_assert(instance);

    return subbrute_worker_transmit(instance->worker, payload);
}

bool subbrute_attack_view_is_worker_running(SubBruteAttackView* instance) {
    furi_assert(instance);

    return subbrute_worker_is_running(instance->worker);
}

void subbrute_attack_view_exit(void* context) {
    furi_assert(context);
    SubBruteAttackView* instance = context;

    // Just stop, make free in free method
    subbrute_worker_stop(instance->worker);
}

void elements_button_top_left(Canvas* canvas, const char* str) {
    const Icon* icon = &I_ButtonUp_7x4;

    const uint8_t button_height = 12;
    const uint8_t vertical_offset = 3;
    const uint8_t horizontal_offset = 3;
    const uint8_t string_width = canvas_string_width(canvas, str);
    const uint8_t icon_h_offset = 3;
    const uint8_t icon_width_with_offset = icon->width + icon_h_offset;
    const uint8_t icon_v_offset = icon->height + vertical_offset;
    const uint8_t button_width = string_width + horizontal_offset * 2 + icon_width_with_offset;

    const uint8_t x = 0;
    const uint8_t y = 0;

    canvas_draw_box(canvas, x, y - button_height, button_width, button_height);
    canvas_draw_line(canvas, x + button_width - 0, y, x + button_width - 0, y + button_height - 0);
    canvas_draw_line(canvas, x + button_width - 1, y, x + button_width - 1, y + button_height - 1);
    canvas_draw_line(canvas, x + button_width - 2, y, x + button_width - 2, y + button_height - 2);

    canvas_invert_color(canvas);
    canvas_draw_icon(canvas, x + horizontal_offset, y + icon_v_offset, icon);
    canvas_draw_str(
        canvas, x + horizontal_offset + icon_width_with_offset, y + vertical_offset, str);
    canvas_invert_color(canvas);
}

void elements_button_top_right(Canvas* canvas, const char* str) {
    const Icon* icon = &I_ButtonDown_7x4;

    const uint8_t button_height = 12;
    const uint8_t vertical_offset = 3;
    const uint8_t horizontal_offset = 3;
    const uint8_t string_width = canvas_string_width(canvas, str);
    const uint8_t icon_h_offset = 3;
    const uint8_t icon_width_with_offset = icon->width + icon_h_offset;
    const uint8_t icon_v_offset = icon->height + vertical_offset;
    const uint8_t button_width = string_width + horizontal_offset * 2 + icon_width_with_offset;

    const uint8_t x = canvas_width(canvas);
    const uint8_t y = 0;

    canvas_draw_box(canvas, x - button_width, y - button_height, button_width, button_height);
    canvas_draw_line(canvas, x - button_width - 1, y, x + button_width + 1, y + button_height - 0);
    canvas_draw_line(canvas, x - button_width - 2, y, x + button_width + 2, y + button_height - 1);
    canvas_draw_line(canvas, x - button_width - 3, y, x + button_width + 3, y + button_height - 2);

    canvas_invert_color(canvas);
    canvas_draw_str(canvas, x - button_width + horizontal_offset, y - vertical_offset, str);
    canvas_draw_icon(canvas, x - horizontal_offset - icon->width, y - icon_v_offset, icon);
    canvas_invert_color(canvas);
}

void subbrute_attack_view_draw(Canvas* canvas, void* context) {
    furi_assert(context);
    SubBruteAttackViewModel* model = context;
    //char buffer[64];

    // Title
    const char* attack_name = NULL;
    attack_name = subbrute_get_menu_name(model->index);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 20, 8, attack_name ? attack_name : "Sub-GHz Bruteforcer");

    // Progress bar
    // Resolution: 128x64 px
    float progress_value = model->max_value / model->current_step;
    elements_progress_bar(canvas, 8, 30, 98, progress_value > 1 ? 1 : progress_value);

    // Selected attack type
    //    const char* attack_name = NULL;
    //    attack_name = subbrute_get_menu_name(model->index);
    //
    //    canvas_set_font(canvas, FontSecondary);
    //    if(attack_name) {
    //        snprintf(buffer, sizeof(buffer), "%s", attack_name);
    //    } else {
    //        snprintf(buffer, sizeof(buffer), "%s", "Unknown";
    //    }
    //    canvas_draw_str(canvas, 9, 42, buffer);

    if(!model->is_attacking) {
        elements_button_left(canvas, "-1");
        elements_button_right(canvas, "+1");
        elements_button_center(canvas, "Start");
        elements_button_top_left(canvas, "Save");
        elements_button_top_right(canvas, "Repeat");
    } else {
        elements_button_center(canvas, "Stop");
    }
}
