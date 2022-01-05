
/******************************************************************************
  * \attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/

/*
 *      PROJECT:   ST25R391x firmware
 *      Revision: 
 *      LANGUAGE:  ISO C99
 */

/*! \file rfal_analogConfig.h
 *
 *  \author bkam
 *
 *  \brief ST25R3916 Analog Configuration Settings
 *  
 */

#ifndef ST25R3916_ANALOGCONFIG_H
#define ST25R3916_ANALOGCONFIG_H

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_analogConfig.h"
#include "st25r3916_com.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */

/*! Macro for Configuration Setting with only one register-mask-value set: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1] */
#define MODE_ENTRY_1_REG(MODE, R0, M0, V0)                         \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 1, \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0)

/*! Macro for Configuration Setting with only two register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1] */
#define MODE_ENTRY_2_REG(MODE, R0, M0, V0, R1, M1, V1)                                        \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 2,                            \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0), \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1)

/*! Macro for Configuration Setting with only three register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_3_REG(MODE, R0, M0, V0, R1, M1, V1, R2, M2, V2)                            \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 3,                            \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0), \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1), \
        (uint8_t)((uint16_t)(R2) >> 8U), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2)

/*! Macro for Configuration Setting with only four register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_4_REG(MODE, R0, M0, V0, R1, M1, V1, R2, M2, V2, R3, M3, V3)                \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 4,                            \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0), \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1), \
        (uint8_t)((uint16_t)(R2) >> 8U), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2), \
        (uint8_t)((uint16_t)(R3) >> 8U), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3)

/*! Macro for Configuration Setting with only five register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_5_REG(MODE, R0, M0, V0, R1, M1, V1, R2, M2, V2, R3, M3, V3, R4, M4, V4)    \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 5,                            \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0), \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1), \
        (uint8_t)((uint16_t)(R2) >> 8U), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2), \
        (uint8_t)((uint16_t)(R3) >> 8U), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3), \
        (uint8_t)((uint16_t)(R4) >> 8U), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4)

/*! Macro for Configuration Setting with only six register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_6_REG(                                                                     \
    MODE, R0, M0, V0, R1, M1, V1, R2, M2, V2, R3, M3, V3, R4, M4, V4, R5, M5, V5)             \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 6,                            \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0), \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1), \
        (uint8_t)((uint16_t)(R2) >> 8U), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2), \
        (uint8_t)((uint16_t)(R3) >> 8U), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3), \
        (uint8_t)((uint16_t)(R4) >> 8U), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4), \
        (uint8_t)((uint16_t)(R5) >> 8U), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5)

/*! Macro for Configuration Setting with only seven register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_7_REG(                                                                     \
    MODE, R0, M0, V0, R1, M1, V1, R2, M2, V2, R3, M3, V3, R4, M4, V4, R5, M5, V5, R6, M6, V6) \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 7,                            \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0), \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1), \
        (uint8_t)((uint16_t)(R2) >> 8U), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2), \
        (uint8_t)((uint16_t)(R3) >> 8U), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3), \
        (uint8_t)((uint16_t)(R4) >> 8U), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4), \
        (uint8_t)((uint16_t)(R5) >> 8U), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5), \
        (uint8_t)((uint16_t)(R6) >> 8U), (uint8_t)((R6)&0xFFU), (uint8_t)(M6), (uint8_t)(V6)

/*! Macro for Configuration Setting with only eight register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_8_REG(                                                                     \
    MODE,                                                                                     \
    R0,                                                                                       \
    M0,                                                                                       \
    V0,                                                                                       \
    R1,                                                                                       \
    M1,                                                                                       \
    V1,                                                                                       \
    R2,                                                                                       \
    M2,                                                                                       \
    V2,                                                                                       \
    R3,                                                                                       \
    M3,                                                                                       \
    V3,                                                                                       \
    R4,                                                                                       \
    M4,                                                                                       \
    V4,                                                                                       \
    R5,                                                                                       \
    M5,                                                                                       \
    V5,                                                                                       \
    R6,                                                                                       \
    M6,                                                                                       \
    V6,                                                                                       \
    R7,                                                                                       \
    M7,                                                                                       \
    V7)                                                                                       \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 8,                            \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0), \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1), \
        (uint8_t)((uint16_t)(R2) >> 8U), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2), \
        (uint8_t)((uint16_t)(R3) >> 8U), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3), \
        (uint8_t)((uint16_t)(R4) >> 8U), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4), \
        (uint8_t)((uint16_t)(R5) >> 8U), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5), \
        (uint8_t)((uint16_t)(R6) >> 8U), (uint8_t)((R6)&0xFFU), (uint8_t)(M6), (uint8_t)(V6), \
        (uint8_t)((uint16_t)(R7) >> 8U), (uint8_t)((R7)&0xFFU), (uint8_t)(M7), (uint8_t)(V7)

/*! Macro for Configuration Setting with only nine register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_9_REG(                                                                     \
    MODE,                                                                                     \
    R0,                                                                                       \
    M0,                                                                                       \
    V0,                                                                                       \
    R1,                                                                                       \
    M1,                                                                                       \
    V1,                                                                                       \
    R2,                                                                                       \
    M2,                                                                                       \
    V2,                                                                                       \
    R3,                                                                                       \
    M3,                                                                                       \
    V3,                                                                                       \
    R4,                                                                                       \
    M4,                                                                                       \
    V4,                                                                                       \
    R5,                                                                                       \
    M5,                                                                                       \
    V5,                                                                                       \
    R6,                                                                                       \
    M6,                                                                                       \
    V6,                                                                                       \
    R7,                                                                                       \
    M7,                                                                                       \
    V7,                                                                                       \
    R8,                                                                                       \
    M8,                                                                                       \
    V8)                                                                                       \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 9,                            \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0), \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1), \
        (uint8_t)((uint16_t)(R2) >> 8U), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2), \
        (uint8_t)((uint16_t)(R3) >> 8U), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3), \
        (uint8_t)((uint16_t)(R4) >> 8U), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4), \
        (uint8_t)((uint16_t)(R5) >> 8U), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5), \
        (uint8_t)((uint16_t)(R6) >> 8U), (uint8_t)((R6)&0xFFU), (uint8_t)(M6), (uint8_t)(V6), \
        (uint8_t)((uint16_t)(R7) >> 8U), (uint8_t)((R7)&0xFFU), (uint8_t)(M7), (uint8_t)(V7), \
        (uint8_t)((uint16_t)(R8) >> 8U), (uint8_t)((R8)&0xFFU), (uint8_t)(M8), (uint8_t)(V8)

/*! Macro for Configuration Setting with only ten register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_10_REG(                                                                    \
    MODE,                                                                                     \
    R0,                                                                                       \
    M0,                                                                                       \
    V0,                                                                                       \
    R1,                                                                                       \
    M1,                                                                                       \
    V1,                                                                                       \
    R2,                                                                                       \
    M2,                                                                                       \
    V2,                                                                                       \
    R3,                                                                                       \
    M3,                                                                                       \
    V3,                                                                                       \
    R4,                                                                                       \
    M4,                                                                                       \
    V4,                                                                                       \
    R5,                                                                                       \
    M5,                                                                                       \
    V5,                                                                                       \
    R6,                                                                                       \
    M6,                                                                                       \
    V6,                                                                                       \
    R7,                                                                                       \
    M7,                                                                                       \
    V7,                                                                                       \
    R8,                                                                                       \
    M8,                                                                                       \
    V8,                                                                                       \
    R9,                                                                                       \
    M9,                                                                                       \
    V9)                                                                                       \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 10,                           \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0), \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1), \
        (uint8_t)((uint16_t)(R2) >> 8U), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2), \
        (uint8_t)((uint16_t)(R3) >> 8U), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3), \
        (uint8_t)((uint16_t)(R4) >> 8U), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4), \
        (uint8_t)((uint16_t)(R5) >> 8U), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5), \
        (uint8_t)((uint16_t)(R6) >> 8U), (uint8_t)((R6)&0xFFU), (uint8_t)(M6), (uint8_t)(V6), \
        (uint8_t)((uint16_t)(R7) >> 8U), (uint8_t)((R7)&0xFFU), (uint8_t)(M7), (uint8_t)(V7), \
        (uint8_t)((uint16_t)(R8) >> 8U), (uint8_t)((R8)&0xFFU), (uint8_t)(M8), (uint8_t)(V8), \
        (uint8_t)((uint16_t)(R9) >> 8U), (uint8_t)((R9)&0xFFU), (uint8_t)(M9), (uint8_t)(V9)

/*! Macro for Configuration Setting with eleven register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_11_REG(                                                                    \
    MODE,                                                                                     \
    R0,                                                                                       \
    M0,                                                                                       \
    V0,                                                                                       \
    R1,                                                                                       \
    M1,                                                                                       \
    V1,                                                                                       \
    R2,                                                                                       \
    M2,                                                                                       \
    V2,                                                                                       \
    R3,                                                                                       \
    M3,                                                                                       \
    V3,                                                                                       \
    R4,                                                                                       \
    M4,                                                                                       \
    V4,                                                                                       \
    R5,                                                                                       \
    M5,                                                                                       \
    V5,                                                                                       \
    R6,                                                                                       \
    M6,                                                                                       \
    V6,                                                                                       \
    R7,                                                                                       \
    M7,                                                                                       \
    V7,                                                                                       \
    R8,                                                                                       \
    M8,                                                                                       \
    V8,                                                                                       \
    R9,                                                                                       \
    M9,                                                                                       \
    V9,                                                                                       \
    R10,                                                                                      \
    M10,                                                                                      \
    V10)                                                                                      \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 11,                           \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0), \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1), \
        (uint8_t)((uint16_t)(R2) >> 8U), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2), \
        (uint8_t)((uint16_t)(R3) >> 8U), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3), \
        (uint8_t)((uint16_t)(R4) >> 8U), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4), \
        (uint8_t)((uint16_t)(R5) >> 8U), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5), \
        (uint8_t)((uint16_t)(R6) >> 8U), (uint8_t)((R6)&0xFFU), (uint8_t)(M6), (uint8_t)(V6), \
        (uint8_t)((uint16_t)(R7) >> 8U), (uint8_t)((R7)&0xFFU), (uint8_t)(M7), (uint8_t)(V7), \
        (uint8_t)((uint16_t)(R8) >> 8U), (uint8_t)((R8)&0xFFU), (uint8_t)(M8), (uint8_t)(V8), \
        (uint8_t)((uint16_t)(R9) >> 8U), (uint8_t)((R9)&0xFFU), (uint8_t)(M9), (uint8_t)(V9), \
        (uint8_t)((uint16_t)(R10) >> 8U), (uint8_t)((R10)&0xFFU), (uint8_t)(M10), (uint8_t)(V10)

/*! Macro for Configuration Setting with twelve register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_12_REG(                                                                        \
    MODE,                                                                                         \
    R0,                                                                                           \
    M0,                                                                                           \
    V0,                                                                                           \
    R1,                                                                                           \
    M1,                                                                                           \
    V1,                                                                                           \
    R2,                                                                                           \
    M2,                                                                                           \
    V2,                                                                                           \
    R3,                                                                                           \
    M3,                                                                                           \
    V3,                                                                                           \
    R4,                                                                                           \
    M4,                                                                                           \
    V4,                                                                                           \
    R5,                                                                                           \
    M5,                                                                                           \
    V5,                                                                                           \
    R6,                                                                                           \
    M6,                                                                                           \
    V6,                                                                                           \
    R7,                                                                                           \
    M7,                                                                                           \
    V7,                                                                                           \
    R8,                                                                                           \
    M8,                                                                                           \
    V8,                                                                                           \
    R9,                                                                                           \
    M9,                                                                                           \
    V9,                                                                                           \
    R10,                                                                                          \
    M10,                                                                                          \
    V10,                                                                                          \
    R11,                                                                                          \
    M11,                                                                                          \
    V11)                                                                                          \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 12,                               \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0),     \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1),     \
        (uint8_t)((uint16_t)(R2) >> 8U), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2),     \
        (uint8_t)((uint16_t)(R3) >> 8U), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3),     \
        (uint8_t)((uint16_t)(R4) >> 8U), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4),     \
        (uint8_t)((uint16_t)(R5) >> 8U), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5),     \
        (uint8_t)((uint16_t)(R6) >> 8U), (uint8_t)((R6)&0xFFU), (uint8_t)(M6), (uint8_t)(V6),     \
        (uint8_t)((uint16_t)(R7) >> 8U), (uint8_t)((R7)&0xFFU), (uint8_t)(M7), (uint8_t)(V7),     \
        (uint8_t)((uint16_t)(R8) >> 8U), (uint8_t)((R8)&0xFFU), (uint8_t)(M8), (uint8_t)(V8),     \
        (uint8_t)((uint16_t)(R9) >> 8U), (uint8_t)((R9)&0xFFU), (uint8_t)(M9), (uint8_t)(V9),     \
        (uint8_t)((uint16_t)(R10) >> 8U), (uint8_t)((R10)&0xFFU), (uint8_t)(M10), (uint8_t)(V10), \
        (uint8_t)((uint16_t)(R11) >> 8U), (uint8_t)((R11)&0xFFU), (uint8_t)(M11), (uint8_t)(V11)

/*! Macro for Configuration Setting with thirteen register-mask-value sets: 
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_13_REG(                                                                        \
    MODE,                                                                                         \
    R0,                                                                                           \
    M0,                                                                                           \
    V0,                                                                                           \
    R1,                                                                                           \
    M1,                                                                                           \
    V1,                                                                                           \
    R2,                                                                                           \
    M2,                                                                                           \
    V2,                                                                                           \
    R3,                                                                                           \
    M3,                                                                                           \
    V3,                                                                                           \
    R4,                                                                                           \
    M4,                                                                                           \
    V4,                                                                                           \
    R5,                                                                                           \
    M5,                                                                                           \
    V5,                                                                                           \
    R6,                                                                                           \
    M6,                                                                                           \
    V6,                                                                                           \
    R7,                                                                                           \
    M7,                                                                                           \
    V7,                                                                                           \
    R8,                                                                                           \
    M8,                                                                                           \
    V8,                                                                                           \
    R9,                                                                                           \
    M9,                                                                                           \
    V9,                                                                                           \
    R10,                                                                                          \
    M10,                                                                                          \
    V10,                                                                                          \
    R11,                                                                                          \
    M11,                                                                                          \
    V11,                                                                                          \
    R12,                                                                                          \
    M12,                                                                                          \
    V12)                                                                                          \
    (uint8_t)((uint16_t)(MODE) >> 8U), (uint8_t)((MODE)&0xFFU), 13,                               \
        (uint8_t)((uint16_t)(R0) >> 8U), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0),     \
        (uint8_t)((uint16_t)(R1) >> 8U), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1),     \
        (uint8_t)((uint16_t)(R2) >> 8U), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2),     \
        (uint8_t)((uint16_t)(R3) >> 8U), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3),     \
        (uint8_t)((uint16_t)(R4) >> 8U), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4),     \
        (uint8_t)((uint16_t)(R5) >> 8U), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5),     \
        (uint8_t)((uint16_t)(R6) >> 8U), (uint8_t)((R6)&0xFFU), (uint8_t)(M6), (uint8_t)(V6),     \
        (uint8_t)((uint16_t)(R7) >> 8U), (uint8_t)((R7)&0xFFU), (uint8_t)(M7), (uint8_t)(V7),     \
        (uint8_t)((uint16_t)(R8) >> 8U), (uint8_t)((R8)&0xFFU), (uint8_t)(M8), (uint8_t)(V8),     \
        (uint8_t)((uint16_t)(R9) >> 8U), (uint8_t)((R9)&0xFFU), (uint8_t)(M9), (uint8_t)(V9),     \
        (uint8_t)((uint16_t)(R10) >> 8U), (uint8_t)((R10)&0xFFU), (uint8_t)(M10), (uint8_t)(V10), \
        (uint8_t)((uint16_t)(R11) >> 8U), (uint8_t)((R11)&0xFFU), (uint8_t)(M11), (uint8_t)(V11), \
        (uint8_t)((uint16_t)(R12) >> 8U), (uint8_t)((R12)&0xFFU), (uint8_t)(M12), (uint8_t)(V12)

/*! Macro for Configuration Setting with fourteen register-mask-value sets:
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_14_REG(                                                                       \
    MODE,                                                                                        \
    R0,                                                                                          \
    M0,                                                                                          \
    V0,                                                                                          \
    R1,                                                                                          \
    M1,                                                                                          \
    V1,                                                                                          \
    R2,                                                                                          \
    M2,                                                                                          \
    V2,                                                                                          \
    R3,                                                                                          \
    M3,                                                                                          \
    V3,                                                                                          \
    R4,                                                                                          \
    M4,                                                                                          \
    V4,                                                                                          \
    R5,                                                                                          \
    M5,                                                                                          \
    V5,                                                                                          \
    R6,                                                                                          \
    M6,                                                                                          \
    V6,                                                                                          \
    R7,                                                                                          \
    M7,                                                                                          \
    V7,                                                                                          \
    R8,                                                                                          \
    M8,                                                                                          \
    V8,                                                                                          \
    R9,                                                                                          \
    M9,                                                                                          \
    V9,                                                                                          \
    R10,                                                                                         \
    M10,                                                                                         \
    V10,                                                                                         \
    R11,                                                                                         \
    M11,                                                                                         \
    V11,                                                                                         \
    R12,                                                                                         \
    M12,                                                                                         \
    V12,                                                                                         \
    R13,                                                                                         \
    M13,                                                                                         \
    V13,                                                                                         \
    R14,                                                                                         \
    M14,                                                                                         \
    V14,                                                                                         \
    R15,                                                                                         \
    M15,                                                                                         \
    V15)                                                                                         \
    (uint8_t)((uint16_t)(MODE) >> 8), (uint8_t)((MODE)&0xFFU), 14,                               \
        (uint8_t)((uint16_t)(R0) >> 8), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0),     \
        (uint8_t)((uint16_t)(R1) >> 8), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1),     \
        (uint8_t)((uint16_t)(R2) >> 8), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2),     \
        (uint8_t)((uint16_t)(R3) >> 8), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3),     \
        (uint8_t)((uint16_t)(R4) >> 8), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4),     \
        (uint8_t)((uint16_t)(R5) >> 8), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5),     \
        (uint8_t)((uint16_t)(R6) >> 8), (uint8_t)((R6)&0xFFU), (uint8_t)(M6), (uint8_t)(V6),     \
        (uint8_t)((uint16_t)(R7) >> 8), (uint8_t)((R7)&0xFFU), (uint8_t)(M7), (uint8_t)(V7),     \
        (uint8_t)((uint16_t)(R8) >> 8), (uint8_t)((R8)&0xFFU), (uint8_t)(M8), (uint8_t)(V8),     \
        (uint8_t)((uint16_t)(R9) >> 8), (uint8_t)((R9)&0xFFU), (uint8_t)(M9), (uint8_t)(V9),     \
        (uint8_t)((uint16_t)(R10) >> 8), (uint8_t)((R10)&0xFFU), (uint8_t)(M10), (uint8_t)(V10), \
        (uint8_t)((uint16_t)(R11) >> 8), (uint8_t)((R11)&0xFFU), (uint8_t)(M11), (uint8_t)(V11), \
        (uint8_t)((uint16_t)(R12) >> 8), (uint8_t)((R12)&0xFFU), (uint8_t)(M12), (uint8_t)(V12), \
        (uint8_t)((uint16_t)(R13) >> 8), (uint8_t)((R13)&0xFFU), (uint8_t)(M13), (uint8_t)(V13)

/*! Macro for Configuration Setting with fifteen register-mask-value sets:
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_15_REG(                                                                       \
    MODE,                                                                                        \
    R0,                                                                                          \
    M0,                                                                                          \
    V0,                                                                                          \
    R1,                                                                                          \
    M1,                                                                                          \
    V1,                                                                                          \
    R2,                                                                                          \
    M2,                                                                                          \
    V2,                                                                                          \
    R3,                                                                                          \
    M3,                                                                                          \
    V3,                                                                                          \
    R4,                                                                                          \
    M4,                                                                                          \
    V4,                                                                                          \
    R5,                                                                                          \
    M5,                                                                                          \
    V5,                                                                                          \
    R6,                                                                                          \
    M6,                                                                                          \
    V6,                                                                                          \
    R7,                                                                                          \
    M7,                                                                                          \
    V7,                                                                                          \
    R8,                                                                                          \
    M8,                                                                                          \
    V8,                                                                                          \
    R9,                                                                                          \
    M9,                                                                                          \
    V9,                                                                                          \
    R10,                                                                                         \
    M10,                                                                                         \
    V10,                                                                                         \
    R11,                                                                                         \
    M11,                                                                                         \
    V11,                                                                                         \
    R12,                                                                                         \
    M12,                                                                                         \
    V12,                                                                                         \
    R13,                                                                                         \
    M13,                                                                                         \
    V13,                                                                                         \
    R14,                                                                                         \
    M14,                                                                                         \
    V14,                                                                                         \
    R15,                                                                                         \
    M15,                                                                                         \
    V15)                                                                                         \
    (uint8_t)((uint16_t)(MODE) >> 8), (uint8_t)((MODE)&0xFFU), 15,                               \
        (uint8_t)((uint16_t)(R0) >> 8), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0),     \
        (uint8_t)((uint16_t)(R1) >> 8), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1),     \
        (uint8_t)((uint16_t)(R2) >> 8), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2),     \
        (uint8_t)((uint16_t)(R3) >> 8), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3),     \
        (uint8_t)((uint16_t)(R4) >> 8), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4),     \
        (uint8_t)((uint16_t)(R5) >> 8), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5),     \
        (uint8_t)((uint16_t)(R6) >> 8), (uint8_t)((R6)&0xFFU), (uint8_t)(M6), (uint8_t)(V6),     \
        (uint8_t)((uint16_t)(R7) >> 8), (uint8_t)((R7)&0xFFU), (uint8_t)(M7), (uint8_t)(V7),     \
        (uint8_t)((uint16_t)(R8) >> 8), (uint8_t)((R8)&0xFFU), (uint8_t)(M8), (uint8_t)(V8),     \
        (uint8_t)((uint16_t)(R9) >> 8), (uint8_t)((R9)&0xFFU), (uint8_t)(M9), (uint8_t)(V9),     \
        (uint8_t)((uint16_t)(R10) >> 8), (uint8_t)((R10)&0xFFU), (uint8_t)(M10), (uint8_t)(V10), \
        (uint8_t)((uint16_t)(R11) >> 8), (uint8_t)((R11)&0xFFU), (uint8_t)(M11), (uint8_t)(V11), \
        (uint8_t)((uint16_t)(R12) >> 8), (uint8_t)((R12)&0xFFU), (uint8_t)(M12), (uint8_t)(V12), \
        (uint8_t)((uint16_t)(R13) >> 8), (uint8_t)((R13)&0xFFU), (uint8_t)(M13), (uint8_t)(V13), \
        (uint8_t)((uint16_t)(R14) >> 8), (uint8_t)((R14)&0xFFU), (uint8_t)(M14), (uint8_t)(V14)

/*! Macro for Configuration Setting with sixteen register-mask-value sets:
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_16_REG(                                                                       \
    MODE,                                                                                        \
    R0,                                                                                          \
    M0,                                                                                          \
    V0,                                                                                          \
    R1,                                                                                          \
    M1,                                                                                          \
    V1,                                                                                          \
    R2,                                                                                          \
    M2,                                                                                          \
    V2,                                                                                          \
    R3,                                                                                          \
    M3,                                                                                          \
    V3,                                                                                          \
    R4,                                                                                          \
    M4,                                                                                          \
    V4,                                                                                          \
    R5,                                                                                          \
    M5,                                                                                          \
    V5,                                                                                          \
    R6,                                                                                          \
    M6,                                                                                          \
    V6,                                                                                          \
    R7,                                                                                          \
    M7,                                                                                          \
    V7,                                                                                          \
    R8,                                                                                          \
    M8,                                                                                          \
    V8,                                                                                          \
    R9,                                                                                          \
    M9,                                                                                          \
    V9,                                                                                          \
    R10,                                                                                         \
    M10,                                                                                         \
    V10,                                                                                         \
    R11,                                                                                         \
    M11,                                                                                         \
    V11,                                                                                         \
    R12,                                                                                         \
    M12,                                                                                         \
    V12,                                                                                         \
    R13,                                                                                         \
    M13,                                                                                         \
    V13,                                                                                         \
    R14,                                                                                         \
    M14,                                                                                         \
    V14,                                                                                         \
    R15,                                                                                         \
    M15,                                                                                         \
    V15)                                                                                         \
    (uint8_t)((uint16_t)(MODE) >> 8), (uint8_t)((MODE)&0xFFU), 16,                               \
        (uint8_t)((uint16_t)(R0) >> 8), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0),     \
        (uint8_t)((uint16_t)(R1) >> 8), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1),     \
        (uint8_t)((uint16_t)(R2) >> 8), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2),     \
        (uint8_t)((uint16_t)(R3) >> 8), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3),     \
        (uint8_t)((uint16_t)(R4) >> 8), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4),     \
        (uint8_t)((uint16_t)(R5) >> 8), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5),     \
        (uint8_t)((uint16_t)(R6) >> 8), (uint8_t)((R6)&0xFFU), (uint8_t)(M6), (uint8_t)(V6),     \
        (uint8_t)((uint16_t)(R7) >> 8), (uint8_t)((R7)&0xFFU), (uint8_t)(M7), (uint8_t)(V7),     \
        (uint8_t)((uint16_t)(R8) >> 8), (uint8_t)((R8)&0xFFU), (uint8_t)(M8), (uint8_t)(V8),     \
        (uint8_t)((uint16_t)(R9) >> 8), (uint8_t)((R9)&0xFFU), (uint8_t)(M9), (uint8_t)(V9),     \
        (uint8_t)((uint16_t)(R10) >> 8), (uint8_t)((R10)&0xFFU), (uint8_t)(M10), (uint8_t)(V10), \
        (uint8_t)((uint16_t)(R11) >> 8), (uint8_t)((R11)&0xFFU), (uint8_t)(M11), (uint8_t)(V11), \
        (uint8_t)((uint16_t)(R12) >> 8), (uint8_t)((R12)&0xFFU), (uint8_t)(M12), (uint8_t)(V12), \
        (uint8_t)((uint16_t)(R13) >> 8), (uint8_t)((R13)&0xFFU), (uint8_t)(M13), (uint8_t)(V13), \
        (uint8_t)((uint16_t)(R14) >> 8), (uint8_t)((R14)&0xFFU), (uint8_t)(M14), (uint8_t)(V14), \
        (uint8_t)((uint16_t)(R15) >> 8), (uint8_t)((R15)&0xFFU), (uint8_t)(M15), (uint8_t)(V15)

/*! Macro for Configuration Setting with seventeen register-mask-value sets:
 *  - Configuration ID[2], Number of Register sets to follow[1], Register[2], Mask[1], Value[1], Register[2], Mask[1], Value[1], Register[2]... */
