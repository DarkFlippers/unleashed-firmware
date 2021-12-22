/* Copyright 2015, Kenneth MacKay. Licensed under the BSD 2-clause license. */

#ifndef _UECC_ASM_ARM_MULT_SQUARE_H_
#define _UECC_ASM_ARM_MULT_SQUARE_H_

#define FAST_MULT_ASM_5                     \
    "push   {r3} \n\t"                      \
    "ldmia  r2!, {r3, r4, r5, r6, r7} \n\t" \
    "push   {r2} \n\t"                      \
                                            \
    "ldr    r2, [r1], #4 \n\t"              \
    "umull  r8, r9, r3, r2 \n\t"            \
    "str    r8, [r0], #4 \n\t"              \
    "mov    r10, #0 \n\t"                   \
    "umaal  r9, r10, r4, r2 \n\t"           \
    "mov    r11, #0 \n\t"                   \
    "umaal  r10, r11, r5, r2 \n\t"          \
    "mov    r12, #0 \n\t"                   \
    "umaal  r11, r12, r6, r2 \n\t"          \
    "mov    r14, #0 \n\t"                   \
    "umaal  r12, r14, r7, r2 \n\t"          \
                                            \
    "ldr    r2, [r1], #4 \n\t"              \
    "mov    r8, #0 \n\t"                    \
    "umaal  r8, r9, r3, r2 \n\t"            \
    "str    r8, [r0], #4 \n\t"              \
    "umaal  r9, r10, r4, r2 \n\t"           \
    "umaal  r10, r11, r5, r2 \n\t"          \
    "umaal  r11, r12, r6, r2 \n\t"          \
    "umaal  r12, r14, r7, r2 \n\t"          \
                                            \
    "ldr    r2, [r1], #4 \n\t"              \
    "mov    r8, #0 \n\t"                    \
    "umaal  r8, r9, r3, r2 \n\t"            \
    "str    r8, [r0], #4 \n\t"              \
    "umaal  r9, r10, r4, r2 \n\t"           \
    "umaal  r10, r11, r5, r2 \n\t"          \
    "umaal  r11, r12, r6, r2 \n\t"          \
    "umaal  r12, r14, r7, r2 \n\t"          \
                                            \
    "ldr    r2, [r1], #4 \n\t"              \
    "mov    r8, #0 \n\t"                    \
    "umaal  r8, r9, r3, r2 \n\t"            \
    "str    r8, [r0], #4 \n\t"              \
    "umaal  r9, r10, r4, r2 \n\t"           \
    "umaal  r10, r11, r5, r2 \n\t"          \
    "umaal  r11, r12, r6, r2 \n\t"          \
    "umaal  r12, r14, r7, r2 \n\t"          \
                                            \
    "ldr    r2, [r1], #4 \n\t"              \
    "mov    r8, #0 \n\t"                    \
    "umaal  r8, r9, r3, r2 \n\t"            \
    "str    r8, [r0], #4 \n\t"              \
    "umaal  r9, r10, r4, r2 \n\t"           \
    "umaal  r10, r11, r5, r2 \n\t"          \
    "umaal  r11, r12, r6, r2 \n\t"          \
    "umaal  r12, r14, r7, r2 \n\t"          \
                                            \
    "str    r9, [r0], #4 \n\t"              \
    "str    r10, [r0], #4 \n\t"             \
    "str    r11, [r0], #4 \n\t"             \
    "str    r12, [r0], #4 \n\t"             \
    "str    r14, [r0], #4 \n\t"             \
                                            \
    "pop   {r2, r3} \n\t"

#define FAST_MULT_ASM_5_TO_6                 \
    "cmp r3, #5 \n\t"                        \
    "beq 1f \n\t"                            \
                                             \
    /* r4 = left high */                     \
    "ldr r4, [r1] \n\t"                      \
                                             \
    "sub r0, #20 \n\t"                       \
    "sub r1, #20 \n\t"                       \
    "sub r2, #20 \n\t"                       \
                                             \
    /* Do right side */                      \
    "ldr r14, [r2], #4 \n\t"                 \
    "mov r5, #0 \n\t"                        \
    "ldr r6, [r0], #4 \n\t"                  \
    "umaal  r5, r6, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r7, [r0], #4 \n\t"                  \
    "umaal  r6, r7, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r8, [r0], #4 \n\t"                  \
    "umaal  r7, r8, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r9, [r0], #4 \n\t"                  \
    "umaal  r8, r9, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r10, [r0], #4 \n\t"                 \
    "umaal  r9, r10, r4, r14 \n\t"           \
    "sub r0, #20 \n\t"                       \
                                             \
    /* r4 = right high */                    \
    "ldr r4, [r2], #4 \n\t"                  \
                                             \
    /* Do left side */                       \
    "ldr r14, [r1], #4 \n\t"                 \
    "mov r12, #0 \n\t"                       \
    "umaal  r12, r5, r4, r14 \n\t"           \
    "str r12, [r0], #4 \n\t"                 \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r5, r6, r4, r14 \n\t"            \
    "str r5, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r6, r7, r4, r14 \n\t"            \
    "str r6, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r7, r8, r4, r14 \n\t"            \
    "str r7, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"            \
    "str r8, [r0], #4 \n\t"                  \
                                             \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r9, r10, r4, r14 \n\t"           \
    "stmia r0!, {r9, r10} \n\t"

