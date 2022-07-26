#pragma once
#include <stdint.h>
#include <gui/view.h>
#include <gui/modules/widget.h>

#include <lib/nfc/protocols/mifare_classic.h>

typedef struct DictAttack DictAttack;

typedef void (*DictAttackCallback)(void* context);

DictAttack* dict_attack_alloc();

void dict_attack_free(DictAttack* dict_attack);

void dict_attack_reset(DictAttack* dict_attack);

View* dict_attack_get_view(DictAttack* dict_attack);

void dict_attack_set_callback(DictAttack* dict_attack, DictAttackCallback callback, void* context);

void dict_attack_set_header(DictAttack* dict_attack, const char* header);

void dict_attack_set_card_detected(DictAttack* dict_attack, MfClassicType type);

void dict_attack_set_card_removed(DictAttack* dict_attack);

void dict_attack_set_sector_read(DictAttack* dict_attack, uint8_t sec_read);

void dict_attack_set_keys_found(DictAttack* dict_attack, uint8_t keys_found);

void dict_attack_set_current_sector(DictAttack* dict_attack, uint8_t curr_sec);

void dict_attack_inc_current_sector(DictAttack* dict_attack);

void dict_attack_inc_keys_found(DictAttack* dict_attack);
