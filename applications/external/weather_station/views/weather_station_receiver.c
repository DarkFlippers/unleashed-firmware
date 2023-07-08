#include "weather_station_receiver.h"
#include "../weather_station_app_i.h"
#include <weather_station_icons.h>
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
} WSReceiverMenuItem;

ARRAY_DEF(WSReceiverMenuItemArray, WSReceiverMenuItem, M_POD_OPLIST)

#define M_OPL_WSReceiverMenuItemArray_t() ARRAY_OPLIST(WSReceiverMenuItemArray, M_POD_OPLIST)

struct WSReceiverHistory {
    WSReceiverMenuItemArray_t data;
};

typedef struct WSReceiverHistory WSReceiverHistory;

static const Icon* ReceiverItemIcons[] = {
    [SubGhzProtocolTypeUnknown] = &I_Quest_7x8,
    [SubGhzProtocolTypeStatic] = &I_Unlock_7x8,
    [SubGhzProtocolTypeDynamic] = &I_Lock_7x8,
    [SubGhzProtocolWeatherStation] = &I_station_icon,
};

typedef enum {
    WSReceiverBarShowDefault,
    WSReceiverBarShowLock,
    WSReceiverBarShowToUnlockPress,
    WSReceiverBarShowUnlock,
} WSReceiverBarShow;

struct WSReceiver {
    WSLock lock;
    uint8_t lock_count;
    FuriTimer* timer;
    View* view;
    WSReceiverCallback callback;
    void* context;
};

typedef struct {
    FuriString* frequency_str;
    FuriString* preset_str;
    FuriString* history_stat_str;
    WSReceiverHistory* history;
    uint16_t idx;
    uint16_t list_offset;
    uint16_t history_item;
    WSReceiverBarShow bar_show;
    uint8_t u_rssi;
    bool external_redio;
} WSReceiverModel;

void ws_view_receiver_set_rssi(WSReceiver* instance, float rssi) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        WSReceiverModel * model,
        {
            if(rssi < SUBGHZ_RAW_THRESHOLD_MIN) {
                model->u_rssi = 0;
            } else {
                model->u_rssi = (uint8_t)(rssi - SUBGHZ_RAW_THRESHOLD_MIN);
            }
        },
        true);
}

void ws_view_receiver_set_lock(WSReceiver* ws_receiver, WSLock lock) {
    furi_assert(ws_receiver);
    ws_receiver->lock_count = 0;
    if(lock == WSLockOn) {
        ws_receiver->lock = lock;
        with_view_model(
            ws_receiver->view,
            WSReceiverModel * model,
            { model->bar_show = WSReceiverBarShowLock; },
            true);
        furi_timer_start(ws_receiver->timer, pdMS_TO_TICKS(1000));
    } else {
        with_view_model(
            ws_receiver->view,
            WSReceiverModel * model,
            { model->bar_show = WSReceiverBarShowDefault; },
            true);
    }
}

void ws_view_receiver_set_callback(
    WSReceiver* ws_receiver,
    WSReceiverCallback callback,
    void* context) {
    furi_assert(ws_receiver);
    furi_assert(callback);
    ws_receiver->callback = callback;
    ws_receiver->context = context;
}

