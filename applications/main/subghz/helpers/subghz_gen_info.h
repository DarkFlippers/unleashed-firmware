#pragma once
#include "subghz_types.h"
#include "subghz_custom_event.h"

typedef enum {
    GenData,
    GenFaacSLH,
    GenKeeloq,
    GenCameAtomo,
    GenKeeloqSeed,
    GenAlutechAt4n,
    GenSomfyTelis,
    GenSomfyKeytis,
    GenKingGatesStylo4k,
    GenBenincaARC,
    GenJarolift,
    GenDitecGOL4,
    GenNiceFlorS,
    GenSecPlus1,
    GenSecPlus2,
    GenPhoenixV2,
    GenPrastel,
    GenBelKorn,
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
            /* te != 0 means the protocol carries one and Advanced asks for it, within
               te_min..te_max - the window that protocol's own loader or decoder accepts. */
            uint32_t te;
            uint16_t te_min;
            uint16_t te_max;
            /* gt != 0 means the protocol carries a guard time - the gap that closes the frame,
               counted in te - and Advanced asks for it, within gt_min..gt_max. */
            uint32_t gt;
            uint16_t gt_min;
            uint16_t gt_max;
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
        } keeloq_seed;
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
        } somfy_keytis;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint16_t cnt;
        } kinggates_stylo_4k;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint32_t cnt;
        } beninca_arc;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint16_t cnt;
        } jarolift;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint16_t cnt;
            uint16_t ic;
            /* Receiver certificate, big endian so byte_input can write it in place.
               cert_wanted marks the variant that asks for one. */
            uint8_t cert[3];
            bool cert_wanted;
            bool nice_one;
            bool o_code;
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
        struct {
            uint32_t serial;
            uint8_t btn;
            uint16_t cnt;
        } ditec_gol4;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint16_t cnt;
        } prastel;
        struct {
            /* 36 bit serial (frame bits 12..47), big endian so byte_input can write it in
               place; only the low 36 bits of the 5 bytes are used */
            uint8_t serial[5];
            uint8_t btn;
            uint16_t cnt;
        } belkorn;
    };
} GenInfo;

/* Both live in the Add Manually plugin. The struct stays app-side because byte_input holds a
 * pointer into it and draws through it; its mod/name/manuf are literals inside the plugin, so its
 * contents do not survive an unmap - entering the flow again refills them. */
void subghz_gen_info_reset(GenInfo* gen_info);

void subghz_scene_set_type_fill_generation_infos(GenInfo* infos_dest, SetType type);
