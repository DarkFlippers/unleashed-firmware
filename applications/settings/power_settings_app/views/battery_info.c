#include "battery_info.h"
#include <furi.h>
#include <gui/elements.h>

struct BatteryInfo {
    View* view;
};

static void draw_stat(Canvas* canvas, int x, int y, const Icon* icon, char* val) {
    canvas_draw_frame(canvas, x - 7, y + 7, 30, 13);
    canvas_draw_icon(canvas, x, y, icon);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x - 4, y + 16, 24, 6);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str_aligned(canvas, x + 8, y + 22, AlignCenter, AlignBottom, val);
};

static void draw_battery(Canvas* canvas, BatteryInfoModel* data, int x, int y) {
    char emote[20] = {};
    char header[20] = {};
    char value[20] = {};

    int32_t drain_current = data->gauge_current * (-1000);
    uint32_t charge_current = data->gauge_current * 1000;

    // Draw battery
    canvas_draw_icon(canvas, x, y, &I_BatteryBody_52x28);
    if(charge_current > 0) {
        canvas_draw_icon(canvas, x + 16, y + 7, &I_FaceCharging_29x14);
    } else if(drain_current > 100) {
        canvas_draw_icon(canvas, x + 16, y + 7, &I_FaceConfused_29x14);
    } else if(data->charge < 10) {
        canvas_draw_icon(canvas, x + 16, y + 7, &I_FaceNopower_29x14);
    } else {
        canvas_draw_icon(canvas, x + 16, y + 7, &I_FaceNormal_29x14);
    }

    // Draw bubble
    elements_bubble(canvas, 53, 0, 71, 39);

    // Set text
    if(charge_current > 0) {
        snprintf(emote, sizeof(emote), "%s", "Yummy!");
        snprintf(header, sizeof(header), "%s", "Charging at");
        snprintf(
            value,
            sizeof(value),
            "%ld.%ldV   %ldmA",
            (uint32_t)(data->vbus_voltage),
            (uint32_t)(data->vbus_voltage * 10) % 10,
            charge_current);
    } else if(drain_current > 0) {
        snprintf(emote, sizeof(emote), "%s", drain_current > 100 ? "Oh no!" : "Om-nom-nom!");
        snprintf(header, sizeof(header), "%s", "Consumption is");
        snprintf(
            value, sizeof(value), "%ld %s", drain_current, drain_current > 100 ? "mA!" : "mA");
    } else if(charge_current != 0 || drain_current != 0) {
        snprintf(header, 20, "...");
    } else {
        snprintf(header, sizeof(header), "Charged!");
    }

    canvas_draw_str_aligned(canvas, 92, y + 3, AlignCenter, AlignCenter, emote);
    canvas_draw_str_aligned(canvas, 92, y + 15, AlignCenter, AlignCenter, header);
    canvas_draw_str_aligned(canvas, 92, y + 27, AlignCenter, AlignCenter, value);
};

static void battery_info_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    BatteryInfoModel* model = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    draw_battery(canvas, model, 0, 5);

    char batt_level[10];
    char temperature[10];
    char voltage[10];
    char health[10];

    snprintf(batt_level, sizeof(batt_level), "%ld%%", (uint32_t)model->charge);
    snprintf(temperature, sizeof(temperature), "%ld C", (uint32_t)model->gauge_temperature);
    snprintf(
        voltage,
        sizeof(voltage),
        "%ld.%01ld V",
        (uint32_t)model->gauge_voltage,
        (uint32_t)(model->gauge_voltage * 10) % 10);
    snprintf(health, sizeof(health), "%d%%", model->health);

    draw_stat(canvas, 8, 42, &I_Battery_16x16, batt_level);
    draw_stat(canvas, 40, 42, &I_Temperature_16x16, temperature);
    draw_stat(canvas, 72, 42, &I_Voltage_16x16, voltage);
    draw_stat(canvas, 104, 42, &I_Health_16x16, health);
}

BatteryInfo* battery_info_alloc() {
    BatteryInfo* battery_info = malloc(sizeof(BatteryInfo));
    battery_info->view = view_alloc();
    view_set_context(battery_info->view, battery_info);
    view_allocate_model(battery_info->view, ViewModelTypeLocking, sizeof(BatteryInfoModel));
    view_set_draw_callback(battery_info->view, battery_info_draw_callback);

    return battery_info;
}

void battery_info_free(BatteryInfo* battery_info) {
    furi_assert(battery_info);
    view_free(battery_info->view);
    free(battery_info);
}

View* battery_info_get_view(BatteryInfo* battery_info) {
    furi_assert(battery_info);
    return battery_info->view;
}

void battery_info_set_data(BatteryInfo* battery_info, BatteryInfoModel* data) {
    furi_assert(battery_info);
    furi_assert(data);
    with_view_model(
        battery_info->view, (BatteryInfoModel * model) {
            memcpy(model, data, sizeof(BatteryInfoModel));
            return true;
        });
}
