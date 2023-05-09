/**************************************************************************//**
 * @file     cmsis_armclang.h
 * @brief    CMSIS compiler armclang (Arm Compiler 6) header file
 * @version  V5.5.0
 * @date     20. January 2023
 ******************************************************************************/
/*
 * Copyright (c) 2009-2023 Arm Limited. All rights reserved.
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

/*lint -esym(9058, IRQn)*/ /* disable MISRA 2012 Rule 2.4 for IRQn */

#ifndef __CMSIS_ARMCLANG_H
#define __CMSIS_ARMCLANG_H

#pragma clang system_header   /* treat file as system include file */

/* CMSIS compiler specific defines */
#ifndef   __ASM
  #define __ASM                                  __asm
#endif
#ifndef   __INLINE
  #define __INLINE                               __inline
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
#ifndef   __PACKED_UNION
  #define __PACKED_UNION                         union __attribute__((packed, aligned(1)))
#endif
#ifndef   __UNALIGNED_UINT32        /* deprecated */
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wpacked"
/*lint -esym(9058, T_UINT32)*/ /* disable MISRA 2012 Rule 2.4 for T_UINT32 */
  struct __attribute__((packed)) T_UINT32 { uint32_t v; };
  #pragma clang diagnostic pop
  #define __UNALIGNED_UINT32(x)                  (((struct T_UINT32 *)(x))->v)
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
/*lint -esym(9058, T_UINT32_READ)*/ /* disable MISRA 2012 Rule 2.4 for T_UINT32_READ */
  __PACKED_STRUCT T_UINT32_READ { uint32_t v; };
  #pragma clang diagnostic pop
  #define __UNALIGNED_UINT32_READ(addr)          (((const struct T_UINT32_READ *)(const void *)(addr))->v)
#endif
#ifndef   __ALIGNED
  #define __ALIGNED(x)                           __attribute__((aligned(x)))
#endif
#ifndef   __RESTRICT
  #define __RESTRICT                             __restrict
#endif
#ifndef   __COMPILER_BARRIER
  #define __COMPILER_BARRIER()                   __ASM volatile("":::"memory")
#endif
#ifndef __NO_INIT
  #define __NO_INIT                              __attribute__ ((section (".bss.noinit")))
#endif
#ifndef __ALIAS
  #define __ALIAS(x)                             __attribute__ ((alias(x)))
#endif


/* #########################  Startup and Lowlevel Init  ######################## */

#ifndef __PROGRAM_START
#define __PROGRAM_START           __main
#endif

#ifndef __INITIAL_SP
#define __INITIAL_SP              Image$$ARM_LIB_STACK$$ZI$$Limit
#endif

#ifndef __STACK_LIMIT
#define __STACK_LIMIT             Image$$ARM_LIB_STACK$$ZI$$Base
#endif

#ifndef __VECTOR_TABLE
#define __VECTOR_TABLE            __Vectors
#endif

#ifndef __VECTOR_TABLE_ATTRIBUTE
#define __VECTOR_TABLE_ATTRIBUTE  __attribute__((used, section("RESET")))
#endif

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
#ifndef __STACK_SEAL
#define __STACK_SEAL              Image$$STACKSEAL$$ZI$$Base
#endif

#ifndef __TZ_STACK_SEAL_SIZE
#define __TZ_STACK_SEAL_SIZE      8U
#endif

#ifndef __TZ_STACK_SEAL_VALUE
#define __TZ_STACK_SEAL_VALUE     0xFEF5EDA5FEF5EDA5ULL
#endif


__STATIC_FORCEINLINE void __TZ_set_STACKSEAL_S (uint32_t* stackTop) {
  *((uint64_t *)stackTop) = __TZ_STACK_SEAL_VALUE;
}
#endif


/* ##########################  Core Instruction Access  ######################### */
/** \defgroup CMSIS_Core_InstructionInterface CMSIS Core Instruction Interface
  Access to dedicated instructions
  @{
*/

/* Define macros for porting to both thumb1 and thumb2.
 * For thumb1, use low register (r0-r7), specified by constraint "l"
 * Otherwise, use general registers, specified by constraint "r" */
#if defined (__thumb__) && !defined (__thumb2__)
#define __CMSIS_GCC_OUT_REG(r) "=l" (r)
#define __CMSIS_GCC_RW_REG(r) "+l" (r)
#define __CMSIS_GCC_USE_REG(r) "l" (r)
#else
#define __CMSIS_GCC_OUT_REG(r) "=r" (r)
#define __CMSIS_GCC_RW_REG(r) "+r" (r)
#define __CMSIS_GCC_USE_REG(r) "r" (r)
#endif

/**
  \brief   No Operation
  \details No Operation does nothing. This instruction can be used for code alignment purposes.
 */
#define __NOP          __builtin_arm_nop

/**
  \brief   Wait For Interrupt
  \details Wait For Interrupt is a hint instruction that suspends execution until one of a number of events occurs.
 */
#define __WFI          __builtin_arm_wfi


/**
  \brief   Wait For Event
  \details Wait For Event is a hint instruction that permits the processor to enter
           a low-power state until one of a number of events occurs.
 */
#define __WFE          __builtin_arm_wfe


/**
  \brief   Send Event
  \details Send Event is a hint instruction. It causes an event to be signaled to the CPU.
 */
#define __SEV          __builtin_arm_sev


/**
  \brief   Instruction Synchronization Barrier
  \details Instruction Synchronization Barrier flushes the pipeline in the processor,
           so that all instructions following the ISB are fetched from cache or memory,
           after the instruction has been completed.
 */
#define __ISB()        __builtin_arm_isb(0xF)

/**
  \brief   Data Synchronization Barrier
  \details Acts as a special kind of Data Memory Barrier.
           It completes when all explicit memory accesses before this instruction complete.
 */
