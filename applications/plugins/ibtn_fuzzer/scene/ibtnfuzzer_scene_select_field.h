#pragma once
#include "../ibtnfuzzer.h"

void ibtnfuzzer_scene_select_field_on_enter(iBtnFuzzerState* context);
void ibtnfuzzer_scene_select_field_on_exit(iBtnFuzzerState* context);
void ibtnfuzzer_scene_select_field_on_tick(iBtnFuzzerState* context);
void ibtnfuzzer_scene_select_field_on_event(iBtnFuzzerEvent event, iBtnFuzzerState* context);
void ibtnfuzzer_scene_select_field_on_draw(Canvas* canvas, iBtnFuzzerState* context);
void center_displayed_key(iBtnFuzzerState* context, uint8_t index);