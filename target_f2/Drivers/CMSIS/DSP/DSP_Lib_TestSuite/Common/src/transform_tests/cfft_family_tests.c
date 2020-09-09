#include "jtest.h"
#include "ref.h"
#include "arr_desc.h"
#include "transform_templates.h"
#include "transform_test_data.h"
#include "type_abbrev.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

#define CFFT_FN_NAME(fn_specifier, type_suffix) \
    arm_cfft_##fn_specifier##_##type_suffix     \

#define CFFT_TEST_NAME(fn_specifier, type_suffix, config_suffix)        \
    arm_cfft_##fn_specifier##_##type_suffix##_##config_suffix##_test    \

/*--------------------------------------------------------------------------------*/
/* Function Aliases */
/*--------------------------------------------------------------------------------*/

/* These aliases allow expansions in the CFFT_FAMILY_DEFINE_TEST() template to
   make sense */
#define arm_cfft_mag_init_f32 arm_cfft_radix4_init_f32
#define arm_cfft_mag_init_q31 arm_cfft_radix4_init_q31
#define arm_cfft_mag_init_q15 arm_cfft_radix4_init_q15
#define arm_cfft_mag_instance_f32 arm_cfft_radix4_instance_f32
#define arm_cfft_mag_instance_q31 arm_cfft_radix4_instance_q31
#define arm_cfft_mag_instance_q15 arm_cfft_radix4_instance_q15
#define transform_mag_fftlens transform_radix4_fftlens

/*--------------------------------------------------------------------------------*/
/* Test Definition */
/*--------------------------------------------------------------------------------*/

/**
 *  Defines a test for the family of CFFT transforms.
 *
 *  The family of CFFT transforms includes:
 *
 *  - arm_cfft_radix4_xxx
 *  - arm_cfft_radix2_xxx
 *  - arm_cfft_mag_xxx
 *
 *  Where xxx can be f32, q31, or q15.
 *
 *  @param fn_specifier Allowed values: radix4, radix2, mag.
 *  @param type_suffix  Allowed values: f32, q31, q15.
 *
 *  @param config_suffix Used to differentiate test names based configuration
 *  (in this case whether the ifft_flag is set or not.)

 *  @param comparison_interface Macro name used to compare reference and fut
 *  outputs.
 *
 *  @param output_tpe The type of variable contained in the output
 *  (e.g. float32_t, uint32_t, etc).
 *
 *  @param ifft_flag Determines whether the arm_cfft_instance_xxx is configured
 *  for an inverse FFT.
 */
#define CFFT_FAMILY_DEFINE_TEST(fn_specifier,                               \
                                type_suffix,                                \
                                config_suffix, /* Delineate between test configs*/ \
                                comparison_interface,                       \
                                output_type,                                \
                                ifft_flag)                                  \
    JTEST_DEFINE_TEST(CFFT_TEST_NAME(fn_specifier, type_suffix,             \
                                     config_suffix),                        \
                      CFFT_FN_NAME(fn_specifier, type_suffix))              \
    {                                                                       \
        arm_cfft_##fn_specifier##_instance_##type_suffix cfft_inst_fut;     \
        arm_cfft_##fn_specifier##_instance_##type_suffix cfft_inst_ref;     \
                                                                            \
        TEMPLATE_DO_ARR_DESC(                                               \
            fftlen_idx, uint16_t, fftlen, transform_##fn_specifier##_fftlens \
            ,                                                               \
                                                                            \
            /* Initialize the cfft instance */                              \
            arm_cfft_##fn_specifier##_init_##type_suffix(                   \
                &cfft_inst_fut, fftlen, ifft_flag, (uint8_t)1);             \
            arm_cfft_##fn_specifier##_init_##type_suffix(                   \
                &cfft_inst_ref, fftlen, ifft_flag, (uint8_t)1);             \
                                                                            \
            TRANSFORM_PREPARE_INPLACE_INPUTS(                               \
                transform_fft_##type_suffix##_inputs,                       \
                fftlen *                                                    \
                sizeof(TYPE_FROM_ABBREV(type_suffix)) *                     \
                2 /*complex_inputs*/);                                      \
                                                                            \
            /* Display parameter values */                                  \
            JTEST_DUMP_STRF("Block Size: %d\n"                              \
                            "Inverse-transform flag: %d\n",                 \
                            (int)fftlen,                                    \
                            (int)ifft_flag);                                \
                                                                            \
            /* Display cycle count and run test */                          \
            JTEST_COUNT_CYCLES(                                             \
                arm_cfft_##fn_specifier##_##type_suffix(                    \
                    &cfft_inst_fut,                                         \
                    (void*) transform_fft_inplace_input_fut));              \
                                                                            \
            ref_cfft_##fn_specifier##_##type_suffix(                        \
                &cfft_inst_ref,                                             \
                (void *) transform_fft_inplace_input_ref);                  \
                                                                            \
            /* Test correctness */                                          \
            comparison_interface(                                           \
                fftlen,                                                     \
                output_type));                                              \
                                                                            \
        return JTEST_TEST_PASSED;                                           \
    }

