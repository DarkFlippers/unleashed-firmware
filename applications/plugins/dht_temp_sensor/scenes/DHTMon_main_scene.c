#include "../quenon_dht_mon.h"

/* ============== Главный экран ============== */
void scene_main(Canvas* const canvas, PluginData* app) {
    //Рисование бара
    canvas_draw_box(canvas, 0, 0, 128, 14);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 32, 11, "DHT Monitor");

    canvas_set_color(canvas, ColorBlack);
    if(app->sensors_count > 0) {
        if(!furi_hal_power_is_otg_enabled()) {
            furi_hal_power_enable_otg();
        }
        for(uint8_t i = 0; i < app->sensors_count; i++) {
            app->data = DHT_getData(&app->sensors[i]);

            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 0, 24 + 10 * i, app->sensors[i].name);

            canvas_set_font(canvas, FontSecondary);
            if(app->data.hum == -128.0f && app->data.temp == -128.0f) {
                canvas_draw_str(canvas, 96, 24 + 10 * i, "timeout");
            } else {
                snprintf(
                    app->txtbuff,
                    sizeof(app->txtbuff),
                    "%2.1f*C/%d%%",
                    (double)app->data.temp,
                    (int8_t)app->data.hum);
                canvas_draw_str(canvas, 64, 24 + 10 * i, app->txtbuff);
            }
        }
    } else {
        canvas_set_font(canvas, FontSecondary);
        if(app->sensors_count == 0) canvas_draw_str(canvas, 0, 24, "Sensors not found");
        if(app->sensors_count == -1) canvas_draw_str(canvas, 0, 24, "Loading...");
    }
}
