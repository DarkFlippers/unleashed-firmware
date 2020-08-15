#ifndef _TEST_TEMPLATES_H_
#define _TEST_TEMPLATES_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/
#include "template.h"
#include <string.h>             /* memcmp() */
#include <inttypes.h>           /* PRIu32 */
#include "math_helper.h"        /* arm_snr_f32() */

/*--------------------------------------------------------------------------------*/
/* Function Aliases for use in Templates. */
/*--------------------------------------------------------------------------------*/
#define ref_q31_t_to_float ref_q31_to_float
#define ref_q15_t_to_float ref_q15_to_float
#define ref_q7_t_to_float  ref_q7_to_float
#define ref_float_to_q31_t ref_float_to_q31
#define ref_float_to_q15_t ref_float_to_q15
#define ref_float_to_q7_t  ref_float_to_q7
#define ref_float32_t_to_float ref_copy_f32
#define ref_float_to_float32_t ref_copy_f32


/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Call the function-under-test.
 */
#define TEST_CALL_FUT(fut, fut_args)                    \
    JTEST_COUNT_CYCLES(TEMPLATE_CALL_FN(fut, fut_args))

/**
 *  Call the reference-function.
 */
#define TEST_CALL_REF(ref, ref_args)            \
    TEMPLATE_CALL_FN(ref, ref_args)

/**
 *  Call the function-under-test and the reference-function.
 */
#define TEST_CALL_FUT_AND_REF(fut, fut_args, ref, ref_args) \
    do {                                                    \
        TEST_CALL_FUT(fut, fut_args);                       \
        TEST_CALL_REF(ref, ref_args);                       \
    } while (0)

/**
 *  This macro eats a variable number of arguments and evaluates to a null
 *  statement.
 */
#define TEST_NULL_STATEMENT(...) (void) "TEST_NULL_STATEMENT"

/**
 *  A function name, Usable in any template where a fut or ref name is accepted,
 *  that evaluates to a #TEST_NULL_STATEMENT().
 */
#define TEST_NULL_FN TEST_NULL_STATEMENT

/**
 *  Assert that buffers A and B are byte-equivalent for a number of bytes.
 */
#define TEST_ASSERT_BUFFERS_EQUAL(buf_a, buf_b, bytes)  \
    do                                                  \
    {                                                   \
        if (memcmp(buf_a, buf_b, bytes) != 0)           \
        {                                               \
            return JTEST_TEST_FAILED;                   \
        }                                               \
    } while (0)

/**
 *  Assert that the two entities are equal.
 */
#define TEST_ASSERT_EQUAL(a, b)                         \
    do                                                  \
    {                                                   \
        if ((a) != (b))                                 \
        {                                               \
            return JTEST_TEST_FAILED;                   \
        }                                               \
    } while (0)

/**
 *  Convert elements to from src_type to float.
 */
#define TEST_CONVERT_TO_FLOAT(src_ptr, dst_ptr, block_size, src_type)   \
    do                                                                  \
    {                                                                   \
        ref_##src_type##_to_float(                                      \
            src_ptr,                                                    \
            dst_ptr,                                                    \
            block_size);                                                \
        } while (0)                                                      \

/**
 *  Convert elements to from float to dst_type .
 */
#define TEST_CONVERT_FLOAT_TO(src_ptr, dst_ptr, block_size, dst_type)   \
    do                                                                  \
    {                                                                   \
        ref_float_to_##dst_type(                                        \
            src_ptr,                                                    \
            dst_ptr,                                                    \
            block_size);                                                \
    } while (0)                                                          \

/**
 *  Assert that the SNR between a reference and test sample is above a given
 *  threshold.
 */
#define TEST_ASSERT_SNR(ref_ptr, tst_ptr, block_size, threshold)    \
    do                                                              \
    {                                                               \
        float32_t snr = arm_snr_f32(ref_ptr, tst_ptr, block_size);  \
        if ( snr <= threshold)                                       \
        {                                                           \
            JTEST_DUMP_STRF("SNR: %f\n", snr);                      \
            return JTEST_TEST_FAILED;                               \
        }                                                           \
    } while (0)                                                      \

/**
 *  Assert that the SNR between a reference and test sample is above a given
 *  threshold.  Special case for float64_t
 */
#define TEST_ASSERT_DBL_SNR(ref_ptr, tst_ptr, block_size, threshold)    \
    do                                                              \
    {                                                               \
        float64_t snr = arm_snr_f64(ref_ptr, tst_ptr, block_size);  \
        if ( snr <= threshold)                                       \
        {                                                           \
            JTEST_DUMP_STRF("SNR: %f\n", snr);                      \
            return JTEST_TEST_FAILED;                               \
        }                                                           \
    } while (0)                                                      \

