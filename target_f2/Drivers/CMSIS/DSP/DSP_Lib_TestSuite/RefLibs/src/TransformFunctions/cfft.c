#include "ref.h"
#include "arm_const_structs.h"
	 
void ref_cfft_f32(
   const arm_cfft_instance_f32 * S, 
   float32_t * p1,
   uint8_t ifftFlag,
   uint8_t bitReverseFlag)
{
	int n, mmax, m, j, istep, i;
	float32_t wtemp, wr, wpr, wpi, wi, theta;
	float32_t tempr, tempi;
	float32_t *  data = p1;
	uint32_t N = S->fftLen;
	int32_t dir = (ifftFlag) ? -1 : 1;

	// decrement pointer since the original version used fortran style indexing.
	data--;

	n = N << 1;
	j = 1;
	for (i = 1; i < n; i += 2) {
		if (j > i) {
			tempr = data[j];     data[j] = data[i];     data[i] = tempr;
			tempr = data[j+1]; data[j+1] = data[i+1]; data[i+1] = tempr;
		}
		m = n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax = 2;
	while (n > mmax) {
		istep = 2*mmax;
		theta = -6.283185307179586f/(dir*mmax);
		wtemp = sinf(0.5f*theta);
		wpr = -2.0f*wtemp*wtemp;
		wpi = sinf(theta);
		wr = 1.0f;
		wi = 0.0f;
		for (m = 1; m < mmax; m += 2) {
			for (i = m; i <= n; i += istep) {
				j =i + mmax;
				tempr = wr*data[j]   - wi*data[j+1];
				tempi = wr*data[j+1] + wi*data[j];
				data[j]   = data[i]   - tempr;
				data[j+1] = data[i+1] - tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr = (wtemp = wr)*wpr - wi*wpi + wr;
			wi = wi*wpr + wtemp*wpi + wi;
		}
		mmax = istep;
	}

	// Inverse transform is scaled by 1/N
	if (ifftFlag)
	{
		data++;
		for(i = 0; i<2*N; i++)
		{
			data[i] /= N;
		}
	}
}

void ref_cfft_q31(
	const arm_cfft_instance_q31 * S,
    q31_t * p1,
    uint8_t ifftFlag,
    uint8_t bitReverseFlag)
{
	uint32_t i;
	float32_t *fSrc = (float32_t*)p1;
	
	for(i=0;i<S->fftLen*2;i++)
	{
		//read the q31 data, cast to float, scale down for float
		fSrc[i] = (float32_t)p1[i] / 2147483648.0f;
	}
	
	switch(S->fftLen)
	{
   case 16: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len16, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 32: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len32, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 64: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len64, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 256: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len256, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 1024: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len1024, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 4096: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len4096, fSrc, ifftFlag, bitReverseFlag);
		 break;
	}
	
	if (ifftFlag)
	{
		for(i=0;i<S->fftLen*2;i++)
		{
			//read the float data, scale up for q31, cast to q31
			p1[i] = (q31_t)( fSrc[i] * 2147483648.0f );
		}
	}
	else
	{
		for(i=0;i<S->fftLen*2;i++)
		{
			//read the float data, scale up for q31, cast to q31
			p1[i] = (q31_t)( fSrc[i] * 2147483648.0f / (float32_t)S->fftLen);
		}
	}
}

void ref_cfft_q15(
	const arm_cfft_instance_q15 * S,
    q15_t * pSrc,
    uint8_t ifftFlag,
    uint8_t bitReverseFlag)
{
	uint32_t i;
	float32_t *fSrc = (float32_t*)pSrc;
	
	for(i=0;i<S->fftLen*2;i++)
	{
		//read the q15 data, cast to float, scale down for float, place in temporary buffer
		scratchArray[i] = (float32_t)pSrc[i] / 32768.0f;
	}
	
	for(i=0;i<S->fftLen*2;i++)
	{
		//copy from temp buffer to final buffer
		fSrc[i] = scratchArray[i];
	}
	
	switch(S->fftLen)
	{
   case 16: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len16, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 32: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len32, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 64: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len64, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 256: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len256, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 1024: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len1024, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, fSrc, ifftFlag, bitReverseFlag);
		 break;
   
   case 4096: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len4096, fSrc, ifftFlag, bitReverseFlag);
		 break;
	}
	
	if (ifftFlag)
	{
		for(i=0;i<S->fftLen*2;i++)
		{
			//read the float data, scale up for q15, cast to q15
			pSrc[i] = (q15_t)( fSrc[i] * 32768.0f );
		}
	}
	else
	{
		for(i=0;i<S->fftLen*2;i++)
		{
			//read the float data, scale up for q15, cast to q15
			pSrc[i] = (q15_t)( fSrc[i] * 32768.0f / (float32_t)S->fftLen);
		}
	}
}

void ref_cfft_radix2_f32(
	const arm_cfft_radix2_instance_f32 * S,
	float32_t * pSrc)
{
	switch(S->fftLen)
	{
   case 16: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len16, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 32: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len32, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 64: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len64, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 256: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len256, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 1024: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len1024, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 4096: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len4096, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
	}
}

