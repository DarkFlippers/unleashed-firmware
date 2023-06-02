#pragma once

#include <m-array.h>

struct SubGhzKeystore {
    SubGhzKeyArray_t data;
    const char* mfname;
    uint8_t kl_type;
};
