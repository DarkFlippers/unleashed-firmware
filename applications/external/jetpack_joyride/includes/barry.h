#ifndef BARRY_H
#define BARRY_H

#include <stdbool.h>

#include <gui/gui.h>
#include "point.h"
#include "game_sprites.h"

#define GRAVITY_TICK 0.2
#define GRAVITY_BOOST -0.4
#define GRAVITY_FALL 0.3

typedef struct {
    float gravity;
    POINT point;
    bool isBoosting;
} BARRY;

void barry_tick(BARRY* const barry);
void draw_barry(const BARRY* barry, Canvas* const canvas, const GameSprites* sprites);

#endif // BARRY_H