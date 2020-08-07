#include "ref.h"

void ref_cmplx_mult_real_f32(
  float32_t * pSrcCmplx,
  float32_t * pSrcReal,
  float32_t * pCmplxDst,
  uint32_t numSamples)
{
	uint32_t i;
	
	for(i=0;i<numSamples;i++)
	{
		pCmplxDst[2*i+0] = pSrcCmplx[2*i+0] * pSrcReal[i];
		pCmplxDst[2*i+1] = pSrcCmplx[2*i+1] * pSrcReal[i];
	}
}

void ref_cmplx_mult_real_q31(
  q31_t * pSrcCmplx,
  q31_t * pSrcReal,
  q31_t * pCmplxDst,
  uint32_t numSamples)
{
	uint32_t i;
	q31_t tempR, tempI;
	
	for(i=0;i<numSamples;i++)
	{
		tempR = ((q63_t) pSrcCmplx[2*i+0] * pSrcReal[i]) >> 32;
		tempI = ((q63_t) pSrcCmplx[2*i+1] * pSrcReal[i]) >> 32;
		pCmplxDst[2*i+0] = ref_sat_n(tempR, 31) << 1;
		pCmplxDst[2*i+1] = ref_sat_n(tempI, 31) << 1;
	}
}

void ref_cmplx_mult_real_q15(
  q15_t * pSrcCmplx,
  q15_t * pSrcReal,
  q15_t * pCmplxDst,
  uint32_t numSamples)
{
	uint32_t i;
	q31_t tempR, tempI;
	
	for(i=0;i<numSamples;i++)
	{
		tempR = ((q31_t) pSrcCmplx[2*i+0] * pSrcReal[i]) >> 15;
		tempI = ((q31_t) pSrcCmplx[2*i+1] * pSrcReal[i]) >> 15;
		pCmplxDst[2*i+0] = ref_sat_q15(tempR);
		pCmplxDst[2*i+1] = ref_sat_q15(tempI);
	}
}
