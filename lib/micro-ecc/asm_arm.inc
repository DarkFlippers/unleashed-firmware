/* Copyright 2015, Kenneth MacKay. Licensed under the BSD 2-clause license. */

#ifndef _UECC_ASM_ARM_H_
#define _UECC_ASM_ARM_H_

#if (uECC_SUPPORTS_secp256r1 || uECC_SUPPORTS_secp256k1)
    #define uECC_MIN_WORDS 8
#endif
#if uECC_SUPPORTS_secp224r1
    #undef uECC_MIN_WORDS
    #define uECC_MIN_WORDS 7
#endif
#if uECC_SUPPORTS_secp192r1
    #undef uECC_MIN_WORDS
    #define uECC_MIN_WORDS 6
#endif
#if uECC_SUPPORTS_secp160r1
    #undef uECC_MIN_WORDS
    #define uECC_MIN_WORDS 5
#endif

#if (uECC_PLATFORM == uECC_arm_thumb)
    #define REG_RW "+l"
    #define REG_WRITE "=l"
#else
    #define REG_RW "+r"
    #define REG_WRITE "=r"
#endif

#if (uECC_PLATFORM == uECC_arm_thumb || uECC_PLATFORM == uECC_arm_thumb2)
    #define REG_RW_LO "+l"
    #define REG_WRITE_LO "=l"
#else
    #define REG_RW_LO "+r"
    #define REG_WRITE_LO "=r"
#endif

#if (uECC_PLATFORM == uECC_arm_thumb2)
    #define RESUME_SYNTAX
#else
    #define RESUME_SYNTAX ".syntax divided \n\t"
#endif

#if (uECC_OPTIMIZATION_LEVEL >= 2)

uECC_VLI_API uECC_word_t uECC_vli_add(uECC_word_t *result,
                                      const uECC_word_t *left,
                                      const uECC_word_t *right,
                                      wordcount_t num_words) {
#if (uECC_MAX_WORDS != uECC_MIN_WORDS)
  #if (uECC_PLATFORM == uECC_arm_thumb) || (uECC_PLATFORM == uECC_arm_thumb2)
    uint32_t jump = (uECC_MAX_WORDS - num_words) * 4 * 2 + 1;
  #else /* ARM */
    uint32_t jump = (uECC_MAX_WORDS - num_words) * 4 * 4;
  #endif
#endif
    uint32_t carry;
    uint32_t left_word;
    uint32_t right_word;
    
    __asm__ volatile (
        ".syntax unified \n\t"
        "movs %[carry], #0 \n\t"
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        "adr %[left], 1f \n\t"
        ".align 4 \n\t"
        "adds %[jump], %[left] \n\t"
    #endif
        
        "ldmia %[lptr]!, {%[left]} \n\t"
        "ldmia %[rptr]!, {%[right]} \n\t"
        "adds %[left], %[right] \n\t"
        "stmia %[dptr]!, {%[left]} \n\t"
        
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        "bx %[jump] \n\t"
    #endif
        "1: \n\t"
        REPEAT(DEC(uECC_MAX_WORDS),
            "ldmia %[lptr]!, {%[left]} \n\t"
            "ldmia %[rptr]!, {%[right]} \n\t"
            "adcs %[left], %[right] \n\t"
            "stmia %[dptr]!, {%[left]} \n\t")
        
        "adcs %[carry], %[carry] \n\t"
        RESUME_SYNTAX
        : [dptr] REG_RW_LO (result), [lptr] REG_RW_LO (left), [rptr] REG_RW_LO (right),
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
          [jump] REG_RW_LO (jump),
    #endif
          [carry] REG_WRITE_LO (carry), [left] REG_WRITE_LO (left_word),
          [right] REG_WRITE_LO (right_word)
        :
        : "cc", "memory"
    );
    return carry;
}
#define asm_add 1