#define __DSB()        __builtin_arm_dsb(0xF)


/**
  \brief   Data Memory Barrier
  \details Ensures the apparent order of the explicit memory operations before
           and after the instruction, without ensuring their completion.
 */
#define __DMB()        __builtin_arm_dmb(0xF)


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
  \details Causes the processor to enter Debug state.
           Debug tools can use this to investigate system state when the instruction at a particular address is reached.
  \param [in]    value  is ignored by the processor.
                 If required, a debugger can use it to store additional information about the breakpoint.
 */
#define __BKPT(value)     __ASM volatile ("bkpt "#value)


/**
  \brief   Reverse bit order of value
  \details Reverses the bit order of the given value.
  \param [in]    value  Value to reverse
  \return               Reversed value
 */
#define __RBIT            __builtin_arm_rbit

/**
  \brief   Count leading zeros
  \details Counts the number of leading zeros of a data value.
  \param [in]  value  Value to count the leading zeros
  \return             number of leading zeros in value
 */
__STATIC_FORCEINLINE uint8_t __CLZ(uint32_t value)
{
  /* Even though __builtin_clz produces a CLZ instruction on ARM, formally
     __builtin_clz(0) is undefined behaviour, so handle this case specially.
     This guarantees ARM-compatible results if happening to compile on a non-ARM
     target, and ensures the compiler doesn't decide to activate any
     optimisations using the logic "value was passed to __builtin_clz, so it
     is non-zero".
     ARM Compiler 6.10 and possibly earlier will optimise this test away, leaving a
     single CLZ instruction.
   */
  if (value == 0U)
  {
    return 32U;
  }
  return __builtin_clz(value);
}


