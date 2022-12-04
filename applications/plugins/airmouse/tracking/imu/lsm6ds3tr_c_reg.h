/**
  ******************************************************************************
  * @file    lsm6ds3tr_c_reg.h
  * @author  Sensors Software Solution Team
  * @brief   This file contains all the functions prototypes for the
  *          lsm6ds3tr_c_reg.c driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LSM6DS3TR_C_DRIVER_H
#define LSM6DS3TR_C_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>
#include <math.h>

/** @addtogroup LSM6DS3TR_C
  * @{
  *
  */

/** @defgroup  Endianness definitions
  * @{
  *
  */

#ifndef DRV_BYTE_ORDER
#ifndef __BYTE_ORDER__

#define DRV_LITTLE_ENDIAN 1234
#define DRV_BIG_ENDIAN 4321

/** if _BYTE_ORDER is not defined, choose the endianness of your architecture
  * by uncommenting the define which fits your platform endianness
  */
//#define DRV_BYTE_ORDER    DRV_BIG_ENDIAN
#define DRV_BYTE_ORDER DRV_LITTLE_ENDIAN

#else /* defined __BYTE_ORDER__ */

#define DRV_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define DRV_BIG_ENDIAN __ORDER_BIG_ENDIAN__
#define DRV_BYTE_ORDER __BYTE_ORDER__

#endif /* __BYTE_ORDER__*/
#endif /* DRV_BYTE_ORDER */

/**
  * @}
  *
  */

/** @defgroup STMicroelectronics sensors common types
  * @{
  *
  */

#ifndef MEMS_SHARED_TYPES
#define MEMS_SHARED_TYPES

typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} bitwise_t;

#define PROPERTY_DISABLE (0U)
#define PROPERTY_ENABLE (1U)

/** @addtogroup  Interfaces_Functions
  * @brief       This section provide a set of functions used to read and
  *              write a generic register of the device.
  *              MANDATORY: return 0 -> no Error.
  * @{
  *
  */

typedef int32_t (*stmdev_write_ptr)(void*, uint8_t, const uint8_t*, uint16_t);
typedef int32_t (*stmdev_read_ptr)(void*, uint8_t, uint8_t*, uint16_t);
typedef void (*stmdev_mdelay_ptr)(uint32_t millisec);

typedef struct {
    /** Component mandatory fields **/
    stmdev_write_ptr write_reg;
    stmdev_read_ptr read_reg;
    /** Component optional fields **/
    stmdev_mdelay_ptr mdelay;
    /** Customizable optional pointer **/
    void* handle;
} stmdev_ctx_t;

/**
  * @}
  *
  */

#endif /* MEMS_SHARED_TYPES */

#ifndef MEMS_UCF_SHARED_TYPES
#define MEMS_UCF_SHARED_TYPES

/** @defgroup    Generic address-data structure definition
  * @brief       This structure is useful to load a predefined configuration
  *              of a sensor.
  *              You can create a sensor configuration by your own or using
  *              Unico / Unicleo tools available on STMicroelectronics
  *              web site.
  *
  * @{
  *
  */

typedef struct {
    uint8_t address;
    uint8_t data;
} ucf_line_t;

/**
  * @}
  *
  */

#endif /* MEMS_UCF_SHARED_TYPES */

/**
  * @}
  *
  */

/** @defgroup LSM6DS3TR_C_Infos
  * @{
  *
  */

/** I2C Device Address 8 bit format  if SA0=0 -> D5 if SA0=1 -> D7 **/
#define LSM6DS3TR_C_I2C_ADD_L 0xD5U
#define LSM6DS3TR_C_I2C_ADD_H 0xD7U

/** Device Identification (Who am I) **/
#define LSM6DS3TR_C_ID 0x6AU

/**
  * @}
  *
  */

#define LSM6DS3TR_C_FUNC_CFG_ACCESS 0x01U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t not_used_01 : 5;
    uint8_t func_cfg_en : 3; /* func_cfg_en + func_cfg_en_b */
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t func_cfg_en : 3; /* func_cfg_en + func_cfg_en_b */
    uint8_t not_used_01 : 5;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_func_cfg_access_t;

#define LSM6DS3TR_C_SENSOR_SYNC_TIME_FRAME 0x04U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t tph : 4;
    uint8_t not_used_01 : 4;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01 : 4;
    uint8_t tph : 4;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensor_sync_time_frame_t;

#define LSM6DS3TR_C_SENSOR_SYNC_RES_RATIO 0x05U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t rr : 2;
    uint8_t not_used_01 : 6;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01 : 6;
    uint8_t rr : 2;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensor_sync_res_ratio_t;

#define LSM6DS3TR_C_FIFO_CTRL1 0x06U
typedef struct {
    uint8_t fth : 8; /* + FIFO_CTRL2(fth) */
} lsm6ds3tr_c_fifo_ctrl1_t;

#define LSM6DS3TR_C_FIFO_CTRL2 0x07U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t fth : 3; /* + FIFO_CTRL1(fth) */
    uint8_t fifo_temp_en : 1;
    uint8_t not_used_01 : 2;
    uint8_t timer_pedo_fifo_drdy : 1;
    uint8_t timer_pedo_fifo_en : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t timer_pedo_fifo_en : 1;
    uint8_t timer_pedo_fifo_drdy : 1;
    uint8_t not_used_01 : 2;
    uint8_t fifo_temp_en : 1;
    uint8_t fth : 3; /* + FIFO_CTRL1(fth) */
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_fifo_ctrl2_t;

#define LSM6DS3TR_C_FIFO_CTRL3 0x08U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t dec_fifo_xl : 3;
    uint8_t dec_fifo_gyro : 3;
    uint8_t not_used_01 : 2;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01 : 2;
    uint8_t dec_fifo_gyro : 3;
    uint8_t dec_fifo_xl : 3;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_fifo_ctrl3_t;

#define LSM6DS3TR_C_FIFO_CTRL4 0x09U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t dec_ds3_fifo : 3;
    uint8_t dec_ds4_fifo : 3;
    uint8_t only_high_data : 1;
    uint8_t stop_on_fth : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t stop_on_fth : 1;
    uint8_t only_high_data : 1;
    uint8_t dec_ds4_fifo : 3;
    uint8_t dec_ds3_fifo : 3;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_fifo_ctrl4_t;

#define LSM6DS3TR_C_FIFO_CTRL5 0x0AU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t fifo_mode : 3;
    uint8_t odr_fifo : 4;
    uint8_t not_used_01 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01 : 1;
    uint8_t odr_fifo : 4;
    uint8_t fifo_mode : 3;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_fifo_ctrl5_t;

#define LSM6DS3TR_C_DRDY_PULSE_CFG_G 0x0BU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t int2_wrist_tilt : 1;
    uint8_t not_used_01 : 6;
    uint8_t drdy_pulsed : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t drdy_pulsed : 1;
    uint8_t not_used_01 : 6;
    uint8_t int2_wrist_tilt : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_drdy_pulse_cfg_g_t;

#define LSM6DS3TR_C_INT1_CTRL 0x0DU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t int1_drdy_xl : 1;
    uint8_t int1_drdy_g : 1;
    uint8_t int1_boot : 1;
    uint8_t int1_fth : 1;
    uint8_t int1_fifo_ovr : 1;
    uint8_t int1_full_flag : 1;
    uint8_t int1_sign_mot : 1;
    uint8_t int1_step_detector : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t int1_step_detector : 1;
    uint8_t int1_sign_mot : 1;
    uint8_t int1_full_flag : 1;
    uint8_t int1_fifo_ovr : 1;
    uint8_t int1_fth : 1;
    uint8_t int1_boot : 1;
    uint8_t int1_drdy_g : 1;
    uint8_t int1_drdy_xl : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_int1_ctrl_t;

#define LSM6DS3TR_C_INT2_CTRL 0x0EU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t int2_drdy_xl : 1;
    uint8_t int2_drdy_g : 1;
    uint8_t int2_drdy_temp : 1;
    uint8_t int2_fth : 1;
    uint8_t int2_fifo_ovr : 1;
    uint8_t int2_full_flag : 1;
    uint8_t int2_step_count_ov : 1;
    uint8_t int2_step_delta : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t int2_step_delta : 1;
    uint8_t int2_step_count_ov : 1;
    uint8_t int2_full_flag : 1;
    uint8_t int2_fifo_ovr : 1;
    uint8_t int2_fth : 1;
    uint8_t int2_drdy_temp : 1;
    uint8_t int2_drdy_g : 1;
    uint8_t int2_drdy_xl : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_int2_ctrl_t;

#define LSM6DS3TR_C_WHO_AM_I 0x0FU
#define LSM6DS3TR_C_CTRL1_XL 0x10U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bw0_xl : 1;
    uint8_t lpf1_bw_sel : 1;
    uint8_t fs_xl : 2;
    uint8_t odr_xl : 4;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t odr_xl : 4;
    uint8_t fs_xl : 2;
    uint8_t lpf1_bw_sel : 1;
    uint8_t bw0_xl : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_ctrl1_xl_t;

#define LSM6DS3TR_C_CTRL2_G 0x11U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t not_used_01 : 1;
    uint8_t fs_g : 3; /* fs_g + fs_125 */
    uint8_t odr_g : 4;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t odr_g : 4;
    uint8_t fs_g : 3; /* fs_g + fs_125 */
    uint8_t not_used_01 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_ctrl2_g_t;

#define LSM6DS3TR_C_CTRL3_C 0x12U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t sw_reset : 1;
    uint8_t ble : 1;
    uint8_t if_inc : 1;
    uint8_t sim : 1;
    uint8_t pp_od : 1;
    uint8_t h_lactive : 1;
    uint8_t bdu : 1;
    uint8_t boot : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t boot : 1;
    uint8_t bdu : 1;
    uint8_t h_lactive : 1;
    uint8_t pp_od : 1;
    uint8_t sim : 1;
    uint8_t if_inc : 1;
    uint8_t ble : 1;
    uint8_t sw_reset : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_ctrl3_c_t;

#define LSM6DS3TR_C_CTRL4_C 0x13U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t not_used_01 : 1;
    uint8_t lpf1_sel_g : 1;
    uint8_t i2c_disable : 1;
    uint8_t drdy_mask : 1;
    uint8_t den_drdy_int1 : 1;
    uint8_t int2_on_int1 : 1;
    uint8_t sleep : 1;
    uint8_t den_xl_en : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t den_xl_en : 1;
    uint8_t sleep : 1;
    uint8_t int2_on_int1 : 1;
    uint8_t den_drdy_int1 : 1;
    uint8_t drdy_mask : 1;
    uint8_t i2c_disable : 1;
    uint8_t lpf1_sel_g : 1;
    uint8_t not_used_01 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_ctrl4_c_t;

#define LSM6DS3TR_C_CTRL5_C 0x14U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t st_xl : 2;
    uint8_t st_g : 2;
    uint8_t den_lh : 1;
    uint8_t rounding : 3;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t rounding : 3;
    uint8_t den_lh : 1;
    uint8_t st_g : 2;
    uint8_t st_xl : 2;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_ctrl5_c_t;