static void ws_view_receiver_update_offset(WSReceiver* ws_receiver) {
    furi_assert(ws_receiver);

    with_view_model(
        ws_receiver->view,
        WSReceiverModel * model,
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

void ws_view_receiver_add_item_to_menu(WSReceiver* ws_receiver, const char* name, uint8_t type) {
    furi_assert(ws_receiver);
    with_view_model(
        ws_receiver->view,
        WSReceiverModel * model,
        {
            WSReceiverMenuItem* item_menu = WSReceiverMenuItemArray_push_raw(model->history->data);
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
    ws_view_receiver_update_offset(ws_receiver);
}

void ws_view_receiver_add_data_statusbar(
    WSReceiver* ws_receiver,
    const char* frequency_str,
    const char* preset_str,
    const char* history_stat_str,
    bool external) {
    furi_assert(ws_receiver);
    with_view_model(
        ws_receiver->view,
        WSReceiverModel * model,
        {
            furi_string_set_str(model->frequency_str, frequency_str);
            furi_string_set_str(model->preset_str, preset_str);
            furi_string_set_str(model->history_stat_str, history_stat_str);
            model->external_redio = external;
        },
        true);
}

static void ws_view_receiver_draw_frame(Canvas* canvas, uint16_t idx, bool scrollbar) {
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

static void ws_view_rssi_draw(Canvas* canvas, WSReceiverModel* model) {
    for(uint8_t i = 1; i < model->u_rssi; i++) {
        if(i % 5) {
            canvas_draw_dot(canvas, 46 + i, 50);
            canvas_draw_dot(canvas, 47 + i, 51);
            canvas_draw_dot(canvas, 46 + i, 52);
        }
    }
}

void ws_view_receiver_draw(Canvas* canvas, WSReceiverModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    elements_button_left(canvas, "Config");

    bool scrollbar = model->history_item > 4;
    FuriString* str_buff;
    str_buff = furi_string_alloc();

    // bool ext_module = furi_hal_subghz_get_radio_type();

    WSReceiverMenuItem* item_menu;

    for(size_t i = 0; i < MIN(model->history_item, MENU_ITEMS); ++i) {
        size_t idx = CLAMP((uint16_t)(i + model->list_offset), model->history_item, 0);
        item_menu = WSReceiverMenuItemArray_get(model->history->data, idx);
        furi_string_set(str_buff, item_menu->item_str);
        elements_string_fit_width(canvas, str_buff, scrollbar ? MAX_LEN_PX - 6 : MAX_LEN_PX);
        if(model->idx == idx) {
            ws_view_receiver_draw_frame(canvas, i, scrollbar);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }
        canvas_draw_icon(canvas, 4, 2 + i * FRAME_HEIGHT, ReceiverItemIcons[item_menu->type]);
        canvas_draw_str(canvas, 14, 9 + i * FRAME_HEIGHT, furi_string_get_cstr(str_buff));
        furi_string_reset(str_buff);
    }
    if(scrollbar) {
        elements_scrollbar_pos(canvas, 128, 0, 49, model->idx, model->history_item);
    }
    furi_string_free(str_buff);

    canvas_set_color(canvas, ColorBlack);

    if(model->history_item == 0) {
        canvas_draw_icon(
            canvas, 0, 0, model->external_redio ? &I_Fishing_123x52 : &I_Scanning_123x52);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 63, 46, "Scanning...");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 44, 10, model->external_redio ? "Ext" : "Int");
    }

    // Draw RSSI
    ws_view_rssi_draw(canvas, model);

    switch(model->bar_show) {
    case WSReceiverBarShowLock:
        canvas_draw_icon(canvas, 64, 55, &I_Lock_7x8);
        canvas_draw_str(canvas, 74, 62, "Locked");
        break;
    case WSReceiverBarShowToUnlockPress:
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
    case WSReceiverBarShowUnlock:
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

static void ws_view_receiver_timer_callback(void* context) {
    furi_assert(context);
    WSReceiver* ws_receiver = context;
    with_view_model(
        ws_receiver->view,
        WSReceiverModel * model,
        { model->bar_show = WSReceiverBarShowDefault; },
        true);
    if(ws_receiver->lock_count < UNLOCK_CNT) {
        ws_receiver->callback(WSCustomEventViewReceiverOffDisplay, ws_receiver->context);
    } else {
        ws_receiver->lock = WSLockOff;
        ws_receiver->callback(WSCustomEventViewReceiverUnlock, ws_receiver->context);
    }
    ws_receiver->lock_count = 0;
}

bool ws_view_receiver_input(InputEvent* event, void* context) {
    furi_assert(context);
    WSReceiver* ws_receiver = context;

    if(ws_receiver->lock == WSLockOn) {
        with_view_model(
            ws_receiver->view,
            WSReceiverModel * model,
            { model->bar_show = WSReceiverBarShowToUnlockPress; },
            true);
        if(ws_receiver->lock_count == 0) {
            furi_timer_start(ws_receiver->timer, pdMS_TO_TICKS(1000));
        }
        if(event->key == InputKeyBack && event->type == InputTypeShort) {
            ws_receiver->lock_count++;
        }
        if(ws_receiver->lock_count >= UNLOCK_CNT) {
            ws_receiver->callback(WSCustomEventViewReceiverUnlock, ws_receiver->context);
            with_view_model(
                ws_receiver->view,
                WSReceiverModel * model,
                { model->bar_show = WSReceiverBarShowUnlock; },
                true);
            ws_receiver->lock = WSLockOff;
            furi_timer_start(ws_receiver->timer, pdMS_TO_TICKS(650));
        }

        return true;
    }

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        ws_receiver->callback(WSCustomEventViewReceiverBack, ws_receiver->context);
    } else if(
        event->key == InputKeyUp &&
        (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
        with_view_model(
            ws_receiver->view,
            WSReceiverModel * model,
            {
                if(model->idx != 0) model->idx--;
            },
            true);
    } else if(
        event->key == InputKeyDown &&
        (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
        with_view_model(
            ws_receiver->view,
            WSReceiverModel * model,
            {
                if(model->history_item && model->idx != model->history_item - 1) model->idx++;
            },
            true);
    } else if(event->key == InputKeyLeft && event->type == InputTypeShort) {
        ws_receiver->callback(WSCustomEventViewReceiverConfig, ws_receiver->context);
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        with_view_model(
            ws_receiver->view,
            WSReceiverModel * model,
            {
                if(model->history_item != 0) {
                    ws_receiver->callback(WSCustomEventViewReceiverOK, ws_receiver->context);
                }
            },
            false);
    }

    ws_view_receiver_update_offset(ws_receiver);

    return true;
}

void ws_view_receiver_enter(void* context) {
    furi_assert(context);
}

void ws_view_receiver_exit(void* context) {
    furi_assert(context);
    WSReceiver* ws_receiver = context;
    with_view_model(
        ws_receiver->view,
        WSReceiverModel * model,
        {
            furi_string_reset(model->frequency_str);
            furi_string_reset(model->preset_str);
            furi_string_reset(model->history_stat_str);
                for
                    M_EACH(item_menu, model->history->data, WSReceiverMenuItemArray_t) {
                        furi_string_free(item_menu->item_str);
                        item_menu->type = 0;
                    }
                WSReceiverMenuItemArray_reset(model->history->data);
                model->idx = 0;
                model->list_offset = 0;
                model->history_item = 0;
        },
        false);
    furi_timer_stop(ws_receiver->timer);
}

WSReceiver* ws_view_receiver_alloc() {
    WSReceiver* ws_receiver = malloc(sizeof(WSReceiver));

    // View allocation and configuration
    ws_receiver->view = view_alloc();

    ws_receiver->lock = WSLockOff;
    ws_receiver->lock_count = 0;
    view_allocate_model(ws_receiver->view, ViewModelTypeLocking, sizeof(WSReceiverModel));
    view_set_context(ws_receiver->view, ws_receiver);
    view_set_draw_callback(ws_receiver->view, (ViewDrawCallback)ws_view_receiver_draw);
    view_set_input_callback(ws_receiver->view, ws_view_receiver_input);
    view_set_enter_callback(ws_receiver->view, ws_view_receiver_enter);
    view_set_exit_callback(ws_receiver->view, ws_view_receiver_exit);

    with_view_model(
        ws_receiver->view,
        WSReceiverModel * model,
        {
            model->frequency_str = furi_string_alloc();
            model->preset_str = furi_string_alloc();
            model->history_stat_str = furi_string_alloc();
            model->bar_show = WSReceiverBarShowDefault;
            model->history = malloc(sizeof(WSReceiverHistory));
            model->external_redio = false;
            WSReceiverMenuItemArray_init(model->history->data);
        },
        true);
    ws_receiver->timer =
        furi_timer_alloc(ws_view_receiver_timer_callback, FuriTimerTypeOnce, ws_receiver);
    return ws_receiver;
}

void ws_view_receiver_free(WSReceiver* ws_receiver) {
    furi_assert(ws_receiver);

    with_view_model(
        ws_receiver->view,
        WSReceiverModel * model,
        {
            furi_string_free(model->frequency_str);
            furi_string_free(model->preset_str);
            furi_string_free(model->history_stat_str);
                for
                    M_EACH(item_menu, model->history->data, WSReceiverMenuItemArray_t) {
                        furi_string_free(item_menu->item_str);
                        item_menu->type = 0;
                    }
                WSReceiverMenuItemArray_clear(model->history->data);
                free(model->history);
        },
        false);
    furi_timer_free(ws_receiver->timer);
    view_free(ws_receiver->view);
    free(ws_receiver);
}

View* ws_view_receiver_get_view(WSReceiver* ws_receiver) {
    furi_assert(ws_receiver);
    return ws_receiver->view;
}

uint16_t ws_view_receiver_get_idx_menu(WSReceiver* ws_receiver) {
    furi_assert(ws_receiver);
    uint32_t idx = 0;
    with_view_model(
        ws_receiver->view, WSReceiverModel * model, { idx = model->idx; }, false);
    return idx;
}

void ws_view_receiver_set_idx_menu(WSReceiver* ws_receiver, uint16_t idx) {
    furi_assert(ws_receiver);
    with_view_model(
        ws_receiver->view,
        WSReceiverModel * model,
        {
            model->idx = idx;
            if(model->idx > 2) model->list_offset = idx - 2;
        },
        true);
    ws_view_receiver_update_offset(ws_receiver);
}
