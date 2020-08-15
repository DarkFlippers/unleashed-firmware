#include "ref.h"

void ref_cmplx_mag_f32(
  float32_t * pSrc,
  float32_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		*pDst++ = sqrtf(pSrc[i] * pSrc[i] + pSrc[i+1] * pSrc[i+1]);
	}
}

void ref_cmplx_mag_q31(
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	q31_t acc0,acc1,out;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		acc0 = (q31_t)(((q63_t)pSrc[i] * pSrc[i]) >> 33);
		acc1 = (q31_t)(((q63_t)pSrc[i+1] * pSrc[i+1]) >> 33);
		out = acc0 + acc1;
		*pDst++ = (q31_t)(sqrtf((float)out / 2147483648.0f) * 2147483648.0f);
	}
}

void ref_cmplx_mag_q15(
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	q31_t acc0,acc1;
	q15_t out;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		acc0 = pSrc[i] * pSrc[i];
		acc1 = pSrc[i+1] * pSrc[i+1];
		out = (q15_t) (((q63_t) acc0 + acc1) >> 17);
		*pDst++ = (q15_t)(sqrtf((float)out / 32768.0f) * 32768.0f);
	}
}