#define LSM6DS3TR_C_CTRL6_C 0x15U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t ftype : 2;
    uint8_t not_used_01 : 1;
    uint8_t usr_off_w : 1;
    uint8_t xl_hm_mode : 1;
    uint8_t den_mode : 3; /* trig_en + lvl_en + lvl2_en */
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t den_mode : 3; /* trig_en + lvl_en + lvl2_en */
    uint8_t xl_hm_mode : 1;
    uint8_t usr_off_w : 1;
    uint8_t not_used_01 : 1;
    uint8_t ftype : 2;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_ctrl6_c_t;

#define LSM6DS3TR_C_CTRL7_G 0x16U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t not_used_01 : 2;
    uint8_t rounding_status : 1;
    uint8_t not_used_02 : 1;
    uint8_t hpm_g : 2;
    uint8_t hp_en_g : 1;
    uint8_t g_hm_mode : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t g_hm_mode : 1;
    uint8_t hp_en_g : 1;
    uint8_t hpm_g : 2;
    uint8_t not_used_02 : 1;
    uint8_t rounding_status : 1;
    uint8_t not_used_01 : 2;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_ctrl7_g_t;

#define LSM6DS3TR_C_CTRL8_XL 0x17U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t low_pass_on_6d : 1;
    uint8_t not_used_01 : 1;
    uint8_t hp_slope_xl_en : 1;
    uint8_t input_composite : 1;
    uint8_t hp_ref_mode : 1;
    uint8_t hpcf_xl : 2;
    uint8_t lpf2_xl_en : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t lpf2_xl_en : 1;
    uint8_t hpcf_xl : 2;
    uint8_t hp_ref_mode : 1;
    uint8_t input_composite : 1;
    uint8_t hp_slope_xl_en : 1;
    uint8_t not_used_01 : 1;
    uint8_t low_pass_on_6d : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_ctrl8_xl_t;

#define LSM6DS3TR_C_CTRL9_XL 0x18U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t not_used_01 : 2;
    uint8_t soft_en : 1;
    uint8_t not_used_02 : 1;
    uint8_t den_xl_g : 1;
    uint8_t den_z : 1;
    uint8_t den_y : 1;
    uint8_t den_x : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t den_x : 1;
    uint8_t den_y : 1;
    uint8_t den_z : 1;
    uint8_t den_xl_g : 1;
    uint8_t not_used_02 : 1;
    uint8_t soft_en : 1;
    uint8_t not_used_01 : 2;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_ctrl9_xl_t;

#define LSM6DS3TR_C_CTRL10_C 0x19U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t sign_motion_en : 1;
    uint8_t pedo_rst_step : 1;
    uint8_t func_en : 1;
    uint8_t tilt_en : 1;
    uint8_t pedo_en : 1;
    uint8_t timer_en : 1;
    uint8_t not_used_01 : 1;
    uint8_t wrist_tilt_en : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t wrist_tilt_en : 1;
    uint8_t not_used_01 : 1;
    uint8_t timer_en : 1;
    uint8_t pedo_en : 1;
    uint8_t tilt_en : 1;
    uint8_t func_en : 1;
    uint8_t pedo_rst_step : 1;
    uint8_t sign_motion_en : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_ctrl10_c_t;

#define LSM6DS3TR_C_MASTER_CONFIG 0x1AU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t master_on : 1;
    uint8_t iron_en : 1;
    uint8_t pass_through_mode : 1;
    uint8_t pull_up_en : 1;
    uint8_t start_config : 1;
    uint8_t not_used_01 : 1;
    uint8_t data_valid_sel_fifo : 1;
    uint8_t drdy_on_int1 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t drdy_on_int1 : 1;
    uint8_t data_valid_sel_fifo : 1;
    uint8_t not_used_01 : 1;
    uint8_t start_config : 1;
    uint8_t pull_up_en : 1;
    uint8_t pass_through_mode : 1;
    uint8_t iron_en : 1;
    uint8_t master_on : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_master_config_t;

#define LSM6DS3TR_C_WAKE_UP_SRC 0x1BU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t z_wu : 1;
    uint8_t y_wu : 1;
    uint8_t x_wu : 1;
    uint8_t wu_ia : 1;
    uint8_t sleep_state_ia : 1;
    uint8_t ff_ia : 1;
    uint8_t not_used_01 : 2;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01 : 2;
    uint8_t ff_ia : 1;
    uint8_t sleep_state_ia : 1;
    uint8_t wu_ia : 1;
    uint8_t x_wu : 1;
    uint8_t y_wu : 1;
    uint8_t z_wu : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_wake_up_src_t;

#define LSM6DS3TR_C_TAP_SRC 0x1CU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t z_tap : 1;
    uint8_t y_tap : 1;
    uint8_t x_tap : 1;
    uint8_t tap_sign : 1;
    uint8_t double_tap : 1;
    uint8_t single_tap : 1;
    uint8_t tap_ia : 1;
    uint8_t not_used_01 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01 : 1;
    uint8_t tap_ia : 1;
    uint8_t single_tap : 1;
    uint8_t double_tap : 1;
    uint8_t tap_sign : 1;
    uint8_t x_tap : 1;
    uint8_t y_tap : 1;
    uint8_t z_tap : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_tap_src_t;

#define LSM6DS3TR_C_D6D_SRC 0x1DU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t xl : 1;
    uint8_t xh : 1;
    uint8_t yl : 1;
    uint8_t yh : 1;
    uint8_t zl : 1;
    uint8_t zh : 1;
    uint8_t d6d_ia : 1;
    uint8_t den_drdy : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t den_drdy : 1;
    uint8_t d6d_ia : 1;
    uint8_t zh : 1;
    uint8_t zl : 1;
    uint8_t yh : 1;
    uint8_t yl : 1;
    uint8_t xh : 1;
    uint8_t xl : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_d6d_src_t;

#define LSM6DS3TR_C_STATUS_REG 0x1EU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t xlda : 1;
    uint8_t gda : 1;
    uint8_t tda : 1;
    uint8_t not_used_01 : 5;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01 : 5;
    uint8_t tda : 1;
    uint8_t gda : 1;
    uint8_t xlda : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_status_reg_t;

#define LSM6DS3TR_C_OUT_TEMP_L 0x20U
#define LSM6DS3TR_C_OUT_TEMP_H 0x21U
#define LSM6DS3TR_C_OUTX_L_G 0x22U
#define LSM6DS3TR_C_OUTX_H_G 0x23U
#define LSM6DS3TR_C_OUTY_L_G 0x24U
#define LSM6DS3TR_C_OUTY_H_G 0x25U
#define LSM6DS3TR_C_OUTZ_L_G 0x26U
#define LSM6DS3TR_C_OUTZ_H_G 0x27U
#define LSM6DS3TR_C_OUTX_L_XL 0x28U
#define LSM6DS3TR_C_OUTX_H_XL 0x29U
#define LSM6DS3TR_C_OUTY_L_XL 0x2AU
#define LSM6DS3TR_C_OUTY_H_XL 0x2BU
#define LSM6DS3TR_C_OUTZ_L_XL 0x2CU
#define LSM6DS3TR_C_OUTZ_H_XL 0x2DU
#define LSM6DS3TR_C_SENSORHUB1_REG 0x2EU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub1_reg_t;

#define LSM6DS3TR_C_SENSORHUB2_REG 0x2FU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub2_reg_t;

#define LSM6DS3TR_C_SENSORHUB3_REG 0x30U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub3_reg_t;

#define LSM6DS3TR_C_SENSORHUB4_REG 0x31U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub4_reg_t;

#define LSM6DS3TR_C_SENSORHUB5_REG 0x32U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub5_reg_t;

#define LSM6DS3TR_C_SENSORHUB6_REG 0x33U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub6_reg_t;

#define LSM6DS3TR_C_SENSORHUB7_REG 0x34U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub7_reg_t;

#define LSM6DS3TR_C_SENSORHUB8_REG 0x35U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub8_reg_t;

#define LSM6DS3TR_C_SENSORHUB9_REG 0x36U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub9_reg_t;

#define LSM6DS3TR_C_SENSORHUB10_REG 0x37U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub10_reg_t;

#define LSM6DS3TR_C_SENSORHUB11_REG 0x38U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub11_reg_t;

#define LSM6DS3TR_C_SENSORHUB12_REG 0x39U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub12_reg_t;

#define LSM6DS3TR_C_FIFO_STATUS1 0x3AU
typedef struct {
    uint8_t diff_fifo : 8; /* + FIFO_STATUS2(diff_fifo) */
} lsm6ds3tr_c_fifo_status1_t;

#define LSM6DS3TR_C_FIFO_STATUS2 0x3BU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t diff_fifo : 3; /* + FIFO_STATUS1(diff_fifo) */
    uint8_t not_used_01 : 1;
    uint8_t fifo_empty : 1;
    uint8_t fifo_full_smart : 1;
    uint8_t over_run : 1;
    uint8_t waterm : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t waterm : 1;
    uint8_t over_run : 1;
    uint8_t fifo_full_smart : 1;
    uint8_t fifo_empty : 1;
    uint8_t not_used_01 : 1;
    uint8_t diff_fifo : 3; /* + FIFO_STATUS1(diff_fifo) */
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_fifo_status2_t;

#define LSM6DS3TR_C_FIFO_STATUS3 0x3CU
typedef struct {
    uint8_t fifo_pattern : 8; /* + FIFO_STATUS4(fifo_pattern) */
} lsm6ds3tr_c_fifo_status3_t;

#define LSM6DS3TR_C_FIFO_STATUS4 0x3DU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t fifo_pattern : 2; /* + FIFO_STATUS3(fifo_pattern) */
    uint8_t not_used_01 : 6;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01 : 6;
    uint8_t fifo_pattern : 2; /* + FIFO_STATUS3(fifo_pattern) */
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_fifo_status4_t;

#define LSM6DS3TR_C_FIFO_DATA_OUT_L 0x3EU
#define LSM6DS3TR_C_FIFO_DATA_OUT_H 0x3FU
#define LSM6DS3TR_C_TIMESTAMP0_REG 0x40U
#define LSM6DS3TR_C_TIMESTAMP1_REG 0x41U
#define LSM6DS3TR_C_TIMESTAMP2_REG 0x42U
#define LSM6DS3TR_C_STEP_TIMESTAMP_L 0x49U
#define LSM6DS3TR_C_STEP_TIMESTAMP_H 0x4AU
#define LSM6DS3TR_C_STEP_COUNTER_L 0x4BU
#define LSM6DS3TR_C_STEP_COUNTER_H 0x4CU

#define LSM6DS3TR_C_SENSORHUB13_REG 0x4DU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub13_reg_t;

#define LSM6DS3TR_C_SENSORHUB14_REG 0x4EU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub14_reg_t;

#define LSM6DS3TR_C_SENSORHUB15_REG 0x4FU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub15_reg_t;

#define LSM6DS3TR_C_SENSORHUB16_REG 0x50U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub16_reg_t;

#define LSM6DS3TR_C_SENSORHUB17_REG 0x51U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub17_reg_t;

