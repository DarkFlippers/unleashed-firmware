#ifndef _ARR_DESC_H_
#define _ARR_DESC_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>             /* memset() */
#include "../util/util.h"       /* CONCAT() */

/*--------------------------------------------------------------------------------*/
/* Type Definitions */
/*--------------------------------------------------------------------------------*/

/**
 *  Array-descriptor struct.
 */
typedef struct ARR_DESC_struct
{
    void *  data_ptr;                /* Pointer to the array contents. */
    int32_t element_count;           /* Number of current elements. */
    int32_t element_size;            /* Size of current elements in bytes. */
    int32_t underlying_size;         /* Size of underlying array in bytes. */
} ARR_DESC_t;

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Prefix of the array variable's name when creating an array and an array
 *  descriptor at the same time.
 */
#define ARR_DESC_ARR_PREFIX ARR_DESC_ARR_

/**
 *  Evaluate to the array variable's name when creating an array and an array
 *  descriptor at the same time.
 */
#define ARR_DESC_ARR_NAME(name)                 \
    CONCAT(ARR_DESC_ARR_PREFIX, name)

/**
 *  Define an #ARR_DESC_t by itself.
 *
 *  @note The user must supply an array to store the data used by the
 *  #ARR_DESC_t.
 */
#define ARR_DESC_INTERNAL_DEFINE(name, data_ptr,                \
                                 element_count, element_size)   \
    ARR_DESC_t name = {                                         \
        data_ptr,                                               \
        element_count,                                          \
        element_size,                                           \
        element_count * element_size                            \
    }                                                           \

/**
 *  Define both an array and an #ARR_DESC_t that describes it.
 *
 *  @note Use the #CURLY() macro for the content field; it provides the curly
 *  braces necessary for an array initialization.
 */
#define ARR_DESC_DEFINE(type, name, element_count, content)             \
    type ARR_DESC_ARR_NAME(name)[element_count] = content;              \
    ARR_DESC_INTERNAL_DEFINE(name,                                      \
                             &ARR_DESC_ARR_NAME(name),                  \
                             element_count,                             \
                             sizeof(type)) /* Note the lacking semicolon */

/**
 *  Create a #ARR_DESC_t which refers to a subset of the data in another.
 *
 *  The new #ARR_DESC_t shares the same underlying array as the aliased
 *  #ARR_DESC_t, but only describes a subset of the originals values.
 */
#define ARR_DESC_DEFINE_SUBSET(name, original, element_cnt)         \
    ARR_DESC_INTERNAL_DEFINE(name,                                  \
                             &ARR_DESC_ARR_NAME(original),          \
                             element_cnt,                           \
                             sizeof(ARR_DESC_ARR_NAME(original)[0]) \
        ) /* Note the lacking semicolon */

/**
 *  Creat an #ARR_DESC_t which points to the data in an existing array.
 *
 *  @param start_idx Offset in array_ptr of first element.
 *  @param element_cnt Number of elements to include in the #ARR_DESC_t.
 *
 *  @example
 *
 *  float my_floats[4] = {0.0f, 1.0f, 2.0f, 3.0f};
 *
 *  ARR_DESC_DEFINE_USING_ARR(my_arr_desc, my_floats, 1, 3);
 *
 *  printf("Element 0: %f\n", ARR_DESC_ELT(float, 0, &my_arr_desc));
 *  printf("Element 1: %f\n", ARR_DESC_ELT(float, 1, &my_arr_desc));
 *
 *  Outputs:
 *
 *  Element 0: 1.000000
 *  Element 1: 2.000000
 *
 *  @warning There are no checks in place to catch invalid start indices; This
 *  is left to the user.
 */
#define ARR_DESC_DEFINE_USING_ARR(type, name, array_ptr, start_idx, element_cnt) \
    ARR_DESC_INTERNAL_DEFINE(                                           \
        name,                                                           \
        (type *) (array_ptr + start_idx),                               \
        element_cnt,                                                    \
        sizeof(type)                                                    \
        ) /* Note the lacking semicolon*/

/**
 *  Declare an #ARR_DESC_t object.
 */