/**
 *  Compare test and reference elements by converting to float and
 *  calculating an SNR.
 *
 *  This macro is a merger of the #TEST_CONVERT_TO_FLOAT() and
 *  #TEST_ASSERT_SNR() macros.
 */
#define TEST_CONVERT_AND_ASSERT_SNR(ref_dst_ptr, ref_src_ptr,   \
                                    tst_dst_ptr, tst_src_ptr,   \
                                    block_size,                 \
                                    tst_src_type,               \
                                    threshold)                  \
        do                                                      \
        {                                                       \
            TEST_CONVERT_TO_FLOAT(ref_src_ptr,                  \
                                  ref_dst_ptr,                  \
                                  block_size,                   \
                                  tst_src_type);                \
            TEST_CONVERT_TO_FLOAT(tst_src_ptr,                  \
                                  tst_dst_ptr,                  \
                                  block_size,                   \
                                  tst_src_type);                \
            TEST_ASSERT_SNR(ref_dst_ptr,                        \
                            tst_dst_ptr,                        \
                            block_size,                         \
                            threshold);                         \
        } while (0)

/**
 *  Execute statements only if the combination of block size, function type
 *  specifier, and input ARR_DESC_t are valid.
 *
 *  @example An ARR_DESC_t that contains 64 bytes cant service a 32 element
 *  block size if they are extracted in float32_t increments.
 *
 *  8 * 32 = 256 > 64.
 */
#define TEST_DO_VALID_BLOCKSIZE(block_size, fn_type_spec,   \
                                input_arr_desc, body)       \
    do                                                      \
    {                                                       \
        if (block_size * sizeof(fn_type_spec) <=             \
           ARR_DESC_BYTES(input_arr_desc))                  \
        {                                                   \
            JTEST_DUMP_STRF("Block Size: %"PRIu32"\n", block_size); \
            body;                                           \
        }                                                   \
    } while (0)                                              \

/**
 *  Template for tests that rely on one input buffer and a blocksize parameter.
 *
 *  The buffer is an #ARR_DESC_t.  It is iterated over and it's values are
 *  passed to the function under test and reference functions through their
 *  appropriate argument interfaces.  The argument interfaces this template to
 *  execute structurally similar functions.
 *
 */
#define TEST_TEMPLATE_BUF1_BLK(arr_desc_inputs,                         \
                              arr_desc_block_sizes,                     \
                              input_type, output_type,                  \
                              fut, fut_arg_interface,                   \
                              ref, ref_arg_interface,                   \
                              compare_interface)                        \
    do                                                                  \
    {                                                                   \
        TEMPLATE_DO_ARR_DESC(                                           \
            input_idx, ARR_DESC_t *, input_ptr, arr_desc_inputs         \
            ,                                                           \
            TEMPLATE_DO_ARR_DESC(                                       \
                block_size_idx, uint32_t, block_size, arr_desc_block_sizes \
                ,                                                       \
                void *   input_data_ptr = input_ptr->data_ptr;          \
                                                                        \
                TEST_DO_VALID_BLOCKSIZE(                                \
                    block_size, input_type, input_ptr                   \
                    ,                                                   \
                    TEST_CALL_FUT_AND_REF(                              \
                        fut, fut_arg_interface(                         \
                            input_data_ptr, block_size),                \
                        ref, ref_arg_interface(                         \
                            input_data_ptr, block_size));               \
                                                                        \
                    compare_interface(block_size, output_type))));      \
                                                                        \
        return JTEST_TEST_PASSED;                                       \
                                                                        \
    } while (0)

/**
 *  Template for tests that rely on an input buffer and an element.
 *
 *  An element can is any thing which doesn't walk and talk like a
 *  sequence. Examples include numbers, and structures.
 */
#define TEST_TEMPLATE_BUF1_ELT1(arr_desc_inputs,                        \
                                arr_desc_elts,                          \
                                input_type, elt_type, output_type,      \
                                fut, fut_arg_interface,                 \
                                ref, ref_arg_interface,                 \
                                compare_interface)                      \
        do                                                              \
        {                                                               \
            TEMPLATE_DO_ARR_DESC(                                       \
                input_idx, ARR_DESC_t *, input_ptr, arr_desc_inputs     \
                ,                                                       \
                TEMPLATE_DO_ARR_DESC(                                   \
                    elt_idx, elt_type, elt, arr_desc_elts               \
                    ,                                                   \
                    void * input_data_ptr = input_ptr->data_ptr;        \
                    TEST_CALL_FUT_AND_REF(                              \
                        fut, fut_arg_interface(input_data_ptr, elt),    \
                        ref, ref_arg_interface(input_data_ptr, elt));   \
                                                                        \
                    compare_interface(output_type)));                   \
            return JTEST_TEST_PASSED;                                   \
        } while (0)

