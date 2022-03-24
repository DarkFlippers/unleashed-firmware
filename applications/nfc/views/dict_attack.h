#pragma once
#include <stdint.h>
#include <gui/view.h>
#include <gui/modules/widget.h>

#include <lib/nfc_protocols/mifare_classic.h>

typedef struct DictAttack DictAttack;

typedef void (*DictAttackResultCallback)(void* context);

DictAttack* dict_attack_alloc();

void dict_attack_free(DictAttack* dict_attack);

void dict_attack_reset(DictAttack* dict_attack);

View* dict_attack_get_view(DictAttack* dict_attack);

void dict_attack_set_result_callback(
    DictAttack* dict_attack,
    DictAttackResultCallback callback,
    void* context);

void dict_attack_card_detected(DictAttack* dict_attack, MfClassicType type);

void dict_attack_card_removed(DictAttack* dict_attack);

void dict_attack_inc_curr_sector(DictAttack* dict_attack);

void dict_attack_inc_found_key(DictAttack* dict_attack, MfClassicKey key);

void dict_attack_set_result(DictAttack* dict_attack, bool success);
