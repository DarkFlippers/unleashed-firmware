#pragma once

#include "defines.h"
#include <gui/gui.h>

void draw_player_scene(Canvas* const canvas, const GameState* game_state);

void draw_dealer_scene(Canvas* const canvas, const GameState* game_state);

void draw_play_menu(Canvas* const canvas, const GameState* game_state);

void draw_score(Canvas* const canvas, bool top, uint8_t amount);

void draw_money(Canvas* const canvas, uint32_t score);
void settings_page(Canvas* const canvas, const GameState* gameState);

void popup_frame(Canvas* const canvas);
void draw_screen(Canvas* const canvas, const bool* points);
