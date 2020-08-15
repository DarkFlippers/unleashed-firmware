#include "ref.h"

void ref_cmplx_conj_f32(
  float32_t * pSrc,
  float32_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	for(i=0;i<numSamples*2;i+=2)
	{
		pDst[i] = pSrc[i];
		pDst[i+1] = -pSrc[i+1];
	}
}

void ref_cmplx_conj_q31(
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	for(i=0;i<numSamples*2;i+=2)
	{
		pDst[i] = pSrc[i];
		pDst[i+1] = -pSrc[i+1];
	}
}

void ref_cmplx_conj_q15(
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t numSamples)
{
	uint32_t i;
	for(i=0;i<numSamples*2;i+=2)
	{
		pDst[i] = pSrc[i];
		pDst[i+1] = -pSrc[i+1];
	}
}
