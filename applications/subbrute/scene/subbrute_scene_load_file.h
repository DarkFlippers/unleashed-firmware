#include "../subbrute.h"

void subbrute_scene_load_file_on_enter(SubBruteState* context);
void subbrute_scene_load_file_on_exit(SubBruteState* context);
void subbrute_scene_load_file_on_tick(SubBruteState* context);
void subbrute_scene_load_file_on_event(SubBruteEvent event, SubBruteState* context);
void subbrute_scene_load_file_on_draw(Canvas* canvas, SubBruteState* context);
bool subbrute_load_protocol_from_file(SubBruteState* context);