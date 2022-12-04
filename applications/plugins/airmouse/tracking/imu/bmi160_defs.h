/**
* Copyright (c) 2021 Bosch Sensortec GmbH. All rights reserved.
*
* BSD-3-Clause
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* @file       bmi160_defs.h
* @date       2021-10-05
* @version    v3.9.2
*
*/

#ifndef BMI160_DEFS_H_
#define BMI160_DEFS_H_

/*************************** C types headers *****************************/
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/kernel.h>
#else
#include <stdint.h>
#include <stddef.h>
#endif

/*************************** Common macros   *****************************/

#if !defined(UINT8_C) && !defined(INT8_C)
#define INT8_C(x) S8_C(x)
#define UINT8_C(x) U8_C(x)
#endif

#if !defined(UINT16_C) && !defined(INT16_C)
#define INT16_C(x) S16_C(x)
#define UINT16_C(x) U16_C(x)
#endif

#if !defined(INT32_C) && !defined(UINT32_C)
#define INT32_C(x) S32_C(x)
#define UINT32_C(x) U32_C(x)
#endif

#if !defined(INT64_C) && !defined(UINT64_C)
#define INT64_C(x) S64_C(x)
#define UINT64_C(x) U64_C(x)
#endif

/**@}*/
/**\name C standard macros */
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

/*************************** Sensor macros   *****************************/
/* Test for an endian machine */
#ifndef __ORDER_LITTLE_ENDIAN__
#define __ORDER_LITTLE_ENDIAN__ 0
#endif

#ifndef __BYTE_ORDER__
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1
#endif
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#ifndef BIG_ENDIAN
#define BIG_ENDIAN 1
#endif
#else
#error "Code does not support Endian format of the processor"
#endif

/** Mask definitions */
#define BMI160_ACCEL_BW_MASK UINT8_C(0x70)
#define BMI160_ACCEL_ODR_MASK UINT8_C(0x0F)
#define BMI160_ACCEL_UNDERSAMPLING_MASK UINT8_C(0x80)
#define BMI160_ACCEL_RANGE_MASK UINT8_C(0x0F)
#define BMI160_GYRO_BW_MASK UINT8_C(0x30)
#define BMI160_GYRO_ODR_MASK UINT8_C(0x0F)
#define BMI160_GYRO_RANGE_MASK UINT8_C(0x07)

#define BMI160_ACCEL_BW_POS UINT8_C(4)
#define BMI160_GYRO_BW_POS UINT8_C(4)

/** Mask definitions for INT_EN registers */
#define BMI160_ANY_MOTION_X_INT_EN_MASK UINT8_C(0x01)
#define BMI160_HIGH_G_X_INT_EN_MASK UINT8_C(0x01)
#define BMI160_NO_MOTION_X_INT_EN_MASK UINT8_C(0x01)
#define BMI160_ANY_MOTION_Y_INT_EN_MASK UINT8_C(0x02)
#define BMI160_HIGH_G_Y_INT_EN_MASK UINT8_C(0x02)
#define BMI160_NO_MOTION_Y_INT_EN_MASK UINT8_C(0x02)
#define BMI160_ANY_MOTION_Z_INT_EN_MASK UINT8_C(0x04)
#define BMI160_HIGH_G_Z_INT_EN_MASK UINT8_C(0x04)
#define BMI160_NO_MOTION_Z_INT_EN_MASK UINT8_C(0x04)
#define BMI160_SIG_MOTION_INT_EN_MASK UINT8_C(0x07)
#define BMI160_ANY_MOTION_ALL_INT_EN_MASK UINT8_C(0x07)
#define BMI160_STEP_DETECT_INT_EN_MASK UINT8_C(0x08)
#define BMI160_DOUBLE_TAP_INT_EN_MASK UINT8_C(0x10)
#define BMI160_SINGLE_TAP_INT_EN_MASK UINT8_C(0x20)
#define BMI160_FIFO_FULL_INT_EN_MASK UINT8_C(0x20)
#define BMI160_ORIENT_INT_EN_MASK UINT8_C(0x40)
#define BMI160_FIFO_WATERMARK_INT_EN_MASK UINT8_C(0x40)
#define BMI160_LOW_G_INT_EN_MASK UINT8_C(0x08)
#define BMI160_STEP_DETECT_EN_MASK UINT8_C(0x08)
#define BMI160_FLAT_INT_EN_MASK UINT8_C(0x80)
#define BMI160_DATA_RDY_INT_EN_MASK UINT8_C(0x10)

/** PMU status Macros */
#define BMI160_AUX_PMU_SUSPEND UINT8_C(0x00)
#define BMI160_AUX_PMU_NORMAL UINT8_C(0x01)
#define BMI160_AUX_PMU_LOW_POWER UINT8_C(0x02)

#define BMI160_GYRO_PMU_SUSPEND UINT8_C(0x00)
#define BMI160_GYRO_PMU_NORMAL UINT8_C(0x01)
#define BMI160_GYRO_PMU_FSU UINT8_C(0x03)

#define BMI160_ACCEL_PMU_SUSPEND UINT8_C(0x00)
#define BMI160_ACCEL_PMU_NORMAL UINT8_C(0x01)
#define BMI160_ACCEL_PMU_LOW_POWER UINT8_C(0x02)

/** Mask definitions for INT_OUT_CTRL register */
#define BMI160_INT1_EDGE_CTRL_MASK UINT8_C(0x01)
#define BMI160_INT1_OUTPUT_MODE_MASK UINT8_C(0x04)
#define BMI160_INT1_OUTPUT_TYPE_MASK UINT8_C(0x02)
#define BMI160_INT1_OUTPUT_EN_MASK UINT8_C(0x08)
#define BMI160_INT2_EDGE_CTRL_MASK UINT8_C(0x10)
#define BMI160_INT2_OUTPUT_MODE_MASK UINT8_C(0x40)
#define BMI160_INT2_OUTPUT_TYPE_MASK UINT8_C(0x20)
#define BMI160_INT2_OUTPUT_EN_MASK UINT8_C(0x80)

/** Mask definitions for INT_LATCH register */
#define BMI160_INT1_INPUT_EN_MASK UINT8_C(0x10)
#define BMI160_INT2_INPUT_EN_MASK UINT8_C(0x20)
#define BMI160_INT_LATCH_MASK UINT8_C(0x0F)

/** Mask definitions for INT_MAP register */
#define BMI160_INT1_LOW_G_MASK UINT8_C(0x01)
#define BMI160_INT1_HIGH_G_MASK UINT8_C(0x02)
#define BMI160_INT1_SLOPE_MASK UINT8_C(0x04)
#define BMI160_INT1_NO_MOTION_MASK UINT8_C(0x08)
#define BMI160_INT1_DOUBLE_TAP_MASK UINT8_C(0x10)
#define BMI160_INT1_SINGLE_TAP_MASK UINT8_C(0x20)
#define BMI160_INT1_FIFO_FULL_MASK UINT8_C(0x20)
#define BMI160_INT1_FIFO_WM_MASK UINT8_C(0x40)
#define BMI160_INT1_ORIENT_MASK UINT8_C(0x40)
#define BMI160_INT1_FLAT_MASK UINT8_C(0x80)
#define BMI160_INT1_DATA_READY_MASK UINT8_C(0x80)
#define BMI160_INT2_LOW_G_MASK UINT8_C(0x01)
#define BMI160_INT1_LOW_STEP_DETECT_MASK UINT8_C(0x01)
#define BMI160_INT2_LOW_STEP_DETECT_MASK UINT8_C(0x01)
#define BMI160_INT2_HIGH_G_MASK UINT8_C(0x02)
#define BMI160_INT2_FIFO_FULL_MASK UINT8_C(0x02)
#define BMI160_INT2_FIFO_WM_MASK UINT8_C(0x04)
#define BMI160_INT2_SLOPE_MASK UINT8_C(0x04)
#define BMI160_INT2_DATA_READY_MASK UINT8_C(0x08)
#define BMI160_INT2_NO_MOTION_MASK UINT8_C(0x08)
#define BMI160_INT2_DOUBLE_TAP_MASK UINT8_C(0x10)
#define BMI160_INT2_SINGLE_TAP_MASK UINT8_C(0x20)
#define BMI160_INT2_ORIENT_MASK UINT8_C(0x40)
#define BMI160_INT2_FLAT_MASK UINT8_C(0x80)

