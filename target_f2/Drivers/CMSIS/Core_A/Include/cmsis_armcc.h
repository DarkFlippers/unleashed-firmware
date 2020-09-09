/**************************************************************************//**
 * @file     cmsis_armcc.h
 * @brief    CMSIS compiler specific macros, functions, instructions
 * @version  V1.0.2
 * @date     10. January 2018
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CMSIS_ARMCC_H
#define __CMSIS_ARMCC_H

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 400677)
  #error "Please use Arm Compiler Toolchain V4.0.677 or later!"
#endif

/* CMSIS compiler control architecture macros */
#if (defined (__TARGET_ARCH_7_A ) && (__TARGET_ARCH_7_A  == 1))
  #define __ARM_ARCH_7A__           1
#endif

/* CMSIS compiler specific defines */
#ifndef   __ASM
  #define __ASM                                  __asm
#endif
#ifndef   __INLINE
  #define __INLINE                               __inline
#endif
#ifndef   __FORCEINLINE
  #define __FORCEINLINE                          __forceinline
#endif
#ifndef   __STATIC_INLINE
  #define __STATIC_INLINE                        static __inline
#endif
#ifndef   __STATIC_FORCEINLINE
  #define __STATIC_FORCEINLINE                   static __forceinline
#endif
#ifndef   __NO_RETURN
  #define __NO_RETURN                            __declspec(noreturn)
#endif
#ifndef   CMSIS_DEPRECATED
  #define CMSIS_DEPRECATED                       __attribute__((deprecated))
#endif
#ifndef   __USED
  #define __USED                                 __attribute__((used))
#endif
#ifndef   __WEAK
  #define __WEAK                                 __attribute__((weak))
#endif
#ifndef   __PACKED
  #define __PACKED                               __attribute__((packed))
#endif
#ifndef   __PACKED_STRUCT
  #define __PACKED_STRUCT                        __packed struct
#endif
#ifndef   __UNALIGNED_UINT16_WRITE
  #define __UNALIGNED_UINT16_WRITE(addr, val)    ((*((__packed uint16_t *)(addr))) = (val))
#endif
#ifndef   __UNALIGNED_UINT16_READ
  #define __UNALIGNED_UINT16_READ(addr)          (*((const __packed uint16_t *)(addr)))
#endif
#ifndef   __UNALIGNED_UINT32_WRITE
  #define __UNALIGNED_UINT32_WRITE(addr, val)    ((*((__packed uint32_t *)(addr))) = (val))
#endif
#ifndef   __UNALIGNED_UINT32_READ
  #define __UNALIGNED_UINT32_READ(addr)          (*((const __packed uint32_t *)(addr)))
#endif
#ifndef   __ALIGNED
  #define __ALIGNED(x)                           __attribute__((aligned(x)))
#endif
#ifndef   __PACKED
  #define __PACKED                               __attribute__((packed))
#endif

/* ##########################  Core Instruction Access  ######################### */
/**
  \brief   No Operation
 */
#define __NOP                             __nop

/**
  \brief   Wait For Interrupt
 */
#define __WFI                             __wfi

/**
  \brief   Wait For Event
 */
#define __WFE                             __wfe

/**
  \brief   Send Event
 */
#define __SEV                             __sev

/**
  \brief   Instruction Synchronization Barrier
 */
#define __ISB() do {\
                   __schedule_barrier();\
                   __isb(0xF);\
                   __schedule_barrier();\
                } while (0U)

/**
  \brief   Data Synchronization Barrier
 */
#define __DSB() do {\
                   __schedule_barrier();\
                   __dsb(0xF);\
                   __schedule_barrier();\
                } while (0U)

/**
  \brief   Data Memory Barrier
 */
#define __DMB() do {\
                   __schedule_barrier();\
                   __dmb(0xF);\
                   __schedule_barrier();\
                } while (0U)

/**
  \brief   Reverse byte order (32 bit)
  \details Reverses the byte order in unsigned integer value. For example, 0x12345678 becomes 0x78563412.
  \param [in]    value  Value to reverse
  \return               Reversed value
 */
#define __REV                             __rev

/**
  \brief   Reverse byte order (16 bit)
  \details Reverses the byte order within each halfword of a word. For example, 0x12345678 becomes 0x34127856.
  \param [in]    value  Value to reverse
  \return               Reversed value
 */
