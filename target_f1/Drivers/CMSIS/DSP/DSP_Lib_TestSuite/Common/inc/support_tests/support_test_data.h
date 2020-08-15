#ifndef ARM_SUPPORT_TEST_DATA_H
#define ARM_SUPPORT_TEST_DATA_H

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "arr_desc.h"

/*--------------------------------------------------------------------------------*/
/* Declare Variables */
/*--------------------------------------------------------------------------------*/

/* Input/Output Buffers */
ARR_DESC_DECLARE(support_output_fut);
ARR_DESC_DECLARE(support_output_ref);

/* Block Sizes*/
ARR_DESC_DECLARE(support_block_sizes);

/* Numbers */
ARR_DESC_DECLARE(support_elts);

/* Float Inputs */
ARR_DESC_DECLARE(support_zeros);
ARR_DESC_DECLARE(support_f_2);
ARR_DESC_DECLARE(support_f_15);
ARR_DESC_DECLARE(support_f_32);
ARR_DESC_DECLARE(support_f_all);

#endif