#define MODE_ENTRY_17_REG(                                                                       \
    MODE,                                                                                        \
    R0,                                                                                          \
    M0,                                                                                          \
    V0,                                                                                          \
    R1,                                                                                          \
    M1,                                                                                          \
    V1,                                                                                          \
    R2,                                                                                          \
    M2,                                                                                          \
    V2,                                                                                          \
    R3,                                                                                          \
    M3,                                                                                          \
    V3,                                                                                          \
    R4,                                                                                          \
    M4,                                                                                          \
    V4,                                                                                          \
    R5,                                                                                          \
    M5,                                                                                          \
    V5,                                                                                          \
    R6,                                                                                          \
    M6,                                                                                          \
    V6,                                                                                          \
    R7,                                                                                          \
    M7,                                                                                          \
    V7,                                                                                          \
    R8,                                                                                          \
    M8,                                                                                          \
    V8,                                                                                          \
    R9,                                                                                          \
    M9,                                                                                          \
    V9,                                                                                          \
    R10,                                                                                         \
    M10,                                                                                         \
    V10,                                                                                         \
    R11,                                                                                         \
    M11,                                                                                         \
    V11,                                                                                         \
    R12,                                                                                         \
    M12,                                                                                         \
    V12,                                                                                         \
    R13,                                                                                         \
    M13,                                                                                         \
    V13,                                                                                         \
    R14,                                                                                         \
    M14,                                                                                         \
    V14,                                                                                         \
    R15,                                                                                         \
    M15,                                                                                         \
    V15,                                                                                         \
    R16,                                                                                         \
    M16,                                                                                         \
    V16)                                                                                         \
    (uint8_t)((uint16_t)(MODE) >> 8), (uint8_t)((MODE)&0xFFU), 17,                               \
        (uint8_t)((uint16_t)(R0) >> 8), (uint8_t)((R0)&0xFFU), (uint8_t)(M0), (uint8_t)(V0),     \
        (uint8_t)((uint16_t)(R1) >> 8), (uint8_t)((R1)&0xFFU), (uint8_t)(M1), (uint8_t)(V1),     \
        (uint8_t)((uint16_t)(R2) >> 8), (uint8_t)((R2)&0xFFU), (uint8_t)(M2), (uint8_t)(V2),     \
        (uint8_t)((uint16_t)(R3) >> 8), (uint8_t)((R3)&0xFFU), (uint8_t)(M3), (uint8_t)(V3),     \
        (uint8_t)((uint16_t)(R4) >> 8), (uint8_t)((R4)&0xFFU), (uint8_t)(M4), (uint8_t)(V4),     \
        (uint8_t)((uint16_t)(R5) >> 8), (uint8_t)((R5)&0xFFU), (uint8_t)(M5), (uint8_t)(V5),     \
        (uint8_t)((uint16_t)(R6) >> 8), (uint8_t)((R6)&0xFFU), (uint8_t)(M6), (uint8_t)(V6),     \
        (uint8_t)((uint16_t)(R7) >> 8), (uint8_t)((R7)&0xFFU), (uint8_t)(M7), (uint8_t)(V7),     \
        (uint8_t)((uint16_t)(R8) >> 8), (uint8_t)((R8)&0xFFU), (uint8_t)(M8), (uint8_t)(V8),     \
        (uint8_t)((uint16_t)(R9) >> 8), (uint8_t)((R9)&0xFFU), (uint8_t)(M9), (uint8_t)(V9),     \
        (uint8_t)((uint16_t)(R10) >> 8), (uint8_t)((R10)&0xFFU), (uint8_t)(M10), (uint8_t)(V10), \
        (uint8_t)((uint16_t)(R11) >> 8), (uint8_t)((R11)&0xFFU), (uint8_t)(M11), (uint8_t)(V11), \
        (uint8_t)((uint16_t)(R12) >> 8), (uint8_t)((R12)&0xFFU), (uint8_t)(M12), (uint8_t)(V12), \
        (uint8_t)((uint16_t)(R13) >> 8), (uint8_t)((R13)&0xFFU), (uint8_t)(M13), (uint8_t)(V13), \
        (uint8_t)((uint16_t)(R14) >> 8), (uint8_t)((R14)&0xFFU), (uint8_t)(M14), (uint8_t)(V14), \
        (uint8_t)((uint16_t)(R15) >> 8), (uint8_t)((R15)&0xFFU), (uint8_t)(M15), (uint8_t)(V15), \
        (uint8_t)((uint16_t)(R16) >> 8), (uint8_t)((R16)&0xFFU), (uint8_t)(M16), (uint8_t)(V16)
