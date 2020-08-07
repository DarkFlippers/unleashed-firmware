#ifndef _FILTERING_TEMPLATES_H_
#define _FILTERING_TEMPLATES_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/
#include "test_templates.h"

/*--------------------------------------------------------------------------------*/
/* Group Specific Templates */
/*--------------------------------------------------------------------------------*/

/*
 * Comparison SNR thresholds for the data types used in statistics_tests.
 */
#define FILTERING_SNR_THRESHOLD_float64_t 120
#define FILTERING_SNR_THRESHOLD_float32_t 99
#define FILTERING_SNR_THRESHOLD_q31_t 90
#define FILTERING_SNR_THRESHOLD_q15_t 60
#define FILTERING_SNR_THRESHOLD_q7_t 30

/**
 *  Compare reference and fut outputs using SNR.
 *
 *  @note The outputs are converted to float32_t before comparison.
 */
#define FILTERING_SNR_COMPARE_INTERFACE(block_size,                     \
                                        output_type)                    \
    FILTERING_SNR_COMPARE_INTERFACE_OFFSET(0, block_size, output_type)

/**
 *  Compare reference and fut outputs starting at some offset using SNR.
 */
#define FILTERING_SNR_COMPARE_INTERFACE_OFFSET(offset,      \
                                               block_size,  \
                                               output_type) \
    do                                                      \
    {                                                       \
        TEST_CONVERT_AND_ASSERT_SNR(                        \
            filtering_output_f32_ref,                       \
            (output_type *) filtering_output_ref + offset,  \
            filtering_output_f32_fut,                       \
            (output_type *) filtering_output_fut + offset,  \
            block_size,                                     \
            output_type,                                    \
            FILTERING_SNR_THRESHOLD_##output_type           \
            );                                              \
    } while (0)                                              

/**
 *  Compare reference and fut outputs starting at some offset using SNR.
 *  Special case for float64_t
 */
#define FILTERING_DBL_SNR_COMPARE_INTERFACE(block_size,  				\
                                            output_type) 				\
    do                                                      		\
    {                                                       		\
        TEST_ASSERT_DBL_SNR(                        						\
            (float64_t*)filtering_output_ref,               \
            (float64_t*)filtering_output_fut,               \
            block_size,                                     		\
            FILTERING_SNR_THRESHOLD_##output_type           		\
            );                                              		\
    } while (0)                                              
		
/*--------------------------------------------------------------------------------*/
/* Input Interfaces */
/*--------------------------------------------------------------------------------*/
/*
 *  General:
 *  Input interfaces provide inputs to functions inside test templates.  They
 *  ONLY provide the inputs.  The output variables should be hard coded.
 *
 *  The input interfaces must have the following format:
 *
 *  ARM_xxx_INPUT_INTERFACE() or
 *  REF_xxx_INPUT_INTERFACE()
 *
 *  The xxx must be lowercase, and is intended to be the indentifying substring
 *  in the function's name.  Acceptable values are 'sub' or 'add' from the
 *  functions arm_add_q31.
 */


/*--------------------------------------------------------------------------------*/
/* Test Templates */
/*--------------------------------------------------------------------------------*/



#endif /* _FILTERING_TEMPLATES_H_ */