#if ((defined (__ARM_ARCH_7M__       ) && (__ARM_ARCH_7M__        == 1)) || \
     (defined (__ARM_ARCH_7EM__      ) && (__ARM_ARCH_7EM__       == 1)) || \
     (defined (__ARM_ARCH_8M_MAIN__  ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
     (defined (__ARM_ARCH_8M_BASE__  ) && (__ARM_ARCH_8M_BASE__   == 1)) || \
     (defined (__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ == 1))     )

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

#endif /* ((defined (__ARM_ARCH_7M__       ) && (__ARM_ARCH_7M__        == 1)) || \
           (defined (__ARM_ARCH_7EM__      ) && (__ARM_ARCH_7EM__       == 1)) || \
           (defined (__ARM_ARCH_8M_MAIN__  ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
           (defined (__ARM_ARCH_8M_BASE__  ) && (__ARM_ARCH_8M_BASE__   == 1)) || \
           (defined (__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ == 1))     ) */


#if ((defined (__ARM_ARCH_7M__       ) && (__ARM_ARCH_7M__        == 1)) || \
     (defined (__ARM_ARCH_7EM__      ) && (__ARM_ARCH_7EM__       == 1)) || \
     (defined (__ARM_ARCH_8M_MAIN__  ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
     (defined (__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ == 1))     )

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


/**
  \brief   Rotate Right with Extend (32 bit)
  \details Moves each bit of a bitstring right by one bit.
           The carry input is shifted in at the left end of the bitstring.
  \param [in]    value  Value to rotate
  \return               Rotated value
 */
__STATIC_FORCEINLINE uint32_t __RRX(uint32_t value)
{
  uint32_t result;

  __ASM volatile ("rrx %0, %1" : __CMSIS_GCC_OUT_REG (result) : __CMSIS_GCC_USE_REG (value) );
  return(result);
}


/**
  \brief   LDRT Unprivileged (8 bit)
  \details Executes a Unprivileged LDRT instruction for 8 bit value.
  \param [in]    ptr  Pointer to data
  \return             value of type uint8_t at (*ptr)
 */
__STATIC_FORCEINLINE uint8_t __LDRBT(volatile uint8_t *ptr)
{
  uint32_t result;

  __ASM volatile ("ldrbt %0, %1" : "=r" (result) : "Q" (*ptr) );
  return ((uint8_t) result);    /* Add explicit type cast here */
}


/**
  \brief   LDRT Unprivileged (16 bit)
  \details Executes a Unprivileged LDRT instruction for 16 bit values.
  \param [in]    ptr  Pointer to data
  \return        value of type uint16_t at (*ptr)
 */
__STATIC_FORCEINLINE uint16_t __LDRHT(volatile uint16_t *ptr)
{
  uint32_t result;

  __ASM volatile ("ldrht %0, %1" : "=r" (result) : "Q" (*ptr) );
  return ((uint16_t) result);    /* Add explicit type cast here */
}


/**
  \brief   LDRT Unprivileged (32 bit)
  \details Executes a Unprivileged LDRT instruction for 32 bit values.
  \param [in]    ptr  Pointer to data
  \return        value of type uint32_t at (*ptr)
 */
__STATIC_FORCEINLINE uint32_t __LDRT(volatile uint32_t *ptr)
{
  uint32_t result;

  __ASM volatile ("ldrt %0, %1" : "=r" (result) : "Q" (*ptr) );
  return(result);
}


/**
  \brief   STRT Unprivileged (8 bit)
  \details Executes a Unprivileged STRT instruction for 8 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
 */
__STATIC_FORCEINLINE void __STRBT(uint8_t value, volatile uint8_t *ptr)
{
  __ASM volatile ("strbt %1, %0" : "=Q" (*ptr) : "r" ((uint32_t)value) );
}


/**
  \brief   STRT Unprivileged (16 bit)
  \details Executes a Unprivileged STRT instruction for 16 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
 */
__STATIC_FORCEINLINE void __STRHT(uint16_t value, volatile uint16_t *ptr)
{
  __ASM volatile ("strht %1, %0" : "=Q" (*ptr) : "r" ((uint32_t)value) );
}


/**
  \brief   STRT Unprivileged (32 bit)
  \details Executes a Unprivileged STRT instruction for 32 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
 */
__STATIC_FORCEINLINE void __STRT(uint32_t value, volatile uint32_t *ptr)
{
  __ASM volatile ("strt %1, %0" : "=Q" (*ptr) : "r" (value) );
}

#else /* ((defined (__ARM_ARCH_7M__       ) && (__ARM_ARCH_7M__        == 1)) || \
          (defined (__ARM_ARCH_7EM__      ) && (__ARM_ARCH_7EM__       == 1)) || \
          (defined (__ARM_ARCH_8M_MAIN__  ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
          (defined (__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ == 1))     ) */

/**
  \brief   Signed Saturate
  \details Saturates a signed value.
  \param [in]  value  Value to be saturated
  \param [in]    sat  Bit position to saturate to (1..32)
  \return             Saturated value
 */
__STATIC_FORCEINLINE int32_t __SSAT(int32_t val, uint32_t sat)
{
  if ((sat >= 1U) && (sat <= 32U))
  {
    const int32_t max = (int32_t)((1U << (sat - 1U)) - 1U);
    const int32_t min = -1 - max ;
    if (val > max)
    {
      return max;
    }
    else if (val < min)
    {
      return min;
    }
  }
  return val;
}

/**
  \brief   Unsigned Saturate
  \details Saturates an unsigned value.
  \param [in]  value  Value to be saturated
  \param [in]    sat  Bit position to saturate to (0..31)
  \return             Saturated value
 */
__STATIC_FORCEINLINE uint32_t __USAT(int32_t val, uint32_t sat)
{
  if (sat <= 31U)
  {
    const uint32_t max = ((1U << sat) - 1U);
    if (val > (int32_t)max)
    {
      return max;
    }
    else if (val < 0)
    {
      return 0U;
    }
  }
  return (uint32_t)val;
}

#endif /* ((defined (__ARM_ARCH_7M__       ) && (__ARM_ARCH_7M__        == 1)) || \
           (defined (__ARM_ARCH_7EM__      ) && (__ARM_ARCH_7EM__       == 1)) || \
           (defined (__ARM_ARCH_8M_MAIN__  ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
           (defined (__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ == 1))     ) */


#if ((defined (__ARM_ARCH_8M_MAIN__  ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
     (defined (__ARM_ARCH_8M_BASE__  ) && (__ARM_ARCH_8M_BASE__   == 1)) || \
     (defined (__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ == 1))     )

/**
  \brief   Load-Acquire (8 bit)
  \details Executes a LDAB instruction for 8 bit value.
  \param [in]    ptr  Pointer to data
  \return             value of type uint8_t at (*ptr)
 */
__STATIC_FORCEINLINE uint8_t __LDAB(volatile uint8_t *ptr)
{
  uint32_t result;

  __ASM volatile ("ldab %0, %1" : "=r" (result) : "Q" (*ptr) : "memory" );
  return ((uint8_t) result);
}


/**
  \brief   Load-Acquire (16 bit)
  \details Executes a LDAH instruction for 16 bit values.
  \param [in]    ptr  Pointer to data
  \return        value of type uint16_t at (*ptr)
 */
__STATIC_FORCEINLINE uint16_t __LDAH(volatile uint16_t *ptr)
{
  uint32_t result;

  __ASM volatile ("ldah %0, %1" : "=r" (result) : "Q" (*ptr) : "memory" );
  return ((uint16_t) result);
}


/**
  \brief   Load-Acquire (32 bit)
  \details Executes a LDA instruction for 32 bit values.
  \param [in]    ptr  Pointer to data
  \return        value of type uint32_t at (*ptr)
 */
__STATIC_FORCEINLINE uint32_t __LDA(volatile uint32_t *ptr)
{
  uint32_t result;

  __ASM volatile ("lda %0, %1" : "=r" (result) : "Q" (*ptr) : "memory" );
  return(result);
}


/**
  \brief   Store-Release (8 bit)
  \details Executes a STLB instruction for 8 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
 */
__STATIC_FORCEINLINE void __STLB(uint8_t value, volatile uint8_t *ptr)
{
  __ASM volatile ("stlb %1, %0" : "=Q" (*ptr) : "r" ((uint32_t)value) : "memory" );
}


/**
  \brief   Store-Release (16 bit)
  \details Executes a STLH instruction for 16 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
 */
__STATIC_FORCEINLINE void __STLH(uint16_t value, volatile uint16_t *ptr)
{
  __ASM volatile ("stlh %1, %0" : "=Q" (*ptr) : "r" ((uint32_t)value) : "memory" );
}


/**
  \brief   Store-Release (32 bit)
  \details Executes a STL instruction for 32 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
 */
__STATIC_FORCEINLINE void __STL(uint32_t value, volatile uint32_t *ptr)
{
  __ASM volatile ("stl %1, %0" : "=Q" (*ptr) : "r" ((uint32_t)value) : "memory" );
}


/**
  \brief   Load-Acquire Exclusive (8 bit)
  \details Executes a LDAB exclusive instruction for 8 bit value.
  \param [in]    ptr  Pointer to data
  \return             value of type uint8_t at (*ptr)
 */
#define     __LDAEXB                 (uint8_t)__builtin_arm_ldaex


/**
  \brief   Load-Acquire Exclusive (16 bit)
  \details Executes a LDAH exclusive instruction for 16 bit values.
  \param [in]    ptr  Pointer to data
  \return        value of type uint16_t at (*ptr)
 */
#define     __LDAEXH                 (uint16_t)__builtin_arm_ldaex


/**
  \brief   Load-Acquire Exclusive (32 bit)
  \details Executes a LDA exclusive instruction for 32 bit values.
  \param [in]    ptr  Pointer to data
  \return        value of type uint32_t at (*ptr)
 */
#define     __LDAEX                  (uint32_t)__builtin_arm_ldaex


/**
  \brief   Store-Release Exclusive (8 bit)
  \details Executes a STLB exclusive instruction for 8 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
  \return          0  Function succeeded
  \return          1  Function failed
 */
#define     __STLEXB                 (uint32_t)__builtin_arm_stlex


/**
  \brief   Store-Release Exclusive (16 bit)
  \details Executes a STLH exclusive instruction for 16 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
  \return          0  Function succeeded
  \return          1  Function failed
 */
#define     __STLEXH                 (uint32_t)__builtin_arm_stlex


/**
  \brief   Store-Release Exclusive (32 bit)
  \details Executes a STL exclusive instruction for 32 bit values.
  \param [in]  value  Value to store
  \param [in]    ptr  Pointer to location
  \return          0  Function succeeded
  \return          1  Function failed
 */
#define     __STLEX                  (uint32_t)__builtin_arm_stlex

#endif /* ((defined (__ARM_ARCH_8M_MAIN__  ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
           (defined (__ARM_ARCH_8M_BASE__  ) && (__ARM_ARCH_8M_BASE__   == 1)) || \
           (defined (__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ == 1))     ) */

/** @}*/ /* end of group CMSIS_Core_InstructionInterface */


/* ###########################  Core Function Access  ########################### */
/** \ingroup  CMSIS_Core_FunctionInterface
    \defgroup CMSIS_Core_RegAccFunctions CMSIS Core Register Access Functions
  @{
 */

/**
  \brief   Enable IRQ Interrupts
  \details Enables IRQ interrupts by clearing special-purpose register PRIMASK.
           Can only be executed in Privileged modes.
 */
#ifndef __ARM_COMPAT_H
__STATIC_FORCEINLINE void __enable_irq(void)
{
  __ASM volatile ("cpsie i" : : : "memory");
}
#endif


/**
  \brief   Disable IRQ Interrupts
  \details Disables IRQ interrupts by setting special-purpose register PRIMASK.
           Can only be executed in Privileged modes.
 */
#ifndef __ARM_COMPAT_H
__STATIC_FORCEINLINE void __disable_irq(void)
{
  __ASM volatile ("cpsid i" : : : "memory");
}
#endif


/**
  \brief   Get Control Register
  \details Returns the content of the Control Register.
  \return               Control Register value
 */
__STATIC_FORCEINLINE uint32_t __get_CONTROL(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, control" : "=r" (result) );
  return(result);
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Get Control Register (non-secure)
  \details Returns the content of the non-secure Control Register when in secure mode.
  \return               non-secure Control Register value
 */
__STATIC_FORCEINLINE uint32_t __TZ_get_CONTROL_NS(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, control_ns" : "=r" (result) );
  return(result);
}
#endif


/**
  \brief   Set Control Register
  \details Writes the given value to the Control Register.
  \param [in]    control  Control Register value to set
 */
__STATIC_FORCEINLINE void __set_CONTROL(uint32_t control)
{
  __ASM volatile ("MSR control, %0" : : "r" (control) : "memory");
  __ISB();
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Set Control Register (non-secure)
  \details Writes the given value to the non-secure Control Register when in secure state.
  \param [in]    control  Control Register value to set
 */
__STATIC_FORCEINLINE void __TZ_set_CONTROL_NS(uint32_t control)
{
  __ASM volatile ("MSR control_ns, %0" : : "r" (control) : "memory");
  __ISB();
}
#endif


/**
  \brief   Get IPSR Register
  \details Returns the content of the IPSR Register.
  \return               IPSR Register value
 */
__STATIC_FORCEINLINE uint32_t __get_IPSR(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, ipsr" : "=r" (result) );
  return(result);
}


/**
  \brief   Get APSR Register
  \details Returns the content of the APSR Register.
  \return               APSR Register value
 */
__STATIC_FORCEINLINE uint32_t __get_APSR(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, apsr" : "=r" (result) );
  return(result);
}


/**
  \brief   Get xPSR Register
  \details Returns the content of the xPSR Register.
  \return               xPSR Register value
 */
__STATIC_FORCEINLINE uint32_t __get_xPSR(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, xpsr" : "=r" (result) );
  return(result);
}


/**
  \brief   Get Process Stack Pointer
  \details Returns the current value of the Process Stack Pointer (PSP).
  \return               PSP Register value
 */
__STATIC_FORCEINLINE uint32_t __get_PSP(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, psp"  : "=r" (result) );
  return(result);
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Get Process Stack Pointer (non-secure)
  \details Returns the current value of the non-secure Process Stack Pointer (PSP) when in secure state.
  \return               PSP Register value
 */
__STATIC_FORCEINLINE uint32_t __TZ_get_PSP_NS(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, psp_ns"  : "=r" (result) );
  return(result);
}
#endif


/**
  \brief   Set Process Stack Pointer
  \details Assigns the given value to the Process Stack Pointer (PSP).
  \param [in]    topOfProcStack  Process Stack Pointer value to set
 */
__STATIC_FORCEINLINE void __set_PSP(uint32_t topOfProcStack)
{
  __ASM volatile ("MSR psp, %0" : : "r" (topOfProcStack) : );
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Set Process Stack Pointer (non-secure)
  \details Assigns the given value to the non-secure Process Stack Pointer (PSP) when in secure state.
  \param [in]    topOfProcStack  Process Stack Pointer value to set
 */
__STATIC_FORCEINLINE void __TZ_set_PSP_NS(uint32_t topOfProcStack)
{
  __ASM volatile ("MSR psp_ns, %0" : : "r" (topOfProcStack) : );
}
#endif


/**
  \brief   Get Main Stack Pointer
  \details Returns the current value of the Main Stack Pointer (MSP).
  \return               MSP Register value
 */
__STATIC_FORCEINLINE uint32_t __get_MSP(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, msp" : "=r" (result) );
  return(result);
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Get Main Stack Pointer (non-secure)
  \details Returns the current value of the non-secure Main Stack Pointer (MSP) when in secure state.
  \return               MSP Register value
 */
__STATIC_FORCEINLINE uint32_t __TZ_get_MSP_NS(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, msp_ns" : "=r" (result) );
  return(result);
}
#endif


/**
  \brief   Set Main Stack Pointer
  \details Assigns the given value to the Main Stack Pointer (MSP).
  \param [in]    topOfMainStack  Main Stack Pointer value to set
 */
__STATIC_FORCEINLINE void __set_MSP(uint32_t topOfMainStack)
{
  __ASM volatile ("MSR msp, %0" : : "r" (topOfMainStack) : );
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Set Main Stack Pointer (non-secure)
  \details Assigns the given value to the non-secure Main Stack Pointer (MSP) when in secure state.
  \param [in]    topOfMainStack  Main Stack Pointer value to set
 */
__STATIC_FORCEINLINE void __TZ_set_MSP_NS(uint32_t topOfMainStack)
{
  __ASM volatile ("MSR msp_ns, %0" : : "r" (topOfMainStack) : );
}
#endif


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Get Stack Pointer (non-secure)
  \details Returns the current value of the non-secure Stack Pointer (SP) when in secure state.
  \return               SP Register value
 */
__STATIC_FORCEINLINE uint32_t __TZ_get_SP_NS(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, sp_ns" : "=r" (result) );
  return(result);
}


/**
  \brief   Set Stack Pointer (non-secure)
  \details Assigns the given value to the non-secure Stack Pointer (SP) when in secure state.
  \param [in]    topOfStack  Stack Pointer value to set
 */
__STATIC_FORCEINLINE void __TZ_set_SP_NS(uint32_t topOfStack)
{
  __ASM volatile ("MSR sp_ns, %0" : : "r" (topOfStack) : );
}
#endif


/**
  \brief   Get Priority Mask
  \details Returns the current state of the priority mask bit from the Priority Mask Register.
  \return               Priority Mask value
 */
__STATIC_FORCEINLINE uint32_t __get_PRIMASK(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, primask" : "=r" (result) );
  return(result);
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Get Priority Mask (non-secure)
  \details Returns the current state of the non-secure priority mask bit from the Priority Mask Register when in secure state.
  \return               Priority Mask value
 */
__STATIC_FORCEINLINE uint32_t __TZ_get_PRIMASK_NS(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, primask_ns" : "=r" (result) );
  return(result);
}
#endif


/**
  \brief   Set Priority Mask
  \details Assigns the given value to the Priority Mask Register.
  \param [in]    priMask  Priority Mask
 */
__STATIC_FORCEINLINE void __set_PRIMASK(uint32_t priMask)
{
  __ASM volatile ("MSR primask, %0" : : "r" (priMask) : "memory");
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Set Priority Mask (non-secure)
  \details Assigns the given value to the non-secure Priority Mask Register when in secure state.
  \param [in]    priMask  Priority Mask
 */
__STATIC_FORCEINLINE void __TZ_set_PRIMASK_NS(uint32_t priMask)
{
  __ASM volatile ("MSR primask_ns, %0" : : "r" (priMask) : "memory");
}
#endif


#if ((defined (__ARM_ARCH_7M__       ) && (__ARM_ARCH_7M__        == 1)) || \
     (defined (__ARM_ARCH_7EM__      ) && (__ARM_ARCH_7EM__       == 1)) || \
     (defined (__ARM_ARCH_8M_MAIN__  ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
     (defined (__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ == 1))     )
/**
  \brief   Enable FIQ
  \details Enables FIQ interrupts by clearing special-purpose register FAULTMASK.
           Can only be executed in Privileged modes.
 */
__STATIC_FORCEINLINE void __enable_fault_irq(void)
{
  __ASM volatile ("cpsie f" : : : "memory");
}


/**
  \brief   Disable FIQ
  \details Disables FIQ interrupts by setting special-purpose register FAULTMASK.
           Can only be executed in Privileged modes.
 */
__STATIC_FORCEINLINE void __disable_fault_irq(void)
{
  __ASM volatile ("cpsid f" : : : "memory");
}


/**
  \brief   Get Base Priority
  \details Returns the current value of the Base Priority register.
  \return               Base Priority register value
 */
__STATIC_FORCEINLINE uint32_t __get_BASEPRI(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, basepri" : "=r" (result) );
  return(result);
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Get Base Priority (non-secure)
  \details Returns the current value of the non-secure Base Priority register when in secure state.
  \return               Base Priority register value
 */
__STATIC_FORCEINLINE uint32_t __TZ_get_BASEPRI_NS(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, basepri_ns" : "=r" (result) );
  return(result);
}
#endif


/**
  \brief   Set Base Priority
  \details Assigns the given value to the Base Priority register.
  \param [in]    basePri  Base Priority value to set
 */
__STATIC_FORCEINLINE void __set_BASEPRI(uint32_t basePri)
{
  __ASM volatile ("MSR basepri, %0" : : "r" (basePri) : "memory");
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Set Base Priority (non-secure)
  \details Assigns the given value to the non-secure Base Priority register when in secure state.
  \param [in]    basePri  Base Priority value to set
 */
__STATIC_FORCEINLINE void __TZ_set_BASEPRI_NS(uint32_t basePri)
{
  __ASM volatile ("MSR basepri_ns, %0" : : "r" (basePri) : "memory");
}
#endif


/**
  \brief   Set Base Priority with condition
  \details Assigns the given value to the Base Priority register only if BASEPRI masking is disabled,
           or the new value increases the BASEPRI priority level.
  \param [in]    basePri  Base Priority value to set
 */
__STATIC_FORCEINLINE void __set_BASEPRI_MAX(uint32_t basePri)
{
  __ASM volatile ("MSR basepri_max, %0" : : "r" (basePri) : "memory");
}


/**
  \brief   Get Fault Mask
  \details Returns the current value of the Fault Mask register.
  \return               Fault Mask register value
 */
__STATIC_FORCEINLINE uint32_t __get_FAULTMASK(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, faultmask" : "=r" (result) );
  return(result);
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Get Fault Mask (non-secure)
  \details Returns the current value of the non-secure Fault Mask register when in secure state.
  \return               Fault Mask register value
 */
__STATIC_FORCEINLINE uint32_t __TZ_get_FAULTMASK_NS(void)
{
  uint32_t result;

  __ASM volatile ("MRS %0, faultmask_ns" : "=r" (result) );
  return(result);
}
#endif


/**
  \brief   Set Fault Mask
  \details Assigns the given value to the Fault Mask register.
  \param [in]    faultMask  Fault Mask value to set
 */
__STATIC_FORCEINLINE void __set_FAULTMASK(uint32_t faultMask)
{
  __ASM volatile ("MSR faultmask, %0" : : "r" (faultMask) : "memory");
}


#if (defined (__ARM_FEATURE_CMSE ) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Set Fault Mask (non-secure)
  \details Assigns the given value to the non-secure Fault Mask register when in secure state.
  \param [in]    faultMask  Fault Mask value to set
 */
__STATIC_FORCEINLINE void __TZ_set_FAULTMASK_NS(uint32_t faultMask)
{
  __ASM volatile ("MSR faultmask_ns, %0" : : "r" (faultMask) : "memory");
}
#endif

#endif /* ((defined (__ARM_ARCH_7M__       ) && (__ARM_ARCH_7M__        == 1)) || \
           (defined (__ARM_ARCH_7EM__      ) && (__ARM_ARCH_7EM__       == 1)) || \
           (defined (__ARM_ARCH_8M_MAIN__  ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
           (defined (__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ == 1))     ) */


#if ((defined (__ARM_ARCH_8M_MAIN__  ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
     (defined (__ARM_ARCH_8M_BASE__  ) && (__ARM_ARCH_8M_BASE__   == 1)) || \
     (defined (__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ == 1))     )

/**
  \brief   Get Process Stack Pointer Limit
  Devices without ARMv8-M Main Extensions (i.e. Cortex-M23) lack the non-secure
  Stack Pointer Limit register hence zero is returned always in non-secure
  mode.

  \details Returns the current value of the Process Stack Pointer Limit (PSPLIM).
  \return               PSPLIM Register value
 */
__STATIC_FORCEINLINE uint32_t __get_PSPLIM(void)
{
#if (!((defined (__ARM_ARCH_8M_MAIN__   ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
       (defined (__ARM_ARCH_8_1M_MAIN__ ) && (__ARM_ARCH_8_1M_MAIN__ == 1))   ) && \
    (!defined (__ARM_FEATURE_CMSE) || (__ARM_FEATURE_CMSE < 3)))
    // without main extensions, the non-secure PSPLIM is RAZ/WI
  return 0U;
#else
  uint32_t result;
  __ASM volatile ("MRS %0, psplim"  : "=r" (result) );
  return result;
#endif
}

#if (defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3))
/**
  \brief   Get Process Stack Pointer Limit (non-secure)
  Devices without ARMv8-M Main Extensions (i.e. Cortex-M23) lack the non-secure
  Stack Pointer Limit register hence zero is returned always in non-secure
  mode.

  \details Returns the current value of the non-secure Process Stack Pointer Limit (PSPLIM) when in secure state.
  \return               PSPLIM Register value
 */
__STATIC_FORCEINLINE uint32_t __TZ_get_PSPLIM_NS(void)
{
#if (!((defined (__ARM_ARCH_8M_MAIN__   ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
       (defined (__ARM_ARCH_8_1M_MAIN__ ) && (__ARM_ARCH_8_1M_MAIN__ == 1))   ) )
  // without main extensions, the non-secure PSPLIM is RAZ/WI
  return 0U;
#else
  uint32_t result;
  __ASM volatile ("MRS %0, psplim_ns"  : "=r" (result) );
  return result;
#endif
}
#endif


/**
  \brief   Set Process Stack Pointer Limit
  Devices without ARMv8-M Main Extensions (i.e. Cortex-M23) lack the non-secure
  Stack Pointer Limit register hence the write is silently ignored in non-secure
  mode.

  \details Assigns the given value to the Process Stack Pointer Limit (PSPLIM).
  \param [in]    ProcStackPtrLimit  Process Stack Pointer Limit value to set
 */
__STATIC_FORCEINLINE void __set_PSPLIM(uint32_t ProcStackPtrLimit)
{
#if (!((defined (__ARM_ARCH_8M_MAIN__   ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
       (defined (__ARM_ARCH_8_1M_MAIN__ ) && (__ARM_ARCH_8_1M_MAIN__ == 1))   ) && \
    (!defined (__ARM_FEATURE_CMSE) || (__ARM_FEATURE_CMSE < 3)))
  // without main extensions, the non-secure PSPLIM is RAZ/WI
  (void)ProcStackPtrLimit;
#else
  __ASM volatile ("MSR psplim, %0" : : "r" (ProcStackPtrLimit));
#endif
}


#if (defined (__ARM_FEATURE_CMSE  ) && (__ARM_FEATURE_CMSE   == 3))
/**
  \brief   Set Process Stack Pointer (non-secure)
  Devices without ARMv8-M Main Extensions (i.e. Cortex-M23) lack the non-secure
  Stack Pointer Limit register hence the write is silently ignored in non-secure
  mode.

  \details Assigns the given value to the non-secure Process Stack Pointer Limit (PSPLIM) when in secure state.
  \param [in]    ProcStackPtrLimit  Process Stack Pointer Limit value to set
 */
__STATIC_FORCEINLINE void __TZ_set_PSPLIM_NS(uint32_t ProcStackPtrLimit)
{
#if (!((defined (__ARM_ARCH_8M_MAIN__   ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
       (defined (__ARM_ARCH_8_1M_MAIN__ ) && (__ARM_ARCH_8_1M_MAIN__ == 1))   ) )
  // without main extensions, the non-secure PSPLIM is RAZ/WI
  (void)ProcStackPtrLimit;
#else
  __ASM volatile ("MSR psplim_ns, %0\n" : : "r" (ProcStackPtrLimit));
#endif
}
#endif


/**
  \brief   Get Main Stack Pointer Limit
  Devices without ARMv8-M Main Extensions (i.e. Cortex-M23) lack the non-secure
  Stack Pointer Limit register hence zero is returned always.

  \details Returns the current value of the Main Stack Pointer Limit (MSPLIM).
  \return               MSPLIM Register value
 */
__STATIC_FORCEINLINE uint32_t __get_MSPLIM(void)
{
#if (!((defined (__ARM_ARCH_8M_MAIN__   ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
       (defined (__ARM_ARCH_8_1M_MAIN__ ) && (__ARM_ARCH_8_1M_MAIN__ == 1))   ) && \
    (!defined (__ARM_FEATURE_CMSE) || (__ARM_FEATURE_CMSE < 3)))
  // without main extensions, the non-secure MSPLIM is RAZ/WI
  return 0U;
#else
  uint32_t result;
  __ASM volatile ("MRS %0, msplim" : "=r" (result) );
  return result;
#endif
}


#if (defined (__ARM_FEATURE_CMSE  ) && (__ARM_FEATURE_CMSE   == 3))
/**
  \brief   Get Main Stack Pointer Limit (non-secure)
  Devices without ARMv8-M Main Extensions (i.e. Cortex-M23) lack the non-secure
  Stack Pointer Limit register hence zero is returned always.

  \details Returns the current value of the non-secure Main Stack Pointer Limit(MSPLIM) when in secure state.
  \return               MSPLIM Register value
 */
__STATIC_FORCEINLINE uint32_t __TZ_get_MSPLIM_NS(void)
{
#if (!((defined (__ARM_ARCH_8M_MAIN__   ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
       (defined (__ARM_ARCH_8_1M_MAIN__ ) && (__ARM_ARCH_8_1M_MAIN__ == 1))   ) )
  // without main extensions, the non-secure MSPLIM is RAZ/WI
  return 0U;
#else
  uint32_t result;
  __ASM volatile ("MRS %0, msplim_ns" : "=r" (result) );
  return result;
#endif
}
#endif


/**
  \brief   Set Main Stack Pointer Limit
  Devices without ARMv8-M Main Extensions (i.e. Cortex-M23) lack the non-secure
  Stack Pointer Limit register hence the write is silently ignored.

  \details Assigns the given value to the Main Stack Pointer Limit (MSPLIM).
  \param [in]    MainStackPtrLimit  Main Stack Pointer Limit value to set
 */
__STATIC_FORCEINLINE void __set_MSPLIM(uint32_t MainStackPtrLimit)
{
#if (!((defined (__ARM_ARCH_8M_MAIN__   ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
       (defined (__ARM_ARCH_8_1M_MAIN__ ) && (__ARM_ARCH_8_1M_MAIN__ == 1))   ) && \
    (!defined (__ARM_FEATURE_CMSE) || (__ARM_FEATURE_CMSE < 3)))
  // without main extensions, the non-secure MSPLIM is RAZ/WI
  (void)MainStackPtrLimit;
#else
  __ASM volatile ("MSR msplim, %0" : : "r" (MainStackPtrLimit));
#endif
}


#if (defined (__ARM_FEATURE_CMSE  ) && (__ARM_FEATURE_CMSE   == 3))
/**
  \brief   Set Main Stack Pointer Limit (non-secure)
  Devices without ARMv8-M Main Extensions (i.e. Cortex-M23) lack the non-secure
  Stack Pointer Limit register hence the write is silently ignored.

  \details Assigns the given value to the non-secure Main Stack Pointer Limit (MSPLIM) when in secure state.
  \param [in]    MainStackPtrLimit  Main Stack Pointer value to set
 */
__STATIC_FORCEINLINE void __TZ_set_MSPLIM_NS(uint32_t MainStackPtrLimit)
{
#if (!((defined (__ARM_ARCH_8M_MAIN__   ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
       (defined (__ARM_ARCH_8_1M_MAIN__ ) && (__ARM_ARCH_8_1M_MAIN__ == 1))   ) )
  // without main extensions, the non-secure MSPLIM is RAZ/WI
  (void)MainStackPtrLimit;
