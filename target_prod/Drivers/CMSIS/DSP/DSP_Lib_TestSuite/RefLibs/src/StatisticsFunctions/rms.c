#include "ref.h"

void ref_rms_f32(
  float32_t * pSrc,
  uint32_t blockSize,
  float32_t * pResult)
{
	uint32_t i;
	float32_t sumsq=0;
	
	for(i=0;i<blockSize;i++)
	{
			sumsq += pSrc[i] * pSrc[i];
	}
	*pResult = sqrtf(sumsq / (float32_t)blockSize);
}

void ref_rms_q31(
  q31_t * pSrc,
  uint32_t blockSize,
  q31_t * pResult)
{
	uint32_t i;
	q63_t sumsq=0;
	q63_t tmp1;
	q31_t tmp2;
  
  float help_float;
  
	for(i=0;i<blockSize;i++)
	{
			sumsq += (q63_t)pSrc[i] * pSrc[i];
	}
	tmp1 = (sumsq / (q63_t)blockSize) >> 31;
	tmp2 = ref_sat_q31(tmp1);

  /* GCC M0 problem: __aeabi_f2iz(QNAN) returns not 0 */
  help_float = (sqrtf((float)tmp2 / 2147483648.0f) * 2147483648.0f);
  /* Checking for a NAN value in help_float */
  if (((*((int *)(&help_float))) & 0x7FC00000) == 0x7FC00000) {
      help_float = 0;
  }
  *pResult = (q31_t)(help_float);

//  *pResult = (q31_t)(sqrtf((float)tmp2 / 2147483648.0f) * 2147483648.0f);
}

void ref_rms_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q15_t * pResult)
{
	uint32_t i;
	q63_t sumsq=0;
	q31_t tmp1;
	q15_t tmp2;
	
	for(i=0;i<blockSize;i++)
	{
			sumsq += (q63_t)pSrc[i] * pSrc[i];
	}
	tmp1 = (sumsq / (q63_t)blockSize) >> 15;
	tmp2 = ref_sat_q15(tmp1);
	*pResult = (q15_t)(sqrtf((float)tmp2 / 32768.0f) * 32768.0f);
}
