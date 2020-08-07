#include "ref.h"

void ref_shift_q31(
  q31_t * pSrc,
  int8_t shiftBits,
  q31_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	if (shiftBits < 0)
	{
		for(i=0;i<blockSize;i++)
		{
			pDst[i] = pSrc[i] << shiftBits;
		}
	}
	else
	{
		for(i=0;i<blockSize;i++)
		{
			pDst[i] = pSrc[i] >> -shiftBits;
		}
	}
}

void ref_shift_q15(
  q15_t * pSrc,
  int8_t shiftBits,
  q15_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	if (shiftBits < 0)
	{
		for(i=0;i<blockSize;i++)
		{
			pDst[i] = pSrc[i] << shiftBits;
		}
	}
	else
	{
		for(i=0;i<blockSize;i++)
		{
			pDst[i] = pSrc[i] >> -shiftBits;
		}
	}
}

void ref_shift_q7(
  q7_t * pSrc,
  int8_t shiftBits,
  q7_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	if (shiftBits < 0)
	{
		for(i=0;i<blockSize;i++)
		{
			pDst[i] = pSrc[i] << shiftBits;
		}
	}
	else
	{
		for(i=0;i<blockSize;i++)
		{
			pDst[i] = pSrc[i] >> -shiftBits;
		}
	}
}
