#include "ref.h"

void ref_scale_f32(
  float32_t * pSrc,
  float32_t scale,
  float32_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = pSrc[i] * scale;
	}
}

void ref_scale_q31(
  q31_t * pSrc,
  q31_t scaleFract,
  int8_t shift,
  q31_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
  int8_t kShift = shift + 1;                     /* Shift to apply after scaling */
  int8_t sign = (kShift & 0x80);
	q63_t temp;
	
	for(i=0;i<blockSize;i++)
	{
		temp = ((q63_t) pSrc[i] * scaleFract) >> 32;
		if (sign)
			pDst[i] = temp >> -kShift;
		else
			pDst[i] = ref_sat_q31( (q63_t)temp << kShift );
	}
}

void ref_scale_q15(
  q15_t * pSrc,
  q15_t scaleFract,
  int8_t shift,
  q15_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
  int8_t kShift = 15 - shift;                      /* Shift to apply after scaling */
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ref_sat_q15(((q31_t) pSrc[i] * scaleFract) >> kShift);
	}
}

void ref_scale_q7(
  q7_t * pSrc,
  q7_t scaleFract,
  int8_t shift,
  q7_t * pDst,
  uint32_t blockSize)
{
	uint32_t i;
  int8_t kShift = 7 - shift;                      /* Shift to apply after scaling */
	
	for(i=0;i<blockSize;i++)
	{
		pDst[i] = ref_sat_q7(((q15_t) pSrc[i] * scaleFract) >> kShift);
	}
}
