/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_biquad_cascade_stereo_df2T_f32.c
 * Description:  Processing function for floating-point transposed direct form II Biquad cascade filter. 2 channels
 *
 * $Date:        27. January 2017
 * $Revision:    V.1.5.1
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2017 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "arm_math.h"

/**
* @ingroup groupFilters
*/

/**
* @defgroup BiquadCascadeDF2T Biquad Cascade IIR Filters Using a Direct Form II Transposed Structure
*
* This set of functions implements arbitrary order recursive (IIR) filters using a transposed direct form II structure.
* The filters are implemented as a cascade of second order Biquad sections.
* These functions provide a slight memory savings as compared to the direct form I Biquad filter functions.
* Only floating-point data is supported.
*
* This function operate on blocks of input and output data and each call to the function
* processes <code>blockSize</code> samples through the filter.
* <code>pSrc</code> points to the array of input data and
* <code>pDst</code> points to the array of output data.
* Both arrays contain <code>blockSize</code> values.
*
* \par Algorithm
* Each Biquad stage implements a second order filter using the difference equation:
* <pre>
*    y[n] = b0 * x[n] + d1
*    d1 = b1 * x[n] + a1 * y[n] + d2
*    d2 = b2 * x[n] + a2 * y[n]
* </pre>
* where d1 and d2 represent the two state values.
*
* \par
* A Biquad filter using a transposed Direct Form II structure is shown below.
* \image html BiquadDF2Transposed.gif "Single transposed Direct Form II Biquad"
* Coefficients <code>b0, b1, and b2 </code> multiply the input signal <code>x[n]</code> and are referred to as the feedforward coefficients.
* Coefficients <code>a1</code> and <code>a2</code> multiply the output signal <code>y[n]</code> and are referred to as the feedback coefficients.
* Pay careful attention to the sign of the feedback coefficients.
* Some design tools flip the sign of the feedback coefficients:
* <pre>
*    y[n] = b0 * x[n] + d1;
*    d1 = b1 * x[n] - a1 * y[n] + d2;
*    d2 = b2 * x[n] - a2 * y[n];
* </pre>
* In this case the feedback coefficients <code>a1</code> and <code>a2</code> must be negated when used with the CMSIS DSP Library.
*
* \par
* Higher order filters are realized as a cascade of second order sections.
* <code>numStages</code> refers to the number of second order stages used.
* For example, an 8th order filter would be realized with <code>numStages=4</code> second order stages.
* A 9th order filter would be realized with <code>numStages=5</code> second order stages with the
* coefficients for one of the stages configured as a first order filter (<code>b2=0</code> and <code>a2=0</code>).
*
* \par
* <code>pState</code> points to the state variable array.
* Each Biquad stage has 2 state variables <code>d1</code> and <code>d2</code>.
* The state variables are arranged in the <code>pState</code> array as:
* <pre>
*     {d11, d12, d21, d22, ...}
* </pre>
* where <code>d1x</code> refers to the state variables for the first Biquad and
* <code>d2x</code> refers to the state variables for the second Biquad.
* The state array has a total length of <code>2*numStages</code> values.
* The state variables are updated after each block of data is processed; the coefficients are untouched.
*
* \par
* The CMSIS library contains Biquad filters in both Direct Form I and transposed Direct Form II.
* The advantage of the Direct Form I structure is that it is numerically more robust for fixed-point data types.
* That is why the Direct Form I structure supports Q15 and Q31 data types.
* The transposed Direct Form II structure, on the other hand, requires a wide dynamic range for the state variables <code>d1</code> and <code>d2</code>.
* Because of this, the CMSIS library only has a floating-point version of the Direct Form II Biquad.
* The advantage of the Direct Form II Biquad is that it requires half the number of state variables, 2 rather than 4, per Biquad stage.
*
* \par Instance Structure
* The coefficients and state variables for a filter are stored together in an instance data structure.
* A separate instance structure must be defined for each filter.
* Coefficient arrays may be shared among several instances while state variable arrays cannot be shared.
*
* \par Init Functions
* There is also an associated initialization function.
* The initialization function performs following operations:
* - Sets the values of the internal structure fields.
* - Zeros out the values in the state buffer.
* To do this manually without calling the init function, assign the follow subfields of the instance structure:
* numStages, pCoeffs, pState. Also set all of the values in pState to zero.
*
* \par
* Use of the initialization function is optional.
* However, if the initialization function is used, then the instance structure cannot be placed into a const data section.
* To place an instance structure into a const data section, the instance structure must be manually initialized.
* Set the values in the state buffer to zeros before static initialization.
* For example, to statically initialize the instance structure use
* <pre>
*     arm_biquad_cascade_df2T_instance_f32 S1 = {numStages, pState, pCoeffs};
* </pre>
* where <code>numStages</code> is the number of Biquad stages in the filter; <code>pState</code> is the address of the state buffer.
* <code>pCoeffs</code> is the address of the coefficient buffer;
*
*/