uECC_VLI_API uECC_word_t uECC_vli_sub(uECC_word_t *result,
                                      const uECC_word_t *left,
                                      const uECC_word_t *right,
                                      wordcount_t num_words) {
#if (uECC_MAX_WORDS != uECC_MIN_WORDS)
  #if (uECC_PLATFORM == uECC_arm_thumb) || (uECC_PLATFORM == uECC_arm_thumb2)
    uint32_t jump = (uECC_MAX_WORDS - num_words) * 4 * 2 + 1;
  #else /* ARM */
    uint32_t jump = (uECC_MAX_WORDS - num_words) * 4 * 4;
  #endif
#endif
    uint32_t carry;
    uint32_t left_word;
    uint32_t right_word;
    
    __asm__ volatile (
        ".syntax unified \n\t"
        "movs %[carry], #0 \n\t"
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        "adr %[left], 1f \n\t"
        ".align 4 \n\t"
        "adds %[jump], %[left] \n\t"
    #endif
        
        "ldmia %[lptr]!, {%[left]} \n\t"
        "ldmia %[rptr]!, {%[right]} \n\t"
        "subs %[left], %[right] \n\t"
        "stmia %[dptr]!, {%[left]} \n\t"
        
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        "bx %[jump] \n\t"
    #endif
        "1: \n\t"
        REPEAT(DEC(uECC_MAX_WORDS),
            "ldmia %[lptr]!, {%[left]} \n\t"
            "ldmia %[rptr]!, {%[right]} \n\t"
            "sbcs %[left], %[right] \n\t"
            "stmia %[dptr]!, {%[left]} \n\t")
        
        "adcs %[carry], %[carry] \n\t"
        RESUME_SYNTAX
        : [dptr] REG_RW_LO (result), [lptr] REG_RW_LO (left), [rptr] REG_RW_LO (right),
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
          [jump] REG_RW_LO (jump),
    #endif
          [carry] REG_WRITE_LO (carry), [left] REG_WRITE_LO (left_word),
          [right] REG_WRITE_LO (right_word)
        :
        : "cc", "memory"
    );
    return !carry; /* Note that on ARM, carry flag set means "no borrow" when subtracting
                      (for some reason...) */
}
#define asm_sub 1

#endif /* (uECC_OPTIMIZATION_LEVEL >= 2) */

#if (uECC_OPTIMIZATION_LEVEL >= 3)

#if (uECC_PLATFORM != uECC_arm_thumb)

#if uECC_ARM_USE_UMAAL
    #include "asm_arm_mult_square_umaal.inc"
#else
    #include "asm_arm_mult_square.inc"
#endif

#if (uECC_OPTIMIZATION_LEVEL == 3)

uECC_VLI_API void uECC_vli_mult(uint32_t *result,
                                const uint32_t *left,
                                const uint32_t *right,
                                wordcount_t num_words) {
    register uint32_t *r0 __asm__("r0") = result;
    register const uint32_t *r1 __asm__("r1") = left;
    register const uint32_t *r2 __asm__("r2") = right;
    register uint32_t r3 __asm__("r3") = num_words;
    
    __asm__ volatile (
        ".syntax unified \n\t"
#if (uECC_MIN_WORDS == 5)
        FAST_MULT_ASM_5
    #if (uECC_MAX_WORDS > 5)
        FAST_MULT_ASM_5_TO_6
    #endif
    #if (uECC_MAX_WORDS > 6)
        FAST_MULT_ASM_6_TO_7
    #endif
    #if (uECC_MAX_WORDS > 7)
        FAST_MULT_ASM_7_TO_8
    #endif
#elif (uECC_MIN_WORDS == 6)
        FAST_MULT_ASM_6
    #if (uECC_MAX_WORDS > 6)
        FAST_MULT_ASM_6_TO_7
    #endif
    #if (uECC_MAX_WORDS > 7)
        FAST_MULT_ASM_7_TO_8
    #endif
#elif (uECC_MIN_WORDS == 7)
        FAST_MULT_ASM_7
    #if (uECC_MAX_WORDS > 7)
        FAST_MULT_ASM_7_TO_8
    #endif
#elif (uECC_MIN_WORDS == 8)
        FAST_MULT_ASM_8
#endif
        "1: \n\t"
        RESUME_SYNTAX
        : "+r" (r0), "+r" (r1), "+r" (r2)
        : "r" (r3)
        : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
    );
}
#define asm_mult 1

#if uECC_SQUARE_FUNC
uECC_VLI_API void uECC_vli_square(uECC_word_t *result,
                                  const uECC_word_t *left,
                                  wordcount_t num_words) {
    register uint32_t *r0 __asm__("r0") = result;
    register const uint32_t *r1 __asm__("r1") = left;
    register uint32_t r2 __asm__("r2") = num_words;
    
    __asm__ volatile (
        ".syntax unified \n\t"
#if (uECC_MIN_WORDS == 5)
        FAST_SQUARE_ASM_5
    #if (uECC_MAX_WORDS > 5)
        FAST_SQUARE_ASM_5_TO_6
    #endif
    #if (uECC_MAX_WORDS > 6)
        FAST_SQUARE_ASM_6_TO_7
    #endif
    #if (uECC_MAX_WORDS > 7)
        FAST_SQUARE_ASM_7_TO_8
    #endif
#elif (uECC_MIN_WORDS == 6)
        FAST_SQUARE_ASM_6
    #if (uECC_MAX_WORDS > 6)
        FAST_SQUARE_ASM_6_TO_7
    #endif
    #if (uECC_MAX_WORDS > 7)
        FAST_SQUARE_ASM_7_TO_8
    #endif
#elif (uECC_MIN_WORDS == 7)
        FAST_SQUARE_ASM_7
    #if (uECC_MAX_WORDS > 7)
        FAST_SQUARE_ASM_7_TO_8
    #endif
#elif (uECC_MIN_WORDS == 8)
        FAST_SQUARE_ASM_8
#endif

        "1: \n\t"
        RESUME_SYNTAX
        : "+r" (r0), "+r" (r1)
        : "r" (r2)
        : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
    );
}
#define asm_square 1
#endif /* uECC_SQUARE_FUNC */