#ifndef __NO_EMBEDDED_ASM
__attribute__((section(".rev16_text"))) __STATIC_INLINE __ASM uint32_t __REV16(uint32_t value)
{
  rev16 r0, r0
  bx lr
}
#endif

/**
  \brief   Reverse byte order (16 bit)
  \details Reverses the byte order in a 16-bit value and returns the signed 16-bit result. For example, 0x0080 becomes 0x8000.
  \param [in]    value  Value to reverse
  \return               Reversed value
 */
#ifndef __NO_EMBEDDED_ASM
__attribute__((section(".revsh_text"))) __STATIC_INLINE __ASM int16_t __REVSH(int16_t value)
{
  revsh r0, r0
  bx lr
}
#endif

/**
  \brief   Rotate Right in unsigned value (32 bit)
  \param [in]    op1  Value to rotate
  \param [in]    op2  Number of Bits to rotate
  \return               Rotated value
 */
#define __ROR                             __ror

/**
  \brief   Breakpoint
  \param [in]    value  is ignored by the processor.
                 If required, a debugger can use it to store additional information about the breakpoint.
 */
#define __BKPT(value)                     __breakpoint(value)

/**
  \brief   Reverse bit order of value
  \param [in]    value  Value to reverse
  \return               Reversed value
 */
#define __RBIT                            __rbit

/**
  \brief   Count leading zeros
  \param [in]  value  Value to count the leading zeros
  \return             number of leading zeros in value
 */
#define __CLZ                             __clz

/**
  \brief   LDR Exclusive (8 bit)
  \details Executes a exclusive LDR instruction for 8 bit value.
  \param [in]    ptr  Pointer to data
  \return             value of type uint8_t at (*ptr)
 */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 5060020)
  #define __LDREXB(ptr)                                                        ((uint8_t ) __ldrex(ptr))
#else
  #define __LDREXB(ptr)          _Pragma("push") _Pragma("diag_suppress 3731") ((uint8_t ) __ldrex(ptr))  _Pragma("pop")
#endif

/**
  \brief   LDR Exclusive (16 bit)
  \details Executes a exclusive LDR instruction for 16 bit values.
  \param [in]    ptr  Pointer to data
  \return        value of type uint16_t at (*ptr)
 */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 5060020)
  #define __LDREXH(ptr)                                                        ((uint16_t) __ldrex(ptr))
#else
  #define __LDREXH(ptr)          _Pragma("push") _Pragma("diag_suppress 3731") ((uint16_t) __ldrex(ptr))  _Pragma("pop")
#endif

/**
  \brief   LDR Exclusive (32 bit)
  \details Executes a exclusive LDR instruction for 32 bit values.
  \param [in]    ptr  Pointer to data
  \return        value of type uint32_t at (*ptr)
 */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 5060020)
  #define __LDREXW(ptr)                                                        ((uint32_t ) __ldrex(ptr))
#else
  #define __LDREXW(ptr)          _Pragma("push") _Pragma("diag_suppress 3731") ((uint32_t ) __ldrex(ptr))  _Pragma("pop")
#endif

/**
  \brief   STR Exclusive (8 bit)
  \details Executes a exclusive STR instruction for 8 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
  \return          0  Function succeeded
  \return          1  Function failed
 */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 5060020)
  #define __STREXB(value, ptr)                                                 __strex(value, ptr)
#else
  #define __STREXB(value, ptr)   _Pragma("push") _Pragma("diag_suppress 3731") __strex(value, ptr)        _Pragma("pop")
#endif

/**
  \brief   STR Exclusive (16 bit)
  \details Executes a exclusive STR instruction for 16 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
  \return          0  Function succeeded
  \return          1  Function failed
 */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 5060020)
  #define __STREXH(value, ptr)                                                 __strex(value, ptr)
#else
  #define __STREXH(value, ptr)   _Pragma("push") _Pragma("diag_suppress 3731") __strex(value, ptr)        _Pragma("pop")
#endif

/**
  \brief   STR Exclusive (32 bit)
  \details Executes a exclusive STR instruction for 32 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
  \return          0  Function succeeded
  \return          1  Function failed
 */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 5060020)
  #define __STREXW(value, ptr)                                                 __strex(value, ptr)
