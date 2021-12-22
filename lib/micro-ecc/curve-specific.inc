/* Copyright 2015, Kenneth MacKay. Licensed under the BSD 2-clause license. */

#ifndef _UECC_CURVE_SPECIFIC_H_
#define _UECC_CURVE_SPECIFIC_H_

#define num_bytes_secp160r1 20
#define num_bytes_secp192r1 24
#define num_bytes_secp224r1 28
#define num_bytes_secp256r1 32
#define num_bytes_secp256k1 32

#if (uECC_WORD_SIZE == 1)

#define num_words_secp160r1 20
#define num_words_secp192r1 24
#define num_words_secp224r1 28
#define num_words_secp256r1 32
#define num_words_secp256k1 32

#define BYTES_TO_WORDS_8(a, b, c, d, e, f, g, h) \
    0x##a, 0x##b, 0x##c, 0x##d, 0x##e, 0x##f, 0x##g, 0x##h
#define BYTES_TO_WORDS_4(a, b, c, d) 0x##a, 0x##b, 0x##c, 0x##d

#elif (uECC_WORD_SIZE == 4)

#define num_words_secp160r1 5
#define num_words_secp192r1 6
#define num_words_secp224r1 7
#define num_words_secp256r1 8
#define num_words_secp256k1 8

#define BYTES_TO_WORDS_8(a, b, c, d, e, f, g, h) 0x##d##c##b##a, 0x##h##g##f##e
#define BYTES_TO_WORDS_4(a, b, c, d) 0x##d##c##b##a

#elif (uECC_WORD_SIZE == 8)

#define num_words_secp160r1 3
#define num_words_secp192r1 3
#define num_words_secp224r1 4
#define num_words_secp256r1 4
#define num_words_secp256k1 4

#define BYTES_TO_WORDS_8(a, b, c, d, e, f, g, h) 0x##h##g##f##e##d##c##b##a##ull
#define BYTES_TO_WORDS_4(a, b, c, d) 0x##d##c##b##a##ull

#endif /* uECC_WORD_SIZE */

#if uECC_SUPPORTS_secp160r1 || uECC_SUPPORTS_secp192r1 || \
    uECC_SUPPORTS_secp224r1 || uECC_SUPPORTS_secp256r1
static void double_jacobian_default(uECC_word_t * X1,
                                    uECC_word_t * Y1,
                                    uECC_word_t * Z1,
                                    uECC_Curve curve) {
    /* t1 = X, t2 = Y, t3 = Z */
    uECC_word_t t4[uECC_MAX_WORDS];
    uECC_word_t t5[uECC_MAX_WORDS];
    wordcount_t num_words = curve->num_words;

    if (uECC_vli_isZero(Z1, num_words)) {
        return;
    }

    uECC_vli_modSquare_fast(t4, Y1, curve);   /* t4 = y1^2 */
    uECC_vli_modMult_fast(t5, X1, t4, curve); /* t5 = x1*y1^2 = A */
    uECC_vli_modSquare_fast(t4, t4, curve);   /* t4 = y1^4 */
    uECC_vli_modMult_fast(Y1, Y1, Z1, curve); /* t2 = y1*z1 = z3 */
    uECC_vli_modSquare_fast(Z1, Z1, curve);   /* t3 = z1^2 */

    uECC_vli_modAdd(X1, X1, Z1, curve->p, num_words); /* t1 = x1 + z1^2 */
    uECC_vli_modAdd(Z1, Z1, Z1, curve->p, num_words); /* t3 = 2*z1^2 */
    uECC_vli_modSub(Z1, X1, Z1, curve->p, num_words); /* t3 = x1 - z1^2 */
    uECC_vli_modMult_fast(X1, X1, Z1, curve);                /* t1 = x1^2 - z1^4 */

    uECC_vli_modAdd(Z1, X1, X1, curve->p, num_words); /* t3 = 2*(x1^2 - z1^4) */
    uECC_vli_modAdd(X1, X1, Z1, curve->p, num_words); /* t1 = 3*(x1^2 - z1^4) */
    if (uECC_vli_testBit(X1, 0)) {
        uECC_word_t l_carry = uECC_vli_add(X1, X1, curve->p, num_words);
        uECC_vli_rshift1(X1, num_words);
        X1[num_words - 1] |= l_carry << (uECC_WORD_BITS - 1);
    } else {
        uECC_vli_rshift1(X1, num_words);
    }
    /* t1 = 3/2*(x1^2 - z1^4) = B */

    uECC_vli_modSquare_fast(Z1, X1, curve);                  /* t3 = B^2 */
    uECC_vli_modSub(Z1, Z1, t5, curve->p, num_words); /* t3 = B^2 - A */
    uECC_vli_modSub(Z1, Z1, t5, curve->p, num_words); /* t3 = B^2 - 2A = x3 */
    uECC_vli_modSub(t5, t5, Z1, curve->p, num_words); /* t5 = A - x3 */
    uECC_vli_modMult_fast(X1, X1, t5, curve);                /* t1 = B * (A - x3) */
    uECC_vli_modSub(t4, X1, t4, curve->p, num_words); /* t4 = B * (A - x3) - y1^4 = y3 */

    uECC_vli_set(X1, Z1, num_words);
    uECC_vli_set(Z1, Y1, num_words);
    uECC_vli_set(Y1, t4, num_words);
}

/* Computes result = x^3 + ax + b. result must not overlap x. */
static void x_side_default(uECC_word_t *result, const uECC_word_t *x, uECC_Curve curve) {
    uECC_word_t _3[uECC_MAX_WORDS] = {3}; /* -a = 3 */
    wordcount_t num_words = curve->num_words;

    uECC_vli_modSquare_fast(result, x, curve);                             /* r = x^2 */
    uECC_vli_modSub(result, result, _3, curve->p, num_words);       /* r = x^2 - 3 */
    uECC_vli_modMult_fast(result, result, x, curve);                       /* r = x^3 - 3x */
    uECC_vli_modAdd(result, result, curve->b, curve->p, num_words); /* r = x^3 - 3x + b */
}
#endif /* uECC_SUPPORTS_secp... */

#if uECC_SUPPORT_COMPRESSED_POINT
#if uECC_SUPPORTS_secp160r1 || uECC_SUPPORTS_secp192r1 || \
    uECC_SUPPORTS_secp256r1 || uECC_SUPPORTS_secp256k1
/* Compute a = sqrt(a) (mod curve_p). */
static void mod_sqrt_default(uECC_word_t *a, uECC_Curve curve) {
    bitcount_t i;
    uECC_word_t p1[uECC_MAX_WORDS] = {1};
    uECC_word_t l_result[uECC_MAX_WORDS] = {1};
    wordcount_t num_words = curve->num_words;
    
    /* When curve->p == 3 (mod 4), we can compute
       sqrt(a) = a^((curve->p + 1) / 4) (mod curve->p). */
    uECC_vli_add(p1, curve->p, p1, num_words); /* p1 = curve_p + 1 */
    for (i = uECC_vli_numBits(p1, num_words) - 1; i > 1; --i) {
        uECC_vli_modSquare_fast(l_result, l_result, curve);
        if (uECC_vli_testBit(p1, i)) {
            uECC_vli_modMult_fast(l_result, l_result, a, curve);
        }
    }
    uECC_vli_set(a, l_result, num_words);
}
#endif /* uECC_SUPPORTS_secp... */
#endif /* uECC_SUPPORT_COMPRESSED_POINT */

#if uECC_SUPPORTS_secp160r1

#if (uECC_OPTIMIZATION_LEVEL > 0)
static void vli_mmod_fast_secp160r1(uECC_word_t *result, uECC_word_t *product);
#endif

