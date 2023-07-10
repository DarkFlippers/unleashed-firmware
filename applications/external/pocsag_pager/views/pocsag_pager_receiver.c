#include "pocsag_pager_receiver.h"
#include "../pocsag_pager_app_i.h"
#include <pocsag_pager_icons.h>
#include <math.h>

#include <input/input.h>
#include <gui/elements.h>
#include <m-array.h>

#define FRAME_HEIGHT 12
#define MAX_LEN_PX 112
#define MENU_ITEMS 4u
#define UNLOCK_CNT 3

#define SUBGHZ_RAW_THRESHOLD_MIN -90.0f

typedef struct {
    FuriString* item_str;
    uint8_t type;
} PCSGReceiverMenuItem;

ARRAY_DEF(PCSGReceiverMenuItemArray, PCSGReceiverMenuItem, M_POD_OPLIST)

#define M_OPL_PCSGReceiverMenuItemArray_t() ARRAY_OPLIST(PCSGReceiverMenuItemArray, M_POD_OPLIST)

struct PCSGReceiverHistory {
    PCSGReceiverMenuItemArray_t data;
};

typedef struct PCSGReceiverHistory PCSGReceiverHistory;

static const Icon* ReceiverItemIcons[] = {
    [SubGhzProtocolTypeUnknown] = &I_Quest_7x8,
    [SubGhzProtocolTypeStatic] = &I_Message_8x7,
    [SubGhzProtocolTypeDynamic] = &I_Lock_7x8,
};

typedef enum {
    PCSGReceiverBarShowDefault,
    PCSGReceiverBarShowLock,
    PCSGReceiverBarShowToUnlockPress,
    PCSGReceiverBarShowUnlock,
} PCSGReceiverBarShow;

struct PCSGReceiver {
    PCSGLock lock;
    uint8_t lock_count;
    FuriTimer* timer;
    View* view;
    PCSGReceiverCallback callback;
    void* context;
};

typedef struct {
    FuriString* frequency_str;
    FuriString* preset_str;
    FuriString* history_stat_str;
    PCSGReceiverHistory* history;
    uint16_t idx;
    uint16_t list_offset;
    uint16_t history_item;
    PCSGReceiverBarShow bar_show;
    uint8_t u_rssi;
    bool ext_module;
} PCSGReceiverModel;

void pcsg_receiver_rssi(PCSGReceiver* instance, float rssi) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        PCSGReceiverModel * model,
        {
            if(rssi < SUBGHZ_RAW_THRESHOLD_MIN) {
                model->u_rssi = 0;
            } else {
                model->u_rssi = (uint8_t)(rssi - SUBGHZ_RAW_THRESHOLD_MIN);
            }
        },
        true);
}

void pcsg_view_receiver_set_lock(PCSGReceiver* pcsg_receiver, PCSGLock lock) {
    furi_assert(pcsg_receiver);
    pcsg_receiver->lock_count = 0;
    if(lock == PCSGLockOn) {
        pcsg_receiver->lock = lock;
        with_view_model(
            pcsg_receiver->view,
            PCSGReceiverModel * model,
            { model->bar_show = PCSGReceiverBarShowLock; },
            true);
        furi_timer_start(pcsg_receiver->timer, pdMS_TO_TICKS(1000));
    } else {
        with_view_model(
            pcsg_receiver->view,
            PCSGReceiverModel * model,
            { model->bar_show = PCSGReceiverBarShowDefault; },
            true);
    }
}

void pcsg_view_receiver_set_ext_module_state(PCSGReceiver* pcsg_receiver, bool is_external) {
    furi_assert(pcsg_receiver);
    with_view_model(
        pcsg_receiver->view, PCSGReceiverModel * model, { model->ext_module = is_external; }, true);
}

void pcsg_view_receiver_set_callback(
    PCSGReceiver* pcsg_receiver,
    PCSGReceiverCallback callback,
    void* context) {
    furi_assert(pcsg_receiver);
    furi_assert(callback);
    pcsg_receiver->callback = callback;
    pcsg_receiver->context = context;
}

