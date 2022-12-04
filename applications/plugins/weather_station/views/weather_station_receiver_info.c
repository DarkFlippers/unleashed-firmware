#include "weather_station_receiver.h"
#include "../weather_station_app_i.h"
#include "weather_station_icons.h"
#include "../protocols/ws_generic.h"
#include <input/input.h>
#include <gui/elements.h>

#define abs(x) ((x) > 0 ? (x) : -(x))

struct WSReceiverInfo {
    View* view;
};

typedef struct {
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
    canvas_draw_str(canvas, 5, 8, buffer);

    if(model->generic->channel != WS_NO_CHANNEL) {
        snprintf(buffer, sizeof(buffer), "Ch: %01d", model->generic->channel);
        canvas_draw_str(canvas, 105, 8, buffer);
    }

    if(model->generic->id != WS_NO_ID) {
        snprintf(buffer, sizeof(buffer), "Sn: 0x%02lX", model->generic->id);
        canvas_draw_str(canvas, 5, 20, buffer);
    }

    if(model->generic->btn != WS_NO_BTN) {
        snprintf(buffer, sizeof(buffer), "Btn: %01d", model->generic->btn);
        canvas_draw_str(canvas, 57, 20, buffer);
    }

    if(model->generic->battery_low != WS_NO_BATT) {
        snprintf(
            buffer, sizeof(buffer), "Batt: %s", (!model->generic->battery_low ? "ok" : "low"));
        canvas_draw_str(canvas, 90, 20, buffer);
    }

    snprintf(buffer, sizeof(buffer), "Data: 0x%llX", model->generic->data);
    canvas_draw_str(canvas, 5, 32, buffer);

    //DATA AGE
    if((int)model->generic->agedata > 0) {
        FuriHalRtcDateTime curr_dt;
        furi_hal_rtc_get_datetime(&curr_dt);
        uint32_t curr_ts = furi_hal_rtc_datetime_to_timestamp(&curr_dt);

        int diffold = (int)curr_ts - (int)model->generic->agedata;

        uint8_t tmp_x_frame = 101;
        uint8_t tmp_y_frame = 23;

        uint8_t tmp_x_text = 113;
        uint8_t tmp_y_text = 29;
        if(model->generic->data_count_bit >= 41) {
            tmp_x_frame = 78;
            tmp_y_frame = 0;

            tmp_x_text = 91;
            tmp_y_text = 6;

            if(model->generic->btn == WS_NO_BTN) {
                if(furi_string_size(model->protocol_name) > (size_t)10) {
                    tmp_x_frame = 61;
                    tmp_y_frame = 11;

                    tmp_x_text = 74;
                    tmp_y_text = 17;
                }
            }
        }

        if(diffold > 60) {
            int tmp_sec = diffold;
            int cnt_min = 1;
            for(int i = 1; tmp_sec > 60; i++) {
                tmp_sec = tmp_sec - 60;
                cnt_min = i;
            }

            if(curr_ts % 2 == 0) {
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_rframe(canvas, tmp_x_frame, tmp_y_frame, 26, 11, 1);
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_str_aligned(
                    canvas, tmp_x_text, tmp_y_text, AlignCenter, AlignCenter, "OLD");
            } else {
                if(cnt_min >= 59) {
                    canvas_set_color(canvas, ColorBlack);
                    canvas_draw_rframe(canvas, tmp_x_frame, tmp_y_frame, 26, 11, 1);
                    canvas_set_color(canvas, ColorBlack);
                    canvas_draw_str_aligned(
                        canvas, tmp_x_text, tmp_y_text, AlignCenter, AlignCenter, "OLD");
                } else {
                    canvas_set_color(canvas, ColorBlack);
                    canvas_draw_rframe(canvas, tmp_x_frame, tmp_y_frame, 26, 11, 1);
                    canvas_draw_box(canvas, tmp_x_frame, tmp_y_frame, 26, 11);
                    canvas_set_color(canvas, ColorWhite);
                    snprintf(buffer, sizeof(buffer), "%dm", cnt_min);
                    canvas_draw_str_aligned(
                        canvas, tmp_x_text, tmp_y_text, AlignCenter, AlignCenter, buffer);
                }
            }

        } else {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_rframe(canvas, tmp_x_frame, tmp_y_frame, 26, 11, 1);
            canvas_set_color(canvas, ColorBlack);
            snprintf(buffer, sizeof(buffer), "%d", diffold);
            canvas_draw_str_aligned(
                canvas, tmp_x_text, tmp_y_text, AlignCenter, AlignCenter, buffer);
        }
    }
    //DATA AGE end

    elements_bold_rounded_frame(canvas, 2, 37, 123, 25);
    canvas_set_font(canvas, FontPrimary);

    if(model->generic->temp != WS_NO_TEMPERATURE) {
        canvas_draw_icon(canvas, 18, 42, &I_Therm_7x16);
        snprintf(buffer, sizeof(buffer), "%3.1f C", (double)model->generic->temp);
        canvas_draw_str_aligned(canvas, 63, 46, AlignRight, AlignTop, buffer);
        canvas_draw_circle(canvas, 55, 45, 1);
    }

    if(model->generic->humidity != WS_NO_HUMIDITY) {
        canvas_draw_icon(canvas, 75, 42, &I_Humid_10x15);
        snprintf(buffer, sizeof(buffer), "%d%%", model->generic->humidity);
        canvas_draw_str(canvas, 91, 54, buffer);
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

void ws_view_receiver_info_enter(void* context) {
    furi_assert(context);
}

void ws_view_receiver_info_exit(void* context) {
    furi_assert(context);
    WSReceiverInfo* ws_receiver_info = context;

    with_view_model(
        ws_receiver_info->view,
        WSReceiverInfoModel * model,
        { furi_string_reset(model->protocol_name); },
        false);
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

    return ws_receiver_info;
}

void ws_view_receiver_info_free(WSReceiverInfo* ws_receiver_info) {
    furi_assert(ws_receiver_info);

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