static const struct uECC_Curve_t curve_secp160r1 = {
    num_words_secp160r1,
    num_bytes_secp160r1,
    161, /* num_n_bits */
    { BYTES_TO_WORDS_8(FF, FF, FF, 7F, FF, FF, FF, FF),
        BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF),
        BYTES_TO_WORDS_4(FF, FF, FF, FF) },
    { BYTES_TO_WORDS_8(57, 22, 75, CA, D3, AE, 27, F9),
        BYTES_TO_WORDS_8(C8, F4, 01, 00, 00, 00, 00, 00),
        BYTES_TO_WORDS_8(00, 00, 00, 00, 01, 00, 00, 00) },
    { BYTES_TO_WORDS_8(82, FC, CB, 13, B9, 8B, C3, 68),
        BYTES_TO_WORDS_8(89, 69, 64, 46, 28, 73, F5, 8E),
        BYTES_TO_WORDS_4(68, B5, 96, 4A),

        BYTES_TO_WORDS_8(32, FB, C5, 7A, 37, 51, 23, 04),
        BYTES_TO_WORDS_8(12, C9, DC, 59, 7D, 94, 68, 31),
        BYTES_TO_WORDS_4(55, 28, A6, 23) },
    { BYTES_TO_WORDS_8(45, FA, 65, C5, AD, D4, D4, 81),
        BYTES_TO_WORDS_8(9F, F8, AC, 65, 8B, 7A, BD, 54),
        BYTES_TO_WORDS_4(FC, BE, 97, 1C) },
    &double_jacobian_default,
#if uECC_SUPPORT_COMPRESSED_POINT
    &mod_sqrt_default,
#endif
    &x_side_default,
#if (uECC_OPTIMIZATION_LEVEL > 0)
    &vli_mmod_fast_secp160r1
#endif
};

uECC_Curve uECC_secp160r1(void) { return &curve_secp160r1; }

#if (uECC_OPTIMIZATION_LEVEL > 0 && !asm_mmod_fast_secp160r1)
/* Computes result = product % curve_p
    see http://www.isys.uni-klu.ac.at/PDF/2001-0126-MT.pdf page 354
    
    Note that this only works if log2(omega) < log2(p) / 2 */
static void omega_mult_secp160r1(uECC_word_t *result, const uECC_word_t *right);
#if uECC_WORD_SIZE == 8
static void vli_mmod_fast_secp160r1(uECC_word_t *result, uECC_word_t *product) {
    uECC_word_t tmp[2 * num_words_secp160r1];
    uECC_word_t copy;
    
    uECC_vli_clear(tmp, num_words_secp160r1);
    uECC_vli_clear(tmp + num_words_secp160r1, num_words_secp160r1);

    omega_mult_secp160r1(tmp, product + num_words_secp160r1 - 1); /* (Rq, q) = q * c */
    
    product[num_words_secp160r1 - 1] &= 0xffffffff;
    copy = tmp[num_words_secp160r1 - 1];
    tmp[num_words_secp160r1 - 1] &= 0xffffffff;
    uECC_vli_add(result, product, tmp, num_words_secp160r1); /* (C, r) = r + q */
    uECC_vli_clear(product, num_words_secp160r1);
    tmp[num_words_secp160r1 - 1] = copy;
    omega_mult_secp160r1(product, tmp + num_words_secp160r1 - 1); /* Rq*c */
    uECC_vli_add(result, result, product, num_words_secp160r1); /* (C1, r) = r + Rq*c */

    while (uECC_vli_cmp_unsafe(result, curve_secp160r1.p, num_words_secp160r1) > 0) {
        uECC_vli_sub(result, result, curve_secp160r1.p, num_words_secp160r1);
    }
}

static void omega_mult_secp160r1(uint64_t *result, const uint64_t *right) {
    uint32_t carry;
    unsigned i;
    
    /* Multiply by (2^31 + 1). */
    carry = 0;
    for (i = 0; i < num_words_secp160r1; ++i) {
        uint64_t tmp = (right[i] >> 32) | (right[i + 1] << 32);
        result[i] = (tmp << 31) + tmp + carry;
        carry = (tmp >> 33) + (result[i] < tmp || (carry && result[i] == tmp));
    }
    result[i] = carry;
}
#else
static void vli_mmod_fast_secp160r1(uECC_word_t *result, uECC_word_t *product) {
    uECC_word_t tmp[2 * num_words_secp160r1];
    uECC_word_t carry;
    
    uECC_vli_clear(tmp, num_words_secp160r1);
    uECC_vli_clear(tmp + num_words_secp160r1, num_words_secp160r1);

    omega_mult_secp160r1(tmp, product + num_words_secp160r1); /* (Rq, q) = q * c */
    
    carry = uECC_vli_add(result, product, tmp, num_words_secp160r1); /* (C, r) = r + q */
    uECC_vli_clear(product, num_words_secp160r1);
    omega_mult_secp160r1(product, tmp + num_words_secp160r1); /* Rq*c */
    carry += uECC_vli_add(result, result, product, num_words_secp160r1); /* (C1, r) = r + Rq*c */

    while (carry > 0) {
        --carry;
        uECC_vli_sub(result, result, curve_secp160r1.p, num_words_secp160r1);
    }
    if (uECC_vli_cmp_unsafe(result, curve_secp160r1.p, num_words_secp160r1) > 0) {
        uECC_vli_sub(result, result, curve_secp160r1.p, num_words_secp160r1);
    }
}
#endif

#if uECC_WORD_SIZE == 1
static void omega_mult_secp160r1(uint8_t *result, const uint8_t *right) {
    uint8_t carry;
    uint8_t i;
    
    /* Multiply by (2^31 + 1). */
    uECC_vli_set(result + 4, right, num_words_secp160r1); /* 2^32 */
    uECC_vli_rshift1(result + 4, num_words_secp160r1); /* 2^31 */
    result[3] = right[0] << 7; /* get last bit from shift */
    
    carry = uECC_vli_add(result, result, right, num_words_secp160r1); /* 2^31 + 1 */
    for (i = num_words_secp160r1; carry; ++i) {
        uint16_t sum = (uint16_t)result[i] + carry;
        result[i] = (uint8_t)sum;
        carry = sum >> 8;
    }
}
#elif uECC_WORD_SIZE == 4
static void omega_mult_secp160r1(uint32_t *result, const uint32_t *right) {
    uint32_t carry;
    unsigned i;
    
    /* Multiply by (2^31 + 1). */
    uECC_vli_set(result + 1, right, num_words_secp160r1); /* 2^32 */
    uECC_vli_rshift1(result + 1, num_words_secp160r1); /* 2^31 */
    result[0] = right[0] << 31; /* get last bit from shift */
    
    carry = uECC_vli_add(result, result, right, num_words_secp160r1); /* 2^31 + 1 */
    for (i = num_words_secp160r1; carry; ++i) {
        uint64_t sum = (uint64_t)result[i] + carry;
        result[i] = (uint32_t)sum;
        carry = sum >> 32;
    }
}
#endif /* uECC_WORD_SIZE */
#endif /* (uECC_OPTIMIZATION_LEVEL > 0 && !asm_mmod_fast_secp160r1) */

#endif /* uECC_SUPPORTS_secp160r1 */

#if uECC_SUPPORTS_secp192r1

#if (uECC_OPTIMIZATION_LEVEL > 0)
static void vli_mmod_fast_secp192r1(uECC_word_t *result, uECC_word_t *product);
#endif

static const struct uECC_Curve_t curve_secp192r1 = {
    num_words_secp192r1,
    num_bytes_secp192r1,
    192, /* num_n_bits */
    { BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF),
        BYTES_TO_WORDS_8(FE, FF, FF, FF, FF, FF, FF, FF),
        BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF) },
    { BYTES_TO_WORDS_8(31, 28, D2, B4, B1, C9, 6B, 14),
        BYTES_TO_WORDS_8(36, F8, DE, 99, FF, FF, FF, FF),
        BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF) },
    { BYTES_TO_WORDS_8(12, 10, FF, 82, FD, 0A, FF, F4),
        BYTES_TO_WORDS_8(00, 88, A1, 43, EB, 20, BF, 7C),
        BYTES_TO_WORDS_8(F6, 90, 30, B0, 0E, A8, 8D, 18),

        BYTES_TO_WORDS_8(11, 48, 79, 1E, A1, 77, F9, 73),
        BYTES_TO_WORDS_8(D5, CD, 24, 6B, ED, 11, 10, 63),
        BYTES_TO_WORDS_8(78, DA, C8, FF, 95, 2B, 19, 07) },
    { BYTES_TO_WORDS_8(B1, B9, 46, C1, EC, DE, B8, FE),
        BYTES_TO_WORDS_8(49, 30, 24, 72, AB, E9, A7, 0F),
        BYTES_TO_WORDS_8(E7, 80, 9C, E5, 19, 05, 21, 64) },
    &double_jacobian_default,
#if uECC_SUPPORT_COMPRESSED_POINT
    &mod_sqrt_default,
#endif
    &x_side_default,
