#include "ref.h"

void ref_offset_f32(
  float32_t * pSrc,
  float32_t offset,
  float32_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = pSrc[i] + offset;
	}
}

void ref_offset_q31(
  q31_t * pSrc,
  q31_t offset,
  q31_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ref_sat_q31( (q63_t)pSrc[i] + offset );
	}
}

void ref_offset_q15(
  q15_t * pSrc,
  q15_t offset,
  q15_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ref_sat_q15( (q31_t)pSrc[i] + offset );
	}
}

void ref_offset_q7(
  q7_t * pSrc,
  q7_t offset,
  q7_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ref_sat_q7( (q15_t)pSrc[i] + offset );
	}
}
