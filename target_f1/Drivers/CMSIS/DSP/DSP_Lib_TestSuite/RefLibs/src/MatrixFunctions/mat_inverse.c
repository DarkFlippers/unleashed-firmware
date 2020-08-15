#include "ref.h"

arm_status ref_mat_inverse_f32(
  const arm_matrix_instance_f32 * pSrc,
  arm_matrix_instance_f32 * pDst)
{
	float32_t det;
	uint32_t i, size;
	arm_matrix_instance_f32 tmp;
	
	tmp.numCols = pSrc->numCols;
	tmp.numRows = pSrc->numRows;
	tmp.pData = scratchArray;
	
	det = ref_detrm(pSrc->pData,scratchArray,pSrc->numCols);
	
	size = pSrc->numCols * pSrc->numCols;
	
	ref_cofact(pSrc->pData,scratchArray,scratchArray + size,pSrc->numCols);
	
	ref_mat_trans_f32(&tmp,pDst);	
	
	for(i=0;i<size;i++)
	{
		pDst->pData[i] /= det;
	}
	
	return ARM_MATH_SUCCESS;
}

arm_status ref_mat_inverse_f64(
  const arm_matrix_instance_f64 * pSrc,
  arm_matrix_instance_f64 * pDst)
{
	float64_t det;
	uint32_t i, size;
	arm_matrix_instance_f64 tmp;
	
	tmp.numCols = pSrc->numCols;
	tmp.numRows = pSrc->numRows;
	tmp.pData = (float64_t*)scratchArray;
	
	det = ref_detrm64(pSrc->pData,(float64_t*)scratchArray,pSrc->numCols);
	
	size = pSrc->numCols * pSrc->numCols;
	
	ref_cofact64(pSrc->pData,(float64_t*)scratchArray,(float64_t*)scratchArray + size,pSrc->numCols);
	
	ref_mat_trans_f64(&tmp,pDst);	
	
	for(i=0;i<size;i++)
	{
		pDst->pData[i] /= det;
	}
	
	return ARM_MATH_SUCCESS;
}
