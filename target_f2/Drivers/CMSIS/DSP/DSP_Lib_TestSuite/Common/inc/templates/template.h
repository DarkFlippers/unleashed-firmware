#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

/*--------------------------------------------------------------------------------*/
/* Looping and Iteration */
/*--------------------------------------------------------------------------------*/

/**
 *  Template for the general structure of a loop.
 */
#define TEMPLATE_LOOP(setup, loop_def, body)    \
    do                                          \
    {                                           \
        setup;                                  \
        loop_def {                              \
            body;                               \
        }                                       \
    } while (0)

/**
 *  Template for looping over an array-like sequence.
 */
#define TEMPLATE_DO_ARR_LIKE(iter_idx, type,                            \
                             arr, arr_length,                           \
                             iter_elem_setup,                           \
                             body)                                      \
    do                                                                  \
    {                                                                   \
        TEMPLATE_LOOP(                                                  \
            int iter_idx,                                               \
            for(iter_idx = 0; iter_idx < (arr_length); ++iter_idx),     \
            iter_elem_setup;                                            \
            body);                                                      \
    } while (0)

/**
 *  Template for looping over the contents of an array.
 */
#define TEMPLATE_DO_ARR(iter_idx, type, iter_elem, arr, arr_length, body) \
    do                                                                  \
    {                                                                   \
        TEMPLATE_DO_ARR_LIKE(                                           \
            iter_idx, type, arr, arr_length,                            \
            type iter_elem = (arr)[iter_idx],                           \
            body);                                                      \
    } while (0)

/**
 *  Template for looping over the contents of an #ARR_DESC.
 */
#define TEMPLATE_DO_ARR_DESC(iter_idx, type, iter_elem, arr_desc, body) \
    do                                                                  \
    {                                                                   \
        TEMPLATE_DO_ARR_LIKE(                                           \
            iter_idx, type, arr_desc, (arr_desc).element_count,         \
            type iter_elem = ARR_DESC_ELT(type, iter_idx, &(arr_desc)), \
            body);                                                      \
    } while (0)

/*--------------------------------------------------------------------------------*/
/* Test Definition */
/*--------------------------------------------------------------------------------*/

/**
 *  Template for the general structure of a test.
 */
#define TEMPLATE_TEST(setup, body, teardown)    \
        do                                      \
        {                                       \
            setup;                              \
            body;                               \
            teardown;                           \
        } while (0)

/**
 *  Template for calling a function.
 *
 *  @note Surround function arguments with the #PAREN() macro.
 *
 *  @example
 *  void my_func(int arg1, int arg2);
 *
 *  TEMPLATE_CALL_FN(my_func, PAREN(3, 7));
 */
#define TEMPLATE_CALL_FN(fn, fn_args)           \
        fn fn_args

#endif /* _TEMPLATE_H_ */
