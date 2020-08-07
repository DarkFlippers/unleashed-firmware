#include "jtest.h"
#include "matrix_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "matrix_templates.h"
#include "type_abbrev.h"

JTEST_DEFINE_TEST(arm_mat_inverse_f32_test, arm_mat_inverse_f32)
{
    TEMPLATE_DO_ARR_DESC(
        mat_idx, arm_matrix_instance_f32 *, mat_ptr, matrix_f32_invertible_inputs
        ,
        JTEST_DUMP_STRF("Matrix Dimensions: %dx%d\n",                
                 (int)mat_ptr->numRows,                         
                 (int)mat_ptr->numCols); 
      
        if (MATRIX_TEST_VALID_SQUARE_DIMENSIONS(arm_matrix_instance_f32 *, mat_ptr))
        {
            MATRIX_TEST_CONFIG_SAMESIZE_OUTPUT(arm_matrix_instance_f32 *, mat_ptr);

            /* arm_mat_inverse_f32() modifies its source input. Use the scratch
             * buffer to store a copy of the intended input. */
            {
                float32_t * original_pdata_ptr = mat_ptr->pData;

                memcpy(matrix_output_scratch,
                       mat_ptr->pData,
                       mat_ptr->numRows * mat_ptr->numCols * sizeof(float32_t));
                mat_ptr->pData = (void*) &matrix_output_scratch;

                JTEST_COUNT_CYCLES(arm_mat_inverse_f32(mat_ptr, &matrix_output_fut));
                mat_ptr->pData = original_pdata_ptr;
            }

            ref_mat_inverse_f32(mat_ptr, &matrix_output_ref);

            MATRIX_SNR_COMPARE_INTERFACE(arm_matrix_instance_f32,
                                         float32_t);
        });

    return JTEST_TEST_PASSED;
}

JTEST_DEFINE_TEST(arm_mat_inverse_f64_test, arm_mat_inverse_f64)
{
    TEMPLATE_DO_ARR_DESC(
        mat_idx, arm_matrix_instance_f64 *, mat_ptr, matrix_f64_invertible_inputs
        ,
        JTEST_DUMP_STRF("Matrix Dimensions: %dx%d\n",                
                         (int)mat_ptr->numRows,                         
                         (int)mat_ptr->numCols);    
                         
        if (MATRIX_TEST_VALID_SQUARE_DIMENSIONS(arm_matrix_instance_f64 *, mat_ptr))
        {
            MATRIX_TEST_CONFIG_SAMESIZE_OUTPUT(arm_matrix_instance_f64 *, mat_ptr);

            /* arm_mat_inverse_f64() modifies its source input. Use the scratch
             * buffer to store a copy of the intended input. */
            {
                float64_t * original_pdata_ptr = mat_ptr->pData;

                memcpy(matrix_output_scratch,
                       mat_ptr->pData,
                       mat_ptr->numRows * mat_ptr->numCols * sizeof(float64_t));
                mat_ptr->pData = (void*) &matrix_output_scratch;

                JTEST_COUNT_CYCLES(arm_mat_inverse_f64(mat_ptr, &matrix_output_fut64));
                mat_ptr->pData = original_pdata_ptr;
            }

            ref_mat_inverse_f64(mat_ptr, &matrix_output_ref64);

            MATRIX_DBL_SNR_COMPARE_INTERFACE(arm_matrix_instance_f64);
        });

    return JTEST_TEST_PASSED;
}

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(mat_inverse_tests)
{
    /*
      To skip a test, comment it out.
    */
    JTEST_TEST_CALL(arm_mat_inverse_f32_test);
    JTEST_TEST_CALL(arm_mat_inverse_f64_test);
}
