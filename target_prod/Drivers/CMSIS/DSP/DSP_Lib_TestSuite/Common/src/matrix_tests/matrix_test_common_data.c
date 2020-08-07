#include "arm_math.h"
#include "matrix_test_data.h"
#include "type_abbrev.h"

/*--------------------------------------------------------------------------------*/
/* Input/Output Buffers */
/*--------------------------------------------------------------------------------*/

MATRIX_TEST_BIGGEST_INPUT_TYPE matrix_output_fut_data[2*MATRIX_TEST_MAX_ELTS] = {0};
MATRIX_TEST_BIGGEST_INPUT_TYPE matrix_output_ref_data[2*MATRIX_TEST_MAX_ELTS] = {0};
MATRIX_TEST_BIGGEST_INPUT_TYPE matrix_output_scratch[MATRIX_TEST_MAX_ELTS] = {0};

MATRIX_TEST_BIGGEST_INPUT_TYPE matrix_output_f32_fut[MATRIX_TEST_MAX_ELTS];
MATRIX_TEST_BIGGEST_INPUT_TYPE matrix_output_f32_ref[MATRIX_TEST_MAX_ELTS];

arm_matrix_instance_f32 matrix_output_fut = {
    0,
    0,
    (float32_t *) &matrix_output_fut_data
};

arm_matrix_instance_f32 matrix_output_ref = {
    0,
    0,
    (float32_t *) &matrix_output_ref_data
};

arm_matrix_instance_f64 matrix_output_fut64 = {
    0,
    0,
    (float64_t *) &matrix_output_fut_data
};

arm_matrix_instance_f64 matrix_output_ref64 = {
    0,
    0,
    (float64_t *) &matrix_output_ref_data
};

/*--------------------------------------------------------------------------------*/
/* Data Buckets */
/*--------------------------------------------------------------------------------*/

/**
 *  Pool of random data to base matrix inputs from.
 */
float32_t matrix_f32_100_rand[100] = {
    -45.0345569674258,  -11.0261163038747, -14.6841428777929,
    0.0345569674258,    -11.0261163038747, -14.6841428777929,
    -20.3679194392227,  27.5712678608402,  -12.1390617339732,
    -19.8753669720509,  42.3379642103244,  -23.7788252219155,
    -23.7517765301667,  40.2716109915281,  -25.8308714086167,
    32.1194040197959,   24.4692807074156,  -1.32083675968276,
    31.1580458282477,   -2.90766514824093, -6.97926086704160,
    10.2843089382083,   30.1014622769739,  44.4787189721646,
    -9.60878544118853,  -48.4596562348445, -31.1044984967456,
    -6.41414114190809,  3.28255887994549,  -26.9511839788442,
    -31.5183679875864,  21.1215780433683,  -47.0779722437854,
    -0.913590753192006, -40.3545474831611, -45.6976198342192,
    18.6775433365315,   -5.32162505701938, -14.9272896423117,
    34.4308792695389,   40.4880968679893,  -27.8253265982760,
    42.8854139478045,   -1.07473615999811, -36.8026707393665,
    -33.1009970537296,  -31.6488844262730, -19.3650527983443,
    43.9001561999887,   -30.5235710432951, 47.9748378356085,
    -38.2582349144194,  23.0330862855453,  -16.2280590178623,
    44.2050590775485,   14.9115474956452,  -13.1515403509664,
    0.850865538112700,  37.5942811492984,  -27.4078219027601,
    -6.11300268738968,  -20.3324126781673, -1.13910261964209,
    40.0053846417662,   45.6134540229802,  23.1722385658670,
    12.5618560729690,   1.07715641721097,  5.01563428984222,
    -32.9291952852141,  -38.8880776559401, -18.1221698074118,
    7.85250610234389,   -13.0753218879785, 7.52085950784656,
    14.7745963136307,   28.0227435151377,  31.7627708322262,
    12.2475086001227,   -27.2335702183447, -24.1935304087933,
    -7.58332402861928,  -26.2716420228479, -38.8797244706213,
    -44.0220457052844,  -4.90762935690551, -41.8874231134215,
    29.4831416883453,   8.70447045314168,  -6.43013158961009,
    -9.12801538874479,  0.785828466111815, -4.11511718200689,
    28.0252068321138,   -26.5220086627594, 4.70088922863450,
    42.9385970968730,   14.4318130193692,  -29.2257707266972,
    46.3088539286913
};

