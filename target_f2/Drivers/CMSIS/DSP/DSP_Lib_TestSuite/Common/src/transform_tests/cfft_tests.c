#include "jtest.h"
#include "ref.h"
#include "arr_desc.h"
#include "transform_templates.h"
#include "transform_test_data.h"

#define CFFT_SNR_THRESHOLD 120

/*
  CFFT function test template. Arguments are: inverse-transform flag, function
  suffix (q7/q15/q31/f32) and the output type (q7_t, q15_t, q31_t, float32_t)
*/
#define CFFT_TEST_BODY(ifft_flag, suffix, output_type)                                  \
    do                                                                                  \
    {                                                                                   \
        /* Go through all arm_cfft_instances */                                         \
        TEMPLATE_DO_ARR_DESC(                                                           \
            cfft_inst_idx, const arm_cfft_instance_##suffix *, cfft_inst_ptr,           \
            transform_cfft_##suffix##_structs                                           \
            ,                                                                           \
                                                                                        \
            TRANSFORM_PREPARE_INPLACE_INPUTS(                                           \
                transform_fft_##suffix##_inputs,                                        \
                cfft_inst_ptr->fftLen *                                                 \
                sizeof(output_type) *                                                   \
                2 /*complex_inputs*/);                                                  \
                                                                                        \
                /* Display parameter values */                                          \
                JTEST_DUMP_STRF("Block Size: %d\n"                                      \
                                "Inverse-transform flag: %d\n",                         \
                                (int)cfft_inst_ptr->fftLen,                             \
                                (int)ifft_flag);                                        \
                                                                                        \
            /* Display cycle count and run test */                                      \
            JTEST_COUNT_CYCLES(                                                         \
                arm_cfft_##suffix(cfft_inst_ptr,                                        \
                             (void *) transform_fft_inplace_input_fut,                  \
                             ifft_flag,              /* IFFT Flag */                    \
                             1));            /* Bitreverse flag */                      \
            ref_cfft_##suffix(cfft_inst_ptr,                                            \
                         (void *) transform_fft_inplace_input_ref,                      \
                         ifft_flag,         /* IFFT Flag */                             \
                         1);        /* Bitreverse flag */                               \
                                                                                        \
            /* Test correctness */                                                      \
            TRANSFORM_SNR_COMPARE_CMPLX_INTERFACE(                                      \
                cfft_inst_ptr->fftLen,                                                  \
                output_type));                                                          \
                                                                                        \
        return JTEST_TEST_PASSED;                                                       \
    } while (0)


/*
  CFFT function with downshift test template. Arguments are: inverse-transform flag,
  function suffix (q7/q15/q31/f32) and the output type (q7_t, q15_t, q31_t, float32_t)
*/
#define CFFT_DOWNSHIFT_INPUT_TEST_BODY(ifft_flag, suffix, output_type)                  \
    do                                                                                  \
    {                                                                                   \
        /* Go through all arm_cfft_instances */                                         \
        TEMPLATE_DO_ARR_DESC(                                                           \
            cfft_inst_idx, const arm_cfft_instance_##suffix *, cfft_inst_ptr,           \
            transform_cfft_##suffix##_structs                                           \
            ,                                                                           \
                                                                                        \
            TRANSFORM_PREPARE_INPLACE_INPUTS_DOWNSHIFT(                                 \
                transform_fft_##suffix##_inputs,                                        \
                cfft_inst_ptr->fftLen *                                                 \
                sizeof(output_type) *                                                   \
                2 /*complex_inputs*/, output_type);                                     \
                                                                                        \
            /* Display parameter values */                                              \
            JTEST_DUMP_STRF("Block Size: %d\n"                                          \
                            "Inverse-transform flag: %d\n",                             \
                            (int)cfft_inst_ptr->fftLen,                                 \
                            (int)ifft_flag);                                            \
                                                                                        \
            /* Display cycle count and run test */                                      \
            JTEST_COUNT_CYCLES(                                                         \
                arm_cfft_##suffix(cfft_inst_ptr,                                        \
                             (void *) transform_fft_inplace_input_fut,                  \
                             ifft_flag,              /* IFFT Flag */                    \
                             1));            /* Bitreverse flag */                      \
            ref_cfft_##suffix(cfft_inst_ptr,                                            \
                         (void *) transform_fft_inplace_input_ref,                      \
                         ifft_flag,         /* IFFT Flag */                             \
                         1);        /* Bitreverse flag */                               \
                                                                                        \
            /* Test correctness */                                                      \
            TRANSFORM_SNR_COMPARE_CMPLX_INTERFACE(                                      \
                cfft_inst_ptr->fftLen,                                                  \
                output_type));                                                          \
                                                                                        \
        return JTEST_TEST_PASSED;                                                       \
    } while (0)


/* Test declarations */
JTEST_DEFINE_TEST(cfft_f32_test, cfft_f32)
{
    CFFT_TEST_BODY((uint8_t) 0, f32, float32_t);
}

JTEST_DEFINE_TEST(cfft_f32_ifft_test, cfft_f32)
{
    CFFT_TEST_BODY((uint8_t) 1, f32, float32_t);
}

JTEST_DEFINE_TEST(cfft_q31_test, cfft_q31)
{
    CFFT_TEST_BODY((uint8_t) 0, q31, q31_t);
}

JTEST_DEFINE_TEST(cfft_q31_ifft_test, cfft_q31)
{
    CFFT_TEST_BODY((uint8_t) 1, q31, q31_t);
}

JTEST_DEFINE_TEST(cfft_q15_test, cfft_q15)
{
    CFFT_TEST_BODY((uint8_t) 0, q15, q15_t);
}

JTEST_DEFINE_TEST(cfft_q15_ifft_test, cfft_q15)
{
    CFFT_TEST_BODY((uint8_t) 1, q15, q15_t);
}

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(cfft_tests)
{
    JTEST_TEST_CALL(cfft_f32_test);
    JTEST_TEST_CALL(cfft_f32_ifft_test);

    JTEST_TEST_CALL(cfft_q31_test);
    JTEST_TEST_CALL(cfft_q31_ifft_test);

    JTEST_TEST_CALL(cfft_q15_test);
    JTEST_TEST_CALL(cfft_q15_ifft_test);
}
