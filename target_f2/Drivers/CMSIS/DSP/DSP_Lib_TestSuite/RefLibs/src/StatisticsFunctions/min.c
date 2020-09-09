#include "ref.h"

void ref_min_f32(
  float32_t * pSrc,
  uint32_t blockSize,
  float32_t * pResult,
  uint32_t * pIndex)
{
	uint32_t i, ind=0;
	float32_t min=FLT_MAX;
	
	for(i=0;i<blockSize;i++)
	{
		if (min > pSrc[i])
		{
			min = pSrc[i];
			ind = i;
		}
	}
	*pResult = min;
	*pIndex = ind;
}

void ref_min_q31(
  q31_t * pSrc,
  uint32_t blockSize,
  q31_t * pResult,
  uint32_t * pIndex)
{
	uint32_t i, ind=0;
	q31_t min=INT_MAX;
	
	for(i=0;i<blockSize;i++)
	{
		if (min > pSrc[i])
		{
			min = pSrc[i];
			ind = i;
		}
	}
	*pResult = min;
	*pIndex = ind;
}

void ref_min_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q15_t * pResult,
  uint32_t * pIndex)
{
	uint32_t i, ind=0;
	q15_t min=SHRT_MAX;
	
	for(i=0;i<blockSize;i++)
	{
		if (min > pSrc[i])
		{
			min = pSrc[i];
			ind = i;
		}
	}
	*pResult = min;
	*pIndex = ind;
}

void ref_min_q7(
  q7_t * pSrc,
  uint32_t blockSize,
  q7_t * pResult,
  uint32_t * pIndex)
{
	uint32_t i, ind=0;
	q7_t min=SCHAR_MAX;
	
	for(i=0;i<blockSize;i++)
	{
		if (min > pSrc[i])
		{
			min = pSrc[i];
			ind = i;
		}
	}
	*pResult = min;
	*pIndex = ind;
}
