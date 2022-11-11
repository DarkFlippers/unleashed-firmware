#pragma once
#include "../ibtnfuzzer.h"

void ibtnfuzzer_scene_load_file_on_enter(iBtnFuzzerState* context);
void ibtnfuzzer_scene_load_file_on_exit(iBtnFuzzerState* context);
void ibtnfuzzer_scene_load_file_on_tick(iBtnFuzzerState* context);
void ibtnfuzzer_scene_load_file_on_event(iBtnFuzzerEvent event, iBtnFuzzerState* context);
void ibtnfuzzer_scene_load_file_on_draw(Canvas* canvas, iBtnFuzzerState* context);
bool ibtnfuzzer_load_protocol_from_file(iBtnFuzzerState* context);