#else
  #define __STREXW(value, ptr)   _Pragma("push") _Pragma("diag_suppress 3731") __strex(value, ptr)        _Pragma("pop")
#endif

/**
  \brief   Remove the exclusive lock
  \details Removes the exclusive lock which is created by LDREX.
 */
#define __CLREX                           __clrex


/**
  \brief   Signed Saturate
  \details Saturates a signed value.
  \param [in]  value  Value to be saturated
  \param [in]    sat  Bit position to saturate to (1..32)
  \return             Saturated value
 */
#define __SSAT                            __ssat

/**
  \brief   Unsigned Saturate
  \details Saturates an unsigned value.
  \param [in]  value  Value to be saturated
  \param [in]    sat  Bit position to saturate to (0..31)
  \return             Saturated value
 */
#define __USAT                            __usat

/* ###########################  Core Function Access  ########################### */

/**
  \brief   Get FPSCR (Floating Point Status/Control)
  \return               Floating Point Status/Control register value
 */
__STATIC_INLINE uint32_t __get_FPSCR(void)
{
#if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined (__FPU_USED   ) && (__FPU_USED    == 1U))     )
  register uint32_t __regfpscr         __ASM("fpscr");
  return(__regfpscr);
#else
   return(0U);
#endif
}

/**
  \brief   Set FPSCR (Floating Point Status/Control)
  \param [in]    fpscr  Floating Point Status/Control value to set
 */
__STATIC_INLINE void __set_FPSCR(uint32_t fpscr)
{
#if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined (__FPU_USED   ) && (__FPU_USED    == 1U))     )
  register uint32_t __regfpscr         __ASM("fpscr");
  __regfpscr = (fpscr);
#else
  (void)fpscr;
#endif
}

/** \brief  Get CPSR (Current Program Status Register)
    \return               CPSR Register value
 */
__STATIC_INLINE uint32_t __get_CPSR(void)
{
  register uint32_t __regCPSR          __ASM("cpsr");
  return(__regCPSR);
}


/** \brief  Set CPSR (Current Program Status Register)
    \param [in]    cpsr  CPSR value to set
 */
__STATIC_INLINE void __set_CPSR(uint32_t cpsr)
{
  register uint32_t __regCPSR          __ASM("cpsr");
  __regCPSR = cpsr;
}

/** \brief  Get Mode
    \return                Processor Mode
 */
__STATIC_INLINE uint32_t __get_mode(void)
{
  return (__get_CPSR() & 0x1FU);
}

/** \brief  Set Mode
    \param [in]    mode  Mode value to set
 */
__STATIC_INLINE __ASM void __set_mode(uint32_t mode)
{
  MOV  r1, lr
  MSR  CPSR_C, r0
  BX   r1
}

/** \brief  Get Stack Pointer
    \return Stack Pointer
 */
__STATIC_INLINE __ASM uint32_t __get_SP(void)
{
  MOV  r0, sp
  BX   lr
}

/** \brief  Set Stack Pointer
    \param [in]    stack  Stack Pointer value to set
 */
__STATIC_INLINE __ASM void __set_SP(uint32_t stack)
{
  MOV  sp, r0
  BX   lr
}


/** \brief  Get USR/SYS Stack Pointer
    \return USR/SYSStack Pointer
 */
__STATIC_INLINE __ASM uint32_t __get_SP_usr(void)
{
  ARM
  PRESERVE8

  MRS     R1, CPSR
  CPS     #0x1F       ;no effect in USR mode
  MOV     R0, SP
  MSR     CPSR_c, R1  ;no effect in USR mode
  ISB
  BX      LR
}

/** \brief  Set USR/SYS Stack Pointer
    \param [in]    topOfProcStack  USR/SYS Stack Pointer value to set
 */
__STATIC_INLINE __ASM void __set_SP_usr(uint32_t topOfProcStack)
{
  ARM
  PRESERVE8

  MRS     R1, CPSR
  CPS     #0x1F       ;no effect in USR mode
  MOV     SP, R0
  MSR     CPSR_c, R1  ;no effect in USR mode
  ISB
  BX      LR
}

/** \brief  Get FPEXC (Floating Point Exception Control Register)
    \return               Floating Point Exception Control Register value
 */
__STATIC_INLINE uint32_t __get_FPEXC(void)
{
#if (__FPU_PRESENT == 1)
  register uint32_t __regfpexc         __ASM("fpexc");
  return(__regfpexc);
#else
  return(0);
#endif
}

