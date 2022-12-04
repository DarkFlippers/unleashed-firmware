#include <stdbool.h>
#include <stdint.h>
#include <gui/canvas.h>

/*  7px 3 width digit font by Sefjor
 * digit encoding example
 *7 ¦¦¦ 111
 *6 ¦ ¦ 101
 *5 ¦ ¦ 101
 *4 ¦ ¦ 101
 *3 ¦ ¦ 101
 *2 ¦ ¦ 101
 *1 ¦¦¦ 111
 *0     000 this string is empty, used to align
 *     ? ? ?
 *   FE 82 FE  //0
 */

static uint8_t font[10][3] = {
    {0xFE, 0x82, 0xFE}, // 0;
    {0x00, 0xFE, 0x00}, // 1;
    {0xF2, 0x92, 0x9E}, // 2;
    {0x92, 0x92, 0xFE}, // 3;
    {0x1E, 0x10, 0xFE}, // 4;
    {0x9E, 0x92, 0xF2}, // 5;
    {0xFE, 0x92, 0xF2}, // 6;
    {0x02, 0x02, 0xFE}, // 7;
    {0xFE, 0x92, 0xFE}, // 8;
    {0x9E, 0x92, 0xFE}, // 9;
};

#define FONT_HEIGHT 8
#define FONT_WIDTH 3

static void game_2048_draw_black_point(Canvas* const canvas, uint8_t x, uint8_t y) {
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_dot(canvas, x, y);
}

static void game_2048_draw_white_square(Canvas* const canvas, uint8_t x, uint8_t y) {
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x, y, 15 - 1, 15 - 3);
}

static void _game_2048_draw_column(
    Canvas* const canvas,
    int digit,
    int coord_x,
    int coord_y,
    uint8_t column) {
    for(int x = 0; x < FONT_HEIGHT; ++x) {
        bool is_filled = (font[digit][column] >> x) & 0x1;
        if(is_filled) {
            game_2048_draw_black_point(canvas, coord_x, coord_y + x);
        }
    }
}

static uint8_t
    _game_2048_draw_digit(Canvas* const canvas, uint8_t digit, uint8_t coord_x, uint8_t coord_y) {
    uint8_t x_shift = 0;

    if(digit != 1) {
        for(int column = 0; column < FONT_WIDTH; column++) {
            _game_2048_draw_column(canvas, digit, coord_x + column, coord_y, column);
        }
        x_shift = 3;
    } else {
        _game_2048_draw_column(canvas, digit, coord_x, coord_y, true);
        x_shift = 1;
    }

    return x_shift;
}

/* We drawing text field with 1px white border
 * at given coords. Total size is:
 *  x = 9 = 1 + 7 + 1
 *  y = 1 + total text width + 1
 */

/*
 * Returns array of digits and it's size,
 * digits should be at least 4 size
 * works from 1 to 9999
 */
static void _game_2048_parse_number(uint16_t number, uint8_t* digits, uint8_t* size) {
    *size = 0;
    uint16_t divider = 1000;
    //find first digit, result is highest divider
    while(number / divider == 0) {
        divider /= 10;
        if(divider == 0) {
            break;
        }
    }

    for(int i = 0; divider != 0; i++) {
        digits[i] = number / divider;
        number %= divider;
        *size += 1;
        divider /= 10;
    }
}

uint8_t _game_2048_calculate_shift(uint16_t num) {
    uint8_t shift = 0;
    switch(num) {
    case 1:
        shift = 7;
        break;
    case 2:
    case 4:
    case 8:
        shift = 6;
        break;
    case 16:
        shift = 5;
        break;
    case 32:
    case 64:
        shift = 4;
        break;
    case 128:
        shift = 3;
        break;
    case 256:
        shift = 2;
        break;
    case 512:
        shift = 3;
        break;
    case 1024:
        shift = 2;
        break;
    }
    return shift;
}

void game_2048_draw_number(Canvas* const canvas, uint8_t x, uint8_t y, int number) {
    uint8_t digits[4];
    uint8_t size;

    _game_2048_parse_number(number, digits, &size);
    if(number > 512) {
        game_2048_draw_white_square(canvas, x, y);
    }

    x += _game_2048_calculate_shift(number);
    y += 4;
    for(int i = 0; i < size; ++i) {
        x += _game_2048_draw_digit(canvas, digits[i], x, y);
        x++;
    }
}