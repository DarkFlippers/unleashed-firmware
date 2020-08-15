/**************************************************************************//**
 * @file     cmsis_iccarm.h
 * @brief    CMSIS compiler ICCARM (IAR Compiler for Arm) header file
 * @version  V5.0.6
 * @date     02. March 2018
 ******************************************************************************/

//------------------------------------------------------------------------------
//
// Copyright (c) 2017-2018 IAR Systems
//
// Licensed under the Apache License, Version 2.0 (the "License")
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//------------------------------------------------------------------------------


#ifndef __CMSIS_ICCARM_H__
#define __CMSIS_ICCARM_H__

#ifndef __ICCARM__
  #error This file should only be compiled by ICCARM
#endif

#pragma system_include

#define __IAR_FT _Pragma("inline=forced") __intrinsic

#if (__VER__ >= 8000000)
  #define __ICCARM_V8 1
#else
  #define __ICCARM_V8 0
#endif

#pragma language=extended

#ifndef __ALIGNED
  #if __ICCARM_V8
    #define __ALIGNED(x) __attribute__((aligned(x)))
  #elif (__VER__ >= 7080000)
    /* Needs IAR language extensions */
    #define __ALIGNED(x) __attribute__((aligned(x)))
  #else
    #warning No compiler specific solution for __ALIGNED.__ALIGNED is ignored.
    #define __ALIGNED(x)
  #endif
#endif


/* Define compiler macros for CPU architecture, used in CMSIS 5.
 */
#if __ARM_ARCH_7A__
/* Macro already defined */
#else
  #if defined(__ARM7A__)
    #define __ARM_ARCH_7A__ 1
  #endif
#endif

#ifndef __ASM
  #define __ASM __asm
#endif

#ifndef __INLINE
  #define __INLINE inline
#endif

#ifndef   __NO_RETURN
  #if __ICCARM_V8
    #define __NO_RETURN __attribute__((__noreturn__))
  #else
    #define __NO_RETURN _Pragma("object_attribute=__noreturn")
  #endif
#endif

#ifndef   __PACKED
  /* Needs IAR language extensions */
  #if __ICCARM_V8
    #define __PACKED __attribute__((packed, aligned(1)))
  #else
    #define __PACKED __packed
  #endif
#endif

#ifndef   __PACKED_STRUCT
  /* Needs IAR language extensions */
  #if __ICCARM_V8
    #define __PACKED_STRUCT struct __attribute__((packed, aligned(1)))
  #else
    #define __PACKED_STRUCT __packed struct
  #endif
#endif

#ifndef   __PACKED_UNION
  /* Needs IAR language extensions */
  #if __ICCARM_V8
    #define __PACKED_UNION union __attribute__((packed, aligned(1)))
  #else
    #define __PACKED_UNION __packed union
  #endif
#endif

#ifndef   __RESTRICT
  #define __RESTRICT            __restrict
#endif

#ifndef   __STATIC_INLINE
  #define __STATIC_INLINE       static inline
#endif

#ifndef   __FORCEINLINE
  #define __FORCEINLINE         _Pragma("inline=forced")
#endif

#ifndef   __STATIC_FORCEINLINE
  #define __STATIC_FORCEINLINE  __FORCEINLINE __STATIC_INLINE
#endif

#ifndef   CMSIS_DEPRECATED
  #define CMSIS_DEPRECATED      __attribute__((deprecated))
#endif

#ifndef __UNALIGNED_UINT16_READ
  #pragma language=save
  #pragma language=extended
  __IAR_FT uint16_t __iar_uint16_read(void const *ptr)
  {
    return *(__packed uint16_t*)(ptr);
  }
  #pragma language=restore
  #define __UNALIGNED_UINT16_READ(PTR) __iar_uint16_read(PTR)
#endif


#ifndef __UNALIGNED_UINT16_WRITE
  #pragma language=save
  #pragma language=extended
  __IAR_FT void __iar_uint16_write(void const *ptr, uint16_t val)
  {
    *(__packed uint16_t*)(ptr) = val;;
  }
  #pragma language=restore
  #define __UNALIGNED_UINT16_WRITE(PTR,VAL) __iar_uint16_write(PTR,VAL)
#endif

