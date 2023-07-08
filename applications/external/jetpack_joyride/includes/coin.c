#include <stdlib.h>
#include <stdbool.h>

#include <jetpack_joyride_icons.h>
#include <gui/gui.h>

#include "coin.h"
#include "barry.h"

#define PATTERN_MAX_HEIGHT 40

// Patterns
const COIN_PATTERN coin_patterns[] = {
    {// Square pattern
     .count = 9,
     .coins = {{0, 0}, {8, 0}, {16, 0}, {0, 8}, {8, 8}, {16, 8}, {0, 16}, {8, 16}, {16, 16}}},
    {// Wavy pattern (approximate sine wave)
     .count = 8,
     .coins = {{0, 8}, {8, 16}, {16, 24}, {24, 16}, {32, 8}, {40, 0}, {48, 8}, {56, 16}}},
    {// Diagonal pattern
     .count = 5,
     .coins = {{0, 0}, {8, 8}, {16, 16}, {24, 24}, {32, 32}}},
    // Add more patterns here
};

void coin_tick(COIN* const coins, BARRY* const barry, int* const total_coins) {
    // Move coins towards the player
    for(int i = 0; i < COINS_MAX; i++) {
        if(coin_colides(&coins[i], barry)) {
            coins[i].point.x = 0; // Remove the coin
            (*total_coins)++;
        }
        if(coins[i].point.x > 0) {
            coins[i].point.x -= 1; // move left by 1 unit
            if(coins[i].point.x < -COIN_WIDTH) { // if the coin is out of screen
                coins[i].point.x = 0; // set coin x coordinate to 0 to mark it as "inactive"
            }
        }
    }
}

bool coin_colides(COIN* const coin, BARRY* const barry) {
    return !(
        barry->point.x > coin->point.x + COIN_WIDTH || // Barry is to the right of the coin
        barry->point.x + BARRY_WIDTH < coin->point.x || // Barry is to the left of the coin
        barry->point.y > coin->point.y + COIN_WIDTH || // Barry is below the coin
        barry->point.y + BARRY_HEIGHT < coin->point.y); // Barry is above the coin
}

void spawn_random_coin(COIN* const coins) {
    // Select a random pattern
    int pattern_index = rand() % (sizeof(coin_patterns) / sizeof(coin_patterns[0]));
    const COIN_PATTERN* pattern = &coin_patterns[pattern_index];

    // Count available slots for new coins
    int available_slots = 0;
    for(int i = 0; i < COINS_MAX; ++i) {
        if(coins[i].point.x <= 0) {
            ++available_slots;
        }
    }

    // If there aren't enough slots, return without spawning coins
    if(available_slots < pattern->count) return;

    // Spawn coins according to the selected pattern
    int coin_index = 0;
    int random_offset = rand() % (SCREEN_HEIGHT - PATTERN_MAX_HEIGHT);
    int random_offset_x = rand() % 16;
    for(int i = 0; i < pattern->count; ++i) {
        // Find an available slot for a new coin
        while(coins[coin_index].point.x > 0 && coin_index < COINS_MAX) {
            ++coin_index;
        }
        // If no slot is available, stop spawning coins
        if(coin_index == COINS_MAX) break;

        // Spawn the coin
        coins[coin_index].point.x = SCREEN_WIDTH - 1 + pattern->coins[i].x + random_offset_x;
        coins[coin_index].point.y =
            random_offset +
            pattern->coins[i]
                .y; // The pattern is spawned at a random y position, but not too close to the screen edge
    }
}

void draw_coins(const COIN* coins, Canvas* const canvas, const GameSprites* sprites) {
    canvas_set_color(canvas, ColorBlack);
    for(int i = 0; i < COINS_MAX; ++i) {
        if(coins[i].point.x > 0) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_icon(canvas, coins[i].point.x, coins[i].point.y, sprites->coin);

            canvas_set_color(canvas, ColorWhite);
            canvas_draw_icon(canvas, coins[i].point.x, coins[i].point.y, sprites->coin_infill);
        }
    }
}