#if (uECC_OPTIMIZATION_LEVEL > 0)
    &vli_mmod_fast_secp192r1
#endif
};

uECC_Curve uECC_secp192r1(void) { return &curve_secp192r1; }

#if (uECC_OPTIMIZATION_LEVEL > 0)
/* Computes result = product % curve_p.
   See algorithm 5 and 6 from http://www.isys.uni-klu.ac.at/PDF/2001-0126-MT.pdf */
#if uECC_WORD_SIZE == 1
static void vli_mmod_fast_secp192r1(uint8_t *result, uint8_t *product) {
    uint8_t tmp[num_words_secp192r1];
    uint8_t carry;
    
    uECC_vli_set(result, product, num_words_secp192r1);
    
    uECC_vli_set(tmp, &product[24], num_words_secp192r1);
    carry = uECC_vli_add(result, result, tmp, num_words_secp192r1);
    
    tmp[0] = tmp[1] = tmp[2] = tmp[3] = tmp[4] = tmp[5] = tmp[6] = tmp[7] = 0;
    tmp[8] = product[24]; tmp[9] = product[25]; tmp[10] = product[26]; tmp[11] = product[27];
    tmp[12] = product[28]; tmp[13] = product[29]; tmp[14] = product[30]; tmp[15] = product[31];
    tmp[16] = product[32]; tmp[17] = product[33]; tmp[18] = product[34]; tmp[19] = product[35];
    tmp[20] = product[36]; tmp[21] = product[37]; tmp[22] = product[38]; tmp[23] = product[39];
    carry += uECC_vli_add(result, result, tmp, num_words_secp192r1);
    
    tmp[0] = tmp[8] = product[40];
    tmp[1] = tmp[9] = product[41];
    tmp[2] = tmp[10] = product[42];
    tmp[3] = tmp[11] = product[43];
    tmp[4] = tmp[12] = product[44];
    tmp[5] = tmp[13] = product[45];
    tmp[6] = tmp[14] = product[46];
    tmp[7] = tmp[15] = product[47];
    tmp[16] = tmp[17] = tmp[18] = tmp[19] = tmp[20] = tmp[21] = tmp[22] = tmp[23] = 0;
    carry += uECC_vli_add(result, result, tmp, num_words_secp192r1);
    
    while (carry || uECC_vli_cmp_unsafe(curve_secp192r1.p, result, num_words_secp192r1) != 1) {
        carry -= uECC_vli_sub(result, result, curve_secp192r1.p, num_words_secp192r1);
    }
}
#elif uECC_WORD_SIZE == 4
static void vli_mmod_fast_secp192r1(uint32_t *result, uint32_t *product) {
    uint32_t tmp[num_words_secp192r1];
    int carry;
    
    uECC_vli_set(result, product, num_words_secp192r1);
    
    uECC_vli_set(tmp, &product[6], num_words_secp192r1);
    carry = uECC_vli_add(result, result, tmp, num_words_secp192r1);
    
    tmp[0] = tmp[1] = 0;
    tmp[2] = product[6];
    tmp[3] = product[7];
    tmp[4] = product[8];
    tmp[5] = product[9];
    carry += uECC_vli_add(result, result, tmp, num_words_secp192r1);
    
    tmp[0] = tmp[2] = product[10];
    tmp[1] = tmp[3] = product[11];
    tmp[4] = tmp[5] = 0;
    carry += uECC_vli_add(result, result, tmp, num_words_secp192r1);
    
    while (carry || uECC_vli_cmp_unsafe(curve_secp192r1.p, result, num_words_secp192r1) != 1) {
        carry -= uECC_vli_sub(result, result, curve_secp192r1.p, num_words_secp192r1);
    }
}
#else
static void vli_mmod_fast_secp192r1(uint64_t *result, uint64_t *product) {
    uint64_t tmp[num_words_secp192r1];
    int carry;
    
    uECC_vli_set(result, product, num_words_secp192r1);
    
    uECC_vli_set(tmp, &product[3], num_words_secp192r1);
    carry = (int)uECC_vli_add(result, result, tmp, num_words_secp192r1);
    
    tmp[0] = 0;
    tmp[1] = product[3];
    tmp[2] = product[4];
    carry += uECC_vli_add(result, result, tmp, num_words_secp192r1);
    
    tmp[0] = tmp[1] = product[5];
    tmp[2] = 0;
    carry += uECC_vli_add(result, result, tmp, num_words_secp192r1);
    
    while (carry || uECC_vli_cmp_unsafe(curve_secp192r1.p, result, num_words_secp192r1) != 1) {
        carry -= uECC_vli_sub(result, result, curve_secp192r1.p, num_words_secp192r1);
    }
}
#endif /* uECC_WORD_SIZE */
#endif /* (uECC_OPTIMIZATION_LEVEL > 0) */

#endif /* uECC_SUPPORTS_secp192r1 */

#if uECC_SUPPORTS_secp224r1

#if uECC_SUPPORT_COMPRESSED_POINT
static void mod_sqrt_secp224r1(uECC_word_t *a, uECC_Curve curve);
#endif
#if (uECC_OPTIMIZATION_LEVEL > 0)
static void vli_mmod_fast_secp224r1(uECC_word_t *result, uECC_word_t *product);
#endif

static const struct uECC_Curve_t curve_secp224r1 = {
    num_words_secp224r1,
    num_bytes_secp224r1,
    224, /* num_n_bits */
    { BYTES_TO_WORDS_8(01, 00, 00, 00, 00, 00, 00, 00),
        BYTES_TO_WORDS_8(00, 00, 00, 00, FF, FF, FF, FF),
        BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF),
        BYTES_TO_WORDS_4(FF, FF, FF, FF) },
    { BYTES_TO_WORDS_8(3D, 2A, 5C, 5C, 45, 29, DD, 13),
        BYTES_TO_WORDS_8(3E, F0, B8, E0, A2, 16, FF, FF),
        BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF),
        BYTES_TO_WORDS_4(FF, FF, FF, FF) },
    { BYTES_TO_WORDS_8(21, 1D, 5C, 11, D6, 80, 32, 34),
        BYTES_TO_WORDS_8(22, 11, C2, 56, D3, C1, 03, 4A),
        BYTES_TO_WORDS_8(B9, 90, 13, 32, 7F, BF, B4, 6B),
        BYTES_TO_WORDS_4(BD, 0C, 0E, B7),

        BYTES_TO_WORDS_8(34, 7E, 00, 85, 99, 81, D5, 44),
        BYTES_TO_WORDS_8(64, 47, 07, 5A, A0, 75, 43, CD),
        BYTES_TO_WORDS_8(E6, DF, 22, 4C, FB, 23, F7, B5),
        BYTES_TO_WORDS_4(88, 63, 37, BD) },
    { BYTES_TO_WORDS_8(B4, FF, 55, 23, 43, 39, 0B, 27),
        BYTES_TO_WORDS_8(BA, D8, BF, D7, B7, B0, 44, 50),
        BYTES_TO_WORDS_8(56, 32, 41, F5, AB, B3, 04, 0C),
        BYTES_TO_WORDS_4(85, 0A, 05, B4) },
    &double_jacobian_default,
#if uECC_SUPPORT_COMPRESSED_POINT
    &mod_sqrt_secp224r1,
#endif
    &x_side_default,
#if (uECC_OPTIMIZATION_LEVEL > 0)
    &vli_mmod_fast_secp224r1
#endif
};

uECC_Curve uECC_secp224r1(void) { return &curve_secp224r1; }


#if uECC_SUPPORT_COMPRESSED_POINT
/* Routine 3.2.4 RS;  from http://www.nsa.gov/ia/_files/nist-routines.pdf */
static void mod_sqrt_secp224r1_rs(uECC_word_t *d1,
                                  uECC_word_t *e1,
                                  uECC_word_t *f1,
                                  const uECC_word_t *d0,
                                  const uECC_word_t *e0,
                                  const uECC_word_t *f0) {
    uECC_word_t t[num_words_secp224r1];

    uECC_vli_modSquare_fast(t, d0, &curve_secp224r1);                    /* t <-- d0 ^ 2 */
    uECC_vli_modMult_fast(e1, d0, e0, &curve_secp224r1);                 /* e1 <-- d0 * e0 */
    uECC_vli_modAdd(d1, t, f0, curve_secp224r1.p, num_words_secp224r1);  /* d1 <-- t  + f0 */
    uECC_vli_modAdd(e1, e1, e1, curve_secp224r1.p, num_words_secp224r1); /* e1 <-- e1 + e1 */
    uECC_vli_modMult_fast(f1, t, f0, &curve_secp224r1);                  /* f1 <-- t  * f0 */
    uECC_vli_modAdd(f1, f1, f1, curve_secp224r1.p, num_words_secp224r1); /* f1 <-- f1 + f1 */
    uECC_vli_modAdd(f1, f1, f1, curve_secp224r1.p, num_words_secp224r1); /* f1 <-- f1 + f1 */
}

