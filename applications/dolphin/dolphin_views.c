#include "dolphin_views.h"
#include <gui/view.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <furi-hal.h>
#include <furi-hal-version.h>

static char* Lockmenu_Items[3] = {"Lock", "Set PIN", "DUMB mode"};

void dolphin_view_first_start_draw(Canvas* canvas, void* model) {
    DolphinViewFirstStartModel* m = model;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    uint8_t width = canvas_width(canvas);
    uint8_t height = canvas_height(canvas);
    const char* my_name = furi_hal_version_get_name_ptr();
    if(m->page == 0) {
        canvas_draw_icon(canvas, 0, height - 48, &I_DolphinFirstStart0_70x53);
        elements_multiline_text_framed(canvas, 75, 20, "Hey m8,\npress > to\ncontinue");
    } else if(m->page == 1) {
        canvas_draw_icon(canvas, 0, height - 48, &I_DolphinFirstStart1_59x53);
        elements_multiline_text_framed(canvas, 64, 20, "First Of All,\n...      >");
    } else if(m->page == 2) {
        canvas_draw_icon(canvas, 0, height - 48, &I_DolphinFirstStart2_59x51);
        elements_multiline_text_framed(canvas, 64, 20, "Thank you\nfor your\nsupport! >");
    } else if(m->page == 3) {
        canvas_draw_icon(canvas, width - 57, height - 48, &I_DolphinFirstStart3_57x48);
        elements_multiline_text_framed(canvas, 0, 20, "Kickstarter\ncampaign\nwas INSANE! >");
    } else if(m->page == 4) {
        canvas_draw_icon(canvas, width - 67, height - 50, &I_DolphinFirstStart4_67x53);
        elements_multiline_text_framed(canvas, 0, 17, "Now\nallow me\nto introduce\nmyself >");
    } else if(m->page == 5) {
        char buf[64];
        snprintf(
            buf,
            64,
            "%s %s%s",
            "I am",
            my_name ? my_name : "Unknown",
            ",\ncyberdolphin\nliving in your\npocket >");
        canvas_draw_icon(canvas, 0, height - 48, &I_DolphinFirstStart5_54x49);
        elements_multiline_text_framed(canvas, 60, 17, buf);
    } else if(m->page == 6) {
        canvas_draw_icon(canvas, 0, height - 48, &I_DolphinFirstStart6_58x54);
        elements_multiline_text_framed(
            canvas, 63, 17, "I can grow\nsmart'n'cool\nif you use me\noften >");
    } else if(m->page == 7) {
        canvas_draw_icon(canvas, width - 61, height - 48, &I_DolphinFirstStart7_61x51);
        elements_multiline_text_framed(
            canvas, 0, 17, "As long as\nyou read, write\nand emulate >");
    } else if(m->page == 8) {
        canvas_draw_icon(canvas, width - 56, height - 48, &I_DolphinFirstStart8_56x51);
        elements_multiline_text_framed(
            canvas, 0, 17, "You can check\nmy level and\nmood in the\nPassport menu");
    }
}

void dolphin_view_idle_main_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    DolphinViewMainModel* m = model;
    if(m->animation) {
        canvas_draw_icon_animation(canvas, 0, -3, m->animation);
    }

    if(m->hint_timeout) {
        m->hint_timeout--;
        if(m->locked) {
            canvas_draw_icon(canvas, 13, 5, &I_LockPopup_100x49);
            elements_multiline_text(canvas, 65, 20, "To unlock\npress:");
        } else {
            canvas_set_font(canvas, FontPrimary);
            elements_multiline_text_framed(canvas, 42, 30, "Unlocked");
        }
    }
}