#define FAST_MULT_ASM_6                  \
    "ldmia  r2!, {r4, r5, r6} \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "umull  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "mov    r10, #0 \n\t"                \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "mov    r11, #0 \n\t"                \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "str    r9, [r0], #4 \n\t"           \
    "str    r10, [r0], #4 \n\t"          \
    "str    r11, [r0], #4 \n\t"          \
                                         \
    "sub r0, #24 \n\t"                   \
    "sub r1, #24 \n\t"                   \
    "ldmia  r2!, {r4, r5, r6} \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "mov    r9, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "mov    r10, #0 \n\t"                \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "mov    r11, #0 \n\t"                \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "str    r9, [r0], #4 \n\t"           \
    "str    r10, [r0], #4 \n\t"          \
    "str    r11, [r0], #4 \n\t"

#define FAST_MULT_ASM_6_TO_7                 \
    "cmp r3, #6 \n\t"                        \
    "beq 1f \n\t"                            \
                                             \
    /* r4 = left high */                     \
    "ldr r4, [r1] \n\t"                      \
                                             \
    "sub r0, #24 \n\t"                       \
    "sub r1, #24 \n\t"                       \
    "sub r2, #24 \n\t"                       \
                                             \
    /* Do right side */                      \
    "ldr r14, [r2], #4 \n\t"                 \
    "mov r5, #0 \n\t"                        \
    "ldr r6, [r0], #4 \n\t"                  \
    "umaal  r5, r6, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r7, [r0], #4 \n\t"                  \
    "umaal  r6, r7, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r8, [r0], #4 \n\t"                  \
    "umaal  r7, r8, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r9, [r0], #4 \n\t"                  \
    "umaal  r8, r9, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r10, [r0], #4 \n\t"                 \
    "umaal  r9, r10, r4, r14 \n\t"           \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r11, [r0], #4 \n\t"                 \
    "umaal  r10, r11, r4, r14 \n\t"          \
    "sub r0, #24 \n\t"                       \
                                             \
    /* r4 = right high */                    \
    "ldr r4, [r2], #4 \n\t"                  \
                                             \
    /* Do left side */                       \
    "ldr r14, [r1], #4 \n\t"                 \
    "mov r12, #0 \n\t"                       \
    "umaal  r12, r5, r4, r14 \n\t"           \
    "str r12, [r0], #4 \n\t"                 \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r5, r6, r4, r14 \n\t"            \
    "str r5, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r6, r7, r4, r14 \n\t"            \
    "str r6, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r7, r8, r4, r14 \n\t"            \
    "str r7, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"            \
    "str r8, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r9, r10, r4, r14 \n\t"           \
    "str r9, [r0], #4 \n\t"                  \
                                             \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r10, r11, r4, r14 \n\t"          \
    "stmia r0!, {r10, r11} \n\t"

#define FAST_MULT_ASM_7                  \
    "ldmia  r2!, {r4, r5, r6, r7} \n\t"  \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "umull  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "mov    r10, #0 \n\t"                \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "mov    r11, #0 \n\t"                \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "mov    r12, #0 \n\t"                \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "str    r9, [r0], #4 \n\t"           \
    "str    r10, [r0], #4 \n\t"          \
    "str    r11, [r0], #4 \n\t"          \
    "str    r12, [r0], #4 \n\t"          \
                                         \
    "sub r0, #28 \n\t"                   \
    "sub r1, #28 \n\t"                   \
    "ldmia  r2!, {r4, r5, r6} \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "mov    r9, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "mov    r10, #0 \n\t"                \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "mov    r11, #0 \n\t"                \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
                                         \
    "str    r9, [r0], #4 \n\t"           \
    "str    r10, [r0], #4 \n\t"          \
    "str    r11, [r0], #4 \n\t"