#else
  __ASM volatile ("MSR msplim_ns, %0" : : "r" (MainStackPtrLimit));
#endif
}
#endif

#endif /* ((defined (__ARM_ARCH_8M_MAIN__  ) && (__ARM_ARCH_8M_MAIN__   == 1)) || \
           (defined (__ARM_ARCH_8M_BASE__  ) && (__ARM_ARCH_8M_BASE__   == 1)) || \
           (defined (__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ == 1))     ) */

/**
  \brief   Get FPSCR
  \details Returns the current value of the Floating Point Status/Control register.
  \return               Floating Point Status/Control register value
 */
#if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined (__FPU_USED   ) && (__FPU_USED    == 1U))     )
#define __get_FPSCR      (uint32_t)__builtin_arm_get_fpscr
#else
#define __get_FPSCR()      ((uint32_t)0U)
#endif

/**
  \brief   Set FPSCR
  \details Assigns the given value to the Floating Point Status/Control register.
  \param [in]    fpscr  Floating Point Status/Control value to set
 */
#if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined (__FPU_USED   ) && (__FPU_USED    == 1U))     )
#define __set_FPSCR      __builtin_arm_set_fpscr
#else
#define __set_FPSCR(fpscr)      ((void)(fpscr))
#endif


/** @} end of CMSIS_Core_RegAccFunctions */


