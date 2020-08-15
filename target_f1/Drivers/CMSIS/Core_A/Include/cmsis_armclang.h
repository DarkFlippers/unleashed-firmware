/**************************************************************************//**
 * @file     cmsis_armclang.h
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

#ifndef __CMSIS_ARMCLANG_H
#define __CMSIS_ARMCLANG_H

#pragma clang system_header   /* treat file as system include file */

#ifndef __ARM_COMPAT_H
#include <arm_compat.h>    /* Compatibility header for Arm Compiler 5 intrinsics */
#endif

/* CMSIS compiler specific defines */
#ifndef   __ASM
  #define __ASM                                  __asm
#endif
#ifndef   __INLINE
  #define __INLINE                               __inline
#endif
#ifndef   __FORCEINLINE
  #define __FORCEINLINE                          __attribute__((always_inline))
#endif
#ifndef   __STATIC_INLINE
  #define __STATIC_INLINE                        static __inline
#endif
#ifndef   __STATIC_FORCEINLINE
  #define __STATIC_FORCEINLINE                   __attribute__((always_inline)) static __inline
#endif
#ifndef   __NO_RETURN
  #define __NO_RETURN                            __attribute__((__noreturn__))
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
  #define __PACKED                               __attribute__((packed, aligned(1)))
#endif
#ifndef   __PACKED_STRUCT
  #define __PACKED_STRUCT                        struct __attribute__((packed, aligned(1)))
#endif
#ifndef   __UNALIGNED_UINT16_WRITE
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wpacked"
/*lint -esym(9058, T_UINT16_WRITE)*/ /* disable MISRA 2012 Rule 2.4 for T_UINT16_WRITE */
  __PACKED_STRUCT T_UINT16_WRITE { uint16_t v; };
  #pragma clang diagnostic pop
  #define __UNALIGNED_UINT16_WRITE(addr, val)    (void)((((struct T_UINT16_WRITE *)(void *)(addr))->v) = (val))
#endif
#ifndef   __UNALIGNED_UINT16_READ
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wpacked"
/*lint -esym(9058, T_UINT16_READ)*/ /* disable MISRA 2012 Rule 2.4 for T_UINT16_READ */
  __PACKED_STRUCT T_UINT16_READ { uint16_t v; };
  #pragma clang diagnostic pop
  #define __UNALIGNED_UINT16_READ(addr)          (((const struct T_UINT16_READ *)(const void *)(addr))->v)
#endif
#ifndef   __UNALIGNED_UINT32_WRITE
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wpacked"
/*lint -esym(9058, T_UINT32_WRITE)*/ /* disable MISRA 2012 Rule 2.4 for T_UINT32_WRITE */
  __PACKED_STRUCT T_UINT32_WRITE { uint32_t v; };
  #pragma clang diagnostic pop
  #define __UNALIGNED_UINT32_WRITE(addr, val)    (void)((((struct T_UINT32_WRITE *)(void *)(addr))->v) = (val))
#endif
#ifndef   __UNALIGNED_UINT32_READ
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wpacked"
  __PACKED_STRUCT T_UINT32_READ { uint32_t v; };
  #pragma clang diagnostic pop
  #define __UNALIGNED_UINT32_READ(addr)          (((const struct T_UINT32_READ *)(const void *)(addr))->v)
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
#define __NOP                             __builtin_arm_nop

/**
  \brief   Wait For Interrupt
 */
#define __WFI                             __builtin_arm_wfi

/**
  \brief   Wait For Event
 */
#define __WFE                             __builtin_arm_wfe

/**
  \brief   Send Event
 */
#define __SEV                             __builtin_arm_sev

/**
  \brief   Instruction Synchronization Barrier
 */
#define __ISB() do {\
                   __schedule_barrier();\
                   __builtin_arm_isb(0xF);\
                   __schedule_barrier();\
                } while (0U)

/**
  \brief   Data Synchronization Barrier
 */
#define __DSB() do {\
                   __schedule_barrier();\
                   __builtin_arm_dsb(0xF);\
                   __schedule_barrier();\
                } while (0U)

/**
  \brief   Data Memory Barrier
 */
