#include "ref.h"

void ref_max_f32(
  float32_t * pSrc,
  uint32_t blockSize,
  float32_t * pResult,
  uint32_t * pIndex)
{
	uint32_t i, ind=0;
	float32_t max=-FLT_MAX;
	
	for(i=0;i<blockSize;i++)
	{
		if (max < pSrc[i])
		{
			max = pSrc[i];
			ind = i;
		}
	}
	*pResult = max;
	*pIndex = ind;
}

void ref_max_q31(
  q31_t * pSrc,
  uint32_t blockSize,
  q31_t * pResult,
  uint32_t * pIndex)
{
	uint32_t i, ind=0;
	q31_t max=INT_MIN;
	
	for(i=0;i<blockSize;i++)
	{
		if (max < pSrc[i])
		{
			max = pSrc[i];
			ind = i;
		}
	}
	*pResult = max;
	*pIndex = ind;
}

void ref_max_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q15_t * pResult,
  uint32_t * pIndex)
{
	uint32_t i, ind=0;
	q15_t max=SHRT_MIN;
	
	for(i=0;i<blockSize;i++)
	{
		if (max < pSrc[i])
		{
			max = pSrc[i];
			ind = i;
		}
	}
	*pResult = max;
	*pIndex = ind;
}

void ref_max_q7(
  q7_t * pSrc,
  uint32_t blockSize,
  q7_t * pResult,
  uint32_t * pIndex)
{
	uint32_t i, ind=0;
	q7_t max=SCHAR_MIN;
	
	for(i=0;i<blockSize;i++)
	{
		if (max < pSrc[i])
		{
			max = pSrc[i];
			ind = i;
		}
	}
	*pResult = max;
	*pIndex = ind;
}
