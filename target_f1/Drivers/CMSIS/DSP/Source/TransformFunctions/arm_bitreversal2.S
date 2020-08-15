;/* ----------------------------------------------------------------------
; * Project:      CMSIS DSP Library
; * Title:        arm_bitreversal2.S
; * Description:  arm_bitreversal_32 function done in assembly for maximum speed.
; *               Called after doing an fft to reorder the output.
; *               The function is loop unrolled by 2. arm_bitreversal_16 as well.
; *
; * $Date:        27. January 2017
; * $Revision:    V.1.5.1
; *
; * Target Processor: Cortex-M cores
; * -------------------------------------------------------------------- */
;/*
; * Copyright (C) 2010-2017 ARM Limited or its affiliates. All rights reserved.
; *
; * SPDX-License-Identifier: Apache-2.0
; *
; * Licensed under the Apache License, Version 2.0 (the License); you may
; * not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; * www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an AS IS BASIS, WITHOUT
; * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; */

#if   defined ( __CC_ARM )     /* Keil */
    #define CODESECT AREA     ||.text||, CODE, READONLY, ALIGN=2
    #define LABEL
#elif defined ( __IASMARM__ )  /* IAR */
    #define CODESECT SECTION `.text`:CODE
    #define PROC
    #define LABEL
    #define ENDP
    #define EXPORT PUBLIC
#elif defined ( __CSMC__ )	   /* Cosmic */
	#define	CODESECT	switch .text
	#define THUMB
	#define EXPORT	xdef
	#define PROC	:
	#define LABEL	:
	#define ENDP
	#define arm_bitreversal_32 _arm_bitreversal_32
#elif defined ( __TI_ARM__ )   /* TI ARM */
    #define THUMB    .thumb
    #define CODESECT .text
    #define EXPORT   .global
    #define PROC     : .asmfunc
    #define LABEL    :
    #define ENDP     .endasmfunc
    #define END
#elif defined ( __GNUC__ )     /* GCC */
    #define THUMB .thumb
    #define CODESECT .section .text
    #define EXPORT .global
    #define PROC :
    #define LABEL :
    #define ENDP
    #define END

    .syntax unified
#endif

	CODESECT
	THUMB

;/*
;* @brief  In-place bit reversal function.
;* @param[in, out] *pSrc        points to the in-place buffer of unknown 32-bit data type.
;* @param[in]      bitRevLen    bit reversal table length
;* @param[in]      *pBitRevTab  points to bit reversal table.
;* @return none.
;*/
	EXPORT arm_bitreversal_32
	EXPORT arm_bitreversal_16

#if   defined ( __CC_ARM )     /* Keil */
#elif defined ( __IASMARM__ )  /* IAR */
#elif defined ( __CSMC__ )	   /* Cosmic */
#elif defined ( __TI_ARM__ )   /* TI ARM */
#elif defined ( __GNUC__ )     /* GCC */
	.type   arm_bitreversal_16, %function
	.type   arm_bitreversal_32, %function
#endif

#if defined(ARM_MATH_CM0) || defined(ARM_MATH_CM0PLUS) || defined(ARM_MATH_ARMV8MBL)

arm_bitreversal_32 PROC
	ADDS     r3,r1,#1
	PUSH     {r4-r6}
	ADDS     r1,r2,#0
	LSRS     r3,r3,#1
arm_bitreversal_32_0 LABEL
	LDRH     r2,[r1,#2]
	LDRH     r6,[r1,#0]
	ADD      r2,r0,r2
	ADD      r6,r0,r6
	LDR      r5,[r2,#0]
	LDR      r4,[r6,#0]
	STR      r5,[r6,#0]
	STR      r4,[r2,#0]
	LDR      r5,[r2,#4]
	LDR      r4,[r6,#4]
	STR      r5,[r6,#4]
	STR      r4,[r2,#4]
	ADDS     r1,r1,#4
	SUBS     r3,r3,#1
	BNE      arm_bitreversal_32_0
	POP      {r4-r6}
	BX       lr
	ENDP

arm_bitreversal_16 PROC
	ADDS     r3,r1,#1
	PUSH     {r4-r6}
	ADDS     r1,r2,#0
	LSRS     r3,r3,#1
arm_bitreversal_16_0 LABEL
	LDRH     r2,[r1,#2]
	LDRH     r6,[r1,#0]
    LSRS     r2,r2,#1
    LSRS     r6,r6,#1
	ADD      r2,r0,r2
	ADD      r6,r0,r6
	LDR      r5,[r2,#0]
	LDR      r4,[r6,#0]
	STR      r5,[r6,#0]
	STR      r4,[r2,#0]
	ADDS     r1,r1,#4
	SUBS     r3,r3,#1
	BNE      arm_bitreversal_16_0
	POP      {r4-r6}
	BX       lr
	ENDP

#else

arm_bitreversal_32 PROC
	ADDS     r3,r1,#1
	CMP      r3,#1
	IT       LS
	BXLS     lr
	PUSH     {r4-r9}
	ADDS     r1,r2,#2
	LSRS     r3,r3,#2
arm_bitreversal_32_0 LABEL       ;/* loop unrolled by 2 */
	LDRH     r8,[r1,#4]
	LDRH     r9,[r1,#2]
	LDRH     r2,[r1,#0]
	LDRH     r12,[r1,#-2]
	ADD      r8,r0,r8
	ADD      r9,r0,r9
	ADD      r2,r0,r2
	ADD      r12,r0,r12
	LDR      r7,[r9,#0]
	LDR      r6,[r8,#0]
	LDR      r5,[r2,#0]
	LDR      r4,[r12,#0]
	STR      r6,[r9,#0]
	STR      r7,[r8,#0]
	STR      r5,[r12,#0]
	STR      r4,[r2,#0]
	LDR      r7,[r9,#4]
	LDR      r6,[r8,#4]
	LDR      r5,[r2,#4]
	LDR      r4,[r12,#4]
	STR      r6,[r9,#4]
	STR      r7,[r8,#4]
	STR      r5,[r12,#4]
	STR      r4,[r2,#4]
	ADDS     r1,r1,#8
	SUBS     r3,r3,#1
	BNE      arm_bitreversal_32_0
	POP      {r4-r9}
	BX       lr
	ENDP

arm_bitreversal_16 PROC
	ADDS     r3,r1,#1
	CMP      r3,#1
	IT       LS
	BXLS     lr
	PUSH     {r4-r9}
	ADDS     r1,r2,#2
	LSRS     r3,r3,#2
arm_bitreversal_16_0 LABEL       ;/* loop unrolled by 2 */
	LDRH     r8,[r1,#4]
	LDRH     r9,[r1,#2]
	LDRH     r2,[r1,#0]
	LDRH     r12,[r1,#-2]
	ADD      r8,r0,r8,LSR #1
	ADD      r9,r0,r9,LSR #1
	ADD      r2,r0,r2,LSR #1
	ADD      r12,r0,r12,LSR #1
	LDR      r7,[r9,#0]
	LDR      r6,[r8,#0]
	LDR      r5,[r2,#0]
	LDR      r4,[r12,#0]
	STR      r6,[r9,#0]
	STR      r7,[r8,#0]
	STR      r5,[r12,#0]
	STR      r4,[r2,#0]
	ADDS     r1,r1,#8
	SUBS     r3,r3,#1
	BNE      arm_bitreversal_16_0
	POP      {r4-r9}
	BX       lr
	ENDP

#endif

	END