#define LSM6DS3TR_C_SENSORHUB18_REG 0x52U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7 : 1;
    uint8_t bit6 : 1;
    uint8_t bit5 : 1;
    uint8_t bit4 : 1;
    uint8_t bit3 : 1;
    uint8_t bit2 : 1;
    uint8_t bit1 : 1;
    uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_sensorhub18_reg_t;

#define LSM6DS3TR_C_FUNC_SRC1 0x53U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t sensorhub_end_op : 1;
    uint8_t si_end_op : 1;
    uint8_t hi_fail : 1;
    uint8_t step_overflow : 1;
    uint8_t step_detected : 1;
    uint8_t tilt_ia : 1;
    uint8_t sign_motion_ia : 1;
    uint8_t step_count_delta_ia : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t step_count_delta_ia : 1;
    uint8_t sign_motion_ia : 1;
    uint8_t tilt_ia : 1;
    uint8_t step_detected : 1;
    uint8_t step_overflow : 1;
    uint8_t hi_fail : 1;
    uint8_t si_end_op : 1;
    uint8_t sensorhub_end_op : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_func_src1_t;

#define LSM6DS3TR_C_FUNC_SRC2 0x54U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t wrist_tilt_ia : 1;
    uint8_t not_used_01 : 2;
    uint8_t slave0_nack : 1;
    uint8_t slave1_nack : 1;
    uint8_t slave2_nack : 1;
    uint8_t slave3_nack : 1;
    uint8_t not_used_02 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_02 : 1;
    uint8_t slave3_nack : 1;
    uint8_t slave2_nack : 1;
    uint8_t slave1_nack : 1;
    uint8_t slave0_nack : 1;
    uint8_t not_used_01 : 2;
    uint8_t wrist_tilt_ia : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_func_src2_t;

#define LSM6DS3TR_C_WRIST_TILT_IA 0x55U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t not_used_01 : 2;
    uint8_t wrist_tilt_ia_zneg : 1;
    uint8_t wrist_tilt_ia_zpos : 1;
    uint8_t wrist_tilt_ia_yneg : 1;
    uint8_t wrist_tilt_ia_ypos : 1;
    uint8_t wrist_tilt_ia_xneg : 1;
    uint8_t wrist_tilt_ia_xpos : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t wrist_tilt_ia_xpos : 1;
    uint8_t wrist_tilt_ia_xneg : 1;
    uint8_t wrist_tilt_ia_ypos : 1;
    uint8_t wrist_tilt_ia_yneg : 1;
    uint8_t wrist_tilt_ia_zpos : 1;
    uint8_t wrist_tilt_ia_zneg : 1;
    uint8_t not_used_01 : 2;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_wrist_tilt_ia_t;

#define LSM6DS3TR_C_TAP_CFG 0x58U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t lir : 1;
    uint8_t tap_z_en : 1;
    uint8_t tap_y_en : 1;
    uint8_t tap_x_en : 1;
    uint8_t slope_fds : 1;
    uint8_t inact_en : 2;
    uint8_t interrupts_enable : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t interrupts_enable : 1;
    uint8_t inact_en : 2;
    uint8_t slope_fds : 1;
    uint8_t tap_x_en : 1;
    uint8_t tap_y_en : 1;
    uint8_t tap_z_en : 1;
    uint8_t lir : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_tap_cfg_t;

#define LSM6DS3TR_C_TAP_THS_6D 0x59U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t tap_ths : 5;
    uint8_t sixd_ths : 2;
    uint8_t d4d_en : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t d4d_en : 1;
    uint8_t sixd_ths : 2;
    uint8_t tap_ths : 5;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_tap_ths_6d_t;

#define LSM6DS3TR_C_INT_DUR2 0x5AU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t shock : 2;
    uint8_t quiet : 2;
    uint8_t dur : 4;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t dur : 4;
    uint8_t quiet : 2;
    uint8_t shock : 2;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_int_dur2_t;

#define LSM6DS3TR_C_WAKE_UP_THS 0x5BU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t wk_ths : 6;
    uint8_t not_used_01 : 1;
    uint8_t single_double_tap : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t single_double_tap : 1;
    uint8_t not_used_01 : 1;
    uint8_t wk_ths : 6;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_wake_up_ths_t;

#define LSM6DS3TR_C_WAKE_UP_DUR 0x5CU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t sleep_dur : 4;
    uint8_t timer_hr : 1;
    uint8_t wake_dur : 2;
    uint8_t ff_dur : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t ff_dur : 1;
    uint8_t wake_dur : 2;
    uint8_t timer_hr : 1;
    uint8_t sleep_dur : 4;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_wake_up_dur_t;

#define LSM6DS3TR_C_FREE_FALL 0x5DU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t ff_ths : 3;
    uint8_t ff_dur : 5;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t ff_dur : 5;
    uint8_t ff_ths : 3;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_free_fall_t;

#define LSM6DS3TR_C_MD1_CFG 0x5EU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t int1_timer : 1;
    uint8_t int1_tilt : 1;
    uint8_t int1_6d : 1;
    uint8_t int1_double_tap : 1;
    uint8_t int1_ff : 1;
    uint8_t int1_wu : 1;
    uint8_t int1_single_tap : 1;
    uint8_t int1_inact_state : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t int1_inact_state : 1;
    uint8_t int1_single_tap : 1;
    uint8_t int1_wu : 1;
    uint8_t int1_ff : 1;
    uint8_t int1_double_tap : 1;
    uint8_t int1_6d : 1;
    uint8_t int1_tilt : 1;
    uint8_t int1_timer : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_md1_cfg_t;

#define LSM6DS3TR_C_MD2_CFG 0x5FU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t int2_iron : 1;
    uint8_t int2_tilt : 1;
    uint8_t int2_6d : 1;
    uint8_t int2_double_tap : 1;
    uint8_t int2_ff : 1;
    uint8_t int2_wu : 1;
    uint8_t int2_single_tap : 1;
    uint8_t int2_inact_state : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t int2_inact_state : 1;
    uint8_t int2_single_tap : 1;
    uint8_t int2_wu : 1;
    uint8_t int2_ff : 1;
    uint8_t int2_double_tap : 1;
    uint8_t int2_6d : 1;
    uint8_t int2_tilt : 1;
    uint8_t int2_iron : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_md2_cfg_t;

#define LSM6DS3TR_C_MASTER_CMD_CODE 0x60U
typedef struct {
    uint8_t master_cmd_code : 8;
} lsm6ds3tr_c_master_cmd_code_t;

#define LSM6DS3TR_C_SENS_SYNC_SPI_ERROR_CODE 0x61U
typedef struct {
    uint8_t error_code : 8;
} lsm6ds3tr_c_sens_sync_spi_error_code_t;

#define LSM6DS3TR_C_OUT_MAG_RAW_X_L 0x66U
#define LSM6DS3TR_C_OUT_MAG_RAW_X_H 0x67U
#define LSM6DS3TR_C_OUT_MAG_RAW_Y_L 0x68U
#define LSM6DS3TR_C_OUT_MAG_RAW_Y_H 0x69U
#define LSM6DS3TR_C_OUT_MAG_RAW_Z_L 0x6AU
#define LSM6DS3TR_C_OUT_MAG_RAW_Z_H 0x6BU
#define LSM6DS3TR_C_X_OFS_USR 0x73U
#define LSM6DS3TR_C_Y_OFS_USR 0x74U
#define LSM6DS3TR_C_Z_OFS_USR 0x75U
#define LSM6DS3TR_C_SLV0_ADD 0x02U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t rw_0 : 1;
    uint8_t slave0_add : 7;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t slave0_add : 7;
    uint8_t rw_0 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_slv0_add_t;

#define LSM6DS3TR_C_SLV0_SUBADD 0x03U
typedef struct {
    uint8_t slave0_reg : 8;
} lsm6ds3tr_c_slv0_subadd_t;

#define LSM6DS3TR_C_SLAVE0_CONFIG 0x04U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t slave0_numop : 3;
    uint8_t src_mode : 1;
    uint8_t aux_sens_on : 2;
    uint8_t slave0_rate : 2;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t slave0_rate : 2;
    uint8_t aux_sens_on : 2;
    uint8_t src_mode : 1;
    uint8_t slave0_numop : 3;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_slave0_config_t;

#define LSM6DS3TR_C_SLV1_ADD 0x05U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t r_1 : 1;
    uint8_t slave1_add : 7;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t slave1_add : 7;
    uint8_t r_1 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_slv1_add_t;

#define LSM6DS3TR_C_SLV1_SUBADD 0x06U
typedef struct {
    uint8_t slave1_reg : 8;
} lsm6ds3tr_c_slv1_subadd_t;

#define LSM6DS3TR_C_SLAVE1_CONFIG 0x07U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t slave1_numop : 3;
    uint8_t not_used_01 : 2;
    uint8_t write_once : 1;
    uint8_t slave1_rate : 2;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t slave1_rate : 2;
    uint8_t write_once : 1;
    uint8_t not_used_01 : 2;
    uint8_t slave1_numop : 3;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_slave1_config_t;

#define LSM6DS3TR_C_SLV2_ADD 0x08U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t r_2 : 1;
    uint8_t slave2_add : 7;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t slave2_add : 7;
    uint8_t r_2 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_slv2_add_t;

#define LSM6DS3TR_C_SLV2_SUBADD 0x09U
typedef struct {
    uint8_t slave2_reg : 8;
} lsm6ds3tr_c_slv2_subadd_t;

#define LSM6DS3TR_C_SLAVE2_CONFIG 0x0AU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t slave2_numop : 3;
    uint8_t not_used_01 : 3;
    uint8_t slave2_rate : 2;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t slave2_rate : 2;
    uint8_t not_used_01 : 3;
    uint8_t slave2_numop : 3;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_slave2_config_t;

#define LSM6DS3TR_C_SLV3_ADD 0x0BU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t r_3 : 1;
    uint8_t slave3_add : 7;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t slave3_add : 7;
    uint8_t r_3 : 1;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_slv3_add_t;

#define LSM6DS3TR_C_SLV3_SUBADD 0x0CU
typedef struct {
    uint8_t slave3_reg : 8;
} lsm6ds3tr_c_slv3_subadd_t;

#define LSM6DS3TR_C_SLAVE3_CONFIG 0x0DU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t slave3_numop : 3;
    uint8_t not_used_01 : 3;
    uint8_t slave3_rate : 2;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t slave3_rate : 2;
    uint8_t not_used_01 : 3;
    uint8_t slave3_numop : 3;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_slave3_config_t;

#define LSM6DS3TR_C_DATAWRITE_SRC_MODE_SUB_SLV0 0x0EU
typedef struct {
    uint8_t slave_dataw : 8;
} lsm6ds3tr_c_datawrite_src_mode_sub_slv0_t;

#define LSM6DS3TR_C_CONFIG_PEDO_THS_MIN 0x0FU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t ths_min : 5;
    uint8_t not_used_01 : 2;
    uint8_t pedo_fs : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t pedo_fs : 1;
    uint8_t not_used_01 : 2;
    uint8_t ths_min : 5;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_config_pedo_ths_min_t;