#define FAST_MULT_ASM_7_TO_8                 \
    "cmp r3, #7 \n\t"                        \
    "beq 1f \n\t"                            \
    "push {r3} \n\t"                         \
                                             \
    /* r4 = left high */                     \
    "ldr r4, [r1] \n\t"                      \
                                             \
    "sub r0, #28 \n\t"                       \
    "sub r1, #28 \n\t"                       \
    "sub r2, #28 \n\t"                       \
                                             \
    /* Do right side */                      \
    "ldr r14, [r2], #4 \n\t"                 \
    "mov r5, #0 \n\t"                        \
    "ldr r6, [r0], #4 \n\t"                  \
    "umaal  r5, r6, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r7, [r0], #4 \n\t"                  \
    "umaal  r6, r7, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r8, [r0], #4 \n\t"                  \
    "umaal  r7, r8, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r9, [r0], #4 \n\t"                  \
    "umaal  r8, r9, r4, r14 \n\t"            \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r10, [r0], #4 \n\t"                 \
    "umaal  r9, r10, r4, r14 \n\t"           \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r11, [r0], #4 \n\t"                 \
    "umaal  r10, r11, r4, r14 \n\t"          \
    "ldr r14, [r2], #4 \n\t"                 \
    "ldr r12, [r0], #4 \n\t"                 \
    "umaal  r11, r12, r4, r14 \n\t"          \
    "sub r0, #28 \n\t"                       \
                                             \
    /* r4 = right high */                    \
    "ldr r4, [r2], #4 \n\t"                  \
                                             \
    /* Do left side */                       \
    "ldr r14, [r1], #4 \n\t"                 \
    "mov r3, #0 \n\t"                        \
    "umaal  r3, r5, r4, r14 \n\t"            \
    "str r3, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r5, r6, r4, r14 \n\t"            \
    "str r5, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r6, r7, r4, r14 \n\t"            \
    "str r6, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r7, r8, r4, r14 \n\t"            \
    "str r7, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"            \
    "str r8, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r9, r10, r4, r14 \n\t"           \
    "str r9, [r0], #4 \n\t"                  \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r10, r11, r4, r14 \n\t"          \
    "str r10, [r0], #4 \n\t"                 \
                                             \
    "ldr r14, [r1], #4 \n\t"                 \
    "umaal  r11, r12, r4, r14 \n\t"          \
    "stmia r0!, {r11, r12} \n\t"             \
    "pop {r3} \n\t"

#define FAST_MULT_ASM_8                  \
    "ldmia  r2!, {r4, r5, r6, r7} \n\t"  \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "umull  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "mov    r10, #0 \n\t"                \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "mov    r11, #0 \n\t"                \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "mov    r12, #0 \n\t"                \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "mov    r8, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "str    r9, [r0], #4 \n\t"           \
    "str    r10, [r0], #4 \n\t"          \
    "str    r11, [r0], #4 \n\t"          \
    "str    r12, [r0], #4 \n\t"          \
                                         \
    "sub r0, #32 \n\t"                   \
    "sub r1, #32 \n\t"                   \
    "ldmia  r2!, {r4, r5, r6, r7} \n\t"  \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "mov    r9, #0 \n\t"                 \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "mov    r10, #0 \n\t"                \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "mov    r11, #0 \n\t"                \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "mov    r12, #0 \n\t"                \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "ldr    r14, [r1], #4 \n\t"          \
    "ldr    r8, [r0] \n\t"               \
    "umaal  r8, r9, r4, r14 \n\t"        \
    "str    r8, [r0], #4 \n\t"           \
    "umaal  r9, r10, r5, r14 \n\t"       \
    "umaal  r10, r11, r6, r14 \n\t"      \
    "umaal  r11, r12, r7, r14 \n\t"      \
                                         \
    "str    r9, [r0], #4 \n\t"           \
    "str    r10, [r0], #4 \n\t"          \
    "str    r11, [r0], #4 \n\t"          \
    "str    r12, [r0], #4 \n\t"

#define FAST_SQUARE_ASM_5               \
    "ldmia r1!, {r9,r10,r11,r12,r14} \n\t" \
    "push {r1, r2} \n\t"                \
                                        \
    "umull r1, r2, r10, r9 \n\t"        \
    "mov r3, #0 \n\t"                   \
    "umaal r2, r3, r11, r9 \n\t"        \
    "mov r4, #0 \n\t"                   \
    "umaal r3, r4, r12, r9 \n\t"        \
    "mov r5, #0 \n\t"                   \
    "umaal r4, r5, r14, r9 \n\t"        \
                                        \
    "mov r6, #0 \n\t"                   \
    "umaal r6, r3, r11, r10 \n\t"       \
    "umaal r3, r4, r12, r10 \n\t"       \
    "adds r1, r1, r1 \n\t"              \
    "adcs r2, r2, r2 \n\t"              \
    "adcs r6, r6, r6 \n\t"              \
    "adcs r3, r3, r3 \n\t"              \
                                        \
    "umull r7, r8, r9, r9 \n\t"         \
    /* Store carry in r9 */             \
    "mov r9, #0 \n\t"                   \
    "adc r9, r9, #0 \n\t"               \
    "adds r8, r8, r1 \n\t"              \
    "stmia r0!, {r7,r8} \n\t"           \
                                        \
    "umull r7, r8, r10, r10 \n\t"       \
    "adcs r7, r7, r2 \n\t"              \
    "adcs r8, r8, r6 \n\t"              \
    "stmia r0!, {r7,r8} \n\t"           \
                                        \
    "umaal r4, r5, r14, r10 \n\t"       \
    /* Store carry in r10 */            \
    "mov r10, #0 \n\t"                  \
    "adc r10, r10, #0 \n\t"             \
                                        \
    "mov r1, #0 \n\t"                   \
    "umaal r1, r4, r12, r11 \n\t"       \
    "umaal r4, r5, r14, r11 \n\t"       \
                                        \
    "mov r2, #0 \n\t"                   \
    "umaal r2, r5, r14, r12 \n\t"       \
    /* Load carry from r9 */            \
    "lsrs r9, #1 \n\t"                  \
    "adcs r1, r1, r1 \n\t"              \
    "adcs r4, r4, r4 \n\t"              \
    "adcs r2, r2, r2 \n\t"              \
    "adcs r5, r5, r5 \n\t"              \
    /* r9 is 0 now */                   \
    "adc r9, r9, #0 \n\t"               \
                                        \
    /* Use carry from r10 */            \
    "umaal r3, r10, r11, r11 \n\t"      \
    "adds r10, r10, r1 \n\t"            \
    "stmia r0!, {r3,r10} \n\t"          \
                                        \
    "umull r6, r10, r12, r12 \n\t"      \
    "adcs r6, r6, r4 \n\t"              \
    "adcs r10, r10, r2 \n\t"            \
    "stmia r0!, {r6,r10} \n\t"          \
                                        \
    "umull r6, r10, r14, r14 \n\t"      \
    "adcs r6, r6, r5 \n\t"              \
    "adcs r10, r10, r9 \n\t"            \
    "stmia r0!, {r6,r10} \n\t"          \
    "pop {r1, r2} \n\t"