#else /* (uECC_OPTIMIZATION_LEVEL > 3) */

uECC_VLI_API void uECC_vli_mult(uint32_t *result,
                                const uint32_t *left,
                                const uint32_t *right,
                                wordcount_t num_words) {
    register uint32_t *r0 __asm__("r0") = result;
    register const uint32_t *r1 __asm__("r1") = left;
    register const uint32_t *r2 __asm__("r2") = right;
    register uint32_t r3 __asm__("r3") = num_words;
    
#if uECC_SUPPORTS_secp160r1
    if (num_words == 5) {
        __asm__ volatile (
            ".syntax unified \n\t"
            FAST_MULT_ASM_5
            RESUME_SYNTAX
            : "+r" (r0), "+r" (r1), "+r" (r2)
            : "r" (r3)
            : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
        );
        return;
    }
#endif
#if uECC_SUPPORTS_secp192r1
    if (num_words == 6) {
        __asm__ volatile (
            ".syntax unified \n\t"
            FAST_MULT_ASM_6
            RESUME_SYNTAX
            : "+r" (r0), "+r" (r1), "+r" (r2)
            : "r" (r3)
            : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
        );
        return;
    }
#endif
#if uECC_SUPPORTS_secp224r1
    if (num_words == 7) {
        __asm__ volatile (
            ".syntax unified \n\t"
            FAST_MULT_ASM_7
            RESUME_SYNTAX
            : "+r" (r0), "+r" (r1), "+r" (r2)
            : "r" (r3)
            : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
        );
        return;
    }
#endif
#if (uECC_SUPPORTS_secp256r1 || uECC_SUPPORTS_secp256k1)
    if (num_words == 8) {
        __asm__ volatile (
            ".syntax unified \n\t"
            FAST_MULT_ASM_8
            RESUME_SYNTAX
            : "+r" (r0), "+r" (r1), "+r" (r2)
            : "r" (r3)
            : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
        );
        return;
    }
#endif
}
#define asm_mult 1

#if uECC_SQUARE_FUNC
uECC_VLI_API void uECC_vli_square(uECC_word_t *result,
                                  const uECC_word_t *left,
                                  wordcount_t num_words) {
    register uint32_t *r0 __asm__("r0") = result;
    register const uint32_t *r1 __asm__("r1") = left;
    register uint32_t r2 __asm__("r2") = num_words;
    
#if uECC_SUPPORTS_secp160r1
    if (num_words == 5) {
        __asm__ volatile (
            ".syntax unified \n\t"
            FAST_SQUARE_ASM_5
            RESUME_SYNTAX
            : "+r" (r0), "+r" (r1)
            : "r" (r2)
            : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
        );
        return;
    }
#endif
#if uECC_SUPPORTS_secp192r1
    if (num_words == 6) {
        __asm__ volatile (
            ".syntax unified \n\t"
            FAST_SQUARE_ASM_6
            RESUME_SYNTAX
            : "+r" (r0), "+r" (r1)
            : "r" (r2)
            : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
        );
        return;
    }
#endif
#if uECC_SUPPORTS_secp224r1
    if (num_words == 7) {
        __asm__ volatile (
            ".syntax unified \n\t"
            FAST_SQUARE_ASM_7
            RESUME_SYNTAX
            : "+r" (r0), "+r" (r1)
            : "r" (r2)
            : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
        );
        return;
    }
#endif
#if (uECC_SUPPORTS_secp256r1 || uECC_SUPPORTS_secp256k1)
    if (num_words == 8) {
        __asm__ volatile (
            ".syntax unified \n\t"
            FAST_SQUARE_ASM_8
            RESUME_SYNTAX
            : "+r" (r0), "+r" (r1)
            : "r" (r2)
            : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
        );
        return;
    }
#endif
}
#define asm_square 1
#endif /* uECC_SQUARE_FUNC */

