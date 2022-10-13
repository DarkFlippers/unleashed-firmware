#include "ui_controls.h"
#include "constants.h"
#include "icons.h"

#define TEXT_BOX_HEIGHT 13
#define TEXT_BOX_MARGIN 4

void ui_control_text_box_render(Canvas* const canvas, int8_t y, char* text, bool is_selected) {
    if(y < -TEXT_BOX_HEIGHT) {
        return;
    }

    if(is_selected) {
        canvas_draw_rframe(
            canvas,
            TEXT_BOX_MARGIN,
            TEXT_BOX_MARGIN + y,
            SCREEN_WIDTH - TEXT_BOX_MARGIN - TEXT_BOX_MARGIN,
            TEXT_BOX_HEIGHT,
            0);
        canvas_draw_rframe(
            canvas,
            TEXT_BOX_MARGIN - 1,
            TEXT_BOX_MARGIN + y - 1,
            SCREEN_WIDTH - TEXT_BOX_MARGIN - TEXT_BOX_MARGIN + 2,
            TEXT_BOX_HEIGHT + 2,
            1);
    } else {
        canvas_draw_rframe(
            canvas,
            TEXT_BOX_MARGIN,
            TEXT_BOX_MARGIN + y,
            SCREEN_WIDTH - TEXT_BOX_MARGIN - TEXT_BOX_MARGIN,
            TEXT_BOX_HEIGHT,
            1);
    }

    canvas_draw_str_aligned(
        canvas, TEXT_BOX_MARGIN + 2, TEXT_BOX_MARGIN + 3 + y, AlignLeft, AlignTop, text);
}

void ui_control_select_render(
    Canvas* const canvas,
    int16_t x,
    int16_t y,
    uint8_t width,
    char* text,
    bool is_selected) {
    if(y < -TEXT_BOX_HEIGHT) {
        return;
    }

    if(is_selected) {
        canvas_draw_rframe(
            canvas,
            x + TEXT_BOX_MARGIN,
            TEXT_BOX_MARGIN + y,
            width - TEXT_BOX_MARGIN - TEXT_BOX_MARGIN,
            TEXT_BOX_HEIGHT,
            0);
        canvas_draw_rframe(
            canvas,
            x + TEXT_BOX_MARGIN - 1,
            TEXT_BOX_MARGIN + y - 1,
            width - TEXT_BOX_MARGIN - TEXT_BOX_MARGIN + 2,
            TEXT_BOX_HEIGHT + 2,
            1);
    } else {
        canvas_draw_rframe(
            canvas,
            x + TEXT_BOX_MARGIN,
            TEXT_BOX_MARGIN + y,
            width - TEXT_BOX_MARGIN - TEXT_BOX_MARGIN,
            TEXT_BOX_HEIGHT,
            1);
    }

    canvas_draw_str_aligned(
        canvas, x + (width >> 1), TEXT_BOX_MARGIN + 3 + y, AlignCenter, AlignTop, text);
    canvas_draw_xbm(
        canvas,
        x + TEXT_BOX_MARGIN + 2,
        TEXT_BOX_MARGIN + 2 + y,
        ICON_ARROW_LEFT_8x9_WIDTH,
        ICON_ARROW_LEFT_8x9_HEIGHT,
        &ICON_ARROW_LEFT_8x9[0]);
    canvas_draw_xbm(
        canvas,
        x + width - TEXT_BOX_MARGIN - 10,
        TEXT_BOX_MARGIN + 2 + y,
        ICON_ARROW_RIGHT_8x9_WIDTH,
        ICON_ARROW_RIGHT_8x9_HEIGHT,
        &ICON_ARROW_RIGHT_8x9[0]);
}

void ui_control_button_render(
    Canvas* const canvas,
    int16_t x,
    int16_t y,
    uint8_t width,
    uint8_t height,
    char* text,
    bool is_selected) {
    if(y < -height) {
        return;
    }

    if(is_selected) {
        canvas_draw_rbox(canvas, x, y, width, height, 1);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_draw_rframe(canvas, x, y, width, height, 1);
    }

    canvas_draw_str_aligned(
        canvas, x + (width >> 1), y + (height >> 1) + 1, AlignCenter, AlignCenter, text);
    if(is_selected) {
        canvas_set_color(canvas, ColorBlack);
    }
}
