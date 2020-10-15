#include "canvas.h"
#include "canvas_i.h"

#include <assert.h>
#include <flipper.h>
#include <u8g2.h>

struct Canvas {
    u8g2_t fb;
    uint8_t offset_x;
    uint8_t offset_y;
    uint8_t width;
    uint8_t height;
};

uint8_t canvas_width(CanvasApi* api);
uint8_t canvas_height(CanvasApi* api);
void canvas_clear(CanvasApi* api);
void canvas_color_set(CanvasApi* api, uint8_t color);
void canvas_font_set(CanvasApi* api, Font font);
void canvas_str_draw(CanvasApi* api, uint8_t x, uint8_t y, const char* str);

CanvasApi* canvas_api_init() {
    CanvasApi* api = furi_alloc(sizeof(CanvasApi));

    u8g2_t _u8g2;
    u8g2_Setup_st7565_erc12864_alt_f(
        &api->canvas.fb, U8G2_R0, u8x8_hw_spi_stm32, u8g2_gpio_and_delay_stm32);
    u8g2_InitDisplay(
        &canvas->fb); // send init sequence to the display, display is in sleep mode after this
    u8g2_SetContrast(&api->canvas.fb, 36);

    u8g2_SetPowerSave(&api->canvas.fb, 0); // wake up display
    u8g2_SendBuffer(&api->canvas.fb);

    api->width = canvas_width;
    api->height = canvas_height;
    api->clear = canvas_clear;
    api->canvas_color_set = canvas_color_set;
    api->canvas_font_set = canvas_font_set;
    api->draw_str = canvas_str_draw;

    api->fonts = {
        .primary = u8g2_font_Born2bSportyV2_tr,
        .secondary = u8g2_font_HelvetiPixel_tr,
    };

    return api;
}

void canvas_api_free(CanvasApi* api) {
    assert(api);
    free(api);
}

void canvas_commit(CanvasApi* api) {
    assert(api);

    u8g2_SetPowerSave(&api->canvas.fb, 0); // wake up display
    u8g2_SendBuffer(&api->canvas.fb);
}

void canvas_frame_set(
    CanvasApi* api,
    uint8_t offset_x,
    uint8_t offset_y,
    uint8_t width,
    uint8_t height) {
    assert(api);
    api->canvas.offset_x = offset_x;
    api->canvas.offset_y = offset_y;
    api->canvas.width = width;
    api->canvas.height = height;
}

uint8_t canvas_width(CanvasApi* api) {
    assert(api);

    return api->canvas.width;
}

uint8_t canvas_height(CanvasApi* api) {
    assert(api);

    return api->canvas.height;
}

void canvas_clear(CanvasApi* api) {
    assert(api);
    u8g2_ClearBuffer(&api->canvas.fb);
}

void canvas_color_set(CanvasApi* api, Color color) {
    assert(api);
    u8g2_SetDrawColor(&api->canvas.fb, color);
}

void canvas_font_set(CanvasApi* api, Font font) {
    assert(api);
    u8g2_SetFontMode(&api->canvas.fb, 1);
    u8g2_SetFont(&api->canvas.fb, font);
}

void canvas_str_draw(CanvasApi* api, uint8_t x, uint8_t y, const char* str) {
    assert(api);
    x += api->canvas.offset_x;
    y += api->canvas.offset_y;
    u8g2_DrawStr(&api->canvas.fb, x, y, str);
}