#define __DMB() do {\
                   __schedule_barrier();\
                   __builtin_arm_dmb(0xF);\
                   __schedule_barrier();\
                } while (0U)

/**
  \brief   Reverse byte order (32 bit)
  \details Reverses the byte order in unsigned integer value. For example, 0x12345678 becomes 0x78563412.
  \param [in]    value  Value to reverse
  \return               Reversed value
 */
#define __REV(value)   __builtin_bswap32(value)

/**
  \brief   Reverse byte order (16 bit)
  \details Reverses the byte order within each halfword of a word. For example, 0x12345678 becomes 0x34127856.
  \param [in]    value  Value to reverse
  \return               Reversed value
 */
#define __REV16(value) __ROR(__REV(value), 16)


/**
  \brief   Reverse byte order (16 bit)
  \details Reverses the byte order in a 16-bit value and returns the signed 16-bit result. For example, 0x0080 becomes 0x8000.
  \param [in]    value  Value to reverse
  \return               Reversed value
 */
#define __REVSH(value) (int16_t)__builtin_bswap16(value)


/**
  \brief   Rotate Right in unsigned value (32 bit)
  \details Rotate Right (immediate) provides the value of the contents of a register rotated by a variable number of bits.
  \param [in]    op1  Value to rotate
  \param [in]    op2  Number of Bits to rotate
  \return               Rotated value
 */
__STATIC_FORCEINLINE uint32_t __ROR(uint32_t op1, uint32_t op2)
{
  op2 %= 32U;
  if (op2 == 0U)
  {
    return op1;
  }
  return (op1 >> op2) | (op1 << (32U - op2));
}


/**
  \brief   Breakpoint
  \param [in]    value  is ignored by the processor.
                 If required, a debugger can use it to store additional information about the breakpoint.
 */
#define __BKPT(value)   __ASM volatile ("bkpt "#value)

/**
  \brief   Reverse bit order of value
  \param [in]    value  Value to reverse
  \return               Reversed value
 */
#define __RBIT          __builtin_arm_rbit

/**
  \brief   Count leading zeros
  \param [in]  value  Value to count the leading zeros
  \return             number of leading zeros in value
 */
#define __CLZ           (uint8_t)__builtin_clz

/**
  \brief   LDR Exclusive (8 bit)
  \details Executes a exclusive LDR instruction for 8 bit value.
  \param [in]    ptr  Pointer to data
  \return             value of type uint8_t at (*ptr)
 */
#define __LDREXB        (uint8_t)__builtin_arm_ldrex


/**
  \brief   LDR Exclusive (16 bit)
  \details Executes a exclusive LDR instruction for 16 bit values.
  \param [in]    ptr  Pointer to data
  \return        value of type uint16_t at (*ptr)
 */
#define __LDREXH        (uint16_t)__builtin_arm_ldrex

/**
  \brief   LDR Exclusive (32 bit)
  \details Executes a exclusive LDR instruction for 32 bit values.
  \param [in]    ptr  Pointer to data
  \return        value of type uint32_t at (*ptr)
 */
#define __LDREXW        (uint32_t)__builtin_arm_ldrex

/**
  \brief   STR Exclusive (8 bit)
  \details Executes a exclusive STR instruction for 8 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
  \return          0  Function succeeded
  \return          1  Function failed
 */
#define __STREXB        (uint32_t)__builtin_arm_strex

/**
  \brief   STR Exclusive (16 bit)
  \details Executes a exclusive STR instruction for 16 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
  \return          0  Function succeeded
  \return          1  Function failed
 */
#define __STREXH        (uint32_t)__builtin_arm_strex

/**
  \brief   STR Exclusive (32 bit)
  \details Executes a exclusive STR instruction for 32 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
  \return          0  Function succeeded
  \return          1  Function failed
 */
#define __STREXW        (uint32_t)__builtin_arm_strex

/**
  \brief   Remove the exclusive lock
  \details Removes the exclusive lock which is created by LDREX.
 */
#define __CLREX             __builtin_arm_clrex

/**
  \brief   Signed Saturate
  \details Saturates a signed value.
  \param [in]  value  Value to be saturated
  \param [in]    sat  Bit position to saturate to (1..32)
  \return             Saturated value
 */
#define __SSAT             __builtin_arm_ssat

