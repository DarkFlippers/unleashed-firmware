#include "submenu.h"

#include <m-array.h>
#include <gui/elements.h>
#include <furi.h>

struct Submenu {
    View* view;
};

typedef struct {
    const char* label;
    uint32_t index;
    SubmenuItemCallback callback;
    void* callback_context;
} SubmenuItem;

ARRAY_DEF(SubmenuItemArray, SubmenuItem, M_POD_OPLIST);

typedef struct {
    SubmenuItemArray_t items;
    const char* header;
    uint8_t position;
    uint8_t window_position;
} SubmenuModel;

static void submenu_process_up(Submenu* submenu);
static void submenu_process_down(Submenu* submenu);
static void submenu_process_ok(Submenu* submenu);

static void submenu_view_draw_callback(Canvas* canvas, void* _model) {
    SubmenuModel* model = _model;

    const uint8_t item_height = 16;
    const uint8_t item_width = 123;

    canvas_clear(canvas);

    uint8_t position = 0;
    SubmenuItemArray_it_t it;

    if(model->header) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 4, 11, model->header);
    }

    canvas_set_font(canvas, FontSecondary);
    for(SubmenuItemArray_it(it, model->items); !SubmenuItemArray_end_p(it);
        SubmenuItemArray_next(it)) {
        uint8_t item_position = position - model->window_position;
        uint8_t items_on_screen = model->header ? 3 : 4;
        uint8_t y_offset = model->header ? 16 : 0;

        if(item_position < items_on_screen) {
            if(position == model->position) {
                canvas_set_color(canvas, ColorBlack);
                elements_slightly_rounded_box(
                    canvas,
                    0,
                    y_offset + (item_position * item_height) + 1,
                    item_width,
                    item_height - 2);
                canvas_set_color(canvas, ColorWhite);
            } else {
                canvas_set_color(canvas, ColorBlack);
            }

            string_t disp_str;
            string_init_set_str(disp_str, SubmenuItemArray_cref(it)->label);
            elements_string_fit_width(canvas, disp_str, item_width - 20);

            canvas_draw_str(
                canvas,
                6,
                y_offset + (item_position * item_height) + item_height - 4,
                string_get_cstr(disp_str));

            string_clear(disp_str);
        }

        position++;
    }

    elements_scrollbar(canvas, model->position, SubmenuItemArray_size(model->items));
}

static bool submenu_view_input_callback(InputEvent* event, void* context) {
    Submenu* submenu = context;
    furi_assert(submenu);
    bool consumed = false;

    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyUp:
            consumed = true;
            submenu_process_up(submenu);
            break;
        case InputKeyDown:
            consumed = true;
            submenu_process_down(submenu);
            break;
        case InputKeyOk:
            consumed = true;
            submenu_process_ok(submenu);
            break;
        default:
            break;
        }
    } else if(event->type == InputTypeRepeat) {
        if(event->key == InputKeyUp) {
            consumed = true;
            submenu_process_up(submenu);
        } else if(event->key == InputKeyDown) {
            consumed = true;
            submenu_process_down(submenu);
        }
    }

    return consumed;
}

Submenu* submenu_alloc() {
    Submenu* submenu = malloc(sizeof(Submenu));
    submenu->view = view_alloc();
    view_set_context(submenu->view, submenu);
    view_allocate_model(submenu->view, ViewModelTypeLocking, sizeof(SubmenuModel));
    view_set_draw_callback(submenu->view, submenu_view_draw_callback);
    view_set_input_callback(submenu->view, submenu_view_input_callback);

    with_view_model(
        submenu->view, (SubmenuModel * model) {
            SubmenuItemArray_init(model->items);
            model->position = 0;
            model->window_position = 0;
            model->header = NULL;
            return true;
        });

    return submenu;
}

void submenu_free(Submenu* submenu) {
    furi_assert(submenu);

    with_view_model(
        submenu->view, (SubmenuModel * model) {
            SubmenuItemArray_clear(model->items);
            return true;
        });
    view_free(submenu->view);
    free(submenu);
}