/** \brief  Set FPEXC (Floating Point Exception Control Register)
    \param [in]    fpexc  Floating Point Exception Control value to set
 */
__STATIC_INLINE void __set_FPEXC(uint32_t fpexc)
{
#if (__FPU_PRESENT == 1)
  register uint32_t __regfpexc         __ASM("fpexc");
  __regfpexc = (fpexc);
#endif
}

/*
 * Include common core functions to access Coprocessor 15 registers
 */

#define __get_CP(cp, op1, Rt, CRn, CRm, op2) do { register volatile uint32_t tmp __ASM("cp" # cp ":" # op1 ":c" # CRn ":c" # CRm ":" # op2); (Rt) = tmp; } while(0)
#define __set_CP(cp, op1, Rt, CRn, CRm, op2) do { register volatile uint32_t tmp __ASM("cp" # cp ":" # op1 ":c" # CRn ":c" # CRm ":" # op2); tmp = (Rt); } while(0)
#define __get_CP64(cp, op1, Rt, CRm) \
  do { \
    uint32_t ltmp, htmp; \
    __ASM volatile("MRRC p" # cp ", " # op1 ", ltmp, htmp, c" # CRm); \
    (Rt) = ((((uint64_t)htmp) << 32U) | ((uint64_t)ltmp)); \
  } while(0)

#define __set_CP64(cp, op1, Rt, CRm) \
  do { \
    const uint64_t tmp = (Rt); \
    const uint32_t ltmp = (uint32_t)(tmp); \
    const uint32_t htmp = (uint32_t)(tmp >> 32U); \
    __ASM volatile("MCRR p" # cp ", " # op1 ", ltmp, htmp, c" # CRm); \
  } while(0)

#include "cmsis_cp15.h"

/** \brief  Enable Floating Point Unit

  Critical section, called from undef handler, so systick is disabled
 */
__STATIC_INLINE __ASM void __FPU_Enable(void)
{
        ARM

        //Permit access to VFP/NEON, registers by modifying CPACR
        MRC     p15,0,R1,c1,c0,2
        ORR     R1,R1,#0x00F00000
        MCR     p15,0,R1,c1,c0,2

        //Ensure that subsequent instructions occur in the context of VFP/NEON access permitted
        ISB

        //Enable VFP/NEON
        VMRS    R1,FPEXC
        ORR     R1,R1,#0x40000000
        VMSR    FPEXC,R1

        //Initialise VFP/NEON registers to 0
        MOV     R2,#0

        //Initialise D16 registers to 0
        VMOV    D0, R2,R2
        VMOV    D1, R2,R2
        VMOV    D2, R2,R2
        VMOV    D3, R2,R2
        VMOV    D4, R2,R2
        VMOV    D5, R2,R2
        VMOV    D6, R2,R2
        VMOV    D7, R2,R2
        VMOV    D8, R2,R2
        VMOV    D9, R2,R2
        VMOV    D10,R2,R2
        VMOV    D11,R2,R2
        VMOV    D12,R2,R2
        VMOV    D13,R2,R2
        VMOV    D14,R2,R2
        VMOV    D15,R2,R2

  IF {TARGET_FEATURE_EXTENSION_REGISTER_COUNT} == 32
        //Initialise D32 registers to 0
        VMOV    D16,R2,R2
        VMOV    D17,R2,R2
        VMOV    D18,R2,R2
        VMOV    D19,R2,R2
        VMOV    D20,R2,R2
        VMOV    D21,R2,R2
        VMOV    D22,R2,R2
        VMOV    D23,R2,R2
        VMOV    D24,R2,R2
        VMOV    D25,R2,R2
        VMOV    D26,R2,R2
        VMOV    D27,R2,R2
        VMOV    D28,R2,R2
        VMOV    D29,R2,R2
        VMOV    D30,R2,R2
        VMOV    D31,R2,R2
  ENDIF

        //Initialise FPSCR to a known state
        VMRS    R2,FPSCR
        LDR     R3,=0x00086060 //Mask off all bits that do not have to be preserved. Non-preserved bits can/should be zero.
        AND     R2,R2,R3
        VMSR    FPSCR,R2

        BX      LR
}

#endif /* __CMSIS_ARMCC_H */