/** Mask definitions for INT_DATA register */
#define BMI160_TAP_SRC_INT_MASK UINT8_C(0x08)
#define BMI160_LOW_HIGH_SRC_INT_MASK UINT8_C(0x80)
#define BMI160_MOTION_SRC_INT_MASK UINT8_C(0x80)

/** Mask definitions for INT_MOTION register */
#define BMI160_SLOPE_INT_DUR_MASK UINT8_C(0x03)
#define BMI160_NO_MOTION_INT_DUR_MASK UINT8_C(0xFC)
#define BMI160_NO_MOTION_SEL_BIT_MASK UINT8_C(0x01)

/** Mask definitions for INT_TAP register */
#define BMI160_TAP_DUR_MASK UINT8_C(0x07)
#define BMI160_TAP_SHOCK_DUR_MASK UINT8_C(0x40)
#define BMI160_TAP_QUIET_DUR_MASK UINT8_C(0x80)
#define BMI160_TAP_THRES_MASK UINT8_C(0x1F)

/** Mask definitions for INT_FLAT register */
#define BMI160_FLAT_THRES_MASK UINT8_C(0x3F)
#define BMI160_FLAT_HOLD_TIME_MASK UINT8_C(0x30)
#define BMI160_FLAT_HYST_MASK UINT8_C(0x07)

/** Mask definitions for INT_LOWHIGH register */
#define BMI160_LOW_G_HYST_MASK UINT8_C(0x03)
#define BMI160_LOW_G_LOW_MODE_MASK UINT8_C(0x04)
#define BMI160_HIGH_G_HYST_MASK UINT8_C(0xC0)

/** Mask definitions for INT_SIG_MOTION register */
#define BMI160_SIG_MOTION_SEL_MASK UINT8_C(0x02)
#define BMI160_SIG_MOTION_SKIP_MASK UINT8_C(0x0C)
#define BMI160_SIG_MOTION_PROOF_MASK UINT8_C(0x30)

/** Mask definitions for INT_ORIENT register */
#define BMI160_ORIENT_MODE_MASK UINT8_C(0x03)
#define BMI160_ORIENT_BLOCK_MASK UINT8_C(0x0C)
#define BMI160_ORIENT_HYST_MASK UINT8_C(0xF0)
#define BMI160_ORIENT_THETA_MASK UINT8_C(0x3F)
#define BMI160_ORIENT_UD_ENABLE UINT8_C(0x40)
#define BMI160_AXES_EN_MASK UINT8_C(0x80)

/** Mask definitions for FIFO_CONFIG register */
#define BMI160_FIFO_GYRO UINT8_C(0x80)
#define BMI160_FIFO_ACCEL UINT8_C(0x40)
#define BMI160_FIFO_AUX UINT8_C(0x20)
#define BMI160_FIFO_TAG_INT1 UINT8_C(0x08)
#define BMI160_FIFO_TAG_INT2 UINT8_C(0x04)
#define BMI160_FIFO_TIME UINT8_C(0x02)
#define BMI160_FIFO_HEADER UINT8_C(0x10)
#define BMI160_FIFO_CONFIG_1_MASK UINT8_C(0xFE)

/** Mask definitions for STEP_CONF register */
#define BMI160_STEP_COUNT_EN_BIT_MASK UINT8_C(0x08)
#define BMI160_STEP_DETECT_MIN_THRES_MASK UINT8_C(0x18)
#define BMI160_STEP_DETECT_STEPTIME_MIN_MASK UINT8_C(0x07)
#define BMI160_STEP_MIN_BUF_MASK UINT8_C(0x07)

/** Mask definition for FIFO Header Data Tag */
#define BMI160_FIFO_TAG_INTR_MASK UINT8_C(0xFC)

/** Fifo byte counter mask definitions */
#define BMI160_FIFO_BYTE_COUNTER_MASK UINT8_C(0x07)

/** Enable/disable bit value */
#define BMI160_ENABLE UINT8_C(0x01)
#define BMI160_DISABLE UINT8_C(0x00)

/** Latch Duration */
#define BMI160_LATCH_DUR_NONE UINT8_C(0x00)
#define BMI160_LATCH_DUR_312_5_MICRO_SEC UINT8_C(0x01)
#define BMI160_LATCH_DUR_625_MICRO_SEC UINT8_C(0x02)
#define BMI160_LATCH_DUR_1_25_MILLI_SEC UINT8_C(0x03)
#define BMI160_LATCH_DUR_2_5_MILLI_SEC UINT8_C(0x04)
#define BMI160_LATCH_DUR_5_MILLI_SEC UINT8_C(0x05)
#define BMI160_LATCH_DUR_10_MILLI_SEC UINT8_C(0x06)
#define BMI160_LATCH_DUR_20_MILLI_SEC UINT8_C(0x07)
#define BMI160_LATCH_DUR_40_MILLI_SEC UINT8_C(0x08)
#define BMI160_LATCH_DUR_80_MILLI_SEC UINT8_C(0x09)
#define BMI160_LATCH_DUR_160_MILLI_SEC UINT8_C(0x0A)
#define BMI160_LATCH_DUR_320_MILLI_SEC UINT8_C(0x0B)
#define BMI160_LATCH_DUR_640_MILLI_SEC UINT8_C(0x0C)
#define BMI160_LATCH_DUR_1_28_SEC UINT8_C(0x0D)
#define BMI160_LATCH_DUR_2_56_SEC UINT8_C(0x0E)
#define BMI160_LATCHED UINT8_C(0x0F)

/** BMI160 Register map */
#define BMI160_CHIP_ID_ADDR UINT8_C(0x00)
#define BMI160_ERROR_REG_ADDR UINT8_C(0x02)
#define BMI160_PMU_STATUS_ADDR UINT8_C(0x03)
#define BMI160_AUX_DATA_ADDR UINT8_C(0x04)
#define BMI160_GYRO_DATA_ADDR UINT8_C(0x0C)
#define BMI160_ACCEL_DATA_ADDR UINT8_C(0x12)
#define BMI160_STATUS_ADDR UINT8_C(0x1B)
#define BMI160_INT_STATUS_ADDR UINT8_C(0x1C)
#define BMI160_FIFO_LENGTH_ADDR UINT8_C(0x22)
#define BMI160_FIFO_DATA_ADDR UINT8_C(0x24)
#define BMI160_ACCEL_CONFIG_ADDR UINT8_C(0x40)
#define BMI160_ACCEL_RANGE_ADDR UINT8_C(0x41)
#define BMI160_GYRO_CONFIG_ADDR UINT8_C(0x42)
#define BMI160_GYRO_RANGE_ADDR UINT8_C(0x43)
#define BMI160_AUX_ODR_ADDR UINT8_C(0x44)
#define BMI160_FIFO_DOWN_ADDR UINT8_C(0x45)
#define BMI160_FIFO_CONFIG_0_ADDR UINT8_C(0x46)
#define BMI160_FIFO_CONFIG_1_ADDR UINT8_C(0x47)
#define BMI160_AUX_IF_0_ADDR UINT8_C(0x4B)
#define BMI160_AUX_IF_1_ADDR UINT8_C(0x4C)
#define BMI160_AUX_IF_2_ADDR UINT8_C(0x4D)
#define BMI160_AUX_IF_3_ADDR UINT8_C(0x4E)
#define BMI160_AUX_IF_4_ADDR UINT8_C(0x4F)
#define BMI160_INT_ENABLE_0_ADDR UINT8_C(0x50)
#define BMI160_INT_ENABLE_1_ADDR UINT8_C(0x51)
#define BMI160_INT_ENABLE_2_ADDR UINT8_C(0x52)
#define BMI160_INT_OUT_CTRL_ADDR UINT8_C(0x53)
#define BMI160_INT_LATCH_ADDR UINT8_C(0x54)
#define BMI160_INT_MAP_0_ADDR UINT8_C(0x55)
#define BMI160_INT_MAP_1_ADDR UINT8_C(0x56)
#define BMI160_INT_MAP_2_ADDR UINT8_C(0x57)
#define BMI160_INT_DATA_0_ADDR UINT8_C(0x58)
#define BMI160_INT_DATA_1_ADDR UINT8_C(0x59)
#define BMI160_INT_LOWHIGH_0_ADDR UINT8_C(0x5A)
#define BMI160_INT_LOWHIGH_1_ADDR UINT8_C(0x5B)
#define BMI160_INT_LOWHIGH_2_ADDR UINT8_C(0x5C)
#define BMI160_INT_LOWHIGH_3_ADDR UINT8_C(0x5D)
#define BMI160_INT_LOWHIGH_4_ADDR UINT8_C(0x5E)
#define BMI160_INT_MOTION_0_ADDR UINT8_C(0x5F)
#define BMI160_INT_MOTION_1_ADDR UINT8_C(0x60)
#define BMI160_INT_MOTION_2_ADDR UINT8_C(0x61)
#define BMI160_INT_MOTION_3_ADDR UINT8_C(0x62)
#define BMI160_INT_TAP_0_ADDR UINT8_C(0x63)
#define BMI160_INT_TAP_1_ADDR UINT8_C(0x64)
#define BMI160_INT_ORIENT_0_ADDR UINT8_C(0x65)
#define BMI160_INT_ORIENT_1_ADDR UINT8_C(0x66)
#define BMI160_INT_FLAT_0_ADDR UINT8_C(0x67)
#define BMI160_INT_FLAT_1_ADDR UINT8_C(0x68)
#define BMI160_FOC_CONF_ADDR UINT8_C(0x69)
#define BMI160_CONF_ADDR UINT8_C(0x6A)