#define FAST_SQUARE_ASM_5_TO_6           \
    "cmp r2, #5 \n\t"                    \
    "beq 1f \n\t"                        \
                                         \
    "sub r0, #20 \n\t"                   \
    "sub r1, #20 \n\t"                   \
                                         \
    /* Do off-center multiplication */   \
    "ldmia r1!, {r5,r6,r7,r8,r9,r14} \n\t" \
    "umull r3, r4, r5, r14 \n\t"         \
    "mov r5, #0 \n\t"                    \
    "umaal r4, r5, r6, r14 \n\t"         \
    "mov r6, #0 \n\t"                    \
    "umaal r5, r6, r7, r14 \n\t"         \
    "mov r7, #0 \n\t"                    \
    "umaal r6, r7, r8, r14 \n\t"         \
    "mov r8, #0 \n\t"                    \
    "umaal r7, r8, r9, r14 \n\t"         \
                                         \
    /* Multiply by 2 */                  \
    "mov r9, #0 \n\t"                    \
    "adds r3, r3, r3 \n\t"               \
    "adcs r4, r4, r4 \n\t"               \
    "adcs r5, r5, r5 \n\t"               \
    "adcs r6, r6, r6 \n\t"               \
    "adcs r7, r7, r7 \n\t"               \
    "adcs r8, r8, r8 \n\t"               \
    "adcs r9, r9, #0 \n\t"               \
                                         \
    /* Add into previous */              \
    "ldr r12, [r0], #4 \n\t"             \
    "adds r3, r3, r12 \n\t"              \
    "ldr r12, [r0], #4 \n\t"             \
    "adcs r4, r4, r12 \n\t"              \
    "ldr r12, [r0], #4 \n\t"             \
    "adcs r5, r5, r12 \n\t"              \
    "ldr r12, [r0], #4 \n\t"             \
    "adcs r6, r6, r12 \n\t"              \
    "ldr r12, [r0], #4 \n\t"             \
    "adcs r7, r7, r12 \n\t"              \
    "adcs r8, r8, #0 \n\t"               \
    "adcs r9, r9, #0 \n\t"               \
    "sub r0, #20 \n\t"                   \
                                         \
    /* Perform center multiplication */  \
    "umlal r8, r9, r14, r14 \n\t"        \
    "stmia r0!, {r3,r4,r5,r6,r7,r8,r9} \n\t"

