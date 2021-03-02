#include "power_views.h"

void power_info_draw_callback(Canvas* canvas, void* context) {
    PowerInfoModel* data = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Power state:");

    char buffer[64];
    canvas_set_font(canvas, FontSecondary);
    snprintf(
        buffer,
        64,
        "Current: %ld/%ldmA",
        (int32_t)(data->current_gauge * 1000),
        (int32_t)(data->current_charger * 1000));
    canvas_draw_str(canvas, 5, 22, buffer);
    snprintf(
        buffer,
        64,
        "Voltage: %ld/%ldmV",
        (uint32_t)(data->voltage_gauge * 1000),
        (uint32_t)(data->voltage_charger * 1000));
    canvas_draw_str(canvas, 5, 32, buffer);
    snprintf(
        buffer,
        64,
        "Charge: %ld%% Health: %ld%%",
        (uint32_t)(data->charge),
        (uint32_t)(data->health));
    canvas_draw_str(canvas, 5, 42, buffer);
    snprintf(buffer, 64, "Capacity: %ld of %ldmAh", data->capacity_remaining, data->capacity_full);
    canvas_draw_str(canvas, 5, 52, buffer);
    snprintf(
        buffer,
        64,
        "Temperature: %ld/%ldC",
        (uint32_t)(data->temperature_gauge),
        (uint32_t)(data->temperature_charger));
    canvas_draw_str(canvas, 5, 62, buffer);
}