void dolphin_view_lockmenu_draw(Canvas* canvas, void* model) {
    DolphinViewLockMenuModel* m = model;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon(canvas, m->door_left_x, 0, &I_DoorLeft_70x55);
    canvas_draw_icon(canvas, m->door_right_x, 0, &I_DoorRight_70x55);
    canvas_set_font(canvas, FontSecondary);

    if(m->locked) {
        m->exit_timeout--;

        m->door_left_x = CLAMP(m->door_left_x + 5, 0, -57);
        m->door_right_x = CLAMP(m->door_right_x - 5, 115, 60);

        if(m->door_left_x > -10) {
            canvas_set_font(canvas, FontPrimary);
            elements_multiline_text_framed(canvas, 42, 30, "Locked");
        }

    } else {
        if(m->door_left_x == -57) {
            for(uint8_t i = 0; i < 3; ++i) {
                canvas_draw_str_aligned(
                    canvas,
                    64,
                    13 + (i * 17),
                    AlignCenter,
                    AlignCenter,
                    (m->hint_timeout && m->idx == i) ? "Not implemented" : Lockmenu_Items[i]);
                if(m->idx == i) elements_frame(canvas, 15, 5 + (i * 17), 98, 15);
            }
        }

        if(m->hint_timeout) {
            m->hint_timeout--;
        }
    }
}

void dolphin_view_idle_down_draw(Canvas* canvas, void* model) {
    DolphinViewStatsModel* m = model;
    const Version* ver;
    char buffer[64];

    static const char* headers[] = {"FW Version info:", "Boot Version info:", "Dolphin info:"};

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 13, headers[m->screen]);
    canvas_set_font(canvas, FontSecondary);

    if(m->screen != DolphinViewStatsMeta) {
        // Hardware version
        const char* my_name = furi_hal_version_get_name_ptr();
        snprintf(
            buffer,
            sizeof(buffer),
            "HW: %d.F%dB%dC%d %s",
            furi_hal_version_get_hw_version(),
            furi_hal_version_get_hw_target(),
            furi_hal_version_get_hw_body(),
            furi_hal_version_get_hw_connect(),
            my_name ? my_name : "Unknown");
        canvas_draw_str(canvas, 5, 23, buffer);

        ver = m->screen == DolphinViewStatsBoot ? furi_hal_version_get_boot_version() :
                                                  furi_hal_version_get_firmware_version();

        if(!ver) {
            canvas_draw_str(canvas, 5, 33, "No info");
            return;
        }

        snprintf(
            buffer,
            sizeof(buffer),
            "%s [%s]",
            version_get_version(ver),
            version_get_builddate(ver));
        canvas_draw_str(canvas, 5, 33, buffer);

        snprintf(
            buffer,
            sizeof(buffer),
            "%s [%s]",
            version_get_githash(ver),
            version_get_gitbranchnum(ver));
        canvas_draw_str(canvas, 5, 43, buffer);

        snprintf(
            buffer, sizeof(buffer), "[%s] %s", version_get_target(ver), version_get_gitbranch(ver));
        canvas_draw_str(canvas, 5, 53, buffer);

    } else {
        char buffer[64];
        canvas_set_font(canvas, FontSecondary);
        snprintf(buffer, 64, "Icounter: %ld", m->icounter);
        canvas_draw_str(canvas, 5, 30, buffer);
        snprintf(buffer, 64, "Butthurt: %ld", m->butthurt);
        canvas_draw_str(canvas, 5, 40, buffer);
        canvas_draw_str(canvas, 0, 53, "[< >] icounter value   [ok] save");
    }
}

void dolphin_view_hw_mismatch_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 15, "!!!! HW Mismatch !!!!");

    char buffer[64];
    canvas_set_font(canvas, FontSecondary);
    snprintf(buffer, 64, "HW target: F%d", furi_hal_version_get_hw_target());
    canvas_draw_str(canvas, 5, 27, buffer);
    canvas_draw_str(canvas, 5, 38, "FW target: " TARGET);
}

uint32_t dolphin_view_idle_back(void* context) {
    return DolphinViewIdleMain;
}