/**
 *  Template for tests that rely on an input buffer, an element, and a blocksize
 *  parameter.
 */
#define TEST_TEMPLATE_BUF1_ELT1_BLK(arr_desc_inputs,                \
                                    arr_desc_elts,                  \
                                    arr_desc_block_sizes,           \
                                    input_type, elt_type, output_type,  \
                                    fut, fut_arg_interface,         \
                                    ref, ref_arg_interface,         \
                                    compare_interface);             \
    do                                                              \
    {                                                               \
        TEMPLATE_DO_ARR_DESC(                                       \
            inut_idx, ARR_DESC_t *, input_ptr, arr_desc_inputs      \
            ,                                                       \
            TEMPLATE_DO_ARR_DESC(                                   \
                block_size_idx, uint32_t, block_size,               \
                arr_desc_block_sizes                                \
                ,                                                   \
                TEMPLATE_DO_ARR_DESC(                               \
                    elt_idx, elt_type, elt, arr_desc_elts           \
                    ,                                               \
                    void * input_data_ptr = input_ptr->data_ptr;    \
                    TEST_DO_VALID_BLOCKSIZE(                        \
                        block_size, input_type, input_ptr,          \
                                              \
                        TEST_CALL_FUT_AND_REF(                      \
                            fut, fut_arg_interface(                 \
                                input_data_ptr, elt, block_size),   \
                            ref, ref_arg_interface(                 \
                                input_data_ptr, elt, block_size));  \
                        compare_interface(block_size, output_type))))); \
        return JTEST_TEST_PASSED;                                   \
    } while (0)

/**
 *  Template for tests that rely on an input buffer, two elements, and a blocksize
 *  parameter.
 */
#define TEST_TEMPLATE_BUF1_ELT2_BLK(arr_desc_inputs,                    \
                                    arr_desc_elt1s,                     \
                                    arr_desc_elt2s,                     \
                                    arr_desc_block_sizes,               \
                                    input_type, elt1_type,              \
                                    elt2_type, output_type,             \
                                    fut, fut_arg_interface,             \
                                    ref, ref_arg_interface,             \
                                    compare_interface)                  \
        do                                                              \
        {                                                               \
            TEMPLATE_DO_ARR_DESC(                                       \
                inut_idx, ARR_DESC_t *, input_ptr, arr_desc_inputs      \
                ,                                                       \
                TEMPLATE_DO_ARR_DESC(                                   \
                    block_size_idx, uint32_t, block_size,               \
                    arr_desc_block_sizes                                \
                    ,                                                   \
                    TEMPLATE_DO_ARR_DESC(                               \
                        elt1_idx, elt1_type, elt1, arr_desc_elt1s       \
                        ,                                               \
                        TEMPLATE_DO_ARR_DESC(                           \
                            elt2_idx, elt2_type, elt2, arr_desc_elt2s   \
                            ,                                           \
                            void * input_data_ptr = input_ptr->data_ptr; \
                            TEST_DO_VALID_BLOCKSIZE(                    \
                                block_size, input_type, input_ptr,      \
                                TEST_CALL_FUT_AND_REF(                  \
                                    fut, fut_arg_interface(             \
                                        input_data_ptr, elt1, elt2, block_size), \
                                    ref, ref_arg_interface(             \
                                        input_data_ptr, elt1, elt2, block_size)); \
                                compare_interface(block_size, output_type)))))); \
            return JTEST_TEST_PASSED;                                   \
        } while (0)

/**
 *  Template for tests that rely on two input buffers and a blocksize parameter.
 *
 *  The two #ARR_DESC_t, input buffers are iterated through in parallel. The
 *  length of the first #ARR_DESC_t determines the length of the iteration.
 */
