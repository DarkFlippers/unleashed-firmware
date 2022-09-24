#include "subbrute_main_view.h"
#include "../subbrute_i.h"

#include <input/input.h>
#include <gui/elements.h>
#include <gui/icon.h>

#define STATUS_BAR_Y_SHIFT 13

struct SubBruteMainView {
    View* view;
    SubBruteMainViewCallback callback;
    void* context;
};

typedef struct {
    uint8_t index;
} SubBruteMainViewModel;

void subbrute_main_view_set_callback(
    SubBruteMainView* instance,
    SubBruteMainViewCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

void subbrute_main_view_draw(Canvas* canvas, SubBruteMainViewModel* model) {
    SubBruteMainViewModel* m = model;

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    for(uint8_t i = 1; i < SubBruteAttackTotalCount - 1; ++i) {
        const char* str = subbrute_get_menu_name(i);
        canvas_draw_str_aligned(
            canvas, 64, 9 + (i * 17) + STATUS_BAR_Y_SHIFT, AlignCenter, AlignCenter, str);

        if(m->index == i) {
            elements_frame(canvas, 15, 1 + (i * 17) + STATUS_BAR_Y_SHIFT, 98, 15);
        }
    }
}

bool subbrute_main_view_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SubBruteMainView* instance = context;
    const uint8_t min_value = SubBruteAttackNone + 1;
    const uint8_t correct_total = SubBruteAttackTotalCount - 1;
    //uint8_t idx = min_value;
    bool consumed = false;
    if((event->type == InputTypeShort) || (event->type == InputTypeRepeat)) {
        with_view_model(
            instance->view, (SubBruteMainViewModel * model) {
                bool ret = false;
                if(event->key == InputKeyUp) {
                    if(model->index == min_value) {
                        model->index = correct_total;
                    } else {
                        model->index = CLAMP(model->index - 1, correct_total, min_value);
                    }
                    ret = true;
                    consumed = true;
                } else if(event->key == InputKeyDown) {
                    if(model->index == correct_total) {
                        model->index = min_value;
                    } else {
                        model->index = CLAMP(model->index + 1, correct_total, min_value);
                    }
                    ret = true;
                    consumed = true;
                }
                if(ret) {
                    model->index++;
                }
                //idx = model->index;
                return ret;
            });
    }

    if(event->key == InputKeyOk && event->type == InputTypeShort) {
        /*if(idx == SubBruteAttackLoadFile) {
            instance->callback(SubBruteCustomEventTypeLoadFile, instance->context);
        } else if(idx > SubBruteAttackNone && idx < SubBruteAttackLoadFile) {*/
        instance->callback(SubBruteCustomEventTypeMenuSelected, instance->context);
        /*}*/
        consumed = true;
    }

    return consumed;
}

void subbrute_main_view_enter(void* context) {
    furi_assert(context);
}

void subbrute_main_view_exit(void* context) {
    furi_assert(context);
}

SubBruteMainView* subbrute_main_view_alloc() {
    SubBruteMainView* instance = malloc(sizeof(SubBruteMainView));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(SubBruteMainViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)subbrute_main_view_draw);
    view_set_input_callback(instance->view, subbrute_main_view_input);
    view_set_enter_callback(instance->view, subbrute_main_view_enter);
    view_set_exit_callback(instance->view, subbrute_main_view_exit);

    return instance;
}

void subbrute_main_view_free(SubBruteMainView* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* subbrute_main_view_get_view(SubBruteMainView* instance) {
    furi_assert(instance);
    return instance->view;
}

void subbrute_main_view_set_index(SubBruteMainView* instance, uint8_t idx) {
    furi_assert(instance);
    furi_assert(idx < SubBruteAttackTotalCount - 2);
    with_view_model(
        instance->view, (SubBruteMainViewModel * model) {
            model->index = idx <= 0 ? 1 : idx;
            return true;
        });
}

SubBruteAttacks subbrute_main_view_get_index(SubBruteMainView* instance) {
    furi_assert(instance);

    uint8_t attack = 0;
    with_view_model(
        instance->view, (SubBruteMainViewModel * model) {
            attack = model->index;
            return false;
        });

    return attack;
}