/*
 ******************************************************************************
 * GLOBAL DATA TYPES
 ******************************************************************************
 */
/*  PRQA S 3406 1 # MISRA 8.6 - Externally generated table included by the library */ /*  PRQA S 1514 1 # MISRA 8.9 - Externally generated table included by the library */
const uint8_t rfalAnalogConfigDefaultSettings[] = {

    /****** Default Analog Configuration for Chip-Specific Reset ******/
    MODE_ENTRY_17_REG(
        (RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_INIT),
        ST25R3916_REG_IO_CONF1,
        (ST25R3916_REG_IO_CONF1_out_cl_mask | ST25R3916_REG_IO_CONF1_lf_clk_off),
        0x07 /* Disable MCU_CLK */
        ,
        ST25R3916_REG_IO_CONF2,
        (ST25R3916_REG_IO_CONF2_miso_pd1 | ST25R3916_REG_IO_CONF2_miso_pd2),
        0x18 /* SPI Pull downs */
        ,
        ST25R3916_REG_IO_CONF2,
        ST25R3916_REG_IO_CONF2_aat_en,
        ST25R3916_REG_IO_CONF2_aat_en /* Enable AAT */
        ,
        ST25R3916_REG_TX_DRIVER,
        ST25R3916_REG_TX_DRIVER_d_res_mask,
        0x00 /* Set RFO resistance Active Tx */
        ,
        ST25R3916_REG_RES_AM_MOD,
        0xFF,
        0x80 /* Use minimum non-overlap */
        ,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_mask,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_105mV /* Lower activation threshold (higher than deactivation)*/
        ,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_mask,
        ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_105mV /* Lower activation threshold (higher than deactivation)*/
        ,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_mask,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV_trg_75mV /* Lower deactivation threshold */
        ,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_mask,
        ST25R3916_REG_FIELD_THRESHOLD_DEACTV_rfe_75mV /* Lower deactivation threshold */
        ,
        ST25R3916_REG_AUX_MOD,
        ST25R3916_REG_AUX_MOD_lm_ext,
        0x00 /* Disable External Load Modulation */
        ,
        ST25R3916_REG_AUX_MOD,
        ST25R3916_REG_AUX_MOD_lm_dri,
        ST25R3916_REG_AUX_MOD_lm_dri /* Use internal Load Modulation */
        ,
        ST25R3916_REG_PASSIVE_TARGET,
        ST25R3916_REG_PASSIVE_TARGET_fdel_mask,
        (5U
         << ST25R3916_REG_PASSIVE_TARGET_fdel_shift) /* Adjust the FDT to be aligned with the bitgrid  */
        ,
        ST25R3916_REG_PT_MOD,
        (ST25R3916_REG_PT_MOD_ptm_res_mask | ST25R3916_REG_PT_MOD_pt_res_mask),
        0x5f /* Reduce RFO resistance in Modulated state */
        ,
        ST25R3916_REG_EMD_SUP_CONF,
        ST25R3916_REG_EMD_SUP_CONF_rx_start_emv,
        ST25R3916_REG_EMD_SUP_CONF_rx_start_emv_on /* Enable start on first 4 bits */
        ,
        ST25R3916_REG_ANT_TUNE_A,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        ST25R3916_REG_ANT_TUNE_B,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        0x84U,
        0x10,
        0x10 /* Avoid chip internal overheat protection */
        )

    /****** Default Analog Configuration for Chip-Specific Poll Common ******/
    ,
    MODE_ENTRY_9_REG(
        (RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_POLL_COMMON),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        ,
        ST25R3916_REG_TX_DRIVER,
        ST25R3916_REG_TX_DRIVER_am_mod_mask,
        ST25R3916_REG_TX_DRIVER_am_mod_12percent /* Set Modulation index */
        ,
        ST25R3916_REG_AUX_MOD,
        (ST25R3916_REG_AUX_MOD_dis_reg_am | ST25R3916_REG_AUX_MOD_res_am),
        0x00 /* Use AM via regulator */
        ,
        ST25R3916_REG_ANT_TUNE_A,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        ST25R3916_REG_ANT_TUNE_B,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll NFC-A Rx Common ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_correlator /* Use Correlator Receiver */
        )

    /****** Default Analog Configuration for Poll NFC-A Tx 106 ******/
    ,
    MODE_ENTRY_5_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_106 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_ook /* Use OOK */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Overshoot Protection */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Overshoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll NFC-A Rx 106 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_106 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x08,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x2D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x51,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-A Tx 212 ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_212 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        ,
        ST25R3916_REG_AUX_MOD,
        (ST25R3916_REG_AUX_MOD_dis_reg_am | ST25R3916_REG_AUX_MOD_res_am),
        0x88 /* Use Resistive AM */
        ,
        ST25R3916_REG_RES_AM_MOD,
        ST25R3916_REG_RES_AM_MOD_md_res_mask,
        0x7F /* Set Resistive modulation */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll NFC-A Rx 212 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_212 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x02,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x14,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-A Tx 424 ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_424 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        ,
        ST25R3916_REG_AUX_MOD,
        (ST25R3916_REG_AUX_MOD_dis_reg_am | ST25R3916_REG_AUX_MOD_res_am),
        0x88 /* Use Resistive AM */
        ,
        ST25R3916_REG_RES_AM_MOD,
        ST25R3916_REG_RES_AM_MOD_md_res_mask,
        0x7F /* Set Resistive modulation */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll NFC-A Rx 424 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_424 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x42,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x54,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-A Tx 848 ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_848 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        ,
        ST25R3916_REG_TX_DRIVER,
        ST25R3916_REG_TX_DRIVER_am_mod_mask,
        ST25R3916_REG_TX_DRIVER_am_mod_40percent /* Set Modulation index */
        ,
        ST25R3916_REG_AUX_MOD,
        (ST25R3916_REG_AUX_MOD_dis_reg_am | ST25R3916_REG_AUX_MOD_res_am),
        0x00 /* Use AM via regulator */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll NFC-A Rx 848 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_848 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x42,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x44,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-A Anticolision setting ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_ANTICOL),
        ST25R3916_REG_CORR_CONF1,
        ST25R3916_REG_CORR_CONF1_corr_s6,
        0x00 /* Set collision detection level different from data */
        )