#define LSM6DS3TR_C_SM_THS 0x13U
#define LSM6DS3TR_C_PEDO_DEB_REG 0x14U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t deb_step : 3;
    uint8_t deb_time : 5;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t deb_time : 5;
    uint8_t deb_step : 3;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_pedo_deb_reg_t;

#define LSM6DS3TR_C_STEP_COUNT_DELTA 0x15U
#define LSM6DS3TR_C_MAG_SI_XX 0x24U
#define LSM6DS3TR_C_MAG_SI_XY 0x25U
#define LSM6DS3TR_C_MAG_SI_XZ 0x26U
#define LSM6DS3TR_C_MAG_SI_YX 0x27U
#define LSM6DS3TR_C_MAG_SI_YY 0x28U
#define LSM6DS3TR_C_MAG_SI_YZ 0x29U
#define LSM6DS3TR_C_MAG_SI_ZX 0x2AU
#define LSM6DS3TR_C_MAG_SI_ZY 0x2BU
#define LSM6DS3TR_C_MAG_SI_ZZ 0x2CU
#define LSM6DS3TR_C_MAG_OFFX_L 0x2DU
#define LSM6DS3TR_C_MAG_OFFX_H 0x2EU
#define LSM6DS3TR_C_MAG_OFFY_L 0x2FU
#define LSM6DS3TR_C_MAG_OFFY_H 0x30U
#define LSM6DS3TR_C_MAG_OFFZ_L 0x31U
#define LSM6DS3TR_C_MAG_OFFZ_H 0x32U
#define LSM6DS3TR_C_A_WRIST_TILT_LAT 0x50U
#define LSM6DS3TR_C_A_WRIST_TILT_THS 0x54U
#define LSM6DS3TR_C_A_WRIST_TILT_MASK 0x59U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t not_used_01 : 2;
    uint8_t wrist_tilt_mask_zneg : 1;
    uint8_t wrist_tilt_mask_zpos : 1;
    uint8_t wrist_tilt_mask_yneg : 1;
    uint8_t wrist_tilt_mask_ypos : 1;
    uint8_t wrist_tilt_mask_xneg : 1;
    uint8_t wrist_tilt_mask_xpos : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t wrist_tilt_mask_xpos : 1;
    uint8_t wrist_tilt_mask_xneg : 1;
    uint8_t wrist_tilt_mask_ypos : 1;
    uint8_t wrist_tilt_mask_yneg : 1;
    uint8_t wrist_tilt_mask_zpos : 1;
    uint8_t wrist_tilt_mask_zneg : 1;
    uint8_t not_used_01 : 2;
#endif /* DRV_BYTE_ORDER */
} lsm6ds3tr_c_a_wrist_tilt_mask_t;

/**
  * @defgroup LSM6DS3TR_C_Register_Union
  * @brief    This union group all the registers having a bit-field
  *           description.
  *           This union is useful but it's not needed by the driver.
  *
  *           REMOVING this union you are compliant with:
  *           MISRA-C 2012 [Rule 19.2] -> " Union are not allowed "
  *
  * @{
  *
  */
typedef union {
    lsm6ds3tr_c_func_cfg_access_t func_cfg_access;
    lsm6ds3tr_c_sensor_sync_time_frame_t sensor_sync_time_frame;
    lsm6ds3tr_c_sensor_sync_res_ratio_t sensor_sync_res_ratio;
    lsm6ds3tr_c_fifo_ctrl1_t fifo_ctrl1;
    lsm6ds3tr_c_fifo_ctrl2_t fifo_ctrl2;
    lsm6ds3tr_c_fifo_ctrl3_t fifo_ctrl3;
    lsm6ds3tr_c_fifo_ctrl4_t fifo_ctrl4;
    lsm6ds3tr_c_fifo_ctrl5_t fifo_ctrl5;
    lsm6ds3tr_c_drdy_pulse_cfg_g_t drdy_pulse_cfg_g;
    lsm6ds3tr_c_int1_ctrl_t int1_ctrl;
    lsm6ds3tr_c_int2_ctrl_t int2_ctrl;
    lsm6ds3tr_c_ctrl1_xl_t ctrl1_xl;
    lsm6ds3tr_c_ctrl2_g_t ctrl2_g;
    lsm6ds3tr_c_ctrl3_c_t ctrl3_c;
    lsm6ds3tr_c_ctrl4_c_t ctrl4_c;
    lsm6ds3tr_c_ctrl5_c_t ctrl5_c;
    lsm6ds3tr_c_ctrl6_c_t ctrl6_c;
    lsm6ds3tr_c_ctrl7_g_t ctrl7_g;
    lsm6ds3tr_c_ctrl8_xl_t ctrl8_xl;
    lsm6ds3tr_c_ctrl9_xl_t ctrl9_xl;
    lsm6ds3tr_c_ctrl10_c_t ctrl10_c;
    lsm6ds3tr_c_master_config_t master_config;
    lsm6ds3tr_c_wake_up_src_t wake_up_src;
    lsm6ds3tr_c_tap_src_t tap_src;
    lsm6ds3tr_c_d6d_src_t d6d_src;
    lsm6ds3tr_c_status_reg_t status_reg;
    lsm6ds3tr_c_sensorhub1_reg_t sensorhub1_reg;
    lsm6ds3tr_c_sensorhub2_reg_t sensorhub2_reg;
    lsm6ds3tr_c_sensorhub3_reg_t sensorhub3_reg;
    lsm6ds3tr_c_sensorhub4_reg_t sensorhub4_reg;
    lsm6ds3tr_c_sensorhub5_reg_t sensorhub5_reg;
    lsm6ds3tr_c_sensorhub6_reg_t sensorhub6_reg;
    lsm6ds3tr_c_sensorhub7_reg_t sensorhub7_reg;
    lsm6ds3tr_c_sensorhub8_reg_t sensorhub8_reg;
    lsm6ds3tr_c_sensorhub9_reg_t sensorhub9_reg;
    lsm6ds3tr_c_sensorhub10_reg_t sensorhub10_reg;
    lsm6ds3tr_c_sensorhub11_reg_t sensorhub11_reg;
    lsm6ds3tr_c_sensorhub12_reg_t sensorhub12_reg;
    lsm6ds3tr_c_fifo_status1_t fifo_status1;
    lsm6ds3tr_c_fifo_status2_t fifo_status2;
    lsm6ds3tr_c_fifo_status3_t fifo_status3;
    lsm6ds3tr_c_fifo_status4_t fifo_status4;
    lsm6ds3tr_c_sensorhub13_reg_t sensorhub13_reg;
    lsm6ds3tr_c_sensorhub14_reg_t sensorhub14_reg;
    lsm6ds3tr_c_sensorhub15_reg_t sensorhub15_reg;
    lsm6ds3tr_c_sensorhub16_reg_t sensorhub16_reg;
    lsm6ds3tr_c_sensorhub17_reg_t sensorhub17_reg;
    lsm6ds3tr_c_sensorhub18_reg_t sensorhub18_reg;
    lsm6ds3tr_c_func_src1_t func_src1;
    lsm6ds3tr_c_func_src2_t func_src2;
    lsm6ds3tr_c_wrist_tilt_ia_t wrist_tilt_ia;
    lsm6ds3tr_c_tap_cfg_t tap_cfg;
    lsm6ds3tr_c_tap_ths_6d_t tap_ths_6d;
    lsm6ds3tr_c_int_dur2_t int_dur2;
    lsm6ds3tr_c_wake_up_ths_t wake_up_ths;
    lsm6ds3tr_c_wake_up_dur_t wake_up_dur;
    lsm6ds3tr_c_free_fall_t free_fall;
    lsm6ds3tr_c_md1_cfg_t md1_cfg;
    lsm6ds3tr_c_md2_cfg_t md2_cfg;
    lsm6ds3tr_c_master_cmd_code_t master_cmd_code;
    lsm6ds3tr_c_sens_sync_spi_error_code_t sens_sync_spi_error_code;
    lsm6ds3tr_c_slv0_add_t slv0_add;
    lsm6ds3tr_c_slv0_subadd_t slv0_subadd;
    lsm6ds3tr_c_slave0_config_t slave0_config;
    lsm6ds3tr_c_slv1_add_t slv1_add;
    lsm6ds3tr_c_slv1_subadd_t slv1_subadd;
    lsm6ds3tr_c_slave1_config_t slave1_config;
    lsm6ds3tr_c_slv2_add_t slv2_add;
    lsm6ds3tr_c_slv2_subadd_t slv2_subadd;
    lsm6ds3tr_c_slave2_config_t slave2_config;
    lsm6ds3tr_c_slv3_add_t slv3_add;
    lsm6ds3tr_c_slv3_subadd_t slv3_subadd;
    lsm6ds3tr_c_slave3_config_t slave3_config;
    lsm6ds3tr_c_datawrite_src_mode_sub_slv0_t datawrite_src_mode_sub_slv0;
    lsm6ds3tr_c_config_pedo_ths_min_t config_pedo_ths_min;
    lsm6ds3tr_c_pedo_deb_reg_t pedo_deb_reg;
    lsm6ds3tr_c_a_wrist_tilt_mask_t a_wrist_tilt_mask;
    bitwise_t bitwise;
    uint8_t byte;
} lsm6ds3tr_c_reg_t;

/**
  * @}
  *
  */

int32_t lsm6ds3tr_c_read_reg(stmdev_ctx_t* ctx, uint8_t reg, uint8_t* data, uint16_t len);
int32_t lsm6ds3tr_c_write_reg(stmdev_ctx_t* ctx, uint8_t reg, uint8_t* data, uint16_t len);

float_t lsm6ds3tr_c_from_fs2g_to_mg(int16_t lsb);
float_t lsm6ds3tr_c_from_fs4g_to_mg(int16_t lsb);
float_t lsm6ds3tr_c_from_fs8g_to_mg(int16_t lsb);
float_t lsm6ds3tr_c_from_fs16g_to_mg(int16_t lsb);

float_t lsm6ds3tr_c_from_fs125dps_to_mdps(int16_t lsb);
float_t lsm6ds3tr_c_from_fs250dps_to_mdps(int16_t lsb);
float_t lsm6ds3tr_c_from_fs500dps_to_mdps(int16_t lsb);
float_t lsm6ds3tr_c_from_fs1000dps_to_mdps(int16_t lsb);
float_t lsm6ds3tr_c_from_fs2000dps_to_mdps(int16_t lsb);

float_t lsm6ds3tr_c_from_lsb_to_celsius(int16_t lsb);

typedef enum {
    LSM6DS3TR_C_2g = 0,
    LSM6DS3TR_C_16g = 1,
    LSM6DS3TR_C_4g = 2,
    LSM6DS3TR_C_8g = 3,
    LSM6DS3TR_C_XL_FS_ND = 4, /* ERROR CODE */
} lsm6ds3tr_c_fs_xl_t;
int32_t lsm6ds3tr_c_xl_full_scale_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_fs_xl_t val);
int32_t lsm6ds3tr_c_xl_full_scale_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_fs_xl_t* val);

