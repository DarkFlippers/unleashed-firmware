#include "jtest.h"
#include "ref.h"
#include "arr_desc.h"
#include "transform_templates.h"
#include "transform_test_data.h"
#include "type_abbrev.h"

/*
FFT fast function test template. Arguments are: function configuration suffix
(q7/q15/q31/f32) and inverse-transform flag
*/
#define RFFT_FAST_DEFINE_TEST(config_suffix, ifft_flag)                 \
    JTEST_DEFINE_TEST(arm_rfft_fast_f32_##config_suffix##_test,         \
                      arm_fft_f32)                                      \
    {                                                                   \
        arm_rfft_fast_instance_f32 rfft_inst_fut = {{0}, 0, 0};         \
        arm_rfft_fast_instance_f32 rfft_inst_ref = {{0}, 0, 0};         \
                                                                        \
        /* Go through all FFT lengths */                                \
        TEMPLATE_DO_ARR_DESC(                                           \
            fftlen_idx, uint16_t, fftlen, transform_rfft_fast_fftlens   \
            ,                                                           \
                                                                        \
            /* Initialize the RFFT and CFFT Instances */                \
            arm_rfft_fast_init_f32(                                     \
                &rfft_inst_fut, fftlen);                                \
                                                                        \
            arm_rfft_fast_init_f32(                                     \
                &rfft_inst_ref, fftlen);                                \
                                                                        \
            TRANSFORM_COPY_INPUTS(                                      \
                transform_fft_f32_inputs,                               \
                fftlen *                                                \
                sizeof(float32_t));                                     \
                                                                        \
            /* Display parameter values */                              \
            JTEST_DUMP_STRF("Block Size: %d\n"                          \
                            "Inverse-transform flag: %d\n",             \
                         (int)fftlen,                                   \
                         (int)ifft_flag);                               \
                                                                        \
            /* Display cycle count and run test */                      \
            JTEST_COUNT_CYCLES(                                         \
                arm_rfft_fast_f32(                                      \
                    &rfft_inst_fut,                                     \
                    (void *) transform_fft_input_fut,                   \
                    (void *) transform_fft_output_fut,                  \
                    ifft_flag));                                        \
                                                                        \
            ref_rfft_fast_f32(                                          \
                &rfft_inst_ref,                                         \
                (void *) transform_fft_input_ref,                       \
                (void *) transform_fft_output_ref,                      \
                ifft_flag);                                             \
                                                                        \
            /* Test correctness */                                      \
            TRANSFORM_SNR_COMPARE_INTERFACE(                            \
                fftlen,                                                 \
                float32_t));                                            \
                                                                        \
        return JTEST_TEST_PASSED;                                       \
    }

RFFT_FAST_DEFINE_TEST(forward, 0U);
RFFT_FAST_DEFINE_TEST(inverse, 1U);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(rfft_fast_tests)
{
    JTEST_TEST_CALL(arm_rfft_fast_f32_forward_test);
    JTEST_TEST_CALL(arm_rfft_fast_f32_inverse_test);
}
