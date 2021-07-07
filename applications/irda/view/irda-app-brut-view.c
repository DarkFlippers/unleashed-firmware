#include "api-hal-resources.h"
#include "assets_icons.h"
#include "gui/canvas.h"
#include "gui/view.h"
#include "input/input.h"
#include <gui/elements.h>
#include <furi.h>
#include "irda-app-brut-view.h"
#include "gui/modules/button_panel.h"
#include <stdint.h>

struct IrdaAppPopupBrut {
    uint16_t progress;
    uint16_t progress_max;
    char percents_string_storage[8];
};

void popup_brut_increase_progress(IrdaAppPopupBrut* popup_brut) {
    furi_assert(popup_brut);

    if(popup_brut->progress < popup_brut->progress_max)
        ++popup_brut->progress;
    else
        furi_assert(0);
}

void popup_brut_draw_callback(Canvas* canvas, void* context) {
    furi_assert(canvas);
    furi_assert(context);
    IrdaAppPopupBrut* popup_brut = (IrdaAppPopupBrut*)context;
    uint8_t x = 0;
    uint8_t width = 64;
    uint8_t x_max = x + width - 1;
    uint8_t y = 36;
    uint8_t height = 59;
    uint8_t y_max = y + height - 1;

    canvas_invert_color(canvas);
    canvas_draw_rbox(canvas, x + 1, y + 1, width - 2, height - 2, 3);
    canvas_invert_color(canvas);
    canvas_draw_rframe(canvas, x, y, width, height, 3);
    canvas_draw_rframe(canvas, x + 1, y + 1, width - 2, height - 2, 3);
    canvas_draw_line(canvas, x + 2, y + 1, x + 2, y + 3);
    canvas_draw_line(canvas, x + 1, y + 2, x + 3, y + 2);
    canvas_draw_line(canvas, x_max - 2, y + 1, x_max - 2, y + 3);
    canvas_draw_line(canvas, x_max - 1, y + 2, x_max - 3, y + 2);
    canvas_draw_line(canvas, x + 2, y_max - 1, x + 2, y_max - 3);
    canvas_draw_line(canvas, x + 1, y_max - 2, x + 3, y_max - 2);
    canvas_draw_line(canvas, x_max - 2, y_max - 1, x_max - 2, y_max - 3);
    canvas_draw_line(canvas, x_max - 1, y_max - 2, x_max - 3, y_max - 2);

    elements_progress_bar(
        canvas, x + 4, y + 19, x_max - 8, popup_brut->progress, popup_brut->progress_max);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, x + 15, y + 12, "Sending ...");
    canvas_draw_icon(canvas, x + 11, y_max - 14, &I_Back_15x10);

    uint8_t percent_value = 100 * popup_brut->progress / popup_brut->progress_max;
    snprintf(
        popup_brut->percents_string_storage,
        sizeof(popup_brut->percents_string_storage),
        "%d%%",
        percent_value);
    elements_multiline_text_aligned(
        canvas, x + 32, y + 40, AlignCenter, AlignBottom, popup_brut->percents_string_storage);
    canvas_draw_str(canvas, x + 30, y_max - 5, "= stop");
}

void popup_brut_set_progress_max(IrdaAppPopupBrut* popup_brut, uint16_t progress_max) {
    furi_assert(popup_brut);
    popup_brut->progress = 0;
    popup_brut->progress_max = progress_max;
}

IrdaAppPopupBrut* popup_brut_alloc(void) {
    return (IrdaAppPopupBrut*)furi_alloc(sizeof(IrdaAppPopupBrut));
}

void popup_brut_free(IrdaAppPopupBrut* popup_brut) {
    free(popup_brut);
}
