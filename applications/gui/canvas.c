#include "canvas.h"
#include "canvas_i.h"
#include "icon.h"
#include "icon_i.h"

#include <flipper.h>
#include <flipper_v2.h>

typedef struct {
    CanvasApi api;

    u8g2_t fb;
    uint8_t offset_x;
    uint8_t offset_y;
    uint8_t width;
    uint8_t height;
} Canvas;

uint8_t canvas_width(CanvasApi* api);
uint8_t canvas_height(CanvasApi* api);
void canvas_clear(CanvasApi* api);
void canvas_color_set(CanvasApi* api, uint8_t color);
void canvas_font_set(CanvasApi* api, Font font);
void canvas_str_draw(CanvasApi* api, uint8_t x, uint8_t y, const char* str);
void canvas_icon_draw(CanvasApi* api, uint8_t x, uint8_t y, Icon* icon);
void canvas_dot_draw(CanvasApi* api, uint8_t x, uint8_t y);
void canvas_box_draw(CanvasApi* api, uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void canvas_draw_frame(CanvasApi* api, uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void canvas_draw_line(CanvasApi* api, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

uint8_t u8g2_gpio_and_delay_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_hw_spi_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);

CanvasApi* canvas_api_init() {
    Canvas* canvas = furi_alloc(sizeof(Canvas));

    u8g2_Setup_st7565_erc12864_alt_f(
        &canvas->fb, U8G2_R0, u8x8_hw_spi_stm32, u8g2_gpio_and_delay_stm32);

    // send init sequence to the display, display is in sleep mode after this
    u8g2_InitDisplay(&canvas->fb);
    u8g2_SetContrast(&canvas->fb, 36);

    u8g2_SetPowerSave(&canvas->fb, 0); // wake up display
    u8g2_SendBuffer(&canvas->fb);

    canvas->api.width = canvas_width;
    canvas->api.height = canvas_height;
    canvas->api.clear = canvas_clear;
    canvas->api.set_color = canvas_color_set;
    canvas->api.set_font = canvas_font_set;
    canvas->api.draw_str = canvas_str_draw;
    canvas->api.draw_icon = canvas_icon_draw;
    canvas->api.draw_dot = canvas_dot_draw;
    canvas->api.draw_box = canvas_box_draw;
    canvas->api.draw_frame = canvas_draw_frame;
    canvas->api.draw_line = canvas_draw_line;

    return (CanvasApi*)canvas;
}

void canvas_api_free(CanvasApi* api) {
    furi_assert(api);
    free(api);
}

void canvas_reset(CanvasApi* api) {
    assert(api);
    canvas_color_set(api, ColorBlack);
    canvas_font_set(api, FontSecondary);
}

void canvas_commit(CanvasApi* api) {
    furi_assert(api);
    Canvas* canvas = (Canvas*)api;
    u8g2_SetPowerSave(&canvas->fb, 0); // wake up display
    u8g2_SendBuffer(&canvas->fb);
}

void canvas_frame_set(
    CanvasApi* api,
    uint8_t offset_x,
    uint8_t offset_y,
    uint8_t width,
    uint8_t height) {
    furi_assert(api);
    Canvas* canvas = (Canvas*)api;
    canvas->offset_x = offset_x;
    canvas->offset_y = offset_y;
    canvas->width = width;
    canvas->height = height;
}

uint8_t canvas_width(CanvasApi* api) {
    furi_assert(api);
    Canvas* canvas = (Canvas*)api;
    return canvas->width;
}

uint8_t canvas_height(CanvasApi* api) {
    furi_assert(api);
    Canvas* canvas = (Canvas*)api;
    return canvas->height;
}

void canvas_clear(CanvasApi* api) {
    furi_assert(api);
    Canvas* canvas = (Canvas*)api;
    u8g2_ClearBuffer(&canvas->fb);
}

void canvas_color_set(CanvasApi* api, Color color) {
    furi_assert(api);
    Canvas* canvas = (Canvas*)api;
    u8g2_SetDrawColor(&canvas->fb, color);
}

void canvas_font_set(CanvasApi* api, Font font) {
    furi_assert(api);
    Canvas* canvas = (Canvas*)api;
    u8g2_SetFontMode(&canvas->fb, 1);
    if(font == FontPrimary) {
        u8g2_SetFont(&canvas->fb, u8g2_font_helvB08_tf);
    } else if(font == FontSecondary) {
        u8g2_SetFont(&canvas->fb, u8g2_font_haxrcorp4089_tr);
    } else {
        furi_check(0);
    }
}

void canvas_str_draw(CanvasApi* api, uint8_t x, uint8_t y, const char* str) {
    furi_assert(api);
    if(!str) return;
    Canvas* canvas = (Canvas*)api;
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawStr(&canvas->fb, x, y, str);
}

void canvas_icon_draw(CanvasApi* api, uint8_t x, uint8_t y, Icon* icon) {
    furi_assert(api);
    if(!icon) return;
    Canvas* canvas = (Canvas*)api;
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawXBM(
        &canvas->fb, x, y, icon_get_width(icon), icon_get_height(icon), icon_get_data(icon));
}

void canvas_dot_draw(CanvasApi* api, uint8_t x, uint8_t y) {
    furi_assert(api);
    Canvas* canvas = (Canvas*)api;
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawPixel(&canvas->fb, x, y);
}

void canvas_box_draw(CanvasApi* api, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    furi_assert(api);
    Canvas* canvas = (Canvas*)api;
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawBox(&canvas->fb, x, y, width, height);
}

void canvas_draw_frame(CanvasApi* api, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    furi_assert(api);
    Canvas* canvas = (Canvas*)api;
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_DrawFrame(&canvas->fb, x, y, width, height);
}

void canvas_draw_line(CanvasApi* api, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    furi_assert(api);
    Canvas* canvas = (Canvas*)api;
    x1 += canvas->offset_x;
    y1 += canvas->offset_y;
    x2 += canvas->offset_x;
    y2 += canvas->offset_y;
    u8g2_DrawLine(&canvas->fb, x1, y1, x2, y2);
}