/* Routine 3.2.5 RSS;  from http://www.nsa.gov/ia/_files/nist-routines.pdf */
static void mod_sqrt_secp224r1_rss(uECC_word_t *d1,
                                   uECC_word_t *e1,
                                   uECC_word_t *f1,
                                   const uECC_word_t *d0,
                                   const uECC_word_t *e0,
                                   const uECC_word_t *f0,
                                   const bitcount_t j) {
    bitcount_t i;

    uECC_vli_set(d1, d0, num_words_secp224r1); /* d1 <-- d0 */
    uECC_vli_set(e1, e0, num_words_secp224r1); /* e1 <-- e0 */
    uECC_vli_set(f1, f0, num_words_secp224r1); /* f1 <-- f0 */
    for (i = 1; i <= j; i++) {
        mod_sqrt_secp224r1_rs(d1, e1, f1, d1, e1, f1); /* RS (d1,e1,f1,d1,e1,f1) */
    }
}

/* Routine 3.2.6 RM;  from http://www.nsa.gov/ia/_files/nist-routines.pdf */
static void mod_sqrt_secp224r1_rm(uECC_word_t *d2,
                                  uECC_word_t *e2,
                                  uECC_word_t *f2,
                                  const uECC_word_t *c,
                                  const uECC_word_t *d0,
                                  const uECC_word_t *e0,
                                  const uECC_word_t *d1,
                                  const uECC_word_t *e1) {
    uECC_word_t t1[num_words_secp224r1];
    uECC_word_t t2[num_words_secp224r1];

    uECC_vli_modMult_fast(t1, e0, e1, &curve_secp224r1); /* t1 <-- e0 * e1 */
    uECC_vli_modMult_fast(t1, t1, c, &curve_secp224r1);  /* t1 <-- t1 * c */
    /* t1 <-- p  - t1 */
    uECC_vli_modSub(t1, curve_secp224r1.p, t1, curve_secp224r1.p, num_words_secp224r1);
    uECC_vli_modMult_fast(t2, d0, d1, &curve_secp224r1);                 /* t2 <-- d0 * d1 */
    uECC_vli_modAdd(t2, t2, t1, curve_secp224r1.p, num_words_secp224r1); /* t2 <-- t2 + t1 */
    uECC_vli_modMult_fast(t1, d0, e1, &curve_secp224r1);                 /* t1 <-- d0 * e1 */
    uECC_vli_modMult_fast(e2, d1, e0, &curve_secp224r1);                 /* e2 <-- d1 * e0 */
    uECC_vli_modAdd(e2, e2, t1, curve_secp224r1.p, num_words_secp224r1); /* e2 <-- e2 + t1 */
    uECC_vli_modSquare_fast(f2, e2, &curve_secp224r1);                   /* f2 <-- e2^2 */
    uECC_vli_modMult_fast(f2, f2, c, &curve_secp224r1);                  /* f2 <-- f2 * c */
    /* f2 <-- p  - f2 */
    uECC_vli_modSub(f2, curve_secp224r1.p, f2, curve_secp224r1.p, num_words_secp224r1);
    uECC_vli_set(d2, t2, num_words_secp224r1); /* d2 <-- t2 */
}

/* Routine 3.2.7 RP;  from http://www.nsa.gov/ia/_files/nist-routines.pdf */
static void mod_sqrt_secp224r1_rp(uECC_word_t *d1,
                                  uECC_word_t *e1,
                                  uECC_word_t *f1,
                                  const uECC_word_t *c,
                                  const uECC_word_t *r) {
    wordcount_t i;
    wordcount_t pow2i = 1;
    uECC_word_t d0[num_words_secp224r1];
    uECC_word_t e0[num_words_secp224r1] = {1}; /* e0 <-- 1 */
    uECC_word_t f0[num_words_secp224r1];

    uECC_vli_set(d0, r, num_words_secp224r1); /* d0 <-- r */
    /* f0 <-- p  - c */
    uECC_vli_modSub(f0, curve_secp224r1.p, c, curve_secp224r1.p, num_words_secp224r1);
    for (i = 0; i <= 6; i++) {
        mod_sqrt_secp224r1_rss(d1, e1, f1, d0, e0, f0, pow2i); /* RSS (d1,e1,f1,d0,e0,f0,2^i) */
        mod_sqrt_secp224r1_rm(d1, e1, f1, c, d1, e1, d0, e0);  /* RM (d1,e1,f1,c,d1,e1,d0,e0) */
        uECC_vli_set(d0, d1, num_words_secp224r1); /* d0 <-- d1 */
        uECC_vli_set(e0, e1, num_words_secp224r1); /* e0 <-- e1 */
        uECC_vli_set(f0, f1, num_words_secp224r1); /* f0 <-- f1 */
        pow2i *= 2;
    }
}

/* Compute a = sqrt(a) (mod curve_p). */
/* Routine 3.2.8 mp_mod_sqrt_224; from http://www.nsa.gov/ia/_files/nist-routines.pdf */
static void mod_sqrt_secp224r1(uECC_word_t *a, uECC_Curve curve) {
    bitcount_t i;
    uECC_word_t e1[num_words_secp224r1];
    uECC_word_t f1[num_words_secp224r1];
    uECC_word_t d0[num_words_secp224r1];
    uECC_word_t e0[num_words_secp224r1];
    uECC_word_t f0[num_words_secp224r1];
    uECC_word_t d1[num_words_secp224r1];

    /* s = a; using constant instead of random value */
    mod_sqrt_secp224r1_rp(d0, e0, f0, a, a);           /* RP (d0, e0, f0, c, s) */
    mod_sqrt_secp224r1_rs(d1, e1, f1, d0, e0, f0);     /* RS (d1, e1, f1, d0, e0, f0) */
    for (i = 1; i <= 95; i++) {
        uECC_vli_set(d0, d1, num_words_secp224r1);          /* d0 <-- d1 */
        uECC_vli_set(e0, e1, num_words_secp224r1);          /* e0 <-- e1 */
        uECC_vli_set(f0, f1, num_words_secp224r1);          /* f0 <-- f1 */
        mod_sqrt_secp224r1_rs(d1, e1, f1, d0, e0, f0); /* RS (d1, e1, f1, d0, e0, f0) */
        if (uECC_vli_isZero(d1, num_words_secp224r1)) {     /* if d1 == 0 */
                break;
        }
    }
    uECC_vli_modInv(f1, e0, curve_secp224r1.p, num_words_secp224r1); /* f1 <-- 1 / e0 */
    uECC_vli_modMult_fast(a, d0, f1, &curve_secp224r1);              /* a  <-- d0 / e0 */
}
#endif /* uECC_SUPPORT_COMPRESSED_POINT */

#if (uECC_OPTIMIZATION_LEVEL > 0)
/* Computes result = product % curve_p
   from http://www.nsa.gov/ia/_files/nist-routines.pdf */
