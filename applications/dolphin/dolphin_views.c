#include "dolphin_views.h"
#include <gui/view.h>

void dolphin_view_first_start_draw(Canvas* canvas, void* model) {
    DolphinViewFirstStartModel* m = model;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    if(m->page == 0) {
        canvas_draw_icon_name(canvas, 0, 1, I_DolphinFirstStart0_128x54);
    } else if(m->page == 1) {
        canvas_draw_icon_name(canvas, 0, 1, I_DolphinFirstStart1_128x54);
    } else if(m->page == 2) {
        canvas_draw_icon_name(canvas, 0, 1, I_DolphinFirstStart2_128x54);
    } else if(m->page == 3) {
        canvas_draw_icon_name(canvas, 0, 1, I_DolphinFirstStart3_128x54);
    } else if(m->page == 4) {
        canvas_draw_icon_name(canvas, 0, 1, I_DolphinFirstStart4_128x54);
    } else if(m->page == 5) {
        canvas_draw_icon_name(canvas, 0, 1, I_DolphinFirstStart5_128x54);
    } else if(m->page == 6) {
        canvas_draw_icon_name(canvas, 0, 1, I_DolphinFirstStart6_128x54);
    } else if(m->page == 7) {
        canvas_draw_icon_name(canvas, 0, 1, I_DolphinFirstStart7_128x54);
    } else if(m->page == 8) {
        canvas_draw_icon_name(canvas, 0, 1, I_DolphinFirstStart8_128x54);
    }
}

void dolphin_view_idle_main_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon_name(canvas, 128 - 80, 0, I_Flipper_young_80x60);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 10, "/\\: Stats");
    canvas_draw_str(canvas, 5, 32, "OK: Menu");
    canvas_draw_str(canvas, 2, 52, "\\/: Version");
}

void dolphin_view_idle_stats_draw(Canvas* canvas, void* model) {
    DolphinViewIdleStatsModel* m = model;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Dolphin stats:");

    char buffer[64];
    canvas_set_font(canvas, FontSecondary);
    snprintf(buffer, 64, "Icounter: %ld", m->icounter);
    canvas_draw_str(canvas, 5, 22, buffer);
    snprintf(buffer, 64, "Butthurt: %ld", m->butthurt);
    canvas_draw_str(canvas, 5, 32, buffer);
    canvas_draw_str(canvas, 5, 40, "< > change icounter");
}

void dolphin_view_idle_debug_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Version info:");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 5, 22, TARGET " " BUILD_DATE);
    canvas_draw_str(canvas, 5, 32, GIT_BRANCH);
    canvas_draw_str(canvas, 5, 42, GIT_BRANCH_NUM);
    canvas_draw_str(canvas, 5, 52, GIT_COMMIT);
}

uint32_t dolphin_view_idle_back(void* context) {
    return DolphinViewIdleMain;
}