float64_t matrix_f64_100_rand[100] = {
    -45.0345569674258,  -11.0261163038747, -14.6841428777929,
    0.0345569674258,    -11.0261163038747, -14.6841428777929,
    -20.3679194392227,  27.5712678608402,  -12.1390617339732,
    -19.8753669720509,  42.3379642103244,  -23.7788252219155,
    -23.7517765301667,  40.2716109915281,  -25.8308714086167,
    32.1194040197959,   24.4692807074156,  -1.32083675968276,
    31.1580458282477,   -2.90766514824093, -6.97926086704160,
    10.2843089382083,   30.1014622769739,  44.4787189721646,
    -9.60878544118853,  -48.4596562348445, -31.1044984967456,
    -6.41414114190809,  3.28255887994549,  -26.9511839788442,
    -31.5183679875864,  21.1215780433683,  -47.0779722437854,
    -0.913590753192006, -40.3545474831611, -45.6976198342192,
    18.6775433365315,   -5.32162505701938, -14.9272896423117,
    34.4308792695389,   40.4880968679893,  -27.8253265982760,
    42.8854139478045,   -1.07473615999811, -36.8026707393665,
    -33.1009970537296,  -31.6488844262730, -19.3650527983443,
    43.9001561999887,   -30.5235710432951, 47.9748378356085,
    -38.2582349144194,  23.0330862855453,  -16.2280590178623,
    44.2050590775485,   14.9115474956452,  -13.1515403509664,
    0.850865538112700,  37.5942811492984,  -27.4078219027601,
    -6.11300268738968,  -20.3324126781673, -1.13910261964209,
    40.0053846417662,   45.6134540229802,  23.1722385658670,
    12.5618560729690,   1.07715641721097,  5.01563428984222,
    -32.9291952852141,  -38.8880776559401, -18.1221698074118,
    7.85250610234389,   -13.0753218879785, 7.52085950784656,
    14.7745963136307,   28.0227435151377,  31.7627708322262,
    12.2475086001227,   -27.2335702183447, -24.1935304087933,
    -7.58332402861928,  -26.2716420228479, -38.8797244706213,
    -44.0220457052844,  -4.90762935690551, -41.8874231134215,
    29.4831416883453,   8.70447045314168,  -6.43013158961009,
    -9.12801538874479,  0.785828466111815, -4.11511718200689,
    28.0252068321138,   -26.5220086627594, 4.70088922863450,
    42.9385970968730,   14.4318130193692,  -29.2257707266972,
    46.3088539286913
};

MATRIX_TEST_BIGGEST_INPUT_TYPE matrix_zeros[MATRIX_TEST_MAX_ELTS] = {0};

const float32_t matrix_f32_scale_values[MATRIX_MAX_COEFFS_LEN] =
{
        43.0264275639  , -17.0525215570 , -94.8488973910 , -8.1924989580  ,
        7.2830326091   , 66.8368719314  , 33.9778190671  , 117.8652289772 ,
        -129.6077797465, -14.6420815368 , 18.0239223278  , 1.0000000000   ,
        55.0375037651  , 1.8674609862   , 0.00000000000  , -33.5750364909
};

const q31_t matrix_q31_scale_values[MATRIX_MAX_COEFFS_LEN] =
{
    0x0201DC90, 0x211F0D7C, 0x80000000, 0xF573B824,
    0xE85ED05B, 0x311DFB52, 0x3529E750, 0x00000000,
    0x7FFFFFFF, 0x21FA525A, 0x0971FD45, 0x05547B68,
    0x270C6366, 0x06FDD5A6, 0xF7025315, 0xB1155A1E
};

const q15_t matrix_q15_scale_values[MATRIX_MAX_COEFFS_LEN] =
{
    0x0201, 0x211F, 0x8000, 0xF573,
    0xE85E, 0x311D, 0x3529, 0x0000,
    0x7FFF, 0x21FA, 0x0971, 0x0554,
    0x270C, 0x06FD, 0xF702, 0xB115
};

const int32_t matrix_shift_values[MATRIX_MAX_SHIFTS_LEN] =
{
    -16, -7, 0, 7, 16
};

/*--------------------------------------------------------------------------------*/
/* Matrix Definitions */
/*--------------------------------------------------------------------------------*/

/**
 *  Define matrices by suffix (f32, q31, q15) for use in test cases.
 *
 *  The rand1 and rand2 suffixes get their data from the same pool of random
 *  data, but their starting points differ by 1 element.
 *
 *  Makes available:
 *  - matrix_`suffix`_1x1_rand1/2
 *  - matrix_`suffix`_1x4_rand1/2
 *  - matrix_`suffix`_2x4_rand1/2
 *  - matrix_`suffix`_4x4_rand1/2
 */
