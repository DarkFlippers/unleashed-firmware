#include "../subbrute.h"

void subbrute_scene_run_attack_on_enter(SubBruteState* context);
void subbrute_scene_run_attack_on_exit(SubBruteState* context);
void subbrute_scene_run_attack_on_tick(SubBruteState* context);
void subbrute_scene_run_attack_on_event(SubBruteEvent event, SubBruteState* context);
void subbrute_scene_run_attack_on_draw(Canvas* canvas, SubBruteState* context);
void send_packet();