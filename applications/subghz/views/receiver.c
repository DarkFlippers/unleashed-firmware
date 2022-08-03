#include "receiver.h"
#include "../subghz_i.h"
#include <math.h>

#include <input/input.h>
#include <gui/elements.h>
#include <assets_icons.h>
#include <m-string.h>
#include <m-array.h>

#define FRAME_HEIGHT 12
#define MAX_LEN_PX 100
#define MENU_ITEMS 4u
#define UNLOCK_CNT 3

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
    [SubGhzProtocolTypeUnknown] = &I_Quest_7x8,
    [SubGhzProtocolTypeStatic] = &I_Unlock_7x8,
    [SubGhzProtocolTypeDynamic] = &I_Lock_7x8,
};

typedef enum {
    SubGhzViewReceiverBarShowDefault,
    SubGhzViewReceiverBarShowLock,
    SubGhzViewReceiverBarShowToUnlockPress,
    SubGhzViewReceiverBarShowUnlock,
} SubGhzViewReceiverBarShow;

struct SubGhzViewReceiver {
    SubGhzLock lock;
    uint8_t lock_count;
    FuriTimer* timer;
    View* view;
    SubGhzViewReceiverCallback callback;
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
    SubGhzViewReceiverBarShow bar_show;
} SubGhzViewReceiverModel;

void subghz_view_receiver_set_lock(SubGhzViewReceiver* subghz_receiver, SubGhzLock lock) {
    furi_assert(subghz_receiver);
    subghz_receiver->lock_count = 0;
    if(lock == SubGhzLockOn) {
        subghz_receiver->lock = lock;
        with_view_model(
            subghz_receiver->view, (SubGhzViewReceiverModel * model) {
                model->bar_show = SubGhzViewReceiverBarShowLock;
                return true;
            });
        furi_timer_start(subghz_receiver->timer, pdMS_TO_TICKS(1000));
    } else {
        with_view_model(
            subghz_receiver->view, (SubGhzViewReceiverModel * model) {
                model->bar_show = SubGhzViewReceiverBarShowDefault;
                return true;
            });
    }
}

void subghz_view_receiver_set_callback(
    SubGhzViewReceiver* subghz_receiver,
    SubGhzViewReceiverCallback callback,
    void* context) {
    furi_assert(subghz_receiver);
    furi_assert(callback);
    subghz_receiver->callback = callback;
    subghz_receiver->context = context;
}

static void subghz_view_receiver_update_offset(SubGhzViewReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);

    with_view_model(
        subghz_receiver->view, (SubGhzViewReceiverModel * model) {
            size_t history_item = model->history_item;
            uint16_t bounds = history_item > 3 ? 2 : history_item;

            if(history_item > 3 && model->idx >= (int16_t)(history_item - 1)) {
                model->list_offset = model->idx - 3;
            } else if(model->list_offset < model->idx - bounds) {
                model->list_offset =
                    CLAMP(model->list_offset + 1, (int16_t)(history_item - bounds), 0);
            } else if(model->list_offset > model->idx - bounds) {
                model->list_offset = CLAMP(model->idx - 1, (int16_t)(history_item - bounds), 0);
            }
            return true;
        });
}

