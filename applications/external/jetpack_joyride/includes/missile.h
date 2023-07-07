#ifndef MISSILE_H
#define MISSILE_H

#include <gui/gui.h>
#include "game_sprites.h"

#include "states.h"
#include "point.h"
#include "barry.h"

#define MISSILES_MAX 5

typedef struct {
    POINT point;
    bool visible;
} MISSILE;

void missile_tick(MISSILE* const missiles, BARRY* const barry, void (*death_handler)());
void spawn_random_missile(MISSILE* const MISSILEs);
bool missile_colides(MISSILE* const MISSILE, BARRY* const barry);
int get_rocket_spawn_distance(int player_distance);
void draw_missiles(const MISSILE* missiles, Canvas* const canvas, const GameSprites* sprites);

#endif // MISSILE_H