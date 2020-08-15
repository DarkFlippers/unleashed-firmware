#include "ref.h"

void ref_sin_cos_f32(
  float32_t theta,
  float32_t * pSinVal,
  float32_t * pCosVal)
{
	//theta is given in degrees
	*pSinVal = sinf(theta * 6.28318530717959f / 360.0f);
	*pCosVal = cosf(theta * 6.28318530717959f / 360.0f);
}

void ref_sin_cos_q31(
  q31_t theta,
  q31_t * pSinVal,
  q31_t * pCosVal)
{
	//theta is given in the range [-1,1) to represent [-pi,pi)
	*pSinVal = (q31_t)(sinf((float32_t)theta * 3.14159265358979f / 2147483648.0f) * 2147483648.0f);
	*pCosVal = (q31_t)(cosf((float32_t)theta * 3.14159265358979f / 2147483648.0f) * 2147483648.0f);
}
