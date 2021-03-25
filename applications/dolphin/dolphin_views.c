#include "dolphin_views.h"
#include <gui/view.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <api-hal.h>

static char* Lockmenu_Items[3] = {"Lock", "Set PIN", "DUMB mode"};
static char* Meta_Items[3] = {"Passport", "Games", "???"};

void dolphin_view_first_start_draw(Canvas* canvas, void* model) {
    DolphinViewFirstStartModel* m = model;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    uint8_t font_height = canvas_current_font_height(canvas);
    uint8_t width = canvas_width(canvas);
    uint8_t height = canvas_height(canvas);
    if(m->page == 0) {
        canvas_draw_icon_name(canvas, 0, height - 53, I_DolphinFirstStart0_70x53);
        elements_multiline_text(canvas, 75, 20, "Hey m8,\npress > to\ncontinue");
        elements_frame(canvas, 72, 20 - font_height, width - 70 - 4, font_height * 3 + 4);
    } else if(m->page == 1) {
        canvas_draw_icon_name(canvas, 0, height - 53, I_DolphinFirstStart1_59x53);
        elements_multiline_text(canvas, 64, 20, "First Of All,\n...      >");
        elements_frame(canvas, 61, 20 - font_height, width - 59 - 4, font_height * 2 + 4);
    } else if(m->page == 2) {
        canvas_draw_icon_name(canvas, 0, height - 51, I_DolphinFirstStart2_59x51);
        elements_multiline_text(canvas, 64, 20, "Thank you\nfor your\nsupport! >");
        elements_frame(canvas, 61, 20 - font_height, width - 59 - 4, font_height * 3 + 4);
    } else if(m->page == 3) {
        canvas_draw_icon_name(canvas, width - 57, height - 48, I_DolphinFirstStart3_57x48);
        elements_multiline_text(canvas, 5, 20, "Kickstarter\ncampaign\nwas INSANE! >");
        elements_frame(canvas, 2, 20 - font_height, width - 57 - 4, font_height * 3 + 4);
    } else if(m->page == 4) {
        canvas_draw_icon_name(canvas, width - 67, height - 53, I_DolphinFirstStart4_67x53);
        elements_multiline_text(canvas, 5, 10, "Now\nallow me\nto introduce\nmyself >");
        elements_frame(canvas, 2, 10 - font_height, width - 67 - 4, font_height * 4 + 4);
    } else if(m->page == 5) {
        canvas_draw_icon_name(canvas, 0, height - 53, I_DolphinFirstStart5_45x53);
        elements_multiline_text(
            canvas, 50, 20, "I am Flipper,\ncyberdolphin\nliving in your\npocket >");
        elements_frame(canvas, 47, 20 - font_height, width - 45 - 4, font_height * 4 + 4);
    } else if(m->page == 6) {
        canvas_draw_icon_name(canvas, 0, height - 54, I_DolphinFirstStart6_58x54);
        elements_multiline_text(
            canvas, 63, 20, "I can grow\n smart'n'cool\nif you use me\noften >");
        elements_frame(canvas, 60, 20 - font_height, width - 58 - 4, font_height * 4 + 4);
    } else if(m->page == 7) {
        canvas_draw_icon_name(canvas, width - 61, height - 51, I_DolphinFirstStart7_61x51);
        elements_multiline_text(canvas, 5, 10, "As long as\nyou read, write\nand emulate >");
        elements_frame(canvas, 2, 10 - font_height, width - 54 - 4, font_height * 3 + 4);
    } else if(m->page == 8) {
        canvas_draw_icon_name(canvas, width - 56, height - 51, I_DolphinFirstStart8_56x51);
        elements_multiline_text(
            canvas, 5, 10, "You can check\nmy level and\nmood in the\nPassport menu");
        elements_frame(canvas, 2, 10 - font_height, width - 56 - 4, font_height * 4 + 4);
    }
}

void dolphin_view_idle_main_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    DolphinViewMainModel* m = model;
    if(m->animation) canvas_draw_icon(canvas, 0, 0, m->animation);
}

void dolphin_view_idle_up_draw(Canvas* canvas, void* model) {
    DolphinViewIdleUpModel* m = model;
    canvas_clear(canvas);

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 15, "Dolphin stats:");

    char buffer[64];
    canvas_set_font(canvas, FontSecondary);
    snprintf(buffer, 64, "Icounter: %ld", m->icounter);
    canvas_draw_str(canvas, 5, 30, buffer);
    snprintf(buffer, 64, "Butthurt: %ld", m->butthurt);
    canvas_draw_str(canvas, 5, 40, buffer);
    canvas_draw_str(canvas, 0, 53, "[< >] icounter value   [ok] save");
}

void dolphin_view_lockmenu_draw(Canvas* canvas, void* model) {
    DolphinViewMenuModel* m = model;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon_name(canvas, 5, 0, I_DoorLeft_8x56);
    canvas_draw_icon_name(canvas, 115, 0, I_DoorRight_8x56);
    canvas_set_font(canvas, FontSecondary);
    for(uint8_t i = 0; i < 3; ++i) {
        canvas_draw_str_aligned(
            canvas, 64, 13 + (i * 17), AlignCenter, AlignCenter, Lockmenu_Items[i]);
        if(m->idx == i) elements_frame(canvas, 15, 5 + (i * 17), 98, 15);
    }
}

void dolphin_view_idle_meta_draw(Canvas* canvas, void* model) {
    DolphinViewMenuModel* m = model;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    canvas_draw_icon_name(canvas, 20, 23, I_BigProfile_24x24);
    canvas_draw_icon_name(canvas, 55, 23, I_BigGames_24x24);
    canvas_draw_icon_name(canvas, 90, 23, I_BigBurger_24x24);

    canvas_draw_str_aligned(canvas, 66, 12, AlignCenter, AlignCenter, Meta_Items[m->idx]);

    canvas_draw_frame(canvas, 17 + (35 * m->idx), 20, 30, 30);
    canvas_set_color(canvas, ColorWhite);

    canvas_draw_dot(canvas, 17 + (35 * m->idx), 20);
    canvas_draw_dot(canvas, 17 + (35 * m->idx), 49);
    canvas_draw_dot(canvas, 46 + (35 * m->idx), 20);
    canvas_draw_dot(canvas, 46 + (35 * m->idx), 49);

    canvas_set_color(canvas, ColorBlack);
}

void dolphin_view_idle_down_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 15, "Version info:");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 5, 25, TARGET " " BUILD_DATE);
    canvas_draw_str(canvas, 5, 35, GIT_BRANCH);
    canvas_draw_str(canvas, 5, 45, GIT_BRANCH_NUM " " GIT_COMMIT);

    char buffer[64];
    snprintf(
        buffer,
        64,
        "HW: %d.F%dB%dC%d",
        api_hal_version_get_hw_version(),
        api_hal_version_get_hw_target(),
        api_hal_version_get_hw_body(),
        api_hal_version_get_hw_connect());
    canvas_draw_str(canvas, 5, 55, buffer);
}

void dolphin_view_hw_mismatch_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "!!!! HW Mismatch !!!!");

    char buffer[64];
    canvas_set_font(canvas, FontSecondary);
    snprintf(buffer, 64, "HW target: F%d", api_hal_version_get_hw_target());
    canvas_draw_str(canvas, 5, 22, buffer);
    canvas_draw_str(canvas, 5, 32, "FW target: " TARGET);
}

uint32_t dolphin_view_idle_back(void* context) {
    return DolphinViewIdleMain;
}
