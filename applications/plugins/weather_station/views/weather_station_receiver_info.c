#include "weather_station_receiver.h"
#include "../weather_station_app_i.h"
#include "weather_station_icons.h"
#include "../protocols/ws_generic.h"
#include <input/input.h>
#include <gui/elements.h>
#include <float_tools.h>

struct WSReceiverInfo {
    View* view;
    FuriTimer* timer;
};

typedef struct {
    uint32_t curr_ts;
    FuriString* protocol_name;
    WSBlockGeneric* generic;
} WSReceiverInfoModel;

void ws_view_receiver_info_update(WSReceiverInfo* ws_receiver_info, FlipperFormat* fff) {
    furi_assert(ws_receiver_info);
    furi_assert(fff);

    with_view_model(
        ws_receiver_info->view,
        WSReceiverInfoModel * model,
        {
            flipper_format_rewind(fff);
            flipper_format_read_string(fff, "Protocol", model->protocol_name);

            ws_block_generic_deserialize(model->generic, fff);

            FuriHalRtcDateTime curr_dt;
            furi_hal_rtc_get_datetime(&curr_dt);
            model->curr_ts = furi_hal_rtc_datetime_to_timestamp(&curr_dt);
        },
        true);
}

void ws_view_receiver_info_draw(Canvas* canvas, WSReceiverInfoModel* model) {
    char buffer[64];
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    snprintf(
        buffer,
        sizeof(buffer),
        "%s %db",
        furi_string_get_cstr(model->protocol_name),
        model->generic->data_count_bit);
    canvas_draw_str(canvas, 0, 8, buffer);

    if(model->generic->channel != WS_NO_CHANNEL) {
        snprintf(buffer, sizeof(buffer), "Ch: %01d", model->generic->channel);
        canvas_draw_str(canvas, 106, 8, buffer);
    }

    if(model->generic->id != WS_NO_ID) {
        snprintf(buffer, sizeof(buffer), "Sn: 0x%02lX", model->generic->id);
        canvas_draw_str(canvas, 0, 20, buffer);
    }

    if(model->generic->btn != WS_NO_BTN) {
        snprintf(buffer, sizeof(buffer), "Btn: %01d", model->generic->btn);
        canvas_draw_str(canvas, 57, 20, buffer);
    }

    if(model->generic->battery_low != WS_NO_BATT) {
        snprintf(
            buffer, sizeof(buffer), "Batt: %s", (!model->generic->battery_low ? "ok" : "low"));
        canvas_draw_str_aligned(canvas, 126, 17, AlignRight, AlignCenter, buffer);
    }

    snprintf(buffer, sizeof(buffer), "Data: 0x%llX", model->generic->data);
    canvas_draw_str(canvas, 0, 32, buffer);

    elements_bold_rounded_frame(canvas, 0, 38, 127, 25);
    canvas_set_font(canvas, FontPrimary);

    if(!float_is_equal(model->generic->temp, WS_NO_TEMPERATURE)) {
        canvas_draw_icon(canvas, 6, 43, &I_Therm_7x16);

        uint8_t temp_x1 = 0;
        uint8_t temp_x2 = 0;
        if(furi_hal_rtc_get_locale_units() == FuriHalRtcLocaleUnitsMetric) {
            snprintf(buffer, sizeof(buffer), "%3.1f C", (double)model->generic->temp);
            if(model->generic->temp < -9.0f) {
                temp_x1 = 49;
                temp_x2 = 40;
            } else {
                temp_x1 = 47;
                temp_x2 = 38;
            }
        } else {
            snprintf(
                buffer,
                sizeof(buffer),
                "%3.1f F",
                (double)locale_celsius_to_fahrenheit(model->generic->temp));
            if((model->generic->temp < -27.77f) || (model->generic->temp > 37.77f)) {
                temp_x1 = 50;
                temp_x2 = 42;
            } else {
                temp_x1 = 48;
                temp_x2 = 40;
            }
        }

        canvas_draw_str_aligned(canvas, temp_x1, 47, AlignRight, AlignTop, buffer);
        canvas_draw_circle(canvas, temp_x2, 46, 1);
    }

    if(model->generic->humidity != WS_NO_HUMIDITY) {
        canvas_draw_icon(canvas, 53, 44, &I_Humid_8x13);
        snprintf(buffer, sizeof(buffer), "%d%%", model->generic->humidity);
        canvas_draw_str(canvas, 64, 55, buffer);
    }

    if((int)model->generic->timestamp > 0 && model->curr_ts) {
        int ts_diff = (int)model->curr_ts - (int)model->generic->timestamp;

        canvas_draw_icon(canvas, 91, 46, &I_Timer_11x11);

        if(ts_diff > 60) {
            int tmp_sec = ts_diff;
            int cnt_min = 1;
            for(int i = 1; tmp_sec > 60; i++) {
                tmp_sec = tmp_sec - 60;
                cnt_min = i;
            }

            if(model->curr_ts % 2 == 0) {
                canvas_draw_str_aligned(canvas, 105, 51, AlignLeft, AlignCenter, "Old");
            } else {
                if(cnt_min >= 59) {
                    canvas_draw_str_aligned(canvas, 105, 51, AlignLeft, AlignCenter, "Old");
                } else {
                    snprintf(buffer, sizeof(buffer), "%dm", cnt_min);
                    canvas_draw_str_aligned(canvas, 114, 51, AlignCenter, AlignCenter, buffer);
                }
            }

        } else {
            snprintf(buffer, sizeof(buffer), "%d", ts_diff);
            canvas_draw_str_aligned(canvas, 112, 51, AlignCenter, AlignCenter, buffer);
        }
    }
}

