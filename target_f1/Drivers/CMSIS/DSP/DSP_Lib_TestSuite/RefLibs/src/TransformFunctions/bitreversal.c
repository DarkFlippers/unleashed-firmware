#include "ref.h"


;/*    
;* @brief  In-place bit reversal function.   
;* @param[in, out] *pSrc        points to the in-place buffer of unknown 32-bit data type. 
;* @param[in]      bitRevLen    bit reversal table length
;* @param[in]      *pBitRevTab  points to bit reversal table.   
;* @return none.   
;*/
void arm_bitreversal_32(uint32_t *pSrc, uint32_t bitRevLen, uint32_t *pBitRevTab)
{
	uint32_t a,b,i,tmp;
	
	for(i=0; i<bitRevLen; i++) 
	{
		 a = pBitRevTab[2*i];
		 b = pBitRevTab[2*i + 1];

	//real
		 tmp = pSrc[a];
		 pSrc[a] = pSrc[b];
		 pSrc[b] = tmp;

	//complex
		 tmp = pSrc[a+1];
		 pSrc[a+1] = pSrc[b+1];
		 pSrc[b+1] = tmp;
	}
}