View* submenu_get_view(Submenu* submenu) {
    furi_assert(submenu);
    return submenu->view;
}

void submenu_add_item(
    Submenu* submenu,
    const char* label,
    uint32_t index,
    SubmenuItemCallback callback,
    void* callback_context) {
    SubmenuItem* item = NULL;
    furi_assert(label);
    furi_assert(submenu);

    with_view_model(
        submenu->view, (SubmenuModel * model) {
            item = SubmenuItemArray_push_new(model->items);
            item->label = label;
            item->index = index;
            item->callback = callback;
            item->callback_context = callback_context;
            return true;
        });
}

void submenu_reset(Submenu* submenu) {
    furi_assert(submenu);

    with_view_model(
        submenu->view, (SubmenuModel * model) {
            SubmenuItemArray_reset(model->items);
            model->position = 0;
            model->window_position = 0;
            model->header = NULL;
            return true;
        });
}

void submenu_set_selected_item(Submenu* submenu, uint32_t index) {
    with_view_model(
        submenu->view, (SubmenuModel * model) {
            uint32_t position = 0;
            SubmenuItemArray_it_t it;
            for(SubmenuItemArray_it(it, model->items); !SubmenuItemArray_end_p(it);
                SubmenuItemArray_next(it)) {
                if(index == SubmenuItemArray_cref(it)->index) {
                    break;
                }
                position++;
            }

            if(position >= SubmenuItemArray_size(model->items)) {
                position = 0;
            }

            model->position = position;
            model->window_position = position;

            if(model->window_position > 0) {
                model->window_position -= 1;
            }

            uint8_t items_on_screen = model->header ? 3 : 4;

            if(SubmenuItemArray_size(model->items) <= items_on_screen) {
                model->window_position = 0;
            } else {
                if(model->window_position >=
                   (SubmenuItemArray_size(model->items) - items_on_screen)) {
                    model->window_position =
                        (SubmenuItemArray_size(model->items) - items_on_screen);
                }
            }

            return true;
        });
}

void submenu_process_up(Submenu* submenu) {
    with_view_model(
        submenu->view, (SubmenuModel * model) {
            uint8_t items_on_screen = model->header ? 3 : 4;
            if(model->position > 0) {
                model->position--;
                if(((model->position - model->window_position) < 1) &&
                   model->window_position > 0) {
                    model->window_position--;
                }
            } else {
                model->position = SubmenuItemArray_size(model->items) - 1;
                if(model->position > (items_on_screen - 1)) {
                    model->window_position = model->position - (items_on_screen - 1);
                }
            }
            return true;
        });
}

void submenu_process_down(Submenu* submenu) {
    with_view_model(
        submenu->view, (SubmenuModel * model) {
            uint8_t items_on_screen = model->header ? 3 : 4;
            if(model->position < (SubmenuItemArray_size(model->items) - 1)) {
                model->position++;
                if((model->position - model->window_position) > (items_on_screen - 2) &&
                   model->window_position <
                       (SubmenuItemArray_size(model->items) - items_on_screen)) {
                    model->window_position++;
                }
            } else {
                model->position = 0;
                model->window_position = 0;
            }
            return true;
        });
}

void submenu_process_ok(Submenu* submenu) {
    SubmenuItem* item = NULL;

    with_view_model(
        submenu->view, (SubmenuModel * model) {
            if(model->position < (SubmenuItemArray_size(model->items))) {
                item = SubmenuItemArray_get(model->items, model->position);
            }
            return true;
        });

    if(item && item->callback) {
        item->callback(item->callback_context, item->index);
    }
}

void submenu_set_header(Submenu* submenu, const char* header) {
    furi_assert(submenu);

    with_view_model(
        submenu->view, (SubmenuModel * model) {
            model->header = header;
            return true;
        });
}
