#include "canvas_i.h"
#include "icon_i.h"
#include "icon_animation_i.h"

#include <furi.h>
#include <api-hal.h>

uint8_t u8g2_gpio_and_delay_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_hw_spi_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);

Canvas* canvas_init() {
    Canvas* canvas = furi_alloc(sizeof(Canvas));

    api_hal_power_insomnia_enter();

    canvas->orientation = CanvasOrientationHorizontal;
    u8g2_Setup_st7565_erc12864_alt_f(
        &canvas->fb, U8G2_R0, u8x8_hw_spi_stm32, u8g2_gpio_and_delay_stm32);

    // send init sequence to the display, display is in sleep mode after this
    u8g2_InitDisplay(&canvas->fb);
    u8g2_SetContrast(&canvas->fb, 36);
    // wake up display
    u8g2_SetPowerSave(&canvas->fb, 0);
    u8g2_SendBuffer(&canvas->fb);

    api_hal_power_insomnia_exit();

    return canvas;
}

void canvas_free(Canvas* canvas) {
    furi_assert(canvas);
    free(canvas);
}

void canvas_reset(Canvas* canvas) {
    furi_assert(canvas);
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
}

void canvas_commit(Canvas* canvas) {
    furi_assert(canvas);
    u8g2_SetPowerSave(&canvas->fb, 0); // wake up display
    u8g2_SendBuffer(&canvas->fb);
}

uint8_t* canvas_get_buffer(Canvas* canvas) {
    furi_assert(canvas);
    return u8g2_GetBufferPtr(&canvas->fb);
}

size_t canvas_get_buffer_size(Canvas* canvas) {
    furi_assert(canvas);
    return u8g2_GetBufferTileWidth(&canvas->fb) * u8g2_GetBufferTileHeight(&canvas->fb) * 8;
}

void canvas_frame_set(
    Canvas* canvas,
    uint8_t offset_x,
    uint8_t offset_y,
    uint8_t width,
    uint8_t height) {
    furi_assert(canvas);
    canvas->offset_x = offset_x;
    canvas->offset_y = offset_y;
    canvas->width = width;
    canvas->height = height;
}

uint8_t canvas_width(Canvas* canvas) {
    furi_assert(canvas);
    return canvas->width;
}

uint8_t canvas_height(Canvas* canvas) {
    furi_assert(canvas);
    return canvas->height;
}

uint8_t canvas_current_font_height(Canvas* canvas) {
    furi_assert(canvas);
    uint8_t font_height = u8g2_GetMaxCharHeight(&canvas->fb);

    if(canvas->fb.font == u8g2_font_haxrcorp4089_tr) {
        font_height += 1;
    }

    return font_height;
}

void canvas_clear(Canvas* canvas) {
    furi_assert(canvas);
    u8g2_ClearBuffer(&canvas->fb);
}

void canvas_set_color(Canvas* canvas, Color color) {
    furi_assert(canvas);
    u8g2_SetDrawColor(&canvas->fb, color);
}

void canvas_invert_color(Canvas* canvas) {
    canvas->fb.draw_color = !canvas->fb.draw_color;
}

void canvas_set_font(Canvas* canvas, Font font) {
    furi_assert(canvas);
    u8g2_SetFontMode(&canvas->fb, 1);
    if(font == FontPrimary) {
        u8g2_SetFont(&canvas->fb, u8g2_font_helvB08_tf);
    } else if(font == FontSecondary) {
        u8g2_SetFont(&canvas->fb, u8g2_font_haxrcorp4089_tr);
    } else if(font == FontKeyboard) {
        u8g2_SetFont(&canvas->fb, u8g2_font_profont11_mf);
    } else {
        furi_check(0);
    }
}

void canvas_draw_str(Canvas* canvas, uint8_t x, uint8_t y, const char* str) {
    furi_assert(canvas);
    if(!str) return;
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawStr(&canvas->fb, x, y, str);
}