static void pcsg_view_receiver_update_offset(PCSGReceiver* pcsg_receiver) {
    furi_assert(pcsg_receiver);

    with_view_model(
        pcsg_receiver->view,
        PCSGReceiverModel * model,
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

void pcsg_view_receiver_add_item_to_menu(
    PCSGReceiver* pcsg_receiver,
    const char* name,
    uint8_t type) {
    furi_assert(pcsg_receiver);
    with_view_model(
        pcsg_receiver->view,
        PCSGReceiverModel * model,
        {
            PCSGReceiverMenuItem* item_menu =
                PCSGReceiverMenuItemArray_push_raw(model->history->data);
            item_menu->item_str = furi_string_alloc_set(name);
            item_menu->type = type;
            if((model->idx == model->history_item - 1)) {
                model->history_item++;
                model->idx++;
            } else {
                model->history_item++;
            }
        },
        true);
    pcsg_view_receiver_update_offset(pcsg_receiver);
}

void pcsg_view_receiver_add_data_statusbar(
    PCSGReceiver* pcsg_receiver,
    const char* frequency_str,
    const char* preset_str,
    const char* history_stat_str) {
    furi_assert(pcsg_receiver);
    with_view_model(
        pcsg_receiver->view,
        PCSGReceiverModel * model,
        {
            furi_string_set_str(model->frequency_str, frequency_str);
            furi_string_set_str(model->preset_str, preset_str);
            furi_string_set_str(model->history_stat_str, history_stat_str);
        },
        true);
}

static void pcsg_view_receiver_draw_frame(Canvas* canvas, uint16_t idx, bool scrollbar) {
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

static void pcsg_view_rssi_draw(Canvas* canvas, PCSGReceiverModel* model) {
    for(uint8_t i = 1; i < model->u_rssi; i++) {
        if(i % 5) {
            canvas_draw_dot(canvas, 46 + i, 50);
            canvas_draw_dot(canvas, 47 + i, 51);
            canvas_draw_dot(canvas, 46 + i, 52);
        }
    }
}

void pcsg_view_receiver_draw(Canvas* canvas, PCSGReceiverModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    elements_button_left(canvas, "Config");
    //canvas_draw_line(canvas, 46, 51, 125, 51);

    bool scrollbar = model->history_item > 4;
    FuriString* str_buff;
    str_buff = furi_string_alloc();

    PCSGReceiverMenuItem* item_menu;

    for(size_t i = 0; i < MIN(model->history_item, MENU_ITEMS); ++i) {
        size_t idx = CLAMP((uint16_t)(i + model->list_offset), model->history_item, 0);
        item_menu = PCSGReceiverMenuItemArray_get(model->history->data, idx);
        furi_string_set(str_buff, item_menu->item_str);
        furi_string_replace_all(str_buff, "#", "");
        elements_string_fit_width(canvas, str_buff, scrollbar ? MAX_LEN_PX - 7 : MAX_LEN_PX);
        if(model->idx == idx) {
            pcsg_view_receiver_draw_frame(canvas, i, scrollbar);
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
        canvas_draw_icon(canvas, 0, 0, model->ext_module ? &I_Fishing_123x52 : &I_Scanning_123x52);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 63, 46, "Scanning...");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 44, 10, model->ext_module ? "Ext" : "Int");
    }

    // Draw RSSI
    pcsg_view_rssi_draw(canvas, model);

    switch(model->bar_show) {
    case PCSGReceiverBarShowLock:
        canvas_draw_icon(canvas, 64, 55, &I_Lock_7x8);
        canvas_draw_str(canvas, 74, 62, "Locked");
        break;
    case PCSGReceiverBarShowToUnlockPress:
        canvas_draw_str(canvas, 44, 62, furi_string_get_cstr(model->frequency_str));
        canvas_draw_str(canvas, 79, 62, furi_string_get_cstr(model->preset_str));
        canvas_draw_str(canvas, 96, 62, furi_string_get_cstr(model->history_stat_str));
        canvas_set_font(canvas, FontSecondary);
        elements_bold_rounded_frame(canvas, 14, 8, 99, 48);
        elements_multiline_text(canvas, 65, 26, "To unlock\npress:");
        canvas_draw_icon(canvas, 65, 42, &I_Pin_back_arrow_10x8);
        canvas_draw_icon(canvas, 80, 42, &I_Pin_back_arrow_10x8);
        canvas_draw_icon(canvas, 95, 42, &I_Pin_back_arrow_10x8);
        canvas_draw_icon(canvas, 16, 13, &I_WarningDolphin_45x42);
        canvas_draw_dot(canvas, 17, 61);
        break;
    case PCSGReceiverBarShowUnlock:
        canvas_draw_icon(canvas, 64, 55, &I_Unlock_7x8);
        canvas_draw_str(canvas, 74, 62, "Unlocked");
        break;
    default:
        canvas_draw_str(canvas, 44, 62, furi_string_get_cstr(model->frequency_str));
        canvas_draw_str(canvas, 79, 62, furi_string_get_cstr(model->preset_str));
        canvas_draw_str(canvas, 96, 62, furi_string_get_cstr(model->history_stat_str));
        break;
    }
}

static void pcsg_view_receiver_timer_callback(void* context) {
    furi_assert(context);
    PCSGReceiver* pcsg_receiver = context;
    with_view_model(
        pcsg_receiver->view,
        PCSGReceiverModel * model,
        { model->bar_show = PCSGReceiverBarShowDefault; },
        true);
    if(pcsg_receiver->lock_count < UNLOCK_CNT) {
        pcsg_receiver->callback(PCSGCustomEventViewReceiverOffDisplay, pcsg_receiver->context);
    } else {
        pcsg_receiver->lock = PCSGLockOff;
        pcsg_receiver->callback(PCSGCustomEventViewReceiverUnlock, pcsg_receiver->context);
    }
    pcsg_receiver->lock_count = 0;
}

bool pcsg_view_receiver_input(InputEvent* event, void* context) {
    furi_assert(context);
    PCSGReceiver* pcsg_receiver = context;

    if(pcsg_receiver->lock == PCSGLockOn) {
        with_view_model(
            pcsg_receiver->view,
            PCSGReceiverModel * model,
            { model->bar_show = PCSGReceiverBarShowToUnlockPress; },
            true);
        if(pcsg_receiver->lock_count == 0) {
            furi_timer_start(pcsg_receiver->timer, pdMS_TO_TICKS(1000));
        }
        if(event->key == InputKeyBack && event->type == InputTypeShort) {
            pcsg_receiver->lock_count++;
        }
        if(pcsg_receiver->lock_count >= UNLOCK_CNT) {
            pcsg_receiver->callback(PCSGCustomEventViewReceiverUnlock, pcsg_receiver->context);
            with_view_model(
                pcsg_receiver->view,
                PCSGReceiverModel * model,
                { model->bar_show = PCSGReceiverBarShowUnlock; },
                true);
            pcsg_receiver->lock = PCSGLockOff;
            furi_timer_start(pcsg_receiver->timer, pdMS_TO_TICKS(650));
        }

        return true;
    }

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        pcsg_receiver->callback(PCSGCustomEventViewReceiverBack, pcsg_receiver->context);
    } else if(
        event->key == InputKeyUp &&
        (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
        with_view_model(
            pcsg_receiver->view,
            PCSGReceiverModel * model,
            {
                if(model->idx != 0) model->idx--;
            },
            true);
    } else if(
        event->key == InputKeyDown &&
        (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
        with_view_model(
            pcsg_receiver->view,
            PCSGReceiverModel * model,
            {
                if(model->history_item && model->idx != model->history_item - 1) model->idx++;
            },
            true);
    } else if(event->key == InputKeyLeft && event->type == InputTypeShort) {
        pcsg_receiver->callback(PCSGCustomEventViewReceiverConfig, pcsg_receiver->context);
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        with_view_model(
            pcsg_receiver->view,
            PCSGReceiverModel * model,
            {
                if(model->history_item != 0) {
                    pcsg_receiver->callback(PCSGCustomEventViewReceiverOK, pcsg_receiver->context);
                }
            },
            false);
    }

    pcsg_view_receiver_update_offset(pcsg_receiver);

    return true;
}

void pcsg_view_receiver_enter(void* context) {
    furi_assert(context);
}

void pcsg_view_receiver_exit(void* context) {
    furi_assert(context);
    PCSGReceiver* pcsg_receiver = context;
    with_view_model(
        pcsg_receiver->view,
        PCSGReceiverModel * model,
        {
            furi_string_reset(model->frequency_str);
            furi_string_reset(model->preset_str);
            furi_string_reset(model->history_stat_str);
                for
                    M_EACH(item_menu, model->history->data, PCSGReceiverMenuItemArray_t) {
                        furi_string_free(item_menu->item_str);
                        item_menu->type = 0;
                    }
                PCSGReceiverMenuItemArray_reset(model->history->data);
                model->idx = 0;
                model->list_offset = 0;
                model->history_item = 0;
        },
        false);
    furi_timer_stop(pcsg_receiver->timer);
}

PCSGReceiver* pcsg_view_receiver_alloc() {
    PCSGReceiver* pcsg_receiver = malloc(sizeof(PCSGReceiver));

    // View allocation and configuration
    pcsg_receiver->view = view_alloc();

    pcsg_receiver->lock = PCSGLockOff;
    pcsg_receiver->lock_count = 0;
    view_allocate_model(pcsg_receiver->view, ViewModelTypeLocking, sizeof(PCSGReceiverModel));
    view_set_context(pcsg_receiver->view, pcsg_receiver);
    view_set_draw_callback(pcsg_receiver->view, (ViewDrawCallback)pcsg_view_receiver_draw);
    view_set_input_callback(pcsg_receiver->view, pcsg_view_receiver_input);
    view_set_enter_callback(pcsg_receiver->view, pcsg_view_receiver_enter);
    view_set_exit_callback(pcsg_receiver->view, pcsg_view_receiver_exit);

    with_view_model(
        pcsg_receiver->view,
        PCSGReceiverModel * model,
        {
            model->frequency_str = furi_string_alloc();
            model->preset_str = furi_string_alloc();
            model->history_stat_str = furi_string_alloc();
            model->bar_show = PCSGReceiverBarShowDefault;
            model->history = malloc(sizeof(PCSGReceiverHistory));
            PCSGReceiverMenuItemArray_init(model->history->data);
        },
        true);
    pcsg_receiver->timer =
        furi_timer_alloc(pcsg_view_receiver_timer_callback, FuriTimerTypeOnce, pcsg_receiver);
    return pcsg_receiver;
}

void pcsg_view_receiver_free(PCSGReceiver* pcsg_receiver) {
    furi_assert(pcsg_receiver);

    with_view_model(
        pcsg_receiver->view,
        PCSGReceiverModel * model,
        {
            furi_string_free(model->frequency_str);
            furi_string_free(model->preset_str);
            furi_string_free(model->history_stat_str);
                for
                    M_EACH(item_menu, model->history->data, PCSGReceiverMenuItemArray_t) {
                        furi_string_free(item_menu->item_str);
                        item_menu->type = 0;
                    }
                PCSGReceiverMenuItemArray_clear(model->history->data);
                free(model->history);
        },
        false);
    furi_timer_free(pcsg_receiver->timer);
    view_free(pcsg_receiver->view);
    free(pcsg_receiver);
}

View* pcsg_view_receiver_get_view(PCSGReceiver* pcsg_receiver) {
    furi_assert(pcsg_receiver);
    return pcsg_receiver->view;
}

uint16_t pcsg_view_receiver_get_idx_menu(PCSGReceiver* pcsg_receiver) {
    furi_assert(pcsg_receiver);
    uint32_t idx = 0;
    with_view_model(
        pcsg_receiver->view, PCSGReceiverModel * model, { idx = model->idx; }, false);
    return idx;
}

void pcsg_view_receiver_set_idx_menu(PCSGReceiver* pcsg_receiver, uint16_t idx) {
    furi_assert(pcsg_receiver);
    with_view_model(
        pcsg_receiver->view,
        PCSGReceiverModel * model,
        {
            model->idx = idx;
            if(model->idx > 2) model->list_offset = idx - 2;
        },
        true);
    pcsg_view_receiver_update_offset(pcsg_receiver);
}