#ifndef __UNALIGNED_UINT32_READ
  #pragma language=save
  #pragma language=extended
  __IAR_FT uint32_t __iar_uint32_read(void const *ptr)
  {
    return *(__packed uint32_t*)(ptr);
  }
  #pragma language=restore
  #define __UNALIGNED_UINT32_READ(PTR) __iar_uint32_read(PTR)
#endif

#ifndef __UNALIGNED_UINT32_WRITE
  #pragma language=save
  #pragma language=extended
  __IAR_FT void __iar_uint32_write(void const *ptr, uint32_t val)
  {
    *(__packed uint32_t*)(ptr) = val;;
  }
  #pragma language=restore
  #define __UNALIGNED_UINT32_WRITE(PTR,VAL) __iar_uint32_write(PTR,VAL)
#endif

#if 0
#ifndef __UNALIGNED_UINT32   /* deprecated */
  #pragma language=save
  #pragma language=extended
  __packed struct  __iar_u32 { uint32_t v; };
  #pragma language=restore
  #define __UNALIGNED_UINT32(PTR) (((struct __iar_u32 *)(PTR))->v)
#endif
#endif

#ifndef   __USED
  #if __ICCARM_V8
    #define __USED __attribute__((used))
  #else
    #define __USED _Pragma("__root")
  #endif
#endif

#ifndef   __WEAK
  #if __ICCARM_V8
    #define __WEAK __attribute__((weak))
  #else
    #define __WEAK _Pragma("__weak")
  #endif
#endif


#ifndef __ICCARM_INTRINSICS_VERSION__
  #define __ICCARM_INTRINSICS_VERSION__  0
#endif

