#include "arm_math.h"
#include "support_test_data.h"

#define MAX_INPUT_ELEMENTS 32
#define BIGGEST_INPUT_TYPE float32_t

/*--------------------------------------------------------------------------------*/
/* Input/Output Buffers */
/*--------------------------------------------------------------------------------*/

ARR_DESC_DEFINE(BIGGEST_INPUT_TYPE,
                support_output_fut,
                MAX_INPUT_ELEMENTS,
                CURLY(0));

ARR_DESC_DEFINE(BIGGEST_INPUT_TYPE,
                support_output_ref,
                MAX_INPUT_ELEMENTS,
                CURLY(0));

/*--------------------------------------------------------------------------------*/
/* Block Sizes */
/*--------------------------------------------------------------------------------*/

/* 
  To change test parameter values add/remove values inside CURLY and update 
  the preceeding parameter to reflect the number of values inside CURLY. 
*/

ARR_DESC_DEFINE(uint32_t,
                support_block_sizes,
                4,
                CURLY( 2, 7, 15, 32));

/*--------------------------------------------------------------------------------*/
/* Numbers */
/*--------------------------------------------------------------------------------*/

ARR_DESC_DEFINE(uint32_t,
                support_elts,
                4,
                CURLY( 0, 1, 0x80000000, 0x7fffffff));

/*--------------------------------------------------------------------------------*/
/* Test Data */
/*--------------------------------------------------------------------------------*/

ARR_DESC_DEFINE(float32_t,
                support_f_32,
                32,
                CURLY(
                      0.24865986 , -0.13364227, -0.27233250 , -7.33488200,
                      0.42190653 , 1.17435880 , -0.49824914 , 0.87883663,
                      0.63066370 , 1.80275680 , -84.83916000, -2.06773800,
                      7.63452500 , 1.01487610 , -0.65785825 , 1.78019030,
                      -0.34160388, 0.68546050 , -1.81721590 , -0.10340453,
                      -4.48600340, -1.69763480, -1.26022340 , -1.58457480,
                      0.51993870 , 2.83526470 , -0.21502694 , -0.57690346,
                      -0.22945681, 0.79509383 , 0.07275216  , -2.16279080
                      ));

/* Alias the 32 element array with wrappers that end sooner. */
ARR_DESC_DEFINE_SUBSET(support_f_15,
                       support_f_32,
                       15);

ARR_DESC_DEFINE_SUBSET(support_f_2,
                       support_f_32,
                       2);

ARR_DESC_DEFINE(float32_t,
                support_zeros,
                32,
                CURLY(0));

/* Aggregate all float datasets. */
ARR_DESC_DEFINE(ARR_DESC_t *,
                support_f_all,
                4,
                CURLY(
                      &support_zeros,
                      &support_f_2,
                      &support_f_15,
                      &support_f_32
                      ));
