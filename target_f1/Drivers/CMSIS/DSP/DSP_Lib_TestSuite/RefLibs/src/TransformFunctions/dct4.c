#include "ref.h"

void ref_dct4_f32(
  const arm_dct4_instance_f32 * S,
  float32_t * pState,
  float32_t * pInlineBuffer)
{
	uint32_t n,k;
	float32_t sum;
   float32_t pi_by_N = 3.14159265358979f / (float32_t)S->N;
   float32_t tmp;
   float32_t normalize = sqrtf(2.0f / (float32_t)S->N);
	
	for(k=0;k<S->N;k++)
	{
		sum=0.0f;
      tmp = ((float32_t)k + 0.5f)*pi_by_N;
		for(n=0;n<S->N;n++)
		{
			sum += pInlineBuffer[n] * cosf(tmp * ((float32_t)n + 0.5f));
		}
		scratchArray[k] = normalize * sum;
	}
	
	for(k=0;k<S->N;k++)
	{
		pInlineBuffer[k] = scratchArray[k];
	}
}

void ref_dct4_q31(
  const arm_dct4_instance_q31 * S,
  q31_t * pState,
  q31_t * pInlineBuffer)
{
	arm_dct4_instance_f32 SS;
	float32_t *fSrc = (float32_t*)pInlineBuffer;
	uint32_t i;
	
	SS.N = S->N;
	
	for(i=0;i<S->N;i++)
	{
		//read the q31 data, cast to float, scale down for float
		fSrc[i] = (float32_t)pInlineBuffer[i] / 2147483648.0f;
	}
	
	ref_dct4_f32(&SS,(float32_t*)0,fSrc);
	
	for(i=0;i<S->N;i++)
	{
		fSrc[i] = fSrc[i] * 2147483648.0f / (float32_t)S->N ;
		fSrc[i] += (fSrc[i] > 0) ? 0.5f : -0.5f;
		pInlineBuffer[i] = (q31_t)fSrc[i];
	}
}

void ref_dct4_q15(
  const arm_dct4_instance_q15 * S,
  q15_t * pState,
  q15_t * pInlineBuffer)
{
	arm_dct4_instance_f32 SS;
	float32_t *fSrc = (float32_t*)pInlineBuffer;
	uint32_t i;
	
	SS.N = S->N;
	
	for(i=0;i<S->N;i++)
	{
		//read the q15 data, cast to float, scale down for float, place in temporary buffer
		scratchArray[i] = (float32_t)pInlineBuffer[i] / 32768.0f;
	}
	
	for(i=0;i<S->N;i++)
	{
		//copy from temp buffer to final buffer
		fSrc[i] = scratchArray[i];
	}
	
	ref_dct4_f32(&SS,(float32_t*)0,fSrc);
	
	for(i=0;i<S->N;i++)
	{
		fSrc[i] = fSrc[i] * 32768.0f / (float32_t)S->N;
		fSrc[i] += (fSrc[i] > 0) ? 0.5f : -0.5f;
		pInlineBuffer[i] = (q15_t)fSrc[i];
	}
}