#define FAST_SQUARE_ASM_6               \
    "ldmia r1!, {r8,r9,r10,r11,r12,r14} \n\t" \
    "push {r1, r2} \n\t"                \
                                        \
    "umull r1, r2, r9, r8 \n\t"         \
    "mov r3, #0 \n\t"                   \
    "umaal r2, r3, r10, r8 \n\t"        \
    "mov r4, #0 \n\t"                   \
    "umaal r3, r4, r11, r8 \n\t"        \
    "mov r5, #0 \n\t"                   \
    "umaal r4, r5, r12, r8 \n\t"        \
    "mov r6, #0 \n\t"                   \
    "umaal r5, r6, r14, r8 \n\t"        \
                                        \
    "mov r7, #0 \n\t"                   \
    "umaal r7, r3, r10, r9 \n\t"        \
    "umaal r3, r4, r11, r9 \n\t"        \
    "umaal r4, r5, r12, r9 \n\t"        \
    "push {r4, r5} \n\t"                \
    "adds r1, r1, r1 \n\t"              \
    "adcs r2, r2, r2 \n\t"              \
    "adcs r7, r7, r7 \n\t"              \
    "adcs r3, r3, r3 \n\t"              \
                                        \
    "umull r4, r5, r8, r8 \n\t"         \
    /* Store carry in r8 */             \
    "mov r8, #0 \n\t"                   \
    "adc r8, r8, #0 \n\t"               \
    "adds r5, r5, r1 \n\t"              \
    "stmia r0!, {r4,r5} \n\t"           \
                                        \
    "umull r4, r5, r9, r9 \n\t"         \
    "adcs r4, r4, r2 \n\t"              \
    "adcs r5, r5, r7 \n\t"              \
    "stmia r0!, {r4,r5} \n\t"           \
                                        \
    "pop {r4, r5} \n\t"                 \
    "umaal r5, r6, r14, r9 \n\t"        \
    /* Store carry in r9 */             \
    "mov r9, #0 \n\t"                   \
    "adc r9, r9, #0 \n\t"               \
                                        \
    "mov r1, #0 \n\t"                   \
    "umaal r1, r4, r11, r10 \n\t"       \
    "umaal r4, r5, r12, r10 \n\t"       \
    "umaal r5, r6, r14, r10 \n\t"       \
                                        \
    "mov r2, #0 \n\t"                   \
    "umaal r2, r5, r12, r11 \n\t"       \
    "umaal r5, r6, r14, r11 \n\t"       \
                                        \
    "mov r7, #0 \n\t"                   \
    "umaal r7, r6, r14, r12 \n\t"       \
                                        \
    /* Load carry from r8 */            \
    "lsrs r8, #1 \n\t"                  \
    "adcs r1, r1, r1 \n\t"              \
    "adcs r4, r4, r4 \n\t"              \
    "adcs r2, r2, r2 \n\t"              \
    "adcs r5, r5, r5 \n\t"              \
    "adcs r7, r7, r7 \n\t"              \
    "adcs r6, r6, r6 \n\t"              \
    "adc r8, r8, #0 \n\t"               \
                                        \
    /* Use carry from r9 */             \
    "umaal r3, r9, r10, r10 \n\t"       \
    "adds r9, r9, r1 \n\t"              \
    "stmia r0!, {r3,r9} \n\t"           \
                                        \
    "umull r9, r10, r11, r11 \n\t"      \
    "adcs r9, r9, r4 \n\t"              \
    "adcs r10, r10, r2 \n\t"            \
    "stmia r0!, {r9,r10} \n\t"          \
                                        \
    "umull r9, r10, r12, r12 \n\t"      \
    "adcs r9, r9, r5 \n\t"              \
    "adcs r10, r10, r7 \n\t"            \
    "stmia r0!, {r9,r10} \n\t"          \
                                        \
    "umull r9, r10, r14, r14 \n\t"      \
    "adcs r9, r9, r6 \n\t"              \
    "adcs r10, r10, r8 \n\t"            \
    "stmia r0!, {r9,r10} \n\t"          \
    "pop {r1, r2} \n\t"

#define FAST_SQUARE_ASM_6_TO_7               \
    "cmp r2, #6 \n\t"                        \
    "beq 1f \n\t"                            \
                                             \
    "sub r0, #24 \n\t"                       \
    "sub r1, #24 \n\t"                       \
                                             \
    /* Do off-center multiplication */       \
    "ldmia r1!, {r5,r6,r7,r8,r9,r10,r14} \n\t" \
    "umull r3, r4, r5, r14 \n\t"             \
    "mov r5, #0 \n\t"                        \
    "umaal r4, r5, r6, r14 \n\t"             \
    "mov r6, #0 \n\t"                        \
    "umaal r5, r6, r7, r14 \n\t"             \
    "mov r7, #0 \n\t"                        \
    "umaal r6, r7, r8, r14 \n\t"             \
    "mov r8, #0 \n\t"                        \
    "umaal r7, r8, r9, r14 \n\t"             \
    "mov r9, #0 \n\t"                        \
    "umaal r8, r9, r10, r14 \n\t"            \
                                             \
    /* Multiply by 2 */                      \
    "mov r10, #0 \n\t"                       \
    "adds r3, r3, r3 \n\t"                   \
    "adcs r4, r4, r4 \n\t"                   \
    "adcs r5, r5, r5 \n\t"                   \
    "adcs r6, r6, r6 \n\t"                   \
    "adcs r7, r7, r7 \n\t"                   \
    "adcs r8, r8, r8 \n\t"                   \
    "adcs r9, r9, r9 \n\t"                   \
    "adcs r10, r10, #0 \n\t"                 \
                                             \
    /* Add into previous */                  \
    "ldr r12, [r0], #4 \n\t"                 \
    "adds r3, r3, r12 \n\t"                  \
    "ldr r12, [r0], #4 \n\t"                 \
    "adcs r4, r4, r12 \n\t"                  \
    "ldr r12, [r0], #4 \n\t"                 \
    "adcs r5, r5, r12 \n\t"                  \
    "ldr r12, [r0], #4 \n\t"                 \
    "adcs r6, r6, r12 \n\t"                  \
    "ldr r12, [r0], #4 \n\t"                 \
    "adcs r7, r7, r12 \n\t"                  \
    "ldr r12, [r0], #4 \n\t"                 \
    "adcs r8, r8, r12 \n\t"                  \
    "adcs r9, r9, #0 \n\t"                   \
    "adcs r10, r10, #0 \n\t"                 \
    "sub r0, #24 \n\t"                       \
                                             \
    /* Perform center multiplication */      \
    "umlal r9, r10, r14, r14 \n\t"           \
    "stmia r0!, {r3,r4,r5,r6,r7,r8,r9,r10} \n\t"

