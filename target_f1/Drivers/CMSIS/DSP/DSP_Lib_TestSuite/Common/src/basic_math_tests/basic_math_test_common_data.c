#include "basic_math_test_data.h"

/*--------------------------------------------------------------------------------*/
/* Input/Output Buffers */
/*--------------------------------------------------------------------------------*/

ARR_DESC_DEFINE(BASIC_MATH_BIGGEST_INPUT_TYPE,
                basic_math_output_fut,
                BASIC_MATH_MAX_INPUT_ELEMENTS,
                CURLY(0));

ARR_DESC_DEFINE(BASIC_MATH_BIGGEST_INPUT_TYPE,
                basic_math_output_ref,
                BASIC_MATH_MAX_INPUT_ELEMENTS,
                CURLY(0));

BASIC_MATH_BIGGEST_INPUT_TYPE
basic_math_output_f32_ref[BASIC_MATH_MAX_INPUT_ELEMENTS];

BASIC_MATH_BIGGEST_INPUT_TYPE
basic_math_output_f32_fut[BASIC_MATH_MAX_INPUT_ELEMENTS];

/*--------------------------------------------------------------------------------*/
/* Block Sizes */
/*--------------------------------------------------------------------------------*/

/* 
  To change test parameter values add/remove values inside CURLY and update 
  the preceeding parameter to reflect the number of values inside CURLY. 
*/

ARR_DESC_DEFINE(uint32_t,
                basic_math_block_sizes,
                4,
                CURLY( 2, 7, 15, 32));

/*--------------------------------------------------------------------------------*/
/* Numbers */
/*--------------------------------------------------------------------------------*/

/* 
  To change test parameter values add/remove values inside CURLY and update 
  the preceeding parameter to reflect the number of values inside CURLY. 
*/

ARR_DESC_DEFINE(uint32_t,
                basic_math_elts,
                4,
                CURLY( 0, 1, 0x80000000, 0x7fffffff));

ARR_DESC_DEFINE(int8_t,
                basic_math_elts2,
                5,
                CURLY( 0, 3, -3, -7, 7));

ARR_DESC_DEFINE(float32_t,
                basic_math_eltsf,
                6,
                CURLY( 0.0f, 1.0f, 1.254001, -1.665584, -127.435646, 245.34634267));

/*--------------------------------------------------------------------------------*/
/* Test Data */
/*--------------------------------------------------------------------------------*/

ARR_DESC_DEFINE(float32_t,
                basic_math_f_32,
                32,
                CURLY(
                      -0.432565, -1.665584,  0.125332,  0.287676, -1.146471,
                       1.190915,  1.189164, -0.037633,  0.327292,  0.174639,
                      -0.186709,  0.725791, -0.588317,  2.183186, -0.136396,
                       0.113931,  1.066768,  0.059281, -0.095648, -0.832349,
                       0.294411, -1.336182,  0.714325,  1.623562, -0.691776,
                       0.857997,  1.254001, -1.593730, -1.440964,  0.571148,
                      -0.399886,  0.689997
                      ));

/* Alias the 32 element array with wrappers that end sooner. */
ARR_DESC_DEFINE_SUBSET(basic_math_f_15,
                       basic_math_f_32,
                       15);

ARR_DESC_DEFINE_SUBSET(basic_math_f_2,
                       basic_math_f_32,
                       2);

ARR_DESC_DEFINE(float32_t,
                basic_math_zeros,
                32,
                CURLY(0));

/* Aggregate all float datasets. */
ARR_DESC_DEFINE(ARR_DESC_t *,
                basic_math_f_all,
                4,
                CURLY(
                      &basic_math_zeros,
                      &basic_math_f_2,
                      &basic_math_f_15,
                      &basic_math_f_32
                      ));
