#include "subghz_receiver.h"
#include "../subghz_i.h"
#include <math.h>

#include <input/input.h>
#include <gui/elements.h>
#include <assets_icons.h>
#include <m-string.h>
#include <m-array.h>

#define FRAME_HEIGHT 12
#define MAX_LEN_PX 100
#define MENU_ITEMS 4

typedef struct {
    string_t item_str;
    uint8_t type;
} SubGhzReceiverMenuItem;

ARRAY_DEF(SubGhzReceiverMenuItemArray, SubGhzReceiverMenuItem, M_POD_OPLIST)

#define M_OPL_SubGhzReceiverMenuItemArray_t() \
    ARRAY_OPLIST(SubGhzReceiverMenuItemArray, M_POD_OPLIST)

struct SubGhzReceiverHistory {
    SubGhzReceiverMenuItemArray_t data;
};

typedef struct SubGhzReceiverHistory SubGhzReceiverHistory;

static const Icon* ReceiverItemIcons[] = {
    [SubGhzProtocolCommonTypeUnknown] = &I_Quest_7x8,
    [SubGhzProtocolCommonTypeStatic] = &I_Unlock_7x8,
    [SubGhzProtocolCommonTypeDynamic] = &I_Lock_7x8,
};

struct SubghzReceiver {
    View* view;
    SubghzReceiverCallback callback;
    void* context;
};

typedef struct {
    string_t frequency_str;
    string_t preset_str;
    string_t history_stat_str;
    SubGhzReceiverHistory* history;
    uint16_t idx;
    uint16_t list_offset;
    uint16_t history_item;
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

static void subghz_receiver_update_offset(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);

    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            size_t history_item = model->history_item;
            uint16_t bounds = history_item > 3 ? 2 : history_item;

            if(history_item > 3 && model->idx >= history_item - 1) {
                model->list_offset = model->idx - 3;
            } else if(model->list_offset < model->idx - bounds) {
                model->list_offset = CLAMP(model->list_offset + 1, history_item - bounds, 0);
            } else if(model->list_offset > model->idx - bounds) {
                model->list_offset = CLAMP(model->idx - 1, history_item - bounds, 0);
            }
            return true;
        });
}

void subghz_receiver_add_item_to_menu(
    SubghzReceiver* subghz_receiver,
    const char* name,
    uint8_t type) {
    furi_assert(subghz_receiver);
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            SubGhzReceiverMenuItem* item_menu =
                SubGhzReceiverMenuItemArray_push_raw(model->history->data);
            string_init_set_str(item_menu->item_str, name);
            item_menu->type = type;
            if((model->idx == model->history_item - 1)) {
                model->history_item++;
                model->idx++;
            } else {
                model->history_item++;
            }

            return true;
        });
    subghz_receiver_update_offset(subghz_receiver);
}

void subghz_receiver_add_data_statusbar(
    SubghzReceiver* subghz_receiver,
    const char* frequency_str,
    const char* preset_str,
    const char* history_stat_str) {
    furi_assert(subghz_receiver);
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            string_set(model->frequency_str, frequency_str);
            string_set(model->preset_str, preset_str);
            string_set(model->history_stat_str, history_stat_str);
            return true;
        });
}

static void subghz_receiver_draw_frame(Canvas* canvas, uint16_t idx, bool scrollbar) {
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0 + idx * FRAME_HEIGHT, scrollbar ? 122 : 127, FRAME_HEIGHT);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_dot(canvas, 0, 0 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, 1, 0 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, 0, (0 + idx * FRAME_HEIGHT) + 1);

    canvas_draw_dot(canvas, 0, (0 + idx * FRAME_HEIGHT) + 11);
    canvas_draw_dot(canvas, scrollbar ? 121 : 126, 0 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, scrollbar ? 121 : 126, (0 + idx * FRAME_HEIGHT) + 11);
}