#define MATRIX_DEFINE_MATRICES(suffix)                              \
    arm_matrix_instance_##suffix matrix_##suffix##_1x1_rand1 =      \
        {1, 1, (TYPE_FROM_ABBREV(suffix) *) matrix_f32_100_rand };  \
    arm_matrix_instance_##suffix matrix_##suffix##_1x1_rand2 =      \
        {1, 1, (TYPE_FROM_ABBREV(suffix) *) (matrix_f32_100_rand+1)}; \
    arm_matrix_instance_##suffix matrix_##suffix##_1x1_zeros =      \
        {1, 1, (TYPE_FROM_ABBREV(suffix) *) matrix_zeros};          \
                                                                    \
    arm_matrix_instance_##suffix matrix_##suffix##_1x4_rand1 =      \
        {1, 4, (TYPE_FROM_ABBREV(suffix) *) matrix_f32_100_rand };  \
    arm_matrix_instance_##suffix matrix_##suffix##_1x4_rand2 =      \
        {1, 4, (TYPE_FROM_ABBREV(suffix) *) (matrix_f32_100_rand+1)}; \
    arm_matrix_instance_##suffix matrix_##suffix##_1x4_zeros =      \
        {1, 4, (TYPE_FROM_ABBREV(suffix) *) matrix_zeros};          \
                                                                    \
    arm_matrix_instance_##suffix matrix_##suffix##_2x4_rand1 =      \
        {2, 4, (TYPE_FROM_ABBREV(suffix) *) matrix_f32_100_rand };  \
    arm_matrix_instance_##suffix matrix_##suffix##_2x4_rand2 =      \
        {2, 4, (TYPE_FROM_ABBREV(suffix) *) (matrix_f32_100_rand+1)}; \
    arm_matrix_instance_##suffix matrix_##suffix##_2x4_zeros =      \
        {2, 4, (TYPE_FROM_ABBREV(suffix) *) matrix_zeros};          \
                                                                    \
    arm_matrix_instance_##suffix matrix_##suffix##_4x4_rand1 =      \
        {4, 4, (TYPE_FROM_ABBREV(suffix) *) matrix_f32_100_rand };  \
    arm_matrix_instance_##suffix matrix_##suffix##_4x4_rand2 =      \
        {4, 4, (TYPE_FROM_ABBREV(suffix) *) (matrix_f32_100_rand+1)}; \
    arm_matrix_instance_##suffix matrix_##suffix##_4x4_zeros =      \
        {4, 4, (TYPE_FROM_ABBREV(suffix) *) matrix_zeros}

MATRIX_DEFINE_MATRICES(f64);
MATRIX_DEFINE_MATRICES(f32);
MATRIX_DEFINE_MATRICES(q31);
MATRIX_DEFINE_MATRICES(q15);

/*--------------------------------------------------------------------------------*/
/* Matrix-Input Arrays */
/*--------------------------------------------------------------------------------*/

/* Define Input #ARR_DESC_t by suffix.
 *
 * Taking inputs in parallel from the 'a' and 'b' arrays yields the following
 * test cases:
 * - 1x1 multiplication by zero
 * - 1x1 multiplication between random numbers
 * - 1x1 * 1x4 valid dimension interaction
 * - 1x1 * 2x4 invalid dimension interaction
 * - 2x4 * 4x4 larger valid dimension interaction
 * - 4x4 * 4x4 larger valid dimension interaction
 */
#define MATRIX_DEFINE_INPUTS(suffix)                        \
    ARR_DESC_DEFINE(arm_matrix_instance_##suffix *,         \
                    matrix_##suffix##_a_inputs,             \
                    6,                                      \
                    CURLY(                                  \
                        &matrix_##suffix##_1x1_rand1,       \
                        &matrix_##suffix##_1x1_rand1,       \
                        &matrix_##suffix##_1x1_rand1,       \
                        &matrix_##suffix##_1x1_rand1,       \
                        &matrix_##suffix##_2x4_rand1,       \
                        &matrix_##suffix##_4x4_rand1        \
                        ));                                 \
                                                            \
    ARR_DESC_DEFINE(arm_matrix_instance_##suffix *,         \
                    matrix_##suffix##_b_inputs,             \
                    6,                                      \
                    CURLY(                                  \
                        &matrix_##suffix##_1x1_zeros,       \
                        &matrix_##suffix##_1x1_rand2,       \
                        &matrix_##suffix##_1x4_rand2,       \
                        &matrix_##suffix##_2x4_rand2,       \
                        &matrix_##suffix##_4x4_rand2,       \
                        &matrix_##suffix##_4x4_rand2        \
                        ));                                 \
                                                            \
    ARR_DESC_DEFINE(arm_matrix_instance_##suffix *,         \
                    matrix_##suffix##_invertible_inputs,    \
                    4,                                      \
                    CURLY(                                  \
                        &matrix_##suffix##_1x1_rand1,       \
                        &matrix_##suffix##_1x1_rand2,       \
                        &matrix_##suffix##_4x4_rand1,       \
                        &matrix_##suffix##_4x4_rand2        \
                        ))                                  \

MATRIX_DEFINE_INPUTS(f64);
MATRIX_DEFINE_INPUTS(f32);
MATRIX_DEFINE_INPUTS(q31);
MATRIX_DEFINE_INPUTS(q15);
