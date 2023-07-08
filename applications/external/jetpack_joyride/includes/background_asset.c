#include <jetpack_joyride_icons.h>

#include "background_assets.h"

static AssetProperties assetProperties[BG_ASSETS_MAX] = {
    {.width = 27, .spawn_chance = 1, .x_offset = 24, .y_offset = 36, .sprite = &I_door},
    {.width = 12, .spawn_chance = 6, .x_offset = 33, .y_offset = 14, .sprite = &I_air_vent}};

void background_assets_tick(BackgroundAsset* const assets) {
    // Move assets towards the player
    for(int i = 0; i < BG_ASSETS_MAX; i++) {
        if(assets[i].visible) {
            assets[i].point.x -= 1; // move left by 2 units
            if(assets[i].point.x <=
               -assets[i].properties->width) { // if the asset is out of screen
                assets[i].visible = false; // set asset x coordinate to 0 to mark it as "inactive"
            }
        }
    }
}

void spawn_random_background_asset(BackgroundAsset* const assets) {
    // Calculate the total spawn chances for all assets
    int total_spawn_chance = 0;
    for(int i = 0; i < BG_ASSETS_MAX; ++i) {
        total_spawn_chance += assetProperties[i].spawn_chance;
    }

    // Generate a random number between 0 and total_spawn_chance
    int random_number = rand() % total_spawn_chance;

    // Select the asset based on the random number
    int chosen_asset = -1;
    int accumulated_chance = 0;
    for(int i = 0; i < BG_ASSETS_MAX; ++i) {
        accumulated_chance += assetProperties[i].spawn_chance;
        if(random_number < accumulated_chance) {
            chosen_asset = i;
            break;
        }
    }

    // If no asset is chosen, return
    if(chosen_asset == -1) {
        return;
    }

    // Look for an available slot for the chosen asset
    for(int i = 0; i < BG_ASSETS_MAX; ++i) {
        if(assets[i].visible == false) {
            // Spawn the asset
            assets[i].point.x = 127 + assetProperties[chosen_asset].x_offset;
            assets[i].point.y = assetProperties[chosen_asset].y_offset;
            assets[i].properties = &assetProperties[chosen_asset];
            assets[i].visible = true;
            break;
        }
    }
}

void draw_background_assets(const BackgroundAsset* assets, Canvas* const canvas, int distance) {
    canvas_draw_box(canvas, 0, 6, 128, 1);
    canvas_draw_box(canvas, 0, 56, 128, 2);

    // Calculate the pillar offset based on the traveled distance
    int pillar_offset = distance % 64;

    // Draw pillars
    for(int x = -pillar_offset; x < 128; x += 64) {
        canvas_draw_icon(canvas, x, 6, &I_pillar);
    }

    // Draw assets
    for(int i = 0; i < BG_ASSETS_MAX; ++i) {
        if(assets[i].visible) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_icon(
                canvas, assets[i].point.x, assets[i].point.y, assets[i].properties->sprite);
        }
    }
}