void subghz_receiver_draw(Canvas* canvas, SubghzReceiverModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    elements_button_left(canvas, "Config");

    canvas_draw_str(canvas, 44, 62, string_get_cstr(model->frequency_str));
    canvas_draw_str(canvas, 79, 62, string_get_cstr(model->preset_str));
    canvas_draw_str(canvas, 96, 62, string_get_cstr(model->history_stat_str));
    if(model->history_item == 0) {
        canvas_draw_icon(canvas, 0, 0, &I_Scanning_123x52);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 63, 46, "Scanning...");
        canvas_draw_line(canvas, 46, 51, 125, 51);
        return;
    }
    canvas_draw_line(canvas, 46, 51, 125, 51);

    bool scrollbar = model->history_item > 4;
    string_t str_buff;
    string_init(str_buff);

    SubGhzReceiverMenuItem* item_menu;

    for(size_t i = 0; i < MIN(model->history_item, MENU_ITEMS); ++i) {
        size_t idx = CLAMP(i + model->list_offset, model->history_item, 0);
        item_menu = SubGhzReceiverMenuItemArray_get(model->history->data, idx);
        string_set(str_buff, item_menu->item_str);
        elements_string_fit_width(canvas, str_buff, scrollbar ? MAX_LEN_PX - 6 : MAX_LEN_PX);
        if(model->idx == idx) {
            subghz_receiver_draw_frame(canvas, i, scrollbar);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }
        canvas_draw_icon(canvas, 1, 2 + i * FRAME_HEIGHT, ReceiverItemIcons[item_menu->type]);
        canvas_draw_str(canvas, 15, 9 + i * FRAME_HEIGHT, string_get_cstr(str_buff));
        string_reset(str_buff);
    }
    if(scrollbar) {
        elements_scrollbar_pos(canvas, 128, 0, 49, model->idx, model->history_item);
    }
    string_clear(str_buff);
}

bool subghz_receiver_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        subghz_receiver->callback(SubghzCustomEventViewReceverBack, subghz_receiver->context);
    } else if(
        event->key == InputKeyUp &&
        (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
        with_view_model(
            subghz_receiver->view, (SubghzReceiverModel * model) {
                if(model->idx != 0) model->idx--;
                return true;
            });
    } else if(
        event->key == InputKeyDown &&
        (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
        with_view_model(
            subghz_receiver->view, (SubghzReceiverModel * model) {
                if(model->idx != model->history_item - 1) model->idx++;
                return true;
            });
    } else if(event->key == InputKeyLeft && event->type == InputTypeShort) {
        subghz_receiver->callback(SubghzCustomEventViewReceverConfig, subghz_receiver->context);
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        with_view_model(
            subghz_receiver->view, (SubghzReceiverModel * model) {
                if(model->history_item != 0) {
                    subghz_receiver->callback(
                        SubghzCustomEventViewReceverOK, subghz_receiver->context);
                }
                return false;
            });
    }

    subghz_receiver_update_offset(subghz_receiver);

    return true;
}

void subghz_receiver_enter(void* context) {
    furi_assert(context);
    //SubghzReceiver* subghz_receiver = context;
}

void subghz_receiver_exit(void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            string_reset(model->frequency_str);
            string_reset(model->preset_str);
            string_reset(model->history_stat_str);
                for
                    M_EACH(item_menu, model->history->data, SubGhzReceiverMenuItemArray_t) {
                        string_clear(item_menu->item_str);
                        item_menu->type = 0;
                    }
                SubGhzReceiverMenuItemArray_reset(model->history->data);
                model->idx = 0;
                model->list_offset = 0;
                model->history_item = 0;
                return false;
        });
}

SubghzReceiver* subghz_receiver_alloc() {
    SubghzReceiver* subghz_receiver = malloc(sizeof(SubghzReceiver));

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
            string_init(model->frequency_str);
            string_init(model->preset_str);
            string_init(model->history_stat_str);
            model->history = malloc(sizeof(SubGhzReceiverHistory));
            SubGhzReceiverMenuItemArray_init(model->history->data);
            return true;
        });

    return subghz_receiver;
}

void subghz_receiver_free(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);

    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            string_clear(model->frequency_str);
            string_clear(model->preset_str);
            string_clear(model->history_stat_str);
                for
                    M_EACH(item_menu, model->history->data, SubGhzReceiverMenuItemArray_t) {
                        string_clear(item_menu->item_str);
                        item_menu->type = 0;
                    }
                SubGhzReceiverMenuItemArray_clear(model->history->data);
                free(model->history);
                return false;
        });
    view_free(subghz_receiver->view);
    free(subghz_receiver);
}

View* subghz_receiver_get_view(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);
    return subghz_receiver->view;
}

uint16_t subghz_receiver_get_idx_menu(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);
    uint32_t idx = 0;
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            idx = model->idx;
            return false;
        });
    return idx;
}

void subghz_receiver_set_idx_menu(SubghzReceiver* subghz_receiver, uint16_t idx) {
    furi_assert(subghz_receiver);
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            model->idx = idx;
            if(model->idx > 2) model->list_offset = idx - 2;
            return true;
        });
    subghz_receiver_update_offset(subghz_receiver);
}
