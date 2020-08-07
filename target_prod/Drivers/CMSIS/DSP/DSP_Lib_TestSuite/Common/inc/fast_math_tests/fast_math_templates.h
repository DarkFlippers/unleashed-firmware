#ifndef _FAST_MATH_TEMPLATES_H_
#define _FAST_MATH_TEMPLATES_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "test_templates.h"
#include <string.h>             /* memcpy() */

/*--------------------------------------------------------------------------------*/
/* Group Specific Templates */
/*--------------------------------------------------------------------------------*/

/**
 * Comparison SNR thresholds for the data types used in transform_tests.
 */
#define FAST_MATH_SNR_THRESHOLD_float32_t 95
#define FAST_MATH_SNR_THRESHOLD_q31_t     95
#define FAST_MATH_SNR_THRESHOLD_q15_t     45

/**
 *  Compare the outputs from the function under test and the reference
 *  function using SNR.
 */
#define FAST_MATH_SNR_COMPARE_INTERFACE(block_size,     \
                                        output_type)    \
    do                                                  \
    {                                                   \
        TEST_CONVERT_AND_ASSERT_SNR(                    \
            fast_math_output_f32_ref,                   \
            (output_type *) fast_math_output_ref,       \
            fast_math_output_f32_fut,                   \
            (output_type *) fast_math_output_fut,       \
            block_size,                                 \
            output_type,                                \
            FAST_MATH_SNR_THRESHOLD_##output_type       \
            );                                          \
    } while (0)


/*--------------------------------------------------------------------------------*/
/* TEST Templates */
/*--------------------------------------------------------------------------------*/

#define SQRT_TEST_TEMPLATE_ELT1(suffix)                             \
                                                                    \
    JTEST_DEFINE_TEST(arm_sqrt_##suffix##_test, arm_sqrt_##suffix)  \
    {                                                               \
        uint32_t i;                                                 \
                                                                    \
        JTEST_COUNT_CYCLES(                                         \
            for(i=0;i<FAST_MATH_MAX_LEN;i++)                        \
            {                                                       \
                arm_sqrt_##suffix(                                  \
                    (suffix##_t)fast_math_##suffix##_inputs[i]      \
                    ,(suffix##_t*)fast_math_output_fut + i);        \
            });                                                     \
                                                                    \
        for(i=0;i<FAST_MATH_MAX_LEN;i++)                            \
        {                                                           \
            ref_sqrt_##suffix(                                      \
                (suffix##_t)fast_math_##suffix##_inputs[i]          \
                ,(suffix##_t*)fast_math_output_ref + i);            \
        }                                                           \
                                                                    \
        FAST_MATH_SNR_COMPARE_INTERFACE(                            \
            FAST_MATH_MAX_LEN,                                      \
            suffix##_t);                                            \
                                                                    \
        return JTEST_TEST_PASSED;                                   \
    }


#define SIN_COS_TEST_TEMPLATE_ELT1(suffix, type, func)                  \
                                                                        \
        JTEST_DEFINE_TEST(arm_##func##_##suffix##_test, arm_##func##_##suffix) \
        {                                                               \
            uint32_t i;                                                 \
                                                                        \
            JTEST_COUNT_CYCLES(                                         \
                for(i=0;i<FAST_MATH_MAX_LEN;i++)                        \
                {                                                       \
                    *((type*)fast_math_output_fut + i) = arm_##func##_##suffix( \
                        fast_math_##suffix##_inputs[i]);                \
                });                                                     \
                                                                        \
            JTEST_COUNT_CYCLES(                                         \
                for(i=0;i<FAST_MATH_MAX_LEN;i++)                        \
                {                                                       \
                    *((type*)fast_math_output_ref + i) = ref_##func##_##suffix( \
                        fast_math_##suffix##_inputs[i]);                \
                });                                                     \
                                                                        \
            FAST_MATH_SNR_COMPARE_INTERFACE(                            \
                FAST_MATH_MAX_LEN,                                      \
                type);                                                  \
                                                                        \
            return JTEST_TEST_PASSED;                                   \
        }

#endif /* _FAST_MATH_TEMPLATES_H_ */