#endif /* (uECC_OPTIMIZATION_LEVEL > 3) */

#endif /* uECC_PLATFORM != uECC_arm_thumb */

#endif /* (uECC_OPTIMIZATION_LEVEL >= 3) */

/* ---- "Small" implementations ---- */

#if !asm_add
uECC_VLI_API uECC_word_t uECC_vli_add(uECC_word_t *result,
                                      const uECC_word_t *left,
                                      const uECC_word_t *right,
                                      wordcount_t num_words) {
    uint32_t carry = 0;
    uint32_t left_word;
    uint32_t right_word;
    
    __asm__ volatile (
        ".syntax unified \n\t"
        "1: \n\t"
        "ldmia %[lptr]!, {%[left]} \n\t"  /* Load left word. */
        "ldmia %[rptr]!, {%[right]} \n\t" /* Load right word. */
        "lsrs %[carry], #1 \n\t"          /* Set up carry flag (carry = 0 after this). */
        "adcs %[left], %[left], %[right] \n\t"   /* Add with carry. */
        "adcs %[carry], %[carry], %[carry] \n\t" /* Store carry bit. */
        "stmia %[dptr]!, {%[left]} \n\t"  /* Store result word. */
        "subs %[ctr], #1 \n\t"            /* Decrement counter. */
        "bne 1b \n\t"                     /* Loop until counter == 0. */
        RESUME_SYNTAX
        : [dptr] REG_RW (result), [lptr] REG_RW (left), [rptr] REG_RW (right),
          [ctr] REG_RW (num_words), [carry] REG_RW (carry),
          [left] REG_WRITE (left_word), [right] REG_WRITE (right_word)
        :
        : "cc", "memory"
    );
    return carry;
}
#define asm_add 1
#endif

#if !asm_sub
uECC_VLI_API uECC_word_t uECC_vli_sub(uECC_word_t *result,
                                      const uECC_word_t *left,
                                      const uECC_word_t *right,
                                      wordcount_t num_words) {
    uint32_t carry = 1; /* carry = 1 initially (means don't borrow) */
    uint32_t left_word;
    uint32_t right_word;
    
    __asm__ volatile (
        ".syntax unified \n\t"
        "1: \n\t"
        "ldmia %[lptr]!, {%[left]} \n\t"  /* Load left word. */
        "ldmia %[rptr]!, {%[right]} \n\t" /* Load right word. */
        "lsrs %[carry], #1 \n\t"          /* Set up carry flag (carry = 0 after this). */
        "sbcs %[left], %[left], %[right] \n\t"   /* Subtract with borrow. */
        "adcs %[carry], %[carry], %[carry] \n\t" /* Store carry bit. */
        "stmia %[dptr]!, {%[left]} \n\t"  /* Store result word. */
        "subs %[ctr], #1 \n\t"            /* Decrement counter. */
        "bne 1b \n\t"                     /* Loop until counter == 0. */
        RESUME_SYNTAX
        : [dptr] REG_RW (result), [lptr] REG_RW (left), [rptr] REG_RW (right),
          [ctr] REG_RW (num_words), [carry] REG_RW (carry),
          [left] REG_WRITE (left_word), [right] REG_WRITE (right_word)
        :
        : "cc", "memory"
    );
    return !carry;
}
#define asm_sub 1
#endif