/* ###################  Compiler specific Intrinsics  ########################### */
/** \defgroup CMSIS_SIMD_intrinsics CMSIS SIMD Intrinsics
  Access to dedicated SIMD instructions
  @{
*/

#if (defined (__ARM_FEATURE_DSP) && (__ARM_FEATURE_DSP == 1))

#define     __SADD8                 __builtin_arm_sadd8
#define     __QADD8                 __builtin_arm_qadd8
#define     __SHADD8                __builtin_arm_shadd8
#define     __UADD8                 __builtin_arm_uadd8
#define     __UQADD8                __builtin_arm_uqadd8
#define     __UHADD8                __builtin_arm_uhadd8
#define     __SSUB8                 __builtin_arm_ssub8
#define     __QSUB8                 __builtin_arm_qsub8
#define     __SHSUB8                __builtin_arm_shsub8
#define     __USUB8                 __builtin_arm_usub8
#define     __UQSUB8                __builtin_arm_uqsub8
#define     __UHSUB8                __builtin_arm_uhsub8
#define     __SADD16                __builtin_arm_sadd16
#define     __QADD16                __builtin_arm_qadd16
#define     __SHADD16               __builtin_arm_shadd16
#define     __UADD16                __builtin_arm_uadd16
#define     __UQADD16               __builtin_arm_uqadd16
#define     __UHADD16               __builtin_arm_uhadd16
#define     __SSUB16                __builtin_arm_ssub16
#define     __QSUB16                __builtin_arm_qsub16
#define     __SHSUB16               __builtin_arm_shsub16
#define     __USUB16                __builtin_arm_usub16
#define     __UQSUB16               __builtin_arm_uqsub16
#define     __UHSUB16               __builtin_arm_uhsub16
#define     __SASX                  __builtin_arm_sasx
#define     __QASX                  __builtin_arm_qasx
#define     __SHASX                 __builtin_arm_shasx
#define     __UASX                  __builtin_arm_uasx
#define     __UQASX                 __builtin_arm_uqasx
#define     __UHASX                 __builtin_arm_uhasx
#define     __SSAX                  __builtin_arm_ssax
#define     __QSAX                  __builtin_arm_qsax
#define     __SHSAX                 __builtin_arm_shsax
#define     __USAX                  __builtin_arm_usax
#define     __UQSAX                 __builtin_arm_uqsax
#define     __UHSAX                 __builtin_arm_uhsax
#define     __USAD8                 __builtin_arm_usad8
#define     __USADA8                __builtin_arm_usada8
#define     __SSAT16                __builtin_arm_ssat16
#define     __USAT16                __builtin_arm_usat16
#define     __UXTB16                __builtin_arm_uxtb16
#define     __UXTAB16               __builtin_arm_uxtab16
#define     __SXTB16                __builtin_arm_sxtb16
#define     __SXTAB16               __builtin_arm_sxtab16
#define     __SMUAD                 __builtin_arm_smuad
#define     __SMUADX                __builtin_arm_smuadx
#define     __SMLAD                 __builtin_arm_smlad
#define     __SMLADX                __builtin_arm_smladx
#define     __SMLALD                __builtin_arm_smlald
#define     __SMLALDX               __builtin_arm_smlaldx
#define     __SMUSD                 __builtin_arm_smusd
#define     __SMUSDX                __builtin_arm_smusdx
#define     __SMLSD                 __builtin_arm_smlsd
#define     __SMLSDX                __builtin_arm_smlsdx
#define     __SMLSLD                __builtin_arm_smlsld
#define     __SMLSLDX               __builtin_arm_smlsldx
#define     __SEL                   __builtin_arm_sel
#define     __QADD                  __builtin_arm_qadd
#define     __QSUB                  __builtin_arm_qsub

