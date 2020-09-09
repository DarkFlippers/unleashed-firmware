#include "ref.h"

void ref_cmplx_mult_cmplx_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  float32_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		pDst[i]   = pSrcA[i] * pSrcB[i]   - pSrcA[i+1] * pSrcB[i+1];
		pDst[i+1] = pSrcA[i] * pSrcB[i+1] + pSrcA[i+1] * pSrcB[i];
	}
}

void ref_cmplx_mult_cmplx_q31(
  q31_t * pSrcA,
  q31_t * pSrcB,
  q31_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	q31_t mul1, mul2, mul3, mul4;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		mul1 = ((q63_t)pSrcA[i]   * pSrcB[i])   >> 33;
		mul2 = ((q63_t)pSrcA[i+1] * pSrcB[i+1]) >> 33;
		mul3 = ((q63_t)pSrcA[i]   * pSrcB[i+1]) >> 33;
		mul4 = ((q63_t)pSrcA[i+1] * pSrcB[i])   >> 33;
		pDst[i]   = mul1 - mul2;
		pDst[i+1] = mul3 + mul4;
	}
}

void ref_cmplx_mult_cmplx_q15(
  q15_t * pSrcA,
  q15_t * pSrcB,
  q15_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	q31_t mul1, mul2, mul3, mul4;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		mul1 = ((q31_t)pSrcA[i]   * pSrcB[i])   >> 17;
		mul2 = ((q31_t)pSrcA[i+1] * pSrcB[i+1]) >> 17;
		mul3 = ((q31_t)pSrcA[i]   * pSrcB[i+1]) >> 17;
		mul4 = ((q31_t)pSrcA[i+1] * pSrcB[i])   >> 17;
		pDst[i]   = mul1 - mul2;
		pDst[i+1] = mul3 + mul4;
	}
}
