#ifndef GAME_SPRITES_H
#define GAME_SPRITES_H

#include "point.h"
#include <gui/icon_animation.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define BARRY_WIDTH 11
#define BARRY_HEIGHT 15

#define MISSILE_WIDTH 26
#define MISSILE_HEIGHT 12

#define SCIENTIST_WIDTH 9
#define SCIENTIST_HEIGHT 14

#define COIN_WIDTH 7

typedef struct {
    IconAnimation* barry;
    const Icon* barry_infill;
    const Icon* scientist_left;
    const Icon* scientist_left_infill;
    const Icon* scientist_right;
    const Icon* scientist_right_infill;
    const Icon* coin;
    const Icon* coin_infill;
    IconAnimation* missile;
    IconAnimation* alert;
    const Icon* missile_infill;
} GameSprites;

#endif // GAME_SPRITES_H