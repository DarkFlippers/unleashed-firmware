#pragma once
#include "../ibtnfuzzer.h"

void ibtnfuzzer_scene_entrypoint_on_enter(iBtnFuzzerState* context);
void ibtnfuzzer_scene_entrypoint_on_exit(iBtnFuzzerState* context);
void ibtnfuzzer_scene_entrypoint_on_tick(iBtnFuzzerState* context);
void ibtnfuzzer_scene_entrypoint_on_event(iBtnFuzzerEvent event, iBtnFuzzerState* context);
void ibtnfuzzer_scene_entrypoint_on_draw(Canvas* canvas, iBtnFuzzerState* context);