#ifdef RFAL_USE_COHE
    /****** Default Analog Configuration for Poll NFC-B Rx Common ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_coherent /* Use Coherent Receiver */
        )
#else
    /****** Default Analog Configuration for Poll NFC-B Rx Common ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_correlator /* Use Correlator Receiver */
        )
#endif /*RFAL_USE_COHE*/

    /****** Default Analog Configuration for Poll NFC-B Rx 106 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | RFAL_ANALOG_CONFIG_BITRATE_106 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x04,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x1B,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-B Rx 212 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | RFAL_ANALOG_CONFIG_BITRATE_212 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x02,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x14,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-B Rx 424 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | RFAL_ANALOG_CONFIG_BITRATE_424 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x42,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x54,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)

    /****** Default Analog Configuration for Poll NFC-B Rx 848 ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | RFAL_ANALOG_CONFIG_BITRATE_848 |
         RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x42,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x44,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)
#ifdef RFAL_USE_COHE

    /****** Default Analog Configuration for Poll NFC-F Rx Common ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCF |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_coherent /* Use Pulse Receiver */
        ,
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x54,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)
#else
    /****** Default Analog Configuration for Poll NFC-F Rx Common ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCF |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_correlator /* Use Correlator Receiver */
        ,
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x3D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x54,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x00)
#endif /*RFAL_USE_COHE*/

        ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV | RFAL_ANALOG_CONFIG_BITRATE_1OF4 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_ook /* Use OOK */
        )

