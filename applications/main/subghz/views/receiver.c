#include "receiver.h"

#include "types.h"
#include <input/input.h>
#include <gui/elements.h>
#include <assets_icons.h>
#include <m-array.h>

#define FRAME_HEIGHT 12
#define MAX_LEN_PX   111
#define MENU_ITEMS   4u
#define UNLOCK_CNT   3

#define SUBGHZ_RAW_THRESHOLD_MIN -90.0f

typedef struct {
    FuriString* item_str;
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
    bool lock;
    uint8_t lock_count;
    FuriTimer* timer;
    View* view;
    SubGhzViewReceiverCallback callback;
    void* context;
};

typedef struct {
    FuriString* frequency_str;
    FuriString* preset_str;
    FuriString* history_stat_str;
    SubGhzReceiverHistory* history;
    uint16_t idx;
    uint16_t list_offset;
    uint16_t history_item;
    SubGhzViewReceiverBarShow bar_show;
    uint8_t u_rssi;
    SubGhzRadioDeviceType device_type;
} SubGhzViewReceiverModel;

void subghz_receiver_rssi(SubGhzViewReceiver* instance, float rssi) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        SubGhzViewReceiverModel * model,
        {
            if(rssi < SUBGHZ_RAW_THRESHOLD_MIN) {
                model->u_rssi = 0;
            } else {
                model->u_rssi = (uint8_t)(rssi - SUBGHZ_RAW_THRESHOLD_MIN);
            }
        },
        true);
}

void subghz_view_receiver_set_lock(SubGhzViewReceiver* subghz_receiver, bool lock) {
    furi_assert(subghz_receiver);
    subghz_receiver->lock_count = 0;

    if(lock == true) {
        subghz_receiver->lock = true;
        with_view_model(
            subghz_receiver->view,
            SubGhzViewReceiverModel * model,
            { model->bar_show = SubGhzViewReceiverBarShowLock; },
            true);
        furi_timer_start(subghz_receiver->timer, 1000);
    } else {
        with_view_model(
            subghz_receiver->view,
            SubGhzViewReceiverModel * model,
            { model->bar_show = SubGhzViewReceiverBarShowDefault; },
            true);
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
        subghz_receiver->view,
        SubGhzViewReceiverModel * model,
        {
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
        },
        true);
}

void subghz_view_receiver_add_item_to_menu(
    SubGhzViewReceiver* subghz_receiver,
    const char* name,
    uint8_t type) {
    furi_assert(subghz_receiver);
    with_view_model(
        subghz_receiver->view,
        SubGhzViewReceiverModel * model,
        {
            SubGhzReceiverMenuItem* item_menu =
                SubGhzReceiverMenuItemArray_push_raw(model->history->data);
            item_menu->item_str = furi_string_alloc_set(name);
            item_menu->type = type;
            if(model->idx == model->history_item - 1) {
                model->history_item++;
                model->idx++;
            } else {
                model->history_item++;
            }
        },
        true);
    subghz_view_receiver_update_offset(subghz_receiver);
}

void subghz_view_receiver_add_data_statusbar(
    SubGhzViewReceiver* subghz_receiver,
    const char* frequency_str,
    const char* preset_str,
    const char* history_stat_str) {
    furi_assert(subghz_receiver);
    with_view_model(
        subghz_receiver->view,
        SubGhzViewReceiverModel * model,
        {
            furi_string_set(model->frequency_str, frequency_str);
            furi_string_set(model->preset_str, preset_str);
            furi_string_set(model->history_stat_str, history_stat_str);
        },
        true);
}

