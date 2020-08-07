#include "jtest.h"
#include "filtering_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "filtering_templates.h"
#include "type_abbrev.h"

#define IIR_DEFINE_TEST(suffix, output_type)                                              \
   JTEST_DEFINE_TEST(arm_iir_lattice_##suffix##_test,                                     \
         arm_iir_lattice_##suffix)                                                        \
   {                                                                                      \
      arm_iir_lattice_instance_##suffix iir_inst_fut = { 0 };                             \
      arm_iir_lattice_instance_##suffix iir_inst_ref = { 0 };                             \
                                                                                          \
      TEMPLATE_DO_ARR_DESC(                                                               \
            blocksize_idx, uint32_t, blockSize, filtering_blocksizes                      \
            ,                                                                             \
         TEMPLATE_DO_ARR_DESC(                                                            \
               numstages_idx, uint16_t, numStages, filtering_numstages                    \
               ,                                                                          \
              /* Display test parameter values */                                         \
              JTEST_DUMP_STRF("Block Size: %d\n"                                          \
                              "Number of Stages: %d\n",                                   \
                              (int)blockSize,                                             \
                              (int)numStages);                                            \
                                                                                          \
               /* Initialize the IIR Instances */                                         \
               arm_iir_lattice_init_##suffix(                                             \
                     &iir_inst_fut, numStages, (output_type*)filtering_coeffs_b_##suffix, \
                     (output_type*)filtering_coeffs_c_##suffix,                           \
                     (void *) filtering_pState, blockSize);                               \
                                                                                          \
               JTEST_COUNT_CYCLES(                                                        \
                     arm_iir_lattice_##suffix(                                            \
                           &iir_inst_fut,                                                 \
                           (void *) filtering_##suffix##_inputs,                          \
                           (void *) filtering_output_fut,                                 \
                           blockSize));                                                   \
                                                                                          \
               arm_iir_lattice_init_##suffix(                                             \
                     &iir_inst_ref, numStages, (output_type*)filtering_coeffs_b_##suffix, \
                     (output_type*)filtering_coeffs_c_##suffix,                           \
                     (void *) filtering_pState, blockSize);                               \
                                                                                          \
               ref_iir_lattice_##suffix(                                                  \
                     &iir_inst_ref,                                                       \
                     (void *) filtering_##suffix##_inputs,                                \
                     (void *) filtering_output_ref,                                       \
                     blockSize);                                                          \
                                                                                          \
               FILTERING_SNR_COMPARE_INTERFACE(                                           \
                     blockSize,                                                           \
                     output_type)));                                                      \
                                                                                          \
            return JTEST_TEST_PASSED;                                                     \
   }

IIR_DEFINE_TEST(f32, float32_t);
IIR_DEFINE_TEST(q31, q31_t);
IIR_DEFINE_TEST(q15, q15_t);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(iir_tests)
{
    /*
      To skip a test, comment it out.
    */
   JTEST_TEST_CALL(arm_iir_lattice_f32_test);
   JTEST_TEST_CALL(arm_iir_lattice_q31_test);
   JTEST_TEST_CALL(arm_iir_lattice_q15_test);
}