#define FAST_SQUARE_ASM_7               \
    "ldmia r1!, {r9,r10,r11,r12} \n\t"  \
    "push {r2} \n\t"                    \
                                        \
    "umull r14, r2, r10, r9 \n\t"       \
    "mov r3, #0 \n\t"                   \
    "umaal r2, r3, r11, r9 \n\t"        \
    "mov r4, #0 \n\t"                   \
    "umaal r3, r4, r12, r9 \n\t"        \
                                        \
    "mov r5, #0 \n\t"                   \
    "umaal r5, r3, r11, r10 \n\t"       \
    "adds r14, r14, r14 \n\t"           \
    "adcs r2, r2, r2 \n\t"              \
    "adcs r5, r5, r5 \n\t"              \
    /* Store carry in r7 */             \
    "mov r7, #0 \n\t"                   \
    "adc r7, r7, #0 \n\t"               \
                                        \
    "umull r6, r8, r9, r9 \n\t"         \
    "adds r8, r8, r14 \n\t"             \
    "stmia r0!, {r6,r8} \n\t"           \
                                        \
    "umull r6, r8, r10, r10 \n\t"       \
    "adcs r6, r6, r2 \n\t"              \
    "adcs r8, r8, r5 \n\t"              \
    "stmia r0!, {r6,r8} \n\t"           \
    /* Store carry in r8 */             \
    "mov r8, #0 \n\t"                   \
    "adc r8, r8, #0 \n\t"               \
                                        \
    "ldmia r1!, {r2, r6, r14} \n\t"     \
    "push {r1} \n\t"                    \
    "umaal r3, r4, r2, r9 \n\t"         \
    "mov r5, #0 \n\t"                   \
    "umaal r4, r5, r6, r9 \n\t"         \
    "mov r1, #0 \n\t"                   \
    "umaal r5, r1, r14, r9 \n\t"        \
                                        \
    "mov r9, #0 \n\t"                   \
    "umaal r3, r9, r12, r10 \n\t"       \
    "umaal r9, r4, r2, r10 \n\t"        \
    "umaal r4, r5, r6, r10 \n\t"        \
    "umaal r5, r1, r14, r10 \n\t"       \
                                        \
    "mov r10, #0 \n\t"                  \
    "umaal r10, r9, r12, r11 \n\t"      \
    "umaal r9, r4, r2, r11 \n\t"        \
    "umaal r4, r5, r6, r11 \n\t"        \
    "umaal r5, r1, r14, r11 \n\t"       \
                                        \
    /* Load carry from r7 */            \
    "lsrs r7, #1 \n\t"                  \
    "adcs r3, r3, r3 \n\t"              \
    "adcs r10, r10, r10 \n\t"           \
    "adcs r9, r9, r9 \n\t"              \
    /* Store carry back in r7 */        \
    "adc r7, r7, #0 \n\t"               \
                                        \
    /* Use carry from r8 */             \
    "umaal r3, r8, r11, r11 \n\t"       \
    "adds r8, r8, r10 \n\t"             \
    "stmia r0!, {r3,r8} \n\t"           \
    /* Store carry back in r8 */        \
    "mov r8, #0 \n\t"                   \
    "adc r8, r8, #0 \n\t"               \
                                        \
    "mov r3, #0 \n\t"                   \
    "umaal r3, r4, r2, r12 \n\t"        \
    "umaal r4, r5, r6, r12 \n\t"        \
    "umaal r5, r1, r14, r12 \n\t"       \
                                        \
    "mov r10, #0 \n\t"                  \
    "umaal r10, r5, r6, r2 \n\t"        \
    "umaal r5, r1, r14, r2 \n\t"        \
                                        \
    "mov r11, #0 \n\t"                  \
    "umaal r11, r1, r14, r6 \n\t"       \
                                        \
    /* Load carry from r7 */            \
    "lsrs r7, #1 \n\t"                  \
    "adcs r3, r3, r3 \n\t"              \
    "adcs r4, r4, r4 \n\t"              \
    "adcs r10, r10, r10 \n\t"           \
    "adcs r5, r5, r5 \n\t"              \
    "adcs r11, r11, r11 \n\t"           \
    "adcs r1, r1, r1 \n\t"              \
    "adc r7, r7, #0 \n\t"               \
                                        \
    /* Use carry from r8 */             \
    "umaal r8, r9, r12, r12 \n\t"       \
    "adds r9, r9, r3 \n\t"              \
    "stmia r0!, {r8,r9} \n\t"           \
                                        \
    "umull r8, r9, r2, r2 \n\t"         \
    "adcs r8, r8, r4 \n\t"              \
    "adcs r9, r9, r10 \n\t"             \
    "stmia r0!, {r8,r9} \n\t"           \
                                        \
    "umull r8, r9, r6, r6 \n\t"         \
    "adcs r8, r8, r5 \n\t"              \
    "adcs r9, r9, r11 \n\t"             \
    "stmia r0!, {r8,r9} \n\t"           \
                                        \
    "umull r8, r9, r14, r14 \n\t"       \
    "adcs r8, r8, r1 \n\t"              \
    "adcs r9, r9, r7 \n\t"              \
    "stmia r0!, {r8,r9} \n\t"           \
    "pop {r1, r2} \n\t"