#if !asm_mult
uECC_VLI_API void uECC_vli_mult(uECC_word_t *result,
                                const uECC_word_t *left,
                                const uECC_word_t *right,
                                wordcount_t num_words) {
#if (uECC_PLATFORM != uECC_arm_thumb)
    uint32_t c0 = 0;
    uint32_t c1 = 0;
    uint32_t c2 = 0;
    uint32_t k = 0;
    uint32_t i;
    uint32_t t0, t1;
    
    __asm__ volatile (
        ".syntax unified \n\t"
        
        "1: \n\t" /* outer loop (k < num_words) */
        "movs %[i], #0 \n\t" /* i = 0 */
        "b 3f \n\t"
        
        "2: \n\t" /* outer loop (k >= num_words) */
        "movs %[i], %[k] \n\t"         /* i = k */
        "subs %[i], %[last_word] \n\t" /* i = k - (num_words - 1) (times 4) */
        
        "3: \n\t" /* inner loop */
        "subs %[t0], %[k], %[i] \n\t" /* t0 = k-i */
        
        "ldr %[t1], [%[right], %[t0]] \n\t" /* t1 = right[k - i] */
        "ldr %[t0], [%[left], %[i]] \n\t"   /* t0 = left[i] */
        
        "umull %[t0], %[t1], %[t0], %[t1] \n\t" /* (t0, t1) = left[i] * right[k - i] */
        
        "adds %[c0], %[c0], %[t0] \n\t" /* add low word to c0 */
        "adcs %[c1], %[c1], %[t1] \n\t" /* add high word to c1, including carry */
        "adcs %[c2], %[c2], #0 \n\t"    /* add carry to c2 */

        "adds %[i], #4 \n\t"          /* i += 4 */
        "cmp %[i], %[last_word] \n\t" /* i > (num_words - 1) (times 4)? */
        "bgt 4f \n\t"                 /*   if so, exit the loop */
        "cmp %[i], %[k] \n\t"         /* i <= k? */
        "ble 3b \n\t"                 /*   if so, continue looping */
        
        "4: \n\t" /* end inner loop */
        
        "str %[c0], [%[result], %[k]] \n\t" /* result[k] = c0 */
        "mov %[c0], %[c1] \n\t"       /* c0 = c1 */
        "mov %[c1], %[c2] \n\t"       /* c1 = c2 */
        "movs %[c2], #0 \n\t"         /* c2 = 0 */
        "adds %[k], #4 \n\t"          /* k += 4 */
        "cmp %[k], %[last_word] \n\t" /* k <= (num_words - 1) (times 4) ? */
        "ble 1b \n\t"                 /*   if so, loop back, start with i = 0 */
        "cmp %[k], %[last_word], lsl #1 \n\t" /* k <= (num_words * 2 - 2) (times 4) ? */
        "ble 2b \n\t"                 /*   if so, loop back, start with i = (k + 1) - num_words */
        /* end outer loop */
        
        "str %[c0], [%[result], %[k]] \n\t" /* result[num_words * 2 - 1] = c0 */
        RESUME_SYNTAX
        : [c0] "+r" (c0), [c1] "+r" (c1), [c2] "+r" (c2),
          [k] "+r" (k), [i] "=&r" (i), [t0] "=&r" (t0), [t1] "=&r" (t1)
        : [result] "r" (result), [left] "r" (left), [right] "r" (right),
          [last_word] "r" ((num_words - 1) * 4)
        : "cc", "memory"
    );
    
#else /* Thumb-1 */
    uint32_t r4, r5, r6, r7;

    __asm__ volatile (
        ".syntax unified \n\t"
        "subs %[r3], #1 \n\t" /* r3 = num_words - 1 */
        "lsls %[r3], #2 \n\t" /* r3 = (num_words - 1) * 4 */
        "mov r8, %[r3] \n\t"  /* r8 = (num_words - 1) * 4 */
        "lsls %[r3], #1 \n\t" /* r3 = (num_words - 1) * 8 */
        "mov r9, %[r3] \n\t"  /* r9 = (num_words - 1) * 8 */
        "movs %[r3], #0 \n\t" /* c0 = 0 */
        "movs %[r4], #0 \n\t" /* c1 = 0 */
        "movs %[r5], #0 \n\t" /* c2 = 0 */
        "movs %[r6], #0 \n\t" /* k = 0 */
        
        "push {%[r0]} \n\t" /* keep result on the stack */
        
        "1: \n\t" /* outer loop (k < num_words) */
        "movs %[r7], #0 \n\t" /* r7 = i = 0 */
        "b 3f \n\t"
        
        "2: \n\t" /* outer loop (k >= num_words) */
        "movs %[r7], %[r6] \n\t" /* r7 = k */
        "mov %[r0], r8 \n\t"     /* r0 = (num_words - 1) * 4 */
        "subs %[r7], %[r0] \n\t" /* r7 = i = k - (num_words - 1) (times 4) */
        
        "3: \n\t" /* inner loop */
        "mov r10, %[r3] \n\t"
        "mov r11, %[r4] \n\t"
        "mov r12, %[r5] \n\t"
        "mov r14, %[r6] \n\t"
        "subs %[r0], %[r6], %[r7] \n\t"          /* r0 = k - i */
        
        "ldr %[r4], [%[r2], %[r0]] \n\t" /* r4 = right[k - i] */
        "ldr %[r0], [%[r1], %[r7]] \n\t" /* r0 = left[i] */
        
        "lsrs %[r3], %[r0], #16 \n\t" /* r3 = a1 */
        "uxth %[r0], %[r0] \n\t"      /* r0 = a0 */
        
        "lsrs %[r5], %[r4], #16 \n\t" /* r5 = b1 */
        "uxth %[r4], %[r4] \n\t"      /* r4 = b0 */
        
        "movs %[r6], %[r3] \n\t"        /* r6 = a1 */
        "muls %[r6], %[r5], %[r6] \n\t" /* r6 = a1 * b1 */
        "muls %[r3], %[r4], %[r3] \n\t" /* r3 = b0 * a1 */
        "muls %[r5], %[r0], %[r5] \n\t" /* r5 = a0 * b1 */
        "muls %[r0], %[r4], %[r0] \n\t" /* r0 = a0 * b0 */
        
        /* Add middle terms */
        "lsls %[r4], %[r3], #16 \n\t"
        "lsrs %[r3], %[r3], #16 \n\t"
        "adds %[r0], %[r4] \n\t"
        "adcs %[r6], %[r3] \n\t"
        
        "lsls %[r4], %[r5], #16 \n\t"
        "lsrs %[r5], %[r5], #16 \n\t"
        "adds %[r0], %[r4] \n\t"
        "adcs %[r6], %[r5] \n\t"
        
        "mov %[r3], r10\n\t"
        "mov %[r4], r11\n\t"
        "mov %[r5], r12\n\t"
        "adds %[r3], %[r0] \n\t"         /* add low word to c0 */
        "adcs %[r4], %[r6] \n\t"         /* add high word to c1, including carry */
        "movs %[r0], #0 \n\t"            /* r0 = 0 (does not affect carry bit) */
        "adcs %[r5], %[r0] \n\t"         /* add carry to c2 */
        
        "mov %[r6], r14\n\t" /* r6 = k */

        "adds %[r7], #4 \n\t"   /* i += 4 */
        "cmp %[r7], r8 \n\t"    /* i > (num_words - 1) (times 4)? */
        "bgt 4f \n\t"           /*   if so, exit the loop */
        "cmp %[r7], %[r6] \n\t" /* i <= k? */
        "ble 3b \n\t"           /*   if so, continue looping */
        
        "4: \n\t" /* end inner loop */
        
        "ldr %[r0], [sp, #0] \n\t" /* r0 = result */
        
        "str %[r3], [%[r0], %[r6]] \n\t" /* result[k] = c0 */
        "mov %[r3], %[r4] \n\t"          /* c0 = c1 */
        "mov %[r4], %[r5] \n\t"          /* c1 = c2 */
        "movs %[r5], #0 \n\t"            /* c2 = 0 */
        "adds %[r6], #4 \n\t"            /* k += 4 */
        "cmp %[r6], r8 \n\t"             /* k <= (num_words - 1) (times 4) ? */
        "ble 1b \n\t"                    /*   if so, loop back, start with i = 0 */
        "cmp %[r6], r9 \n\t"             /* k <= (num_words * 2 - 2) (times 4) ? */
        "ble 2b \n\t"                    /*   if so, loop back, with i = (k + 1) - num_words */
        /* end outer loop */
        
        "str %[r3], [%[r0], %[r6]] \n\t" /* result[num_words * 2 - 1] = c0 */
        "pop {%[r0]} \n\t"               /* pop result off the stack */
        
        ".syntax divided \n\t"
        : [r3] "+l" (num_words), [r4] "=&l" (r4),
          [r5] "=&l" (r5), [r6] "=&l" (r6), [r7] "=&l" (r7)
        : [r0] "l" (result), [r1] "l" (left), [r2] "l" (right)
        : "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
    );
#endif
}
#define asm_mult 1
#endif

