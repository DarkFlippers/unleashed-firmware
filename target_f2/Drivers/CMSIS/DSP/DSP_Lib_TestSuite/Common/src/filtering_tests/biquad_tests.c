#include "jtest.h"
#include "filtering_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "filtering_templates.h"
#include "type_abbrev.h"

#define BIQUAD_DEFINE_TEST(suffix, instance_name, config_suffix, output_type)    \
   JTEST_DEFINE_TEST(arm_biquad_cascade_##config_suffix##_##suffix##_test,       \
         arm_biquad_cascade_##config_suffix##_##suffix)                          \
   {                                                                             \
      instance_name biquad_inst_fut = { 0 };                                     \
      instance_name biquad_inst_ref = { 0 };                                     \
                                                                                 \
      TEMPLATE_DO_ARR_DESC(                                                      \
            blocksize_idx, uint32_t, blockSize, filtering_blocksizes             \
            ,                                                                    \
         TEMPLATE_DO_ARR_DESC(                                                   \
               numstages_idx, uint16_t, numStages, filtering_numstages           \
               ,                                                                 \
               /* Initialize the BIQUAD Instances */                             \
               arm_biquad_cascade_##config_suffix##_init_##suffix(               \
                     &biquad_inst_fut, numStages,                                \
                     (output_type*)filtering_coeffs_b_##suffix,                  \
                     (void *) filtering_pState);                                 \
                                                                                 \
               /* Display test parameter values */                               \
               JTEST_DUMP_STRF("Block Size: %d\n"                                \
                               "Number of Stages: %d\n",                         \
                               (int)blockSize,                                   \
                               (int)numStages);                                  \
                                                                                 \
               JTEST_COUNT_CYCLES(                                               \
                     arm_biquad_cascade_##config_suffix##_##suffix(              \
                           &biquad_inst_fut,                                     \
                           (void *) filtering_##suffix##_inputs,                 \
                           (void *) filtering_output_fut,                        \
                           blockSize));                                          \
                                                                                 \
               arm_biquad_cascade_##config_suffix##_init_##suffix(               \
                     &biquad_inst_ref, numStages,                                \
                     (output_type*)filtering_coeffs_b_##suffix,                  \
                     (void *) filtering_pState);                                 \
                                                                                 \
               ref_biquad_cascade_##config_suffix##_##suffix(                    \
                     &biquad_inst_ref,                                           \
                     (void *) filtering_##suffix##_inputs,                       \
                     (void *) filtering_output_ref,                              \
                     blockSize);                                                 \
                                                                                 \
               FILTERING_SNR_COMPARE_INTERFACE(                                  \
                     blockSize,                                                  \
                     output_type)));                                             \
                                                                                 \
            return JTEST_TEST_PASSED;                                            \
   }

#define BIQUAD_WITH_POSTSHIFT_DEFINE_TEST(suffix, config_suffix, speed, output_type)   \
   JTEST_DEFINE_TEST(arm_biquad_cascade_##config_suffix##speed##_##suffix##_test,      \
         arm_biquad_cascade_##config_suffix##speed##_##suffix)                         \
   {                                                                                   \
      arm_biquad_casd_##config_suffix##_inst_##suffix biquad_inst_fut = { 0 };         \
      arm_biquad_casd_##config_suffix##_inst_##suffix biquad_inst_ref = { 0 };         \
                                                                                       \
      TEMPLATE_DO_ARR_DESC(                                                            \
            blocksize_idx, uint32_t, blockSize, filtering_blocksizes                   \
            ,                                                                          \
         TEMPLATE_DO_ARR_DESC(                                                         \
               numstages_idx, uint16_t, numStages, filtering_numstages                 \
               ,                                                                       \
            TEMPLATE_DO_ARR_DESC(                                                      \
                  postshifts_idx, uint8_t, postShift, filtering_postshifts             \
                  ,                                                                    \
                  /* Display test parameter values */                                  \
                  JTEST_DUMP_STRF("Block Size: %d\n"                                   \
                                  "Number of Stages: %d\n"                             \
                                  "Post Shift: %d\n",                                  \
                                  (int)blockSize,                                      \
                                  (int)numStages,                                      \
                                  (int)postShift);                                     \
                                                                                       \
                  /* Initialize the BIQUAD Instances */                                \
                  arm_biquad_cascade_##config_suffix##_init_##suffix(                  \
                        &biquad_inst_fut, numStages,                                   \
                        (output_type*)filtering_coeffs_b_##suffix,                     \
                        (void *) filtering_pState, postShift);                         \
                                                                                       \
                  JTEST_COUNT_CYCLES(                                                  \
                        arm_biquad_cascade_##config_suffix##speed##_##suffix(          \
                              &biquad_inst_fut,                                        \
                              (void *) filtering_##suffix##_inputs,                    \
                              (void *) filtering_output_fut,                           \
                              blockSize));                                             \
                                                                                       \
                  arm_biquad_cascade_##config_suffix##_init_##suffix(                  \
                        &biquad_inst_ref, numStages,                                   \
                        (output_type*)filtering_coeffs_b_##suffix,                     \
                        (void *) filtering_pState, postShift);                         \
                                                                                       \
                  ref_biquad_cascade_##config_suffix##speed##_##suffix(                \
                        &biquad_inst_ref,                                              \
                        (void *) filtering_##suffix##_inputs,                          \
                        (void *) filtering_output_ref,                                 \
                        blockSize);                                                    \
                                                                                       \
                  FILTERING_SNR_COMPARE_INTERFACE(                                     \
                        blockSize,                                                     \
                        output_type))));                                               \
                                                                                       \
            return JTEST_TEST_PASSED;                                                  \
   }


