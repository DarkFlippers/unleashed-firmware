#include "ref.h"

void ref_cmplx_mag_squared_f32(
  float32_t * pSrc,
  float32_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		*pDst++ = pSrc[i] * pSrc[i] + pSrc[i+1] * pSrc[i+1];
	}
}

void ref_cmplx_mag_squared_q31(
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	q31_t acc0,acc1;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		acc0 = (q31_t)(((q63_t)pSrc[i] * pSrc[i]) >> 33);
		acc1 = (q31_t)(((q63_t)pSrc[i+1] * pSrc[i+1]) >> 33);
		*pDst++ = acc0 + acc1;
	}
}

void ref_cmplx_mag_squared_q15(
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	q31_t acc0,acc1;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		acc0 = pSrc[i] * pSrc[i];
		acc1 = pSrc[i+1] * pSrc[i+1];
		*pDst++ = (q15_t) (((q63_t) acc0 + acc1) >> 17);
	}
}
