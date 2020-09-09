#include "ref.h"

arm_status ref_mat_cmplx_mult_f32(
  const arm_matrix_instance_f32 * pSrcA,
  const arm_matrix_instance_f32 * pSrcB,
  arm_matrix_instance_f32 * pDst)
{
	uint32_t r,c,i,outR,outC,innerSize;
	float32_t sumR,sumI;
	float32_t a0,b0,c0,d0;
	
	outR = pSrcA->numRows;
	outC = pSrcB->numCols;
	innerSize = pSrcA->numCols;
	
	for(r=0;r<outR;r++)
	{
		for(c=0;c<outC;c++)
		{
			sumR = 0;
			sumI = 0;
			
			for(i=0;i<innerSize;i++)
			{
				a0 = pSrcA->pData[2*(r*innerSize + i) + 0];
				b0 = pSrcA->pData[2*(r*innerSize + i) + 1];
				c0 = pSrcB->pData[2*(i*outC + c) + 0]; 
				d0 = pSrcB->pData[2*(i*outC + c) + 1];	
				
				sumR += a0 * c0 - b0 * d0;
				sumI += b0 * c0 + a0 * d0;
			}
			
			pDst->pData[2*(r*outC + c) + 0] = sumR;
			pDst->pData[2*(r*outC + c) + 1] = sumI;
		}
	}
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_mat_cmplx_mult_q31(
  const arm_matrix_instance_q31 * pSrcA,
  const arm_matrix_instance_q31 * pSrcB,
  arm_matrix_instance_q31 * pDst)
{
	uint32_t r,c,i,outR,outC,innerSize;
	q63_t sumR,sumI;
	q31_t a0,b0,c0,d0;
	
	outR = pSrcA->numRows;
	outC = pSrcB->numCols;
	innerSize = pSrcA->numCols;
	
	for(r=0;r<outR;r++)
	{
		for(c=0;c<outC;c++)
		{
			sumR = 0;
			sumI = 0;
			
			for(i=0;i<innerSize;i++)
			{
				a0 = pSrcA->pData[2*(r*innerSize + i) + 0];
				b0 = pSrcA->pData[2*(r*innerSize + i) + 1];
				c0 = pSrcB->pData[2*(i*outC + c) + 0]; 
				d0 = pSrcB->pData[2*(i*outC + c) + 1];	
				
				sumR += (q63_t)a0 * c0 - (q63_t)b0 * d0;
				sumI += (q63_t)b0 * c0 + (q63_t)a0 * d0;
			}
			
			pDst->pData[2*(r*outC + c) + 0] = ref_sat_q31(sumR >> 31);
			pDst->pData[2*(r*outC + c) + 1] = ref_sat_q31(sumI >> 31);
		}
	}
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_mat_cmplx_mult_q15(
  const arm_matrix_instance_q15 * pSrcA,
  const arm_matrix_instance_q15 * pSrcB,
  arm_matrix_instance_q15 * pDst)
{
	uint32_t r,c,i,outR,outC,innerSize;
	q63_t sumR,sumI;
	q15_t a0,b0,c0,d0;
	
	outR = pSrcA->numRows;
	outC = pSrcB->numCols;
	innerSize = pSrcA->numCols;
	
	for(r=0;r<outR;r++)
	{
		for(c=0;c<outC;c++)
		{
			sumR = 0;
			sumI = 0;
			
			for(i=0;i<innerSize;i++)
			{
				a0 = pSrcA->pData[2*(r*innerSize + i) + 0];
				b0 = pSrcA->pData[2*(r*innerSize + i) + 1];
				c0 = pSrcB->pData[2*(i*outC + c) + 0]; 
				d0 = pSrcB->pData[2*(i*outC + c) + 1];	
				
				sumR += (q31_t)a0 * c0 - (q31_t)b0 * d0;
				sumI += (q31_t)b0 * c0 + (q31_t)a0 * d0;
			}
			
			pDst->pData[2*(r*outC + c) + 0] = ref_sat_q15(sumR >> 15);
			pDst->pData[2*(r*outC + c) + 1] = ref_sat_q15(sumI >> 15);
		}
	}
	
	return ARM_MATH_SUCCESS;
}
