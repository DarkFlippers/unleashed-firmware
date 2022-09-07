#pragma once
#include "../subbrute.h"

void subbrute_scene_entrypoint_on_enter(SubBruteState* context);
void subbrute_scene_entrypoint_on_exit(SubBruteState* context);
void subbrute_scene_entrypoint_on_tick(SubBruteState* context);
void subbrute_scene_entrypoint_on_event(SubBruteEvent event, SubBruteState* context);
void subbrute_scene_entrypoint_on_draw(Canvas* canvas, SubBruteState* context);