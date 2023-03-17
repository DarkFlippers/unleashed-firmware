#pragma once

#include <stdint.h>
#include <stdbool.h>

#define TOTP_NULLABLE_STRUCT(value_type)       \
    typedef struct TotpNullable_##value_type { \
        bool is_null;                          \
        value_type value;                      \
    } TotpNullable_##value_type

#define TOTP_NULLABLE_NULL(s) s.is_null = true
#define TOTP_NULLABLE_VALUE(s, v) \
    s.is_null = false;            \
    s.value = v

TOTP_NULLABLE_STRUCT(uint16_t);
