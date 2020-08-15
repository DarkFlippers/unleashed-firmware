#include "jtest.h"
#include "filtering_test_data.h"
#include "arr_desc.h"
#include "arm_math.h"           /* FUTs */
#include "ref.h"                /* Reference Functions */
#include "test_templates.h"
#include "filtering_templates.h"
#include "type_abbrev.h"

#define FIR_DEFINE_TEST(suffix, config_suffix, output_type)             \
   JTEST_DEFINE_TEST(arm_fir##config_suffix##_##suffix##_test,          \
         arm_fir##config_suffix##_##suffix)                             \
   {                                                                    \
      arm_fir_instance_##suffix fir_inst_fut = { 0 };                   \
      arm_fir_instance_##suffix fir_inst_ref = { 0 };                   \
                                                                        \
      TEMPLATE_DO_ARR_DESC(                                             \
            blocksize_idx, uint32_t, blockSize, filtering_blocksizes    \
            ,                                                           \
         TEMPLATE_DO_ARR_DESC(                                          \
               numtaps_idx, uint16_t, numTaps, filtering_numtaps        \
               ,                                                        \
               /* Initialize the FIR Instances */                       \
               arm_fir_init_##suffix(                                   \
                     &fir_inst_fut, numTaps,                            \
                     (output_type*)filtering_coeffs_##suffix,           \
                     (void *) filtering_pState, blockSize);             \
                                                                        \
              /* Display test parameter values */                       \
              JTEST_DUMP_STRF("Block Size: %d\n"                        \
                              "Number of Taps: %d\n",                   \
                         (int)blockSize,                                \
                         (int)numTaps);                                 \
                                                                        \
               JTEST_COUNT_CYCLES(                                      \
                     arm_fir##config_suffix##_##suffix(                 \
                           &fir_inst_fut,                               \
                           (void *) filtering_##suffix##_inputs,        \
                           (void *) filtering_output_fut,               \
                           blockSize));                                 \
                                                                        \
               arm_fir_init_##suffix(                                   \
                     &fir_inst_ref, numTaps,                            \
                     (output_type*)filtering_coeffs_##suffix,           \
                     (void *) filtering_pState, blockSize);             \
                                                                        \
               ref_fir##config_suffix##_##suffix(                       \
                     &fir_inst_ref,                                     \
                     (void *) filtering_##suffix##_inputs,              \
                     (void *) filtering_output_ref,                     \
                     blockSize);                                        \
                                                                        \
               FILTERING_SNR_COMPARE_INTERFACE(                         \
                     blockSize,                                         \
                     output_type)));                                    \
                                                                        \
            return JTEST_TEST_PASSED;                                   \
   }

