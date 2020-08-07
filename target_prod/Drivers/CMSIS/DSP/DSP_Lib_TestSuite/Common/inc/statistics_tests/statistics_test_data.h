#ifndef _STATISTICS_TEST_DATA_H_
#define _STATISTICS_TEST_DATA_H_

/*--------------------------------------------------------------------------------*/
/* Includes */
/*--------------------------------------------------------------------------------*/

#include "arr_desc.h"
#include "arm_math.h"

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/
#define STATISTICS_MAX_INPUT_ELEMENTS 32
#define STATISTICS_BIGGEST_INPUT_TYPE float32_t

/*--------------------------------------------------------------------------------*/
/* Declare Variables */
/*--------------------------------------------------------------------------------*/

/* Input/Output Buffers */
ARR_DESC_DECLARE(statistics_output_fut);
ARR_DESC_DECLARE(statistics_output_ref);
extern uint32_t statistics_idx_fut;
extern uint32_t statistics_idx_ref;

extern STATISTICS_BIGGEST_INPUT_TYPE
statistics_output_f32_ref[STATISTICS_MAX_INPUT_ELEMENTS];

extern STATISTICS_BIGGEST_INPUT_TYPE
statistics_output_f32_fut[STATISTICS_MAX_INPUT_ELEMENTS];


/* Block Sizes */
ARR_DESC_DECLARE(statistics_block_sizes);

/* Float Inputs */
ARR_DESC_DECLARE(statistics_zeros);
ARR_DESC_DECLARE(statistics_f_2);
ARR_DESC_DECLARE(statistics_f_15);
ARR_DESC_DECLARE(statistics_f_32);
ARR_DESC_DECLARE(statistics_f_all);

#endif /* _STATISTICS_TEST_DATA_H_ */
