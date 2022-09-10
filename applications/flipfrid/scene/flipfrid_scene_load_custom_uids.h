#pragma once
#include "../flipfrid.h"

void flipfrid_scene_load_custom_uids_on_enter(FlipFridState* context);
void flipfrid_scene_load_custom_uids_on_exit(FlipFridState* context);
void flipfrid_scene_load_custom_uids_on_tick(FlipFridState* context);
void flipfrid_scene_load_custom_uids_on_event(FlipFridEvent event, FlipFridState* context);
void flipfrid_scene_load_custom_uids_on_draw(Canvas* canvas, FlipFridState* context);
bool flipfrid_load_custom_uids_from_file(FlipFridState* context);