#define FIR_INTERPOLATE_DEFINE_TEST(suffix, output_type)                      \
   JTEST_DEFINE_TEST(arm_fir_interpolate_##suffix##_test,                     \
         arm_fir_interpolate_##suffix)                                        \
   {                                                                          \
      arm_fir_interpolate_instance_##suffix fir_inst_fut = { 0 };             \
      arm_fir_interpolate_instance_##suffix fir_inst_ref = { 0 };             \
                                                                              \
      TEMPLATE_DO_ARR_DESC(                                                   \
            blocksize_idx, uint32_t, blockSize, filtering_blocksizes          \
            ,                                                                 \
         TEMPLATE_DO_ARR_DESC(                                                \
               numtaps_idx, uint16_t, numTaps, filtering_numtaps2             \
               ,                                                              \
            TEMPLATE_DO_ARR_DESC(                                             \
                  L_idx, uint8_t, L, filtering_Ls                             \
                  ,                                                           \
                  /* Display test parameter values */                         \
                  JTEST_DUMP_STRF("Block Size: %d\n"                          \
                                 "Number of Taps: %d\n"                       \
                                 "Upsample factor: %d\n",                     \
                                (int)blockSize,                               \
                                (int)numTaps,                                 \
                                (int)L);                                      \
                                                                              \
                  /* Initialize the FIR Instances */                          \
                  arm_fir_interpolate_init_##suffix(                          \
                        &fir_inst_fut, L, numTaps,                            \
                        (output_type*)filtering_coeffs_##suffix,              \
                        (void *) filtering_pState, blockSize);                \
                                                                              \
                  JTEST_COUNT_CYCLES(                                         \
                        arm_fir_interpolate_##suffix(                         \
                              &fir_inst_fut,                                  \
                              (void *) filtering_##suffix##_inputs,           \
                              (void *) filtering_output_fut,                  \
                              blockSize));                                    \
                                                                              \
                  arm_fir_interpolate_init_##suffix(                          \
                        &fir_inst_ref, L, numTaps,                            \
                        (output_type*)filtering_coeffs_##suffix,              \
                        (void *) filtering_pState, blockSize);                \
                                                                              \
                  ref_fir_interpolate_##suffix(                               \
                        &fir_inst_ref,                                        \
                        (void *) filtering_##suffix##_inputs,                 \
                        (void *) filtering_output_ref,                        \
                        blockSize);                                           \
                                                                              \
                  FILTERING_SNR_COMPARE_INTERFACE(                            \
                        blockSize * (uint32_t)L,                              \
                        output_type))));                                      \
                                                                              \
            return JTEST_TEST_PASSED;                                         \
   }

#define FIR_DECIMATE_DEFINE_TEST(suffix, config_suffix, output_type)       \
   JTEST_DEFINE_TEST(arm_fir_decimate##config_suffix##_##suffix##_test,    \
         arm_fir_decimate##config_suffix##_##suffix)                       \
   {                                                                       \
      arm_fir_decimate_instance_##suffix fir_inst_fut = { 0 };             \
      arm_fir_decimate_instance_##suffix fir_inst_ref = { 0 };             \
                                                                           \
      TEMPLATE_DO_ARR_DESC(                                                \
            blocksize_idx, uint32_t, blockSize, filtering_blocksizes       \
            ,                                                              \
         TEMPLATE_DO_ARR_DESC(                                             \
               numtaps_idx, uint16_t, numTaps, filtering_numtaps           \
               ,                                                           \
            TEMPLATE_DO_ARR_DESC(                                          \
                  M_idx, uint8_t, M, filtering_Ms                          \
                  ,                                                        \
                  if (blockSize % M == 0)                                   \
                  {                                                        \
                     /* Display test parameter values */                   \
                     JTEST_DUMP_STRF("Block Size: %d\n"                    \
                                     "Number of Taps: %d\n"                \
                                     "Decimation Factor: %d\n",            \
                                     (int)blockSize,                       \
                                     (int)numTaps,                         \
                                     (int)M);                              \
                                                                           \
                     /* Initialize the FIR Instances */                    \
                     arm_fir_decimate_init_##suffix(                       \
                           &fir_inst_fut, numTaps, M,                      \
                           (output_type*)filtering_coeffs_##suffix,        \
                           (void *) filtering_pState, blockSize);          \
                                                                           \
                     JTEST_COUNT_CYCLES(                                   \
                           arm_fir_decimate##config_suffix##_##suffix(     \
                                 &fir_inst_fut,                            \
                                 (void *) filtering_##suffix##_inputs,     \
                                 (void *) filtering_output_fut,            \
                                 blockSize));                              \
                                                                           \
                     arm_fir_decimate_init_##suffix(                       \
                           &fir_inst_ref, numTaps, M,                      \
                           (output_type*)filtering_coeffs_##suffix,        \
                           (void *) filtering_pState, blockSize);          \
                                                                           \
                     ref_fir_decimate##config_suffix##_##suffix(           \
                           &fir_inst_ref,                                  \
                           (void *) filtering_##suffix##_inputs,           \
                           (void *) filtering_output_ref,                  \
                           blockSize);                                     \
                                                                           \
                     FILTERING_SNR_COMPARE_INTERFACE(                      \
                           blockSize / M,                                  \
                           output_type);                                   \
                  })));                                                    \
                                                                           \
            return JTEST_TEST_PASSED;                                      \
   }

