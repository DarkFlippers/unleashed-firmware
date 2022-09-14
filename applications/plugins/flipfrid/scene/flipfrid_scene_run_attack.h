#pragma once
#include "../flipfrid.h"

void flipfrid_scene_run_attack_on_enter(FlipFridState* context);
void flipfrid_scene_run_attack_on_exit(FlipFridState* context);
void flipfrid_scene_run_attack_on_tick(FlipFridState* context);
void flipfrid_scene_run_attack_on_event(FlipFridEvent event, FlipFridState* context);
void flipfrid_scene_run_attack_on_draw(Canvas* canvas, FlipFridState* context);
