/**
 * @file strint.h
 * Performs conversions between strings and integers.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** String to integer conversion error */
typedef enum {
    StrintParseNoError, //!< Conversion performed successfully
    StrintParseSignError, //!< Multiple leading `+` or `-` characters, or leading `-` character if the type is unsigned
    StrintParseAbsentError, //!< No valid digits after the leading whitespace, sign and prefix
    StrintParseOverflowError, //!< Result does not fit in the requested type
} StrintParseError;

/** See `strint_to_uint32` */
StrintParseError strint_to_uint64(const char* str, char** end, uint64_t* out, uint8_t base);

/** See `strint_to_uint32` */
StrintParseError strint_to_int64(const char* str, char** end, int64_t* out, uint8_t base);

/** Converts a string to a `uint32_t`
 *
 * @param[in]  str   Input string
 * @param[out] end   Pointer to first character after the number in input string
 * @param[out] out   Parse result
 * @param[in]  base  Integer base
 *
 * @return     Parse error
 *
 * Parses the number in the input string. The number may be surrounded by
 * whitespace characters to the left and any non-digit characters to the right.
 * What's considered a digit is determined by the input base in the following
 * order: `0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ`. The number may be prefixed
 * with either a `+` or a `-` to indicate its sign. The pointer to the first
 * character after the leading whitespace, allowed prefixes and digits is
 * assigned to `end`.
 *
 * If the input base is 0, the base is inferred from the leading characters of
 * the number:
 *   - If it starts with `0x`, it's read in base 16;
 *   - If it starts with a `0`, it's read in base 8;
 *   - If it starts with `0b`, it's read in base 2.
 *   - Otherwise, it's read in base 10.
 *
 * For a description of the return codes, see `StrintParseError`. If the return
 * code is something other than `StrintParseNoError`, the values at `end` and
 * `out` are unaltered.
 */
StrintParseError strint_to_uint32(const char* str, char** end, uint32_t* out, uint8_t base);

/** See `strint_to_uint32` */
StrintParseError strint_to_int32(const char* str, char** end, int32_t* out, uint8_t base);

/** See `strint_to_uint32` */
StrintParseError strint_to_uint16(const char* str, char** end, uint16_t* out, uint8_t base);

/** See `strint_to_uint32` */
StrintParseError strint_to_int16(const char* str, char** end, int16_t* out, uint8_t base);

#ifdef __cplusplus
}
#endif