#if __ICCARM_INTRINSICS_VERSION__ == 2

  #if defined(__CLZ)
    #undef __CLZ
  #endif
  #if defined(__REVSH)
    #undef __REVSH
  #endif
  #if defined(__RBIT)
    #undef __RBIT
  #endif
  #if defined(__SSAT)
    #undef __SSAT
  #endif
  #if defined(__USAT)
    #undef __USAT
  #endif

  #include "iccarm_builtin.h"

  #define __enable_irq        __iar_builtin_enable_interrupt
  #define __disable_irq       __iar_builtin_disable_interrupt
  #define __enable_fault_irq    __iar_builtin_enable_fiq
  #define __disable_fault_irq   __iar_builtin_disable_fiq
  #define __arm_rsr           __iar_builtin_rsr
  #define __arm_wsr           __iar_builtin_wsr

  #if __FPU_PRESENT
    #define __get_FPSCR()             (__arm_rsr("FPSCR"))
  #else
    #define __get_FPSCR()             ( 0 )
  #endif

  #define __set_FPSCR(VALUE)          (__arm_wsr("FPSCR", VALUE))

  #define __get_CPSR()                (__arm_rsr("CPSR"))
  #define __get_mode()                (__get_CPSR() & 0x1FU)

  #define __set_CPSR(VALUE)           (__arm_wsr("CPSR", (VALUE)))
  #define __set_mode(VALUE)           (__arm_wsr("CPSR_c", (VALUE)))


  #define __get_FPEXC()       (__arm_rsr("FPEXC"))
  #define __set_FPEXC(VALUE)    (__arm_wsr("FPEXC", VALUE))

  #define __get_CP(cp, op1, RT, CRn, CRm, op2) \
    ((RT) = __arm_rsr("p" # cp ":" # op1 ":c" # CRn ":c" # CRm ":" # op2))

  #define __set_CP(cp, op1, RT, CRn, CRm, op2) \
    (__arm_wsr("p" # cp ":" # op1 ":c" # CRn ":c" # CRm ":" # op2, (RT)))

  #define __get_CP64(cp, op1, Rt, CRm) \
    __ASM volatile("MRRC p" # cp ", " # op1 ", %Q0, %R0, c" # CRm  : "=r" (Rt) : : "memory" )

  #define __set_CP64(cp, op1, Rt, CRm) \
    __ASM volatile("MCRR p" # cp ", " # op1 ", %Q0, %R0, c" # CRm  : : "r" (Rt) : "memory" )

  #include "cmsis_cp15.h"

  #define __NOP     __iar_builtin_no_operation

  #define __CLZ     __iar_builtin_CLZ
  #define __CLREX   __iar_builtin_CLREX

  #define __DMB     __iar_builtin_DMB
  #define __DSB     __iar_builtin_DSB
  #define __ISB     __iar_builtin_ISB

  #define __LDREXB  __iar_builtin_LDREXB
  #define __LDREXH  __iar_builtin_LDREXH
  #define __LDREXW  __iar_builtin_LDREX

  #define __RBIT    __iar_builtin_RBIT
  #define __REV     __iar_builtin_REV
  #define __REV16   __iar_builtin_REV16

  __IAR_FT int16_t __REVSH(int16_t val)
  {
    return (int16_t) __iar_builtin_REVSH(val);
  }

  #define __ROR     __iar_builtin_ROR
  #define __RRX     __iar_builtin_RRX

  #define __SEV     __iar_builtin_SEV

  #define __SSAT    __iar_builtin_SSAT

  #define __STREXB  __iar_builtin_STREXB
  #define __STREXH  __iar_builtin_STREXH
  #define __STREXW  __iar_builtin_STREX

  #define __USAT    __iar_builtin_USAT

  #define __WFE     __iar_builtin_WFE
  #define __WFI     __iar_builtin_WFI

  #define __SADD8   __iar_builtin_SADD8
  #define __QADD8   __iar_builtin_QADD8
  #define __SHADD8  __iar_builtin_SHADD8
  #define __UADD8   __iar_builtin_UADD8
  #define __UQADD8  __iar_builtin_UQADD8
  #define __UHADD8  __iar_builtin_UHADD8
  #define __SSUB8   __iar_builtin_SSUB8
  #define __QSUB8   __iar_builtin_QSUB8
  #define __SHSUB8  __iar_builtin_SHSUB8
  #define __USUB8   __iar_builtin_USUB8
  #define __UQSUB8  __iar_builtin_UQSUB8
  #define __UHSUB8  __iar_builtin_UHSUB8
  #define __SADD16  __iar_builtin_SADD16
  #define __QADD16  __iar_builtin_QADD16
  #define __SHADD16 __iar_builtin_SHADD16
  #define __UADD16  __iar_builtin_UADD16
  #define __UQADD16 __iar_builtin_UQADD16
  #define __UHADD16 __iar_builtin_UHADD16
  #define __SSUB16  __iar_builtin_SSUB16
  #define __QSUB16  __iar_builtin_QSUB16
  #define __SHSUB16 __iar_builtin_SHSUB16
  #define __USUB16  __iar_builtin_USUB16
  #define __UQSUB16 __iar_builtin_UQSUB16
  #define __UHSUB16 __iar_builtin_UHSUB16
  #define __SASX    __iar_builtin_SASX
  #define __QASX    __iar_builtin_QASX
  #define __SHASX   __iar_builtin_SHASX
  #define __UASX    __iar_builtin_UASX
  #define __UQASX   __iar_builtin_UQASX
  #define __UHASX   __iar_builtin_UHASX
  #define __SSAX    __iar_builtin_SSAX
  #define __QSAX    __iar_builtin_QSAX
  #define __SHSAX   __iar_builtin_SHSAX
  #define __USAX    __iar_builtin_USAX
  #define __UQSAX   __iar_builtin_UQSAX
  #define __UHSAX   __iar_builtin_UHSAX
  #define __USAD8   __iar_builtin_USAD8
  #define __USADA8  __iar_builtin_USADA8
  #define __SSAT16  __iar_builtin_SSAT16
  #define __USAT16  __iar_builtin_USAT16
  #define __UXTB16  __iar_builtin_UXTB16
  #define __UXTAB16 __iar_builtin_UXTAB16
  #define __SXTB16  __iar_builtin_SXTB16
  #define __SXTAB16 __iar_builtin_SXTAB16
  #define __SMUAD   __iar_builtin_SMUAD
  #define __SMUADX  __iar_builtin_SMUADX
  #define __SMMLA   __iar_builtin_SMMLA
  #define __SMLAD   __iar_builtin_SMLAD
  #define __SMLADX  __iar_builtin_SMLADX
  #define __SMLALD  __iar_builtin_SMLALD
  #define __SMLALDX __iar_builtin_SMLALDX
  #define __SMUSD   __iar_builtin_SMUSD
  #define __SMUSDX  __iar_builtin_SMUSDX
  #define __SMLSD   __iar_builtin_SMLSD
  #define __SMLSDX  __iar_builtin_SMLSDX
  #define __SMLSLD  __iar_builtin_SMLSLD
  #define __SMLSLDX __iar_builtin_SMLSLDX
  #define __SEL     __iar_builtin_SEL
  #define __QADD    __iar_builtin_QADD
  #define __QSUB    __iar_builtin_QSUB
  #define __PKHBT   __iar_builtin_PKHBT
  #define __PKHTB   __iar_builtin_PKHTB

#else /* __ICCARM_INTRINSICS_VERSION__ == 2 */

  #if !__FPU_PRESENT
  #define __get_FPSCR __cmsis_iar_get_FPSR_not_active
  #endif

  #ifdef __INTRINSICS_INCLUDED
  #error intrinsics.h is already included previously!
  #endif

  #include <intrinsics.h>

  #if !__FPU_PRESENT
  #define __get_FPSCR() (0)
  #endif

  #pragma diag_suppress=Pe940
  #pragma diag_suppress=Pe177

  #define __enable_irq        __enable_interrupt
  #define __disable_irq       __disable_interrupt
  #define __enable_fault_irq    __enable_fiq
  #define __disable_fault_irq   __disable_fiq
  #define __NOP               __no_operation

  #define __get_xPSR          __get_PSR

  __IAR_FT void __set_mode(uint32_t mode)
  {
    __ASM volatile("MSR  cpsr_c, %0" : : "r" (mode) : "memory");
  }

  __IAR_FT uint32_t __LDREXW(uint32_t volatile *ptr)
  {
    return __LDREX((unsigned long *)ptr);
  }

  __IAR_FT uint32_t __STREXW(uint32_t value, uint32_t volatile *ptr)
  {
    return __STREX(value, (unsigned long *)ptr);
  }


  __IAR_FT uint32_t __RRX(uint32_t value)
  {
    uint32_t result;
    __ASM("RRX      %0, %1" : "=r"(result) : "r" (value) : "cc");
    return(result);
  }


  __IAR_FT uint32_t __ROR(uint32_t op1, uint32_t op2)
  {
    return (op1 >> op2) | (op1 << ((sizeof(op1)*8)-op2));
  }

  __IAR_FT uint32_t __get_FPEXC(void)
  {
  #if (__FPU_PRESENT == 1)
    uint32_t result;
    __ASM volatile("VMRS %0, fpexc" : "=r" (result) : : "memory");
    return(result);
  #else
    return(0);
  #endif
  }

  __IAR_FT void __set_FPEXC(uint32_t fpexc)
  {
  #if (__FPU_PRESENT == 1)
    __ASM volatile ("VMSR fpexc, %0" : : "r" (fpexc) : "memory");
  #endif
  }


  #define __get_CP(cp, op1, Rt, CRn, CRm, op2) \
    __ASM volatile("MRC p" # cp ", " # op1 ", %0, c" # CRn ", c" # CRm ", " # op2 : "=r" (Rt) : : "memory" )
  #define __set_CP(cp, op1, Rt, CRn, CRm, op2) \
    __ASM volatile("MCR p" # cp ", " # op1 ", %0, c" # CRn ", c" # CRm ", " # op2 : : "r" (Rt) : "memory" )
  #define __get_CP64(cp, op1, Rt, CRm) \
    __ASM volatile("MRRC p" # cp ", " # op1 ", %Q0, %R0, c" # CRm  : "=r" (Rt) : : "memory" )
  #define __set_CP64(cp, op1, Rt, CRm) \
    __ASM volatile("MCRR p" # cp ", " # op1 ", %Q0, %R0, c" # CRm  : : "r" (Rt) : "memory" )

  #include "cmsis_cp15.h"

#endif   /* __ICCARM_INTRINSICS_VERSION__ == 2 */

#define __BKPT(value)    __asm volatile ("BKPT     %0" : : "i"(value))


__IAR_FT uint32_t __get_SP_usr(void)
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

__IAR_FT void __set_SP_usr(uint32_t topOfProcStack)
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

#define __get_mode()                (__get_CPSR() & 0x1FU)

__STATIC_INLINE
void __FPU_Enable(void)
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

#ifdef __ARM_ADVANCED_SIMD__
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
    "        MOV32   R3,#0x00086060    \n" //Mask off all bits that do not have to be preserved. Non-preserved bits can/should be zero.
    "        AND     R2,R2,R3          \n"
    "        VMSR    FPSCR,R2          \n");
}



#undef __IAR_FT
#undef __ICCARM_V8

#pragma diag_default=Pe940
#pragma diag_default=Pe177

#endif /* __CMSIS_ICCARM_H__ */