#define BMI160_IF_CONF_ADDR UINT8_C(0x6B)
#define BMI160_SELF_TEST_ADDR UINT8_C(0x6D)
#define BMI160_OFFSET_ADDR UINT8_C(0x71)
#define BMI160_OFFSET_CONF_ADDR UINT8_C(0x77)
#define BMI160_INT_STEP_CNT_0_ADDR UINT8_C(0x78)
#define BMI160_INT_STEP_CONFIG_0_ADDR UINT8_C(0x7A)
#define BMI160_INT_STEP_CONFIG_1_ADDR UINT8_C(0x7B)
#define BMI160_COMMAND_REG_ADDR UINT8_C(0x7E)
#define BMI160_SPI_COMM_TEST_ADDR UINT8_C(0x7F)
#define BMI160_INTL_PULLUP_CONF_ADDR UINT8_C(0x85)

/** Error code definitions */
#define BMI160_OK INT8_C(0)
#define BMI160_E_NULL_PTR INT8_C(-1)
#define BMI160_E_COM_FAIL INT8_C(-2)
#define BMI160_E_DEV_NOT_FOUND INT8_C(-3)
#define BMI160_E_OUT_OF_RANGE INT8_C(-4)
#define BMI160_E_INVALID_INPUT INT8_C(-5)
#define BMI160_E_ACCEL_ODR_BW_INVALID INT8_C(-6)
#define BMI160_E_GYRO_ODR_BW_INVALID INT8_C(-7)
#define BMI160_E_LWP_PRE_FLTR_INT_INVALID INT8_C(-8)
#define BMI160_E_LWP_PRE_FLTR_INVALID INT8_C(-9)
#define BMI160_E_AUX_NOT_FOUND INT8_C(-10)
#define BMI160_E_FOC_FAILURE INT8_C(-11)
#define BMI160_E_READ_WRITE_LENGTH_INVALID INT8_C(-12)
#define BMI160_E_INVALID_CONFIG INT8_C(-13)

/**\name API warning codes */
#define BMI160_W_GYRO_SELF_TEST_FAIL INT8_C(1)
#define BMI160_W_ACCEl_SELF_TEST_FAIL INT8_C(2)

/** BMI160 unique chip identifier */
#define BMI160_CHIP_ID UINT8_C(0xD1)

/** Soft reset command */
#define BMI160_SOFT_RESET_CMD UINT8_C(0xb6)
#define BMI160_SOFT_RESET_DELAY_MS UINT8_C(1)

/** Start FOC command */
#define BMI160_START_FOC_CMD UINT8_C(0x03)

/** NVM backup enabling command */
#define BMI160_NVM_BACKUP_EN UINT8_C(0xA0)

/* Delay in ms settings */
#define BMI160_ACCEL_DELAY_MS UINT8_C(5)
#define BMI160_GYRO_DELAY_MS UINT8_C(80)
#define BMI160_ONE_MS_DELAY UINT8_C(1)
#define BMI160_AUX_COM_DELAY UINT8_C(10)
#define BMI160_GYRO_SELF_TEST_DELAY UINT8_C(20)
#define BMI160_ACCEL_SELF_TEST_DELAY UINT8_C(50)

/** Self test configurations */
#define BMI160_ACCEL_SELF_TEST_CONFIG UINT8_C(0x2C)
#define BMI160_ACCEL_SELF_TEST_POSITIVE_EN UINT8_C(0x0D)
#define BMI160_ACCEL_SELF_TEST_NEGATIVE_EN UINT8_C(0x09)
#define BMI160_ACCEL_SELF_TEST_LIMIT UINT16_C(8192)

/** Power mode settings */
/* Accel power mode */
#define BMI160_ACCEL_NORMAL_MODE UINT8_C(0x11)
#define BMI160_ACCEL_LOWPOWER_MODE UINT8_C(0x12)
#define BMI160_ACCEL_SUSPEND_MODE UINT8_C(0x10)

/* Gyro power mode */
#define BMI160_GYRO_SUSPEND_MODE UINT8_C(0x14)
#define BMI160_GYRO_NORMAL_MODE UINT8_C(0x15)
#define BMI160_GYRO_FASTSTARTUP_MODE UINT8_C(0x17)

/* Aux power mode */
#define BMI160_AUX_SUSPEND_MODE UINT8_C(0x18)
#define BMI160_AUX_NORMAL_MODE UINT8_C(0x19)
#define BMI160_AUX_LOWPOWER_MODE UINT8_C(0x1A)

/** Range settings */
/* Accel Range */
#define BMI160_ACCEL_RANGE_2G UINT8_C(0x03)
#define BMI160_ACCEL_RANGE_4G UINT8_C(0x05)
#define BMI160_ACCEL_RANGE_8G UINT8_C(0x08)
#define BMI160_ACCEL_RANGE_16G UINT8_C(0x0C)

/* Gyro Range */
#define BMI160_GYRO_RANGE_2000_DPS UINT8_C(0x00)
#define BMI160_GYRO_RANGE_1000_DPS UINT8_C(0x01)
#define BMI160_GYRO_RANGE_500_DPS UINT8_C(0x02)
#define BMI160_GYRO_RANGE_250_DPS UINT8_C(0x03)
#define BMI160_GYRO_RANGE_125_DPS UINT8_C(0x04)

/** Bandwidth settings */
/* Accel Bandwidth */
#define BMI160_ACCEL_BW_OSR4_AVG1 UINT8_C(0x00)
#define BMI160_ACCEL_BW_OSR2_AVG2 UINT8_C(0x01)
#define BMI160_ACCEL_BW_NORMAL_AVG4 UINT8_C(0x02)
#define BMI160_ACCEL_BW_RES_AVG8 UINT8_C(0x03)
#define BMI160_ACCEL_BW_RES_AVG16 UINT8_C(0x04)
#define BMI160_ACCEL_BW_RES_AVG32 UINT8_C(0x05)
#define BMI160_ACCEL_BW_RES_AVG64 UINT8_C(0x06)
#define BMI160_ACCEL_BW_RES_AVG128 UINT8_C(0x07)

#define BMI160_GYRO_BW_OSR4_MODE UINT8_C(0x00)
#define BMI160_GYRO_BW_OSR2_MODE UINT8_C(0x01)
#define BMI160_GYRO_BW_NORMAL_MODE UINT8_C(0x02)

/* Output Data Rate settings */
/* Accel Output data rate */
#define BMI160_ACCEL_ODR_RESERVED UINT8_C(0x00)
#define BMI160_ACCEL_ODR_0_78HZ UINT8_C(0x01)
#define BMI160_ACCEL_ODR_1_56HZ UINT8_C(0x02)
#define BMI160_ACCEL_ODR_3_12HZ UINT8_C(0x03)
#define BMI160_ACCEL_ODR_6_25HZ UINT8_C(0x04)
#define BMI160_ACCEL_ODR_12_5HZ UINT8_C(0x05)
#define BMI160_ACCEL_ODR_25HZ UINT8_C(0x06)
#define BMI160_ACCEL_ODR_50HZ UINT8_C(0x07)
#define BMI160_ACCEL_ODR_100HZ UINT8_C(0x08)
#define BMI160_ACCEL_ODR_200HZ UINT8_C(0x09)
#define BMI160_ACCEL_ODR_400HZ UINT8_C(0x0A)
#define BMI160_ACCEL_ODR_800HZ UINT8_C(0x0B)
#define BMI160_ACCEL_ODR_1600HZ UINT8_C(0x0C)
#define BMI160_ACCEL_ODR_RESERVED0 UINT8_C(0x0D)
#define BMI160_ACCEL_ODR_RESERVED1 UINT8_C(0x0E)
#define BMI160_ACCEL_ODR_RESERVED2 UINT8_C(0x0F)