#define TEST_TEMPLATE_BUF2_BLK(arr_desc_inputs_a,                       \
                              arr_desc_inputs_b,                        \
                              arr_desc_block_sizes,                     \
                              input_type, output_type,                  \
                              fut, fut_arg_interface,                   \
                              ref, ref_arg_interface,                   \
                              compare_interface)                        \
    do                                                                  \
    {                                                                   \
        /* Iterate over two input arrays in parallel.*/                 \
        TEMPLATE_DO_ARR_DESC(                                           \
            input_idx, ARR_DESC_t *, input_ptr, arr_desc_inputs_a       \
            ,                                                           \
            TEMPLATE_DO_ARR_DESC(                                       \
                block_size_idx, uint32_t, block_size, arr_desc_block_sizes, \
                void * input_a_ptr = input_ptr->data_ptr;               \
                void * input_b_ptr = ARR_DESC_ELT(                      \
                    ARR_DESC_t *, input_idx,                            \
                    &(arr_desc_inputs_b))->data_ptr;                    \
                                                                        \
                TEST_DO_VALID_BLOCKSIZE(                                \
                    block_size, input_type, input_ptr                   \
                    ,                                                   \
                    TEST_CALL_FUT_AND_REF(                              \
                        fut, fut_arg_interface(                         \
                            input_a_ptr, input_b_ptr, block_size),      \
                        ref, ref_arg_interface(                         \
                            input_a_ptr, input_b_ptr, block_size));     \
                                                                        \
                    compare_interface(block_size, output_type))));      \
        return JTEST_TEST_PASSED;                                       \
    } while (0)

/**
 *  Test template that uses a single element.
 */
#define TEST_TEMPLATE_ELT1(arr_desc_elts,                       \
                           elt_type, output_type,               \
                           fut, fut_arg_interface,              \
                           ref, ref_arg_interface,              \
                           compare_interface)                   \
        do                                                      \
        {                                                       \
            TEMPLATE_DO_ARR_DESC(                               \
                elt_idx, elt_type, elt, arr_desc_elts           \
                ,                                               \
                TEST_CALL_FUT_AND_REF(                          \
                    fut, fut_arg_interface(                     \
                        elt),                                   \
                    ref, ref_arg_interface(                     \
                        elt));                                  \
                /* Comparison interfaces typically accept */    \
                /* a block_size. Pass a dummy value 1.*/        \
                compare_interface(1, output_type));             \
            return JTEST_TEST_PASSED;                           \
        } while (0)

/**
 *  Test template that iterates over two sets of elements in parallel.
 *
 *  The length of the first set determines the number of iteratsions.
 */
#define TEST_TEMPLATE_ELT2(arr_desc_elts_a,                     \
                           arr_desc_elts_b,                     \
                           elt_a_type, elt_b_type, output_type, \
                           fut, fut_arg_interface,              \
                           ref, ref_arg_interface,              \
                           compare_interface)                   \
        do                                                      \
        {                                                       \
            TEMPLATE_DO_ARR_DESC(                               \
                elt_a_idx, elt_a_type, elt_a, arr_desc_elts_a   \
                ,                                               \
                elt_b_type * elt_b = ARR_DESC_ELT(              \
                    elt_b_type,                                 \
                    elt_a_idx,                                  \
                    arr_desc_elts_b);                           \
                                                                \
                TEST_CALL_FUT_AND_REF(                          \
                    fut, fut_arg_interface(                     \
                        elt_a, elt_b),                          \
                    ref, ref_arg_interface(                     \
                        elt_a, elt_b));                         \
                /* Comparison interfaces typically accept */    \
                /* a block_size. Pass a dummy value 1.*/        \
                compare_interface(1, output_type));             \
            return JTEST_TEST_PASSED;                           \
        } while (0)

/**
 *  Test template that uses an element and a block size.
 */
#define TEST_TEMPLATE_ELT1_BLK(arr_desc_elts,                       \
                               arr_desc_block_sizes,                \
                               elt_type, output_type,               \
                               fut, fut_arg_interface,              \
                               ref, ref_arg_interface,              \
                               compare_interface)                   \
        do                                                          \
        {                                                           \
            TEMPLATE_DO_ARR_DESC(                                   \
                block_size_idx, uint32_t, block_size,               \
                arr_desc_block_sizes                                \
                ,                                                   \
                TEMPLATE_DO_ARR_DESC(                               \
                    elt_idx, elt_type, elt, arr_desc_elts           \
                    ,                                               \
                    JTEST_DUMP_STRF("Block Size: %d\n",             \
                         (int)block_size);                          \
                    TEST_CALL_FUT_AND_REF(                          \
                        fut, fut_arg_interface(                     \
                            elt, block_size),                       \
                        ref, ref_arg_interface(                     \
                            elt, block_size));                      \
                    compare_interface(block_size, output_type)));   \
            return JTEST_TEST_PASSED;                               \
        } while (0)

#endif /* _TEST_TEMPLATES_H_ */
