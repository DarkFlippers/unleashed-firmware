#include "ref.h"	
#include "arm_const_structs.h"
	
void ref_rfft_f32(
	arm_rfft_instance_f32 * S,
  float32_t * pSrc,
  float32_t * pDst)
{
	uint32_t i;
	
	if (S->ifftFlagR)
	{
		for(i=0;i<S->fftLenReal*2;i++)
		{
			pDst[i] = pSrc[i];
		}
	}
	else
	{
		for(i=0;i<S->fftLenReal;i++)
		{
			pDst[2*i+0] = pSrc[i];
			pDst[2*i+1] = 0.0f;
		}
	}
	
	switch(S->fftLenReal)
	{   
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, pDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
   
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, pDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, pDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
   
   case 8192: 
		 ref_cfft_f32(&ref_cfft_sR_f32_len8192, pDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	}
	
	if (S->ifftFlagR)
	{
		//throw away the imaginary part which should be all zeros
		for(i=0;i<S->fftLenReal;i++)
		{
			pDst[i] = pDst[2*i];
		}
	}
}
	
void ref_rfft_fast_f32(
	arm_rfft_fast_instance_f32 * S,
	float32_t * p, float32_t * pOut,
	uint8_t ifftFlag)
{
	uint32_t i,j;
	
	if (ifftFlag)
	{
		for(i=0;i<S->fftLenRFFT;i++)
		{
			pOut[i] = p[i];
		}
		//unpack first sample's complex part into middle sample's real part
		pOut[S->fftLenRFFT] = pOut[1];
		pOut[S->fftLenRFFT+1] = 0;
		pOut[1] = 0;
		j=4;
		for(i = S->fftLenRFFT / 2 + 1;i < S->fftLenRFFT;i++)
		{
			pOut[2*i+0] = p[2*i+0 - j];
			pOut[2*i+1] = -p[2*i+1 - j];
			j+=4;
		}
	}
	else
	{
		for(i=0;i<S->fftLenRFFT;i++)
		{
			pOut[2*i+0] = p[i];
			pOut[2*i+1] = 0.0f;
		}
	}
	
	switch(S->fftLenRFFT)
	{   
   case 32: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len32, pOut, ifftFlag, 1);
		 break;
   
   case 64: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len64, pOut, ifftFlag, 1);
		 break;
   
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, pOut, ifftFlag, 1);
		 break;
   
   case 256: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len256, pOut, ifftFlag, 1);
		 break;
   
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, pOut, ifftFlag, 1);
		 break;
   
   case 1024: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len1024, pOut, ifftFlag, 1);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, pOut, ifftFlag, 1);
		 break;
   
   case 4096: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len4096, pOut, ifftFlag, 1);
		 break;
	}
	
	if (ifftFlag)
	{
		//throw away the imaginary part which should be all zeros
		for(i=0;i<S->fftLenRFFT;i++)
		{
			pOut[i] = pOut[2*i];
		}
	}
	else
	{
		//pack last sample's real part into first sample's complex part
		pOut[1] = pOut[S->fftLenRFFT];
	}
}
	
void ref_rfft_q31(
  const arm_rfft_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst)
{
	uint32_t i;
	float32_t *fDst = (float32_t*)pDst;
	
	if (S->ifftFlagR)
	{
		for(i=0;i<S->fftLenReal*2;i++)
		{
			fDst[i] = (float32_t)pSrc[i] / 2147483648.0f;
		}
	}
	else
	{
		for(i=0;i<S->fftLenReal;i++)
		{
			fDst[2*i+0] = (float32_t)pSrc[i] / 2147483648.0f;
			fDst[2*i+1] = 0.0f;
		}
	}
	
	switch(S->fftLenReal)
	{
   case 32: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len32, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	 
   case 64: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len64, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	 
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	 
   case 256: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len256, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	 
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	 
   case 1024: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len1024, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
   
   case 4096: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len4096, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
   
   case 8192: 
		 ref_cfft_f32(&ref_cfft_sR_f32_len8192, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	}
	
	if (S->ifftFlagR)
	{
		//throw away the imaginary part which should be all zeros		
		for(i=0;i<S->fftLenReal;i++)
		{
			//read the float data, scale up for q31, cast to q31
			pDst[i] = (q31_t)( fDst[2*i] * 2147483648.0f);
		}
	}
	else
	{
		for(i=0;i<S->fftLenReal;i++)
		{
			//read the float data, scale up for q31, cast to q31
			pDst[i] = (q31_t)( fDst[i] * 2147483648.0f / (float32_t)S->fftLenReal);
		}
	}
}

void ref_rfft_q15(
  const arm_rfft_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst)
{
	uint32_t i;
	float32_t *fDst = (float32_t*)pDst;
	
	
	if (S->ifftFlagR)
	{
		for(i=0;i<S->fftLenReal*2;i++)
		{
			fDst[i] = (float32_t)pSrc[i] / 32768.0f;
		}
	}
	else
	{
		for(i=0;i<S->fftLenReal;i++)
		{
			//read the q15 data, cast to float, scale down for float
			fDst[2*i+0] = (float32_t)pSrc[i] / 32768.0f;
			fDst[2*i+1] = 0.0f;
		}
	}
	
	switch(S->fftLenReal)
	{   
   case 32: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len32, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	 
   case 64: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len64, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	 
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	 
   case 256: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len256, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	 
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	 
   case 1024: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len1024, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
   
   case 4096: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len4096, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
   
   case 8192: 
		 ref_cfft_f32(&ref_cfft_sR_f32_len8192, fDst, S->ifftFlagR, S->bitReverseFlagR);
		 break;
	}
	
	if (S->ifftFlagR)
	{
		//throw away the imaginary part which should be all zeros
		for(i=0;i<S->fftLenReal;i++)
		{
			pDst[i] = (q15_t)( fDst[2*i] * 32768.0f);
		}
	}
	else
	{
		for(i=0;i<S->fftLenReal;i++)
		{
			pDst[i] = (q15_t)( fDst[i] * 32768.0f / (float32_t)S->fftLenReal);
		}
	}
}
