#include "ref.h"

void ref_q63_to_float(
  q63_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ((float32_t)pSrc[i]) / 9223372036854775808.0f;
	}
}

void ref_q31_to_float(
  q31_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ((float32_t)pSrc[i]) / 2147483648.0f;
	}
}
	
void ref_q15_to_float(
  q15_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ((float32_t)pSrc[i]) / 32768.0f;
	}
}
	
void ref_q7_to_float(
  q7_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ((float32_t)pSrc[i]) / 128.0f;
	}
}
