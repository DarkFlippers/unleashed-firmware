#include "ref.h"

arm_status ref_sqrt_q31(q31_t in, q31_t * pOut)
{
	*pOut = (q31_t)(sqrtf((float32_t)in / 2147483648.0f) * 2147483648.0f);
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_sqrt_q15(q15_t in, q15_t * pOut)
{
	*pOut = (q15_t)(sqrtf((float32_t)in / 32768.0f) * 32768.0f);
	
	return ARM_MATH_SUCCESS;
}
