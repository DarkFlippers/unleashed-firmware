#include "power_views.h"

static void draw_stat(Canvas* canvas, int x, int y, IconName icon, char* val) {
    canvas_draw_frame(canvas, x - 7, y + 7, 30, 13);
    canvas_draw_icon_name(canvas, x, y, icon);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x - 4, y + 16, 24, 6);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str_aligned(canvas, x + 8, y + 22, AlignCenter, AlignBottom, val);
};

static void draw_battery(Canvas* canvas, PowerInfoModel* data, int x, int y) {
    char emote[20];
    char header[20];
    char value[20];

    int32_t drain_current = -data->current_gauge * 1000;
    uint32_t charge_current = data->current_gauge * 1000;
    // battery
    canvas_draw_icon_name(canvas, x, y, I_BatteryBody_52x28);
    if(charge_current > 0) {
        canvas_draw_icon_name(canvas, x + 16, y + 7, I_FaceCharging_29x14);
    } else if(drain_current > 100) {
        canvas_draw_icon_name(canvas, x + 16, y + 7, I_FaceConfused_29x14);
    } else if(data->charge < 10) {
        canvas_draw_icon_name(canvas, x + 16, y + 7, I_FaceNopower_29x14);
    } else {
        canvas_draw_icon_name(canvas, x + 16, y + 7, I_FaceNormal_29x14);
    }

    //bubble
    canvas_draw_frame(canvas, 57, 0, 71, 39);
    canvas_draw_line(canvas, 53, 23, 57, 19);
    canvas_draw_line(canvas, 53, 23, 57, 27);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 57, 0, 2, 2);
    canvas_draw_box(canvas, 57, 37, 2, 2);
    canvas_draw_box(canvas, 126, 0, 2, 2);
    canvas_draw_box(canvas, 126, 37, 2, 2);
    canvas_draw_line(canvas, 57, 20, 57, 26);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_dot(canvas, 58, 1);
    canvas_draw_dot(canvas, 58, 37);
    canvas_draw_dot(canvas, 126, 1);
    canvas_draw_dot(canvas, 126, 37);

    // text
    if(charge_current > 0) {
        snprintf(emote, sizeof(emote), "%s", "Yummy!");
        snprintf(header, sizeof(header), "%s", "Charging at");
        snprintf(
            value,
            sizeof(value),
            "%ld.%ldV   %ldmA",
            (uint32_t)(data->voltage_vbus),
            (uint32_t)(data->voltage_vbus * 10) % 10,
            charge_current);
    } else if(drain_current > 0) {
        snprintf(emote, sizeof(emote), "%s", drain_current > 100 ? "Oh no!" : "Om-nom-nom!");
        snprintf(header, sizeof(header), "%s", "Consumption is");
        snprintf(
            value, sizeof(value), "%ld %s", drain_current, drain_current > 100 ? "mA!" : "mA");
    } else if(charge_current != 0 || drain_current != 0) {
        snprintf(header, 20, "%s", "...");
        memset(value, 0, sizeof(value));
        memset(emote, 0, sizeof(emote));
    } else {
        snprintf(header, sizeof(header), "%s", "Charged!");
        memset(value, 0, sizeof(value));
        memset(emote, 0, sizeof(emote));
    }

    canvas_draw_str_aligned(canvas, 92, y + 3, AlignCenter, AlignCenter, emote);
    canvas_draw_str_aligned(canvas, 92, y + 15, AlignCenter, AlignCenter, header);
    canvas_draw_str_aligned(canvas, 92, y + 27, AlignCenter, AlignCenter, value);
};

void power_info_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    PowerInfoModel* data = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    draw_battery(canvas, data, 0, 5);

    char batt_level[10];
    char temperature[10];
    char voltage[10];
    char health[10];

    snprintf(batt_level, sizeof(batt_level), "%ld%s", (uint32_t)data->charge, "%");
    snprintf(temperature, sizeof(temperature), "%ld %s", (uint32_t)data->temperature_gauge, "C");
    snprintf(
        voltage,
        sizeof(voltage),
        "%ld.%01ld V",
        (uint32_t)data->voltage_gauge,
        (uint32_t)(data->voltage_gauge * 10) % 10);
    snprintf(health, sizeof(health), "%d%s", data->health, "%");

    draw_stat(canvas, 8, 42, I_Battery_16x16, batt_level);
    draw_stat(canvas, 40, 42, I_Temperature_16x16, temperature);
    draw_stat(canvas, 72, 42, I_Voltage_16x16, voltage);
    draw_stat(canvas, 104, 42, I_Health_16x16, health);
}

void power_off_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    PowerOffModel* model = context;

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 15, "!!! Low Battery !!!");

    char buffer[64];
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 5, 30, "Connect to charger");
    snprintf(
        buffer,
        64,
        "Or poweroff in %lds",
        (model->poweroff_tick - osKernelGetTickCount()) / osKernelGetTickFreq());
    canvas_draw_str(canvas, 5, 42, buffer);
}