#define __PKHBT(ARG1,ARG2,ARG3)          ( ((((uint32_t)(ARG1))          ) & 0x0000FFFFUL) |  \
                                           ((((uint32_t)(ARG2)) << (ARG3)) & 0xFFFF0000UL)  )

#define __PKHTB(ARG1,ARG2,ARG3)          ( ((((uint32_t)(ARG1))          ) & 0xFFFF0000UL) |  \
                                           ((((uint32_t)(ARG2)) >> (ARG3)) & 0x0000FFFFUL)  )

#define __SXTB16_RORn(ARG1, ARG2)        __SXTB16(__ROR(ARG1, ARG2))

#define __SXTAB16_RORn(ARG1, ARG2, ARG3) __SXTAB16(ARG1, __ROR(ARG2, ARG3))

__STATIC_FORCEINLINE int32_t __SMMLA (int32_t op1, int32_t op2, int32_t op3)
{
  int32_t result;

  __ASM volatile ("smmla %0, %1, %2, %3" : "=r" (result): "r"  (op1), "r" (op2), "r" (op3) );
  return(result);
}

#endif /* (__ARM_FEATURE_DSP == 1) */
/** @} end of group CMSIS_SIMD_intrinsics */


#endif /* __CMSIS_ARMCLANG_H */
