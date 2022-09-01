#pragma once
#include "../flipfrid.h"

void flipfrid_scene_select_field_on_enter(FlipFridState* context);
void flipfrid_scene_select_field_on_exit(FlipFridState* context);
void flipfrid_scene_select_field_on_tick(FlipFridState* context);
void flipfrid_scene_select_field_on_event(FlipFridEvent event, FlipFridState* context);
void flipfrid_scene_select_field_on_draw(Canvas* canvas, FlipFridState* context);
void center_displayed_key(FlipFridState* context, uint8_t index);