/* Gyro Output data rate */
#define BMI160_GYRO_ODR_RESERVED UINT8_C(0x00)
#define BMI160_GYRO_ODR_25HZ UINT8_C(0x06)
#define BMI160_GYRO_ODR_50HZ UINT8_C(0x07)
#define BMI160_GYRO_ODR_100HZ UINT8_C(0x08)
#define BMI160_GYRO_ODR_200HZ UINT8_C(0x09)
#define BMI160_GYRO_ODR_400HZ UINT8_C(0x0A)
#define BMI160_GYRO_ODR_800HZ UINT8_C(0x0B)
#define BMI160_GYRO_ODR_1600HZ UINT8_C(0x0C)
#define BMI160_GYRO_ODR_3200HZ UINT8_C(0x0D)

/* Auxiliary sensor Output data rate */
#define BMI160_AUX_ODR_RESERVED UINT8_C(0x00)
#define BMI160_AUX_ODR_0_78HZ UINT8_C(0x01)
#define BMI160_AUX_ODR_1_56HZ UINT8_C(0x02)
#define BMI160_AUX_ODR_3_12HZ UINT8_C(0x03)
#define BMI160_AUX_ODR_6_25HZ UINT8_C(0x04)
#define BMI160_AUX_ODR_12_5HZ UINT8_C(0x05)
#define BMI160_AUX_ODR_25HZ UINT8_C(0x06)
#define BMI160_AUX_ODR_50HZ UINT8_C(0x07)
#define BMI160_AUX_ODR_100HZ UINT8_C(0x08)
#define BMI160_AUX_ODR_200HZ UINT8_C(0x09)
#define BMI160_AUX_ODR_400HZ UINT8_C(0x0A)
#define BMI160_AUX_ODR_800HZ UINT8_C(0x0B)

/** FIFO_CONFIG Definitions */
#define BMI160_FIFO_TIME_ENABLE UINT8_C(0x02)
#define BMI160_FIFO_TAG_INT2_ENABLE UINT8_C(0x04)
#define BMI160_FIFO_TAG_INT1_ENABLE UINT8_C(0x08)
#define BMI160_FIFO_HEAD_ENABLE UINT8_C(0x10)
#define BMI160_FIFO_M_ENABLE UINT8_C(0x20)
#define BMI160_FIFO_A_ENABLE UINT8_C(0x40)
#define BMI160_FIFO_M_A_ENABLE UINT8_C(0x60)
#define BMI160_FIFO_G_ENABLE UINT8_C(0x80)
#define BMI160_FIFO_M_G_ENABLE UINT8_C(0xA0)
#define BMI160_FIFO_G_A_ENABLE UINT8_C(0xC0)
#define BMI160_FIFO_M_G_A_ENABLE UINT8_C(0xE0)

/* Macro to specify the number of bytes over-read from the
 * FIFO in order to get the sensor time at the end of FIFO */
#ifndef BMI160_FIFO_BYTES_OVERREAD
#define BMI160_FIFO_BYTES_OVERREAD UINT8_C(25)
#endif

/* Accel, gyro and aux. sensor length and also their combined
 * length definitions in FIFO */
#define BMI160_FIFO_G_LENGTH UINT8_C(6)
#define BMI160_FIFO_A_LENGTH UINT8_C(6)
#define BMI160_FIFO_M_LENGTH UINT8_C(8)
#define BMI160_FIFO_GA_LENGTH UINT8_C(12)
#define BMI160_FIFO_MA_LENGTH UINT8_C(14)
#define BMI160_FIFO_MG_LENGTH UINT8_C(14)
#define BMI160_FIFO_MGA_LENGTH UINT8_C(20)

/** FIFO Header Data definitions */
#define BMI160_FIFO_HEAD_SKIP_FRAME UINT8_C(0x40)
#define BMI160_FIFO_HEAD_SENSOR_TIME UINT8_C(0x44)
#define BMI160_FIFO_HEAD_INPUT_CONFIG UINT8_C(0x48)
#define BMI160_FIFO_HEAD_OVER_READ UINT8_C(0x80)
#define BMI160_FIFO_HEAD_A UINT8_C(0x84)
#define BMI160_FIFO_HEAD_G UINT8_C(0x88)
#define BMI160_FIFO_HEAD_G_A UINT8_C(0x8C)
#define BMI160_FIFO_HEAD_M UINT8_C(0x90)
#define BMI160_FIFO_HEAD_M_A UINT8_C(0x94)
#define BMI160_FIFO_HEAD_M_G UINT8_C(0x98)
#define BMI160_FIFO_HEAD_M_G_A UINT8_C(0x9C)

/** FIFO sensor time length definitions */
#define BMI160_SENSOR_TIME_LENGTH UINT8_C(3)

/** FIFO DOWN selection */
/* Accel fifo down-sampling values*/
#define BMI160_ACCEL_FIFO_DOWN_ZERO UINT8_C(0x00)
#define BMI160_ACCEL_FIFO_DOWN_ONE UINT8_C(0x10)
#define BMI160_ACCEL_FIFO_DOWN_TWO UINT8_C(0x20)
#define BMI160_ACCEL_FIFO_DOWN_THREE UINT8_C(0x30)
#define BMI160_ACCEL_FIFO_DOWN_FOUR UINT8_C(0x40)
#define BMI160_ACCEL_FIFO_DOWN_FIVE UINT8_C(0x50)
#define BMI160_ACCEL_FIFO_DOWN_SIX UINT8_C(0x60)
#define BMI160_ACCEL_FIFO_DOWN_SEVEN UINT8_C(0x70)

/* Gyro fifo down-smapling values*/
#define BMI160_GYRO_FIFO_DOWN_ZERO UINT8_C(0x00)
#define BMI160_GYRO_FIFO_DOWN_ONE UINT8_C(0x01)
#define BMI160_GYRO_FIFO_DOWN_TWO UINT8_C(0x02)
#define BMI160_GYRO_FIFO_DOWN_THREE UINT8_C(0x03)
#define BMI160_GYRO_FIFO_DOWN_FOUR UINT8_C(0x04)
#define BMI160_GYRO_FIFO_DOWN_FIVE UINT8_C(0x05)
#define BMI160_GYRO_FIFO_DOWN_SIX UINT8_C(0x06)
#define BMI160_GYRO_FIFO_DOWN_SEVEN UINT8_C(0x07)

/* Accel Fifo filter enable*/
#define BMI160_ACCEL_FIFO_FILT_EN UINT8_C(0x80)

/* Gyro Fifo filter enable*/
#define BMI160_GYRO_FIFO_FILT_EN UINT8_C(0x08)

/** Definitions to check validity of FIFO frames */
#define FIFO_CONFIG_MSB_CHECK UINT8_C(0x80)
#define FIFO_CONFIG_LSB_CHECK UINT8_C(0x00)

/*! BMI160 accel FOC configurations */
#define BMI160_FOC_ACCEL_DISABLED UINT8_C(0x00)
#define BMI160_FOC_ACCEL_POSITIVE_G UINT8_C(0x01)
#define BMI160_FOC_ACCEL_NEGATIVE_G UINT8_C(0x02)
#define BMI160_FOC_ACCEL_0G UINT8_C(0x03)

/** Array Parameter DefinItions */
#define BMI160_SENSOR_TIME_LSB_BYTE UINT8_C(0)
#define BMI160_SENSOR_TIME_XLSB_BYTE UINT8_C(1)
#define BMI160_SENSOR_TIME_MSB_BYTE UINT8_C(2)

/** Interface settings */
#define BMI160_SPI_INTF UINT8_C(1)
#define BMI160_I2C_INTF UINT8_C(0)
#define BMI160_SPI_RD_MASK UINT8_C(0x80)
#define BMI160_SPI_WR_MASK UINT8_C(0x7F)

/* Sensor & time select definition*/
#define BMI160_ACCEL_SEL UINT8_C(0x01)
#define BMI160_GYRO_SEL UINT8_C(0x02)
#define BMI160_TIME_SEL UINT8_C(0x04)

/* Sensor select mask*/
#define BMI160_SEN_SEL_MASK UINT8_C(0x07)

/* Error code mask */
#define BMI160_ERR_REG_MASK UINT8_C(0x0F)

