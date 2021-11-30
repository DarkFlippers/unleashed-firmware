#pragma once

#ifndef MAX

#define MAX(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })
#endif

#ifndef MIN
#define MIN(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
#endif

#ifndef ROUND_UP_TO
#define ROUND_UP_TO(a, b)       \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a / _b + !!(_a % _b);  \
    })
#endif

#ifndef CLAMP
#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
#endif

// need some common semantics for those two
#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#ifndef COUNT_OF
#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef FURI_SWAP
#define FURI_SWAP(x, y)     \
    do {                    \
        typeof(x) SWAP = x; \
        x = y;              \
        y = SWAP;           \
    } while(0)
#endif

#ifndef PLACE_IN_SECTION
#define PLACE_IN_SECTION(x) __attribute__((section(x)))
#endif

#ifndef ALIGN
#define ALIGN(n) __attribute__((aligned(n)))
#endif

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif

#ifndef TOSTRING
#define TOSTRING(x) STRINGIFY(x)
#endif

#ifndef REVERSE_BYTES_U32
#define REVERSE_BYTES_U32(x)                                                        \
    ((((x)&0x000000FF) << 24) | (((x)&0x0000FF00) << 8) | (((x)&0x00FF0000) >> 8) | \
     (((x)&0xFF000000) >> 24))
#endif

#ifndef FURI_CRITICAL_ENTER
#define FURI_CRITICAL_ENTER()               \
    uint32_t primask_bit = __get_PRIMASK(); \
    __disable_irq()
#endif

#ifndef FURI_CRITICAL_EXIT
#define FURI_CRITICAL_EXIT() __set_PRIMASK(primask_bit)
#endif