#include "ref.h"

void ref_cmplx_dot_prod_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  uint32_t numSamples,
  float32_t * realResult,
  float32_t * imagResult)
{
	float32_t sumr, sumi;
	uint32_t i;
	
	sumr = 0;
	sumi = 0;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		sumr += pSrcA[i] * pSrcB[i]   - pSrcA[i+1] * pSrcB[i+1];
		sumi += pSrcA[i] * pSrcB[i+1] + pSrcA[i+1] * pSrcB[i];
	}
	
	*realResult = sumr;
	*imagResult = sumi;
}

void ref_cmplx_dot_prod_q31(
  q31_t * pSrcA,
  q31_t * pSrcB,
  uint32_t numSamples,
  q63_t * realResult,
  q63_t * imagResult)
{
	q63_t sumr, sumi;
	uint32_t i;
	
	sumr = 0;
	sumi = 0;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		//shifting down 14 here to provide guard bits
		sumr += (((q63_t)pSrcA[i] * pSrcB[i]  ) >> 14) - (((q63_t)pSrcA[i+1] * pSrcB[i+1]) >> 14);
		sumi += (((q63_t)pSrcA[i] * pSrcB[i+1]) >> 14) + (((q63_t)pSrcA[i+1] * pSrcB[i]  ) >> 14);
	}
	
	*realResult = sumr;
	*imagResult = sumi;
}

void ref_cmplx_dot_prod_q15(
  q15_t * pSrcA,
  q15_t * pSrcB,
  uint32_t numSamples,
  q31_t * realResult,
  q31_t * imagResult)
{
	q63_t sumr, sumi;
	uint32_t i;
	
	sumr = 0;
	sumi = 0;
	
	for(i=0;i<numSamples*2;i+=2)
	{
		sumr += (q31_t)pSrcA[i] * pSrcB[i]   - (q31_t)pSrcA[i+1] * pSrcB[i+1];
		sumi += (q31_t)pSrcA[i] * pSrcB[i+1] + (q31_t)pSrcA[i+1] * pSrcB[i];
	}
	
	//shifting down 6 at the end here because there are already 32 guard bits available, this method is more accurate
	*realResult = (q31_t)(sumr >> 6);
	*imagResult = (q31_t)(sumi >> 6);
}