#define FIR_LATTICE_DEFINE_TEST(suffix, output_type)                       \
   JTEST_DEFINE_TEST(arm_fir_lattice_##suffix##_test,                      \
         arm_fir_lattice_##suffix)                                         \
   {                                                                       \
      arm_fir_lattice_instance_##suffix fir_inst_fut = { 0 };              \
      arm_fir_lattice_instance_##suffix fir_inst_ref = { 0 };              \
                                                                           \
      TEMPLATE_DO_ARR_DESC(                                                \
            blocksize_idx, uint32_t, blockSize, filtering_blocksizes       \
            ,                                                              \
         TEMPLATE_DO_ARR_DESC(                                             \
               numstages_idx, uint16_t, numStages, filtering_numstages     \
               ,                                                           \
               /* Display test parameter values */                         \
               JTEST_DUMP_STRF("Block Size: %d\n"                          \
                               "Number of Stages: %d\n",                   \
                               (int)blockSize,                             \
                               (int)numStages);                            \
                                                                           \
               /* Initialize the FIR Instances */                          \
               arm_fir_lattice_init_##suffix(                              \
                     &fir_inst_fut, numStages,                             \
                     (output_type*)filtering_coeffs_##suffix,              \
                     (void *) filtering_pState);                           \
                                                                           \
               JTEST_COUNT_CYCLES(                                         \
                     arm_fir_lattice_##suffix(                             \
                           &fir_inst_fut,                                  \
                           (void *) filtering_##suffix##_inputs,           \
                           (void *) filtering_output_fut,                  \
                           blockSize));                                    \
                                                                           \
               arm_fir_lattice_init_##suffix(                              \
                     &fir_inst_ref, numStages,                             \
                     (output_type*)filtering_coeffs_##suffix,              \
                     (void *) filtering_pState);                           \
                                                                           \
               ref_fir_lattice_##suffix(                                   \
                     &fir_inst_ref,                                        \
                     (void *) filtering_##suffix##_inputs,                 \
                     (void *) filtering_output_ref,                        \
                     blockSize);                                           \
                                                                           \
               FILTERING_SNR_COMPARE_INTERFACE(                            \
                     blockSize,                                            \
                     output_type)));                                       \
                                                                           \
            return JTEST_TEST_PASSED;                                      \
   }


