#pragma once

#include "../subbrute_custom_event.h"
#include <gui/view.h>
#include <input/input.h>
#include <gui/elements.h>

typedef void (*SubBruteMainViewCallback)(SubBruteCustomEvent event, void* context);
typedef struct SubBruteMainView SubBruteMainView;

void subbrute_main_view_set_callback(
    SubBruteMainView* instance,
    SubBruteMainViewCallback callback,
    void* context);

SubBruteMainView* subbrute_main_view_alloc();
void subbrute_main_view_free(SubBruteMainView* instance);
View* subbrute_main_view_get_view(SubBruteMainView* instance);
void subbrute_main_view_set_index(
    SubBruteMainView* instance,
    uint8_t idx,
    bool is_select_byte,
    const char* key_field);
uint8_t subbrute_main_view_get_index(SubBruteMainView* instance);
void subbrute_attack_view_enter(void* context);
void subbrute_attack_view_exit(void* context);
bool subbrute_attack_view_input(InputEvent* event, void* context);
void subbrute_attack_view_draw(Canvas* canvas, void* context);