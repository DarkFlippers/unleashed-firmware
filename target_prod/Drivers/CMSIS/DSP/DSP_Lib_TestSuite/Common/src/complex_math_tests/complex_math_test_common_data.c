#include "complex_math_test_data.h"

/*--------------------------------------------------------------------------------*/
/* Input/Output Buffers */
/*--------------------------------------------------------------------------------*/

ARR_DESC_DEFINE(COMPLEX_MATH_BIGGEST_INPUT_TYPE,
                complex_math_output_fut_a,
                COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2 /*Complex data has two parts*/,
                CURLY(0));

ARR_DESC_DEFINE(COMPLEX_MATH_BIGGEST_INPUT_TYPE,
                complex_math_output_fut_b,
                COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2 /*Complex data has two parts*/,
                CURLY(0));

ARR_DESC_DEFINE(COMPLEX_MATH_BIGGEST_INPUT_TYPE,
                complex_math_output_ref_a,
                COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2 /*Complex data has two parts*/,
                CURLY(0));


ARR_DESC_DEFINE(COMPLEX_MATH_BIGGEST_INPUT_TYPE,
                complex_math_output_ref_b,
                COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2 /*Complex data has two parts*/,
                CURLY(0));


COMPLEX_MATH_BIGGEST_INPUT_TYPE
complex_math_output_f32_ref_a[COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2];

COMPLEX_MATH_BIGGEST_INPUT_TYPE
complex_math_output_f32_ref_b[COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2];

COMPLEX_MATH_BIGGEST_INPUT_TYPE
complex_math_output_f32_fut_a[COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2];

COMPLEX_MATH_BIGGEST_INPUT_TYPE
complex_math_output_f32_fut_b[COMPLEX_MATH_MAX_INPUT_ELEMENTS * 2];

/*--------------------------------------------------------------------------------*/
/* Block Sizes */
/*--------------------------------------------------------------------------------*/

ARR_DESC_DEFINE(uint32_t,
                complex_math_block_sizes,
                4,
                CURLY(1, 2, 15, 32));

/*--------------------------------------------------------------------------------*/
/* Test Data */
/*--------------------------------------------------------------------------------*/

ARR_DESC_DEFINE(float32_t,
                complex_math_f_32,
                32 * 2 /*Complex data has two parts*/,
                CURLY(
                      -0.432564811528220680 ,	0.815622288876143300,	
                      -1.665584378238097000 ,	0.711908323500893280,	
                      0.125332306474830680  ,	1.290249754932477000,	
                      0.287676420358548850  ,	0.668600505682040320,	
                      -1.146471350681463700 ,	1.190838074243369100,	
                      1.190915465642998800  ,	-1.202457114773944000,	
                      1.189164201652103100  ,	-0.019789557768770449,	
                      -0.037633276593317645 ,	-0.156717298831980680,	
                      0.327292361408654140  ,	-1.604085562001158500,	
                      0.174639142820924520  ,	0.257304234677489860,	
                      -0.186708577681439360 ,	-1.056472928081482400,	
                      0.725790548293302700  ,	1.415141485872338600,	
                      -0.588316543014188680 ,	-0.805090404196879830,	
                      2.183185818197101100  ,	0.528743010962224870,	
                      -0.136395883086595700 ,	0.219320672667622370,	
                      0.113931313520809620  ,	-0.921901624355539130,	
                      1.066768211359188800  ,	-2.170674494305262500,	
                      0.059281460523605348  ,	-0.059187824521191180,	
                      -0.095648405483669041 ,	-1.010633706474247400,	
                      -0.832349463650022490 ,	0.614463048895480980,	
                      0.294410816392640380  ,	0.507740785341985520,	
                      -1.336181857937804000 ,	1.692429870190521400,	
                      0.714324551818952160  ,	0.591282586924175900,	
                      1.623562064446270700  ,	-0.643595202682526120,	
                      -0.691775701702286750 ,	0.380337251713910140,	
                      0.857996672828262640  ,	-1.009115524340785000,	
                      1.254001421602532400  ,	-0.019510669530289293,	
                      -1.593729576447476800 ,	-0.048220789145312269,	
                      -1.440964431901020000 ,	0.000043191841625545,	
                      0.571147623658177950  ,	-0.317859451247687890,	
                      -0.399885577715363150 ,	1.095003738787492500,	
                      0.689997375464345140  ,	-1.873990257640960800
                      ));

ARR_DESC_DEFINE_SUBSET(complex_math_f_15,
                       complex_math_f_32,
                       15 * 2 /*Complex data has two parts*/);

ARR_DESC_DEFINE_SUBSET(complex_math_f_2,
                       complex_math_f_32,
                       2 * 2 /*Complex data has two parts*/);

ARR_DESC_DEFINE(float32_t,
                complex_math_zeros,
                32 * 2 /*Complex data has two parts*/,
                CURLY(0));

/* Aggregate all float datasets */
ARR_DESC_DEFINE(ARR_DESC_t *,
                complex_math_f_all,
                4,
                CURLY(
                      &complex_math_zeros,
                      &complex_math_f_2,
                      &complex_math_f_15,
                      &complex_math_f_32
                      ));
