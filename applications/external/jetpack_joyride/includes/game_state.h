#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <gui/icon_animation.h>
#include <furi.h>

#include "barry.h"
#include "scientist.h"
#include "coin.h"
#include "particle.h"
#include "game_sprites.h"
#include "states.h"
#include "missile.h"
#include "background_assets.h"
typedef struct {
    int total_coins;
    int distance;
    bool new_highscore;
    BARRY barry;
    COIN coins[COINS_MAX];
    PARTICLE particles[PARTICLES_MAX];
    SCIENTIST scientists[SCIENTISTS_MAX];
    MISSILE missiles[MISSILES_MAX];
    BackgroundAsset bg_assets[BG_ASSETS_MAX];
    State state;
    GameSprites sprites;
    FuriMutex* mutex;
    FuriTimer* timer;
    void (*death_handler)();
} GameState;

void game_state_tick(GameState* const game_state);

#endif // GAMESTATE_H