/**
  \brief   Unsigned Saturate
  \details Saturates an unsigned value.
  \param [in]  value  Value to be saturated
  \param [in]    sat  Bit position to saturate to (0..31)
  \return             Saturated value
 */
#define __USAT             __builtin_arm_usat


/* ###########################  Core Function Access  ########################### */

/**
  \brief   Get FPSCR
  \details Returns the current value of the Floating Point Status/Control register.
  \return               Floating Point Status/Control register value
 */
#define __get_FPSCR      __builtin_arm_get_fpscr

/**
  \brief   Set FPSCR
  \details Assigns the given value to the Floating Point Status/Control register.
  \param [in]    fpscr  Floating Point Status/Control value to set
 */
#define __set_FPSCR      __builtin_arm_set_fpscr

/** \brief  Get CPSR Register
    \return               CPSR Register value
 */
__STATIC_FORCEINLINE uint32_t __get_CPSR(void)
{
  uint32_t result;
  __ASM volatile("MRS %0, cpsr" : "=r" (result) );
  return(result);
}

/** \brief  Set CPSR Register
    \param [in]    cpsr  CPSR value to set
 */
__STATIC_FORCEINLINE void __set_CPSR(uint32_t cpsr)
{
__ASM volatile ("MSR cpsr, %0" : : "r" (cpsr) : "memory");
}

/** \brief  Get Mode
    \return                Processor Mode
 */
__STATIC_FORCEINLINE uint32_t __get_mode(void)
{
	return (__get_CPSR() & 0x1FU);
}

/** \brief  Set Mode
    \param [in]    mode  Mode value to set
 */
__STATIC_FORCEINLINE void __set_mode(uint32_t mode)
{
  __ASM volatile("MSR  cpsr_c, %0" : : "r" (mode) : "memory");
}

/** \brief  Get Stack Pointer
    \return Stack Pointer value
 */
__STATIC_FORCEINLINE uint32_t __get_SP()
{
  uint32_t result;
  __ASM volatile("MOV  %0, sp" : "=r" (result) : : "memory");
  return result;
}

/** \brief  Set Stack Pointer
    \param [in]    stack  Stack Pointer value to set
 */
__STATIC_FORCEINLINE void __set_SP(uint32_t stack)
{
  __ASM volatile("MOV  sp, %0" : : "r" (stack) : "memory");
}

/** \brief  Get USR/SYS Stack Pointer
    \return USR/SYS Stack Pointer value
 */
__STATIC_FORCEINLINE uint32_t __get_SP_usr()
{
  uint32_t cpsr;
  uint32_t result;
  __ASM volatile(
    "MRS     %0, cpsr   \n"
    "CPS     #0x1F      \n" // no effect in USR mode
    "MOV     %1, sp     \n"
    "MSR     cpsr_c, %2 \n" // no effect in USR mode
    "ISB" :  "=r"(cpsr), "=r"(result) : "r"(cpsr) : "memory"
   );
  return result;
}

/** \brief  Set USR/SYS Stack Pointer
    \param [in]    topOfProcStack  USR/SYS Stack Pointer value to set
 */
__STATIC_FORCEINLINE void __set_SP_usr(uint32_t topOfProcStack)
{
  uint32_t cpsr;
  __ASM volatile(
    "MRS     %0, cpsr   \n"
    "CPS     #0x1F      \n" // no effect in USR mode
    "MOV     sp, %1     \n"
    "MSR     cpsr_c, %2 \n" // no effect in USR mode
    "ISB" : "=r"(cpsr) : "r" (topOfProcStack), "r"(cpsr) : "memory"
   );
}

/** \brief  Get FPEXC
    \return               Floating Point Exception Control register value
 */
__STATIC_FORCEINLINE uint32_t __get_FPEXC(void)
{
#if (__FPU_PRESENT == 1)
  uint32_t result;
  __ASM volatile("VMRS %0, fpexc" : "=r" (result) : : "memory");
  return(result);
#else
  return(0);
#endif
}

/** \brief  Set FPEXC
    \param [in]    fpexc  Floating Point Exception Control value to set
 */
