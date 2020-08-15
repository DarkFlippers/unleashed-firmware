#include "jtest.h"
#include "filtering_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "filtering_templates.h"
#include "type_abbrev.h"

static const float32_t mu_f32 = 0.00854f;//1.0f;
static const float32_t mu2_f32 = 1.0f;
static const q31_t mu_q31 = 0x7fffffff;
static const q15_t mu_q15 = 0x7fff;

#define LMS_DEFINE_TEST(suffix, config_suffix, output_type, mu)                        \
   JTEST_DEFINE_TEST(arm_lms##config_suffix##_##suffix##_test,                         \
         arm_lms##config_suffix##_##suffix)                                            \
   {                                                                                   \
      arm_lms##config_suffix##_instance_##suffix lms_inst_fut = { 0 };                 \
      arm_lms##config_suffix##_instance_##suffix lms_inst_ref = { 0 };                 \
      arm_fir_instance_##suffix fir_inst = { 0 };                                      \
      uint32_t i;                                                                      \
                                                                                       \
      TEMPLATE_DO_ARR_DESC(                                                            \
            blocksize_idx, uint32_t, blockSize, lms_blocksizes                         \
            ,                                                                          \
         TEMPLATE_DO_ARR_DESC(                                                         \
               numtaps_idx, uint16_t, numTaps, filtering_numtaps                       \
               ,                                                                       \
               /* Initialize the FIR Instances */                                      \
               arm_fir_init_##suffix(                                                  \
                     &fir_inst, numTaps,                                               \
                     (output_type*)filtering_coeffs_##suffix,                          \
                     (void *) filtering_pState, blockSize);                            \
                                                                                       \
               ref_fir_##suffix(                                                       \
                     &fir_inst,                                                        \
                     (void *) filtering_##suffix##_inputs,                             \
                     (void *) filtering_input_lms,                                     \
                     blockSize);                                                       \
                                                                                       \
               for(i=0;i<numTaps;i++)                                                  \
               {                                                                       \
                  *((output_type*)filtering_coeffs_lms + i) = (output_type)0;          \
               }                                                                       \
                                                                                       \
               for(i=0;i<blockSize;i++)                                                \
               {                                                                       \
                  /* scaled down so that lms will converge           */                \
                  /* scaled down by almost the max of the abs(input) */                \
                  *((output_type*)filtering_input_lms + i) =                           \
                        *((output_type*)filtering_input_lms + i) / 200.0f;             \
                                                                                       \
                  *((output_type*)filtering_output_f32_fut + i) =                      \
                        *((output_type*)filtering_##suffix##_inputs + i) / 200.0f;     \
               }                                                                       \
                                                                                       \
               /* Display test parameter values */                                     \
               JTEST_DUMP_STRF("Block Size: %d\n"                                      \
                               "Number of Taps: %d\n",                                 \
                               (int)blockSize,                                         \
                               (int)numTaps);                                          \
                                                                                       \
               /* Initialize the LMS Instances */                                      \
               arm_lms##config_suffix##_init_##suffix(                                 \
                     &lms_inst_fut, numTaps,                                           \
                     (output_type*)filtering_coeffs_lms,                               \
                     (void *) filtering_pState, mu, blockSize);                        \
                                                                                       \
               JTEST_COUNT_CYCLES(                                                     \
                     arm_lms##config_suffix##_##suffix(                                \
                           &lms_inst_fut,                                              \
                           (void *) filtering_output_f32_fut,                          \
                           (void *) filtering_input_lms,                               \
                           (void *) filtering_output_fut,                              \
                           (void *) ((output_type*)filtering_output_fut+blockSize),    \
                           blockSize));                                                \
                                                                                       \
               for(i=0;i<numTaps;i++)                                                  \
               {                                                                       \
                  *((output_type*)filtering_coeffs_lms + i) = (output_type)0;          \
               }                                                                       \
                                                                                       \
               arm_lms##config_suffix##_init_##suffix(                                 \
                     &lms_inst_ref, numTaps,                                           \
                     (output_type*)filtering_coeffs_lms,                               \
                     (void *) filtering_pState, mu, blockSize);                        \
                                                                                       \
               ref_lms##config_suffix##_##suffix(                                      \
                     &lms_inst_ref,                                                    \
                     (void *) filtering_output_f32_fut,                                \
                     (void *) filtering_input_lms,                                     \
                     (void *) filtering_output_ref,                                    \
                     (void *) ((output_type*)filtering_output_fut+blockSize),          \
                     blockSize);                                                       \
                                                                                       \
               FILTERING_SNR_COMPARE_INTERFACE(                                        \
                     blockSize,                                                        \
                     output_type)));                                                   \
                                                                                       \
            return JTEST_TEST_PASSED;                                                  \
   }

