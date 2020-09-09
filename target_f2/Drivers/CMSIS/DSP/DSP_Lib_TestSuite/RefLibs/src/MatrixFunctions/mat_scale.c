#include "ref.h"

arm_status ref_mat_scale_f32(
  const arm_matrix_instance_f32 * pSrc,
  float32_t scale,
  arm_matrix_instance_f32 * pDst)
{
	uint32_t i;
  uint32_t numSamples;                           /* total number of elements in the matrix  */

	/* Total number of samples in the input matrix */
	numSamples = (uint32_t) pSrc->numRows * pSrc->numCols;
	
	for(i=0;i<numSamples;i++)
	{
		pDst->pData[i] = pSrc->pData[i] * scale;
	}
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_mat_scale_q31(
  const arm_matrix_instance_q31 * pSrc,
  q31_t scale,
  int32_t shift,
  arm_matrix_instance_q31 * pDst)
{
	uint32_t i;
  uint32_t numSamples;                           /* total number of elements in the matrix  */
	int32_t totShift = shift + 1; 
	q31_t tmp;

	/* Total number of samples in the input matrix */
	numSamples = (uint32_t) pSrc->numRows * pSrc->numCols;
	
	for(i=0;i<numSamples;i++)
	{
		tmp = ((q63_t)pSrc->pData[i] * scale) >> 32;
		pDst->pData[i] = ref_sat_q31((q63_t)tmp << totShift );
	}
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_mat_scale_q15(
  const arm_matrix_instance_q15 * pSrc,
  q15_t scale,
  int32_t shift,
  arm_matrix_instance_q15 * pDst)
{
	uint32_t i;
  uint32_t numSamples;                           /* total number of elements in the matrix  */
	int32_t totShift = 15 - shift;

	/* Total number of samples in the input matrix */
	numSamples = (uint32_t) pSrc->numRows * pSrc->numCols;
	
	for(i=0;i<numSamples;i++)
	{
		pDst->pData[i] = ref_sat_q15( ((q31_t)pSrc->pData[i] * scale) >> totShift);
	}
	
	return ARM_MATH_SUCCESS;
}
