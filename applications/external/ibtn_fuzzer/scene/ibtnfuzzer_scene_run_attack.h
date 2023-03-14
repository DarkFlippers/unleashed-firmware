#pragma once
#include "../ibtnfuzzer.h"

void ibtnfuzzer_scene_run_attack_on_enter(iBtnFuzzerState* context);
void ibtnfuzzer_scene_run_attack_on_exit(iBtnFuzzerState* context);
void ibtnfuzzer_scene_run_attack_on_tick(iBtnFuzzerState* context);
void ibtnfuzzer_scene_run_attack_on_event(iBtnFuzzerEvent event, iBtnFuzzerState* context);
void ibtnfuzzer_scene_run_attack_on_draw(Canvas* canvas, iBtnFuzzerState* context);