void subghz_view_receiver_set_radio_device_type(
    SubGhzViewReceiver* subghz_receiver,
    SubGhzRadioDeviceType device_type) {
    furi_assert(subghz_receiver);
    with_view_model(
        subghz_receiver->view,
        SubGhzViewReceiverModel * model,
        { model->device_type = device_type; },
        true);
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

static void subghz_view_rssi_draw(Canvas* canvas, SubGhzViewReceiverModel* model) {
    for(uint8_t i = 1; i < model->u_rssi; i++) {
        if(i % 5) {
            canvas_draw_dot(canvas, 46 + i, 52);
            canvas_draw_dot(canvas, 47 + i, 53);
            canvas_draw_dot(canvas, 46 + i, 54);
        }
    }
}

void subghz_view_receiver_draw(Canvas* canvas, SubGhzViewReceiverModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    elements_button_left(canvas, "Config");

    bool scrollbar = model->history_item > 4;
    FuriString* str_buff;
    str_buff = furi_string_alloc();

    SubGhzReceiverMenuItem* item_menu;

    for(size_t i = 0; i < MIN(model->history_item, MENU_ITEMS); ++i) {
        size_t idx = CLAMP((uint16_t)(i + model->list_offset), model->history_item, 0);
        item_menu = SubGhzReceiverMenuItemArray_get(model->history->data, idx);
        furi_string_set(str_buff, item_menu->item_str);
        elements_string_fit_width(canvas, str_buff, scrollbar ? MAX_LEN_PX - 7 : MAX_LEN_PX);
        if(model->idx == idx) {
            subghz_view_receiver_draw_frame(canvas, i, scrollbar);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }
        canvas_draw_icon(canvas, 4, 2 + i * FRAME_HEIGHT, ReceiverItemIcons[item_menu->type]);
        canvas_draw_str(canvas, 15, 9 + i * FRAME_HEIGHT, furi_string_get_cstr(str_buff));
        furi_string_reset(str_buff);
    }
    if(scrollbar) {
        elements_scrollbar_pos(canvas, 128, 0, 49, model->idx, model->history_item);
    }
    furi_string_free(str_buff);

    canvas_set_color(canvas, ColorBlack);

    if(model->history_item == 0) {
        canvas_draw_icon(canvas, 0, 0, &I_Scanning_short_96x52);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 63, 44, "Scanning...");
        canvas_set_font(canvas, FontSecondary);
    }

    if(model->device_type == SubGhzRadioDeviceTypeInternal) {
        canvas_draw_icon(canvas, 109, 0, &I_Internal_ant_1_9x11);
    } else {
        canvas_draw_icon(canvas, 109, 0, &I_External_ant_1_9x11);
    }

    subghz_view_rssi_draw(canvas, model);
    switch(model->bar_show) {
    case SubGhzViewReceiverBarShowLock:
        canvas_draw_icon(canvas, 64, 56, &I_Lock_7x8);
        canvas_draw_str(canvas, 74, 64, "Locked");
        break;
    case SubGhzViewReceiverBarShowToUnlockPress:
        canvas_draw_str(canvas, 44, 64, furi_string_get_cstr(model->frequency_str));
        canvas_draw_str(canvas, 79, 64, furi_string_get_cstr(model->preset_str));
        canvas_draw_str(canvas, 97, 64, furi_string_get_cstr(model->history_stat_str));
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
        canvas_draw_icon(canvas, 64, 56, &I_Unlock_7x8);
        canvas_draw_str(canvas, 74, 64, "Unlocked");
        break;
    default:
        canvas_draw_str(canvas, 44, 64, furi_string_get_cstr(model->frequency_str));
        canvas_draw_str(canvas, 79, 64, furi_string_get_cstr(model->preset_str));
        canvas_draw_str(canvas, 97, 64, furi_string_get_cstr(model->history_stat_str));
        break;
    }
}

static void subghz_view_receiver_timer_callback(void* context) {
    furi_assert(context);
    SubGhzViewReceiver* subghz_receiver = context;
    with_view_model(
        subghz_receiver->view,
        SubGhzViewReceiverModel * model,
        { model->bar_show = SubGhzViewReceiverBarShowDefault; },
        true);
    if(subghz_receiver->lock_count < UNLOCK_CNT) {
        subghz_receiver->callback(
            SubGhzCustomEventViewReceiverOffDisplay, subghz_receiver->context);
    } else {
        subghz_receiver->lock = false;
        subghz_receiver->callback(SubGhzCustomEventViewReceiverUnlock, subghz_receiver->context);
    }
    subghz_receiver->lock_count = 0;
}