__STATIC_FORCEINLINE void __set_FPEXC(uint32_t fpexc)
{
#if (__FPU_PRESENT == 1)
  __ASM volatile ("VMSR fpexc, %0" : : "r" (fpexc) : "memory");
#endif
}

/*
 * Include common core functions to access Coprocessor 15 registers
 */

#define __get_CP(cp, op1, Rt, CRn, CRm, op2) __ASM volatile("MRC p" # cp ", " # op1 ", %0, c" # CRn ", c" # CRm ", " # op2 : "=r" (Rt) : : "memory" )
#define __set_CP(cp, op1, Rt, CRn, CRm, op2) __ASM volatile("MCR p" # cp ", " # op1 ", %0, c" # CRn ", c" # CRm ", " # op2 : : "r" (Rt) : "memory" )
#define __get_CP64(cp, op1, Rt, CRm)         __ASM volatile("MRRC p" # cp ", " # op1 ", %Q0, %R0, c" # CRm  : "=r" (Rt) : : "memory" )
#define __set_CP64(cp, op1, Rt, CRm)         __ASM volatile("MCRR p" # cp ", " # op1 ", %Q0, %R0, c" # CRm  : : "r" (Rt) : "memory" )

#include "cmsis_cp15.h"

/** \brief  Enable Floating Point Unit

  Critical section, called from undef handler, so systick is disabled
 */
__STATIC_INLINE void __FPU_Enable(void)
{
  __ASM volatile(
    //Permit access to VFP/NEON, registers by modifying CPACR
    "        MRC     p15,0,R1,c1,c0,2  \n"
    "        ORR     R1,R1,#0x00F00000 \n"
    "        MCR     p15,0,R1,c1,c0,2  \n"

    //Ensure that subsequent instructions occur in the context of VFP/NEON access permitted
    "        ISB                       \n"

    //Enable VFP/NEON
    "        VMRS    R1,FPEXC          \n"
    "        ORR     R1,R1,#0x40000000 \n"
    "        VMSR    FPEXC,R1          \n"

    //Initialise VFP/NEON registers to 0
    "        MOV     R2,#0             \n"

    //Initialise D16 registers to 0
    "        VMOV    D0, R2,R2         \n"
    "        VMOV    D1, R2,R2         \n"
    "        VMOV    D2, R2,R2         \n"
    "        VMOV    D3, R2,R2         \n"
    "        VMOV    D4, R2,R2         \n"
    "        VMOV    D5, R2,R2         \n"
    "        VMOV    D6, R2,R2         \n"
    "        VMOV    D7, R2,R2         \n"
    "        VMOV    D8, R2,R2         \n"
    "        VMOV    D9, R2,R2         \n"
    "        VMOV    D10,R2,R2         \n"
    "        VMOV    D11,R2,R2         \n"
    "        VMOV    D12,R2,R2         \n"
    "        VMOV    D13,R2,R2         \n"
    "        VMOV    D14,R2,R2         \n"
    "        VMOV    D15,R2,R2         \n"

#if __ARM_NEON == 1
    //Initialise D32 registers to 0
    "        VMOV    D16,R2,R2         \n"
    "        VMOV    D17,R2,R2         \n"
    "        VMOV    D18,R2,R2         \n"
    "        VMOV    D19,R2,R2         \n"
    "        VMOV    D20,R2,R2         \n"
    "        VMOV    D21,R2,R2         \n"
    "        VMOV    D22,R2,R2         \n"
    "        VMOV    D23,R2,R2         \n"
    "        VMOV    D24,R2,R2         \n"
    "        VMOV    D25,R2,R2         \n"
    "        VMOV    D26,R2,R2         \n"
    "        VMOV    D27,R2,R2         \n"
    "        VMOV    D28,R2,R2         \n"
    "        VMOV    D29,R2,R2         \n"
    "        VMOV    D30,R2,R2         \n"
    "        VMOV    D31,R2,R2         \n"
#endif

    //Initialise FPSCR to a known state
    "        VMRS    R2,FPSCR          \n"
    "        LDR     R3,=0x00086060    \n" //Mask off all bits that do not have to be preserved. Non-preserved bits can/should be zero.
    "        AND     R2,R2,R3          \n"
    "        VMSR    FPSCR,R2            "
  );
}

#endif /* __CMSIS_ARMCLANG_H */