#if uECC_SQUARE_FUNC
#if !asm_square
uECC_VLI_API void uECC_vli_square(uECC_word_t *result,
                                  const uECC_word_t *left,
                                  wordcount_t num_words) {
#if (uECC_PLATFORM != uECC_arm_thumb)
    uint32_t c0 = 0;
    uint32_t c1 = 0;
    uint32_t c2 = 0;
    uint32_t k = 0;
    uint32_t i, tt;
    uint32_t t0, t1;
    
    __asm__ volatile (
        ".syntax unified \n\t"
        
        "1: \n\t" /* outer loop (k < num_words) */
        "movs %[i], #0 \n\t" /* i = 0 */
        "b 3f \n\t"
        
        "2: \n\t" /* outer loop (k >= num_words) */
        "movs %[i], %[k] \n\t"         /* i = k */
        "subs %[i], %[last_word] \n\t" /* i = k - (num_words - 1) (times 4) */
        
        "3: \n\t" /* inner loop */
        "subs %[tt], %[k], %[i] \n\t" /* tt = k-i */
        
        "ldr %[t1], [%[left], %[tt]] \n\t" /* t1 = left[k - i] */
        "ldr %[t0], [%[left], %[i]] \n\t"  /* t0 = left[i] */
        
        "umull %[t0], %[t1], %[t0], %[t1] \n\t" /* (t0, t1) = left[i] * right[k - i] */
        
        "cmp %[i], %[tt] \n\t"      /* (i < k - i) ? */
        "bge 4f \n\t"               /*   if i >= k - i, skip */
        "adds %[c0], %[c0], %[t0] \n\t" /* add low word to c0 */
        "adcs %[c1], %[c1], %[t1] \n\t" /* add high word to c1, including carry */
        "adcs %[c2], %[c2], #0 \n\t"    /* add carry to c2 */
        
        "4: \n\t"
        "adds %[c0], %[c0], %[t0] \n\t" /* add low word to c0 */
        "adcs %[c1], %[c1], %[t1] \n\t" /* add high word to c1, including carry */
        "adcs %[c2], %[c2], #0 \n\t"    /* add carry to c2 */
        
        "adds %[i], #4 \n\t"          /* i += 4 */
        "cmp %[i], %[k] \n\t"         /* i >= k? */
        "bge 5f \n\t"                 /*   if so, exit the loop */
        "subs %[tt], %[k], %[i] \n\t" /* tt = k - i */
        "cmp %[i], %[tt] \n\t"        /* i <= k - i? */
        "ble 3b \n\t"                 /*   if so, continue looping */
        
        "5: \n\t" /* end inner loop */
        
        "str %[c0], [%[result], %[k]] \n\t" /* result[k] = c0 */
        "mov %[c0], %[c1] \n\t"       /* c0 = c1 */
        "mov %[c1], %[c2] \n\t"       /* c1 = c2 */
        "movs %[c2], #0 \n\t"         /* c2 = 0 */
        "adds %[k], #4 \n\t"          /* k += 4 */
        "cmp %[k], %[last_word] \n\t" /* k <= (num_words - 1) (times 4) ? */
        "ble 1b \n\t"                 /*   if so, loop back, start with i = 0 */
        "cmp %[k], %[last_word], lsl #1 \n\t" /* k <= (num_words * 2 - 2) (times 4) ? */
        "ble 2b \n\t"                 /*   if so, loop back, start with i = (k + 1) - num_words */
        /* end outer loop */
        
        "str %[c0], [%[result], %[k]] \n\t" /* result[num_words * 2 - 1] = c0 */
        RESUME_SYNTAX
        : [c0] "+r" (c0), [c1] "+r" (c1), [c2] "+r" (c2),
          [k] "+r" (k), [i] "=&r" (i), [tt] "=&r" (tt), [t0] "=&r" (t0), [t1] "=&r" (t1)
        : [result] "r" (result), [left] "r" (left), [last_word] "r" ((num_words - 1) * 4)
        : "cc", "memory"
    );
    
#else
    uint32_t r3, r4, r5, r6, r7;

    __asm__ volatile (
        ".syntax unified \n\t"
        "subs %[r2], #1 \n\t" /* r2 = num_words - 1 */
        "lsls %[r2], #2 \n\t" /* r2 = (num_words - 1) * 4 */
        "mov r8, %[r2] \n\t"  /* r8 = (num_words - 1) * 4 */
        "lsls %[r2], #1 \n\t" /* r2 = (num_words - 1) * 8 */
        "mov r9, %[r2] \n\t"  /* r9 = (num_words - 1) * 8 */
        "movs %[r2], #0 \n\t" /* c0 = 0 */
        "movs %[r3], #0 \n\t" /* c1 = 0 */
        "movs %[r4], #0 \n\t" /* c2 = 0 */
        "movs %[r5], #0 \n\t" /* k = 0 */
        
        "push {%[r0]} \n\t" /* keep result on the stack */
        
        "1: \n\t" /* outer loop (k < num_words) */
        "movs %[r6], #0 \n\t" /* r6 = i = 0 */
        "b 3f \n\t"
        
        "2: \n\t" /* outer loop (k >= num_words) */
        "movs %[r6], %[r5] \n\t" /* r6 = k */
        "mov %[r0], r8 \n\t"     /* r0 = (num_words - 1) * 4 */
        "subs %[r6], %[r0] \n\t" /* r6 = i = k - (num_words - 1) (times 4) */
        
        "3: \n\t" /* inner loop */
        "mov r10, %[r2] \n\t"
        "mov r11, %[r3] \n\t"
        "mov r12, %[r4] \n\t"
        "mov r14, %[r5] \n\t"
        "subs %[r7], %[r5], %[r6] \n\t"  /* r7 = k - i */
        
        "ldr %[r3], [%[r1], %[r7]] \n\t" /* r3 = left[k - i] */
        "ldr %[r0], [%[r1], %[r6]] \n\t" /* r0 = left[i] */
        
        "lsrs %[r2], %[r0], #16 \n\t" /* r2 = a1 */
        "uxth %[r0], %[r0] \n\t"      /* r0 = a0 */
        
        "lsrs %[r4], %[r3], #16 \n\t" /* r4 = b1 */
        "uxth %[r3], %[r3] \n\t"      /* r3 = b0 */
        
        "movs %[r5], %[r2] \n\t"        /* r5 = a1 */
        "muls %[r5], %[r4], %[r5] \n\t" /* r5 = a1 * b1 */
        "muls %[r2], %[r3], %[r2] \n\t" /* r2 = b0 * a1 */
        "muls %[r4], %[r0], %[r4] \n\t" /* r4 = a0 * b1 */
        "muls %[r0], %[r3], %[r0] \n\t" /* r0 = a0 * b0 */
        
        /* Add middle terms */
        "lsls %[r3], %[r2], #16 \n\t"
        "lsrs %[r2], %[r2], #16 \n\t"
        "adds %[r0], %[r3] \n\t"
        "adcs %[r5], %[r2] \n\t"
        
        "lsls %[r3], %[r4], #16 \n\t"
        "lsrs %[r4], %[r4], #16 \n\t"
        "adds %[r0], %[r3] \n\t"
        "adcs %[r5], %[r4] \n\t"
        
        /* Add to acc, doubling if necessary */
        "mov %[r2], r10\n\t"
        "mov %[r3], r11\n\t"
        "mov %[r4], r12\n\t"
        
        "cmp %[r6], %[r7] \n\t"    /* (i < k - i) ? */
        "bge 4f \n\t"            /*   if i >= k - i, skip */
        "movs %[r7], #0 \n\t"    /* r7 = 0 */
        "adds %[r2], %[r0] \n\t" /* add low word to c0 */
        "adcs %[r3], %[r5] \n\t" /* add high word to c1, including carry */
        "adcs %[r4], %[r7] \n\t" /* add carry to c2 */
        "4: \n\t"
        "movs %[r7], #0 \n\t"    /* r7 = 0 */
        "adds %[r2], %[r0] \n\t" /* add low word to c0 */
        "adcs %[r3], %[r5] \n\t" /* add high word to c1, including carry */
        "adcs %[r4], %[r7] \n\t" /* add carry to c2 */
        
        "mov %[r5], r14\n\t" /* r5 = k */
        
        "adds %[r6], #4 \n\t"           /* i += 4 */
        "cmp %[r6], %[r5] \n\t"         /* i >= k? */
        "bge 5f \n\t"                   /*   if so, exit the loop */
        "subs %[r7], %[r5], %[r6] \n\t" /* r7 = k - i */
        "cmp %[r6], %[r7] \n\t"         /* i <= k - i? */
        "ble 3b \n\t"                   /*   if so, continue looping */
        
        "5: \n\t" /* end inner loop */
        
        "ldr %[r0], [sp, #0] \n\t" /* r0 = result */
        
        "str %[r2], [%[r0], %[r5]] \n\t" /* result[k] = c0 */
        "mov %[r2], %[r3] \n\t"          /* c0 = c1 */
        "mov %[r3], %[r4] \n\t"          /* c1 = c2 */
        "movs %[r4], #0 \n\t"            /* c2 = 0 */
        "adds %[r5], #4 \n\t"            /* k += 4 */
        "cmp %[r5], r8 \n\t"             /* k <= (num_words - 1) (times 4) ? */
        "ble 1b \n\t"                    /*   if so, loop back, start with i = 0 */
        "cmp %[r5], r9 \n\t"             /* k <= (num_words * 2 - 2) (times 4) ? */
        "ble 2b \n\t"                    /*   if so, loop back, with i = (k + 1) - num_words */
        /* end outer loop */
        
        "str %[r2], [%[r0], %[r5]] \n\t" /* result[num_words * 2 - 1] = c0 */
        "pop {%[r0]} \n\t"               /* pop result off the stack */

        ".syntax divided \n\t"
        : [r2] "+l" (num_words), [r3] "=&l" (r3), [r4] "=&l" (r4),
          [r5] "=&l" (r5), [r6] "=&l" (r6), [r7] "=&l" (r7)
        : [r0] "l" (result), [r1] "l" (left)
        : "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
    );
#endif
}
#define asm_square 1
#endif
#endif /* uECC_SQUARE_FUNC */

#endif /* _UECC_ASM_ARM_H_ */