void subghz_view_receiver_add_item_to_menu(
    SubGhzViewReceiver* subghz_receiver,
    const char* name,
    uint8_t type) {
    furi_assert(subghz_receiver);
    with_view_model(
        subghz_receiver->view, (SubGhzViewReceiverModel * model) {
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
    subghz_view_receiver_update_offset(subghz_receiver);
}

void subghz_view_receiver_add_data_statusbar(
    SubGhzViewReceiver* subghz_receiver,
    const char* frequency_str,
    const char* preset_str,
    const char* history_stat_str) {
    furi_assert(subghz_receiver);
    with_view_model(
        subghz_receiver->view, (SubGhzViewReceiverModel * model) {
            string_set_str(model->frequency_str, frequency_str);
            string_set_str(model->preset_str, preset_str);
            string_set_str(model->history_stat_str, history_stat_str);
            return true;
        });
}

static void subghz_view_receiver_draw_frame(Canvas* canvas, uint16_t idx, bool scrollbar) {
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

void subghz_view_receiver_draw(Canvas* canvas, SubGhzViewReceiverModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    elements_button_left(canvas, "Config");
    canvas_draw_line(canvas, 46, 51, 125, 51);

    bool scrollbar = model->history_item > 4;
    string_t str_buff;
    string_init(str_buff);

    SubGhzReceiverMenuItem* item_menu;

    for(size_t i = 0; i < MIN(model->history_item, MENU_ITEMS); ++i) {
        size_t idx = CLAMP((uint16_t)(i + model->list_offset), model->history_item, 0);
        item_menu = SubGhzReceiverMenuItemArray_get(model->history->data, idx);
        string_set(str_buff, item_menu->item_str);
        elements_string_fit_width(canvas, str_buff, scrollbar ? MAX_LEN_PX - 6 : MAX_LEN_PX);
        if(model->idx == idx) {
            subghz_view_receiver_draw_frame(canvas, i, scrollbar);
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

    canvas_set_color(canvas, ColorBlack);

    if(model->history_item == 0) {
        canvas_draw_icon(canvas, 0, 0, &I_Scanning_123x52);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 63, 46, "Scanning...");
        canvas_draw_line(canvas, 46, 51, 125, 51);
        canvas_set_font(canvas, FontSecondary);
    }

    switch(model->bar_show) {
    case SubGhzViewReceiverBarShowLock:
        canvas_draw_icon(canvas, 64, 55, &I_Lock_7x8);
        canvas_draw_str(canvas, 74, 62, "Locked");
        break;
    case SubGhzViewReceiverBarShowToUnlockPress:
        canvas_draw_str(canvas, 44, 62, string_get_cstr(model->frequency_str));
        canvas_draw_str(canvas, 79, 62, string_get_cstr(model->preset_str));
        canvas_draw_str(canvas, 96, 62, string_get_cstr(model->history_stat_str));
        canvas_set_font(canvas, FontSecondary);
        elements_bold_rounded_frame(canvas, 14, 8, 99, 48);
        elements_multiline_text(canvas, 65, 26, "To unlock\npress:");
        canvas_draw_icon(canvas, 65, 42, &I_Pin_back_arrow_10x8);
        canvas_draw_icon(canvas, 80, 42, &I_Pin_back_arrow_10x8);
        canvas_draw_icon(canvas, 95, 42, &I_Pin_back_arrow_10x8);
        canvas_draw_icon(canvas, 16, 13, &I_WarningDolphin_45x42);
        canvas_draw_dot(canvas, 17, 61);
        break;
    case SubGhzViewReceiverBarShowUnlock:
        canvas_draw_icon(canvas, 64, 55, &I_Unlock_7x8);
        canvas_draw_str(canvas, 74, 62, "Unlocked");
        break;
    default:
        canvas_draw_str(canvas, 44, 62, string_get_cstr(model->frequency_str));
        canvas_draw_str(canvas, 79, 62, string_get_cstr(model->preset_str));
        canvas_draw_str(canvas, 96, 62, string_get_cstr(model->history_stat_str));
        break;
    }
}

static void subghz_view_receiver_timer_callback(void* context) {
    furi_assert(context);
    SubGhzViewReceiver* subghz_receiver = context;
    with_view_model(
        subghz_receiver->view, (SubGhzViewReceiverModel * model) {
            model->bar_show = SubGhzViewReceiverBarShowDefault;
            return true;
        });
    if(subghz_receiver->lock_count < UNLOCK_CNT) {
        subghz_receiver->callback(
            SubGhzCustomEventViewReceiverOffDisplay, subghz_receiver->context);
    } else {
        subghz_receiver->lock = SubGhzLockOff;
        subghz_receiver->callback(SubGhzCustomEventViewReceiverUnlock, subghz_receiver->context);
    }
    subghz_receiver->lock_count = 0;
}

bool subghz_view_receiver_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubGhzViewReceiver* subghz_receiver = context;

    if(subghz_receiver->lock == SubGhzLockOn) {
        with_view_model(
            subghz_receiver->view, (SubGhzViewReceiverModel * model) {
                model->bar_show = SubGhzViewReceiverBarShowToUnlockPress;
                return true;
            });
        if(subghz_receiver->lock_count == 0) {
            furi_timer_start(subghz_receiver->timer, pdMS_TO_TICKS(1000));
        }
        if(event->key == InputKeyBack && event->type == InputTypeShort) {
            subghz_receiver->lock_count++;
        }
        if(subghz_receiver->lock_count >= UNLOCK_CNT) {
            // subghz_receiver->callback(
            //     SubGhzCustomEventViewReceiverUnlock, subghz_receiver->context);
            with_view_model(
                subghz_receiver->view, (SubGhzViewReceiverModel * model) {
                    model->bar_show = SubGhzViewReceiverBarShowUnlock;
                    return true;
                });
            //subghz_receiver->lock = SubGhzLockOff;
            furi_timer_start(subghz_receiver->timer, pdMS_TO_TICKS(650));
        }

        return true;
    }

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        subghz_receiver->callback(SubGhzCustomEventViewReceiverBack, subghz_receiver->context);
    } else if(
        event->key == InputKeyUp &&
        (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
        with_view_model(
            subghz_receiver->view, (SubGhzViewReceiverModel * model) {
                if(model->idx != 0) model->idx--;
                return true;
            });
    } else if(
        event->key == InputKeyDown &&
        (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
        with_view_model(
            subghz_receiver->view, (SubGhzViewReceiverModel * model) {
                if(model->idx != model->history_item - 1) model->idx++;
                return true;
            });
    } else if(event->key == InputKeyLeft && event->type == InputTypeShort) {
        subghz_receiver->callback(SubGhzCustomEventViewReceiverConfig, subghz_receiver->context);
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        with_view_model(
            subghz_receiver->view, (SubGhzViewReceiverModel * model) {
                if(model->history_item != 0) {
                    subghz_receiver->callback(
                        SubGhzCustomEventViewReceiverOK, subghz_receiver->context);
                }
                return false;
            });
    }

    subghz_view_receiver_update_offset(subghz_receiver);

    return true;
}

void subghz_view_receiver_enter(void* context) {
    furi_assert(context);
}

void subghz_view_receiver_exit(void* context) {
    furi_assert(context);
    SubGhzViewReceiver* subghz_receiver = context;
    with_view_model(
        subghz_receiver->view, (SubGhzViewReceiverModel * model) {
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
    furi_timer_stop(subghz_receiver->timer);
}

SubGhzViewReceiver* subghz_view_receiver_alloc() {
    SubGhzViewReceiver* subghz_receiver = malloc(sizeof(SubGhzViewReceiver));

    // View allocation and configuration
    subghz_receiver->view = view_alloc();

    subghz_receiver->lock = SubGhzLockOff;
    subghz_receiver->lock_count = 0;
    view_allocate_model(
        subghz_receiver->view, ViewModelTypeLocking, sizeof(SubGhzViewReceiverModel));
    view_set_context(subghz_receiver->view, subghz_receiver);
    view_set_draw_callback(subghz_receiver->view, (ViewDrawCallback)subghz_view_receiver_draw);
    view_set_input_callback(subghz_receiver->view, subghz_view_receiver_input);
    view_set_enter_callback(subghz_receiver->view, subghz_view_receiver_enter);
    view_set_exit_callback(subghz_receiver->view, subghz_view_receiver_exit);

    with_view_model(
        subghz_receiver->view, (SubGhzViewReceiverModel * model) {
            string_init(model->frequency_str);
            string_init(model->preset_str);
            string_init(model->history_stat_str);
            model->bar_show = SubGhzViewReceiverBarShowDefault;
            model->history = malloc(sizeof(SubGhzReceiverHistory));
            SubGhzReceiverMenuItemArray_init(model->history->data);
            return true;
        });
    subghz_receiver->timer =
        furi_timer_alloc(subghz_view_receiver_timer_callback, FuriTimerTypeOnce, subghz_receiver);
    return subghz_receiver;
}

void subghz_view_receiver_free(SubGhzViewReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);

    with_view_model(
        subghz_receiver->view, (SubGhzViewReceiverModel * model) {
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
    furi_timer_free(subghz_receiver->timer);
    view_free(subghz_receiver->view);
    free(subghz_receiver);
}

View* subghz_view_receiver_get_view(SubGhzViewReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);
    return subghz_receiver->view;
}

uint16_t subghz_view_receiver_get_idx_menu(SubGhzViewReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);
    uint32_t idx = 0;
    with_view_model(
        subghz_receiver->view, (SubGhzViewReceiverModel * model) {
            idx = model->idx;
            return false;
        });
    return idx;
}

void subghz_view_receiver_set_idx_menu(SubGhzViewReceiver* subghz_receiver, uint16_t idx) {
    furi_assert(subghz_receiver);
    with_view_model(
        subghz_receiver->view, (SubGhzViewReceiverModel * model) {
            model->idx = idx;
            if(model->idx > 2) model->list_offset = idx - 2;
            return true;
        });
    subghz_view_receiver_update_offset(subghz_receiver);
}