#if uECC_WORD_SIZE == 1
static void vli_mmod_fast_secp224r1(uint8_t *result, uint8_t *product) {
    uint8_t tmp[num_words_secp224r1];
    int8_t carry;

    /* t */
    uECC_vli_set(result, product, num_words_secp224r1);

    /* s1 */
    tmp[0] = tmp[1] = tmp[2] = tmp[3] = 0;
    tmp[4] = tmp[5] = tmp[6] = tmp[7] = 0;
    tmp[8] = tmp[9] = tmp[10] = tmp[11] = 0;
    tmp[12] = product[28]; tmp[13] = product[29]; tmp[14] = product[30]; tmp[15] = product[31];
    tmp[16] = product[32]; tmp[17] = product[33]; tmp[18] = product[34]; tmp[19] = product[35];
    tmp[20] = product[36]; tmp[21] = product[37]; tmp[22] = product[38]; tmp[23] = product[39];
    tmp[24] = product[40]; tmp[25] = product[41]; tmp[26] = product[42]; tmp[27] = product[43];
    carry = uECC_vli_add(result, result, tmp, num_words_secp224r1);

    /* s2 */
    tmp[12] = product[44]; tmp[13] = product[45]; tmp[14] = product[46]; tmp[15] = product[47];
    tmp[16] = product[48]; tmp[17] = product[49]; tmp[18] = product[50]; tmp[19] = product[51];
    tmp[20] = product[52]; tmp[21] = product[53]; tmp[22] = product[54]; tmp[23] = product[55];
    tmp[24] = tmp[25] = tmp[26] = tmp[27] = 0;
    carry += uECC_vli_add(result, result, tmp, num_words_secp224r1);

    /* d1 */
    tmp[0]  = product[28]; tmp[1]  = product[29]; tmp[2]  = product[30]; tmp[3]  = product[31];
    tmp[4]  = product[32]; tmp[5]  = product[33]; tmp[6]  = product[34]; tmp[7]  = product[35];
    tmp[8]  = product[36]; tmp[9]  = product[37]; tmp[10] = product[38]; tmp[11] = product[39];
    tmp[12] = product[40]; tmp[13] = product[41]; tmp[14] = product[42]; tmp[15] = product[43];
    tmp[16] = product[44]; tmp[17] = product[45]; tmp[18] = product[46]; tmp[19] = product[47];
    tmp[20] = product[48]; tmp[21] = product[49]; tmp[22] = product[50]; tmp[23] = product[51];
    tmp[24] = product[52]; tmp[25] = product[53]; tmp[26] = product[54]; tmp[27] = product[55];
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp224r1);

    /* d2 */
    tmp[0]  = product[44]; tmp[1]  = product[45]; tmp[2]  = product[46]; tmp[3]  = product[47];
    tmp[4]  = product[48]; tmp[5]  = product[49]; tmp[6]  = product[50]; tmp[7]  = product[51];
    tmp[8]  = product[52]; tmp[9]  = product[53]; tmp[10] = product[54]; tmp[11] = product[55];
    tmp[12] = tmp[13] = tmp[14] = tmp[15] = 0;
    tmp[16] = tmp[17] = tmp[18] = tmp[19] = 0;
    tmp[20] = tmp[21] = tmp[22] = tmp[23] = 0;
    tmp[24] = tmp[25] = tmp[26] = tmp[27] = 0;
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp224r1);

    if (carry < 0) {
        do {
            carry += uECC_vli_add(result, result, curve_secp224r1.p, num_words_secp224r1);
        } while (carry < 0);
    } else {
        while (carry || uECC_vli_cmp_unsafe(curve_secp224r1.p, result, num_words_secp224r1) != 1) {
            carry -= uECC_vli_sub(result, result, curve_secp224r1.p, num_words_secp224r1);
        }
    }
}
#elif uECC_WORD_SIZE == 4
static void vli_mmod_fast_secp224r1(uint32_t *result, uint32_t *product)
{
    uint32_t tmp[num_words_secp224r1];
    int carry;

    /* t */
    uECC_vli_set(result, product, num_words_secp224r1);

    /* s1 */
    tmp[0] = tmp[1] = tmp[2] = 0;
    tmp[3] = product[7];
    tmp[4] = product[8];
    tmp[5] = product[9];
    tmp[6] = product[10];
    carry = uECC_vli_add(result, result, tmp, num_words_secp224r1);

    /* s2 */
    tmp[3] = product[11];
    tmp[4] = product[12];
    tmp[5] = product[13];
    tmp[6] = 0;
    carry += uECC_vli_add(result, result, tmp, num_words_secp224r1);

    /* d1 */
    tmp[0] = product[7];
    tmp[1] = product[8];
    tmp[2] = product[9];
    tmp[3] = product[10];
    tmp[4] = product[11];
    tmp[5] = product[12];
    tmp[6] = product[13];
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp224r1);

    /* d2 */
    tmp[0] = product[11];
    tmp[1] = product[12];
    tmp[2] = product[13];
    tmp[3] = tmp[4] = tmp[5] = tmp[6] = 0;
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp224r1);

    if (carry < 0) {
        do {
            carry += uECC_vli_add(result, result, curve_secp224r1.p, num_words_secp224r1);
        } while (carry < 0);
    } else {
        while (carry || uECC_vli_cmp_unsafe(curve_secp224r1.p, result, num_words_secp224r1) != 1) {
            carry -= uECC_vli_sub(result, result, curve_secp224r1.p, num_words_secp224r1);
        }
    }
}
#else
static void vli_mmod_fast_secp224r1(uint64_t *result, uint64_t *product)
{
    uint64_t tmp[num_words_secp224r1];
    int carry = 0;

    /* t */
    uECC_vli_set(result, product, num_words_secp224r1);
    result[num_words_secp224r1 - 1] &= 0xffffffff;

    /* s1 */
    tmp[0] = 0;
    tmp[1] = product[3] & 0xffffffff00000000ull;
    tmp[2] = product[4];
    tmp[3] = product[5] & 0xffffffff;
    uECC_vli_add(result, result, tmp, num_words_secp224r1);

    /* s2 */
    tmp[1] = product[5] & 0xffffffff00000000ull;
    tmp[2] = product[6];
    tmp[3] = 0;
    uECC_vli_add(result, result, tmp, num_words_secp224r1);

    /* d1 */
    tmp[0] = (product[3] >> 32) | (product[4] << 32);
    tmp[1] = (product[4] >> 32) | (product[5] << 32);
    tmp[2] = (product[5] >> 32) | (product[6] << 32);
    tmp[3] = product[6] >> 32;
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp224r1);

    /* d2 */
    tmp[0] = (product[5] >> 32) | (product[6] << 32);
    tmp[1] = product[6] >> 32;
    tmp[2] = tmp[3] = 0;
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp224r1);

    if (carry < 0) {
        do {
            carry += uECC_vli_add(result, result, curve_secp224r1.p, num_words_secp224r1);
        } while (carry < 0);
    } else {
        while (uECC_vli_cmp_unsafe(curve_secp224r1.p, result, num_words_secp224r1) != 1) {
            uECC_vli_sub(result, result, curve_secp224r1.p, num_words_secp224r1);
        }
    }
}
#endif /* uECC_WORD_SIZE */
#endif /* (uECC_OPTIMIZATION_LEVEL > 0) */

#endif /* uECC_SUPPORTS_secp224r1 */

#if uECC_SUPPORTS_secp256r1

#if (uECC_OPTIMIZATION_LEVEL > 0)
static void vli_mmod_fast_secp256r1(uECC_word_t *result, uECC_word_t *product);
#endif

static const struct uECC_Curve_t curve_secp256r1 = {
    num_words_secp256r1,
    num_bytes_secp256r1,
    256, /* num_n_bits */
    { BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF),
        BYTES_TO_WORDS_8(FF, FF, FF, FF, 00, 00, 00, 00),
        BYTES_TO_WORDS_8(00, 00, 00, 00, 00, 00, 00, 00),
        BYTES_TO_WORDS_8(01, 00, 00, 00, FF, FF, FF, FF) },
    { BYTES_TO_WORDS_8(51, 25, 63, FC, C2, CA, B9, F3),
        BYTES_TO_WORDS_8(84, 9E, 17, A7, AD, FA, E6, BC),
        BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF),
        BYTES_TO_WORDS_8(00, 00, 00, 00, FF, FF, FF, FF) },
    { BYTES_TO_WORDS_8(96, C2, 98, D8, 45, 39, A1, F4),
        BYTES_TO_WORDS_8(A0, 33, EB, 2D, 81, 7D, 03, 77),
        BYTES_TO_WORDS_8(F2, 40, A4, 63, E5, E6, BC, F8),
        BYTES_TO_WORDS_8(47, 42, 2C, E1, F2, D1, 17, 6B),

        BYTES_TO_WORDS_8(F5, 51, BF, 37, 68, 40, B6, CB),
        BYTES_TO_WORDS_8(CE, 5E, 31, 6B, 57, 33, CE, 2B),
        BYTES_TO_WORDS_8(16, 9E, 0F, 7C, 4A, EB, E7, 8E),
        BYTES_TO_WORDS_8(9B, 7F, 1A, FE, E2, 42, E3, 4F) },
    { BYTES_TO_WORDS_8(4B, 60, D2, 27, 3E, 3C, CE, 3B),
        BYTES_TO_WORDS_8(F6, B0, 53, CC, B0, 06, 1D, 65),
        BYTES_TO_WORDS_8(BC, 86, 98, 76, 55, BD, EB, B3),
        BYTES_TO_WORDS_8(E7, 93, 3A, AA, D8, 35, C6, 5A) },
    &double_jacobian_default,
