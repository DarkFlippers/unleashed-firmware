#pragma once
#include "subghz_types.h"
#include "subghz_custom_event.h"

typedef enum {
    GenData,
    GenFaacSLH,
    GenKeeloq,
    GenCameAtomo,
    GenKeeloqBFT,
    GenAlutechAt4n,
    GenSomfyTelis,
    GenKingGatesStylo4k,
    GenNiceFlorS,
    GenSecPlus1,
    GenSecPlus2,
    GenPhoenixV2,
} GenType;

typedef struct {
    GenType type;
    const char* mod;
    uint32_t freq;
    union {
        struct {
            const char* name;
            uint64_t key;
            uint8_t bits;
            uint32_t te;
        } data;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint32_t cnt;
            uint32_t seed;
            const char* manuf;
        } faac_slh;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint16_t cnt;
            const char* manuf;
        } keeloq;
        struct {
            uint32_t serial;
            uint16_t cnt;
        } came_atomo;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint16_t cnt;
            uint32_t seed;
            const char* manuf;
        } keeloq_bft;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint16_t cnt;
        } alutech_at_4n;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint16_t cnt;
        } somfy_telis;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint16_t cnt;
        } kinggates_stylo_4k;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint16_t cnt;
            bool nice_one;
        } nice_flor_s;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint32_t cnt;
        } sec_plus_2;
        struct {
            uint32_t serial;
            uint16_t cnt;
        } phoenix_v2;
    };
} GenInfo;

void subghz_gen_info_reset(GenInfo* gen_info);

void subghz_scene_set_type_fill_generation_infos(GenInfo* infos_dest, SetType type);
