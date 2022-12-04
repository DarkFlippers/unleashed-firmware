#pragma once

#include "snake_types.h"
#include <furi.h>
#include <flipper_format/flipper_format.h>

#define APPS_DATA EXT_PATH("apps_data")
#define SNAKE_GAME_FILE_DIR_PATH APPS_DATA "/snake_game"
#define SNAKE_GAME_FILE_PATH SNAKE_GAME_FILE_DIR_PATH "/.snake"

#define SNAKE_GAME_FILE_HEADER "Flipper Snake plugin run file"
#define SNAKE_GAME_FILE_ACTUAL_VERSION 1

#define SNAKE_GAME_CONFIG_KEY_POINTS "SnakePoints"
#define SNAKE_GAME_CONFIG_KEY_LEN "SnakeLen"
#define SNAKE_GAME_CONFIG_KEY_CURRENT_MOVEMENT "CurrentMovement"
#define SNAKE_GAME_CONFIG_KEY_NEXT_MOVEMENT "NextMovement"
#define SNAKE_GAME_CONFIG_KEY_FRUIT_POINTS "FruitPoints"
#define SNAKE_GAME_CONFIG_HIGHSCORE "Highscore"

void snake_game_save_score_to_file(int16_t highscore);

void snake_game_save_game_to_file(SnakeState* const snake_state);

bool snake_game_init_game_from_file(SnakeState* const snake_state);
