#include "ref.h"

q31_t ref_cos_q31(q31_t x)
{
	return (q31_t)(cosf((float32_t)x * 6.28318530717959f / 2147483648.0f) * 2147483648.0f);
}

q15_t ref_cos_q15(q15_t x)
{
	return (q15_t)(cosf((float32_t)x * 6.28318530717959f / 32768.0f) * 32768.0f);
}
