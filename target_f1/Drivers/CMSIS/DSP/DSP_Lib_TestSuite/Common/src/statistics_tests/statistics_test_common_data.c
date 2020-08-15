#include "statistics_test_data.h"

/*--------------------------------------------------------------------------------*/
/* Input/Output Buffers */
/*--------------------------------------------------------------------------------*/


ARR_DESC_DEFINE(STATISTICS_BIGGEST_INPUT_TYPE,
                statistics_output_fut,
                STATISTICS_MAX_INPUT_ELEMENTS,
                CURLY(0));

ARR_DESC_DEFINE(STATISTICS_BIGGEST_INPUT_TYPE,
                statistics_output_ref,
                STATISTICS_MAX_INPUT_ELEMENTS,
                CURLY(0));

uint32_t statistics_idx_fut = 0;
uint32_t statistics_idx_ref = 0;

STATISTICS_BIGGEST_INPUT_TYPE
statistics_output_f32_ref[STATISTICS_MAX_INPUT_ELEMENTS];

STATISTICS_BIGGEST_INPUT_TYPE
statistics_output_f32_fut[STATISTICS_MAX_INPUT_ELEMENTS];

/*--------------------------------------------------------------------------------*/
/* Block Sizes */
/*--------------------------------------------------------------------------------*/

/* 
  To change test parameter values add/remove values inside CURLY and update 
  the preceeding parameter to reflect the number of values inside CURLY. 
*/

ARR_DESC_DEFINE(uint32_t,
                statistics_block_sizes,
                4,
                CURLY(1, 2, 15, 32));

/*--------------------------------------------------------------------------------*/
/* Test Data */
/*--------------------------------------------------------------------------------*/

ARR_DESC_DEFINE(float32_t,
                statistics_f_32,
                32,
                CURLY(
                    -0.0865129623056441 , -0.3331168756476194,
                    0.0250664612949661  , 0.0575352840717098,
                    -0.2292942701362928 , 0.2381830931285998,
                    0.2378328403304206  ,  -0.0075266553186635,
                    0.0654584722817308  , 0.0349278285641849,
                    -0.0373417155362879 , 0.1451581096586606,
                    -0.1176633086028378 , 0.4366371636394202,
                    -0.0272791766173191 , 0.0227862627041619,
                    0.2133536422718378  ,  0.0118562921047211,
                    -0.0191296810967338 , -0.1664698927300045,
                    0.0588821632785281  , -0.2672363715875608,
                    0.1428649103637904  ,  0.3247124128892542,
                    -0.1383551403404573 , 0.1715993345656525,
                    0.2508002843205065  , -0.3187459152894954,
                    -0.2881928863802040 , 0.1142295247316356,
                    -0.0799771155430726 , 0.1379994750928690
                    ));


ARR_DESC_DEFINE_SUBSET(statistics_f_31,
                       statistics_f_32,
                       31);

ARR_DESC_DEFINE_SUBSET(statistics_f_15,
                       statistics_f_32,
                       15);

ARR_DESC_DEFINE_SUBSET(statistics_f_2,
                       statistics_f_32,
                       2);

ARR_DESC_DEFINE(float32_t,
                statistics_zeros,
                32,
                CURLY(0));

/* Aggregate all float datasets */
ARR_DESC_DEFINE(ARR_DESC_t *,
                statistics_f_all,
                4,
                CURLY(
                    &statistics_zeros,
                    &statistics_f_2,
                    &statistics_f_15,
                    &statistics_f_32
                    ));
