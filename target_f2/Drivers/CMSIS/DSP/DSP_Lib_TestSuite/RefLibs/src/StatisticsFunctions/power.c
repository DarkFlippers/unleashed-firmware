#include "ref.h"

void ref_power_f32(
  float32_t * pSrc,
  uint32_t blockSize,
  float32_t * pResult)
{
	uint32_t i;
	float32_t sumsq=0;
	
	for(i=0;i<blockSize;i++)
	{
			sumsq += pSrc[i] * pSrc[i];
	}
	*pResult = sumsq;
}

void ref_power_q31(
  q31_t * pSrc,
  uint32_t blockSize,
  q63_t * pResult)
{
	uint32_t i;
	q63_t sumsq=0;
	
	for(i=0;i<blockSize;i++)
	{
			sumsq += ((q63_t)pSrc[i] * pSrc[i]) >> 14;
	}
	*pResult = sumsq;
}

void ref_power_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q63_t * pResult)
{
	uint32_t i;
	q63_t sumsq=0;
	
	for(i=0;i<blockSize;i++)
	{
			sumsq += (q63_t)pSrc[i] * pSrc[i];
	}
	*pResult = sumsq;
}

void ref_power_q7(
  q7_t * pSrc,
  uint32_t blockSize,
  q31_t * pResult)
{
	uint32_t i;
	q31_t sumsq=0;
	
	for(i=0;i<blockSize;i++)
	{
			sumsq += (q31_t)pSrc[i] * pSrc[i];
	}
	*pResult = sumsq;
}
