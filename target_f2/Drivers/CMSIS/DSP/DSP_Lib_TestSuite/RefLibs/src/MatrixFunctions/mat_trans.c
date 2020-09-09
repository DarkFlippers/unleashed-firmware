#include "ref.h"

arm_status ref_mat_trans_f64(
  const arm_matrix_instance_f64 * pSrc,
  arm_matrix_instance_f64 * pDst)
{
	uint64_t r,c;
	uint64_t numR = pSrc->numRows;
	uint64_t numC = pSrc->numCols;
	
	for(r=0;r<numR;r++)
	{
		for(c=0;c<numC;c++)
		{
			pDst->pData[c*numR + r] = pSrc->pData[r*numC + c];
		}
	}
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_mat_trans_f32(
  const arm_matrix_instance_f32 * pSrc,
  arm_matrix_instance_f32 * pDst)
{
	uint32_t r,c;
	uint32_t numR = pSrc->numRows;
	uint32_t numC = pSrc->numCols;
	
	for(r=0;r<numR;r++)
	{
		for(c=0;c<numC;c++)
		{
			pDst->pData[c*numR + r] = pSrc->pData[r*numC + c];
		}
	}
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_mat_trans_q31(
  const arm_matrix_instance_q31 * pSrc,
  arm_matrix_instance_q31 * pDst)
{
	uint32_t r,c;
	uint32_t numR = pSrc->numRows;
	uint32_t numC = pSrc->numCols;
	
	for(r=0;r<numR;r++)
	{
		for(c=0;c<numC;c++)
		{
			pDst->pData[c*numR + r] = pSrc->pData[r*numC + c];
		}
	}
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_mat_trans_q15(
  const arm_matrix_instance_q15 * pSrc,
  arm_matrix_instance_q15 * pDst)
{
	uint32_t r,c;
	uint32_t numR = pSrc->numRows;
	uint32_t numC = pSrc->numCols;
	
	for(r=0;r<numR;r++)
	{
		for(c=0;c<numC;c++)
		{
			pDst->pData[c*numR + r] = pSrc->pData[r*numC + c];
		}
	}
	
	return ARM_MATH_SUCCESS;
}