bool subghz_view_receiver_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubGhzViewReceiver* subghz_receiver = context;

    if(subghz_receiver->lock == true) {
        with_view_model(
            subghz_receiver->view,
            SubGhzViewReceiverModel * model,
            { model->bar_show = SubGhzViewReceiverBarShowToUnlockPress; },
            true);
        if(subghz_receiver->lock_count == 0) {
            furi_timer_start(subghz_receiver->timer, 1000);
        }
        if(event->key == InputKeyBack && event->type == InputTypeShort) {
            subghz_receiver->lock_count++;
        }
        if(subghz_receiver->lock_count >= UNLOCK_CNT) {
            // subghz_receiver->callback(
            //     SubGhzCustomEventViewReceiverUnlock, subghz_receiver->context);
            with_view_model(
                subghz_receiver->view,
                SubGhzViewReceiverModel * model,
                { model->bar_show = SubGhzViewReceiverBarShowUnlock; },
                true);
            //subghz_receiver->lock = false;
            furi_timer_start(subghz_receiver->timer, 650);
        }

        return true;
    }

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        subghz_receiver->callback(SubGhzCustomEventViewReceiverBack, subghz_receiver->context);
    } else if(
        event->key == InputKeyUp &&
        (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
        with_view_model(
            subghz_receiver->view,
            SubGhzViewReceiverModel * model,
            {
                if(model->idx != 0) model->idx--;
            },
            true);
    } else if(
        event->key == InputKeyDown &&
        (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
        with_view_model(
            subghz_receiver->view,
            SubGhzViewReceiverModel * model,
            {
                if((model->history_item != 0) && (model->idx != model->history_item - 1))
                    model->idx++;
            },
            true);
    } else if(event->key == InputKeyLeft && event->type == InputTypeShort) {
        subghz_receiver->callback(SubGhzCustomEventViewReceiverConfig, subghz_receiver->context);
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        with_view_model(
            subghz_receiver->view,
            SubGhzViewReceiverModel * model,
            {
                if(model->history_item != 0) {
                    subghz_receiver->callback(
                        SubGhzCustomEventViewReceiverOK, subghz_receiver->context);
                }
            },
            false);
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
        subghz_receiver->view,
        SubGhzViewReceiverModel * model,
        {
            furi_string_reset(model->frequency_str);
            furi_string_reset(model->preset_str);
            furi_string_reset(model->history_stat_str);
                for
                    M_EACH(item_menu, model->history->data, SubGhzReceiverMenuItemArray_t) {
                        furi_string_free(item_menu->item_str);
                        item_menu->type = 0;
                    }
                SubGhzReceiverMenuItemArray_reset(model->history->data);
                model->idx = 0;
                model->list_offset = 0;
                model->history_item = 0;
        },
        false);
    furi_timer_stop(subghz_receiver->timer);
}

SubGhzViewReceiver* subghz_view_receiver_alloc(void) {
    SubGhzViewReceiver* subghz_receiver = malloc(sizeof(SubGhzViewReceiver));

    // View allocation and configuration
    subghz_receiver->view = view_alloc();

    subghz_receiver->lock = false;
    subghz_receiver->lock_count = 0;
    view_allocate_model(
        subghz_receiver->view, ViewModelTypeLocking, sizeof(SubGhzViewReceiverModel));
    view_set_context(subghz_receiver->view, subghz_receiver);
    view_set_draw_callback(subghz_receiver->view, (ViewDrawCallback)subghz_view_receiver_draw);
    view_set_input_callback(subghz_receiver->view, subghz_view_receiver_input);
    view_set_enter_callback(subghz_receiver->view, subghz_view_receiver_enter);
    view_set_exit_callback(subghz_receiver->view, subghz_view_receiver_exit);

    with_view_model(
        subghz_receiver->view,
        SubGhzViewReceiverModel * model,
        {
            model->frequency_str = furi_string_alloc();
            model->preset_str = furi_string_alloc();
            model->history_stat_str = furi_string_alloc();
            model->bar_show = SubGhzViewReceiverBarShowDefault;
            model->history = malloc(sizeof(SubGhzReceiverHistory));
            SubGhzReceiverMenuItemArray_init(model->history->data);
        },
        true);
    subghz_receiver->timer =
        furi_timer_alloc(subghz_view_receiver_timer_callback, FuriTimerTypeOnce, subghz_receiver);
    return subghz_receiver;
}

void subghz_view_receiver_free(SubGhzViewReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);

    with_view_model(
        subghz_receiver->view,
        SubGhzViewReceiverModel * model,
        {
            furi_string_free(model->frequency_str);
            furi_string_free(model->preset_str);
            furi_string_free(model->history_stat_str);
                for
                    M_EACH(item_menu, model->history->data, SubGhzReceiverMenuItemArray_t) {
                        furi_string_free(item_menu->item_str);
                        item_menu->type = 0;
                    }
                SubGhzReceiverMenuItemArray_clear(model->history->data);
                free(model->history);
        },
        false);
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
        subghz_receiver->view, SubGhzViewReceiverModel * model, { idx = model->idx; }, false);
    return idx;
}

void subghz_view_receiver_set_idx_menu(SubGhzViewReceiver* subghz_receiver, uint16_t idx) {
    furi_assert(subghz_receiver);
    with_view_model(
        subghz_receiver->view,
        SubGhzViewReceiverModel * model,
        {
            model->idx = idx;
            if(model->idx > 2) model->list_offset = idx - 2;
        },
        true);
    subghz_view_receiver_update_offset(subghz_receiver);
}
