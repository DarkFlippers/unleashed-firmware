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

#ifndef FURI_CONST_ASSIGN
#define FURI_CONST_ASSIGN_(T, x, y) \
    ({                              \
        T* tmp_x = (T*)&x;          \
        *tmp_x = y;                 \
        *tmp_x;                     \
    })
#define FURI_CONST_ASSIGN(x, y)                                        \
    _Generic((x), signed char                                          \
             : FURI_CONST_ASSIGN_(signed char, x, y), unsigned char    \
             : FURI_CONST_ASSIGN_(unsigned char, x, y), short          \
             : FURI_CONST_ASSIGN_(short, x, y), unsigned short         \
             : FURI_CONST_ASSIGN_(unsigned short, x, y), int           \
             : FURI_CONST_ASSIGN_(int, x, y), unsigned                 \
             : FURI_CONST_ASSIGN_(unsigned, x, y), long                \
             : FURI_CONST_ASSIGN_(long, x, y), unsigned long           \
             : FURI_CONST_ASSIGN_(unsigned long, x, y), long long      \
             : FURI_CONST_ASSIGN_(long long, x, y), unsigned long long \
             : FURI_CONST_ASSIGN_(unsigned long long, x, y), float     \
             : FURI_CONST_ASSIGN_(float, x, y), double                 \
             : FURI_CONST_ASSIGN_(double, x, y), long double           \
             : FURI_CONST_ASSIGN_(long double, x, y))
#endif
