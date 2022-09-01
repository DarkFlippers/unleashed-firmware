#pragma once
#include "../flipfrid.h"

void flipfrid_scene_entrypoint_on_enter(FlipFridState* context);
void flipfrid_scene_entrypoint_on_exit(FlipFridState* context);
void flipfrid_scene_entrypoint_on_tick(FlipFridState* context);
void flipfrid_scene_entrypoint_on_event(FlipFridEvent event, FlipFridState* context);
void flipfrid_scene_entrypoint_on_draw(Canvas* canvas, FlipFridState* context);