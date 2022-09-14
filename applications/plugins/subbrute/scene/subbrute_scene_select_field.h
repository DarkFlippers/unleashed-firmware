#include "../subbrute.h"

void subbrute_scene_select_field_on_enter(SubBruteState* context);
void subbrute_scene_select_field_on_exit(SubBruteState* context);
void subbrute_scene_select_field_on_tick(SubBruteState* context);
void subbrute_scene_select_field_on_event(SubBruteEvent event, SubBruteState* context);
void subbrute_scene_select_field_on_draw(Canvas* canvas, SubBruteState* context);
void center_displayed_key(SubBruteState* context, uint8_t index);