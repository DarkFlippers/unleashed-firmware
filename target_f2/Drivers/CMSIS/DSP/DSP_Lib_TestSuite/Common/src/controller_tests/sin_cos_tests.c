#include "jtest.h"
#include "arr_desc.h"
#include "arm_math.h"
#include "ref.h"
#include "type_abbrev.h"
#include "test_templates.h"

/*--------------------------------------------------------------------------------*/
/* Input Data */
/*--------------------------------------------------------------------------------*/

ARR_DESC_DEFINE(float32_t,
                arm_sin_cos_degrees_f32,
                9,
                CURLY(
                    0,
                    17,
                    45,
                    90,
                    180,
                    360,
                    362,
                    -73,
                    -191.111
                      ));

/* The Q31 version of the function maps numbers in the range [-1, 0.9999999]
 * to degrees in the range [-180, 179]*/
ARR_DESC_DEFINE(q31_t,
                arm_sin_cos_degrees_q31,
                6,
                CURLY(
                    0,
                    0x80000000, /* -1 */
                    0x7fffffff, /* 0.99999 */
                    /* Randoms */
                    0xf7badafa,
                    0x285954a1,
                    0xb9d09511
                      ));

/*--------------------------------------------------------------------------------*/
/* Output Variables */
/*--------------------------------------------------------------------------------*/
float32_t sin_val_fut = 0;
float32_t cos_val_fut = 0;
float32_t sin_val_ref = 0;
float32_t cos_val_ref = 0;

/*--------------------------------------------------------------------------------*/
/* Test Definitions */
/*--------------------------------------------------------------------------------*/

#define MAX_DELTA_f32 50.0e-8f
#define ABS(x) ((x) > 0 ? (x) : -(x))

/*
  Function to test correctness of sin_cos output by comparing it with reference library
*/
#define COMPARISON_INTERFACE(type, threshold)                           \
    if ( (ABS((type) sin_val_ref - (type) sin_val_fut) >                 \
         (type) threshold ) ||                                          \
        (ABS((type) cos_val_ref - (type) cos_val_fut) >                 \
         (type) threshold))                                             \
    {                                                                   \
        JTEST_DUMP_STRF("Error: %f %f\n",                               \
                        ABS((type) sin_val_ref - (type) sin_val_fut),   \
                        ABS((type) cos_val_ref - (type) cos_val_fut));  \
        return JTEST_TEST_FAILED;                                       \
    }

/*
  Sine and cosine test function for float32_t input
*/
JTEST_DEFINE_TEST(arm_sin_cos_f32_test, arm_sin_cos_f32)
{
    /* Test function for all input degree values */
    TEMPLATE_DO_ARR_DESC(
        degree_idx, TYPE_FROM_ABBREV(f32),
        degree, arm_sin_cos_degrees_f32
        ,
        /* Display cycle count and run test */
        JTEST_COUNT_CYCLES(
            arm_sin_cos_f32(
                degree,
                (TYPE_FROM_ABBREV(f32) *) &sin_val_fut,
                (TYPE_FROM_ABBREV(f32) *) &cos_val_fut)
        );
        ref_sin_cos_f32(
            degree,
            (TYPE_FROM_ABBREV(f32) *) &sin_val_ref,
            (TYPE_FROM_ABBREV(f32) *) &cos_val_ref);

        /* Test correctness */
        COMPARISON_INTERFACE(
            TYPE_FROM_ABBREV(f32),
            MAX_DELTA_f32));

    return JTEST_TEST_PASSED;
}


/*
  Sine and cosine test function for q31_t input
*/
JTEST_DEFINE_TEST(arm_sin_cos_q31_test,
                  arm_sin_cos_q31)
{
    /* Test function for all input degree values */
    TEMPLATE_DO_ARR_DESC(
        degree_idx, TYPE_FROM_ABBREV(q31),
        degree, arm_sin_cos_degrees_q31
        ,
        /* Display cycle count and run test */
        JTEST_COUNT_CYCLES(
            arm_sin_cos_q31(
                degree,
                (TYPE_FROM_ABBREV(q31) *) &sin_val_fut,
                (TYPE_FROM_ABBREV(q31) *) &cos_val_fut)
        );
        ref_sin_cos_q31(
            degree,
            (TYPE_FROM_ABBREV(q31) *) &sin_val_ref,
            (TYPE_FROM_ABBREV(q31) *) &cos_val_ref);

        /* Convert q31 numbers to float for comparison purposes. */
        ref_q31_t_to_float((TYPE_FROM_ABBREV(q31) *) &sin_val_fut, &sin_val_fut, 1);
        ref_q31_t_to_float((TYPE_FROM_ABBREV(q31) *) &cos_val_fut, &cos_val_fut, 1);
        ref_q31_t_to_float((TYPE_FROM_ABBREV(q31) *) &sin_val_ref, &sin_val_ref, 1);
        ref_q31_t_to_float((TYPE_FROM_ABBREV(q31) *) &cos_val_ref, &cos_val_ref, 1);

        /* Test correctness */
        COMPARISON_INTERFACE(
            TYPE_FROM_ABBREV(f32),
            MAX_DELTA_f32));

    return JTEST_TEST_PASSED;
}

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(sin_cos_tests)
{
    /*
      To skip a test, comment it out.
    */
    JTEST_TEST_CALL(arm_sin_cos_f32_test);
    JTEST_TEST_CALL(arm_sin_cos_q31_test);
}
