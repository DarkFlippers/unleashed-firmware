#include "ref.h"

arm_status ref_mat_mult_f32(
  const arm_matrix_instance_f32 * pSrcA,
  const arm_matrix_instance_f32 * pSrcB,
  arm_matrix_instance_f32 * pDst)
{
	uint32_t r,c,i,outR,outC,innerSize;
	float32_t sum;
	
	outR = pSrcA->numRows;
	outC = pSrcB->numCols;
	innerSize = pSrcA->numCols;
	
	for(r=0;r<outR;r++)
	{
		for(c=0;c<outC;c++)
		{
			sum = 0;
			
			for(i=0;i<innerSize;i++)
			{
				sum += pSrcA->pData[r*innerSize + i] * pSrcB->pData[i*outC + c];
			}
			
			pDst->pData[r*outC + c] = sum;
		}
	}
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_mat_mult_q31(
  const arm_matrix_instance_q31 * pSrcA,
  const arm_matrix_instance_q31 * pSrcB,
  arm_matrix_instance_q31 * pDst)
{
	uint32_t r,c,i,outR,outC,innerSize;
	q63_t sum;
	
	outR = pSrcA->numRows;
	outC = pSrcB->numCols;
	innerSize = pSrcA->numCols;
	
	for(r=0;r<outR;r++)
	{
		for(c=0;c<outC;c++)
		{
			sum = 0;
			
			for(i=0;i<innerSize;i++)
			{
				sum += (q63_t)(pSrcA->pData[r*innerSize + i]) * pSrcB->pData[i*outC + c];
			}
			
			pDst->pData[r*outC + c] = ref_sat_q31(sum >> 31);
		}
	}
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_mat_mult_q15(
  const arm_matrix_instance_q15 * pSrcA,
  const arm_matrix_instance_q15 * pSrcB,
  arm_matrix_instance_q15 * pDst)
{
	uint32_t r,c,i,outR,outC,innerSize;
	q63_t sum;
	
	outR = pSrcA->numRows;
	outC = pSrcB->numCols;
	innerSize = pSrcA->numCols;
	
	for(r=0;r<outR;r++)
	{
		for(c=0;c<outC;c++)
		{
			sum = 0;
			
			for(i=0;i<innerSize;i++)
			{
				sum += (q31_t)(pSrcA->pData[r*innerSize + i]) * pSrcB->pData[i*outC + c];
			}
			
			pDst->pData[r*outC + c] = ref_sat_q15(sum >> 15);
		}
	}
	
	return ARM_MATH_SUCCESS;
}
