#include "jtest.h"
#include "matrix_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "matrix_templates.h"
#include "type_abbrev.h"

/* This is for the two fixed point cases */
#define JTEST_ARM_MAT_SCALE_TEST(suffix,type)                           \
    JTEST_DEFINE_TEST(arm_mat_scale_##suffix##_test, arm_mat_scale_##suffix) \
    {                                                                   \
        uint32_t i,j;                                                   \
                                                                        \
        TEMPLATE_DO_ARR_DESC(                                           \
            mat_idx, arm_matrix_instance_##suffix *,                    \
            mat_ptr, matrix_##suffix##_b_inputs                         \
            ,                                                           \
            MATRIX_TEST_CONFIG_SAMESIZE_OUTPUT(                         \
                arm_matrix_instance_##suffix *, mat_ptr);               \
                                                                        \
            for(i=0;i<MATRIX_MAX_COEFFS_LEN;i++)                        \
            {                                                           \
                for(j=0;j<MATRIX_MAX_SHIFTS_LEN;j++)                    \
                {                                                       \
                    JTEST_DUMP_STRF("Matrix Dimensions: %dx%d\n",       \
                         (int)mat_ptr->numRows,                         \
                         (int)mat_ptr->numCols);                        \
                                                                        \
                    JTEST_COUNT_CYCLES(                                 \
                        arm_mat_scale_##suffix(mat_ptr,                 \
                                               matrix_##suffix##_scale_values[i], \
                                               matrix_shift_values[j],  \
                                               (arm_matrix_instance_##suffix*) &matrix_output_fut)); \
                                                                        \
                    ref_mat_scale_##suffix(mat_ptr,                     \
                                           matrix_##suffix##_scale_values[i], \
                                           matrix_shift_values[j],      \
                                           (arm_matrix_instance_##suffix*) &matrix_output_ref); \
                                                                        \
                        MATRIX_SNR_COMPARE_INTERFACE(arm_matrix_instance_##suffix, \
                                                     type);             \
                }                                                       \
            });                                                         \
                                                                        \
        return JTEST_TEST_PASSED;                                       \
    }

JTEST_DEFINE_TEST(arm_mat_scale_f32_test, arm_mat_scale_f32)
{
    uint32_t i;

    TEMPLATE_DO_ARR_DESC(
        mat_idx, arm_matrix_instance_f32 *, mat_ptr, matrix_f32_b_inputs
        ,
        MATRIX_TEST_CONFIG_SAMESIZE_OUTPUT(arm_matrix_instance_f32 *, mat_ptr);

        for(i=0;i<MATRIX_MAX_COEFFS_LEN;i++)
        {
            JTEST_DUMP_STRF("Matrix Dimensions: %dx%d\n",                
                         (int)mat_ptr->numRows,                         
                         (int)mat_ptr->numCols);                        
            JTEST_COUNT_CYCLES(arm_mat_scale_f32(mat_ptr, matrix_f32_scale_values[i], &matrix_output_fut));

            ref_mat_scale_f32(mat_ptr, matrix_f32_scale_values[i], &matrix_output_ref);

            MATRIX_SNR_COMPARE_INTERFACE(arm_matrix_instance_f32,
                                         float32_t);
        });

    return JTEST_TEST_PASSED;
}

JTEST_ARM_MAT_SCALE_TEST(q31,q31_t);
JTEST_ARM_MAT_SCALE_TEST(q15,q15_t);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(mat_scale_tests)
{
    /*
      To skip a test, comment it out.
    */
    JTEST_TEST_CALL(arm_mat_scale_f32_test);
    JTEST_TEST_CALL(arm_mat_scale_q31_test);
    JTEST_TEST_CALL(arm_mat_scale_q15_test);
}
