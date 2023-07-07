#ifndef BACKGROUND_ASSETS_H
#define BACKGROUND_ASSETS_H

#include <stdlib.h>
#include <stdbool.h>

#include <gui/gui.h>

#include "point.h"
#include "states.h"
#include "game_sprites.h"
#include <jetpack_joyride_icons.h>

#define BG_ASSETS_MAX 3

typedef struct {
    int width;
    int spawn_chance;
    int x_offset;
    int y_offset;
    const Icon* sprite;
} AssetProperties;

typedef struct {
    POINT point;
    AssetProperties* properties;
    bool visible;
} BackgroundAsset;

void background_assets_tick(BackgroundAsset* const assets);
void spawn_random_background_asset(BackgroundAsset* const assets);
void draw_background_assets(const BackgroundAsset* assets, Canvas* const canvas, int distance);

#endif // BACKGROUND_ASSETS_H