void ref_cfft_radix2_q31(
	const arm_cfft_radix2_instance_q31 * S,
	q31_t * pSrc)
{
	uint32_t i;
	float32_t *fSrc = (float32_t*)pSrc;
	
	for(i=0;i<S->fftLen*2;i++)
	{
		//read the q31 data, cast to float, scale down for float
		fSrc[i] = (float32_t)pSrc[i] / 2147483648.0f;
	}
	
	switch(S->fftLen)
	{
   case 16: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len16, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 32: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len32, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 64: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len64, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 256: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len256, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 1024: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len1024, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 4096: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len4096, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
	}
	
	if (S->ifftFlag)
	{
		for(i=0;i<S->fftLen*2;i++)
		{
			//read the float data, scale up for q31, cast to q31
			pSrc[i] = (q31_t)( fSrc[i] * 2147483648.0f );
		}
	}
	else
	{
		for(i=0;i<S->fftLen*2;i++)
		{
			//read the float data, scale up for q31, cast to q31
			pSrc[i] = (q31_t)( fSrc[i] * 2147483648.0f / (float32_t)S->fftLen);
		}
	}
}

void ref_cfft_radix2_q15(
	const arm_cfft_radix2_instance_q15 * S,
	q15_t * pSrc)
{
	uint32_t i;
	float32_t *fSrc = (float32_t*)pSrc;
	
	for(i=0;i<S->fftLen*2;i++)
	{
		//read the q15 data, cast to float, scale down for float, place in temporary buffer
		scratchArray[i] = (float32_t)pSrc[i] / 32768.0f;
	}
	
	for(i=0;i<S->fftLen*2;i++)
	{
		//copy from temp buffer to final buffer
		fSrc[i] = scratchArray[i];
	}
	
	switch(S->fftLen)
	{
   case 16: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len16, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 32: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len32, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 64: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len64, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 256: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len256, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 1024: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len1024, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 4096: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len4096, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
	}
	
	if (S->ifftFlag)
	{
		for(i=0;i<S->fftLen*2;i++)
		{
		//read the float data, scale up for q15, cast to q15
		pSrc[i] = (q15_t)( fSrc[i] * 32768.0f );
		}
	}
	else
	{
		for(i=0;i<S->fftLen*2;i++)
		{
		//read the float data, scale up for q15, cast to q15
		pSrc[i] = (q15_t)( fSrc[i] * 32768.0f / (float32_t)S->fftLen);
		}
	}
}

void ref_cfft_radix4_f32(
	const arm_cfft_radix4_instance_f32 * S,
	float32_t * pSrc)
{		
	switch(S->fftLen)
	{
   case 16: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len16, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 32: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len32, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 64: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len64, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 256: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len256, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 1024: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len1024, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 4096: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len4096, pSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
	}
}

void ref_cfft_radix4_q31(
	const arm_cfft_radix4_instance_q31 * S,
	q31_t * pSrc)
{
	uint32_t i;
	float32_t *fSrc = (float32_t*)pSrc;
	
	for(i=0;i<S->fftLen*2;i++)
	{
		//read the q31 data, cast to float, scale down for float
		fSrc[i] = (float32_t)pSrc[i] / 2147483648.0f;
	}
	
	switch(S->fftLen)
	{
   case 16: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len16, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 32: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len32, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 64: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len64, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 256: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len256, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 1024: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len1024, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 4096: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len4096, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
	}
	
	if (S->ifftFlag)
	{
		for(i=0;i<S->fftLen*2;i++)
		{
			//read the float data, scale up for q31, cast to q31
			pSrc[i] = (q31_t)( fSrc[i] * 2147483648.0f );
		}
	}
	else
	{
		for(i=0;i<S->fftLen*2;i++)
		{
			//read the float data, scale up for q31, cast to q31
			pSrc[i] = (q31_t)( fSrc[i] * 2147483648.0f / (float32_t)S->fftLen);
		}
	}
}

void ref_cfft_radix4_q15(
	const arm_cfft_radix4_instance_q15 * S,
	q15_t * pSrc)
{
	uint32_t i;
	float32_t *fSrc = (float32_t*)pSrc;
	
	for(i=0;i<S->fftLen*2;i++)
	{
		//read the q15 data, cast to float, scale down for float, place in temporary buffer
		scratchArray[i] = (float32_t)pSrc[i] / 32768.0f;
	}
	
	for(i=0;i<S->fftLen*2;i++)
	{
		//copy from temp buffer to final buffer
		fSrc[i] = scratchArray[i];
	}
	
	switch(S->fftLen)
	{
   case 16: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len16, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 32: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len32, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 64: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len64, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 128: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len128, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 256: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len256, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 512: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len512, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 1024: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len1024, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 2048: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len2048, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
   
   case 4096: 
		 ref_cfft_f32(&arm_cfft_sR_f32_len4096, fSrc, S->ifftFlag, S->bitReverseFlag);
		 break;
	}
	
	if (S->ifftFlag)
	{
		for(i=0;i<S->fftLen*2;i++)
		{
		//read the float data, scale up for q15, cast to q15
		pSrc[i] = (q15_t)( fSrc[i] * 32768.0f );
		}
	}
	else
	{
		for(i=0;i<S->fftLen*2;i++)
		{
		//read the float data, scale up for q15, cast to q15
		pSrc[i] = (q15_t)( fSrc[i] * 32768.0f / (float32_t)S->fftLen);
		}
	}
}