JTEST_DEFINE_TEST(arm_biquad_cas_df1_32x64_q31_test,
      arm_biquad_cas_df1_32x64_q31)
{
   arm_biquad_cas_df1_32x64_ins_q31 biquad_inst_fut = { 0 };
   arm_biquad_cas_df1_32x64_ins_q31 biquad_inst_ref = { 0 };

   TEMPLATE_DO_ARR_DESC(
         blocksize_idx, uint32_t, blockSize, filtering_blocksizes
         ,
      TEMPLATE_DO_ARR_DESC(
            numstages_idx, uint16_t, numStages, filtering_numstages
            ,
         TEMPLATE_DO_ARR_DESC(
               postshifts_idx, uint8_t, postShift, filtering_postshifts
               ,
               /* Initialize the BIQUAD Instances */
               arm_biquad_cas_df1_32x64_init_q31(
                     &biquad_inst_fut, numStages,
                     (q31_t*)filtering_coeffs_b_q31,
                     (void *) filtering_pState, postShift);

              /* Display test parameter values */
              JTEST_DUMP_STRF("Block Size: %d\n"
                              "Number of Stages: %d\n",
                              (int)blockSize,
                              (int)numStages);

               JTEST_COUNT_CYCLES(
                     arm_biquad_cas_df1_32x64_q31(
                           &biquad_inst_fut,
                           (void *) filtering_q31_inputs,
                           (void *) filtering_output_fut,
                           blockSize));

               arm_biquad_cas_df1_32x64_init_q31(
                     &biquad_inst_ref, numStages,
                     (q31_t*)filtering_coeffs_b_q31,
                     (void *) filtering_pState, postShift);

               ref_biquad_cas_df1_32x64_q31(
                     &biquad_inst_ref,
                     (void *) filtering_q31_inputs,
                     (void *) filtering_output_ref,
                     blockSize);

               FILTERING_SNR_COMPARE_INTERFACE(
                     blockSize,
                     q31_t))));

         return JTEST_TEST_PASSED;
}

JTEST_DEFINE_TEST(arm_biquad_cascade_df2T_f64_test,
     arm_biquad_cascade_df2T_f64)
{
  arm_biquad_cascade_df2T_instance_f64 biquad_inst_fut = { 0 };
  arm_biquad_cascade_df2T_instance_f64 biquad_inst_ref = { 0 };

  TEMPLATE_DO_ARR_DESC(
        blocksize_idx, uint32_t, blockSize, filtering_blocksizes
        ,
     TEMPLATE_DO_ARR_DESC(
           numstages_idx, uint16_t, numStages, filtering_numstages
           ,
           /* Display test parameter values */
           JTEST_DUMP_STRF("Block Size: %d\n"
                           "Number of Stages: %d\n",
                           (int)blockSize,
                           (int)numStages);

           /* Initialize the BIQUAD Instances */
           arm_biquad_cascade_df2T_init_f64(
                 &biquad_inst_fut, numStages,
                 (float64_t*)filtering_coeffs_b_f64,
                 (void *) filtering_pState);

           JTEST_COUNT_CYCLES(
                 arm_biquad_cascade_df2T_f64(
                       &biquad_inst_fut,
                       (void *) filtering_f64_inputs,
                       (void *) filtering_output_fut,
                       blockSize));

           arm_biquad_cascade_df2T_init_f64(
                 &biquad_inst_ref, numStages,
                 (float64_t*)filtering_coeffs_b_f64,
                 (void *) filtering_pState);

           ref_biquad_cascade_df2T_f64(
                 &biquad_inst_ref,
                 (void *) filtering_f64_inputs,
                 (void *) filtering_output_ref,
                 blockSize);

           FILTERING_DBL_SNR_COMPARE_INTERFACE(
                 blockSize,
                 float64_t)));

        return JTEST_TEST_PASSED;
}


BIQUAD_DEFINE_TEST(f32,arm_biquad_casd_df1_inst_f32, df1,float32_t);
BIQUAD_DEFINE_TEST(f32,arm_biquad_cascade_df2T_instance_f32,df2T,float32_t);
BIQUAD_DEFINE_TEST(f32,arm_biquad_cascade_stereo_df2T_instance_f32,stereo_df2T,float32_t);
BIQUAD_WITH_POSTSHIFT_DEFINE_TEST(q31,df1,,q31_t);
BIQUAD_WITH_POSTSHIFT_DEFINE_TEST(q15,df1,,q15_t);
BIQUAD_WITH_POSTSHIFT_DEFINE_TEST(q31,df1,_fast,q31_t);
BIQUAD_WITH_POSTSHIFT_DEFINE_TEST(q15,df1,_fast,q15_t);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(biquad_tests)
{
    /*
      To skip a test, comment it out.
    */
   JTEST_TEST_CALL(arm_biquad_cascade_df1_f32_test);
   JTEST_TEST_CALL(arm_biquad_cascade_df2T_f32_test);
   JTEST_TEST_CALL(arm_biquad_cascade_stereo_df2T_f32_test);
   JTEST_TEST_CALL(arm_biquad_cascade_df2T_f64_test);
   JTEST_TEST_CALL(arm_biquad_cascade_df1_q31_test);
   JTEST_TEST_CALL(arm_biquad_cascade_df1_q15_test);
   JTEST_TEST_CALL(arm_biquad_cascade_df1_fast_q31_test);
   JTEST_TEST_CALL(arm_biquad_cascade_df1_fast_q15_test);
   JTEST_TEST_CALL(arm_biquad_cas_df1_32x64_q31_test);
}
