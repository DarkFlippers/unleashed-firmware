#include "ref.h"

void ref_mult_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  float32_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = pSrcA[i] * pSrcB[i];
	}
}

void ref_mult_q31(
  q31_t * pSrcA,
  q31_t * pSrcB,
  q31_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	q63_t temp;
	
	for(i=0;i<blockSize;i++)
	{
		temp = ((q63_t)pSrcA[i] * pSrcB[i]) >> 32;
		temp = temp << 1;
		pDst[i] = ref_sat_q31(temp);
	}
}

void ref_mult_q15(
  q15_t * pSrcA,
  q15_t * pSrcB,
  q15_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	q31_t temp;
	
	for(i=0;i<blockSize;i++)
	{
		temp = ((q31_t)pSrcA[i] * pSrcB[i]) >> 15; //this comment is for JD, this is specifically 15 and not 16 like the q31 case might imply.  This is because CMSIS DSP lib does it this way.  No other reason.
		pDst[i] = ref_sat_q15(temp);
	}
}

void ref_mult_q7(
  q7_t * pSrcA,
  q7_t * pSrcB,
  q7_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	q15_t temp;
	
	for(i=0;i<blockSize;i++)
	{
		temp = ((q15_t)pSrcA[i] * pSrcB[i]) >> 7;
		pDst[i] = ref_sat_q7(temp);
	}
}