/* BMI160 I2C address */
#define BMI160_I2C_ADDR UINT8_C(0x68)

/* BMI160 secondary IF address */
#define BMI160_AUX_BMM150_I2C_ADDR UINT8_C(0x10)

/** BMI160 Length definitions */
#define BMI160_ONE UINT8_C(1)
#define BMI160_TWO UINT8_C(2)
#define BMI160_THREE UINT8_C(3)
#define BMI160_FOUR UINT8_C(4)
#define BMI160_FIVE UINT8_C(5)

/** BMI160 fifo level Margin */
#define BMI160_FIFO_LEVEL_MARGIN UINT8_C(16)

/** BMI160 fifo flush Command */
#define BMI160_FIFO_FLUSH_VALUE UINT8_C(0xB0)

/** BMI160 offset values for xyz axes of accel */
#define BMI160_ACCEL_MIN_OFFSET INT8_C(-128)
#define BMI160_ACCEL_MAX_OFFSET INT8_C(127)

/** BMI160 offset values for xyz axes of gyro */
#define BMI160_GYRO_MIN_OFFSET INT16_C(-512)
#define BMI160_GYRO_MAX_OFFSET INT16_C(511)

/** BMI160 fifo full interrupt position and mask */
#define BMI160_FIFO_FULL_INT_POS UINT8_C(5)
#define BMI160_FIFO_FULL_INT_MSK UINT8_C(0x20)
#define BMI160_FIFO_WTM_INT_POS UINT8_C(6)
#define BMI160_FIFO_WTM_INT_MSK UINT8_C(0x40)

#define BMI160_FIFO_FULL_INT_PIN1_POS UINT8_C(5)
#define BMI160_FIFO_FULL_INT_PIN1_MSK UINT8_C(0x20)
#define BMI160_FIFO_FULL_INT_PIN2_POS UINT8_C(1)
#define BMI160_FIFO_FULL_INT_PIN2_MSK UINT8_C(0x02)

#define BMI160_FIFO_WTM_INT_PIN1_POS UINT8_C(6)
#define BMI160_FIFO_WTM_INT_PIN1_MSK UINT8_C(0x40)
#define BMI160_FIFO_WTM_INT_PIN2_POS UINT8_C(2)
#define BMI160_FIFO_WTM_INT_PIN2_MSK UINT8_C(0x04)

#define BMI160_MANUAL_MODE_EN_POS UINT8_C(7)
#define BMI160_MANUAL_MODE_EN_MSK UINT8_C(0x80)
#define BMI160_AUX_READ_BURST_POS UINT8_C(0)
#define BMI160_AUX_READ_BURST_MSK UINT8_C(0x03)

#define BMI160_GYRO_SELF_TEST_POS UINT8_C(4)
#define BMI160_GYRO_SELF_TEST_MSK UINT8_C(0x10)
#define BMI160_GYRO_SELF_TEST_STATUS_POS UINT8_C(1)
#define BMI160_GYRO_SELF_TEST_STATUS_MSK UINT8_C(0x02)

#define BMI160_GYRO_FOC_EN_POS UINT8_C(6)
#define BMI160_GYRO_FOC_EN_MSK UINT8_C(0x40)

#define BMI160_ACCEL_FOC_X_CONF_POS UINT8_C(4)
#define BMI160_ACCEL_FOC_X_CONF_MSK UINT8_C(0x30)

#define BMI160_ACCEL_FOC_Y_CONF_POS UINT8_C(2)
#define BMI160_ACCEL_FOC_Y_CONF_MSK UINT8_C(0x0C)

#define BMI160_ACCEL_FOC_Z_CONF_MSK UINT8_C(0x03)

#define BMI160_FOC_STATUS_POS UINT8_C(3)
#define BMI160_FOC_STATUS_MSK UINT8_C(0x08)

#define BMI160_GYRO_OFFSET_X_MSK UINT8_C(0x03)

#define BMI160_GYRO_OFFSET_Y_POS UINT8_C(2)
#define BMI160_GYRO_OFFSET_Y_MSK UINT8_C(0x0C)

#define BMI160_GYRO_OFFSET_Z_POS UINT8_C(4)
#define BMI160_GYRO_OFFSET_Z_MSK UINT8_C(0x30)

#define BMI160_GYRO_OFFSET_EN_POS UINT8_C(7)
#define BMI160_GYRO_OFFSET_EN_MSK UINT8_C(0x80)

#define BMI160_ACCEL_OFFSET_EN_POS UINT8_C(6)
#define BMI160_ACCEL_OFFSET_EN_MSK UINT8_C(0x40)

#define BMI160_GYRO_OFFSET_POS UINT16_C(8)
#define BMI160_GYRO_OFFSET_MSK UINT16_C(0x0300)

#define BMI160_NVM_UPDATE_POS UINT8_C(1)
#define BMI160_NVM_UPDATE_MSK UINT8_C(0x02)

#define BMI160_NVM_STATUS_POS UINT8_C(4)
#define BMI160_NVM_STATUS_MSK UINT8_C(0x10)

#define BMI160_MAG_POWER_MODE_MSK UINT8_C(0x03)

#define BMI160_ACCEL_POWER_MODE_MSK UINT8_C(0x30)
#define BMI160_ACCEL_POWER_MODE_POS UINT8_C(4)

#define BMI160_GYRO_POWER_MODE_MSK UINT8_C(0x0C)
#define BMI160_GYRO_POWER_MODE_POS UINT8_C(2)

