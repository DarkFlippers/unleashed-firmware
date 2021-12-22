/* Copyright 2015, Kenneth MacKay. Licensed under the BSD 2-clause license. */

#ifndef _UECC_VLI_H_
#define _UECC_VLI_H_

#include "uECC.h"
#include "types.h"

/* Functions for raw large-integer manipulation. These are only available
   if uECC.c is compiled with uECC_ENABLE_VLI_API defined to 1. */
#ifndef uECC_ENABLE_VLI_API
    #define uECC_ENABLE_VLI_API 0
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#if uECC_ENABLE_VLI_API

void uECC_vli_clear(uECC_word_t *vli, wordcount_t num_words);

/* Constant-time comparison to zero - secure way to compare long integers */
/* Returns 1 if vli == 0, 0 otherwise. */
uECC_word_t uECC_vli_isZero(const uECC_word_t *vli, wordcount_t num_words);

/* Returns nonzero if bit 'bit' of vli is set. */
uECC_word_t uECC_vli_testBit(const uECC_word_t *vli, bitcount_t bit);

/* Counts the number of bits required to represent vli. */
bitcount_t uECC_vli_numBits(const uECC_word_t *vli, const wordcount_t max_words);

/* Sets dest = src. */
void uECC_vli_set(uECC_word_t *dest, const uECC_word_t *src, wordcount_t num_words);

/* Constant-time comparison function - secure way to compare long integers */
/* Returns one if left == right, zero otherwise */
uECC_word_t uECC_vli_equal(const uECC_word_t *left,
                           const uECC_word_t *right,
                           wordcount_t num_words);

/* Constant-time comparison function - secure way to compare long integers */
/* Returns sign of left - right, in constant time. */
cmpresult_t uECC_vli_cmp(const uECC_word_t *left, const uECC_word_t *right, wordcount_t num_words);

/* Computes vli = vli >> 1. */
void uECC_vli_rshift1(uECC_word_t *vli, wordcount_t num_words);

/* Computes result = left + right, returning carry. Can modify in place. */
uECC_word_t uECC_vli_add(uECC_word_t *result,
                         const uECC_word_t *left,
                         const uECC_word_t *right,
                         wordcount_t num_words);

/* Computes result = left - right, returning borrow. Can modify in place. */
uECC_word_t uECC_vli_sub(uECC_word_t *result,
                         const uECC_word_t *left,
                         const uECC_word_t *right,
                         wordcount_t num_words);

/* Computes result = left * right. Result must be 2 * num_words long. */
void uECC_vli_mult(uECC_word_t *result,
                   const uECC_word_t *left,
                   const uECC_word_t *right,
                   wordcount_t num_words);

/* Computes result = left^2. Result must be 2 * num_words long. */
void uECC_vli_square(uECC_word_t *result, const uECC_word_t *left, wordcount_t num_words);

/* Computes result = (left + right) % mod.
   Assumes that left < mod and right < mod, and that result does not overlap mod. */
void uECC_vli_modAdd(uECC_word_t *result,
                     const uECC_word_t *left,
                     const uECC_word_t *right,
                     const uECC_word_t *mod,
                     wordcount_t num_words);

/* Computes result = (left - right) % mod.
   Assumes that left < mod and right < mod, and that result does not overlap mod. */
void uECC_vli_modSub(uECC_word_t *result,
                     const uECC_word_t *left,
                     const uECC_word_t *right,
                     const uECC_word_t *mod,
                     wordcount_t num_words);

/* Computes result = product % mod, where product is 2N words long.
   Currently only designed to work for mod == curve->p or curve_n. */
void uECC_vli_mmod(uECC_word_t *result,
                   uECC_word_t *product,
                   const uECC_word_t *mod,
                   wordcount_t num_words);

/* Calculates result = product (mod curve->p), where product is up to
   2 * curve->num_words long. */
void uECC_vli_mmod_fast(uECC_word_t *result, uECC_word_t *product, uECC_Curve curve);

/* Computes result = (left * right) % mod.
   Currently only designed to work for mod == curve->p or curve_n. */
void uECC_vli_modMult(uECC_word_t *result,
                      const uECC_word_t *left,
                      const uECC_word_t *right,
                      const uECC_word_t *mod,
                      wordcount_t num_words);

/* Computes result = (left * right) % curve->p. */
void uECC_vli_modMult_fast(uECC_word_t *result,
                           const uECC_word_t *left,
                           const uECC_word_t *right,
                           uECC_Curve curve);

/* Computes result = left^2 % mod.
   Currently only designed to work for mod == curve->p or curve_n. */
void uECC_vli_modSquare(uECC_word_t *result,
                        const uECC_word_t *left,
                        const uECC_word_t *mod,
                        wordcount_t num_words);

/* Computes result = left^2 % curve->p. */
void uECC_vli_modSquare_fast(uECC_word_t *result, const uECC_word_t *left, uECC_Curve curve);

/* Computes result = (1 / input) % mod.*/
void uECC_vli_modInv(uECC_word_t *result,
                     const uECC_word_t *input,
                     const uECC_word_t *mod,
                     wordcount_t num_words);

#if uECC_SUPPORT_COMPRESSED_POINT
/* Calculates a = sqrt(a) (mod curve->p) */
void uECC_vli_mod_sqrt(uECC_word_t *a, uECC_Curve curve);
#endif

/* Converts an integer in uECC native format to big-endian bytes. */
void uECC_vli_nativeToBytes(uint8_t *bytes, int num_bytes, const uECC_word_t *native);
/* Converts big-endian bytes to an integer in uECC native format. */
void uECC_vli_bytesToNative(uECC_word_t *native, const uint8_t *bytes, int num_bytes);

unsigned uECC_curve_num_words(uECC_Curve curve);
unsigned uECC_curve_num_bytes(uECC_Curve curve);
unsigned uECC_curve_num_bits(uECC_Curve curve);
unsigned uECC_curve_num_n_words(uECC_Curve curve);
unsigned uECC_curve_num_n_bytes(uECC_Curve curve);
unsigned uECC_curve_num_n_bits(uECC_Curve curve);

const uECC_word_t *uECC_curve_p(uECC_Curve curve);
const uECC_word_t *uECC_curve_n(uECC_Curve curve);
const uECC_word_t *uECC_curve_G(uECC_Curve curve);
const uECC_word_t *uECC_curve_b(uECC_Curve curve);

int uECC_valid_point(const uECC_word_t *point, uECC_Curve curve);

/* Multiplies a point by a scalar. Points are represented by the X coordinate followed by
   the Y coordinate in the same array, both coordinates are curve->num_words long. Note
   that scalar must be curve->num_n_words long (NOT curve->num_words). */
void uECC_point_mult(uECC_word_t *result,
                     const uECC_word_t *point,
                     const uECC_word_t *scalar,
                     uECC_Curve curve);

/* Generates a random integer in the range 0 < random < top.
   Both random and top have num_words words. */
int uECC_generate_random_int(uECC_word_t *random,
                             const uECC_word_t *top,
                             wordcount_t num_words);

#endif /* uECC_ENABLE_VLI_API */

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _UECC_VLI_H_ */