typedef enum {
    LSM6DS3TR_C_XL_ODR_OFF = 0,
    LSM6DS3TR_C_XL_ODR_12Hz5 = 1,
    LSM6DS3TR_C_XL_ODR_26Hz = 2,
    LSM6DS3TR_C_XL_ODR_52Hz = 3,
    LSM6DS3TR_C_XL_ODR_104Hz = 4,
    LSM6DS3TR_C_XL_ODR_208Hz = 5,
    LSM6DS3TR_C_XL_ODR_416Hz = 6,
    LSM6DS3TR_C_XL_ODR_833Hz = 7,
    LSM6DS3TR_C_XL_ODR_1k66Hz = 8,
    LSM6DS3TR_C_XL_ODR_3k33Hz = 9,
    LSM6DS3TR_C_XL_ODR_6k66Hz = 10,
    LSM6DS3TR_C_XL_ODR_1Hz6 = 11,
    LSM6DS3TR_C_XL_ODR_ND = 12, /* ERROR CODE */
} lsm6ds3tr_c_odr_xl_t;
int32_t lsm6ds3tr_c_xl_data_rate_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_odr_xl_t val);
int32_t lsm6ds3tr_c_xl_data_rate_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_odr_xl_t* val);

typedef enum {
    LSM6DS3TR_C_250dps = 0,
    LSM6DS3TR_C_125dps = 1,
    LSM6DS3TR_C_500dps = 2,
    LSM6DS3TR_C_1000dps = 4,
    LSM6DS3TR_C_2000dps = 6,
    LSM6DS3TR_C_GY_FS_ND = 7, /* ERROR CODE */
} lsm6ds3tr_c_fs_g_t;
int32_t lsm6ds3tr_c_gy_full_scale_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_fs_g_t val);
int32_t lsm6ds3tr_c_gy_full_scale_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_fs_g_t* val);

typedef enum {
    LSM6DS3TR_C_GY_ODR_OFF = 0,
    LSM6DS3TR_C_GY_ODR_12Hz5 = 1,
    LSM6DS3TR_C_GY_ODR_26Hz = 2,
    LSM6DS3TR_C_GY_ODR_52Hz = 3,
    LSM6DS3TR_C_GY_ODR_104Hz = 4,
    LSM6DS3TR_C_GY_ODR_208Hz = 5,
    LSM6DS3TR_C_GY_ODR_416Hz = 6,
    LSM6DS3TR_C_GY_ODR_833Hz = 7,
    LSM6DS3TR_C_GY_ODR_1k66Hz = 8,
    LSM6DS3TR_C_GY_ODR_3k33Hz = 9,
    LSM6DS3TR_C_GY_ODR_6k66Hz = 10,
    LSM6DS3TR_C_GY_ODR_ND = 11, /* ERROR CODE */
} lsm6ds3tr_c_odr_g_t;
int32_t lsm6ds3tr_c_gy_data_rate_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_odr_g_t val);
int32_t lsm6ds3tr_c_gy_data_rate_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_odr_g_t* val);

int32_t lsm6ds3tr_c_block_data_update_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_block_data_update_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_LSb_1mg = 0,
    LSM6DS3TR_C_LSb_16mg = 1,
    LSM6DS3TR_C_WEIGHT_ND = 2,
} lsm6ds3tr_c_usr_off_w_t;
int32_t lsm6ds3tr_c_xl_offset_weight_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_usr_off_w_t val);
int32_t lsm6ds3tr_c_xl_offset_weight_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_usr_off_w_t* val);

typedef enum {
    LSM6DS3TR_C_XL_HIGH_PERFORMANCE = 0,
    LSM6DS3TR_C_XL_NORMAL = 1,
    LSM6DS3TR_C_XL_PW_MODE_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_xl_hm_mode_t;
int32_t lsm6ds3tr_c_xl_power_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_xl_hm_mode_t val);
int32_t lsm6ds3tr_c_xl_power_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_xl_hm_mode_t* val);

typedef enum {
    LSM6DS3TR_C_STAT_RND_DISABLE = 0,
    LSM6DS3TR_C_STAT_RND_ENABLE = 1,
    LSM6DS3TR_C_STAT_RND_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_rounding_status_t;
int32_t lsm6ds3tr_c_rounding_on_status_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_rounding_status_t val);
int32_t lsm6ds3tr_c_rounding_on_status_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_rounding_status_t* val);

typedef enum {
    LSM6DS3TR_C_GY_HIGH_PERFORMANCE = 0,
    LSM6DS3TR_C_GY_NORMAL = 1,
    LSM6DS3TR_C_GY_PW_MODE_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_g_hm_mode_t;
int32_t lsm6ds3tr_c_gy_power_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_g_hm_mode_t val);
int32_t lsm6ds3tr_c_gy_power_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_g_hm_mode_t* val);

typedef struct {
    lsm6ds3tr_c_wake_up_src_t wake_up_src;
    lsm6ds3tr_c_tap_src_t tap_src;
    lsm6ds3tr_c_d6d_src_t d6d_src;
    lsm6ds3tr_c_status_reg_t status_reg;
    lsm6ds3tr_c_func_src1_t func_src1;
    lsm6ds3tr_c_func_src2_t func_src2;
    lsm6ds3tr_c_wrist_tilt_ia_t wrist_tilt_ia;
    lsm6ds3tr_c_a_wrist_tilt_mask_t a_wrist_tilt_mask;
} lsm6ds3tr_c_all_sources_t;
int32_t lsm6ds3tr_c_all_sources_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_all_sources_t* val);

int32_t lsm6ds3tr_c_status_reg_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_status_reg_t* val);

int32_t lsm6ds3tr_c_xl_flag_data_ready_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_gy_flag_data_ready_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_temp_flag_data_ready_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_xl_usr_offset_set(stmdev_ctx_t* ctx, uint8_t* buff);
int32_t lsm6ds3tr_c_xl_usr_offset_get(stmdev_ctx_t* ctx, uint8_t* buff);
int32_t lsm6ds3tr_c_timestamp_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_timestamp_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_LSB_6ms4 = 0,
    LSM6DS3TR_C_LSB_25us = 1,
    LSM6DS3TR_C_TS_RES_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_timer_hr_t;
int32_t lsm6ds3tr_c_timestamp_res_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_timer_hr_t val);
int32_t lsm6ds3tr_c_timestamp_res_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_timer_hr_t* val);

typedef enum {
    LSM6DS3TR_C_ROUND_DISABLE = 0,
    LSM6DS3TR_C_ROUND_XL = 1,
    LSM6DS3TR_C_ROUND_GY = 2,
    LSM6DS3TR_C_ROUND_GY_XL = 3,
    LSM6DS3TR_C_ROUND_SH1_TO_SH6 = 4,
    LSM6DS3TR_C_ROUND_XL_SH1_TO_SH6 = 5,
    LSM6DS3TR_C_ROUND_GY_XL_SH1_TO_SH12 = 6,
    LSM6DS3TR_C_ROUND_GY_XL_SH1_TO_SH6 = 7,
    LSM6DS3TR_C_ROUND_OUT_ND = 8, /* ERROR CODE */
} lsm6ds3tr_c_rounding_t;
int32_t lsm6ds3tr_c_rounding_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_rounding_t val);
int32_t lsm6ds3tr_c_rounding_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_rounding_t* val);

int32_t lsm6ds3tr_c_temperature_raw_get(stmdev_ctx_t* ctx, int16_t* val);
int32_t lsm6ds3tr_c_angular_rate_raw_get(stmdev_ctx_t* ctx, int16_t* val);
int32_t lsm6ds3tr_c_acceleration_raw_get(stmdev_ctx_t* ctx, int16_t* val);

int32_t lsm6ds3tr_c_mag_calibrated_raw_get(stmdev_ctx_t* ctx, int16_t* val);

int32_t lsm6ds3tr_c_fifo_raw_data_get(stmdev_ctx_t* ctx, uint8_t* buffer, uint8_t len);

typedef enum {
    LSM6DS3TR_C_USER_BANK = 0,
    LSM6DS3TR_C_BANK_A = 4,
    LSM6DS3TR_C_BANK_B = 5,
    LSM6DS3TR_C_BANK_ND = 6, /* ERROR CODE */
} lsm6ds3tr_c_func_cfg_en_t;
int32_t lsm6ds3tr_c_mem_bank_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_func_cfg_en_t val);
int32_t lsm6ds3tr_c_mem_bank_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_func_cfg_en_t* val);

typedef enum {
    LSM6DS3TR_C_DRDY_LATCHED = 0,
    LSM6DS3TR_C_DRDY_PULSED = 1,
    LSM6DS3TR_C_DRDY_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_drdy_pulsed_g_t;
int32_t lsm6ds3tr_c_data_ready_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_drdy_pulsed_g_t val);
int32_t lsm6ds3tr_c_data_ready_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_drdy_pulsed_g_t* val);

int32_t lsm6ds3tr_c_device_id_get(stmdev_ctx_t* ctx, uint8_t* buff);
int32_t lsm6ds3tr_c_reset_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_reset_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_LSB_AT_LOW_ADD = 0,
    LSM6DS3TR_C_MSB_AT_LOW_ADD = 1,
    LSM6DS3TR_C_DATA_FMT_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_ble_t;
int32_t lsm6ds3tr_c_data_format_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_ble_t val);
int32_t lsm6ds3tr_c_data_format_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_ble_t* val);

int32_t lsm6ds3tr_c_auto_increment_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_auto_increment_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_boot_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_boot_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_XL_ST_DISABLE = 0,
    LSM6DS3TR_C_XL_ST_POSITIVE = 1,
    LSM6DS3TR_C_XL_ST_NEGATIVE = 2,
    LSM6DS3TR_C_XL_ST_ND = 3, /* ERROR CODE */
} lsm6ds3tr_c_st_xl_t;
int32_t lsm6ds3tr_c_xl_self_test_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_st_xl_t val);
int32_t lsm6ds3tr_c_xl_self_test_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_st_xl_t* val);

typedef enum {
    LSM6DS3TR_C_GY_ST_DISABLE = 0,
    LSM6DS3TR_C_GY_ST_POSITIVE = 1,
    LSM6DS3TR_C_GY_ST_NEGATIVE = 3,
    LSM6DS3TR_C_GY_ST_ND = 4, /* ERROR CODE */
} lsm6ds3tr_c_st_g_t;
int32_t lsm6ds3tr_c_gy_self_test_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_st_g_t val);
int32_t lsm6ds3tr_c_gy_self_test_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_st_g_t* val);

int32_t lsm6ds3tr_c_filter_settling_mask_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_filter_settling_mask_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_USE_SLOPE = 0,
    LSM6DS3TR_C_USE_HPF = 1,
    LSM6DS3TR_C_HP_PATH_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_slope_fds_t;
int32_t lsm6ds3tr_c_xl_hp_path_internal_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_slope_fds_t val);
int32_t lsm6ds3tr_c_xl_hp_path_internal_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_slope_fds_t* val);

