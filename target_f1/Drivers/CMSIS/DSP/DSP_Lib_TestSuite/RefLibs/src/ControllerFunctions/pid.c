#include "ref.h"

float32_t ref_pid_f32(
	arm_pid_instance_f32 * S,
	float32_t in)
{
	float32_t out;

	/* y[n] = y[n-1] + A0 * x[n] + A1 * x[n-1] + A2 * x[n-2]  */
	out = S->state[2] + S->A0 * in + S->A1 * S->state[0] + S->A2 * S->state[1];

	/* Update state */
	S->state[1] = S->state[0];
	S->state[0] = in;
	S->state[2] = out;

	/* return to application */
	return (out);
}

q31_t ref_pid_q31(
	arm_pid_instance_q31 * S,
	q31_t in)
{
	q63_t acc;
	q31_t out;

	/* acc = A0 * x[n]  */
	acc = (q63_t) S->A0 * in;

	/* acc += A1 * x[n-1] */
	acc += (q63_t) S->A1 * S->state[0];

	/* acc += A2 * x[n-2]  */
	acc += (q63_t) S->A2 * S->state[1];

	/* convert output to 1.31 format to add y[n-1] */
	out = (q31_t) (acc >> 31U);

	/* out += y[n-1] */
	out += S->state[2];

	/* Update state */
	S->state[1] = S->state[0];
	S->state[0] = in;
	S->state[2] = out;

	/* return to application */
	return (out);
}

q15_t ref_pid_q15(
	arm_pid_instance_q15 * S,
	q15_t in)
{
	q63_t acc;
	q15_t out;
	q15_t A1, A2;
	
#if defined (ARM_MATH_DSP)
   
#ifndef  ARM_MATH_BIG_ENDIAN
	A2 = S->A1 >> 16;
	A1 = (q15_t)S->A1;	
#else
	A1 = S->A1 >> 16;
	A2 = (q15_t)S->A1;	
#endif
   
#else
	
	A1 = S->A1;
	A2 = S->A2;
	
#endif	
	
	/* acc = A0 * x[n]  */
	acc = ((q31_t) S->A0) * in;

	/* acc += A1 * x[n-1] + A2 * x[n-2]  */
	acc += (q31_t) A1 * S->state[0];
	acc += (q31_t) A2 * S->state[1];

	/* acc += y[n-1] */
	acc += (q31_t) S->state[2] << 15;

	/* saturate the output */
	out = ref_sat_q15(acc >> 15);

	/* Update state */
	S->state[1] = S->state[0];
	S->state[0] = in;
	S->state[2] = out;

	/* return to application */
	return (out);
}
