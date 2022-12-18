#include "subbrute_main_view.h"
#include "../subbrute_i.h"

#include <input/input.h>
#include <gui/elements.h>
#include "assets_icons.h"
#include <gui/icon.h>

#define STATUS_BAR_Y_SHIFT 14
#define TAG "SubBruteMainView"

struct SubBruteMainView {
    View* view;
    SubBruteMainViewCallback callback;
    void* context;
};

typedef struct {
    uint8_t index;
    uint8_t window_position;
    bool is_select_byte;
    const char* key_field;
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

void center_displayed_key(string_t result, const char* key_cstr, uint8_t index) {
    uint8_t str_index = (index * 3);

    char display_menu[] = {
        'X', 'X', ' ', 'X', 'X', ' ', '<', 'X', 'X', '>', ' ', 'X', 'X', ' ', 'X', 'X', '\0'};

    if(key_cstr != NULL) {
        if(index > 1) {
            display_menu[0] = key_cstr[str_index - 6];
            display_menu[1] = key_cstr[str_index - 5];
        } else {
            display_menu[0] = ' ';
            display_menu[1] = ' ';
        }

        if(index > 0) {
            display_menu[3] = key_cstr[str_index - 3];
            display_menu[4] = key_cstr[str_index - 2];
        } else {
            display_menu[3] = ' ';
            display_menu[4] = ' ';
        }

        display_menu[7] = key_cstr[str_index];
        display_menu[8] = key_cstr[str_index + 1];

        if((str_index + 4) <= (uint8_t)strlen(key_cstr)) {
            display_menu[11] = key_cstr[str_index + 3];
            display_menu[12] = key_cstr[str_index + 4];
        } else {
            display_menu[11] = ' ';
            display_menu[12] = ' ';
        }

        if((str_index + 8) <= (uint8_t)strlen(key_cstr)) {
            display_menu[14] = key_cstr[str_index + 6];
            display_menu[15] = key_cstr[str_index + 7];
        } else {
            display_menu[14] = ' ';
            display_menu[15] = ' ';
        }
    }
    string_init_set_str(result, display_menu);
}

void subbrute_main_view_draw(Canvas* canvas, SubBruteMainViewModel* model) {
    SubBruteMainViewModel* m = model;

    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_box(canvas, 0, 0, canvas_width(canvas), STATUS_BAR_Y_SHIFT);
    canvas_invert_color(canvas);
    canvas_draw_str_aligned(canvas, 64, 3, AlignCenter, AlignTop, "Sub-GHz Bruteforcer");
    canvas_invert_color(canvas);

    if(m->is_select_byte) {
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "key_field: %s", m->key_field);
#endif
        char msg_index[18];
        snprintf(msg_index, sizeof(msg_index), "Field index : %d", m->index);
        canvas_draw_str_aligned(canvas, 64, 26, AlignCenter, AlignTop, msg_index);

        string_t menu_items;
        string_init(menu_items);

        center_displayed_key(menu_items, m->key_field, m->index);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(
            canvas, 64, 40, AlignCenter, AlignTop, string_get_cstr(menu_items));

        elements_button_center(canvas, "Select");
        elements_button_left(canvas, "<");
        elements_button_right(canvas, ">");

        string_reset(menu_items);
    } else {
        // Menu
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontSecondary);
        uint8_t items_on_screen = 3;
        const uint8_t item_height = 16;

#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "window_position: %d, index: %d", model->window_position, m->index);
#endif
        for(uint8_t position = 0; position < SubBruteAttackTotalCount; ++position) {
            uint8_t item_position = position - model->window_position;

            if(item_position < items_on_screen) {
                const char* str = subbrute_get_menu_name(position);
                if(m->index == position) {
                    canvas_draw_str_aligned(
                        canvas,
                        4,
                        9 + (item_position * item_height) + STATUS_BAR_Y_SHIFT,
                        AlignLeft,
                        AlignCenter,
                        str);
                    elements_frame(
                        canvas, 1, 1 + (item_position * item_height) + STATUS_BAR_Y_SHIFT, 124, 15);
                } else {
                    canvas_draw_str_aligned(
                        canvas,
                        4,
                        9 + (item_position * item_height) + STATUS_BAR_Y_SHIFT,
                        AlignLeft,
                        AlignCenter,
                        str);
                }
            }
        }

        elements_scrollbar_pos(
            canvas,
            canvas_width(canvas),
            STATUS_BAR_Y_SHIFT + 2,
            canvas_height(canvas) - STATUS_BAR_Y_SHIFT,
            m->index,
            SubBruteAttackTotalCount);
    }
}