void canvas_draw_str_aligned(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    const char* str) {
    furi_assert(canvas);
    if(!str) return;
    x += canvas->offset_x;
    y += canvas->offset_y;

    switch(horizontal) {
    case AlignLeft:
        break;
    case AlignRight:
        x -= u8g2_GetStrWidth(&canvas->fb, str);
        break;
    case AlignCenter:
        x -= (u8g2_GetStrWidth(&canvas->fb, str) / 2);
        break;
    default:
        furi_check(0);
        break;
    }

    switch(vertical) {
    case AlignTop:
        y += u8g2_GetAscent(&canvas->fb);
        break;
    case AlignBottom:
        break;
    case AlignCenter:
        y += (u8g2_GetAscent(&canvas->fb) / 2);
        break;
    default:
        furi_check(0);
        break;
    }

    u8g2_DrawStr(&canvas->fb, x, y, str);
}

uint16_t canvas_string_width(Canvas* canvas, const char* str) {
    furi_assert(canvas);
    if(!str) return 0;
    return u8g2_GetStrWidth(&canvas->fb, str);
}

uint8_t canvas_glyph_width(Canvas* canvas, char symbol) {
    furi_assert(canvas);
    return u8g2_GetGlyphWidth(&canvas->fb, symbol);
}

void canvas_draw_icon_animation(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    IconAnimation* icon_animation) {
    furi_assert(canvas);
    furi_assert(icon_animation);

    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawXBM(
        &canvas->fb,
        x,
        y,
        icon_animation_get_width(icon_animation),
        icon_animation_get_height(icon_animation),
        icon_animation_get_data(icon_animation));
}

void canvas_draw_icon(Canvas* canvas, uint8_t x, uint8_t y, const Icon* icon) {
    furi_assert(canvas);
    furi_assert(icon);

    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawXBM(
        &canvas->fb, x, y, icon_get_width(icon), icon_get_height(icon), icon_get_data(icon));
}

void canvas_draw_dot(Canvas* canvas, uint8_t x, uint8_t y) {
    furi_assert(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawPixel(&canvas->fb, x, y);
}

void canvas_draw_box(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    furi_assert(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawBox(&canvas->fb, x, y, width, height);
}

void canvas_draw_rbox(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius) {
    furi_assert(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawRBox(&canvas->fb, x, y, width, height, radius);
}

void canvas_draw_frame(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    furi_assert(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawFrame(&canvas->fb, x, y, width, height);
}

void canvas_draw_rframe(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius) {
    furi_assert(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawRFrame(&canvas->fb, x, y, width, height, radius);
}

void canvas_draw_line(Canvas* canvas, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    furi_assert(canvas);
    x1 += canvas->offset_x;
    y1 += canvas->offset_y;
    x2 += canvas->offset_x;
    y2 += canvas->offset_y;
    u8g2_DrawLine(&canvas->fb, x1, y1, x2, y2);
}

void canvas_draw_circle(Canvas* canvas, uint8_t x, uint8_t y, uint8_t radius) {
    furi_assert(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawCircle(&canvas->fb, x, y, radius, U8G2_DRAW_ALL);
}

void canvas_draw_disc(Canvas* canvas, uint8_t x, uint8_t y, uint8_t radius) {
    furi_assert(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawDisc(&canvas->fb, x, y, radius, U8G2_DRAW_ALL);
}

void canvas_draw_xbm(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t w,
    uint8_t h,
    const uint8_t* bitmap) {
    furi_assert(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawXBM(&canvas->fb, x, y, w, h, bitmap);
}

void canvas_draw_glyph(Canvas* canvas, uint8_t x, uint8_t y, uint16_t ch) {
    furi_assert(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawGlyph(&canvas->fb, x, y, ch);
}

void canvas_set_bitmap_mode(Canvas* canvas, bool alpha) {
    u8g2_SetBitmapMode(&canvas->fb, alpha ? 1 : 0);
}

void canvas_set_orientation(Canvas* canvas, CanvasOrientation orientation) {
    furi_assert(canvas);
    if(canvas->orientation != orientation) {
        canvas->orientation = orientation;
        if(canvas->orientation == CanvasOrientationHorizontal)
            u8g2_SetDisplayRotation(&canvas->fb, U8G2_R0);
        else if(canvas->orientation == CanvasOrientationVertical)
            u8g2_SetDisplayRotation(&canvas->fb, U8G2_R3);
        else
            furi_assert(0);
    }
}

CanvasOrientation canvas_get_orientation(const Canvas* canvas) {
    return canvas->orientation;
}
