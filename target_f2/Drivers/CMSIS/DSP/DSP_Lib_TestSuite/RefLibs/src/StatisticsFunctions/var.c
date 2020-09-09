#include "ref.h"

void ref_var_f32(
  float32_t * pSrc,
  uint32_t blockSize,
  float32_t * pResult)
{
	uint32_t i;
	float32_t sum=0, sumsq=0;
	
	if (blockSize == 1)
	{
		*pResult = 0;
		return;
	}
	
	for(i=0;i<blockSize;i++)
	{
			sum += pSrc[i];
			sumsq += pSrc[i] * pSrc[i];
	}
	*pResult = (sumsq - sum * sum / (float32_t)blockSize) / ((float32_t)blockSize - 1);
}

void ref_var_q31(
  q31_t * pSrc,
  uint32_t blockSize,
  q31_t * pResult)
{
	uint32_t i;
	q63_t sum=0, sumsq=0;
	q31_t in;
	
	if (blockSize == 1)
	{
		*pResult = 0;
		return;
	}
	
	for(i=0;i<blockSize;i++)
	{
			in = pSrc[i] >> 8;
			sum += in;
			sumsq += (q63_t)in * in;
	}
	*pResult = (sumsq - sum * sum / (q31_t)blockSize) / ((q31_t)blockSize - 1) >> 15;
}

void ref_var_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q15_t * pResult)
{
	uint32_t i;
	q31_t sum=0;
	q63_t sumsq=0;
	
	if (blockSize == 1)
	{
		*pResult = 0;
		return;
	}
	
	for(i=0;i<blockSize;i++)
	{
			sum += pSrc[i];
			sumsq += (q63_t)pSrc[i] * pSrc[i];
	}
	*pResult = (q31_t)((sumsq - (q63_t)sum * sum / (q63_t)blockSize) / ((q63_t)blockSize - 1)) >> 15;
}