typedef enum {
    LSM6DS3TR_C_XL_ANA_BW_1k5Hz = 0,
    LSM6DS3TR_C_XL_ANA_BW_400Hz = 1,
    LSM6DS3TR_C_XL_ANA_BW_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_bw0_xl_t;
int32_t lsm6ds3tr_c_xl_filter_analog_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_bw0_xl_t val);
int32_t lsm6ds3tr_c_xl_filter_analog_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_bw0_xl_t* val);

typedef enum {
    LSM6DS3TR_C_XL_LP1_ODR_DIV_2 = 0,
    LSM6DS3TR_C_XL_LP1_ODR_DIV_4 = 1,
    LSM6DS3TR_C_XL_LP1_NA = 2, /* ERROR CODE */
} lsm6ds3tr_c_lpf1_bw_sel_t;
int32_t lsm6ds3tr_c_xl_lp1_bandwidth_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_lpf1_bw_sel_t val);
int32_t lsm6ds3tr_c_xl_lp1_bandwidth_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_lpf1_bw_sel_t* val);

typedef enum {
    LSM6DS3TR_C_XL_LOW_LAT_LP_ODR_DIV_50 = 0x00,
    LSM6DS3TR_C_XL_LOW_LAT_LP_ODR_DIV_100 = 0x01,
    LSM6DS3TR_C_XL_LOW_LAT_LP_ODR_DIV_9 = 0x02,
    LSM6DS3TR_C_XL_LOW_LAT_LP_ODR_DIV_400 = 0x03,
    LSM6DS3TR_C_XL_LOW_NOISE_LP_ODR_DIV_50 = 0x10,
    LSM6DS3TR_C_XL_LOW_NOISE_LP_ODR_DIV_100 = 0x11,
    LSM6DS3TR_C_XL_LOW_NOISE_LP_ODR_DIV_9 = 0x12,
    LSM6DS3TR_C_XL_LOW_NOISE_LP_ODR_DIV_400 = 0x13,
    LSM6DS3TR_C_XL_LP_NA = 0x20, /* ERROR CODE */
} lsm6ds3tr_c_input_composite_t;
int32_t lsm6ds3tr_c_xl_lp2_bandwidth_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_input_composite_t val);
int32_t lsm6ds3tr_c_xl_lp2_bandwidth_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_input_composite_t* val);

int32_t lsm6ds3tr_c_xl_reference_mode_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_xl_reference_mode_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_XL_HP_ODR_DIV_4 = 0x00, /* Slope filter */
    LSM6DS3TR_C_XL_HP_ODR_DIV_100 = 0x01,
    LSM6DS3TR_C_XL_HP_ODR_DIV_9 = 0x02,
    LSM6DS3TR_C_XL_HP_ODR_DIV_400 = 0x03,
    LSM6DS3TR_C_XL_HP_NA = 0x10, /* ERROR CODE */
} lsm6ds3tr_c_hpcf_xl_t;
int32_t lsm6ds3tr_c_xl_hp_bandwidth_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_hpcf_xl_t val);
int32_t lsm6ds3tr_c_xl_hp_bandwidth_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_hpcf_xl_t* val);

typedef enum {
    LSM6DS3TR_C_LP2_ONLY = 0x00,

    LSM6DS3TR_C_HP_16mHz_LP2 = 0x80,
    LSM6DS3TR_C_HP_65mHz_LP2 = 0x90,
    LSM6DS3TR_C_HP_260mHz_LP2 = 0xA0,
    LSM6DS3TR_C_HP_1Hz04_LP2 = 0xB0,

    LSM6DS3TR_C_HP_DISABLE_LP1_LIGHT = 0x0A,
    LSM6DS3TR_C_HP_DISABLE_LP1_NORMAL = 0x09,
    LSM6DS3TR_C_HP_DISABLE_LP_STRONG = 0x08,
    LSM6DS3TR_C_HP_DISABLE_LP1_AGGRESSIVE = 0x0B,

    LSM6DS3TR_C_HP_16mHz_LP1_LIGHT = 0x8A,
    LSM6DS3TR_C_HP_65mHz_LP1_NORMAL = 0x99,
    LSM6DS3TR_C_HP_260mHz_LP1_STRONG = 0xA8,
    LSM6DS3TR_C_HP_1Hz04_LP1_AGGRESSIVE = 0xBB,

    LSM6DS3TR_C_HP_GY_BAND_NA = 0xFF, /* ERROR CODE */
} lsm6ds3tr_c_lpf1_sel_g_t;
int32_t lsm6ds3tr_c_gy_band_pass_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_lpf1_sel_g_t val);
int32_t lsm6ds3tr_c_gy_band_pass_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_lpf1_sel_g_t* val);

typedef enum {
    LSM6DS3TR_C_SPI_4_WIRE = 0,
    LSM6DS3TR_C_SPI_3_WIRE = 1,
    LSM6DS3TR_C_SPI_MODE_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_sim_t;
int32_t lsm6ds3tr_c_spi_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_sim_t val);
int32_t lsm6ds3tr_c_spi_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_sim_t* val);

typedef enum {
    LSM6DS3TR_C_I2C_ENABLE = 0,
    LSM6DS3TR_C_I2C_DISABLE = 1,
    LSM6DS3TR_C_I2C_MODE_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_i2c_disable_t;
int32_t lsm6ds3tr_c_i2c_interface_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_i2c_disable_t val);
int32_t lsm6ds3tr_c_i2c_interface_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_i2c_disable_t* val);

typedef struct {
    uint8_t int1_drdy_xl : 1;
    uint8_t int1_drdy_g : 1;
    uint8_t int1_boot : 1;
    uint8_t int1_fth : 1;
    uint8_t int1_fifo_ovr : 1;
    uint8_t int1_full_flag : 1;
    uint8_t int1_sign_mot : 1;
    uint8_t int1_step_detector : 1;
    uint8_t int1_timer : 1;
    uint8_t int1_tilt : 1;
    uint8_t int1_6d : 1;
    uint8_t int1_double_tap : 1;
    uint8_t int1_ff : 1;
    uint8_t int1_wu : 1;
    uint8_t int1_single_tap : 1;
    uint8_t int1_inact_state : 1;
    uint8_t den_drdy_int1 : 1;
    uint8_t drdy_on_int1 : 1;
} lsm6ds3tr_c_int1_route_t;
int32_t lsm6ds3tr_c_pin_int1_route_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_int1_route_t val);
int32_t lsm6ds3tr_c_pin_int1_route_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_int1_route_t* val);

typedef struct {
    uint8_t int2_drdy_xl : 1;
    uint8_t int2_drdy_g : 1;
    uint8_t int2_drdy_temp : 1;
    uint8_t int2_fth : 1;
    uint8_t int2_fifo_ovr : 1;
    uint8_t int2_full_flag : 1;
    uint8_t int2_step_count_ov : 1;
    uint8_t int2_step_delta : 1;
    uint8_t int2_iron : 1;
    uint8_t int2_tilt : 1;
    uint8_t int2_6d : 1;
    uint8_t int2_double_tap : 1;
    uint8_t int2_ff : 1;
    uint8_t int2_wu : 1;
    uint8_t int2_single_tap : 1;
    uint8_t int2_inact_state : 1;
    uint8_t int2_wrist_tilt : 1;
} lsm6ds3tr_c_int2_route_t;
int32_t lsm6ds3tr_c_pin_int2_route_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_int2_route_t val);
int32_t lsm6ds3tr_c_pin_int2_route_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_int2_route_t* val);

typedef enum {
    LSM6DS3TR_C_PUSH_PULL = 0,
    LSM6DS3TR_C_OPEN_DRAIN = 1,
    LSM6DS3TR_C_PIN_MODE_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_pp_od_t;
int32_t lsm6ds3tr_c_pin_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_pp_od_t val);
int32_t lsm6ds3tr_c_pin_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_pp_od_t* val);

typedef enum {
    LSM6DS3TR_C_ACTIVE_HIGH = 0,
    LSM6DS3TR_C_ACTIVE_LOW = 1,
    LSM6DS3TR_C_POLARITY_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_h_lactive_t;
int32_t lsm6ds3tr_c_pin_polarity_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_h_lactive_t val);
int32_t lsm6ds3tr_c_pin_polarity_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_h_lactive_t* val);

int32_t lsm6ds3tr_c_all_on_int1_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_all_on_int1_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_INT_PULSED = 0,
    LSM6DS3TR_C_INT_LATCHED = 1,
    LSM6DS3TR_C_INT_MODE = 2, /* ERROR CODE */
} lsm6ds3tr_c_lir_t;
int32_t lsm6ds3tr_c_int_notification_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_lir_t val);
int32_t lsm6ds3tr_c_int_notification_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_lir_t* val);

int32_t lsm6ds3tr_c_wkup_threshold_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_wkup_threshold_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_wkup_dur_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_wkup_dur_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_gy_sleep_mode_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_gy_sleep_mode_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_PROPERTY_DISABLE = 0,
    LSM6DS3TR_C_XL_12Hz5_GY_NOT_AFFECTED = 1,
    LSM6DS3TR_C_XL_12Hz5_GY_SLEEP = 2,
    LSM6DS3TR_C_XL_12Hz5_GY_PD = 3,
    LSM6DS3TR_C_ACT_MODE_ND = 4, /* ERROR CODE */
} lsm6ds3tr_c_inact_en_t;
int32_t lsm6ds3tr_c_act_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_inact_en_t val);
int32_t lsm6ds3tr_c_act_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_inact_en_t* val);

int32_t lsm6ds3tr_c_act_sleep_dur_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_act_sleep_dur_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_tap_src_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_tap_src_t* val);

int32_t lsm6ds3tr_c_tap_detection_on_z_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_tap_detection_on_z_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_tap_detection_on_y_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_tap_detection_on_y_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_tap_detection_on_x_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_tap_detection_on_x_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_tap_threshold_x_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_tap_threshold_x_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_tap_shock_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_tap_shock_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_tap_quiet_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_tap_quiet_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_tap_dur_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_tap_dur_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_ONLY_SINGLE = 0,
    LSM6DS3TR_C_BOTH_SINGLE_DOUBLE = 1,
    LSM6DS3TR_C_TAP_MODE_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_single_double_tap_t;
int32_t lsm6ds3tr_c_tap_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_single_double_tap_t val);
int32_t lsm6ds3tr_c_tap_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_single_double_tap_t* val);

typedef enum {
    LSM6DS3TR_C_ODR_DIV_2_FEED = 0,
    LSM6DS3TR_C_LPF2_FEED = 1,
    LSM6DS3TR_C_6D_FEED_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_low_pass_on_6d_t;
int32_t lsm6ds3tr_c_6d_feed_data_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_low_pass_on_6d_t val);
int32_t lsm6ds3tr_c_6d_feed_data_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_low_pass_on_6d_t* val);

typedef enum {
    LSM6DS3TR_C_DEG_80 = 0,
    LSM6DS3TR_C_DEG_70 = 1,
    LSM6DS3TR_C_DEG_60 = 2,
    LSM6DS3TR_C_DEG_50 = 3,
    LSM6DS3TR_C_6D_TH_ND = 4, /* ERROR CODE */
} lsm6ds3tr_c_sixd_ths_t;
int32_t lsm6ds3tr_c_6d_threshold_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_sixd_ths_t val);
int32_t lsm6ds3tr_c_6d_threshold_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_sixd_ths_t* val);

