#pragma once

#include <gui/view.h>
#include "assets_icons.h"
#include <input/input.h>
#include <gui/elements.h>
#include <gui/icon.h>
#include <subghz/types.h>
#include "../subbrute_custom_event.h"

typedef void (*SubBruteAttackViewCallback)(SubBruteCustomEvent event, void* context);
typedef struct SubBruteAttackView SubBruteAttackView;

void subbrute_attack_view_set_callback(
    SubBruteAttackView* instance,
    SubBruteAttackViewCallback callback,
    void* context);
SubBruteAttackView* subbrute_attack_view_alloc();
void subbrute_attack_view_free(SubBruteAttackView* instance);
View* subbrute_attack_view_get_view(SubBruteAttackView* instance);
void subbrute_attack_view_set_current_step(SubBruteAttackView* instance, uint64_t current_step);
void subbrute_attack_view_set_worker_type(SubBruteAttackView* instance, bool is_continuous_worker);
void subbrute_attack_view_init_values(
    SubBruteAttackView* instance,
    uint8_t index,
    uint64_t max_value,
    uint64_t current_step,
    bool is_attacking);