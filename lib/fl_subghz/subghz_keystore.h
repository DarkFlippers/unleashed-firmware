#pragma once

#include <m-string.h>
#include <m-array.h>
#include <stdint.h>

typedef struct {
    string_t name;
    uint64_t key;
    uint16_t type;
} SubGhzKey;

ARRAY_DEF(SubGhzKeyArray, SubGhzKey, M_POD_OPLIST)

#define M_OPL_SubGhzKeyArray_t() ARRAY_OPLIST(SubGhzKeyArray, M_POD_OPLIST)

typedef struct SubGhzKeystore SubGhzKeystore;

SubGhzKeystore* subghz_keystore_alloc();

void subghz_keystore_free(SubGhzKeystore* instance);

void subghz_keystore_load(SubGhzKeystore* instance, const char* filename);

SubGhzKeyArray_t* subghz_keystore_get_data(SubGhzKeystore* instance);