int32_t lsm6ds3tr_c_4d_mode_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_4d_mode_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_ff_dur_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_ff_dur_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_FF_TSH_156mg = 0,
    LSM6DS3TR_C_FF_TSH_219mg = 1,
    LSM6DS3TR_C_FF_TSH_250mg = 2,
    LSM6DS3TR_C_FF_TSH_312mg = 3,
    LSM6DS3TR_C_FF_TSH_344mg = 4,
    LSM6DS3TR_C_FF_TSH_406mg = 5,
    LSM6DS3TR_C_FF_TSH_469mg = 6,
    LSM6DS3TR_C_FF_TSH_500mg = 7,
    LSM6DS3TR_C_FF_TSH_ND = 8, /* ERROR CODE */
} lsm6ds3tr_c_ff_ths_t;
int32_t lsm6ds3tr_c_ff_threshold_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_ff_ths_t val);
int32_t lsm6ds3tr_c_ff_threshold_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_ff_ths_t* val);

int32_t lsm6ds3tr_c_fifo_watermark_set(stmdev_ctx_t* ctx, uint16_t val);
int32_t lsm6ds3tr_c_fifo_watermark_get(stmdev_ctx_t* ctx, uint16_t* val);

int32_t lsm6ds3tr_c_fifo_data_level_get(stmdev_ctx_t* ctx, uint16_t* val);

int32_t lsm6ds3tr_c_fifo_wtm_flag_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_fifo_pattern_get(stmdev_ctx_t* ctx, uint16_t* val);

int32_t lsm6ds3tr_c_fifo_temp_batch_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_fifo_temp_batch_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_TRG_XL_GY_DRDY = 0,
    LSM6DS3TR_C_TRG_STEP_DETECT = 1,
    LSM6DS3TR_C_TRG_SH_DRDY = 2,
    LSM6DS3TR_C_TRG_SH_ND = 3, /* ERROR CODE */
} lsm6ds3tr_c_trigger_fifo_t;
int32_t lsm6ds3tr_c_fifo_write_trigger_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_trigger_fifo_t val);
int32_t lsm6ds3tr_c_fifo_write_trigger_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_trigger_fifo_t* val);

int32_t lsm6ds3tr_c_fifo_pedo_and_timestamp_batch_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_fifo_pedo_and_timestamp_batch_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_FIFO_XL_DISABLE = 0,
    LSM6DS3TR_C_FIFO_XL_NO_DEC = 1,
    LSM6DS3TR_C_FIFO_XL_DEC_2 = 2,
    LSM6DS3TR_C_FIFO_XL_DEC_3 = 3,
    LSM6DS3TR_C_FIFO_XL_DEC_4 = 4,
    LSM6DS3TR_C_FIFO_XL_DEC_8 = 5,
    LSM6DS3TR_C_FIFO_XL_DEC_16 = 6,
    LSM6DS3TR_C_FIFO_XL_DEC_32 = 7,
    LSM6DS3TR_C_FIFO_XL_DEC_ND = 8, /* ERROR CODE */
} lsm6ds3tr_c_dec_fifo_xl_t;
int32_t lsm6ds3tr_c_fifo_xl_batch_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_dec_fifo_xl_t val);
int32_t lsm6ds3tr_c_fifo_xl_batch_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_dec_fifo_xl_t* val);

typedef enum {
    LSM6DS3TR_C_FIFO_GY_DISABLE = 0,
    LSM6DS3TR_C_FIFO_GY_NO_DEC = 1,
    LSM6DS3TR_C_FIFO_GY_DEC_2 = 2,
    LSM6DS3TR_C_FIFO_GY_DEC_3 = 3,
    LSM6DS3TR_C_FIFO_GY_DEC_4 = 4,
    LSM6DS3TR_C_FIFO_GY_DEC_8 = 5,
    LSM6DS3TR_C_FIFO_GY_DEC_16 = 6,
    LSM6DS3TR_C_FIFO_GY_DEC_32 = 7,
    LSM6DS3TR_C_FIFO_GY_DEC_ND = 8, /* ERROR CODE */
} lsm6ds3tr_c_dec_fifo_gyro_t;
int32_t lsm6ds3tr_c_fifo_gy_batch_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_dec_fifo_gyro_t val);
int32_t lsm6ds3tr_c_fifo_gy_batch_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_dec_fifo_gyro_t* val);

typedef enum {
    LSM6DS3TR_C_FIFO_DS3_DISABLE = 0,
    LSM6DS3TR_C_FIFO_DS3_NO_DEC = 1,
    LSM6DS3TR_C_FIFO_DS3_DEC_2 = 2,
    LSM6DS3TR_C_FIFO_DS3_DEC_3 = 3,
    LSM6DS3TR_C_FIFO_DS3_DEC_4 = 4,
    LSM6DS3TR_C_FIFO_DS3_DEC_8 = 5,
    LSM6DS3TR_C_FIFO_DS3_DEC_16 = 6,
    LSM6DS3TR_C_FIFO_DS3_DEC_32 = 7,
    LSM6DS3TR_C_FIFO_DS3_DEC_ND = 8, /* ERROR CODE */
} lsm6ds3tr_c_dec_ds3_fifo_t;
int32_t lsm6ds3tr_c_fifo_dataset_3_batch_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_dec_ds3_fifo_t val);
int32_t lsm6ds3tr_c_fifo_dataset_3_batch_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_dec_ds3_fifo_t* val);

typedef enum {
    LSM6DS3TR_C_FIFO_DS4_DISABLE = 0,
    LSM6DS3TR_C_FIFO_DS4_NO_DEC = 1,
    LSM6DS3TR_C_FIFO_DS4_DEC_2 = 2,
    LSM6DS3TR_C_FIFO_DS4_DEC_3 = 3,
    LSM6DS3TR_C_FIFO_DS4_DEC_4 = 4,
    LSM6DS3TR_C_FIFO_DS4_DEC_8 = 5,
    LSM6DS3TR_C_FIFO_DS4_DEC_16 = 6,
    LSM6DS3TR_C_FIFO_DS4_DEC_32 = 7,
    LSM6DS3TR_C_FIFO_DS4_DEC_ND = 8, /* ERROR CODE */
} lsm6ds3tr_c_dec_ds4_fifo_t;
int32_t lsm6ds3tr_c_fifo_dataset_4_batch_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_dec_ds4_fifo_t val);
int32_t lsm6ds3tr_c_fifo_dataset_4_batch_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_dec_ds4_fifo_t* val);

int32_t lsm6ds3tr_c_fifo_xl_gy_8bit_format_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_fifo_xl_gy_8bit_format_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_fifo_stop_on_wtm_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_fifo_stop_on_wtm_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_BYPASS_MODE = 0,
    LSM6DS3TR_C_FIFO_MODE = 1,
    LSM6DS3TR_C_STREAM_TO_FIFO_MODE = 3,
    LSM6DS3TR_C_BYPASS_TO_STREAM_MODE = 4,
    LSM6DS3TR_C_STREAM_MODE = 6,
    LSM6DS3TR_C_FIFO_MODE_ND = 8, /* ERROR CODE */
} lsm6ds3tr_c_fifo_mode_t;
int32_t lsm6ds3tr_c_fifo_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_fifo_mode_t val);
int32_t lsm6ds3tr_c_fifo_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_fifo_mode_t* val);

typedef enum {
    LSM6DS3TR_C_FIFO_DISABLE = 0,
    LSM6DS3TR_C_FIFO_12Hz5 = 1,
    LSM6DS3TR_C_FIFO_26Hz = 2,
    LSM6DS3TR_C_FIFO_52Hz = 3,
    LSM6DS3TR_C_FIFO_104Hz = 4,
    LSM6DS3TR_C_FIFO_208Hz = 5,
    LSM6DS3TR_C_FIFO_416Hz = 6,
    LSM6DS3TR_C_FIFO_833Hz = 7,
    LSM6DS3TR_C_FIFO_1k66Hz = 8,
    LSM6DS3TR_C_FIFO_3k33Hz = 9,
    LSM6DS3TR_C_FIFO_6k66Hz = 10,
    LSM6DS3TR_C_FIFO_RATE_ND = 11, /* ERROR CODE */
} lsm6ds3tr_c_odr_fifo_t;
int32_t lsm6ds3tr_c_fifo_data_rate_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_odr_fifo_t val);
int32_t lsm6ds3tr_c_fifo_data_rate_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_odr_fifo_t* val);

typedef enum {
    LSM6DS3TR_C_DEN_ACT_LOW = 0,
    LSM6DS3TR_C_DEN_ACT_HIGH = 1,
    LSM6DS3TR_C_DEN_POL_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_den_lh_t;
int32_t lsm6ds3tr_c_den_polarity_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_den_lh_t val);
int32_t lsm6ds3tr_c_den_polarity_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_den_lh_t* val);

typedef enum {
    LSM6DS3TR_C_DEN_DISABLE = 0,
    LSM6DS3TR_C_LEVEL_FIFO = 6,
    LSM6DS3TR_C_LEVEL_LETCHED = 3,
    LSM6DS3TR_C_LEVEL_TRIGGER = 2,
    LSM6DS3TR_C_EDGE_TRIGGER = 4,
    LSM6DS3TR_C_DEN_MODE_ND = 5, /* ERROR CODE */
} lsm6ds3tr_c_den_mode_t;
int32_t lsm6ds3tr_c_den_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_den_mode_t val);
int32_t lsm6ds3tr_c_den_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_den_mode_t* val);

typedef enum {
    LSM6DS3TR_C_STAMP_IN_GY_DATA = 0,
    LSM6DS3TR_C_STAMP_IN_XL_DATA = 1,
    LSM6DS3TR_C_STAMP_IN_GY_XL_DATA = 2,
    LSM6DS3TR_C_DEN_STAMP_ND = 3, /* ERROR CODE */
} lsm6ds3tr_c_den_xl_en_t;
int32_t lsm6ds3tr_c_den_enable_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_den_xl_en_t val);
int32_t lsm6ds3tr_c_den_enable_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_den_xl_en_t* val);

int32_t lsm6ds3tr_c_den_mark_axis_z_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_den_mark_axis_z_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_den_mark_axis_y_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_den_mark_axis_y_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_den_mark_axis_x_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_den_mark_axis_x_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_pedo_step_reset_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_pedo_step_reset_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_pedo_sens_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_pedo_sens_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_pedo_threshold_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_pedo_threshold_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_PEDO_AT_2g = 0,
    LSM6DS3TR_C_PEDO_AT_4g = 1,
    LSM6DS3TR_C_PEDO_FS_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_pedo_fs_t;
int32_t lsm6ds3tr_c_pedo_full_scale_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_pedo_fs_t val);
int32_t lsm6ds3tr_c_pedo_full_scale_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_pedo_fs_t* val);