/**
 *  Bulk wrapper for all tests instantiated using #CFFT_FAMILY_DEFINE_TEST().
 *
 *  This macro allows several test definitions to share the same config_suffix
 *  and ifft_flag settings.
 */
#define CFFT_FAMILY_DEFINE_ALL_TESTS(config_suffix, ifft_flag)      \
    /* Radix2 tests*/                                               \
    CFFT_FAMILY_DEFINE_TEST(radix2, q31, config_suffix,             \
                            TRANSFORM_SNR_COMPARE_CMPLX_INTERFACE,  \
                            TYPE_FROM_ABBREV(q31),                  \
                            ifft_flag);                             \
    CFFT_FAMILY_DEFINE_TEST(radix2, q15, config_suffix,             \
                            TRANSFORM_SNR_COMPARE_CMPLX_INTERFACE,  \
                            TYPE_FROM_ABBREV(q15),                  \
                            ifft_flag);                             \
    /* Radix4 tests*/                                               \
    CFFT_FAMILY_DEFINE_TEST(radix4, q31, config_suffix,             \
                            TRANSFORM_SNR_COMPARE_CMPLX_INTERFACE,  \
                            TYPE_FROM_ABBREV(q31),                  \
                            ifft_flag);                             \
    CFFT_FAMILY_DEFINE_TEST(radix4, q15, config_suffix,             \
                            TRANSFORM_SNR_COMPARE_CMPLX_INTERFACE,  \
                            TYPE_FROM_ABBREV(q15),                  \
                            ifft_flag)
    /* /\* Mag tests*\/                                                  \ */
    /* CFFT_FAMILY_DEFINE_TEST(mag, f32, config_suffix,                \ */
    /*                         TRANSFORM_SNR_COMPARE_INTERFACE,        \ */
    /*                         TYPE_FROM_ABBREV(f32),                  \ */
    /*                         ifft_flag);                             \ */
    /* CFFT_FAMILY_DEFINE_TEST(mag, q31, config_suffix,                \ */
    /*                         TRANSFORM_SNR_COMPARE_INTERFACE,        \ */
    /*                         TYPE_FROM_ABBREV(q31),                  \ */
    /*                         ifft_flag);                             \ */
    /* CFFT_FAMILY_DEFINE_TEST(mag, q15, config_suffix,                \ */
    /*                         TRANSFORM_SNR_COMPARE_INTERFACE,        \ */
    /*                         TYPE_FROM_ABBREV(q15),                  \ */
    /*                         ifft_flag) */

CFFT_FAMILY_DEFINE_ALL_TESTS(forward, 0U);
CFFT_FAMILY_DEFINE_ALL_TESTS(inverse, 1U);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(cfft_family_tests)
{
    /* Forward FFT tests */
    JTEST_TEST_CALL(arm_cfft_radix2_q31_forward_test);
    JTEST_TEST_CALL(arm_cfft_radix2_q15_forward_test);
    JTEST_TEST_CALL(arm_cfft_radix4_q31_forward_test);
    JTEST_TEST_CALL(arm_cfft_radix4_q15_forward_test);

    /* Inverse FFT Tests */
    JTEST_TEST_CALL(arm_cfft_radix2_q31_inverse_test);
    JTEST_TEST_CALL(arm_cfft_radix2_q15_inverse_test);
    JTEST_TEST_CALL(arm_cfft_radix4_q31_inverse_test);
    JTEST_TEST_CALL(arm_cfft_radix4_q15_inverse_test);

    /* Magnitude tests removed from the DSP Library. Keeping them here in case
       minds are changed. */
    /* JTEST_TEST_CALL(arm_cfft_mag_f32_forward_test); */
    /* JTEST_TEST_CALL(arm_cfft_mag_q31_forward_test); */
    /* JTEST_TEST_CALL(arm_cfft_mag_q15_forward_test); */
    /* JTEST_TEST_CALL(arm_cfft_mag_f32_inverse_test); */
    /* JTEST_TEST_CALL(arm_cfft_mag_q31_inverse_test); */
    /* JTEST_TEST_CALL(arm_cfft_mag_q15_inverse_test); */
}