/**
* @addtogroup BiquadCascadeDF2T
* @{
*/

/**
* @brief Processing function for the floating-point transposed direct form II Biquad cascade filter.
* @param[in]  *S        points to an instance of the filter data structure.
* @param[in]  *pSrc     points to the block of input data.
* @param[out] *pDst     points to the block of output data
* @param[in]  blockSize number of samples to process.
* @return none.
*/


LOW_OPTIMIZATION_ENTER
void arm_biquad_cascade_stereo_df2T_f32(
const arm_biquad_cascade_stereo_df2T_instance_f32 * S,
float32_t * pSrc,
float32_t * pDst,
uint32_t blockSize)
{

    float32_t *pIn = pSrc;                         /*  source pointer            */
    float32_t *pOut = pDst;                        /*  destination pointer       */
    float32_t *pState = S->pState;                 /*  State pointer             */
    float32_t *pCoeffs = S->pCoeffs;               /*  coefficient pointer       */
    float32_t acc1a, acc1b;                        /*  accumulator               */
    float32_t b0, b1, b2, a1, a2;                  /*  Filter coefficients       */
    float32_t Xn1a, Xn1b;                          /*  temporary input           */
    float32_t d1a, d2a, d1b, d2b;                  /*  state variables           */
    uint32_t sample, stage = S->numStages;         /*  loop counters             */

#if defined(ARM_MATH_CM7)

    float32_t Xn2a, Xn3a, Xn4a, Xn5a, Xn6a, Xn7a, Xn8a;         /*  Input State variables     */
    float32_t Xn2b, Xn3b, Xn4b, Xn5b, Xn6b, Xn7b, Xn8b;         /*  Input State variables     */
    float32_t acc2a, acc3a, acc4a, acc5a, acc6a, acc7a, acc8a;  /*  Simulates the accumulator */
    float32_t acc2b, acc3b, acc4b, acc5b, acc6b, acc7b, acc8b;  /*  Simulates the accumulator */

    do
    {
        /* Reading the coefficients */
        b0 = pCoeffs[0];
        b1 = pCoeffs[1];
        b2 = pCoeffs[2];
        a1 = pCoeffs[3];
        /* Apply loop unrolling and compute 8 output values simultaneously. */
        sample = blockSize >> 3U;
        a2 = pCoeffs[4];

        /*Reading the state values */
        d1a = pState[0];
        d2a = pState[1];
        d1b = pState[2];
        d2b = pState[3];

        pCoeffs += 5U;

        /* First part of the processing with loop unrolling.  Compute 8 outputs at a time.
        ** a second loop below computes the remaining 1 to 7 samples. */
        while (sample > 0U) {

            /* y[n] = b0 * x[n] + d1 */
            /* d1 = b1 * x[n] + a1 * y[n] + d2 */
            /* d2 = b2 * x[n] + a2 * y[n] */

            /* Read the first 2 inputs. 2 cycles */
            Xn1a  = pIn[0 ];
            Xn1b  = pIn[1 ];

            /* Sample 1. 5 cycles */
            Xn2a  = pIn[2 ];
            acc1a = b0 * Xn1a + d1a;

            Xn2b  = pIn[3 ];
            d1a = b1 * Xn1a + d2a;

            Xn3a  = pIn[4 ];
            d2a = b2 * Xn1a;

            Xn3b  = pIn[5 ];
            d1a += a1 * acc1a;

            Xn4a  = pIn[6 ];
            d2a += a2 * acc1a;

            /* Sample 2. 5 cycles */
            Xn4b  = pIn[7 ];
            acc1b = b0 * Xn1b + d1b;

            Xn5a  = pIn[8 ];
            d1b = b1 * Xn1b + d2b;

            Xn5b = pIn[9 ];
            d2b = b2 * Xn1b;

            Xn6a = pIn[10];
            d1b += a1 * acc1b;

            Xn6b = pIn[11];
            d2b += a2 * acc1b;

            /* Sample 3. 5 cycles */
            Xn7a = pIn[12];
            acc2a = b0 * Xn2a + d1a;

            Xn7b = pIn[13];
            d1a = b1 * Xn2a + d2a;

            Xn8a = pIn[14];
            d2a = b2 * Xn2a;

            Xn8b = pIn[15];
            d1a += a1 * acc2a;

            pIn += 16;
            d2a += a2 * acc2a;

            /* Sample 4. 5 cycles */
            acc2b = b0 * Xn2b + d1b;
            d1b = b1 * Xn2b + d2b;
            d2b = b2 * Xn2b;
            d1b += a1 * acc2b;
            d2b += a2 * acc2b;

            /* Sample 5. 5 cycles */
            acc3a = b0 * Xn3a + d1a;
            d1a = b1 * Xn3a + d2a;
            d2a = b2 * Xn3a;
            d1a += a1 * acc3a;
            d2a += a2 * acc3a;

            /* Sample 6. 5 cycles */
            acc3b = b0 * Xn3b + d1b;
            d1b = b1 * Xn3b + d2b;
            d2b = b2 * Xn3b;
            d1b += a1 * acc3b;
            d2b += a2 * acc3b;

            /* Sample 7. 5 cycles */
            acc4a = b0 * Xn4a + d1a;
            d1a = b1 * Xn4a + d2a;
            d2a = b2 * Xn4a;
            d1a += a1 * acc4a;
            d2a += a2 * acc4a;

            /* Sample 8. 5 cycles */
            acc4b = b0 * Xn4b + d1b;
            d1b = b1 * Xn4b + d2b;
            d2b = b2 * Xn4b;
            d1b += a1 * acc4b;
            d2b += a2 * acc4b;

            /* Sample 9. 5 cycles */
            acc5a = b0 * Xn5a + d1a;
            d1a = b1 * Xn5a + d2a;
            d2a = b2 * Xn5a;
            d1a += a1 * acc5a;
            d2a += a2 * acc5a;

            /* Sample 10. 5 cycles */
            acc5b = b0 * Xn5b + d1b;
            d1b = b1 * Xn5b + d2b;
            d2b = b2 * Xn5b;
            d1b += a1 * acc5b;
            d2b += a2 * acc5b;

            /* Sample 11. 5 cycles */
            acc6a = b0 * Xn6a + d1a;
            d1a = b1 * Xn6a + d2a;
            d2a = b2 * Xn6a;
            d1a += a1 * acc6a;
            d2a += a2 * acc6a;

            /* Sample 12. 5 cycles */
            acc6b = b0 * Xn6b + d1b;
            d1b = b1 * Xn6b + d2b;
            d2b = b2 * Xn6b;
            d1b += a1 * acc6b;
            d2b += a2 * acc6b;

            /* Sample 13. 5 cycles */
            acc7a = b0 * Xn7a + d1a;
            d1a = b1 * Xn7a + d2a;

            pOut[0 ] = acc1a ;
            d2a = b2 * Xn7a;

            pOut[1 ] = acc1b ;
            d1a += a1 * acc7a;

            pOut[2 ] = acc2a ;
            d2a += a2 * acc7a;

            /* Sample 14. 5 cycles */
            pOut[3 ] = acc2b ;
            acc7b = b0 * Xn7b + d1b;

            pOut[4 ] = acc3a ;
            d1b = b1 * Xn7b + d2b;

            pOut[5 ] = acc3b ;
            d2b = b2 * Xn7b;

            pOut[6 ] = acc4a ;
            d1b += a1 * acc7b;

            pOut[7 ] = acc4b ;
            d2b += a2 * acc7b;

            /* Sample 15. 5 cycles */
            pOut[8 ] = acc5a ;
            acc8a = b0 * Xn8a + d1a;

            pOut[9 ] = acc5b;
            d1a = b1 * Xn8a + d2a;

            pOut[10] = acc6a;
            d2a = b2 * Xn8a;

            pOut[11] = acc6b;
            d1a += a1 * acc8a;

            pOut[12] = acc7a;
            d2a += a2 * acc8a;

            /* Sample 16. 5 cycles */
            pOut[13] = acc7b;
            acc8b = b0 * Xn8b + d1b;

            pOut[14] = acc8a;
            d1b = b1 * Xn8b + d2b;

            pOut[15] = acc8b;
            d2b = b2 * Xn8b;

            sample--;
            d1b += a1 * acc8b;

            pOut += 16;
            d2b += a2 * acc8b;
        }

        sample = blockSize & 0x7U;
        while (sample > 0U) {
            /* Read the input */
            Xn1a = *pIn++; //Channel a
            Xn1b = *pIn++; //Channel b

            /* y[n] = b0 * x[n] + d1 */
            acc1a = (b0 * Xn1a) + d1a;
            acc1b = (b0 * Xn1b) + d1b;

            /* Store the result in the accumulator in the destination buffer. */
            *pOut++ = acc1a;
            *pOut++ = acc1b;

            /* Every time after the output is computed state should be updated. */
            /* d1 = b1 * x[n] + a1 * y[n] + d2 */
            d1a = ((b1 * Xn1a) + (a1 * acc1a)) + d2a;
            d1b = ((b1 * Xn1b) + (a1 * acc1b)) + d2b;

            /* d2 = b2 * x[n] + a2 * y[n] */
            d2a = (b2 * Xn1a) + (a2 * acc1a);
            d2b = (b2 * Xn1b) + (a2 * acc1b);

            sample--;
        }

        /* Store the updated state variables back into the state array */
        pState[0] = d1a;
        pState[1] = d2a;

        pState[2] = d1b;
        pState[3] = d2b;

        /* The current stage input is given as the output to the next stage */
        pIn = pDst;
        /* decrement the loop counter */
        stage--;

        pState += 4U;
        /*Reset the output working pointer */
        pOut = pDst;

    } while (stage > 0U);

#elif defined(ARM_MATH_CM0_FAMILY)

    /* Run the below code for Cortex-M0 */

    do
    {
        /* Reading the coefficients */
        b0 = *pCoeffs++;
        b1 = *pCoeffs++;
        b2 = *pCoeffs++;
        a1 = *pCoeffs++;
        a2 = *pCoeffs++;

        /*Reading the state values */
        d1a = pState[0];
        d2a = pState[1];
        d1b = pState[2];
        d2b = pState[3];


        sample = blockSize;

        while (sample > 0U)
        {
            /* Read the input */
            Xn1a = *pIn++; //Channel a
            Xn1b = *pIn++; //Channel b

            /* y[n] = b0 * x[n] + d1 */
            acc1a = (b0 * Xn1a) + d1a;
            acc1b = (b0 * Xn1b) + d1b;

            /* Store the result in the accumulator in the destination buffer. */
            *pOut++ = acc1a;
            *pOut++ = acc1b;

            /* Every time after the output is computed state should be updated. */
            /* d1 = b1 * x[n] + a1 * y[n] + d2 */
            d1a = ((b1 * Xn1a) + (a1 * acc1a)) + d2a;
            d1b = ((b1 * Xn1b) + (a1 * acc1b)) + d2b;

            /* d2 = b2 * x[n] + a2 * y[n] */
            d2a = (b2 * Xn1a) + (a2 * acc1a);
            d2b = (b2 * Xn1b) + (a2 * acc1b);

            /* decrement the loop counter */
            sample--;
        }

        /* Store the updated state variables back into the state array */
        *pState++ = d1a;
        *pState++ = d2a;
        *pState++ = d1b;
        *pState++ = d2b;

        /* The current stage input is given as the output to the next stage */
        pIn = pDst;

        /*Reset the output working pointer */
        pOut = pDst;

        /* decrement the loop counter */
        stage--;

    } while (stage > 0U);

#else

    float32_t Xn2a, Xn3a, Xn4a;                          /*  Input State variables     */
    float32_t Xn2b, Xn3b, Xn4b;                          /*  Input State variables     */
    float32_t acc2a, acc3a, acc4a;                       /*  accumulator               */
    float32_t acc2b, acc3b, acc4b;                       /*  accumulator               */
    float32_t p0a, p1a, p2a, p3a, p4a, A1a;
    float32_t p0b, p1b, p2b, p3b, p4b, A1b;

    /* Run the below code for Cortex-M4 and Cortex-M3 */
    do
    {
        /* Reading the coefficients */
        b0 = *pCoeffs++;
        b1 = *pCoeffs++;
        b2 = *pCoeffs++;
        a1 = *pCoeffs++;
        a2 = *pCoeffs++;

        /*Reading the state values */
        d1a = pState[0];
        d2a = pState[1];
        d1b = pState[2];
        d2b = pState[3];

        /* Apply loop unrolling and compute 4 output values simultaneously. */
        sample = blockSize >> 2U;

        /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
        ** a second loop below computes the remaining 1 to 3 samples. */
        while (sample > 0U) {

            /* y[n] = b0 * x[n] + d1 */
            /* d1 = b1 * x[n] + a1 * y[n] + d2 */
            /* d2 = b2 * x[n] + a2 * y[n] */

            /* Read the four inputs */
            Xn1a = pIn[0];
            Xn1b = pIn[1];
            Xn2a = pIn[2];
            Xn2b = pIn[3];
            Xn3a = pIn[4];
            Xn3b = pIn[5];
            Xn4a = pIn[6];
            Xn4b = pIn[7];
            pIn += 8;

            p0a = b0 * Xn1a;
            p0b = b0 * Xn1b;
            p1a = b1 * Xn1a;
            p1b = b1 * Xn1b;
            acc1a = p0a + d1a;
            acc1b = p0b + d1b;
            p0a = b0 * Xn2a;
            p0b = b0 * Xn2b;
            p3a = a1 * acc1a;
            p3b = a1 * acc1b;
            p2a = b2 * Xn1a;
            p2b = b2 * Xn1b;
            A1a = p1a + p3a;
            A1b = p1b + p3b;
            p4a = a2 * acc1a;
            p4b = a2 * acc1b;
            d1a = A1a + d2a;
            d1b = A1b + d2b;
            d2a = p2a + p4a;
            d2b = p2b + p4b;

            p1a = b1 * Xn2a;
            p1b = b1 * Xn2b;
            acc2a = p0a + d1a;
            acc2b = p0b + d1b;
            p0a = b0 * Xn3a;
            p0b = b0 * Xn3b;
            p3a = a1 * acc2a;
            p3b = a1 * acc2b;
            p2a = b2 * Xn2a;
            p2b = b2 * Xn2b;
            A1a = p1a + p3a;
            A1b = p1b + p3b;
            p4a = a2 * acc2a;
            p4b = a2 * acc2b;
            d1a = A1a + d2a;
            d1b = A1b + d2b;
            d2a = p2a + p4a;
            d2b = p2b + p4b;

            p1a = b1 * Xn3a;
            p1b = b1 * Xn3b;
            acc3a = p0a + d1a;
            acc3b = p0b + d1b;
            p0a = b0 * Xn4a;
            p0b = b0 * Xn4b;
            p3a = a1 * acc3a;
            p3b = a1 * acc3b;
            p2a = b2 * Xn3a;
            p2b = b2 * Xn3b;
            A1a = p1a + p3a;
            A1b = p1b + p3b;
            p4a = a2 * acc3a;
            p4b = a2 * acc3b;
            d1a = A1a + d2a;
            d1b = A1b + d2b;
            d2a = p2a + p4a;
            d2b = p2b + p4b;

            acc4a = p0a + d1a;
            acc4b = p0b + d1b;
            p1a = b1 * Xn4a;
            p1b = b1 * Xn4b;
            p3a = a1 * acc4a;
            p3b = a1 * acc4b;
            p2a = b2 * Xn4a;
            p2b = b2 * Xn4b;
            A1a = p1a + p3a;
            A1b = p1b + p3b;
            p4a = a2 * acc4a;
            p4b = a2 * acc4b;
            d1a = A1a + d2a;
            d1b = A1b + d2b;
            d2a = p2a + p4a;
            d2b = p2b + p4b;

            pOut[0] = acc1a;
            pOut[1] = acc1b;
            pOut[2] = acc2a;
            pOut[3] = acc2b;
            pOut[4] = acc3a;
            pOut[5] = acc3b;
            pOut[6] = acc4a;
            pOut[7] = acc4b;
            pOut += 8;

            sample--;
        }

        sample = blockSize & 0x3U;
        while (sample > 0U) {
            Xn1a = *pIn++;
            Xn1b = *pIn++;

            p0a = b0 * Xn1a;
            p0b = b0 * Xn1b;
            p1a = b1 * Xn1a;
            p1b = b1 * Xn1b;
            acc1a = p0a + d1a;
            acc1b = p0b + d1b;
            p3a = a1 * acc1a;
            p3b = a1 * acc1b;
            p2a = b2 * Xn1a;
            p2b = b2 * Xn1b;
            A1a = p1a + p3a;
            A1b = p1b + p3b;
            p4a = a2 * acc1a;
            p4b = a2 * acc1b;
            d1a = A1a + d2a;
            d1b = A1b + d2b;
            d2a = p2a + p4a;
            d2b = p2b + p4b;

            *pOut++ = acc1a;
            *pOut++ = acc1b;

            sample--;
        }

        /* Store the updated state variables back into the state array */
        *pState++ = d1a;
        *pState++ = d2a;
        *pState++ = d1b;
        *pState++ = d2b;

        /* The current stage input is given as the output to the next stage */
        pIn = pDst;

        /*Reset the output working pointer */
        pOut = pDst;

        /* decrement the loop counter */
        stage--;

    } while (stage > 0U);

#endif

}
LOW_OPTIMIZATION_EXIT

/**
   * @} end of BiquadCascadeDF2T group
   */