int32_t lsm6ds3tr_c_pedo_debounce_steps_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_pedo_debounce_steps_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_pedo_timeout_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_pedo_timeout_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_pedo_steps_period_set(stmdev_ctx_t* ctx, uint8_t* buff);
int32_t lsm6ds3tr_c_pedo_steps_period_get(stmdev_ctx_t* ctx, uint8_t* buff);

int32_t lsm6ds3tr_c_motion_sens_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_motion_sens_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_motion_threshold_set(stmdev_ctx_t* ctx, uint8_t* buff);
int32_t lsm6ds3tr_c_motion_threshold_get(stmdev_ctx_t* ctx, uint8_t* buff);

int32_t lsm6ds3tr_c_tilt_sens_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_tilt_sens_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_wrist_tilt_sens_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_wrist_tilt_sens_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_tilt_latency_set(stmdev_ctx_t* ctx, uint8_t* buff);
int32_t lsm6ds3tr_c_tilt_latency_get(stmdev_ctx_t* ctx, uint8_t* buff);

int32_t lsm6ds3tr_c_tilt_threshold_set(stmdev_ctx_t* ctx, uint8_t* buff);
int32_t lsm6ds3tr_c_tilt_threshold_get(stmdev_ctx_t* ctx, uint8_t* buff);

int32_t lsm6ds3tr_c_tilt_src_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_a_wrist_tilt_mask_t* val);
int32_t lsm6ds3tr_c_tilt_src_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_a_wrist_tilt_mask_t* val);

int32_t lsm6ds3tr_c_mag_soft_iron_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_mag_soft_iron_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_mag_hard_iron_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_mag_hard_iron_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_mag_soft_iron_mat_set(stmdev_ctx_t* ctx, uint8_t* buff);
int32_t lsm6ds3tr_c_mag_soft_iron_mat_get(stmdev_ctx_t* ctx, uint8_t* buff);

int32_t lsm6ds3tr_c_mag_offset_set(stmdev_ctx_t* ctx, int16_t* val);
int32_t lsm6ds3tr_c_mag_offset_get(stmdev_ctx_t* ctx, int16_t* val);

int32_t lsm6ds3tr_c_func_en_set(stmdev_ctx_t* ctx, uint8_t val);

int32_t lsm6ds3tr_c_sh_sync_sens_frame_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_sh_sync_sens_frame_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_RES_RATIO_2_11 = 0,
    LSM6DS3TR_C_RES_RATIO_2_12 = 1,
    LSM6DS3TR_C_RES_RATIO_2_13 = 2,
    LSM6DS3TR_C_RES_RATIO_2_14 = 3,
    LSM6DS3TR_C_RES_RATIO_ND = 4, /* ERROR CODE */
} lsm6ds3tr_c_rr_t;
int32_t lsm6ds3tr_c_sh_sync_sens_ratio_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_rr_t val);
int32_t lsm6ds3tr_c_sh_sync_sens_ratio_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_rr_t* val);

int32_t lsm6ds3tr_c_sh_master_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_sh_master_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_sh_pass_through_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_sh_pass_through_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_EXT_PULL_UP = 0,
    LSM6DS3TR_C_INTERNAL_PULL_UP = 1,
    LSM6DS3TR_C_SH_PIN_MODE = 2, /* ERROR CODE */
} lsm6ds3tr_c_pull_up_en_t;
int32_t lsm6ds3tr_c_sh_pin_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_pull_up_en_t val);
int32_t lsm6ds3tr_c_sh_pin_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_pull_up_en_t* val);

typedef enum {
    LSM6DS3TR_C_XL_GY_DRDY = 0,
    LSM6DS3TR_C_EXT_ON_INT2_PIN = 1,
    LSM6DS3TR_C_SH_SYNCRO_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_start_config_t;
int32_t lsm6ds3tr_c_sh_syncro_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_start_config_t val);
int32_t lsm6ds3tr_c_sh_syncro_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_start_config_t* val);

int32_t lsm6ds3tr_c_sh_drdy_on_int1_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_sh_drdy_on_int1_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef struct {
    lsm6ds3tr_c_sensorhub1_reg_t sh_byte_1;
    lsm6ds3tr_c_sensorhub2_reg_t sh_byte_2;
    lsm6ds3tr_c_sensorhub3_reg_t sh_byte_3;
    lsm6ds3tr_c_sensorhub4_reg_t sh_byte_4;
    lsm6ds3tr_c_sensorhub5_reg_t sh_byte_5;
    lsm6ds3tr_c_sensorhub6_reg_t sh_byte_6;
    lsm6ds3tr_c_sensorhub7_reg_t sh_byte_7;
    lsm6ds3tr_c_sensorhub8_reg_t sh_byte_8;
    lsm6ds3tr_c_sensorhub9_reg_t sh_byte_9;
    lsm6ds3tr_c_sensorhub10_reg_t sh_byte_10;
    lsm6ds3tr_c_sensorhub11_reg_t sh_byte_11;
    lsm6ds3tr_c_sensorhub12_reg_t sh_byte_12;
    lsm6ds3tr_c_sensorhub13_reg_t sh_byte_13;
    lsm6ds3tr_c_sensorhub14_reg_t sh_byte_14;
    lsm6ds3tr_c_sensorhub15_reg_t sh_byte_15;
    lsm6ds3tr_c_sensorhub16_reg_t sh_byte_16;
    lsm6ds3tr_c_sensorhub17_reg_t sh_byte_17;
    lsm6ds3tr_c_sensorhub18_reg_t sh_byte_18;
} lsm6ds3tr_c_emb_sh_read_t;
int32_t lsm6ds3tr_c_sh_read_data_raw_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_emb_sh_read_t* val);

int32_t lsm6ds3tr_c_sh_cmd_sens_sync_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_sh_cmd_sens_sync_get(stmdev_ctx_t* ctx, uint8_t* val);

int32_t lsm6ds3tr_c_sh_spi_sync_error_set(stmdev_ctx_t* ctx, uint8_t val);
int32_t lsm6ds3tr_c_sh_spi_sync_error_get(stmdev_ctx_t* ctx, uint8_t* val);

typedef enum {
    LSM6DS3TR_C_SLV_0 = 0,
    LSM6DS3TR_C_SLV_0_1 = 1,
    LSM6DS3TR_C_SLV_0_1_2 = 2,
    LSM6DS3TR_C_SLV_0_1_2_3 = 3,
    LSM6DS3TR_C_SLV_EN_ND = 4, /* ERROR CODE */
} lsm6ds3tr_c_aux_sens_on_t;
int32_t lsm6ds3tr_c_sh_num_of_dev_connected_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_aux_sens_on_t val);
int32_t lsm6ds3tr_c_sh_num_of_dev_connected_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_aux_sens_on_t* val);

typedef struct {
    uint8_t slv0_add;
    uint8_t slv0_subadd;
    uint8_t slv0_data;
} lsm6ds3tr_c_sh_cfg_write_t;
int32_t lsm6ds3tr_c_sh_cfg_write(stmdev_ctx_t* ctx, lsm6ds3tr_c_sh_cfg_write_t* val);

typedef struct {
    uint8_t slv_add;
    uint8_t slv_subadd;
    uint8_t slv_len;
} lsm6ds3tr_c_sh_cfg_read_t;
int32_t lsm6ds3tr_c_sh_slv0_cfg_read(stmdev_ctx_t* ctx, lsm6ds3tr_c_sh_cfg_read_t* val);
int32_t lsm6ds3tr_c_sh_slv1_cfg_read(stmdev_ctx_t* ctx, lsm6ds3tr_c_sh_cfg_read_t* val);
int32_t lsm6ds3tr_c_sh_slv2_cfg_read(stmdev_ctx_t* ctx, lsm6ds3tr_c_sh_cfg_read_t* val);
int32_t lsm6ds3tr_c_sh_slv3_cfg_read(stmdev_ctx_t* ctx, lsm6ds3tr_c_sh_cfg_read_t* val);

typedef enum {
    LSM6DS3TR_C_SL0_NO_DEC = 0,
    LSM6DS3TR_C_SL0_DEC_2 = 1,
    LSM6DS3TR_C_SL0_DEC_4 = 2,
    LSM6DS3TR_C_SL0_DEC_8 = 3,
    LSM6DS3TR_C_SL0_DEC_ND = 4, /* ERROR CODE */
} lsm6ds3tr_c_slave0_rate_t;
int32_t lsm6ds3tr_c_sh_slave_0_dec_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_slave0_rate_t val);
int32_t lsm6ds3tr_c_sh_slave_0_dec_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_slave0_rate_t* val);

typedef enum {
    LSM6DS3TR_C_EACH_SH_CYCLE = 0,
    LSM6DS3TR_C_ONLY_FIRST_CYCLE = 1,
    LSM6DS3TR_C_SH_WR_MODE_ND = 2, /* ERROR CODE */
} lsm6ds3tr_c_write_once_t;
int32_t lsm6ds3tr_c_sh_write_mode_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_write_once_t val);
int32_t lsm6ds3tr_c_sh_write_mode_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_write_once_t* val);

typedef enum {
    LSM6DS3TR_C_SL1_NO_DEC = 0,
    LSM6DS3TR_C_SL1_DEC_2 = 1,
    LSM6DS3TR_C_SL1_DEC_4 = 2,
    LSM6DS3TR_C_SL1_DEC_8 = 3,
    LSM6DS3TR_C_SL1_DEC_ND = 4, /* ERROR CODE */
} lsm6ds3tr_c_slave1_rate_t;
int32_t lsm6ds3tr_c_sh_slave_1_dec_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_slave1_rate_t val);
int32_t lsm6ds3tr_c_sh_slave_1_dec_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_slave1_rate_t* val);

typedef enum {
    LSM6DS3TR_C_SL2_NO_DEC = 0,
    LSM6DS3TR_C_SL2_DEC_2 = 1,
    LSM6DS3TR_C_SL2_DEC_4 = 2,
    LSM6DS3TR_C_SL2_DEC_8 = 3,
    LSM6DS3TR_C_SL2_DEC_ND = 4, /* ERROR CODE */
} lsm6ds3tr_c_slave2_rate_t;
int32_t lsm6ds3tr_c_sh_slave_2_dec_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_slave2_rate_t val);
int32_t lsm6ds3tr_c_sh_slave_2_dec_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_slave2_rate_t* val);

typedef enum {
    LSM6DS3TR_C_SL3_NO_DEC = 0,
    LSM6DS3TR_C_SL3_DEC_2 = 1,
    LSM6DS3TR_C_SL3_DEC_4 = 2,
    LSM6DS3TR_C_SL3_DEC_8 = 3,
    LSM6DS3TR_C_SL3_DEC_ND = 4, /* ERROR CODE */
} lsm6ds3tr_c_slave3_rate_t;
int32_t lsm6ds3tr_c_sh_slave_3_dec_set(stmdev_ctx_t* ctx, lsm6ds3tr_c_slave3_rate_t val);
int32_t lsm6ds3tr_c_sh_slave_3_dec_get(stmdev_ctx_t* ctx, lsm6ds3tr_c_slave3_rate_t* val);

/**
  * @}
  *
  */

#ifdef __cplusplus
}
#endif

#endif /* LSM6DS3TR_C_DRIVER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
