#pragma once
#include "../flipfrid.h"

void flipfrid_scene_load_file_on_enter(FlipFridState* context);
void flipfrid_scene_load_file_on_exit(FlipFridState* context);
void flipfrid_scene_load_file_on_tick(FlipFridState* context);
void flipfrid_scene_load_file_on_event(FlipFridEvent event, FlipFridState* context);
void flipfrid_scene_load_file_on_draw(Canvas* canvas, FlipFridState* context);
bool flipfrid_load_protocol_from_file(FlipFridState* context);