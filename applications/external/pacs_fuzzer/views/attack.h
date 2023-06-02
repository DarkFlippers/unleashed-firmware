#pragma once

#include <gui/view.h>
#include "../helpers/fuzzer_custom_event.h"

typedef struct FuzzerViewAttack FuzzerViewAttack;

typedef void (*FuzzerViewAttackCallback)(FuzzerCustomEvent event, void* context);

void fuzzer_view_attack_set_callback(
    FuzzerViewAttack* view_attack,
    FuzzerViewAttackCallback callback,
    void* context);

FuzzerViewAttack* fuzzer_view_attack_alloc();

void fuzzer_view_attack_free(FuzzerViewAttack* view_attack);

View* fuzzer_view_attack_get_view(FuzzerViewAttack* view_attack);

void fuzzer_view_attack_reset_data(
    FuzzerViewAttack* view,
    const char* attack_name,
    const char* protocol_name,
    uint8_t uid_size);

void fuzzer_view_attack_set_uid(FuzzerViewAttack* view, const uint8_t* uid);

void fuzzer_view_attack_set_attack(FuzzerViewAttack* view, bool attack);

uint8_t fuzzer_view_attack_get_time_delay(FuzzerViewAttack* view);