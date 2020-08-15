#include "ref.h"

void ref_dot_prod_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  uint32_t blockSize,
  float32_t * result)
{
	uint32_t i;
  float32_t sum = 0.0f;
	
	for(i=0;i<blockSize;i++)
	{
		sum += pSrcA[i] * pSrcB[i];
	}
  *result = sum;
}

void ref_dot_prod_q31(
  q31_t * pSrcA,
  q31_t * pSrcB,
  uint32_t blockSize,
  q63_t * result)
{
	uint32_t i;
  q63_t sum = 0.0f;
	
	for(i=0;i<blockSize;i++)
	{
		sum += ((q63_t)pSrcA[i] * pSrcB[i]) >> 14; //16.48
	}
  *result = sum;
}

void ref_dot_prod_q15(
  q15_t * pSrcA,
  q15_t * pSrcB,
  uint32_t blockSize,
  q63_t * result)
{
	uint32_t i;
  q63_t sum = 0.0f;
	
	for(i=0;i<blockSize;i++)
	{
		sum += (q31_t)pSrcA[i] * pSrcB[i]; //34.30
	}
  *result = sum;
}

void ref_dot_prod_q7(
  q7_t * pSrcA,
  q7_t * pSrcB,
  uint32_t blockSize,
  q31_t * result)
{
	uint32_t i;
  q31_t sum = 0.0f;
	
	for(i=0;i<blockSize;i++)
	{
		sum += (q31_t)pSrcA[i] * pSrcB[i]; //18.14
	}
  *result = sum;
}
