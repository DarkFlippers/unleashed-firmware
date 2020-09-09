#include "ref.h"

void ref_float_to_q31(
  float32_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	float32_t in;
	
	for(i=0;i<blockSize;i++)
	{
		in = pSrc[i];
		in *= 2147483648.0f;				//scale up
		in += in > 0.0f ? 0.5f : -0.5f;		//round
		pDst[i] = ref_sat_q31((q63_t)in);	//cast and saturate
	}
}
	
void ref_float_to_q15(
  float32_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	float32_t in;
	
	for(i=0;i<blockSize;i++)
	{
		in = pSrc[i];
		in *= 32768.0f;
		in += in > 0.0f ? 0.5f : -0.5f;
		pDst[i] = ref_sat_q15((q31_t)in);
	}
}
	
void ref_float_to_q7(
  float32_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	float32_t in;
	
	for(i=0;i<blockSize;i++)
	{
		in = pSrc[i];
		in *= 128.0f;
		in += in > 0.0f ? 0.5f : -0.5f;
		pDst[i] = ref_sat_q7((q15_t)in);
	}
}