#if uECC_SUPPORT_COMPRESSED_POINT
    &mod_sqrt_default,
#endif
    &x_side_default,
#if (uECC_OPTIMIZATION_LEVEL > 0)
    &vli_mmod_fast_secp256r1
#endif
};

uECC_Curve uECC_secp256r1(void) { return &curve_secp256r1; }


#if (uECC_OPTIMIZATION_LEVEL > 0 && !asm_mmod_fast_secp256r1)
/* Computes result = product % curve_p
   from http://www.nsa.gov/ia/_files/nist-routines.pdf */
#if uECC_WORD_SIZE == 1
static void vli_mmod_fast_secp256r1(uint8_t *result, uint8_t *product) {
    uint8_t tmp[num_words_secp256r1];
    int8_t carry;
    
    /* t */
    uECC_vli_set(result, product, num_words_secp256r1);
    
    /* s1 */
    tmp[0] = tmp[1] = tmp[2] = tmp[3] = 0;
    tmp[4] = tmp[5] = tmp[6] = tmp[7] = 0;
    tmp[8] = tmp[9] = tmp[10] = tmp[11] = 0;
    tmp[12] = product[44]; tmp[13] = product[45]; tmp[14] = product[46]; tmp[15] = product[47];
    tmp[16] = product[48]; tmp[17] = product[49]; tmp[18] = product[50]; tmp[19] = product[51];
    tmp[20] = product[52]; tmp[21] = product[53]; tmp[22] = product[54]; tmp[23] = product[55];
    tmp[24] = product[56]; tmp[25] = product[57]; tmp[26] = product[58]; tmp[27] = product[59];
    tmp[28] = product[60]; tmp[29] = product[61]; tmp[30] = product[62]; tmp[31] = product[63];
    carry = uECC_vli_add(tmp, tmp, tmp, num_words_secp256r1);
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* s2 */
    tmp[12] = product[48]; tmp[13] = product[49]; tmp[14] = product[50]; tmp[15] = product[51];
    tmp[16] = product[52]; tmp[17] = product[53]; tmp[18] = product[54]; tmp[19] = product[55];
    tmp[20] = product[56]; tmp[21] = product[57]; tmp[22] = product[58]; tmp[23] = product[59];
    tmp[24] = product[60]; tmp[25] = product[61]; tmp[26] = product[62]; tmp[27] = product[63];
    tmp[28] = tmp[29] = tmp[30] = tmp[31] = 0;
    carry += uECC_vli_add(tmp, tmp, tmp, num_words_secp256r1);
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* s3 */
    tmp[0] = product[32]; tmp[1] = product[33]; tmp[2] = product[34]; tmp[3] = product[35];
    tmp[4] = product[36]; tmp[5] = product[37]; tmp[6] = product[38]; tmp[7] = product[39];
    tmp[8] = product[40]; tmp[9] = product[41]; tmp[10] = product[42]; tmp[11] = product[43];
    tmp[12] = tmp[13] = tmp[14] = tmp[15] = 0;
    tmp[16] = tmp[17] = tmp[18] = tmp[19] = 0;
    tmp[20] = tmp[21] = tmp[22] = tmp[23] = 0;
    tmp[24] = product[56]; tmp[25] = product[57]; tmp[26] = product[58]; tmp[27] = product[59];
    tmp[28] = product[60]; tmp[29] = product[61]; tmp[30] = product[62]; tmp[31] = product[63];
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* s4 */
    tmp[0] = product[36]; tmp[1] = product[37]; tmp[2] = product[38]; tmp[3] = product[39];
    tmp[4] = product[40]; tmp[5] = product[41]; tmp[6] = product[42]; tmp[7] = product[43];
    tmp[8] = product[44]; tmp[9] = product[45]; tmp[10] = product[46]; tmp[11] = product[47];
    tmp[12] = product[52]; tmp[13] = product[53]; tmp[14] = product[54]; tmp[15] = product[55];
    tmp[16] = product[56]; tmp[17] = product[57]; tmp[18] = product[58]; tmp[19] = product[59];
    tmp[20] = product[60]; tmp[21] = product[61]; tmp[22] = product[62]; tmp[23] = product[63];
    tmp[24] = product[52]; tmp[25] = product[53]; tmp[26] = product[54]; tmp[27] = product[55];
    tmp[28] = product[32]; tmp[29] = product[33]; tmp[30] = product[34]; tmp[31] = product[35];
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* d1 */
    tmp[0] = product[44]; tmp[1] = product[45]; tmp[2] = product[46]; tmp[3] = product[47];
    tmp[4] = product[48]; tmp[5] = product[49]; tmp[6] = product[50]; tmp[7] = product[51];
    tmp[8] = product[52]; tmp[9] = product[53]; tmp[10] = product[54]; tmp[11] = product[55];
    tmp[12] = tmp[13] = tmp[14] = tmp[15] = 0;
    tmp[16] = tmp[17] = tmp[18] = tmp[19] = 0;
    tmp[20] = tmp[21] = tmp[22] = tmp[23] = 0;
    tmp[24] = product[32]; tmp[25] = product[33]; tmp[26] = product[34]; tmp[27] = product[35];
    tmp[28] = product[40]; tmp[29] = product[41]; tmp[30] = product[42]; tmp[31] = product[43];
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    /* d2 */
    tmp[0] = product[48]; tmp[1] = product[49]; tmp[2] = product[50]; tmp[3] = product[51];
    tmp[4] = product[52]; tmp[5] = product[53]; tmp[6] = product[54]; tmp[7] = product[55];
    tmp[8] = product[56]; tmp[9] = product[57]; tmp[10] = product[58]; tmp[11] = product[59];
    tmp[12] = product[60]; tmp[13] = product[61]; tmp[14] = product[62]; tmp[15] = product[63];
    tmp[16] = tmp[17] = tmp[18] = tmp[19] = 0;
    tmp[20] = tmp[21] = tmp[22] = tmp[23] = 0;
    tmp[24] = product[36]; tmp[25] = product[37]; tmp[26] = product[38]; tmp[27] = product[39];
    tmp[28] = product[44]; tmp[29] = product[45]; tmp[30] = product[46]; tmp[31] = product[47];
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    /* d3 */
    tmp[0] = product[52]; tmp[1] = product[53]; tmp[2] = product[54]; tmp[3] = product[55];
    tmp[4] = product[56]; tmp[5] = product[57]; tmp[6] = product[58]; tmp[7] = product[59];
    tmp[8] = product[60]; tmp[9] = product[61]; tmp[10] = product[62]; tmp[11] = product[63];
    tmp[12] = product[32]; tmp[13] = product[33]; tmp[14] = product[34]; tmp[15] = product[35];
    tmp[16] = product[36]; tmp[17] = product[37]; tmp[18] = product[38]; tmp[19] = product[39];
    tmp[20] = product[40]; tmp[21] = product[41]; tmp[22] = product[42]; tmp[23] = product[43];
    tmp[24] = tmp[25] = tmp[26] = tmp[27] = 0;
    tmp[28] = product[48]; tmp[29] = product[49]; tmp[30] = product[50]; tmp[31] = product[51];
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    /* d4 */
    tmp[0] = product[56]; tmp[1] = product[57]; tmp[2] = product[58]; tmp[3] = product[59];
    tmp[4] = product[60]; tmp[5] = product[61]; tmp[6] = product[62]; tmp[7] = product[63];
    tmp[8] = tmp[9] = tmp[10] = tmp[11] = 0;
    tmp[12] = product[36]; tmp[13] = product[37]; tmp[14] = product[38]; tmp[15] = product[39];
    tmp[16] = product[40]; tmp[17] = product[41]; tmp[18] = product[42]; tmp[19] = product[43];
    tmp[20] = product[44]; tmp[21] = product[45]; tmp[22] = product[46]; tmp[23] = product[47];
    tmp[24] = tmp[25] = tmp[26] = tmp[27] = 0;
    tmp[28] = product[52]; tmp[29] = product[53]; tmp[30] = product[54]; tmp[31] = product[55];
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    if (carry < 0) {
        do {
            carry += uECC_vli_add(result, result, curve_secp256r1.p, num_words_secp256r1);
        } while (carry < 0);
    } else {
        while (carry || uECC_vli_cmp_unsafe(curve_secp256r1.p, result, num_words_secp256r1) != 1) {
            carry -= uECC_vli_sub(result, result, curve_secp256r1.p, num_words_secp256r1);
        }
    }
}
#elif uECC_WORD_SIZE == 4
static void vli_mmod_fast_secp256r1(uint32_t *result, uint32_t *product) {
    uint32_t tmp[num_words_secp256r1];
    int carry;
    
    /* t */
    uECC_vli_set(result, product, num_words_secp256r1);
    
    /* s1 */
    tmp[0] = tmp[1] = tmp[2] = 0;
    tmp[3] = product[11];
    tmp[4] = product[12];
    tmp[5] = product[13];
    tmp[6] = product[14];
    tmp[7] = product[15];
    carry = uECC_vli_add(tmp, tmp, tmp, num_words_secp256r1);
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* s2 */
    tmp[3] = product[12];
    tmp[4] = product[13];
    tmp[5] = product[14];
    tmp[6] = product[15];
    tmp[7] = 0;
    carry += uECC_vli_add(tmp, tmp, tmp, num_words_secp256r1);
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* s3 */
    tmp[0] = product[8];
    tmp[1] = product[9];
    tmp[2] = product[10];
    tmp[3] = tmp[4] = tmp[5] = 0;
    tmp[6] = product[14];
    tmp[7] = product[15];
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* s4 */
    tmp[0] = product[9];
    tmp[1] = product[10];
    tmp[2] = product[11];
    tmp[3] = product[13];
    tmp[4] = product[14];
    tmp[5] = product[15];
    tmp[6] = product[13];
    tmp[7] = product[8];
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* d1 */
    tmp[0] = product[11];
    tmp[1] = product[12];
    tmp[2] = product[13];
    tmp[3] = tmp[4] = tmp[5] = 0;
    tmp[6] = product[8];
    tmp[7] = product[10];
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    /* d2 */
    tmp[0] = product[12];
    tmp[1] = product[13];
    tmp[2] = product[14];
    tmp[3] = product[15];
    tmp[4] = tmp[5] = 0;
    tmp[6] = product[9];
    tmp[7] = product[11];
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    /* d3 */
    tmp[0] = product[13];
    tmp[1] = product[14];
    tmp[2] = product[15];
    tmp[3] = product[8];
    tmp[4] = product[9];
    tmp[5] = product[10];
    tmp[6] = 0;
    tmp[7] = product[12];
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    /* d4 */
    tmp[0] = product[14];
    tmp[1] = product[15];
    tmp[2] = 0;
    tmp[3] = product[9];
    tmp[4] = product[10];
    tmp[5] = product[11];
    tmp[6] = 0;
    tmp[7] = product[13];
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    if (carry < 0) {
        do {
            carry += uECC_vli_add(result, result, curve_secp256r1.p, num_words_secp256r1);
        } while (carry < 0);
    } else {
        while (carry || uECC_vli_cmp_unsafe(curve_secp256r1.p, result, num_words_secp256r1) != 1) {
            carry -= uECC_vli_sub(result, result, curve_secp256r1.p, num_words_secp256r1);
        }
    }
}
#else
static void vli_mmod_fast_secp256r1(uint64_t *result, uint64_t *product) {
    uint64_t tmp[num_words_secp256r1];
    int carry;
    
    /* t */
    uECC_vli_set(result, product, num_words_secp256r1);
    
    /* s1 */
    tmp[0] = 0;
    tmp[1] = product[5] & 0xffffffff00000000ull;
    tmp[2] = product[6];
    tmp[3] = product[7];
    carry = (int)uECC_vli_add(tmp, tmp, tmp, num_words_secp256r1);
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* s2 */
    tmp[1] = product[6] << 32;
    tmp[2] = (product[6] >> 32) | (product[7] << 32);
    tmp[3] = product[7] >> 32;
    carry += uECC_vli_add(tmp, tmp, tmp, num_words_secp256r1);
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* s3 */
    tmp[0] = product[4];
    tmp[1] = product[5] & 0xffffffff;
    tmp[2] = 0;
    tmp[3] = product[7];
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* s4 */
    tmp[0] = (product[4] >> 32) | (product[5] << 32);
    tmp[1] = (product[5] >> 32) | (product[6] & 0xffffffff00000000ull);
    tmp[2] = product[7];
    tmp[3] = (product[6] >> 32) | (product[4] << 32);
    carry += uECC_vli_add(result, result, tmp, num_words_secp256r1);
    
    /* d1 */
    tmp[0] = (product[5] >> 32) | (product[6] << 32);
    tmp[1] = (product[6] >> 32);
    tmp[2] = 0;
    tmp[3] = (product[4] & 0xffffffff) | (product[5] << 32);
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    /* d2 */
    tmp[0] = product[6];
    tmp[1] = product[7];
    tmp[2] = 0;
    tmp[3] = (product[4] >> 32) | (product[5] & 0xffffffff00000000ull);
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    /* d3 */
    tmp[0] = (product[6] >> 32) | (product[7] << 32);
    tmp[1] = (product[7] >> 32) | (product[4] << 32);
    tmp[2] = (product[4] >> 32) | (product[5] << 32);
    tmp[3] = (product[6] << 32);
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    /* d4 */
    tmp[0] = product[7];
    tmp[1] = product[4] & 0xffffffff00000000ull;
    tmp[2] = product[5];
    tmp[3] = product[6] & 0xffffffff00000000ull;
    carry -= uECC_vli_sub(result, result, tmp, num_words_secp256r1);
    
    if (carry < 0) {
        do {
            carry += uECC_vli_add(result, result, curve_secp256r1.p, num_words_secp256r1);
        } while (carry < 0);
    } else {
        while (carry || uECC_vli_cmp_unsafe(curve_secp256r1.p, result, num_words_secp256r1) != 1) {
            carry -= uECC_vli_sub(result, result, curve_secp256r1.p, num_words_secp256r1);
        }
    }
}
#endif /* uECC_WORD_SIZE */
#endif /* (uECC_OPTIMIZATION_LEVEL > 0 && !asm_mmod_fast_secp256r1) */

