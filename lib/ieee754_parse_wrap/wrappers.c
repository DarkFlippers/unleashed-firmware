#include "wrappers.h"

#include <furi/core/core_defines.h>

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

// newlib's strtod carries arbitrary-precision machinery (_Balloc, __gethex) for
// hex-float literals, inf/nan spellings and correct rounding of long mantissas -
// ~8 KiB of flash. Everything that reaches here is a plain decimal literal: file
// fields (flipper_format), CLI arguments (toolbox/args), and JS/JSON numbers
// (mjs, which routes 0x/0X to strtoul first). Also exported to apps, so the
// contract below is public: decimal only, and up to 4 ulp rather than correctly
// rounded. strtof is unaffected - the double it narrows from is always exact
// enough to round to the same float.

// The most decimal digits that always fit a uint64_t (UINT64_MAX has 20, so the
// 20th can overflow). A double only carries ~17 anyway, hence the slack
#define STRTOD_MANTISSA_DIGITS_MAX 19

// 1e1, 1e2, 1e4 ... 1e256, as raw doubles: -fsingle-precision-constant truncates
// an unsuffixed literal to float, and leaning on long double == double to escape
// that is a target assumption this file should not make
static const uint64_t strtod_pow10_bits[] = {
    0x4024000000000000ULL,
    0x4059000000000000ULL,
    0x40C3880000000000ULL,
    0x4197D78400000000ULL,
    0x4341C37937E08000ULL,
    0x4693B8B5B5056E17ULL,
    0x4D384F03E93FF9F5ULL,
    0x5A827748F9301D32ULL,
    0x75154FDD7F73BF3CULL,
};

// The table's exponents sum to 511, and a mantissa this size times 1e511 is inf
// while divided by it is 0 - which is what the clamped-away inputs produce anyway
#define STRTOD_EXP_MAX ((1 << COUNT_OF(strtod_pow10_bits)) - 1)

static double strtod_scale_pow10(double value, int32_t exp) {
    const bool negative_exp = exp < 0;
    if(negative_exp) exp = -exp;
    if(exp > STRTOD_EXP_MAX) exp = STRTOD_EXP_MAX;

    // Ascending, so intermediates move monotonically towards the result and
    // cannot overflow or underflow on the way to a value that is in range.
    // The clamp above is what bounds this: exp is below 2^COUNT_OF(table)
    for(const uint64_t* bits = strtod_pow10_bits; exp; exp >>= 1, bits++) {
        if(!(exp & 1)) continue;

        double power;
        __builtin_memcpy(&power, bits, sizeof(power));
        value = negative_exp ? value / power : value * power;
    }

    return value;
}

static double strtod_parse(const char* in, char** tail) {
    const char* p = in;
    while(isspace((unsigned char)*p))
        p++;

    bool negative = false;
    if(*p == '+' || *p == '-') negative = (*p++ == '-');

    uint64_t mantissa = 0;
    int32_t exp10 = 0;
    uint8_t digits = 0;
    bool has_digits = false;

    // One loop for both sides of the point: a digit's only side-dependent
    // effect is which way it moves exp10
    bool fraction = false;
    for(;; p++) {
        if(*p == '.' && !fraction) {
            fraction = true;
            continue;
        }
        if(*p < '0' || *p > '9') break;
        has_digits = true;

        // A leading zero carries no significance; charging it to the budget
        // would spend it before the digits that matter ("000...0001" is 1).
        // It still moves the point on the fraction side ("0.001")
        if(mantissa == 0 && *p == '0') {
            exp10 -= fraction;
        } else if(digits < STRTOD_MANTISSA_DIGITS_MAX) {
            mantissa = mantissa * 10 + (uint64_t)(*p - '0');
            digits++;
            exp10 -= fraction;
        } else {
            // Past the budget an integer digit only scales what we already
            // have, and a fraction digit changes nothing at all
            exp10 += !fraction;
        }
    }

    if(!has_digits) {
        // C requires the original pointer on no conversion, not the one past
        // the whitespace - args.c compares against it to detect failure
        if(tail) *tail = (char*)in;
        return 0;
    }

    if(*p == 'e' || *p == 'E') {
        const char* exp_start = p++;

        bool negative_exp = false;
        if(*p == '+' || *p == '-') negative_exp = (*p++ == '-');

        const char* exp_digits = p;
        int32_t exp = 0;
        for(; *p >= '0' && *p <= '9'; p++) {
            // Stop accumulating once it is certainly past the clamp, but keep
            // consuming digits so the tail lands after all of them
            if(exp <= STRTOD_EXP_MAX) exp = exp * 10 + (*p - '0');
        }

        if(p == exp_digits) {
            // Not an exponent after all, leave the 'e' to the caller
            p = exp_start;
        } else {
            exp10 += negative_exp ? -exp : exp;
        }
    }

    if(tail) *tail = (char*)p;

    // exp10 == 0 skips the loop entirely, so a plain integer stays exact to 2^53
    const double value = strtod_scale_pow10((double)mantissa, exp10);

    // newlib reported range errors this way and args_read_duration still checks.
    // Reading the bits rather than comparing doubles keeps four softfloat calls
    // out of every conversion; value is never negative, so there is no sign bit
    uint64_t bits;
    __builtin_memcpy(&bits, &value, sizeof(bits));
    if(mantissa != 0 && (bits == 0 || bits >= 0x7FF0000000000000ULL)) errno = ERANGE;

    return negative ? -value : value;
}

float __wrap_strtof(const char* in, char** tail) {
    return (float)strtod_parse(in, tail);
}

double __wrap_strtod(const char* in, char** tail) {
    return strtod_parse(in, tail);
}