/* BIT SLICE GET AND SET FUNCTIONS */
#define BMI160_GET_BITS(regvar, bitname) ((regvar & bitname##_MSK) >> bitname##_POS)
#define BMI160_SET_BITS(regvar, bitname, val) \
    ((regvar & ~bitname##_MSK) | ((val << bitname##_POS) & bitname##_MSK))

#define BMI160_SET_BITS_POS_0(reg_data, bitname, data) \
    ((reg_data & ~(bitname##_MSK)) | (data & bitname##_MSK))

#define BMI160_GET_BITS_POS_0(reg_data, bitname) (reg_data & (bitname##_MSK))

/**\name UTILITY MACROS */
#define BMI160_SET_LOW_BYTE UINT16_C(0x00FF)
#define BMI160_SET_HIGH_BYTE UINT16_C(0xFF00)

#define BMI160_GET_LSB(var) (uint8_t)(var & BMI160_SET_LOW_BYTE)
#define BMI160_GET_MSB(var) (uint8_t)((var & BMI160_SET_HIGH_BYTE) >> 8)

/*****************************************************************************/
/* type definitions */

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific read functions of the user
 */
typedef int8_t (
    *bmi160_read_fptr_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint16_t len);

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific write functions of the user
 */
typedef int8_t (
    *bmi160_write_fptr_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t* read_data, uint16_t len);
typedef void (*bmi160_delay_fptr_t)(uint32_t period);

/*************************** Data structures *********************************/

/*!
 * @brief bmi160 interrupt status selection enum.
 */
enum bmi160_int_status_sel {
    BMI160_INT_STATUS_0 = 1,
    BMI160_INT_STATUS_1 = 2,
    BMI160_INT_STATUS_2 = 4,
    BMI160_INT_STATUS_3 = 8,
    BMI160_INT_STATUS_ALL = 15
};

/*!
 * @brief bmi160 interrupt status bits structure
 */
struct bmi160_int_status_bits {
#ifdef LITTLE_ENDIAN

    uint32_t step : 1;
    uint32_t sigmot : 1;
    uint32_t anym : 1;

    /* pmu trigger will be handled later */
    uint32_t pmu_trigger_reserved : 1;
    uint32_t d_tap : 1;
    uint32_t s_tap : 1;
    uint32_t orient : 1;
    uint32_t flat_int : 1;
    uint32_t reserved : 2;
    uint32_t high_g : 1;
    uint32_t low_g : 1;
    uint32_t drdy : 1;
    uint32_t ffull : 1;
    uint32_t fwm : 1;
    uint32_t nomo : 1;
    uint32_t anym_first_x : 1;
    uint32_t anym_first_y : 1;
    uint32_t anym_first_z : 1;
    uint32_t anym_sign : 1;
    uint32_t tap_first_x : 1;
    uint32_t tap_first_y : 1;
    uint32_t tap_first_z : 1;
    uint32_t tap_sign : 1;
    uint32_t high_first_x : 1;
    uint32_t high_first_y : 1;
    uint32_t high_first_z : 1;
    uint32_t high_sign : 1;
    uint32_t orient_1_0 : 2;
    uint32_t orient_2 : 1;
    uint32_t flat : 1;
#else
    uint32_t high_first_x : 1;
    uint32_t high_first_y : 1;
    uint32_t high_first_z : 1;
    uint32_t high_sign : 1;
    uint32_t orient_1_0 : 2;
    uint32_t orient_2 : 1;
    uint32_t flat : 1;
    uint32_t anym_first_x : 1;
    uint32_t anym_first_y : 1;
    uint32_t anym_first_z : 1;
    uint32_t anym_sign : 1;
    uint32_t tap_first_x : 1;
    uint32_t tap_first_y : 1;
    uint32_t tap_first_z : 1;
    uint32_t tap_sign : 1;
    uint32_t reserved : 2;
    uint32_t high_g : 1;
    uint32_t low_g : 1;
    uint32_t drdy : 1;
    uint32_t ffull : 1;
    uint32_t fwm : 1;
    uint32_t nomo : 1;
    uint32_t step : 1;
    uint32_t sigmot : 1;
    uint32_t anym : 1;

    /* pmu trigger will be handled later */
    uint32_t pmu_trigger_reserved : 1;
    uint32_t d_tap : 1;
    uint32_t s_tap : 1;
    uint32_t orient : 1;
    uint32_t flat_int : 1;
#endif
};

/*!
 * @brief bmi160 interrupt status structure
 */
union bmi160_int_status {
    uint8_t data[4];
    struct bmi160_int_status_bits bit;
};

/*!
 * @brief bmi160 sensor data structure which comprises of accel data
 */
struct bmi160_sensor_data {
    /*! X-axis sensor data */
    int16_t x;

    /*! Y-axis sensor data */
    int16_t y;

    /*! Z-axis sensor data */
    int16_t z;

    /*! sensor time */
    uint32_t sensortime;
};

/*!
 * @brief bmi160 aux data structure which comprises of 8 bytes of accel data
 */
struct bmi160_aux_data {
    /*! Auxiliary data */
    uint8_t data[8];
};

/*!
 * @brief bmi160 FOC configuration structure
 */
struct bmi160_foc_conf {
    /*! Enabling FOC in gyro
     * Assignable macros :
     *  - BMI160_ENABLE
     *  - BMI160_DISABLE
     */
    uint8_t foc_gyr_en;

    /*! Accel FOC configurations
     * Assignable macros :
     *  - BMI160_FOC_ACCEL_DISABLED
     *  - BMI160_FOC_ACCEL_POSITIVE_G
     *  - BMI160_FOC_ACCEL_NEGATIVE_G
     *  - BMI160_FOC_ACCEL_0G
     */
    uint8_t foc_acc_x;
    uint8_t foc_acc_y;
    uint8_t foc_acc_z;

    /*! Enabling offset compensation for accel in data registers
     * Assignable macros :
     *  - BMI160_ENABLE
     *  - BMI160_DISABLE
     */
    uint8_t acc_off_en;

    /*! Enabling offset compensation for gyro in data registers
     * Assignable macros :
     *  - BMI160_ENABLE
     *  - BMI160_DISABLE
     */
    uint8_t gyro_off_en;
};

/*!
 * @brief bmi160 accel gyro offsets
 */
struct bmi160_offsets {
    /*! Accel offset for x axis */
    int8_t off_acc_x;

    /*! Accel offset for y axis */
    int8_t off_acc_y;

    /*! Accel offset for z axis */
    int8_t off_acc_z;

    /*! Gyro offset for x axis */
    int16_t off_gyro_x;

    /*! Gyro offset for y axis */
    int16_t off_gyro_y;

    /*! Gyro offset for z axis */
    int16_t off_gyro_z;
};

/*!
 * @brief FIFO aux. sensor data structure
 */
struct bmi160_aux_fifo_data {
    /*! The value of aux. sensor x LSB data */
    uint8_t aux_x_lsb;

    /*! The value of aux. sensor x MSB data */
    uint8_t aux_x_msb;

    /*! The value of aux. sensor y LSB data */
    uint8_t aux_y_lsb;

    /*! The value of aux. sensor y MSB data */
    uint8_t aux_y_msb;

    /*! The value of aux. sensor z LSB data */
    uint8_t aux_z_lsb;

    /*! The value of aux. sensor z MSB data */
    uint8_t aux_z_msb;

    /*! The value of aux. sensor r for BMM150 LSB data */
    uint8_t aux_r_y2_lsb;

    /*! The value of aux. sensor r for BMM150 MSB data */
    uint8_t aux_r_y2_msb;
};

/*!
 * @brief bmi160 sensor select structure
 */
enum bmi160_select_sensor { BMI160_ACCEL_ONLY = 1, BMI160_GYRO_ONLY, BMI160_BOTH_ACCEL_AND_GYRO };

/*!
 * @brief bmi160 sensor step detector mode structure
 */
enum bmi160_step_detect_mode {
    BMI160_STEP_DETECT_NORMAL,
    BMI160_STEP_DETECT_SENSITIVE,
    BMI160_STEP_DETECT_ROBUST,

    /*! Non recommended User defined setting */
    BMI160_STEP_DETECT_USER_DEFINE
};

/*!
 * @brief enum for auxiliary burst read selection
 */
enum bmi160_aux_read_len {
    BMI160_AUX_READ_LEN_0,
    BMI160_AUX_READ_LEN_1,
    BMI160_AUX_READ_LEN_2,
    BMI160_AUX_READ_LEN_3
};

/*!
 * @brief bmi160 sensor configuration structure
 */
struct bmi160_cfg {
    /*! power mode */
    uint8_t power;

    /*! output data rate */
    uint8_t odr;

    /*! range */
    uint8_t range;

    /*! bandwidth */
    uint8_t bw;
};

/*!
 * @brief Aux sensor configuration structure
 */
struct bmi160_aux_cfg {
    /*! Aux sensor, 1 - enable 0 - disable */
    uint8_t aux_sensor_enable : 1;

    /*! Aux manual/auto mode status */
    uint8_t manual_enable : 1;

    /*! Aux read burst length */
    uint8_t aux_rd_burst_len : 2;

    /*! output data rate */
    uint8_t aux_odr : 4;

    /*! i2c addr of auxiliary sensor */
    uint8_t aux_i2c_addr;
};

/*!
 * @brief bmi160 interrupt channel selection structure
 */
enum bmi160_int_channel {
    /*! Un-map both channels */
    BMI160_INT_CHANNEL_NONE,

    /*! interrupt Channel 1 */
    BMI160_INT_CHANNEL_1,

    /*! interrupt Channel 2 */
    BMI160_INT_CHANNEL_2,

    /*! Map both channels */
    BMI160_INT_CHANNEL_BOTH
};
enum bmi160_int_types {
    /*! Slope/Any-motion interrupt */
    BMI160_ACC_ANY_MOTION_INT,

    /*! Significant motion interrupt */
    BMI160_ACC_SIG_MOTION_INT,

    /*! Step detector interrupt */
    BMI160_STEP_DETECT_INT,

    /*! double tap interrupt */
    BMI160_ACC_DOUBLE_TAP_INT,

    /*! single tap interrupt */
    BMI160_ACC_SINGLE_TAP_INT,

    /*! orientation interrupt */
    BMI160_ACC_ORIENT_INT,

    /*! flat interrupt */
    BMI160_ACC_FLAT_INT,

    /*! high-g interrupt */
    BMI160_ACC_HIGH_G_INT,

    /*! low-g interrupt */
    BMI160_ACC_LOW_G_INT,

    /*! slow/no-motion interrupt */
    BMI160_ACC_SLOW_NO_MOTION_INT,

    /*! data ready interrupt  */
    BMI160_ACC_GYRO_DATA_RDY_INT,

    /*! fifo full interrupt */
    BMI160_ACC_GYRO_FIFO_FULL_INT,

    /*! fifo watermark interrupt */
    BMI160_ACC_GYRO_FIFO_WATERMARK_INT,

    /*! fifo tagging feature support */
    BMI160_FIFO_TAG_INT_PIN
};

/*!
 * @brief bmi160 active state of any & sig motion interrupt.
 */
enum bmi160_any_sig_motion_active_interrupt_state {
    /*! Both any & sig motion are disabled */
    BMI160_BOTH_ANY_SIG_MOTION_DISABLED = -1,

    /*! Any-motion selected */
    BMI160_ANY_MOTION_ENABLED,

    /*! Sig-motion selected */
    BMI160_SIG_MOTION_ENABLED
};
struct bmi160_acc_tap_int_cfg {
#ifdef LITTLE_ENDIAN

    /*! tap threshold */
    uint16_t tap_thr : 5;

    /*! tap shock */
    uint16_t tap_shock : 1;

    /*! tap quiet */
    uint16_t tap_quiet : 1;

    /*! tap duration */
    uint16_t tap_dur : 3;

    /*! data source 0- filter & 1 pre-filter*/
    uint16_t tap_data_src : 1;

    /*! tap enable, 1 - enable, 0 - disable */
    uint16_t tap_en : 1;
#else

    /*! tap enable, 1 - enable, 0 - disable */
    uint16_t tap_en : 1;

    /*! data source 0- filter & 1 pre-filter*/
    uint16_t tap_data_src : 1;

    /*! tap duration */
    uint16_t tap_dur : 3;

    /*! tap quiet */
    uint16_t tap_quiet : 1;

    /*! tap shock */
    uint16_t tap_shock : 1;

    /*! tap threshold */
    uint16_t tap_thr : 5;
#endif
};
struct bmi160_acc_any_mot_int_cfg {
#ifdef LITTLE_ENDIAN

    /*! 1 any-motion enable, 0 - any-motion disable */
    uint8_t anymotion_en : 1;

    /*! slope interrupt x, 1 - enable, 0 - disable */
    uint8_t anymotion_x : 1;

    /*! slope interrupt y, 1 - enable, 0 - disable */
    uint8_t anymotion_y : 1;

    /*! slope interrupt z, 1 - enable, 0 - disable */
    uint8_t anymotion_z : 1;

    /*! slope duration */
    uint8_t anymotion_dur : 2;

    /*! data source 0- filter & 1 pre-filter*/
    uint8_t anymotion_data_src : 1;

    /*! slope threshold */
    uint8_t anymotion_thr;
#else

    /*! slope threshold */
    uint8_t anymotion_thr;

    /*! data source 0- filter & 1 pre-filter*/
    uint8_t anymotion_data_src : 1;

    /*! slope duration */
    uint8_t anymotion_dur : 2;

    /*! slope interrupt z, 1 - enable, 0 - disable */
    uint8_t anymotion_z : 1;

    /*! slope interrupt y, 1 - enable, 0 - disable */
    uint8_t anymotion_y : 1;

    /*! slope interrupt x, 1 - enable, 0 - disable */
    uint8_t anymotion_x : 1;

    /*! 1 any-motion enable, 0 - any-motion disable */
    uint8_t anymotion_en : 1;
#endif
};
struct bmi160_acc_sig_mot_int_cfg {
#ifdef LITTLE_ENDIAN

    /*! skip time of sig-motion interrupt */
    uint8_t sig_mot_skip : 2;

    /*! proof time of sig-motion interrupt */
    uint8_t sig_mot_proof : 2;

    /*! data source 0- filter & 1 pre-filter*/
    uint8_t sig_data_src : 1;

    /*! 1 - enable sig, 0 - disable sig & enable anymotion */
    uint8_t sig_en : 1;

    /*! sig-motion threshold */
    uint8_t sig_mot_thres;
#else

    /*! sig-motion threshold */
    uint8_t sig_mot_thres;

    /*! 1 - enable sig, 0 - disable sig & enable anymotion */
    uint8_t sig_en : 1;

    /*! data source 0- filter & 1 pre-filter*/
    uint8_t sig_data_src : 1;

    /*! proof time of sig-motion interrupt */
    uint8_t sig_mot_proof : 2;

    /*! skip time of sig-motion interrupt */
    uint8_t sig_mot_skip : 2;
#endif
};
struct bmi160_acc_step_detect_int_cfg {
#ifdef LITTLE_ENDIAN

    /*! 1- step detector enable, 0- step detector disable */
    uint16_t step_detector_en : 1;

    /*! minimum threshold */
    uint16_t min_threshold : 2;

    /*! minimal detectable step time */
    uint16_t steptime_min : 3;

    /*! enable step counter mode setting */
    uint16_t step_detector_mode : 2;

    /*! minimum step buffer size*/
    uint16_t step_min_buf : 3;
#else

    /*! minimum step buffer size*/
    uint16_t step_min_buf : 3;

    /*! enable step counter mode setting */
    uint16_t step_detector_mode : 2;

    /*! minimal detectable step time */
    uint16_t steptime_min : 3;

    /*! minimum threshold */
    uint16_t min_threshold : 2;

    /*! 1- step detector enable, 0- step detector disable */
    uint16_t step_detector_en : 1;
#endif
};
struct bmi160_acc_no_motion_int_cfg {
#ifdef LITTLE_ENDIAN

    /*! no motion interrupt x */
    uint16_t no_motion_x : 1;

    /*! no motion interrupt y */
    uint16_t no_motion_y : 1;

    /*! no motion interrupt z */
    uint16_t no_motion_z : 1;

    /*! no motion duration */
    uint16_t no_motion_dur : 6;

    /*! no motion sel , 1 - enable no-motion ,0- enable slow-motion */
    uint16_t no_motion_sel : 1;

    /*! data source 0- filter & 1 pre-filter*/
    uint16_t no_motion_src : 1;

    /*! no motion threshold */
    uint8_t no_motion_thres;
#else

    /*! no motion threshold */
    uint8_t no_motion_thres;

    /*! data source 0- filter & 1 pre-filter*/
    uint16_t no_motion_src : 1;

    /*! no motion sel , 1 - enable no-motion ,0- enable slow-motion */
    uint16_t no_motion_sel : 1;

    /*! no motion duration */
    uint16_t no_motion_dur : 6;

    /* no motion interrupt z */
    uint16_t no_motion_z : 1;

    /*! no motion interrupt y */
    uint16_t no_motion_y : 1;

    /*! no motion interrupt x */
    uint16_t no_motion_x : 1;
#endif
};
struct bmi160_acc_orient_int_cfg {
#ifdef LITTLE_ENDIAN

    /*! thresholds for switching between the different orientations */
    uint16_t orient_mode : 2;

    /*! blocking_mode */
    uint16_t orient_blocking : 2;

    /*! Orientation interrupt hysteresis */
    uint16_t orient_hyst : 4;

    /*! Orientation interrupt theta */
    uint16_t orient_theta : 6;

    /*! Enable/disable Orientation interrupt */
    uint16_t orient_ud_en : 1;

    /*! exchange x- and z-axis in algorithm ,0 - z, 1 - x */
    uint16_t axes_ex : 1;

    /*! 1 - orient enable, 0 - orient disable */
    uint8_t orient_en : 1;
#else

    /*! 1 - orient enable, 0 - orient disable */
    uint8_t orient_en : 1;

    /*! exchange x- and z-axis in algorithm ,0 - z, 1 - x */
    uint16_t axes_ex : 1;

    /*! Enable/disable Orientation interrupt */
    uint16_t orient_ud_en : 1;

    /*! Orientation interrupt theta */
    uint16_t orient_theta : 6;

    /*! Orientation interrupt hysteresis */
    uint16_t orient_hyst : 4;

    /*! blocking_mode */
    uint16_t orient_blocking : 2;

    /*! thresholds for switching between the different orientations */
    uint16_t orient_mode : 2;
#endif
};
struct bmi160_acc_flat_detect_int_cfg {
#ifdef LITTLE_ENDIAN

    /*! flat threshold */
    uint16_t flat_theta : 6;

    /*! flat interrupt hysteresis */
    uint16_t flat_hy : 3;

    /*! delay time for which the flat value must remain stable for the
     * flat interrupt to be generated */
    uint16_t flat_hold_time : 2;

    /*! 1 - flat enable, 0 - flat disable */
    uint16_t flat_en : 1;
#else

    /*! 1 - flat enable, 0 - flat disable */
    uint16_t flat_en : 1;

    /*! delay time for which the flat value must remain stable for the
     * flat interrupt to be generated */
    uint16_t flat_hold_time : 2;

    /*! flat interrupt hysteresis */
    uint16_t flat_hy : 3;

    /*! flat threshold */
    uint16_t flat_theta : 6;
#endif
};
struct bmi160_acc_low_g_int_cfg {
#ifdef LITTLE_ENDIAN

    /*! low-g interrupt trigger delay */
    uint8_t low_dur;

    /*! low-g interrupt trigger threshold */
    uint8_t low_thres;

    /*! hysteresis of low-g interrupt */
    uint8_t low_hyst : 2;

    /*! 0 - single-axis mode ,1 - axis-summing mode */
    uint8_t low_mode : 1;

    /*! data source 0- filter & 1 pre-filter */
    uint8_t low_data_src : 1;

    /*! 1 - enable low-g, 0 - disable low-g */
    uint8_t low_en : 1;
#else

    /*! 1 - enable low-g, 0 - disable low-g */
    uint8_t low_en : 1;

    /*! data source 0- filter & 1 pre-filter */
    uint8_t low_data_src : 1;

    /*! 0 - single-axis mode ,1 - axis-summing mode */
    uint8_t low_mode : 1;

    /*! hysteresis of low-g interrupt */
    uint8_t low_hyst : 2;

    /*! low-g interrupt trigger threshold */
    uint8_t low_thres;

    /*! low-g interrupt trigger delay */
    uint8_t low_dur;
#endif
};
struct bmi160_acc_high_g_int_cfg {
#ifdef LITTLE_ENDIAN

    /*! High-g interrupt x, 1 - enable, 0 - disable */
    uint8_t high_g_x : 1;

    /*! High-g interrupt y, 1 - enable, 0 - disable */
    uint8_t high_g_y : 1;

    /*! High-g interrupt z, 1 - enable, 0 - disable */
    uint8_t high_g_z : 1;

    /*! High-g hysteresis  */
    uint8_t high_hy : 2;

    /*! data source 0- filter & 1 pre-filter */
    uint8_t high_data_src : 1;

    /*! High-g threshold */
    uint8_t high_thres;

    /*! High-g duration */
    uint8_t high_dur;
#else

    /*! High-g duration */
    uint8_t high_dur;

    /*! High-g threshold */
    uint8_t high_thres;

    /*! data source 0- filter & 1 pre-filter */
    uint8_t high_data_src : 1;

    /*! High-g hysteresis  */
    uint8_t high_hy : 2;

    /*! High-g interrupt z, 1 - enable, 0 - disable */
    uint8_t high_g_z : 1;

    /*! High-g interrupt y, 1 - enable, 0 - disable */
    uint8_t high_g_y : 1;

    /*! High-g interrupt x, 1 - enable, 0 - disable */
    uint8_t high_g_x : 1;
#endif
};
struct bmi160_int_pin_settg {
#ifdef LITTLE_ENDIAN

    /*! To enable either INT1 or INT2 pin as output.
     * 0- output disabled ,1- output enabled */
    uint16_t output_en : 1;

    /*! 0 - push-pull 1- open drain,only valid if output_en is set 1 */
    uint16_t output_mode : 1;

    /*! 0 - active low , 1 - active high level.
     * if output_en is 1,this applies to interrupts,else PMU_trigger */
    uint16_t output_type : 1;

    /*! 0 - level trigger , 1 - edge trigger  */
    uint16_t edge_ctrl : 1;

    /*! To enable either INT1 or INT2 pin as input.
     * 0 - input disabled ,1 - input enabled */
    uint16_t input_en : 1;

    /*! latch duration*/
    uint16_t latch_dur : 4;
#else

    /*! latch duration*/
    uint16_t latch_dur : 4;

    /*! Latched,non-latched or temporary interrupt modes */
    uint16_t input_en : 1;

    /*! 1 - edge trigger, 0 - level trigger */
    uint16_t edge_ctrl : 1;

    /*! 0 - active low , 1 - active high level.
     * if output_en is 1,this applies to interrupts,else PMU_trigger */
    uint16_t output_type : 1;

    /*! 0 - push-pull , 1 - open drain,only valid if output_en is set 1 */
    uint16_t output_mode : 1;

    /*! To enable either INT1 or INT2 pin as output.
     * 0 - output disabled , 1 - output enabled */
    uint16_t output_en : 1;
#endif
};
union bmi160_int_type_cfg {
    /*! Tap interrupt structure */
    struct bmi160_acc_tap_int_cfg acc_tap_int;

    /*! Slope interrupt structure */
    struct bmi160_acc_any_mot_int_cfg acc_any_motion_int;

    /*! Significant motion interrupt structure */
    struct bmi160_acc_sig_mot_int_cfg acc_sig_motion_int;

    /*! Step detector interrupt structure */
    struct bmi160_acc_step_detect_int_cfg acc_step_detect_int;

    /*! No motion interrupt structure */
    struct bmi160_acc_no_motion_int_cfg acc_no_motion_int;

    /*! Orientation interrupt structure */
    struct bmi160_acc_orient_int_cfg acc_orient_int;

    /*! Flat interrupt structure */
    struct bmi160_acc_flat_detect_int_cfg acc_flat_int;

    /*! Low-g interrupt structure */
    struct bmi160_acc_low_g_int_cfg acc_low_g_int;

    /*! High-g interrupt structure */
    struct bmi160_acc_high_g_int_cfg acc_high_g_int;
};
struct bmi160_int_settg {
    /*! Interrupt channel */
    enum bmi160_int_channel int_channel;

    /*! Select Interrupt */
    enum bmi160_int_types int_type;

    /*! Structure configuring Interrupt pins */
    struct bmi160_int_pin_settg int_pin_settg;

    /*! Union configures required interrupt */
    union bmi160_int_type_cfg int_type_cfg;

    /*! FIFO FULL INT 1-enable, 0-disable */
    uint8_t fifo_full_int_en : 1;

    /*! FIFO WTM INT 1-enable, 0-disable */
    uint8_t fifo_wtm_int_en : 1;
};

/*!
 *  @brief This structure holds the information for usage of
 *  FIFO by the user.
 */
struct bmi160_fifo_frame {
    /*! Data buffer of user defined length is to be mapped here */
    uint8_t* data;

    /*! While calling the API  "bmi160_get_fifo_data" , length stores
     *  number of bytes in FIFO to be read (specified by user as input)
     *  and after execution of the API ,number of FIFO data bytes
     *  available is provided as an output to user
     */
    uint16_t length;

    /*! FIFO time enable */
    uint8_t fifo_time_enable;

    /*! Enabling of the FIFO header to stream in header mode */
    uint8_t fifo_header_enable;

    /*! Streaming of the Accelerometer, Gyroscope
     * sensor data or both in FIFO */
    uint8_t fifo_data_enable;

    /*! Will be equal to length when no more frames are there to parse */
    uint16_t accel_byte_start_idx;

    /*! Will be equal to length when no more frames are there to parse */
    uint16_t gyro_byte_start_idx;

    /*! Will be equal to length when no more frames are there to parse */
    uint16_t aux_byte_start_idx;

    /*! Value of FIFO sensor time time */
    uint32_t sensor_time;

    /*! Value of Skipped frame counts */
    uint8_t skipped_frame_count;
};
struct bmi160_dev {
    /*! Chip Id */
    uint8_t chip_id;

    /*! Device Id */
    uint8_t id;

    /*! 0 - I2C , 1 - SPI Interface */
    uint8_t intf;

    /*! Hold active interrupts status for any and sig motion
     *  0 - Any-motion enable, 1 - Sig-motion enable,
     *  -1 neither any-motion nor sig-motion selected */
    enum bmi160_any_sig_motion_active_interrupt_state any_sig_sel;

    /*! Structure to configure Accel sensor */
    struct bmi160_cfg accel_cfg;

    /*! Structure to hold previous/old accel config parameters.
     * This is used at driver level to prevent overwriting of same
     * data, hence user does not change it in the code */
    struct bmi160_cfg prev_accel_cfg;

    /*! Structure to configure Gyro sensor */
    struct bmi160_cfg gyro_cfg;

    /*! Structure to hold previous/old gyro config parameters.
     * This is used at driver level to prevent overwriting of same
     * data, hence user does not change it in the code */
    struct bmi160_cfg prev_gyro_cfg;

    /*! Structure to configure the auxiliary sensor */
    struct bmi160_aux_cfg aux_cfg;

    /*! Structure to hold previous/old aux config parameters.
     * This is used at driver level to prevent overwriting of same
     * data, hence user does not change it in the code */
    struct bmi160_aux_cfg prev_aux_cfg;

    /*! FIFO related configurations */
    struct bmi160_fifo_frame* fifo;

    /*! Read function pointer */
    bmi160_read_fptr_t read;

    /*! Write function pointer */
    bmi160_write_fptr_t write;

    /*!  Delay function pointer */
    bmi160_delay_fptr_t delay_ms;

    /*! User set read/write length */
    uint16_t read_write_len;
};

#endif /* BMI160_DEFS_H_ */