#define ARR_DESC_DECLARE(name)                              \
    extern ARR_DESC_t name /* Note the lacking semicolon */

/**
 *  Evaluate to the number of bytes stored in the #ARR_DESC_t.
 */
#define ARR_DESC_BYTES(arr_desc_ptr)                                \
    ((arr_desc_ptr)->element_count * (arr_desc_ptr)->element_size)

/**
 *  Set the contents of #ARR_DESC_t to value.
 */
#define ARR_DESC_MEMSET(arr_desc_ptr, value, bytes)     \
    do                                                  \
    {                                                   \
        memset((arr_desc_ptr)->data_ptr,                \
               value,                                   \
               BOUND(0,                                 \
                     (arr_desc_ptr)->underlying_size,   \
                     bytes)                             \
            );                                          \
    } while (0)

/**
 *  Perform a memcpy of 'bytes' bytes from the source #ARR_DESC_t to the
 *  destination #ARR_DESC_t.
 */
#define ARR_DESC_MEMCPY(arr_desc_dest_ptr, arr_desc_src_ptr, bytes) \
    do                                                              \
    {                                                               \
        memcpy((arr_desc_dest_ptr)->data_ptr,                       \
               (arr_desc_src_ptr)->data_ptr,                        \
               BOUND(0,                                             \
                     (arr_desc_dest_ptr)->underlying_size,          \
                     bytes));                                       \
    } while (0)

/**
 *  Evaluate to true if the source #ARR_DESC_t contents will fit into the
 *  destination #ARR_DESC_t and false otherwise.
 */
#define ARR_DESC_COPYABLE(arr_desc_dest_ptr, arr_desc_src_ptr)  \
      (ARR_DESC_BYTES(arr_desc_src_ptr) <=                      \
       (arr_desc_dest_ptr)->underlying_size)

/**
 *  Copy all the data from the source #ARR_DESC_t to the destination
 *  #ARR_DESC_t.
 *
 *  @note If the destination #ARR_DESC_t is too small to fit the source data the
 *  copy is aborted and nothing happens.
 */
#define ARR_DESC_COPY(arr_desc_dest_ptr, arr_desc_src_ptr)      \
    do                                                          \
    {                                                           \
        if (ARR_DESC_COPYABLE(arr_desc_dest_ptr,                 \
                             arr_desc_src_ptr))                 \
        {                                                       \
            ARR_DESC_MEMCPY(arr_desc_dest_ptr,                  \
                            arr_desc_src_ptr,                   \
                            ARR_DESC_BYTES(arr_desc_src_ptr));  \
            /* Update the properties*/                          \
            (arr_desc_dest_ptr)->element_count =                \
                (arr_desc_src_ptr)->element_count;              \
            (arr_desc_dest_ptr)->element_size =                 \
                (arr_desc_src_ptr)->element_size;               \
        }                                                       \
    } while (0)

/**
 *  Compare the data in two #ARR_DESC_t structs for the specified number of
 *  bytes.
 */
#define ARR_DESC_MEMCMP(arr_desc_ptr_a, arr_desc_ptr_b, bytes)  \
        memcmp((arr_desc_ptr_a)->data_ptr,                      \
            (arr_desc_ptr_b)->data_ptr,                         \
               bytes) /* Note the lacking semicolon */          \

/**
 *  Zero out the contents of the #ARR_DESC_t.
 */
#define ARR_DESC_ZERO(arr_desc_ptr)             \
        ARR_DESC_MEMSET(arr_desc_ptr,           \
                        0,                      \
                        (arr_desc_ptr)->underlying_size)

/**
 *  Evaluate to the data address in #ARR_DESC_t at offset.
 */
#define ARR_DESC_DATA_ADDR(type, arr_desc_ptr, offset)  \
        ((void*)(((type *)                              \
                  ((arr_desc_ptr)->data_ptr))           \
                 + offset))

/**
 *  Evaluate to the element in #ARR_DESC_t with type at idx.
 */
#define ARR_DESC_ELT(type, idx, arr_desc_ptr)           \
        (*((type *) ARR_DESC_DATA_ADDR(type,            \
                                       arr_desc_ptr,    \
                                       idx)))

#endif /* _ARR_DESC_H_ */
