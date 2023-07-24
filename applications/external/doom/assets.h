#pragma once
#include <gui/icon.h>

#ifndef _sprites_h
#define _sprites_h

#define bmp_font_width 24 // in bytes
#define bmp_font_height 6
#define bmp_font_width_pxs 192
#define bmp_font_height_pxs 48
#define CHAR_MAP " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.,-_(){}[]#"
#define CHAR_WIDTH 4
#define CHAR_HEIGHT 6

#define BMP_GUN_WIDTH 32
#define BMP_GUN_HEIGHT 32

#define BMP_FIRE_WIDTH 24
#define BMP_FIRE_HEIGHT 20

#define BMP_IMP_WIDTH 32
#define BMP_IMP_HEIGHT 32
#define BMP_IMP_COUNT 5

#define BMP_FIREBALL_WIDTH 16
#define BMP_FIREBALL_HEIGHT 16

#define BMP_DOOR_WIDTH 100
#define BMP_DOOR_HEIGHT 100

#define BMP_ITEMS_WIDTH 16
#define BMP_ITEMS_HEIGHT 16
#define BMP_ITEMS_COUNT 2

#define BMP_LOGO_WIDTH 128
#define BMP_LOGO_HEIGHT 64

#define GRADIENT_WIDTH 2
#define GRADIENT_HEIGHT 8
#define GRADIENT_COUNT 8
#define GRADIENT_WHITE 7
#define GRADIENT_BLACK 0

// Fonts
extern const uint8_t zero[];
extern const uint8_t one[];
extern const uint8_t two[];
extern const uint8_t three[];
extern const uint8_t four[];
extern const uint8_t five[];
extern const uint8_t six[];
extern const uint8_t seven[];
extern const uint8_t eight[];
extern const uint8_t nine[];
extern const uint8_t A[];
extern const uint8_t B[];
extern const uint8_t C[];
extern const uint8_t D[];
extern const uint8_t E[];
extern const uint8_t F[];
extern const uint8_t G[];
extern const uint8_t H[];
extern const uint8_t I[];
extern const uint8_t J[];
extern const uint8_t K[];
extern const uint8_t L[];
extern const uint8_t M[];
extern const uint8_t N[];
extern const uint8_t O[];
extern const uint8_t P[];
extern const uint8_t Q[];
extern const uint8_t R[];
extern const uint8_t S[];
extern const uint8_t T[];
extern const uint8_t U[];
extern const uint8_t V[];
extern const uint8_t W[];
extern const uint8_t X[];
extern const uint8_t Y[];
extern const uint8_t Z[];
extern const uint8_t dot[];
extern const uint8_t comma[];
extern const uint8_t dash[];
extern const uint8_t underscore[];
extern const uint8_t bracket_open[];
extern const uint8_t bracket_close[];
extern const uint8_t cross_left[];
extern const uint8_t cross_right[];
extern const uint8_t pacman_left[];
extern const uint8_t pacman_right[];
extern const uint8_t box[];
extern const uint8_t* char_arr[48];
extern const uint8_t gradient[];
//extern const uint8_t gun[]
//extern const uint8_t gun_mask[]
extern const uint8_t gun[];
extern const uint8_t gun_mask[];

extern const uint8_t imp_inv[];
extern const uint8_t imp_mask_inv[];
extern const uint8_t fireball[];
extern const uint8_t fireball_mask[];
extern const uint8_t item[];
extern const uint8_t item_mask[];

extern const uint8_t door[];

#endif