#define FAST_SQUARE_ASM_7_TO_8           \
    "cmp r2, #7 \n\t"                    \
    "beq 1f \n\t"                        \
                                         \
    "sub r0, #28 \n\t"                   \
    "sub r1, #28 \n\t"                   \
                                         \
    /* Do off-center multiplication */   \
    "ldmia r1!, {r5,r6,r7,r8,r9,r10,r11,r14} \n\t" \
    "umull r3, r4, r5, r14 \n\t"         \
    "mov r5, #0 \n\t"                    \
    "umaal r4, r5, r6, r14 \n\t"         \
    "mov r6, #0 \n\t"                    \
    "umaal r5, r6, r7, r14 \n\t"         \
    "mov r7, #0 \n\t"                    \
    "umaal r6, r7, r8, r14 \n\t"         \
    "mov r8, #0 \n\t"                    \
    "umaal r7, r8, r9, r14 \n\t"         \
    "mov r9, #0 \n\t"                    \
    "umaal r8, r9, r10, r14 \n\t"        \
    "mov r10, #0 \n\t"                   \
    "umaal r9, r10, r11, r14 \n\t"       \
                                         \
    /* Multiply by 2 */                  \
    "mov r11, #0 \n\t"                   \
    "adds r3, r3, r3 \n\t"               \
    "adcs r4, r4, r4 \n\t"               \
    "adcs r5, r5, r5 \n\t"               \
    "adcs r6, r6, r6 \n\t"               \
    "adcs r7, r7, r7 \n\t"               \
    "adcs r8, r8, r8 \n\t"               \
    "adcs r9, r9, r9 \n\t"               \
    "adcs r10, r10, r10 \n\t"            \
    "adcs r11, r11, #0 \n\t"             \
                                         \
    /* Add into previous */              \
    "ldr r12, [r0], #4 \n\t"             \
    "adds r3, r3, r12 \n\t"              \
    "ldr r12, [r0], #4 \n\t"             \
    "adcs r4, r4, r12 \n\t"              \
    "ldr r12, [r0], #4 \n\t"             \
    "adcs r5, r5, r12 \n\t"              \
    "ldr r12, [r0], #4 \n\t"             \
    "adcs r6, r6, r12 \n\t"              \
    "ldr r12, [r0], #4 \n\t"             \
    "adcs r7, r7, r12 \n\t"              \
    "ldr r12, [r0], #4 \n\t"             \
    "adcs r8, r8, r12 \n\t"              \
    "ldr r12, [r0], #4 \n\t"             \
    "adcs r9, r9, r12 \n\t"              \
    "adcs r10, r10, #0 \n\t"             \
    "adcs r11, r11, #0 \n\t"             \
    "sub r0, #28 \n\t"                   \
                                         \
    /* Perform center multiplication */  \
    "umlal r10, r11, r14, r14 \n\t"      \
    "stmia r0!, {r3,r4,r5,r6,r7,r8,r9,r10,r11} \n\t"

