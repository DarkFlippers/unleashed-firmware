#include "ref.h"

void ref_mean_f32(
  float32_t * pSrc,
  uint32_t blockSize,
  float32_t * pResult)
{
	uint32_t i;
	float32_t sum=0;
	
	for(i=0;i<blockSize;i++)
	{
			sum += pSrc[i];
	}
	*pResult = sum / (float32_t)blockSize;
}

void ref_mean_q31(
  q31_t * pSrc,
  uint32_t blockSize,
  q31_t * pResult)
{
	uint32_t i;
	q63_t sum=0;
	
	for(i=0;i<blockSize;i++)
	{
			sum += pSrc[i];
	}
	*pResult = (q31_t) (sum / (int32_t) blockSize);
}

void ref_mean_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q15_t * pResult)
{
	uint32_t i;
	q31_t sum=0;
	
	for(i=0;i<blockSize;i++)
	{
			sum += pSrc[i];
	}
	*pResult = (q15_t) (sum / (int32_t) blockSize);
}

void ref_mean_q7(
  q7_t * pSrc,
  uint32_t blockSize,
  q7_t * pResult)
{
	uint32_t i;
	q31_t sum=0;
	
	for(i=0;i<blockSize;i++)
	{
			sum += pSrc[i];
	}
	*pResult = (q7_t) (sum / (int32_t) blockSize);
}