#endif /* uECC_SUPPORTS_secp256r1 */

#if uECC_SUPPORTS_secp256k1

static void double_jacobian_secp256k1(uECC_word_t * X1,
                                      uECC_word_t * Y1,
                                      uECC_word_t * Z1,
                                      uECC_Curve curve);
static void x_side_secp256k1(uECC_word_t *result, const uECC_word_t *x, uECC_Curve curve);
#if (uECC_OPTIMIZATION_LEVEL > 0)
static void vli_mmod_fast_secp256k1(uECC_word_t *result, uECC_word_t *product);
#endif

static const struct uECC_Curve_t curve_secp256k1 = {
    num_words_secp256k1,
    num_bytes_secp256k1,
    256, /* num_n_bits */
    { BYTES_TO_WORDS_8(2F, FC, FF, FF, FE, FF, FF, FF),
        BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF),
        BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF),
        BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF) },
    { BYTES_TO_WORDS_8(41, 41, 36, D0, 8C, 5E, D2, BF),
        BYTES_TO_WORDS_8(3B, A0, 48, AF, E6, DC, AE, BA),
        BYTES_TO_WORDS_8(FE, FF, FF, FF, FF, FF, FF, FF),
        BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF) },
    { BYTES_TO_WORDS_8(98, 17, F8, 16, 5B, 81, F2, 59),
        BYTES_TO_WORDS_8(D9, 28, CE, 2D, DB, FC, 9B, 02),
        BYTES_TO_WORDS_8(07, 0B, 87, CE, 95, 62, A0, 55),
        BYTES_TO_WORDS_8(AC, BB, DC, F9, 7E, 66, BE, 79),

        BYTES_TO_WORDS_8(B8, D4, 10, FB, 8F, D0, 47, 9C),
        BYTES_TO_WORDS_8(19, 54, 85, A6, 48, B4, 17, FD),
        BYTES_TO_WORDS_8(A8, 08, 11, 0E, FC, FB, A4, 5D),
        BYTES_TO_WORDS_8(65, C4, A3, 26, 77, DA, 3A, 48) },
    { BYTES_TO_WORDS_8(07, 00, 00, 00, 00, 00, 00, 00),
        BYTES_TO_WORDS_8(00, 00, 00, 00, 00, 00, 00, 00),
        BYTES_TO_WORDS_8(00, 00, 00, 00, 00, 00, 00, 00),
        BYTES_TO_WORDS_8(00, 00, 00, 00, 00, 00, 00, 00) },
    &double_jacobian_secp256k1,