#define FAST_SQUARE_ASM_8               \
    "ldmia r1!, {r10,r11,r12,r14} \n\t" \
    "push {r2} \n\t"                    \
                                        \
    "umull r2, r3, r11, r10 \n\t"       \
    "mov r4, #0 \n\t"                   \
    "umaal r3, r4, r12, r10 \n\t"       \
    "mov r5, #0 \n\t"                   \
    "umaal r4, r5, r14, r10 \n\t"       \
                                        \
    "mov r6, #0 \n\t"                   \
    "umaal r6, r4, r12, r11 \n\t"       \
    "adds r2, r2, r2 \n\t"              \
    "adcs r3, r3, r3 \n\t"              \
    "adcs r6, r6, r6 \n\t"              \
    /* Store carry in r7 */             \
    "mov r7, #0 \n\t"                   \
    "adc r7, r7, #0 \n\t"               \
                                        \
    "umull r8, r9, r10, r10 \n\t"       \
    "adds r9, r9, r2 \n\t"              \
    "stmia r0!, {r8,r9} \n\t"           \
                                        \
    "umull r8, r9, r11, r11 \n\t"       \
    "adcs r8, r8, r3 \n\t"              \
    "adcs r9, r9, r6 \n\t"              \
    "stmia r0!, {r8,r9} \n\t"           \
    /* Store carry in r8 */             \
    "mov r8, #0 \n\t"                   \
    "adc r8, r8, #0 \n\t"               \
                                        \
    "ldmia r1!, {r2, r3} \n\t"          \
    "push {r1} \n\t"                    \
    "umaal r4, r5, r2, r10 \n\t"        \
    "mov r6, #0 \n\t"                   \
    "umaal r5, r6, r3, r10 \n\t"        \
                                        \
    "mov r9, #0 \n\t"                   \
    "umaal r9, r4, r14, r11 \n\t"       \
    "umaal r4, r5, r2, r11 \n\t"        \
                                        \
    "mov r1, #0 \n\t"                   \
    "umaal r1, r4, r14, r12 \n\t"       \
                                        \
    /* Load carry from r7 */            \
    "lsrs r7, #1 \n\t"                  \
    "adcs r9, r9, r9 \n\t"              \
    "adcs r1, r1, r1 \n\t"              \
    /* Store carry back in r7 */        \
    "adc r7, r7, #0 \n\t"               \
                                        \
    /* Use carry from r8 */             \
    "umaal r8, r9, r12, r12 \n\t"       \
    "adds r9, r9, r1  \n\t"             \
    "stmia r0!, {r8,r9} \n\t"           \
    /* Store carry back in r8 */        \
    "mov r8, #0 \n\t"                   \
    "adc r8, r8, #0 \n\t"               \
                                        \
    "pop {r1} \n\t"                     \
    /* TODO could fix up r1 value on stack here */      \
    /* and leave the value on the stack (rather */      \
    /* than popping) if supporting curves > 256 bits */ \
    "ldr r9, [r1], #4 \n\t"             \
    "ldr r1, [r1] \n\t"                 \
                                        \
    "push {r7} \n\t"                    \
    "umaal r5, r6, r9, r10 \n\t"        \
    "mov r7, #0 \n\t"                   \
    "umaal r6, r7, r1, r10 \n\t"        \
    /* Carry now stored in r10 */       \
    "pop {r10} \n\t"                    \
                                        \
    "umaal r4, r5, r3, r11 \n\t"        \
    "umaal r5, r6, r9, r11 \n\t"        \
    "umaal r6, r7, r1, r11 \n\t"        \
                                        \
    "mov r11, #0 \n\t"                  \
    "umaal r11, r4, r2, r12 \n\t"       \
    "umaal r4, r5, r3, r12 \n\t"        \
    "umaal r5, r6, r9, r12 \n\t"        \
    "umaal r6, r7, r1, r12 \n\t"        \
                                        \
    "mov r12, #0 \n\t"                  \
    "umaal r12, r4, r2, r14 \n\t"       \
    "umaal r4, r5, r3, r14 \n\t"        \
    "umaal r5, r6, r9, r14 \n\t"        \
    "umaal r6, r7, r1, r14 \n\t"        \
                                        \
    /* Load carry from r10 */           \
    "lsrs r10, #1 \n\t"                 \
    "adcs r11, r11, r11 \n\t"           \
    "adcs r12, r12, r12 \n\t"           \
    "adc r10, r10, #0 \n\t"             \
                                        \
    /* Use carry from r8 */             \
    "umaal r8, r11, r14, r14 \n\t"      \
    "adds r11, r11, r12 \n\t"           \
    "stmia r0!, {r8,r11} \n\t"          \
    /* Store carry back in r8 */        \
    "mov r8, #0 \n\t"                   \
    "adc r8, r8, #0 \n\t"               \
                                        \
    "mov r11, #0 \n\t"                  \
    "umaal r11, r5, r3, r2 \n\t"        \
    "umaal r5, r6, r9, r2 \n\t"         \
    "umaal r6, r7, r1, r2 \n\t"         \
                                        \
    "mov r12, #0 \n\t"                  \
    "umaal r12, r6, r9, r3 \n\t"        \
    "umaal r6, r7, r1, r3 \n\t"         \
                                        \
    "mov r14, #0 \n\t"                  \
    "umaal r14, r7, r1, r9 \n\t"        \
                                        \
    /* Load carry from r10 */           \
    "lsrs r10, #1 \n\t"                 \
    "adcs r4, r4, r4 \n\t"              \
    "adcs r11, r11, r11 \n\t"           \
    "adcs r5, r5, r5 \n\t"              \
    "adcs r12, r12, r12 \n\t"           \
    "adcs r6, r6, r6 \n\t"              \
    "adcs r14, r14, r14 \n\t"           \
    "adcs r7, r7, r7 \n\t"              \
    "adc r10, r10, #0 \n\t"             \
                                        \
    /* Use carry from r8 */             \
    "umaal r4, r8, r2, r2 \n\t"         \
    "adds r8, r8, r11 \n\t"             \
    "stmia r0!, {r4,r8} \n\t"           \
                                        \
    "umull r4, r8, r3, r3 \n\t"         \
    "adcs r4, r4, r5 \n\t"              \
    "adcs r8, r8, r12 \n\t"             \
    "stmia r0!, {r4,r8} \n\t"           \
                                        \
    "umull r4, r8, r9, r9 \n\t"         \
    "adcs r4, r4, r6 \n\t"              \
    "adcs r8, r8, r14 \n\t"             \
    "stmia r0!, {r4,r8} \n\t"           \
                                        \
    "umull r4, r8, r1, r1 \n\t"         \
    "adcs r4, r4, r7 \n\t"              \
    "adcs r8, r8, r10 \n\t"             \
    "stmia r0!, {r4,r8} \n\t"           \
    /* TODO pop {r1, r2} if supporting curves > 256 bits */ \
    "pop {r2} \n\t"

#endif /* _UECC_ASM_ARM_MULT_SQUARE_H_ */