#define FIR_SPARSE_DEFINE_TEST(suffix, output_type)                     \
   JTEST_DEFINE_TEST(arm_fir_sparse_##suffix##_test,                    \
         arm_fir_sparse_##suffix)                                       \
   {                                                                    \
      arm_fir_sparse_instance_##suffix fir_inst_fut = { 0 };            \
      arm_fir_sparse_instance_##suffix fir_inst_ref = { 0 };            \
                                                                        \
      TEMPLATE_DO_ARR_DESC(                                             \
            blocksize_idx, uint32_t, blockSize, filtering_blocksizes    \
            ,                                                           \
         TEMPLATE_DO_ARR_DESC(                                          \
               numtaps_idx, uint16_t, numTaps, filtering_numtaps        \
               ,                                                        \
               /* Display test parameter values */                      \
               JTEST_DUMP_STRF("Block Size: %d\n"                       \
                               "Number of Taps: %d\n"                   \
                               "Tap Delay: %d\n",                       \
                               (int)blockSize,                          \
                               (int)numTaps,                            \
                               (int)FILTERING_MAX_TAP_DELAY);           \
                                                                        \
               /* Initialize the FIR Instances */                       \
               arm_fir_sparse_init_##suffix(                            \
                     &fir_inst_fut, numTaps,                            \
                     (output_type*)filtering_coeffs_##suffix,           \
                     (void *) filtering_pState,                         \
                     (int32_t*)filtering_tap_delay,                     \
                     FILTERING_MAX_TAP_DELAY, blockSize);               \
                                                                        \
               JTEST_COUNT_CYCLES(                                      \
                     arm_fir_sparse_##suffix(                           \
                           &fir_inst_fut,                               \
                           (void *) filtering_##suffix##_inputs,        \
                           (void *) filtering_output_fut,               \
                           (void *) filtering_scratch,                  \
                           blockSize));                                 \
                                                                        \
               arm_fir_sparse_init_##suffix(                            \
                     &fir_inst_ref, numTaps,                            \
                     (output_type*)filtering_coeffs_##suffix,           \
                     (void *) filtering_pState,                         \
                     (int32_t*)filtering_tap_delay,                     \
                     FILTERING_MAX_TAP_DELAY, blockSize);               \
                                                                        \
               ref_fir_sparse_##suffix(                                 \
                     &fir_inst_ref,                                     \
                     (void *) filtering_##suffix##_inputs,              \
                     (void *) filtering_output_ref,                     \
                     (void *) filtering_scratch,                        \
                     blockSize);                                        \
                                                                        \
               FILTERING_SNR_COMPARE_INTERFACE(                         \
                     blockSize,                                         \
                     output_type)));                                    \
                                                                        \
            return JTEST_TEST_PASSED;                                   \
   }

#define FIR_SPARSE2_DEFINE_TEST(suffix, output_type)                    \
   JTEST_DEFINE_TEST(arm_fir_sparse_##suffix##_test,                    \
         arm_fir_sparse_##suffix)                                       \
   {                                                                    \
      arm_fir_sparse_instance_##suffix fir_inst_fut = { 0 };            \
      arm_fir_sparse_instance_##suffix fir_inst_ref = { 0 };            \
                                                                        \
      TEMPLATE_DO_ARR_DESC(                                             \
            blocksize_idx, uint32_t, blockSize, filtering_blocksizes    \
            ,                                                           \
         TEMPLATE_DO_ARR_DESC(                                          \
               numtaps_idx, uint16_t, numTaps, filtering_numtaps        \
               ,                                                        \
              /* Display test parameter values */                       \
              JTEST_DUMP_STRF("Block Size: %d\n"                        \
                              "Number of Taps: %d\n"                    \
                              "Tap Delay: %d\n",                        \
                              (int)blockSize,                           \
                              (int)numTaps,                             \
                              (int)FILTERING_MAX_TAP_DELAY);            \
                                                                        \
               /* Initialize the FIR Instances */                       \
               arm_fir_sparse_init_##suffix(                            \
                     &fir_inst_fut, numTaps,                            \
                     (output_type*)filtering_coeffs_##suffix,           \
                     (void *) filtering_pState,                         \
                     (int32_t*)filtering_tap_delay,                     \
                     FILTERING_MAX_TAP_DELAY, blockSize);               \
                                                                        \
               JTEST_COUNT_CYCLES(                                      \
                     arm_fir_sparse_##suffix(                           \
                           &fir_inst_fut,                               \
                           (void *) filtering_##suffix##_inputs,        \
                           (void *) filtering_output_fut,               \
                           (void *) filtering_scratch,                  \
                           (void *) filtering_scratch2,                 \
                           blockSize));                                 \
                                                                        \
               arm_fir_sparse_init_##suffix(                            \
                     &fir_inst_ref, numTaps,                            \
                     (output_type*)filtering_coeffs_##suffix,           \
                     (void *) filtering_pState,                         \
                     (int32_t*)filtering_tap_delay,                     \
                     FILTERING_MAX_TAP_DELAY, blockSize);               \
                                                                        \
               ref_fir_sparse_##suffix(                                 \
                     &fir_inst_ref,                                     \
                     (void *) filtering_##suffix##_inputs,              \
                     (void *) filtering_output_ref,                     \
                     (void *) filtering_scratch,                        \
                     (void *) filtering_scratch2,                       \
                     blockSize);                                        \
                                                                        \
               FILTERING_SNR_COMPARE_INTERFACE(                         \
                     blockSize,                                         \
                     output_type)));                                    \
                                                                        \
            return JTEST_TEST_PASSED;                                   \
   }

