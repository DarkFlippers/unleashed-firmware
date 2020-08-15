#include "ref.h"

void ref_q31_to_q15(
  q31_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = pSrc[i] >> 16;
	}
}

void ref_q31_to_q7(
  q31_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = pSrc[i] >> 24;
	}
}

void ref_q15_to_q31(
  q15_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ((q31_t)pSrc[i]) << 16;
	}
}

void ref_q15_to_q7(
  q15_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = pSrc[i] >> 8;
	}
}

void ref_q7_to_q31(
  q7_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ((q31_t)pSrc[i]) << 24;
	}
}

void ref_q7_to_q15(
  q7_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ((q15_t)pSrc[i]) << 8;
	}
}