#if uECC_SUPPORT_COMPRESSED_POINT
    &mod_sqrt_default,
#endif
    &x_side_secp256k1,
#if (uECC_OPTIMIZATION_LEVEL > 0)
    &vli_mmod_fast_secp256k1
#endif
};

uECC_Curve uECC_secp256k1(void) { return &curve_secp256k1; }


/* Double in place */
static void double_jacobian_secp256k1(uECC_word_t * X1,
                                      uECC_word_t * Y1,
                                      uECC_word_t * Z1,
                                      uECC_Curve curve) {
    /* t1 = X, t2 = Y, t3 = Z */
    uECC_word_t t4[num_words_secp256k1];
    uECC_word_t t5[num_words_secp256k1];
    
    if (uECC_vli_isZero(Z1, num_words_secp256k1)) {
        return;
    }
    
    uECC_vli_modSquare_fast(t5, Y1, curve);   /* t5 = y1^2 */
    uECC_vli_modMult_fast(t4, X1, t5, curve); /* t4 = x1*y1^2 = A */
    uECC_vli_modSquare_fast(X1, X1, curve);   /* t1 = x1^2 */
    uECC_vli_modSquare_fast(t5, t5, curve);   /* t5 = y1^4 */
    uECC_vli_modMult_fast(Z1, Y1, Z1, curve); /* t3 = y1*z1 = z3 */
    
    uECC_vli_modAdd(Y1, X1, X1, curve->p, num_words_secp256k1); /* t2 = 2*x1^2 */
    uECC_vli_modAdd(Y1, Y1, X1, curve->p, num_words_secp256k1); /* t2 = 3*x1^2 */
    if (uECC_vli_testBit(Y1, 0)) {
        uECC_word_t carry = uECC_vli_add(Y1, Y1, curve->p, num_words_secp256k1);
        uECC_vli_rshift1(Y1, num_words_secp256k1);
        Y1[num_words_secp256k1 - 1] |= carry << (uECC_WORD_BITS - 1);
    } else {
        uECC_vli_rshift1(Y1, num_words_secp256k1);
    }
    /* t2 = 3/2*(x1^2) = B */
    
    uECC_vli_modSquare_fast(X1, Y1, curve);                     /* t1 = B^2 */
    uECC_vli_modSub(X1, X1, t4, curve->p, num_words_secp256k1); /* t1 = B^2 - A */
    uECC_vli_modSub(X1, X1, t4, curve->p, num_words_secp256k1); /* t1 = B^2 - 2A = x3 */
    
    uECC_vli_modSub(t4, t4, X1, curve->p, num_words_secp256k1); /* t4 = A - x3 */
    uECC_vli_modMult_fast(Y1, Y1, t4, curve);                   /* t2 = B * (A - x3) */
    uECC_vli_modSub(Y1, Y1, t5, curve->p, num_words_secp256k1); /* t2 = B * (A - x3) - y1^4 = y3 */
}

/* Computes result = x^3 + b. result must not overlap x. */
static void x_side_secp256k1(uECC_word_t *result, const uECC_word_t *x, uECC_Curve curve) {
    uECC_vli_modSquare_fast(result, x, curve);                                /* r = x^2 */
    uECC_vli_modMult_fast(result, result, x, curve);                          /* r = x^3 */
    uECC_vli_modAdd(result, result, curve->b, curve->p, num_words_secp256k1); /* r = x^3 + b */
}

#if (uECC_OPTIMIZATION_LEVEL > 0 && !asm_mmod_fast_secp256k1)
static void omega_mult_secp256k1(uECC_word_t *result, const uECC_word_t *right);
static void vli_mmod_fast_secp256k1(uECC_word_t *result, uECC_word_t *product) {
    uECC_word_t tmp[2 * num_words_secp256k1];
    uECC_word_t carry;
    
    uECC_vli_clear(tmp, num_words_secp256k1);
    uECC_vli_clear(tmp + num_words_secp256k1, num_words_secp256k1);
    
    omega_mult_secp256k1(tmp, product + num_words_secp256k1); /* (Rq, q) = q * c */
    
    carry = uECC_vli_add(result, product, tmp, num_words_secp256k1); /* (C, r) = r + q       */
    uECC_vli_clear(product, num_words_secp256k1);
    omega_mult_secp256k1(product, tmp + num_words_secp256k1); /* Rq*c */
    carry += uECC_vli_add(result, result, product, num_words_secp256k1); /* (C1, r) = r + Rq*c */
    
    while (carry > 0) {
        --carry;
        uECC_vli_sub(result, result, curve_secp256k1.p, num_words_secp256k1);
    }
    if (uECC_vli_cmp_unsafe(result, curve_secp256k1.p, num_words_secp256k1) > 0) {
        uECC_vli_sub(result, result, curve_secp256k1.p, num_words_secp256k1);
    }
}

#if uECC_WORD_SIZE == 1
static void omega_mult_secp256k1(uint8_t * result, const uint8_t * right) {
    /* Multiply by (2^32 + 2^9 + 2^8 + 2^7 + 2^6 + 2^4 + 1). */
    uECC_word_t r0 = 0;
    uECC_word_t r1 = 0;
    uECC_word_t r2 = 0;
    wordcount_t k;
    
    /* Multiply by (2^9 + 2^8 + 2^7 + 2^6 + 2^4 + 1). */
    muladd(0xD1, right[0], &r0, &r1, &r2);
    result[0] = r0;
    r0 = r1;
    r1 = r2;
    /* r2 is still 0 */
    
    for (k = 1; k < num_words_secp256k1; ++k) {
        muladd(0x03, right[k - 1], &r0, &r1, &r2);
        muladd(0xD1, right[k], &r0, &r1, &r2);
        result[k] = r0;
        r0 = r1;
        r1 = r2;
        r2 = 0;
    }
    muladd(0x03, right[num_words_secp256k1 - 1], &r0, &r1, &r2);
    result[num_words_secp256k1] = r0;
    result[num_words_secp256k1 + 1] = r1;
    /* add the 2^32 multiple */
    result[4 + num_words_secp256k1] =
        uECC_vli_add(result + 4, result + 4, right, num_words_secp256k1); 
}
#elif uECC_WORD_SIZE == 4
static void omega_mult_secp256k1(uint32_t * result, const uint32_t * right) {
    /* Multiply by (2^9 + 2^8 + 2^7 + 2^6 + 2^4 + 1). */
    uint32_t carry = 0;
    wordcount_t k;
    
    for (k = 0; k < num_words_secp256k1; ++k) {
        uint64_t p = (uint64_t)0x3D1 * right[k] + carry;
        result[k] = (uint32_t) p;
        carry = p >> 32;
    }
    result[num_words_secp256k1] = carry;
    /* add the 2^32 multiple */
    result[1 + num_words_secp256k1] =
        uECC_vli_add(result + 1, result + 1, right, num_words_secp256k1); 
}
#else
static void omega_mult_secp256k1(uint64_t * result, const uint64_t * right) {
    uECC_word_t r0 = 0;
    uECC_word_t r1 = 0;
    uECC_word_t r2 = 0;
    wordcount_t k;
    
    /* Multiply by (2^32 + 2^9 + 2^8 + 2^7 + 2^6 + 2^4 + 1). */
    for (k = 0; k < num_words_secp256k1; ++k) {
        muladd(0x1000003D1ull, right[k], &r0, &r1, &r2);
        result[k] = r0;
        r0 = r1;
        r1 = r2;
        r2 = 0;
    }
    result[num_words_secp256k1] = r0;
}
#endif /* uECC_WORD_SIZE */
#endif /* (uECC_OPTIMIZATION_LEVEL > 0 &&  && !asm_mmod_fast_secp256k1) */

#endif /* uECC_SUPPORTS_secp256k1 */

#endif /* _UECC_CURVE_SPECIFIC_H_ */