bool subbrute_main_view_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "InputKey: %d", event->key);
#endif

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        return false;
    }

    SubBruteMainView* instance = context;
    const uint8_t min_value = 0;
    const uint8_t correct_total = SubBruteAttackTotalCount - 1;
    uint8_t index = 0;
    bool is_select_byte = false;
    with_view_model(
        instance->view, (SubBruteMainViewModel * model) {
            is_select_byte = model->is_select_byte;
            return false;
        });

    bool consumed = false;
    if(!is_select_byte) {
        if((event->type == InputTypeShort) || (event->type == InputTypeRepeat)) {
            with_view_model(
                instance->view, (SubBruteMainViewModel * model) {
                    bool ret = false;
                    uint8_t items_on_screen = 3;
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
                        model->window_position = model->index;
                        if(model->window_position > 0) {
                            model->window_position -= 1;
                        }

                        if(SubBruteAttackTotalCount <= items_on_screen) {
                            model->window_position = 0;
                        } else {
                            if(model->window_position >=
                               (SubBruteAttackTotalCount - items_on_screen)) {
                                model->window_position =
                                    (SubBruteAttackTotalCount - items_on_screen);
                            }
                        }
                    }
                    index = model->index;
                    return ret;
                });
        }

#ifdef FURI_DEBUG
        with_view_model(
            instance->view, (SubBruteMainViewModel * model) {
                index = model->index;
                return false;
            });
        FURI_LOG_I(TAG, "Index: %d", index);
#endif

        if(event->key == InputKeyOk && event->type == InputTypeShort) {
            if(index == SubBruteAttackLoadFile) {
                instance->callback(SubBruteCustomEventTypeLoadFile, instance->context);
            } else {
                instance->callback(SubBruteCustomEventTypeMenuSelected, instance->context);
            }
            consumed = true;
        }
    } else {
        if((event->type == InputTypeShort) || (event->type == InputTypeRepeat)) {
            with_view_model(
                instance->view, (SubBruteMainViewModel * model) {
                    if(event->key == InputKeyLeft) {
                        if(model->index > 0) {
                            model->index--;
                        }
                    } else if(event->key == InputKeyRight) {
                        if(model->index < 7) {
                            model->index++;
                        }
                    }

                    index = model->index;
                    return true;
                });
        }

#ifdef FURI_DEBUG
        with_view_model(
            instance->view, (SubBruteMainViewModel * model) {
                index = model->index;
                return false;
            });
        FURI_LOG_I(TAG, "Index: %d", index);
#endif

        if(event->key == InputKeyOk && event->type == InputTypeShort) {
            instance->callback(SubBruteCustomEventTypeIndexSelected, instance->context);
            consumed = true;
        }
    }

    return consumed;
}

void subbrute_main_view_enter(void* context) {
    furi_assert(context);

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_main_view_enter");
#endif
}

void subbrute_main_view_exit(void* context) {
    furi_assert(context);

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_main_view_exit");
#endif
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

    with_view_model(
        instance->view, (SubBruteMainViewModel * model) {
            model->index = 0;
            model->window_position = 0;
            model->key_field = NULL;
            model->is_select_byte = false;
            return true;
        });

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

void subbrute_main_view_set_index(
    SubBruteMainView* instance,
    uint8_t idx,
    bool is_select_byte,
    const char* key_field) {
    furi_assert(instance);
    furi_assert(idx < SubBruteAttackTotalCount);
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Set index: %d", idx);
#endif
    with_view_model(
        instance->view, (SubBruteMainViewModel * model) {
            model->is_select_byte = is_select_byte;
            model->key_field = key_field;
            model->index = idx;
            model->window_position = idx;

            if(!is_select_byte) {
                uint8_t items_on_screen = 3;

                if(model->window_position > 0) {
                    model->window_position -= 1;
                }

                if(SubBruteAttackTotalCount <= items_on_screen) {
                    model->window_position = 0;
                } else {
                    if(model->window_position >= (SubBruteAttackTotalCount - items_on_screen)) {
                        model->window_position = (SubBruteAttackTotalCount - items_on_screen);
                    }
                }
            }
            return true;
        });
}

SubBruteAttacks subbrute_main_view_get_index(SubBruteMainView* instance) {
    furi_assert(instance);

    uint8_t idx = 0;
    with_view_model(
        instance->view, (SubBruteMainViewModel * model) {
            idx = model->index;
            return false;
        });

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "Get index: %d", idx);
#endif

    return idx;
}