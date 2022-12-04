#pragma once

#include <furi.h>

typedef struct {
    //    +-----x
    //    |
    //    |
    //    y
    uint8_t x;
    uint8_t y;
} Point;

typedef enum {
    GameStateLife,

    // https://melmagazine.com/en-us/story/snake-nokia-6110-oral-history-taneli-armanto
    // Armanto: While testing the early versions of the game, I noticed it was hard
    // to control the snake upon getting close to and edge but not crashing — especially
    // in the highest speed levels. I wanted the highest level to be as fast as I could
    // possibly make the device "run," but on the other hand, I wanted to be friendly
    // and help the player manage that level. Otherwise it might not be fun to play. So
    // I implemented a little delay. A few milliseconds of extra time right before
    // the player crashes, during which she can still change the directions. And if
    // she does, the game continues.
    GameStateLastChance,

    GameStateGameOver,
} GameState;

// Note: do not change without purpose. Current values are used in smart
// orthogonality calculation in `snake_game_get_turn_snake`.
typedef enum {
    DirectionUp,
    DirectionRight,
    DirectionDown,
    DirectionLeft,
} Direction;

#define MAX_SNAKE_LEN 253

typedef struct {
    Point points[MAX_SNAKE_LEN];
    uint16_t len;
    bool isNewHighscore;
    int16_t highscore;
    Direction currentMovement;
    Direction nextMovement; // if backward of currentMovement, ignore
    Point fruit;
    GameState state;
} SnakeState;
