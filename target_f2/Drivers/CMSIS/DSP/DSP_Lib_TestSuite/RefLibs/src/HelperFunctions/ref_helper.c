#include "ref.h"

float32_t scratchArray[8192*2]; 

arm_cfft_instance_f32 ref_cfft_sR_f32_len8192 = { 8192, 0, 0, 0 };
	
q31_t ref_sat_n(q31_t num, uint32_t bits)
{
	int32_t posMax, negMin;
	uint32_t i;

	posMax = 1;
	for (i = 0; i < (bits - 1); i++)
	{
		posMax = posMax * 2;
	}

	if (num > 0)
	{
		posMax = (posMax - 1);

		if (num > posMax)
		{
			num = posMax;
		}
	}
	else
	{
		negMin = -posMax;

		if (num < negMin)
		{
			num = negMin;
		}
	}
	return (num);
}

q31_t ref_sat_q31(q63_t num)
{
	if (num > (q63_t)INT_MAX)
	{
		return INT_MAX;
	}
	else if (num < (q63_t)0xffffffff80000000ll)
	{
		return INT_MIN;
	}
	else
	{
		return (q31_t)num;
	}
}

q15_t ref_sat_q15(q31_t num)
{
	if (num > (q31_t)SHRT_MAX)
	{
		return SHRT_MAX;
	}
	else if (num < (q31_t)0xffff8000)
	{
		return SHRT_MIN;
	}
	else
	{
		return (q15_t)num;
	}
}

q7_t ref_sat_q7(q15_t num)
{
	if (num > (q15_t)SCHAR_MAX)
	{
		return SCHAR_MAX;
	}
	else if (num < (q15_t)0xff80)
	{
		return SCHAR_MIN;
	}
	else
	{
		return (q7_t)num;
	}
}

float32_t ref_pow(float32_t a, uint32_t b)
{
	uint32_t i;
	float32_t r = a;
	
	for(i=1;i<b;i++) 
	{
		r *= a;
	}
	
	if ( b == 0)
	{
		return 1;
	}
	
	return r;
}
