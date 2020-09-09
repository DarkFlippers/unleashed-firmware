#ifndef _INTRINSICS_TEMPLATES_H_
#define _INTRINSICS_TEMPLATES_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "test_templates.h"
#include <string.h>             /* memcpy() */

/*--------------------------------------------------------------------------------*/
/* Group Specific Templates */
/*--------------------------------------------------------------------------------*/

/**
* Comparison SNR thresholds for the data types used in transform_tests.
*/
#define INTRINSICS_SNR_THRESHOLD_q63_t     120
#define INTRINSICS_SNR_THRESHOLD_q31_t     95

/**
*  Compare the outputs from the function under test and the reference
*  function using SNR.
*/
#define INTRINSICS_SNR_COMPARE_INTERFACE(block_size,  \
   output_type)                                       \
   do                                                 \
   {                                                  \
      TEST_CONVERT_AND_ASSERT_SNR(                    \
         intrinsics_output_f32_ref,                   \
         (output_type##_t *) intrinsics_output_ref,   \
         intrinsics_output_f32_fut,                   \
         (output_type##_t *) intrinsics_output_fut,   \
         block_size,                                  \
         output_type,                                 \
         INTRINSICS_SNR_THRESHOLD_##output_type##_t   \
      );                                              \
   } while (0)


/*--------------------------------------------------------------------------------*/
/* TEST Templates */
/*--------------------------------------------------------------------------------*/

#define INTRINSICS_TEST_TEMPLATE_ELT1(functionName, dataType)              \
                                                                           \
   JTEST_DEFINE_TEST(functionName##_test, functionName)                    \
   {                                                                       \
      uint32_t i;                                                          \
                                                                           \
      JTEST_COUNT_CYCLES(                                                  \
         for(i=0;i<INTRINSICS_MAX_LEN;i++)                                 \
         {                                                                 \
            *((dataType##_t*)intrinsics_output_fut + i) =                  \
               functionName(                                               \
                  (dataType##_t)intrinsics_##dataType##_inputs[i]);        \
         });                                                               \
                                                                           \
      for(i=0;i<INTRINSICS_MAX_LEN;i++)                                    \
      {                                                                    \
         *((dataType##_t*)intrinsics_output_ref + i) =                     \
            ref##functionName(                                             \
               (dataType##_t)intrinsics_##dataType##_inputs[i]);           \
      }                                                                    \
                                                                           \
      INTRINSICS_SNR_COMPARE_INTERFACE(                                    \
         INTRINSICS_MAX_LEN,                                               \
         dataType);                                                        \
                                                                           \
      return JTEST_TEST_PASSED;                                            \
   }

#define INTRINSICS_TEST_TEMPLATE_ELT2(functionName, dataType)              \
                                                                           \
   JTEST_DEFINE_TEST(functionName##_test, functionName)                    \
   {                                                                       \
      uint32_t i;                                                          \
                                                                           \
      JTEST_COUNT_CYCLES(                                                  \
         for(i=0;i<INTRINSICS_MAX_LEN;i++)                                 \
         {                                                                 \
            *((dataType##_t*)intrinsics_output_fut + i) =                  \
               functionName(                                               \
                  (dataType##_t)intrinsics_##dataType##_inputs[i]          \
                  ,(dataType##_t)intrinsics_##dataType##_inputs[i]);       \
         });                                                               \
                                                                           \
      for(i=0;i<INTRINSICS_MAX_LEN;i++)                                    \
      {                                                                    \
         *((dataType##_t*)intrinsics_output_ref + i) =                     \
            ref##functionName(                                             \
               (dataType##_t)intrinsics_##dataType##_inputs[i]             \
               ,(dataType##_t)intrinsics_##dataType##_inputs[i]);          \
      }                                                                    \
                                                                           \
      INTRINSICS_SNR_COMPARE_INTERFACE(                                    \
         INTRINSICS_MAX_LEN,                                               \
         dataType);                                                        \
                                                                           \
      return JTEST_TEST_PASSED;                                            \
   }

#define INTRINSICS_TEST_TEMPLATE_ELT3(functionName, dataType)              \
                                                                           \
   JTEST_DEFINE_TEST(functionName##_test, functionName)                    \
   {                                                                       \
      uint32_t i;                                                          \
                                                                           \
      JTEST_COUNT_CYCLES(                                                  \
         for(i=0;i<INTRINSICS_MAX_LEN;i++)                                 \
         {                                                                 \
            *((dataType##_t*)intrinsics_output_fut + i) =                  \
               functionName(                                               \
                  (dataType##_t)intrinsics_##dataType##_inputs[i]          \
                  ,(dataType##_t)intrinsics_##dataType##_inputs[i]         \
                  ,(dataType##_t)intrinsics_##dataType##_inputs[i]);       \
         });                                                               \
                                                                           \
      for(i=0;i<INTRINSICS_MAX_LEN;i++)                                    \
      {                                                                    \
         *((dataType##_t*)intrinsics_output_ref + i) =                     \
            ref##functionName(                                             \
               (dataType##_t)intrinsics_##dataType##_inputs[i]             \
               ,(dataType##_t)intrinsics_##dataType##_inputs[i]            \
               ,(dataType##_t)intrinsics_##dataType##_inputs[i]);          \
      }                                                                    \
                                                                           \
      INTRINSICS_SNR_COMPARE_INTERFACE(                                    \
         INTRINSICS_MAX_LEN,                                               \
         dataType);                                                        \
                                                                           \
      return JTEST_TEST_PASSED;                                            \
   }

#define INTRINSICS_TEST_TEMPLATE_ELT4(functionName, dataType, dataType2)   \
   JTEST_DEFINE_TEST(functionName##_test, functionName)                    \
   {                                                                       \
      uint32_t i;                                                          \
                                                                           \
      JTEST_COUNT_CYCLES(                                                  \
         for(i=0;i<INTRINSICS_MAX_LEN;i++)                                 \
         {                                                                 \
            *((dataType2##_t*)intrinsics_output_fut + i) =                 \
               functionName(                                               \
                  (dataType##_t)intrinsics_##dataType##_inputs[i]          \
                  ,(dataType##_t)intrinsics_##dataType##_inputs[i]         \
                  ,(dataType2##_t)intrinsics_##dataType2##_inputs[i]);     \
         });                                                               \
                                                                           \
      for(i=0;i<INTRINSICS_MAX_LEN;i++)                                    \
      {                                                                    \
         *((dataType2##_t*)intrinsics_output_ref + i) =                    \
            ref##functionName(                                             \
               (dataType##_t)intrinsics_##dataType##_inputs[i]             \
               ,(dataType##_t)intrinsics_##dataType##_inputs[i]            \
               ,(dataType2##_t)intrinsics_##dataType2##_inputs[i]);        \
      }                                                                    \
                                                                           \
      INTRINSICS_SNR_COMPARE_INTERFACE(                                    \
         INTRINSICS_MAX_LEN,                                               \
         dataType2);                                                       \
                                                                           \
      return JTEST_TEST_PASSED;                                            \
   }

#endif /* _INTRINSICS_TEMPLATES_H_ */