#ifdef RFAL_USE_COHE
    /****** Default Analog Configuration for Poll NFC-V Rx Common ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_coherent /* Use Pulse Receiver */
        ,
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x2D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x01)
#else
    /****** Default Analog Configuration for Poll NFC-V Rx Common ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_AUX,
        ST25R3916_REG_AUX_dis_corr,
        ST25R3916_REG_AUX_dis_corr_correlator /* Use Correlator Receiver */
        ,
        ST25R3916_REG_RX_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_RX_CONF2,
        0xFF,
        0x2D,
        ST25R3916_REG_RX_CONF3,
        0xFF,
        0x00,
        ST25R3916_REG_RX_CONF4,
        0xFF,
        0x00,
        ST25R3916_REG_CORR_CONF1,
        0xFF,
        0x13,
        ST25R3916_REG_CORR_CONF2,
        0xFF,
        0x01)
#endif /*RFAL_USE_COHE*/

    /****** Default Analog Configuration for Poll AP2P Tx 106 ******/
    ,
    MODE_ENTRY_5_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_AP2P | RFAL_ANALOG_CONFIG_BITRATE_106 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_ook /* Use OOK modulation */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Undershoot Protection */
        )

    /****** Default Analog Configuration for Poll AP2P Tx 212 ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_AP2P | RFAL_ANALOG_CONFIG_BITRATE_212 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        )

    /****** Default Analog Configuration for Poll AP2P Tx 424 ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_AP2P | RFAL_ANALOG_CONFIG_BITRATE_424 |
         RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        )

    /****** Default Analog Configuration for Chip-Specific Listen On ******/
    ,
    MODE_ENTRY_6_REG(
        (RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_LISTEN_ON),
        ST25R3916_REG_ANT_TUNE_A,
        0xFF,
        0x00 /* Set Antenna Tuning (Listener): ANTL */
        ,
        ST25R3916_REG_ANT_TUNE_B,
        0xFF,
        0xff /* Set Antenna Tuning (Listener): ANTL */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        )

    /****** Default Analog Configuration for Listen AP2P Tx Common ******/
    ,
    MODE_ENTRY_7_REG(
        (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_ANT_TUNE_A,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        ST25R3916_REG_ANT_TUNE_B,
        0xFF,
        0x82 /* Set Antenna Tuning (Poller): ANTL */
        ,
        ST25R3916_REG_TX_DRIVER,
        ST25R3916_REG_TX_DRIVER_am_mod_mask,
        ST25R3916_REG_TX_DRIVER_am_mod_12percent /* Set Modulation index */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x00 /* Disable Undershoot Protection */
        )

    /****** Default Analog Configuration for Listen AP2P Rx Common ******/
    ,
    MODE_ENTRY_3_REG(
        (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX),
        ST25R3916_REG_RX_CONF1,
        ST25R3916_REG_RX_CONF1_lp_mask,
        ST25R3916_REG_RX_CONF1_lp_1200khz /* Set Rx filter configuration */
        ,
        ST25R3916_REG_RX_CONF1,
        ST25R3916_REG_RX_CONF1_hz_mask,
        ST25R3916_REG_RX_CONF1_hz_12_200khz /* Set Rx filter configuration */
        ,
        ST25R3916_REG_RX_CONF2,
        ST25R3916_REG_RX_CONF2_amd_sel,
        ST25R3916_REG_RX_CONF2_amd_sel_mixer /* AM demodulator: mixer */
        )

    /****** Default Analog Configuration for Listen AP2P Tx 106 ******/
    ,
    MODE_ENTRY_5_REG(
        (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
         RFAL_ANALOG_CONFIG_BITRATE_106 | RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_ook /* Use OOK modulation */
        ,
        ST25R3916_REG_OVERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_OVERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Overshoot Protection  */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF1,
        0xFF,
        0x40 /* Set default Undershoot Protection */
        ,
        ST25R3916_REG_UNDERSHOOT_CONF2,
        0xFF,
        0x03 /* Set default Undershoot Protection */
        )

    /****** Default Analog Configuration for Listen AP2P Tx 212 ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
         RFAL_ANALOG_CONFIG_BITRATE_212 | RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        )

    /****** Default Analog Configuration for Listen AP2P Tx 424 ******/
    ,
    MODE_ENTRY_1_REG(
        (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
         RFAL_ANALOG_CONFIG_BITRATE_424 | RFAL_ANALOG_CONFIG_TX),
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_tr_am_am /* Use AM modulation */
        )

};

#endif /* ST25R3916_ANALOGCONFIG_H */
