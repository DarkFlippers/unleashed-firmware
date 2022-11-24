#pragma once

#include <furi.h>
#include <gui/canvas.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

typedef enum {
    Black,
    White,
    Inverse,
    Filled //Currently only for Icon clip drawing
} DrawMode;

// size is the screen size

typedef struct {
    uint8_t* data;
    unsigned long iconId;
} TileMap;

bool test_pixel(uint8_t* data, uint8_t x, uint8_t y, uint8_t w);

uint8_t* image_data(Canvas* const canvas, const Icon* icon);

uint32_t pixel_index(uint8_t x, uint8_t y);

void draw_icon_clip(
    Canvas* const canvas,
    const Icon* icon,
    int16_t x,
    int16_t y,
    uint8_t left,
    uint8_t top,
    uint8_t w,
    uint8_t h,
    DrawMode drawMode);

void draw_icon_clip_flipped(
    Canvas* const canvas,
    const Icon* icon,
    int16_t x,
    int16_t y,
    uint8_t left,
    uint8_t top,
    uint8_t w,
    uint8_t h,
    DrawMode drawMode);

void draw_rounded_box(
    Canvas* const canvas,
    int16_t x,
    int16_t y,
    uint8_t w,
    uint8_t h,
    DrawMode drawMode);

void draw_rounded_box_frame(
    Canvas* const canvas,
    int16_t x,
    int16_t y,
    uint8_t w,
    uint8_t h,
    DrawMode drawMode);

void draw_rectangle(
    Canvas* const canvas,
    int16_t x,
    int16_t y,
    uint8_t w,
    uint8_t h,
    DrawMode drawMode);

void invert_rectangle(Canvas* const canvas, int16_t x, int16_t y, uint8_t w, uint8_t h);

void invert_shape(Canvas* const canvas, uint8_t* data, int16_t x, int16_t y, uint8_t w, uint8_t h);

void draw_pixels(
    Canvas* const canvas,
    uint8_t* data,
    int16_t x,
    int16_t y,
    uint8_t w,
    uint8_t h,
    DrawMode drawMode);

bool read_pixel(Canvas* const canvas, int16_t x, int16_t y);

void set_pixel(Canvas* const canvas, int16_t x, int16_t y, DrawMode draw_mode);

void draw_line(
    Canvas* const canvas,
    int16_t x1,
    int16_t y1,
    int16_t x2,
    int16_t y2,
    DrawMode draw_mode);

bool in_screen(int16_t x, int16_t y);

void ui_cleanup();
uint8_t* get_buffer(Canvas* const canvas);
uint8_t* make_buffer();
void clone_buffer(uint8_t* canvas, uint8_t* data);