#pragma once

#include <gui/view.h>

#include "../helpers/fuzzer_custom_event.h"
#include "../helpers/fuzzer_types.h"

#include "../lib/worker/protocol.h"

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
    const char* protocol_name);

void fuzzer_view_attack_set_uid(FuzzerViewAttack* view, const FuzzerPayload* uid);

void fuzzer_view_attack_start(FuzzerViewAttack* view);

void fuzzer_view_attack_stop(FuzzerViewAttack* view);

void fuzzer_view_attack_pause(FuzzerViewAttack* view);

void fuzzer_view_attack_end(FuzzerViewAttack* view);

uint8_t fuzzer_view_attack_get_time_delay(FuzzerViewAttack* view);

uint8_t fuzzer_view_attack_get_emu_time(FuzzerViewAttack* view);