#define LMS_WITH_POSTSHIFT_DEFINE_TEST(suffix, config_suffix, output_type)             \
   JTEST_DEFINE_TEST(arm_lms##config_suffix##_##suffix##_test,                         \
         arm_lms##config_suffix##_##suffix)                                            \
   {                                                                                   \
      arm_lms##config_suffix##_instance_##suffix lms_inst_fut = { 0 };                 \
      arm_lms##config_suffix##_instance_##suffix lms_inst_ref = { 0 };                 \
      arm_fir_instance_##suffix fir_inst = { 0 };                                      \
      uint32_t i;                                                                      \
                                                                                       \
      TEMPLATE_DO_ARR_DESC(                                                            \
            blocksize_idx, uint32_t, blockSize, lms_blocksizes                         \
            ,                                                                          \
         TEMPLATE_DO_ARR_DESC(                                                         \
               numtaps_idx, uint16_t, numTaps, filtering_numtaps                       \
               ,                                                                       \
               TEMPLATE_DO_ARR_DESC(                                                   \
                     postshifts_idx, uint8_t, postShift, filtering_postshifts          \
                     ,                                                                 \
                  /* Initialize the FIR Instances */                                   \
                  arm_fir_init_##suffix(                                               \
                        &fir_inst, numTaps,                                            \
                        (output_type*)filtering_coeffs_##suffix,                       \
                        (void *) filtering_pState, blockSize);                         \
                                                                                       \
                  ref_fir_##suffix(                                                    \
                        &fir_inst,                                                     \
                        (void *) filtering_##suffix##_inputs,                          \
                        (void *) filtering_input_lms,                                  \
                        blockSize);                                                    \
                                                                                       \
                  for(i=0;i<numTaps;i++)                                               \
                  {                                                                    \
                     *((output_type*)filtering_coeffs_lms + i) = (output_type)0;       \
                  }                                                                    \
                                                                                       \
                  for(i=0;i<blockSize;i++)                                             \
                  {                                                                    \
                     /* scaled down so that lms will converge */                       \
                     /* scaled down by log2(numTaps) bits     */                       \
                     *((output_type*)filtering_output_f32_fut + i) =                   \
                           *((output_type*)filtering_##suffix##_inputs + i) >> 6;      \
                  }                                                                    \
                                                                                       \
                  /* Display test parameter values */                                  \
                  JTEST_DUMP_STRF("Block Size: %d\n"                                   \
                                  "Number of Taps: %d\n"                               \
                                  "Post Shift: %d\n",                                  \
                                  (int)blockSize,                                      \
                                  (int)numTaps,                                        \
                                  (int)postShift);                                     \
                                                                                       \
                  /* Initialize the LMS Instances */                                   \
                  arm_lms##config_suffix##_init_##suffix(                              \
                        &lms_inst_fut, numTaps,                                        \
                        (output_type*)filtering_coeffs_lms,                            \
                        (void *) filtering_pState, mu_##suffix, blockSize, postShift); \
                                                                                       \
                  JTEST_COUNT_CYCLES(                                                  \
                        arm_lms##config_suffix##_##suffix(                             \
                              &lms_inst_fut,                                           \
                              (void *) filtering_output_f32_fut,                       \
                              (void *) filtering_input_lms,                            \
                              (void *) filtering_output_fut,                           \
                              (void *) ((output_type*)filtering_output_fut+blockSize), \
                              blockSize));                                             \
                                                                                       \
                  for(i=0;i<numTaps;i++)                                               \
                  {                                                                    \
                     *((output_type*)filtering_coeffs_lms + i) = (output_type)0;       \
                  }                                                                    \
                                                                                       \
                  arm_lms##config_suffix##_init_##suffix(                              \
                        &lms_inst_ref, numTaps,                                        \
                        (output_type*)filtering_coeffs_lms,                            \
                        (void *) filtering_pState, mu_##suffix, blockSize, postShift); \
                                                                                       \
                  ref_lms##config_suffix##_##suffix(                                   \
                        &lms_inst_ref,                                                 \
                        (void *) filtering_output_f32_fut,                             \
                        (void *) filtering_input_lms,                                  \
                        (void *) filtering_output_ref,                                 \
                        (void *) ((output_type*)filtering_output_ref+blockSize),       \
                        blockSize);                                                    \
                                                                                       \
                  FILTERING_SNR_COMPARE_INTERFACE(                                     \
                        blockSize,                                                     \
                        output_type))));                                               \
                                                                                       \
            return JTEST_TEST_PASSED;                                                  \
   }

LMS_DEFINE_TEST(f32,,float32_t, mu_f32);
LMS_WITH_POSTSHIFT_DEFINE_TEST(q31,,q31_t);
LMS_WITH_POSTSHIFT_DEFINE_TEST(q15,,q15_t);

LMS_DEFINE_TEST(f32,_norm,float32_t, mu2_f32);
LMS_WITH_POSTSHIFT_DEFINE_TEST(q31,_norm,q31_t);
LMS_WITH_POSTSHIFT_DEFINE_TEST(q15,_norm,q15_t);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(lms_tests)
{
    /*
      To skip a test, comment it out.
    */
   JTEST_TEST_CALL(arm_lms_f32_test);
   JTEST_TEST_CALL(arm_lms_q31_test);
   JTEST_TEST_CALL(arm_lms_q15_test);

   JTEST_TEST_CALL(arm_lms_norm_f32_test);
   JTEST_TEST_CALL(arm_lms_norm_q31_test);
   JTEST_TEST_CALL(arm_lms_norm_q15_test);
}