bool ws_view_receiver_info_input(InputEvent* event, void* context) {
    furi_assert(context);
    //WSReceiverInfo* ws_receiver_info = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    return true;
}

static void ws_view_receiver_info_enter(void* context) {
    furi_assert(context);
    WSReceiverInfo* ws_receiver_info = context;

    furi_timer_start(ws_receiver_info->timer, 1000);
}

static void ws_view_receiver_info_exit(void* context) {
    furi_assert(context);
    WSReceiverInfo* ws_receiver_info = context;

    furi_timer_stop(ws_receiver_info->timer);

    with_view_model(
        ws_receiver_info->view,
        WSReceiverInfoModel * model,
        { furi_string_reset(model->protocol_name); },
        false);
}

static void ws_view_receiver_info_timer(void* context) {
    WSReceiverInfo* ws_receiver_info = context;
    // Force redraw
    with_view_model(
        ws_receiver_info->view,
        WSReceiverInfoModel * model,
        {
            FuriHalRtcDateTime curr_dt;
            furi_hal_rtc_get_datetime(&curr_dt);
            model->curr_ts = furi_hal_rtc_datetime_to_timestamp(&curr_dt);
        },
        true);
}

WSReceiverInfo* ws_view_receiver_info_alloc() {
    WSReceiverInfo* ws_receiver_info = malloc(sizeof(WSReceiverInfo));

    // View allocation and configuration
    ws_receiver_info->view = view_alloc();

    view_allocate_model(ws_receiver_info->view, ViewModelTypeLocking, sizeof(WSReceiverInfoModel));
    view_set_context(ws_receiver_info->view, ws_receiver_info);
    view_set_draw_callback(ws_receiver_info->view, (ViewDrawCallback)ws_view_receiver_info_draw);
    view_set_input_callback(ws_receiver_info->view, ws_view_receiver_info_input);
    view_set_enter_callback(ws_receiver_info->view, ws_view_receiver_info_enter);
    view_set_exit_callback(ws_receiver_info->view, ws_view_receiver_info_exit);

    with_view_model(
        ws_receiver_info->view,
        WSReceiverInfoModel * model,
        {
            model->generic = malloc(sizeof(WSBlockGeneric));
            model->protocol_name = furi_string_alloc();
        },
        true);

    ws_receiver_info->timer =
        furi_timer_alloc(ws_view_receiver_info_timer, FuriTimerTypePeriodic, ws_receiver_info);

    return ws_receiver_info;
}

void ws_view_receiver_info_free(WSReceiverInfo* ws_receiver_info) {
    furi_assert(ws_receiver_info);

    furi_timer_free(ws_receiver_info->timer);

    with_view_model(
        ws_receiver_info->view,
        WSReceiverInfoModel * model,
        {
            furi_string_free(model->protocol_name);
            free(model->generic);
        },
        false);

    view_free(ws_receiver_info->view);
    free(ws_receiver_info);
}

View* ws_view_receiver_info_get_view(WSReceiverInfo* ws_receiver_info) {
    furi_assert(ws_receiver_info);
    return ws_receiver_info->view;
}