FIR_DEFINE_TEST(f32,,float32_t);
FIR_DEFINE_TEST(q31,,q31_t);
FIR_DEFINE_TEST(q15,,q15_t);
FIR_DEFINE_TEST(q31,_fast,q31_t);
FIR_DEFINE_TEST(q15,_fast,q15_t);
FIR_DEFINE_TEST(q7,,q7_t);

FIR_LATTICE_DEFINE_TEST(f32,float32_t);
FIR_LATTICE_DEFINE_TEST(q31,q31_t);
FIR_LATTICE_DEFINE_TEST(q15,q15_t);

FIR_INTERPOLATE_DEFINE_TEST(f32,float32_t);
FIR_INTERPOLATE_DEFINE_TEST(q31,q31_t);
FIR_INTERPOLATE_DEFINE_TEST(q15,q15_t);

FIR_DECIMATE_DEFINE_TEST(f32,,float32_t);
FIR_DECIMATE_DEFINE_TEST(q31,,q31_t);
FIR_DECIMATE_DEFINE_TEST(q15,,q15_t);
FIR_DECIMATE_DEFINE_TEST(q31,_fast,q31_t);
FIR_DECIMATE_DEFINE_TEST(q15,_fast,q15_t);

FIR_SPARSE_DEFINE_TEST(f32,float32_t);
FIR_SPARSE_DEFINE_TEST(q31,q31_t);
FIR_SPARSE2_DEFINE_TEST(q15,q15_t);
FIR_SPARSE2_DEFINE_TEST(q7,q7_t);

/*--------------------------------------------------------------------------------*/
/* Collect all tests in a group. */
/*--------------------------------------------------------------------------------*/

JTEST_DEFINE_GROUP(fir_tests)
{
    /*
      To skip a test, comment it out.
    */
   JTEST_TEST_CALL(arm_fir_f32_test);
   JTEST_TEST_CALL(arm_fir_q31_test);
   JTEST_TEST_CALL(arm_fir_q15_test);
   JTEST_TEST_CALL(arm_fir_q7_test);
   JTEST_TEST_CALL(arm_fir_fast_q31_test);
   JTEST_TEST_CALL(arm_fir_fast_q15_test);

   JTEST_TEST_CALL(arm_fir_lattice_f32_test);
   JTEST_TEST_CALL(arm_fir_lattice_q31_test);
   JTEST_TEST_CALL(arm_fir_lattice_q15_test);

   JTEST_TEST_CALL(arm_fir_interpolate_f32_test);
   JTEST_TEST_CALL(arm_fir_interpolate_q31_test);
   JTEST_TEST_CALL(arm_fir_interpolate_q15_test);

   JTEST_TEST_CALL(arm_fir_decimate_f32_test);
   JTEST_TEST_CALL(arm_fir_decimate_q31_test);
   JTEST_TEST_CALL(arm_fir_decimate_q15_test);
   JTEST_TEST_CALL(arm_fir_decimate_fast_q31_test);
   JTEST_TEST_CALL(arm_fir_decimate_fast_q15_test);

   JTEST_TEST_CALL(arm_fir_sparse_f32_test);
   JTEST_TEST_CALL(arm_fir_sparse_q31_test);
   JTEST_TEST_CALL(arm_fir_sparse_q15_test);
   JTEST_TEST_CALL(arm_fir_sparse_q7_test);
}
