#include "standby.h"
#include <totp_icons.h>
#include "../../constants.h"

void totp_scene_standby_render(Canvas* const canvas) {
    canvas_draw_icon(canvas, SCREEN_WIDTH - 56, SCREEN_HEIGHT - 48, &I_DolphinCommon_56x48);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 5, 10, AlignLeft, AlignTop, "CLI command");

    canvas_draw_str_aligned(canvas, 5, 24, AlignLeft, AlignTop, "is running now");
}