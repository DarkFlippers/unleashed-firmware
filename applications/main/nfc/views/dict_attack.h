#pragma once

#include <stdint.h>
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DictAttack DictAttack;

typedef enum {
    MfClassicNestedPhaseNone,
    MfClassicNestedPhaseAnalyzePRNG,
    MfClassicNestedPhaseDictAttack,
    MfClassicNestedPhaseDictAttackResume,
    MfClassicNestedPhaseCalibrate,
    MfClassicNestedPhaseCollectNtEnc,
    MfClassicNestedPhaseFinished,
} MfClassicNestedPhase;

typedef enum {
    MfClassicPrngTypeUnknown, // Tag not yet tested
    MfClassicPrngTypeNoTag, // No tag detected during test
    MfClassicPrngTypeWeak, // Weak PRNG, standard Nested
    MfClassicPrngTypeHard, // Hard PRNG, Hardnested
} MfClassicPrngType;

typedef enum {
    MfClassicBackdoorUnknown, // Tag not yet tested
    MfClassicBackdoorNone, // No observed backdoor
    MfClassicBackdoorAuth1, // Tag responds to v1 auth backdoor
    MfClassicBackdoorAuth2, // Tag responds to v2 auth backdoor
    MfClassicBackdoorAuth3, // Tag responds to v3 auth backdoor (static encrypted nonce)
} MfClassicBackdoor;

typedef enum {
    DictAttackEventSkipPressed,
} DictAttackEvent;

typedef void (*DictAttackCallback)(DictAttackEvent event, void* context);

DictAttack* dict_attack_alloc(void);

void dict_attack_free(DictAttack* instance);

void dict_attack_reset(DictAttack* instance);

View* dict_attack_get_view(DictAttack* instance);

void dict_attack_set_callback(DictAttack* instance, DictAttackCallback callback, void* context);

void dict_attack_set_header(DictAttack* instance, const char* header);

void dict_attack_set_card_state(DictAttack* instance, bool detected);

void dict_attack_set_sectors_total(DictAttack* instance, uint8_t sectors_total);

void dict_attack_set_sectors_read(DictAttack* instance, uint8_t sectors_read);

void dict_attack_set_keys_found(DictAttack* instance, uint8_t keys_found);

void dict_attack_set_current_sector(DictAttack* instance, uint8_t curr_sec);

void dict_attack_set_total_dict_keys(DictAttack* instance, size_t dict_keys_total);

void dict_attack_set_current_dict_key(DictAttack* instance, size_t cur_key_num);

void dict_attack_set_key_attack(DictAttack* instance, uint8_t sector);

void dict_attack_reset_key_attack(DictAttack* instance);

void dict_attack_set_nested_phase(DictAttack* instance, uint8_t nested_phase);

void dict_attack_set_prng_type(DictAttack* instance, uint8_t prng_type);

void dict_attack_set_backdoor(DictAttack* instance, uint8_t backdoor);

#ifdef __cplusplus
}
#endif
