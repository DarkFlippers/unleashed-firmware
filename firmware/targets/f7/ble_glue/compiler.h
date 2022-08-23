/*****************************************************************************
 * @file    compiler.h
 * @author  MDG
 * @brief   This file contains the definitions which are compiler dependent.
 *****************************************************************************
 * @attention
 *
 * Copyright (c) 2018-2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 *****************************************************************************
 */

#ifndef COMPILER_H__
#define COMPILER_H__

/**
  * @brief  This is the section dedicated to IAR toolchain
  */
#if defined(__ICCARM__) || defined(__IAR_SYSTEMS_ASM__)

#ifndef __WEAK
#define __WEAK __weak
#endif

#define QUOTE_(a) #a

/**
 * @brief  PACKED
 *         Use the PACKED macro for variables that needs to be packed.
 *         Usage:  PACKED(struct) myStruct_s
 *                 PACKED(union) myStruct_s
 */
#define PACKED(decl) __packed decl

/**
 * @brief  SECTION
 *         Use the SECTION macro to assign data or code in a specific section.
 *         Usage:  SECTION(".my_section")
 */
#define SECTION(name) _Pragma(QUOTE_(location = name))

/**
 * @brief  ALIGN_DEF
 *         Use the ALIGN_DEF macro to specify the alignment of a variable.
 *         Usage:  ALIGN_DEF(4)
 */
#define ALIGN_DEF(v) _Pragma(QUOTE_(data_alignment = v))

/**
 * @brief  NO_INIT
 *         Use the NO_INIT macro to declare a not initialized variable.
 *         Usage:  NO_INIT(int my_no_init_var)
 *         Usage:  NO_INIT(uint16_t my_no_init_array[10])
 */
#define NO_INIT(var) __no_init var

/**
 * @brief  This is the section dedicated to GNU toolchain
 */
#else
#ifdef __GNUC__

#ifndef __WEAK
#define __WEAK __attribute__((weak))
#endif

/**
 * @brief  PACKED
 *         Use the PACKED macro for variables that needs to be packed.
 *         Usage:  PACKED(struct) myStruct_s
 *                 PACKED(union) myStruct_s
 */
#define PACKED(decl) decl __attribute__((packed))

/**
 * @brief  SECTION
 *         Use the SECTION macro to assign data or code in a specific section.
 *         Usage:  SECTION(".my_section")
 */
#define SECTION(name) __attribute__((section(name)))

/**
 * @brief  ALIGN_DEF
 *         Use the ALIGN_DEF macro to specify the alignment of a variable.
 *         Usage:  ALIGN_DEF(4)
 */
#define ALIGN_DEF(N) __attribute__((aligned(N)))

/**
 * @brief  NO_INIT
 *         Use the NO_INIT macro to declare a not initialized variable.
 *         Usage:  NO_INIT(int my_no_init_var)
 *         Usage:  NO_INIT(uint16_t my_no_init_array[10])
 */
#define NO_INIT(var) var __attribute__((section(".noinit")))

/**
 * @brief  This is the section dedicated to Keil toolchain
 */
#else
#ifdef __CC_ARM

#ifndef __WEAK
#define __WEAK __weak
#endif

/**
 * @brief  PACKED
 *         Use the PACKED macro for variables that needs to be packed.
 *         Usage:  PACKED(struct) myStruct_s
 *                 PACKED(union) myStruct_s
 */
#define PACKED(decl) decl __attribute__((packed))

/**
 * @brief  SECTION
 *         Use the SECTION macro to assign data or code in a specific section.
 *         Usage:  SECTION(".my_section")
 */
#define SECTION(name) __attribute__((section(name)))

/**
 * @brief  ALIGN_DEF
 *         Use the ALIGN_DEF macro to specify the alignment of a variable.
 *         Usage:  ALIGN_DEF(4)
 */
#define ALIGN_DEF(N) __attribute__((aligned(N)))

/**
 * @brief  NO_INIT
 *         Use the NO_INIT macro to declare a not initialized variable.
 *         Usage:  NO_INIT(int my_no_init_var)
 *         Usage:  NO_INIT(uint16_t my_no_init_array[10])
 */
#define NO_INIT(var) var __attribute__((section("NoInit")))

#else

#error Neither ICCARM, CC ARM nor GNUC C detected. Define your macros.

#endif
#endif
#endif

#endif /* COMPILER_H__ */
