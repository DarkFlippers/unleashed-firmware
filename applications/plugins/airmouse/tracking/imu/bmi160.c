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
* @file       bmi160.c
* @date       2021-10-05
* @version    v3.9.2
*
*/

#include "bmi160.h"

/* Below look up table follows the enum bmi160_int_types.
 * Hence any change should match to the enum bmi160_int_types
 */
const uint8_t int_mask_lookup_table[13] = {
    BMI160_INT1_SLOPE_MASK,
    BMI160_INT1_SLOPE_MASK,
    BMI160_INT2_LOW_STEP_DETECT_MASK,
    BMI160_INT1_DOUBLE_TAP_MASK,
    BMI160_INT1_SINGLE_TAP_MASK,
    BMI160_INT1_ORIENT_MASK,
    BMI160_INT1_FLAT_MASK,
    BMI160_INT1_HIGH_G_MASK,
    BMI160_INT1_LOW_G_MASK,
    BMI160_INT1_NO_MOTION_MASK,
    BMI160_INT2_DATA_READY_MASK,
    BMI160_INT2_FIFO_FULL_MASK,
    BMI160_INT2_FIFO_WM_MASK};

/*********************************************************************/
/* Static function declarations */

/*!
 * @brief This API configures the pins to fire the
 * interrupt signal when it occurs
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t
    set_intr_pin_config(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This API sets the any-motion interrupt of the sensor.
 * This interrupt occurs when accel values exceeds preset threshold
 * for a certain period of time.
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t
    set_accel_any_motion_int(struct bmi160_int_settg* int_config, struct bmi160_dev* dev);

/*!
 * @brief This API sets tap interrupts.Interrupt is fired when
 * tap movements happen.
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t set_accel_tap_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This API sets the data ready interrupt for both accel and gyro.
 * This interrupt occurs when new accel and gyro data come.
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t set_accel_gyro_data_ready_int(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_dev* dev);

/*!
 * @brief This API sets the significant motion interrupt of the sensor.This
 * interrupt occurs when there is change in user location.
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t
    set_accel_sig_motion_int(struct bmi160_int_settg* int_config, struct bmi160_dev* dev);

/*!
 * @brief This API sets the no motion/slow motion interrupt of the sensor.
 * Slow motion is similar to any motion interrupt.No motion interrupt
 * occurs when slope bet. two accel values falls below preset threshold
 * for preset duration.
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t
    set_accel_no_motion_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This API sets the step detection interrupt.This interrupt
 * occurs when the single step causes accel values to go above
 * preset threshold.
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t
    set_accel_step_detect_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This API sets the orientation interrupt of the sensor.This
 * interrupt occurs when there is orientation change in the sensor
 * with respect to gravitational field vector g.
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t
    set_accel_orientation_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This API sets the flat interrupt of the sensor.This interrupt
 * occurs in case of flat orientation
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t
    set_accel_flat_detect_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This API sets the low-g interrupt of the sensor.This interrupt
 * occurs during free-fall.
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t
    set_accel_low_g_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This API sets the high-g interrupt of the sensor.The interrupt
 * occurs if the absolute value of acceleration data of any enabled axis
 * exceeds the programmed threshold and the sign of the value does not
 * change for a preset duration.
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t
    set_accel_high_g_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This API sets the default configuration parameters of accel & gyro.
 * Also maintain the previous state of configurations.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static void default_param_settg(struct bmi160_dev* dev);

/*!
 * @brief This API is used to validate the device structure pointer for
 * null conditions.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t null_ptr_check(const struct bmi160_dev* dev);

/*!
 * @brief This API set the accel configuration.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t set_accel_conf(struct bmi160_dev* dev);

/*!
 * @brief This API gets the accel configuration.
 *
 * @param[out] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t get_accel_conf(struct bmi160_dev* dev);

/*!
 * @brief This API check the accel configuration.
 *
 * @param[in] data        : Pointer to store the updated accel config.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t check_accel_config(uint8_t* data, const struct bmi160_dev* dev);

/*!
 * @brief This API process the accel odr.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t process_accel_odr(uint8_t* data, const struct bmi160_dev* dev);

/*!
 * @brief This API process the accel bandwidth.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t process_accel_bw(uint8_t* data, const struct bmi160_dev* dev);

/*!
 * @brief This API process the accel range.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t process_accel_range(uint8_t* data, const struct bmi160_dev* dev);

/*!
 * @brief This API checks the invalid settings for ODR & Bw for Accel and Gyro.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t check_invalid_settg(const struct bmi160_dev* dev);

/*!
 * @brief This API set the gyro configuration.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t set_gyro_conf(struct bmi160_dev* dev);

/*!
 * @brief This API get the gyro configuration.
 *
 * @param[out] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t get_gyro_conf(struct bmi160_dev* dev);

/*!
 * @brief This API check the gyro configuration.
 *
 * @param[in] data        : Pointer to store the updated gyro config.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t check_gyro_config(uint8_t* data, const struct bmi160_dev* dev);

/*!
 * @brief This API process the gyro odr.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t process_gyro_odr(uint8_t* data, const struct bmi160_dev* dev);

/*!
 * @brief This API process the gyro bandwidth.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t process_gyro_bw(uint8_t* data, const struct bmi160_dev* dev);

/*!
 * @brief This API process the gyro range.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t process_gyro_range(uint8_t* data, const struct bmi160_dev* dev);

/*!
 * @brief This API sets the accel power mode.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t set_accel_pwr(struct bmi160_dev* dev);

/*!
 * @brief This API process the undersampling setting of Accel.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t process_under_sampling(uint8_t* data, const struct bmi160_dev* dev);

/*!
 * @brief This API sets the gyro power mode.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error.
 */
static int8_t set_gyro_pwr(struct bmi160_dev* dev);

/*!
 * @brief This API reads accel data along with sensor time if time is requested
 * by user. Kindly refer the user guide(README.md) for more info.
 *
 * @param[in] len    : len to read no of bytes
 * @param[out] accel    : Structure pointer to store accel data
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t
    get_accel_data(uint8_t len, struct bmi160_sensor_data* accel, const struct bmi160_dev* dev);

/*!
 * @brief This API reads accel data along with sensor time if time is requested
 * by user. Kindly refer the user guide(README.md) for more info.
 *
 * @param[in] len    : len to read no of bytes
 * @param[out] gyro    : Structure pointer to store accel data
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t
    get_gyro_data(uint8_t len, struct bmi160_sensor_data* gyro, const struct bmi160_dev* dev);

/*!
 * @brief This API reads accel and gyro data along with sensor time
 * if time is requested by user.
 * Kindly refer the user guide(README.md) for more info.
 *
 * @param[in] len    : len to read no of bytes
 * @param[out] accel    : Structure pointer to store accel data
 * @param[out] gyro    : Structure pointer to store accel data
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t get_accel_gyro_data(
    uint8_t len,
    struct bmi160_sensor_data* accel,
    struct bmi160_sensor_data* gyro,
    const struct bmi160_dev* dev);

/*!
 * @brief This API enables the any-motion interrupt for accel.
 *
 * @param[in] any_motion_int_cfg   : Structure instance of
 *                   bmi160_acc_any_mot_int_cfg.
 * @param[in] dev          : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_accel_any_motion_int(
    const struct bmi160_acc_any_mot_int_cfg* any_motion_int_cfg,
    struct bmi160_dev* dev);

/*!
 * @brief This API disable the sig-motion interrupt.
 *
 * @param[in] dev   : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t disable_sig_motion_int(const struct bmi160_dev* dev);

/*!
 * @brief This API configure the source of data(filter & pre-filter)
 * for any-motion interrupt.
 *
 * @param[in] any_motion_int_cfg  : Structure instance of
 *                  bmi160_acc_any_mot_int_cfg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_any_motion_src(
    const struct bmi160_acc_any_mot_int_cfg* any_motion_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the duration and threshold of
 * any-motion interrupt.
 *
 * @param[in] any_motion_int_cfg  : Structure instance of
 *                  bmi160_acc_any_mot_int_cfg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_any_dur_threshold(
    const struct bmi160_acc_any_mot_int_cfg* any_motion_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure necessary setting of any-motion interrupt.
 *
 * @param[in] int_config       : Structure instance of bmi160_int_settg.
 * @param[in] any_motion_int_cfg   : Structure instance of
 *                   bmi160_acc_any_mot_int_cfg.
 * @param[in] dev          : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_any_motion_int_settg(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_any_mot_int_cfg* any_motion_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API enable the data ready interrupt.
 *
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_data_ready_int(const struct bmi160_dev* dev);

/*!
 * @brief This API enables the no motion/slow motion interrupt.
 *
 * @param[in] no_mot_int_cfg    : Structure instance of
 *                bmi160_acc_no_motion_int_cfg.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_no_motion_int(
    const struct bmi160_acc_no_motion_int_cfg* no_mot_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the interrupt PIN setting for
 * no motion/slow motion interrupt.
 *
 * @param[in] int_config    : structure instance of bmi160_int_settg.
 * @param[in] no_mot_int_cfg    : Structure instance of
 *                bmi160_acc_no_motion_int_cfg.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_no_motion_int_settg(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_no_motion_int_cfg* no_mot_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the source of interrupt for no motion.
 *
 * @param[in] no_mot_int_cfg    : Structure instance of
 *                bmi160_acc_no_motion_int_cfg.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_no_motion_data_src(
    const struct bmi160_acc_no_motion_int_cfg* no_mot_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the duration and threshold of
 * no motion/slow motion interrupt along with selection of no/slow motion.
 *
 * @param[in] no_mot_int_cfg    : Structure instance of
 *                bmi160_acc_no_motion_int_cfg.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_no_motion_dur_thr(
    const struct bmi160_acc_no_motion_int_cfg* no_mot_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API enables the sig-motion motion interrupt.
 *
 * @param[in] sig_mot_int_cfg   : Structure instance of
 *                bmi160_acc_sig_mot_int_cfg.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_sig_motion_int(
    const struct bmi160_acc_sig_mot_int_cfg* sig_mot_int_cfg,
    struct bmi160_dev* dev);

/*!
 * @brief This API configure the interrupt PIN setting for
 * significant motion interrupt.
 *
 * @param[in] int_config    : Structure instance of bmi160_int_settg.
 * @param[in] sig_mot_int_cfg   : Structure instance of
 *                bmi160_acc_sig_mot_int_cfg.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_sig_motion_int_settg(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_sig_mot_int_cfg* sig_mot_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the source of data(filter & pre-filter)
 * for sig motion interrupt.
 *
 * @param[in] sig_mot_int_cfg   : Structure instance of
 *                bmi160_acc_sig_mot_int_cfg.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_sig_motion_data_src(
    const struct bmi160_acc_sig_mot_int_cfg* sig_mot_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the threshold, skip and proof time of
 * sig motion interrupt.
 *
 * @param[in] sig_mot_int_cfg   : Structure instance of
 *                bmi160_acc_sig_mot_int_cfg.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_sig_dur_threshold(
    const struct bmi160_acc_sig_mot_int_cfg* sig_mot_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API enables the step detector interrupt.
 *
 * @param[in] step_detect_int_cfg   : Structure instance of
 *                    bmi160_acc_step_detect_int_cfg.
 * @param[in] dev           : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_step_detect_int(
    const struct bmi160_acc_step_detect_int_cfg* step_detect_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the step detector parameter.
 *
 * @param[in] step_detect_int_cfg   : Structure instance of
 *                    bmi160_acc_step_detect_int_cfg.
 * @param[in] dev           : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_step_detect(
    const struct bmi160_acc_step_detect_int_cfg* step_detect_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API enables the single/double tap interrupt.
 *
 * @param[in] int_config    : Structure instance of bmi160_int_settg.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_tap_int(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_tap_int_cfg* tap_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the interrupt PIN setting for
 * tap interrupt.
 *
 * @param[in] int_config    : Structure instance of bmi160_int_settg.
 * @param[in] tap_int_cfg   : Structure instance of bmi160_acc_tap_int_cfg.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_tap_int_settg(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_tap_int_cfg* tap_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the source of data(filter & pre-filter)
 * for tap interrupt.
 *
 * @param[in] tap_int_cfg   : Structure instance of bmi160_acc_tap_int_cfg.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_tap_data_src(
    const struct bmi160_acc_tap_int_cfg* tap_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the  parameters of tap interrupt.
 * Threshold, quite, shock, and duration.
 *
 * @param[in] int_config    : Structure instance of bmi160_int_settg.
 * @param[in] tap_int_cfg   : Structure instance of bmi160_acc_tap_int_cfg.
 * @param[in] dev       : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_tap_param(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_tap_int_cfg* tap_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API enable the external mode configuration.
 *
 * @param[in] dev   : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_sec_if(const struct bmi160_dev* dev);

/*!
 * @brief This API configure the ODR of the auxiliary sensor.
 *
 * @param[in] dev   : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_aux_odr(const struct bmi160_dev* dev);

/*!
 * @brief This API maps the actual burst read length set by user.
 *
 * @param[in] len   : Pointer to store the read length.
 * @param[in] dev   : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t map_read_len(uint16_t* len, const struct bmi160_dev* dev);

/*!
 * @brief This API configure the settings of auxiliary sensor.
 *
 * @param[in] dev   : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_aux_settg(const struct bmi160_dev* dev);

/*!
 * @brief This API extract the read data from auxiliary sensor.
 *
 * @param[in] map_len     : burst read value.
 * @param[in] reg_addr    : Address of register to read.
 * @param[in] aux_data    : Pointer to store the read data.
 * @param[in] len     : length to read the data.
 * @param[in] dev         : Structure instance of bmi160_dev.
 * @note : Refer user guide for detailed info.
 *
 * @return Result of API execution status
 * @retval zero -> Success / -ve value -> Error
 */
static int8_t extract_aux_read(
    uint16_t map_len,
    uint8_t reg_addr,
    uint8_t* aux_data,
    uint16_t len,
    const struct bmi160_dev* dev);

/*!
 * @brief This API enables the orient interrupt.
 *
 * @param[in] orient_int_cfg : Structure instance of bmi160_acc_orient_int_cfg.
 * @param[in] dev        : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_orient_int(
    const struct bmi160_acc_orient_int_cfg* orient_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the necessary setting of orientation interrupt.
 *
 * @param[in] orient_int_cfg : Structure instance of bmi160_acc_orient_int_cfg.
 * @param[in] dev        : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_orient_int_settg(
    const struct bmi160_acc_orient_int_cfg* orient_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API enables the flat interrupt.
 *
 * @param[in] flat_int  : Structure instance of bmi160_acc_flat_detect_int_cfg.
 * @param[in] dev       : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_flat_int(
    const struct bmi160_acc_flat_detect_int_cfg* flat_int,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the necessary setting of flat interrupt.
 *
 * @param[in] flat_int  : Structure instance of bmi160_acc_flat_detect_int_cfg.
 * @param[in] dev   : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_flat_int_settg(
    const struct bmi160_acc_flat_detect_int_cfg* flat_int,
    const struct bmi160_dev* dev);

/*!
 * @brief This API enables the Low-g interrupt.
 *
 * @param[in] low_g_int : Structure instance of bmi160_acc_low_g_int_cfg.
 * @param[in] dev   : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_low_g_int(
    const struct bmi160_acc_low_g_int_cfg* low_g_int,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the source of data(filter & pre-filter) for low-g interrupt.
 *
 * @param[in] low_g_int : Structure instance of bmi160_acc_low_g_int_cfg.
 * @param[in] dev   : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_low_g_data_src(
    const struct bmi160_acc_low_g_int_cfg* low_g_int,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the necessary setting of low-g interrupt.
 *
 * @param[in] low_g_int : Structure instance of bmi160_acc_low_g_int_cfg.
 * @param[in] dev   : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_low_g_int_settg(
    const struct bmi160_acc_low_g_int_cfg* low_g_int,
    const struct bmi160_dev* dev);

/*!
 * @brief This API enables the high-g interrupt.
 *
 * @param[in] high_g_int_cfg : Structure instance of bmi160_acc_high_g_int_cfg.
 * @param[in] dev        : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_high_g_int(
    const struct bmi160_acc_high_g_int_cfg* high_g_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the source of data(filter & pre-filter)
 * for high-g interrupt.
 *
 * @param[in] high_g_int_cfg : Structure instance of bmi160_acc_high_g_int_cfg.
 * @param[in] dev        : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_high_g_data_src(
    const struct bmi160_acc_high_g_int_cfg* high_g_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the necessary setting of high-g interrupt.
 *
 * @param[in] high_g_int_cfg : Structure instance of bmi160_acc_high_g_int_cfg.
 * @param[in] dev        : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t config_high_g_int_settg(
    const struct bmi160_acc_high_g_int_cfg* high_g_int_cfg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API configure the behavioural setting of interrupt pin.
 *
 * @param[in] int_config    : Structure instance of bmi160_int_settg.
 * @param[in] dev       : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t
    config_int_out_ctrl(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This API configure the mode(input enable, latch or non-latch) of interrupt pin.
 *
 * @param[in] int_config    : Structure instance of bmi160_int_settg.
 * @param[in] dev       : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t
    config_int_latch(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This API performs the self test for accelerometer of BMI160
 *
 * @param[in] dev   : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t perform_accel_self_test(struct bmi160_dev* dev);

/*!
 * @brief This API enables to perform the accel self test by setting proper
 * configurations to facilitate accel self test
 *
 * @param[in] dev   : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_accel_self_test(struct bmi160_dev* dev);

/*!
 * @brief This API performs accel self test with positive excitation
 *
 * @param[in] accel_pos : Structure pointer to store accel data
 *                        for positive excitation
 * @param[in] dev   : structure instance of bmi160_dev
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t accel_self_test_positive_excitation(
    struct bmi160_sensor_data* accel_pos,
    const struct bmi160_dev* dev);

/*!
 * @brief This API performs accel self test with negative excitation
 *
 * @param[in] accel_neg : Structure pointer to store accel data
 *                        for negative excitation
 * @param[in] dev   : structure instance of bmi160_dev
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t accel_self_test_negative_excitation(
    struct bmi160_sensor_data* accel_neg,
    const struct bmi160_dev* dev);

/*!
 * @brief This API validates the accel self test results
 *
 * @param[in] accel_pos : Structure pointer to store accel data
 *                        for positive excitation
 * @param[in] accel_neg : Structure pointer to store accel data
 *                        for negative excitation
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error / +ve value -> Self test fail
 */
static int8_t validate_accel_self_test(
    const struct bmi160_sensor_data* accel_pos,
    const struct bmi160_sensor_data* accel_neg);

/*!
 * @brief This API performs the self test for gyroscope of BMI160
 *
 * @param[in] dev   : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t perform_gyro_self_test(const struct bmi160_dev* dev);

/*!
 * @brief This API enables the self test bit to trigger self test for gyro
 *
 * @param[in] dev   : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t enable_gyro_self_test(const struct bmi160_dev* dev);

/*!
 * @brief This API validates the self test results of gyro
 *
 * @param[in] dev   : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t validate_gyro_self_test(const struct bmi160_dev* dev);

/*!
 *  @brief This API sets FIFO full interrupt of the sensor.This interrupt
 *  occurs when the FIFO is full and the next full data sample would cause
 *  a FIFO overflow, which may delete the old samples.
 *
 * @param[in] int_config    : Structure instance of bmi160_int_settg.
 * @param[in] dev       : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t
    set_fifo_full_int(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This enable the FIFO full interrupt engine.
 *
 * @param[in] int_config    : Structure instance of bmi160_int_settg.
 * @param[in] dev       : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t
    enable_fifo_full_int(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 *  @brief This API sets FIFO watermark interrupt of the sensor.The FIFO
 *  watermark interrupt is fired, when the FIFO fill level is above a fifo
 *  watermark.
 *
 * @param[in] int_config    : Structure instance of bmi160_int_settg.
 * @param[in] dev       : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t
    set_fifo_watermark_int(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This enable the FIFO watermark interrupt engine.
 *
 * @param[in] int_config    : Structure instance of bmi160_int_settg.
 * @param[in] dev       : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static int8_t
    enable_fifo_wtm_int(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 * @brief This API is used to reset the FIFO related configurations
 *  in the fifo_frame structure.
 *
 * @param[in] dev       : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static void reset_fifo_data_structure(const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to read number of bytes filled
 *  currently in FIFO buffer.
 *
 *  @param[in] bytes_to_read  : Number of bytes available in FIFO at the
 *                              instant which is obtained from FIFO counter.
 *  @param[in] dev            : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success / -ve value -> Error.
 *  @retval Any non zero value -> Fail
 *
 */
static int8_t get_fifo_byte_counter(uint16_t* bytes_to_read, struct bmi160_dev const* dev);

/*!
 *  @brief This API is used to compute the number of bytes of accel FIFO data
 *  which is to be parsed in header-less mode
 *
 *  @param[out] data_index        : The start index for parsing data
 *  @param[out] data_read_length  : Number of bytes to be parsed
 *  @param[in]  acc_frame_count   : Number of accelerometer frames to be read
 *  @param[in]  dev               : Structure instance of bmi160_dev.
 *
 */
static void get_accel_len_to_parse(
    uint16_t* data_index,
    uint16_t* data_read_length,
    const uint8_t* acc_frame_count,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to parse the accelerometer data from the
 *  FIFO data in both header mode and header-less mode.
 *  It updates the idx value which is used to store the index of
 *  the current data byte which is parsed.
 *
 *  @param[in,out] acc      : structure instance of sensor data
 *  @param[in,out] idx      : Index value of number of bytes parsed
 *  @param[in,out] acc_idx  : Index value of accelerometer data
 *                                (x,y,z axes) frames parsed
 *  @param[in] frame_info       : It consists of either fifo_data_enable
 *                                parameter in header-less mode or
 *                                frame header data in header mode
 *  @param[in] dev      : structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static void unpack_accel_frame(
    struct bmi160_sensor_data* acc,
    uint16_t* idx,
    uint8_t* acc_idx,
    uint8_t frame_info,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to parse the accelerometer data from the
 *  FIFO data and store it in the instance of the structure bmi160_sensor_data.
 *
 * @param[in,out] accel_data        : structure instance of sensor data
 * @param[in,out] data_start_index  : Index value of number of bytes parsed
 * @param[in] dev           : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static void unpack_accel_data(
    struct bmi160_sensor_data* accel_data,
    uint16_t data_start_index,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to parse the accelerometer data from the
 *  FIFO data in header mode.
 *
 *  @param[in,out] accel_data    : Structure instance of sensor data
 *  @param[in,out] accel_length  : Number of accelerometer frames
 *  @param[in] dev               : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static void extract_accel_header_mode(
    struct bmi160_sensor_data* accel_data,
    uint8_t* accel_length,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API computes the number of bytes of gyro FIFO data
 *  which is to be parsed in header-less mode
 *
 *  @param[out] data_index       : The start index for parsing data
 *  @param[out] data_read_length : No of bytes to be parsed from FIFO buffer
 *  @param[in] gyro_frame_count  : Number of Gyro data frames to be read
 *  @param[in] dev               : Structure instance of bmi160_dev.
 */
static void get_gyro_len_to_parse(
    uint16_t* data_index,
    uint16_t* data_read_length,
    const uint8_t* gyro_frame_count,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to parse the gyroscope's data from the
 *  FIFO data in both header mode and header-less mode.
 *  It updates the idx value which is used to store the index of
 *  the current data byte which is parsed.
 *
 *  @param[in,out] gyro     : structure instance of sensor data
 *  @param[in,out] idx      : Index value of number of bytes parsed
 *  @param[in,out] gyro_idx : Index value of gyro data
 *                                (x,y,z axes) frames parsed
 *  @param[in] frame_info       : It consists of either fifo_data_enable
 *                                parameter in header-less mode or
 *                                frame header data in header mode
 *  @param[in] dev      : structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static void unpack_gyro_frame(
    struct bmi160_sensor_data* gyro,
    uint16_t* idx,
    uint8_t* gyro_idx,
    uint8_t frame_info,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to parse the gyro data from the
 *  FIFO data and store it in the instance of the structure bmi160_sensor_data.
 *
 *  @param[in,out] gyro_data         : structure instance of sensor data
 *  @param[in,out] data_start_index  : Index value of number of bytes parsed
 *  @param[in] dev           : structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static void unpack_gyro_data(
    struct bmi160_sensor_data* gyro_data,
    uint16_t data_start_index,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to parse the gyro data from the
 *  FIFO data in header mode.
 *
 *  @param[in,out] gyro_data     : Structure instance of sensor data
 *  @param[in,out] gyro_length   : Number of gyro frames
 *  @param[in] dev               : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static void extract_gyro_header_mode(
    struct bmi160_sensor_data* gyro_data,
    uint8_t* gyro_length,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API computes the number of bytes of aux FIFO data
 *  which is to be parsed in header-less mode
 *
 *  @param[out] data_index       : The start index for parsing data
 *  @param[out] data_read_length : No of bytes to be parsed from FIFO buffer
 *  @param[in] aux_frame_count   : Number of Aux data frames to be read
 *  @param[in] dev               : Structure instance of bmi160_dev.
 */
static void get_aux_len_to_parse(
    uint16_t* data_index,
    uint16_t* data_read_length,
    const uint8_t* aux_frame_count,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to parse the aux's data from the
 *  FIFO data in both header mode and header-less mode.
 *  It updates the idx value which is used to store the index of
 *  the current data byte which is parsed
 *
 *  @param[in,out] aux_data : structure instance of sensor data
 *  @param[in,out] idx      : Index value of number of bytes parsed
 *  @param[in,out] aux_index    : Index value of gyro data
 *                                (x,y,z axes) frames parsed
 *  @param[in] frame_info       : It consists of either fifo_data_enable
 *                                parameter in header-less mode or
 *                                frame header data in header mode
 *  @param[in] dev      : structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static void unpack_aux_frame(
    struct bmi160_aux_data* aux_data,
    uint16_t* idx,
    uint8_t* aux_index,
    uint8_t frame_info,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to parse the aux data from the
 *  FIFO data and store it in the instance of the structure bmi160_aux_data.
 *
 * @param[in,out] aux_data      : structure instance of sensor data
 * @param[in,out] data_start_index  : Index value of number of bytes parsed
 * @param[in] dev           : structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error
 */
static void unpack_aux_data(
    struct bmi160_aux_data* aux_data,
    uint16_t data_start_index,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to parse the aux data from the
 *  FIFO data in header mode.
 *
 *  @param[in,out] aux_data     : Structure instance of sensor data
 *  @param[in,out] aux_length   : Number of aux frames
 *  @param[in] dev              : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static void extract_aux_header_mode(
    struct bmi160_aux_data* aux_data,
    uint8_t* aux_length,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API checks the presence of non-valid frames in the read fifo data.
 *
 *  @param[in,out] data_index    : The index of the current data to
 *                                be parsed from fifo data
 *  @param[in] dev               : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static void check_frame_validity(uint16_t* data_index, const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to move the data index ahead of the
 *  current_frame_length parameter when unnecessary FIFO data appears while
 *  extracting the user specified data.
 *
 *  @param[in,out] data_index       : Index of the FIFO data which
 *                                  is to be moved ahead of the
 *                                  current_frame_length
 *  @param[in] current_frame_length : Number of bytes in a particular frame
 *  @param[in] dev                  : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static void move_next_frame(
    uint16_t* data_index,
    uint8_t current_frame_length,
    const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to parse and store the sensor time from the
 *  FIFO data in the structure instance dev.
 *
 *  @param[in,out] data_index : Index of the FIFO data which
 *                              has the sensor time.
 *  @param[in] dev            : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static void unpack_sensortime_frame(uint16_t* data_index, const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to parse and store the skipped_frame_count from
 *  the FIFO data in the structure instance dev.
 *
 *  @param[in,out] data_index   : Index of the FIFO data which
 *                                    has the skipped frame count.
 *  @param[in] dev              : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static void unpack_skipped_frame(uint16_t* data_index, const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to get the FOC status from the sensor
 *
 *  @param[in,out] foc_status   : Result of FOC status.
 *  @param[in] dev              : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static int8_t get_foc_status(uint8_t* foc_status, struct bmi160_dev const* dev);

/*!
 *  @brief This API is used to configure the offset enable bits in the sensor
 *
 *  @param[in,out] foc_conf   : Structure instance of bmi160_foc_conf which
 *                                   has the FOC and offset configurations
 *  @param[in] dev            : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static int8_t
    configure_offset_enable(const struct bmi160_foc_conf* foc_conf, struct bmi160_dev const* dev);

/*!
 *  @brief This API is used to trigger the FOC in the sensor
 *
 *  @param[in,out] offset     : Structure instance of bmi160_offsets which
 *                              reads and stores the offset values after FOC
 *  @param[in] dev            : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static int8_t trigger_foc(struct bmi160_offsets* offset, struct bmi160_dev const* dev);

/*!
 *  @brief This API is used to map/unmap the Dataready(Accel & Gyro), FIFO full
 *  and FIFO watermark interrupt
 *
 *  @param[in] int_config     : Structure instance of bmi160_int_settg which
 *                              stores the interrupt type and interrupt channel
 *              configurations to map/unmap the interrupt pins
 *  @param[in] dev            : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static int8_t
    map_hardware_interrupt(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*!
 *  @brief This API is used to map/unmap the Any/Sig motion, Step det/Low-g,
 *  Double tap, Single tap, Orientation, Flat, High-G, Nomotion interrupt pins.
 *
 *  @param[in] int_config     : Structure instance of bmi160_int_settg which
 *                              stores the interrupt type and interrupt channel
 *              configurations to map/unmap the interrupt pins
 *  @param[in] dev            : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval zero -> Success  / -ve value -> Error
 */
static int8_t
    map_feature_interrupt(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev);

/*********************** User function definitions ****************************/

/*!
 * @brief This API reads the data from the given register address
 * of sensor.
 */
int8_t
    bmi160_get_regs(uint8_t reg_addr, uint8_t* data, uint16_t len, const struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;

    /* Null-pointer check */
    if((dev == NULL) || (dev->read == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else if(len == 0) {
        rslt = BMI160_E_READ_WRITE_LENGTH_INVALID;
    } else {
        /* Configuring reg_addr for SPI Interface */
        if(dev->intf == BMI160_SPI_INTF) {
            reg_addr = (reg_addr | BMI160_SPI_RD_MASK);
        }

        rslt = dev->read(dev->id, reg_addr, data, len);
    }

    return rslt;
}

/*!
 * @brief This API writes the given data to the register address
 * of sensor.
 */
int8_t
    bmi160_set_regs(uint8_t reg_addr, uint8_t* data, uint16_t len, const struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;
    uint8_t count = 0;

    /* Null-pointer check */
    if((dev == NULL) || (dev->write == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else if(len == 0) {
        rslt = BMI160_E_READ_WRITE_LENGTH_INVALID;
    } else {
        /* Configuring reg_addr for SPI Interface */
        if(dev->intf == BMI160_SPI_INTF) {
            reg_addr = (reg_addr & BMI160_SPI_WR_MASK);
        }

        if((dev->prev_accel_cfg.power == BMI160_ACCEL_NORMAL_MODE) ||
           (dev->prev_gyro_cfg.power == BMI160_GYRO_NORMAL_MODE)) {
            rslt = dev->write(dev->id, reg_addr, data, len);

            /* Kindly refer bmi160 data sheet section 3.2.4 */
            dev->delay_ms(1);

        } else {
            /*Burst write is not allowed in
             * suspend & low power mode */
            for(; count < len; count++) {
                rslt = dev->write(dev->id, reg_addr, &data[count], 1);
                reg_addr++;

                /* Kindly refer bmi160 data sheet section 3.2.4 */
                dev->delay_ms(1);
            }
        }

        if(rslt != BMI160_OK) {
            rslt = BMI160_E_COM_FAIL;
        }
    }

    return rslt;
}

/*!
 *  @brief This API is the entry point for sensor.It performs
 *  the selection of I2C/SPI read mechanism according to the
 *  selected interface and reads the chip-id of bmi160 sensor.
 */
int8_t bmi160_init(struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data;
    uint8_t try = 3;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);

    /* Dummy read of 0x7F register to enable SPI Interface
     * if SPI is used */
    if((rslt == BMI160_OK) && (dev->intf == BMI160_SPI_INTF)) {
        rslt = bmi160_get_regs(BMI160_SPI_COMM_TEST_ADDR, &data, 1, dev);
    }

    if(rslt == BMI160_OK) {
        /* Assign chip id as zero */
        dev->chip_id = 0;

        while((try--) && (dev->chip_id != BMI160_CHIP_ID)) {
            /* Read chip_id */
            rslt = bmi160_get_regs(BMI160_CHIP_ID_ADDR, &dev->chip_id, 1, dev);
        }

        if((rslt == BMI160_OK) && (dev->chip_id == BMI160_CHIP_ID)) {
            dev->any_sig_sel = BMI160_BOTH_ANY_SIG_MOTION_DISABLED;

            /* Soft reset */
            rslt = bmi160_soft_reset(dev);
        } else {
            rslt = BMI160_E_DEV_NOT_FOUND;
        }
    }

    return rslt;
}

/*!
 * @brief This API resets and restarts the device.
 * All register values are overwritten with default parameters.
 */
int8_t bmi160_soft_reset(struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = BMI160_SOFT_RESET_CMD;

    /* Null-pointer check */
    if((dev == NULL) || (dev->delay_ms == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* Reset the device */
        rslt = bmi160_set_regs(BMI160_COMMAND_REG_ADDR, &data, 1, dev);
        dev->delay_ms(BMI160_SOFT_RESET_DELAY_MS);
        if((rslt == BMI160_OK) && (dev->intf == BMI160_SPI_INTF)) {
            /* Dummy read of 0x7F register to enable SPI Interface
             * if SPI is used */
            rslt = bmi160_get_regs(BMI160_SPI_COMM_TEST_ADDR, &data, 1, dev);
        }

        if(rslt == BMI160_OK) {
            /* Update the default parameters */
            default_param_settg(dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API configures the power mode, range and bandwidth
 * of sensor.
 */
int8_t bmi160_set_sens_conf(struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;

    /* Null-pointer check */
    if((dev == NULL) || (dev->delay_ms == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        rslt = set_accel_conf(dev);
        if(rslt == BMI160_OK) {
            rslt = set_gyro_conf(dev);
            if(rslt == BMI160_OK) {
                /* write power mode for accel and gyro */
                rslt = bmi160_set_power_mode(dev);
                if(rslt == BMI160_OK) {
                    rslt = check_invalid_settg(dev);
                }
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API gets accel and gyro configurations.
 */
int8_t bmi160_get_sens_conf(struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;

    /* Null-pointer check */
    if((dev == NULL) || (dev->delay_ms == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        rslt = get_accel_conf(dev);
        if(rslt == BMI160_OK) {
            rslt = get_gyro_conf(dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the power mode of the sensor.
 */
int8_t bmi160_set_power_mode(struct bmi160_dev* dev) {
    int8_t rslt = 0;

    /* Null-pointer check */
    if((dev == NULL) || (dev->delay_ms == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        rslt = set_accel_pwr(dev);
        if(rslt == BMI160_OK) {
            rslt = set_gyro_pwr(dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API gets the power mode of the sensor.
 */
int8_t bmi160_get_power_mode(struct bmi160_dev* dev) {
    int8_t rslt = 0;
    uint8_t power_mode = 0;

    /* Null-pointer check */
    if((dev == NULL) || (dev->delay_ms == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        rslt = bmi160_get_regs(BMI160_PMU_STATUS_ADDR, &power_mode, 1, dev);
        if(rslt == BMI160_OK) {
            /* Power mode of the accel, gyro sensor is obtained */
            dev->gyro_cfg.power = BMI160_GET_BITS(power_mode, BMI160_GYRO_POWER_MODE);
            dev->accel_cfg.power = BMI160_GET_BITS(power_mode, BMI160_ACCEL_POWER_MODE);
        }
    }

    return rslt;
}

/*!
 * @brief This API reads sensor data, stores it in
 * the bmi160_sensor_data structure pointer passed by the user.
 */
int8_t bmi160_get_sensor_data(
    uint8_t select_sensor,
    struct bmi160_sensor_data* accel,
    struct bmi160_sensor_data* gyro,
    const struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;
    uint8_t time_sel;
    uint8_t sen_sel;
    uint8_t len = 0;

    /*Extract the sensor  and time select information*/
    sen_sel = select_sensor & BMI160_SEN_SEL_MASK;
    time_sel = ((sen_sel & BMI160_TIME_SEL) >> 2);
    sen_sel = sen_sel & (BMI160_ACCEL_SEL | BMI160_GYRO_SEL);
    if(time_sel == 1) {
        len = 3;
    }

    /* Null-pointer check */
    if(dev != NULL) {
        switch(sen_sel) {
        case BMI160_ACCEL_ONLY:

            /* Null-pointer check */
            if(accel == NULL) {
                rslt = BMI160_E_NULL_PTR;
            } else {
                rslt = get_accel_data(len, accel, dev);
            }

            break;
        case BMI160_GYRO_ONLY:

            /* Null-pointer check */
            if(gyro == NULL) {
                rslt = BMI160_E_NULL_PTR;
            } else {
                rslt = get_gyro_data(len, gyro, dev);
            }

            break;
        case BMI160_BOTH_ACCEL_AND_GYRO:

            /* Null-pointer check */
            if((gyro == NULL) || (accel == NULL)) {
                rslt = BMI160_E_NULL_PTR;
            } else {
                rslt = get_accel_gyro_data(len, accel, gyro, dev);
            }

            break;
        default:
            rslt = BMI160_E_INVALID_INPUT;
            break;
        }
    } else {
        rslt = BMI160_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API configures the necessary interrupt based on
 *  the user settings in the bmi160_int_settg structure instance.
 */
int8_t bmi160_set_int_config(struct bmi160_int_settg* int_config, struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;

    switch(int_config->int_type) {
    case BMI160_ACC_ANY_MOTION_INT:

        /*Any-motion  interrupt*/
        rslt = set_accel_any_motion_int(int_config, dev);
        break;
    case BMI160_ACC_SIG_MOTION_INT:

        /* Significant motion interrupt */
        rslt = set_accel_sig_motion_int(int_config, dev);
        break;
    case BMI160_ACC_SLOW_NO_MOTION_INT:

        /* Slow or no motion interrupt */
        rslt = set_accel_no_motion_int(int_config, dev);
        break;
    case BMI160_ACC_DOUBLE_TAP_INT:
    case BMI160_ACC_SINGLE_TAP_INT:

        /* Double tap and single tap Interrupt */
        rslt = set_accel_tap_int(int_config, dev);
        break;
    case BMI160_STEP_DETECT_INT:

        /* Step detector interrupt */
        rslt = set_accel_step_detect_int(int_config, dev);
        break;
    case BMI160_ACC_ORIENT_INT:

        /* Orientation interrupt */
        rslt = set_accel_orientation_int(int_config, dev);
        break;
    case BMI160_ACC_FLAT_INT:

        /* Flat detection interrupt */
        rslt = set_accel_flat_detect_int(int_config, dev);
        break;
    case BMI160_ACC_LOW_G_INT:

        /* Low-g interrupt */
        rslt = set_accel_low_g_int(int_config, dev);
        break;
    case BMI160_ACC_HIGH_G_INT:

        /* High-g interrupt */
        rslt = set_accel_high_g_int(int_config, dev);
        break;
    case BMI160_ACC_GYRO_DATA_RDY_INT:

        /* Data ready interrupt */
        rslt = set_accel_gyro_data_ready_int(int_config, dev);
        break;
    case BMI160_ACC_GYRO_FIFO_FULL_INT:

        /* Fifo full interrupt */
        rslt = set_fifo_full_int(int_config, dev);
        break;
    case BMI160_ACC_GYRO_FIFO_WATERMARK_INT:

        /* Fifo water-mark interrupt */
        rslt = set_fifo_watermark_int(int_config, dev);
        break;
    case BMI160_FIFO_TAG_INT_PIN:

        /* Fifo tagging feature support */
        /* Configure Interrupt pins */
        rslt = set_intr_pin_config(int_config, dev);
        break;
    default:
        break;
    }

    return rslt;
}

/*!
 * @brief This API enables or disable the step counter feature.
 * 1 - enable step counter (0 - disable)
 */
int8_t bmi160_set_step_counter(uint8_t step_cnt_enable, const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if(rslt != BMI160_OK) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        rslt = bmi160_get_regs(BMI160_INT_STEP_CONFIG_1_ADDR, &data, 1, dev);
        if(rslt == BMI160_OK) {
            if(step_cnt_enable == BMI160_ENABLE) {
                data |= (uint8_t)(step_cnt_enable << 3);
            } else {
                data &= ~BMI160_STEP_COUNT_EN_BIT_MASK;
            }

            rslt = bmi160_set_regs(BMI160_INT_STEP_CONFIG_1_ADDR, &data, 1, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API reads the step counter value.
 */
int8_t bmi160_read_step_counter(uint16_t* step_val, const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data[2] = {0, 0};
    uint16_t msb = 0;
    uint8_t lsb = 0;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if(rslt != BMI160_OK) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        rslt = bmi160_get_regs(BMI160_INT_STEP_CNT_0_ADDR, data, 2, dev);
        if(rslt == BMI160_OK) {
            lsb = data[0];
            msb = data[1] << 8;
            *step_val = msb | lsb;
        }
    }

    return rslt;
}

/*!
 * @brief This API reads the mention no of byte of data from the given
 * register address of auxiliary sensor.
 */
int8_t bmi160_aux_read(
    uint8_t reg_addr,
    uint8_t* aux_data,
    uint16_t len,
    const struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;
    uint16_t map_len = 0;

    /* Null-pointer check */
    if((dev == NULL) || (dev->read == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        if(dev->aux_cfg.aux_sensor_enable == BMI160_ENABLE) {
            rslt = map_read_len(&map_len, dev);
            if(rslt == BMI160_OK) {
                rslt = extract_aux_read(map_len, reg_addr, aux_data, len, dev);
            }
        } else {
            rslt = BMI160_E_INVALID_INPUT;
        }
    }

    return rslt;
}

/*!
 * @brief This API writes the mention no of byte of data to the given
 * register address of auxiliary sensor.
 */
int8_t bmi160_aux_write(
    uint8_t reg_addr,
    uint8_t* aux_data,
    uint16_t len,
    const struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;
    uint8_t count = 0;

    /* Null-pointer check */
    if((dev == NULL) || (dev->write == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        for(; count < len; count++) {
            /* set data to write */
            rslt = bmi160_set_regs(BMI160_AUX_IF_4_ADDR, aux_data, 1, dev);
            dev->delay_ms(BMI160_AUX_COM_DELAY);
            if(rslt == BMI160_OK) {
                /* set address to write */
                rslt = bmi160_set_regs(BMI160_AUX_IF_3_ADDR, &reg_addr, 1, dev);
                dev->delay_ms(BMI160_AUX_COM_DELAY);
                if(rslt == BMI160_OK && (count < len - 1)) {
                    aux_data++;
                    reg_addr++;
                }
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API initialize the auxiliary sensor
 * in order to access it.
 */
int8_t bmi160_aux_init(const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if(rslt != BMI160_OK) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        if(dev->aux_cfg.aux_sensor_enable == BMI160_ENABLE) {
            /* Configures the auxiliary sensor interface settings */
            rslt = config_aux_settg(dev);
        } else {
            rslt = BMI160_E_INVALID_INPUT;
        }
    }

    return rslt;
}

/*!
 * @brief This API is used to setup the auxiliary sensor of bmi160 in auto mode
 * Thus enabling the auto update of 8 bytes of data from auxiliary sensor
 * to BMI160 register address 0x04 to 0x0B
 */
int8_t bmi160_set_aux_auto_mode(uint8_t* data_addr, struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if(rslt != BMI160_OK) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        if(dev->aux_cfg.aux_sensor_enable == BMI160_ENABLE) {
            /* Write the aux. address to read in 0x4D of BMI160*/
            rslt = bmi160_set_regs(BMI160_AUX_IF_2_ADDR, data_addr, 1, dev);
            dev->delay_ms(BMI160_AUX_COM_DELAY);
            if(rslt == BMI160_OK) {
                /* Configure the polling ODR for
                 * auxiliary sensor */
                rslt = config_aux_odr(dev);
                if(rslt == BMI160_OK) {
                    /* Disable the aux. manual mode, i.e aux.
                     * sensor is in auto-mode (data-mode) */
                    dev->aux_cfg.manual_enable = BMI160_DISABLE;
                    rslt = bmi160_config_aux_mode(dev);

                    /*  Auxiliary sensor data is obtained
                     * in auto mode from this point */
                }
            }
        } else {
            rslt = BMI160_E_INVALID_INPUT;
        }
    }

    return rslt;
}

/*!
 * @brief This API configures the 0x4C register and settings like
 * Auxiliary sensor manual enable/ disable and aux burst read length.
 */
int8_t bmi160_config_aux_mode(const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t aux_if[2] = {(uint8_t)(dev->aux_cfg.aux_i2c_addr * 2), 0};

    rslt = bmi160_get_regs(BMI160_AUX_IF_1_ADDR, &aux_if[1], 1, dev);
    if(rslt == BMI160_OK) {
        /* update the Auxiliary interface to manual/auto mode */
        aux_if[1] = BMI160_SET_BITS(aux_if[1], BMI160_MANUAL_MODE_EN, dev->aux_cfg.manual_enable);

        /* update the burst read length defined by user */
        aux_if[1] =
            BMI160_SET_BITS_POS_0(aux_if[1], BMI160_AUX_READ_BURST, dev->aux_cfg.aux_rd_burst_len);

        /* Set the secondary interface address and manual mode
         * along with burst read length */
        rslt = bmi160_set_regs(BMI160_AUX_IF_0_ADDR, &aux_if[0], 2, dev);
        dev->delay_ms(BMI160_AUX_COM_DELAY);
    }

    return rslt;
}

/*!
 * @brief This API is used to read the raw uncompensated auxiliary sensor
 * data of 8 bytes from BMI160 register address 0x04 to 0x0B
 */
int8_t bmi160_read_aux_data_auto_mode(uint8_t* aux_data, const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if(rslt != BMI160_OK) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        if((dev->aux_cfg.aux_sensor_enable == BMI160_ENABLE) &&
           (dev->aux_cfg.manual_enable == BMI160_DISABLE)) {
            /* Read the aux. sensor's raw data */
            rslt = bmi160_get_regs(BMI160_AUX_DATA_ADDR, aux_data, 8, dev);
        } else {
            rslt = BMI160_E_INVALID_INPUT;
        }
    }

    return rslt;
}

/*!
 * @brief This is used to perform self test of accel/gyro of the BMI160 sensor
 */
int8_t bmi160_perform_self_test(uint8_t select_sensor, struct bmi160_dev* dev) {
    int8_t rslt;
    int8_t self_test_rslt = 0;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if(rslt != BMI160_OK) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* Proceed if null check is fine */
        switch(select_sensor) {
        case BMI160_ACCEL_ONLY:
            rslt = perform_accel_self_test(dev);
            break;
        case BMI160_GYRO_ONLY:

            /* Set the power mode as normal mode */
            dev->gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;
            rslt = bmi160_set_power_mode(dev);

            /* Perform gyro self test */
            if(rslt == BMI160_OK) {
                /* Perform gyro self test */
                rslt = perform_gyro_self_test(dev);
            }

            break;
        default:
            rslt = BMI160_E_INVALID_INPUT;
            break;
        }

        /* Check to ensure bus error does not occur */
        if(rslt >= BMI160_OK) {
            /* Store the status of self test result */
            self_test_rslt = rslt;

            /* Perform soft reset */
            rslt = bmi160_soft_reset(dev);
        }

        /* Check to ensure bus operations are success */
        if(rslt == BMI160_OK) {
            /* Restore self_test_rslt as return value */
            rslt = self_test_rslt;
        }
    }

    return rslt;
}

/*!
 * @brief This API reads the data from fifo buffer.
 */
int8_t bmi160_get_fifo_data(struct bmi160_dev const* dev) {
    int8_t rslt = 0;
    uint16_t bytes_to_read = 0;
    uint16_t user_fifo_len = 0;

    /* check the bmi160 structure as NULL*/
    if((dev == NULL) || (dev->fifo->data == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        reset_fifo_data_structure(dev);

        /* get current FIFO fill-level*/
        rslt = get_fifo_byte_counter(&bytes_to_read, dev);
        if(rslt == BMI160_OK) {
            user_fifo_len = dev->fifo->length;
            if((dev->fifo->length > bytes_to_read)) {
                /* Handling the case where user requests
                 * more data than available in FIFO */
                dev->fifo->length = bytes_to_read;
            }

            if((dev->fifo->fifo_time_enable == BMI160_FIFO_TIME_ENABLE) &&
               (bytes_to_read + BMI160_FIFO_BYTES_OVERREAD <= user_fifo_len)) {
                /* Handling case of sensor time availability*/
                dev->fifo->length = dev->fifo->length + BMI160_FIFO_BYTES_OVERREAD;
            }

            /* read only the filled bytes in the FIFO Buffer */
            rslt = bmi160_get_regs(BMI160_FIFO_DATA_ADDR, dev->fifo->data, dev->fifo->length, dev);
        }
    }

    return rslt;
}

/*!
 *  @brief This API writes fifo_flush command to command register.This
 *  action clears all data in the Fifo without changing fifo configuration
 *  settings
 */
int8_t bmi160_set_fifo_flush(const struct bmi160_dev* dev) {
    int8_t rslt = 0;
    uint8_t data = BMI160_FIFO_FLUSH_VALUE;
    uint8_t reg_addr = BMI160_COMMAND_REG_ADDR;

    /* Check the bmi160_dev structure for NULL address*/
    if(dev == NULL) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        rslt = bmi160_set_regs(reg_addr, &data, BMI160_ONE, dev);
    }

    return rslt;
}

/*!
 * @brief This API sets the FIFO configuration in the sensor.
 */
int8_t bmi160_set_fifo_config(uint8_t config, uint8_t enable, struct bmi160_dev const* dev) {
    int8_t rslt = 0;
    uint8_t data = 0;
    uint8_t reg_addr = BMI160_FIFO_CONFIG_1_ADDR;
    uint8_t fifo_config = config & BMI160_FIFO_CONFIG_1_MASK;

    /* Check the bmi160_dev structure for NULL address*/
    if(dev == NULL) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        rslt = bmi160_get_regs(reg_addr, &data, BMI160_ONE, dev);
        if(rslt == BMI160_OK) {
            if(fifo_config > 0) {
                if(enable == BMI160_ENABLE) {
                    data = data | fifo_config;
                } else {
                    data = data & (~fifo_config);
                }
            }

            /* write fifo frame content configuration*/
            rslt = bmi160_set_regs(reg_addr, &data, BMI160_ONE, dev);
            if(rslt == BMI160_OK) {
                /* read fifo frame content configuration*/
                rslt = bmi160_get_regs(reg_addr, &data, BMI160_ONE, dev);
                if(rslt == BMI160_OK) {
                    /* extract fifo header enabled status */
                    dev->fifo->fifo_header_enable = data & BMI160_FIFO_HEAD_ENABLE;

                    /* extract accel/gyr/aux. data enabled status */
                    dev->fifo->fifo_data_enable = data & BMI160_FIFO_M_G_A_ENABLE;

                    /* extract fifo sensor time enabled status */
                    dev->fifo->fifo_time_enable = data & BMI160_FIFO_TIME_ENABLE;
                }
            }
        }
    }

    return rslt;
}

/*! @brief This API is used to configure the down sampling ratios of
 *  the accel and gyro data for FIFO.Also, it configures filtered or
 *  pre-filtered data for accel and gyro.
 *
 */
int8_t bmi160_set_fifo_down(uint8_t fifo_down, const struct bmi160_dev* dev) {
    int8_t rslt = 0;
    uint8_t data = 0;
    uint8_t reg_addr = BMI160_FIFO_DOWN_ADDR;

    /* Check the bmi160_dev structure for NULL address*/
    if(dev == NULL) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        rslt = bmi160_get_regs(reg_addr, &data, BMI160_ONE, dev);
        if(rslt == BMI160_OK) {
            data = data | fifo_down;
            rslt = bmi160_set_regs(reg_addr, &data, BMI160_ONE, dev);
        }
    }

    return rslt;
}

/*!
 *  @brief This API sets the FIFO watermark level in the sensor.
 *
 */
int8_t bmi160_set_fifo_wm(uint8_t fifo_wm, const struct bmi160_dev* dev) {
    int8_t rslt = 0;
    uint8_t data = fifo_wm;
    uint8_t reg_addr = BMI160_FIFO_CONFIG_0_ADDR;

    /* Check the bmi160_dev structure for NULL address*/
    if(dev == NULL) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        rslt = bmi160_set_regs(reg_addr, &data, BMI160_ONE, dev);
    }

    return rslt;
}

/*!
 *  @brief This API parses and extracts the accelerometer frames from
 *  FIFO data read by the "bmi160_get_fifo_data" API and stores it in
 *  the "accel_data" structure instance.
 */
int8_t bmi160_extract_accel(
    struct bmi160_sensor_data* accel_data,
    uint8_t* accel_length,
    struct bmi160_dev const* dev) {
    int8_t rslt = 0;
    uint16_t data_index = 0;
    uint16_t data_read_length = 0;
    uint8_t accel_index = 0;
    uint8_t fifo_data_enable = 0;

    if(dev == NULL || dev->fifo == NULL || dev->fifo->data == NULL) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* Parsing the FIFO data in header-less mode */
        if(dev->fifo->fifo_header_enable == 0) {
            /* Number of bytes to be parsed from FIFO */
            get_accel_len_to_parse(&data_index, &data_read_length, accel_length, dev);
            for(; data_index < data_read_length;) {
                /*Check for the availability of next two bytes of FIFO data */
                check_frame_validity(&data_index, dev);
                fifo_data_enable = dev->fifo->fifo_data_enable;
                unpack_accel_frame(accel_data, &data_index, &accel_index, fifo_data_enable, dev);
            }

            /* update number of accel data read*/
            *accel_length = accel_index;

            /*update the accel byte index*/
            dev->fifo->accel_byte_start_idx = data_index;
        } else {
            /* Parsing the FIFO data in header mode */
            extract_accel_header_mode(accel_data, accel_length, dev);
        }
    }

    return rslt;
}

/*!
 *  @brief This API parses and extracts the gyro frames from
 *  FIFO data read by the "bmi160_get_fifo_data" API and stores it in
 *  the "gyro_data" structure instance.
 */
int8_t bmi160_extract_gyro(
    struct bmi160_sensor_data* gyro_data,
    uint8_t* gyro_length,
    struct bmi160_dev const* dev) {
    int8_t rslt = 0;
    uint16_t data_index = 0;
    uint16_t data_read_length = 0;
    uint8_t gyro_index = 0;
    uint8_t fifo_data_enable = 0;

    if(dev == NULL || dev->fifo->data == NULL) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* Parsing the FIFO data in header-less mode */
        if(dev->fifo->fifo_header_enable == 0) {
            /* Number of bytes to be parsed from FIFO */
            get_gyro_len_to_parse(&data_index, &data_read_length, gyro_length, dev);
            for(; data_index < data_read_length;) {
                /*Check for the availability of next two bytes of FIFO data */
                check_frame_validity(&data_index, dev);
                fifo_data_enable = dev->fifo->fifo_data_enable;
                unpack_gyro_frame(gyro_data, &data_index, &gyro_index, fifo_data_enable, dev);
            }

            /* update number of gyro data read */
            *gyro_length = gyro_index;

            /* update the gyro byte index */
            dev->fifo->gyro_byte_start_idx = data_index;
        } else {
            /* Parsing the FIFO data in header mode */
            extract_gyro_header_mode(gyro_data, gyro_length, dev);
        }
    }

    return rslt;
}

/*!
 *  @brief This API parses and extracts the aux frames from
 *  FIFO data read by the "bmi160_get_fifo_data" API and stores it in
 *  the "aux_data" structure instance.
 */
int8_t bmi160_extract_aux(
    struct bmi160_aux_data* aux_data,
    uint8_t* aux_len,
    struct bmi160_dev const* dev) {
    int8_t rslt = 0;
    uint16_t data_index = 0;
    uint16_t data_read_length = 0;
    uint8_t aux_index = 0;
    uint8_t fifo_data_enable = 0;

    if((dev == NULL) || (dev->fifo->data == NULL) || (aux_data == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* Parsing the FIFO data in header-less mode */
        if(dev->fifo->fifo_header_enable == 0) {
            /* Number of bytes to be parsed from FIFO */
            get_aux_len_to_parse(&data_index, &data_read_length, aux_len, dev);
            for(; data_index < data_read_length;) {
                /* Check for the availability of next two
                 * bytes of FIFO data */
                check_frame_validity(&data_index, dev);
                fifo_data_enable = dev->fifo->fifo_data_enable;
                unpack_aux_frame(aux_data, &data_index, &aux_index, fifo_data_enable, dev);
            }

            /* update number of aux data read */
            *aux_len = aux_index;

            /* update the aux byte index */
            dev->fifo->aux_byte_start_idx = data_index;
        } else {
            /* Parsing the FIFO data in header mode */
            extract_aux_header_mode(aux_data, aux_len, dev);
        }
    }

    return rslt;
}

/*!
 *  @brief This API starts the FOC of accel and gyro
 *
 *  @note FOC should not be used in low-power mode of sensor
 *
 *  @note Accel FOC targets values of +1g , 0g , -1g
 *  Gyro FOC always targets value of 0 dps
 */
int8_t bmi160_start_foc(
    const struct bmi160_foc_conf* foc_conf,
    struct bmi160_offsets* offset,
    struct bmi160_dev const* dev) {
    int8_t rslt;
    uint8_t data;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if(rslt != BMI160_OK) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* Set the offset enable bits */
        rslt = configure_offset_enable(foc_conf, dev);
        if(rslt == BMI160_OK) {
            /* Read the FOC config from the sensor */
            rslt = bmi160_get_regs(BMI160_FOC_CONF_ADDR, &data, 1, dev);

            /* Set the FOC config for gyro */
            data = BMI160_SET_BITS(data, BMI160_GYRO_FOC_EN, foc_conf->foc_gyr_en);

            /* Set the FOC config for accel xyz axes */
            data = BMI160_SET_BITS(data, BMI160_ACCEL_FOC_X_CONF, foc_conf->foc_acc_x);
            data = BMI160_SET_BITS(data, BMI160_ACCEL_FOC_Y_CONF, foc_conf->foc_acc_y);
            data = BMI160_SET_BITS_POS_0(data, BMI160_ACCEL_FOC_Z_CONF, foc_conf->foc_acc_z);
            if(rslt == BMI160_OK) {
                /* Set the FOC config in the sensor */
                rslt = bmi160_set_regs(BMI160_FOC_CONF_ADDR, &data, 1, dev);
                if(rslt == BMI160_OK) {
                    /* Procedure to trigger
                     * FOC and check status */
                    rslt = trigger_foc(offset, dev);
                }
            }
        }
    }

    return rslt;
}

/*!
 *  @brief This API reads and stores the offset values of accel and gyro
 */
int8_t bmi160_get_offsets(struct bmi160_offsets* offset, const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data[7];
    uint8_t lsb, msb;
    int16_t offset_msb, offset_lsb;
    int16_t offset_data;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if(rslt != BMI160_OK) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* Read the FOC config from the sensor */
        rslt = bmi160_get_regs(BMI160_OFFSET_ADDR, data, 7, dev);

        /* Accel offsets */
        offset->off_acc_x = (int8_t)data[0];
        offset->off_acc_y = (int8_t)data[1];
        offset->off_acc_z = (int8_t)data[2];

        /* Gyro x-axis offset */
        lsb = data[3];
        msb = BMI160_GET_BITS_POS_0(data[6], BMI160_GYRO_OFFSET_X);
        offset_msb = (int16_t)(msb << 14);
        offset_lsb = lsb << 6;
        offset_data = offset_msb | offset_lsb;

        /* Divide by 64 to get the Right shift by 6 value */
        offset->off_gyro_x = (int16_t)(offset_data / 64);

        /* Gyro y-axis offset */
        lsb = data[4];
        msb = BMI160_GET_BITS(data[6], BMI160_GYRO_OFFSET_Y);
        offset_msb = (int16_t)(msb << 14);
        offset_lsb = lsb << 6;
        offset_data = offset_msb | offset_lsb;

        /* Divide by 64 to get the Right shift by 6 value */
        offset->off_gyro_y = (int16_t)(offset_data / 64);

        /* Gyro z-axis offset */
        lsb = data[5];
        msb = BMI160_GET_BITS(data[6], BMI160_GYRO_OFFSET_Z);
        offset_msb = (int16_t)(msb << 14);
        offset_lsb = lsb << 6;
        offset_data = offset_msb | offset_lsb;

        /* Divide by 64 to get the Right shift by 6 value */
        offset->off_gyro_z = (int16_t)(offset_data / 64);
    }

    return rslt;
}

/*!
 *  @brief This API writes the offset values of accel and gyro to
 *  the sensor but these values will be reset on POR or soft reset.
 */
int8_t bmi160_set_offsets(
    const struct bmi160_foc_conf* foc_conf,
    const struct bmi160_offsets* offset,
    struct bmi160_dev const* dev) {
    int8_t rslt;
    uint8_t data[7];
    uint8_t x_msb, y_msb, z_msb;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if(rslt != BMI160_OK) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* Update the accel offset */
        data[0] = (uint8_t)offset->off_acc_x;
        data[1] = (uint8_t)offset->off_acc_y;
        data[2] = (uint8_t)offset->off_acc_z;

        /* Update the LSB of gyro offset */
        data[3] = BMI160_GET_LSB(offset->off_gyro_x);
        data[4] = BMI160_GET_LSB(offset->off_gyro_y);
        data[5] = BMI160_GET_LSB(offset->off_gyro_z);

        /* Update the MSB of gyro offset */
        x_msb = BMI160_GET_BITS(offset->off_gyro_x, BMI160_GYRO_OFFSET);
        y_msb = BMI160_GET_BITS(offset->off_gyro_y, BMI160_GYRO_OFFSET);
        z_msb = BMI160_GET_BITS(offset->off_gyro_z, BMI160_GYRO_OFFSET);
        data[6] = (uint8_t)(z_msb << 4 | y_msb << 2 | x_msb);

        /* Set the offset enable/disable for gyro and accel */
        data[6] = BMI160_SET_BITS(data[6], BMI160_GYRO_OFFSET_EN, foc_conf->gyro_off_en);
        data[6] = BMI160_SET_BITS(data[6], BMI160_ACCEL_OFFSET_EN, foc_conf->acc_off_en);

        /* Set the offset config and values in the sensor */
        rslt = bmi160_set_regs(BMI160_OFFSET_ADDR, data, 7, dev);
    }

    return rslt;
}

/*!
 *  @brief This API writes the image registers values to NVM which is
 *  stored even after POR or soft reset
 */
int8_t bmi160_update_nvm(struct bmi160_dev const* dev) {
    int8_t rslt;
    uint8_t data;
    uint8_t cmd = BMI160_NVM_BACKUP_EN;

    /* Read the nvm_prog_en configuration */
    rslt = bmi160_get_regs(BMI160_CONF_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        data = BMI160_SET_BITS(data, BMI160_NVM_UPDATE, 1);

        /* Set the nvm_prog_en bit in the sensor */
        rslt = bmi160_set_regs(BMI160_CONF_ADDR, &data, 1, dev);
        if(rslt == BMI160_OK) {
            /* Update NVM */
            rslt = bmi160_set_regs(BMI160_COMMAND_REG_ADDR, &cmd, 1, dev);
            if(rslt == BMI160_OK) {
                /* Check for NVM ready status */
                rslt = bmi160_get_regs(BMI160_STATUS_ADDR, &data, 1, dev);
                if(rslt == BMI160_OK) {
                    data = BMI160_GET_BITS(data, BMI160_NVM_STATUS);
                    if(data != BMI160_ENABLE) {
                        /* Delay to update NVM */
                        dev->delay_ms(25);
                    }
                }
            }
        }
    }

    return rslt;
}

/*!
 *  @brief This API gets the interrupt status from the sensor.
 */
int8_t bmi160_get_int_status(
    enum bmi160_int_status_sel int_status_sel,
    union bmi160_int_status* int_status,
    struct bmi160_dev const* dev) {
    int8_t rslt = 0;

    /* To get the status of all interrupts */
    if(int_status_sel == BMI160_INT_STATUS_ALL) {
        rslt = bmi160_get_regs(BMI160_INT_STATUS_ADDR, &int_status->data[0], 4, dev);
    } else {
        if(int_status_sel & BMI160_INT_STATUS_0) {
            rslt = bmi160_get_regs(BMI160_INT_STATUS_ADDR, &int_status->data[0], 1, dev);
        }

        if(int_status_sel & BMI160_INT_STATUS_1) {
            rslt = bmi160_get_regs(BMI160_INT_STATUS_ADDR + 1, &int_status->data[1], 1, dev);
        }

        if(int_status_sel & BMI160_INT_STATUS_2) {
            rslt = bmi160_get_regs(BMI160_INT_STATUS_ADDR + 2, &int_status->data[2], 1, dev);
        }

        if(int_status_sel & BMI160_INT_STATUS_3) {
            rslt = bmi160_get_regs(BMI160_INT_STATUS_ADDR + 3, &int_status->data[3], 1, dev);
        }
    }

    return rslt;
}

/*********************** Local function definitions ***************************/

/*!
 * @brief This API sets the any-motion interrupt of the sensor.
 * This interrupt occurs when accel values exceeds preset threshold
 * for a certain period of time.
 */
static int8_t
    set_accel_any_motion_int(struct bmi160_int_settg* int_config, struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if((rslt != BMI160_OK) || (int_config == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* updating the interrupt structure to local structure */
        struct bmi160_acc_any_mot_int_cfg* any_motion_int_cfg =
            &(int_config->int_type_cfg.acc_any_motion_int);
        rslt = enable_accel_any_motion_int(any_motion_int_cfg, dev);
        if(rslt == BMI160_OK) {
            rslt = config_any_motion_int_settg(int_config, any_motion_int_cfg, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API sets tap interrupts.Interrupt is fired when
 * tap movements happen.
 */
static int8_t
    set_accel_tap_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if((rslt != BMI160_OK) || (int_config == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* updating the interrupt structure to local structure */
        struct bmi160_acc_tap_int_cfg* tap_int_cfg = &(int_config->int_type_cfg.acc_tap_int);
        rslt = enable_tap_int(int_config, tap_int_cfg, dev);
        if(rslt == BMI160_OK) {
            /* Configure Interrupt pins */
            rslt = set_intr_pin_config(int_config, dev);
            if(rslt == BMI160_OK) {
                rslt = config_tap_int_settg(int_config, tap_int_cfg, dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the data ready interrupt for both accel and gyro.
 * This interrupt occurs when new accel and gyro data comes.
 */
static int8_t set_accel_gyro_data_ready_int(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if((rslt != BMI160_OK) || (int_config == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        rslt = enable_data_ready_int(dev);
        if(rslt == BMI160_OK) {
            /* Configure Interrupt pins */
            rslt = set_intr_pin_config(int_config, dev);
            if(rslt == BMI160_OK) {
                rslt = map_hardware_interrupt(int_config, dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the significant motion interrupt of the sensor.This
 * interrupt occurs when there is change in user location.
 */
static int8_t
    set_accel_sig_motion_int(struct bmi160_int_settg* int_config, struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if((rslt != BMI160_OK) || (int_config == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* updating the interrupt structure to local structure */
        struct bmi160_acc_sig_mot_int_cfg* sig_mot_int_cfg =
            &(int_config->int_type_cfg.acc_sig_motion_int);
        rslt = enable_sig_motion_int(sig_mot_int_cfg, dev);
        if(rslt == BMI160_OK) {
            rslt = config_sig_motion_int_settg(int_config, sig_mot_int_cfg, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the no motion/slow motion interrupt of the sensor.
 * Slow motion is similar to any motion interrupt.No motion interrupt
 * occurs when slope bet. two accel values falls below preset threshold
 * for preset duration.
 */
static int8_t
    set_accel_no_motion_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if((rslt != BMI160_OK) || (int_config == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* updating the interrupt structure to local structure */
        struct bmi160_acc_no_motion_int_cfg* no_mot_int_cfg =
            &(int_config->int_type_cfg.acc_no_motion_int);
        rslt = enable_no_motion_int(no_mot_int_cfg, dev);
        if(rslt == BMI160_OK) {
            /* Configure the INT PIN settings*/
            rslt = config_no_motion_int_settg(int_config, no_mot_int_cfg, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the step detection interrupt.This interrupt
 * occurs when the single step causes accel values to go above
 * preset threshold.
 */
static int8_t
    set_accel_step_detect_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if((rslt != BMI160_OK) || (int_config == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* updating the interrupt structure to local structure */
        struct bmi160_acc_step_detect_int_cfg* step_detect_int_cfg =
            &(int_config->int_type_cfg.acc_step_detect_int);
        rslt = enable_step_detect_int(step_detect_int_cfg, dev);
        if(rslt == BMI160_OK) {
            /* Configure Interrupt pins */
            rslt = set_intr_pin_config(int_config, dev);
            if(rslt == BMI160_OK) {
                rslt = map_feature_interrupt(int_config, dev);
                if(rslt == BMI160_OK) {
                    rslt = config_step_detect(step_detect_int_cfg, dev);
                }
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the orientation interrupt of the sensor.This
 * interrupt occurs when there is orientation change in the sensor
 * with respect to gravitational field vector g.
 */
static int8_t
    set_accel_orientation_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if((rslt != BMI160_OK) || (int_config == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* updating the interrupt structure to local structure */
        struct bmi160_acc_orient_int_cfg* orient_int_cfg =
            &(int_config->int_type_cfg.acc_orient_int);
        rslt = enable_orient_int(orient_int_cfg, dev);
        if(rslt == BMI160_OK) {
            /* Configure Interrupt pins */
            rslt = set_intr_pin_config(int_config, dev);
            if(rslt == BMI160_OK) {
                /* map INT pin to orient interrupt */
                rslt = map_feature_interrupt(int_config, dev);
                if(rslt == BMI160_OK) {
                    /* configure the
                     * orientation setting*/
                    rslt = config_orient_int_settg(orient_int_cfg, dev);
                }
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the flat interrupt of the sensor.This interrupt
 * occurs in case of flat orientation
 */
static int8_t
    set_accel_flat_detect_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if((rslt != BMI160_OK) || (int_config == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* updating the interrupt structure to local structure */
        struct bmi160_acc_flat_detect_int_cfg* flat_detect_int =
            &(int_config->int_type_cfg.acc_flat_int);

        /* enable the flat interrupt */
        rslt = enable_flat_int(flat_detect_int, dev);
        if(rslt == BMI160_OK) {
            /* Configure Interrupt pins */
            rslt = set_intr_pin_config(int_config, dev);
            if(rslt == BMI160_OK) {
                /* map INT pin to flat interrupt */
                rslt = map_feature_interrupt(int_config, dev);
                if(rslt == BMI160_OK) {
                    /* configure the flat setting*/
                    rslt = config_flat_int_settg(flat_detect_int, dev);
                }
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the low-g interrupt of the sensor.This interrupt
 * occurs during free-fall.
 */
static int8_t
    set_accel_low_g_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if((rslt != BMI160_OK) || (int_config == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* updating the interrupt structure to local structure */
        struct bmi160_acc_low_g_int_cfg* low_g_int = &(int_config->int_type_cfg.acc_low_g_int);

        /* Enable the low-g interrupt*/
        rslt = enable_low_g_int(low_g_int, dev);
        if(rslt == BMI160_OK) {
            /* Configure Interrupt pins */
            rslt = set_intr_pin_config(int_config, dev);
            if(rslt == BMI160_OK) {
                /* Map INT pin to low-g interrupt */
                rslt = map_feature_interrupt(int_config, dev);
                if(rslt == BMI160_OK) {
                    /* configure the data source
                     * for low-g interrupt*/
                    rslt = config_low_g_data_src(low_g_int, dev);
                    if(rslt == BMI160_OK) {
                        rslt = config_low_g_int_settg(low_g_int, dev);
                    }
                }
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the high-g interrupt of the sensor.The interrupt
 * occurs if the absolute value of acceleration data of any enabled axis
 * exceeds the programmed threshold and the sign of the value does not
 * change for a preset duration.
 */
static int8_t
    set_accel_high_g_int(struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if((rslt != BMI160_OK) || (int_config == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* updating the interrupt structure to local structure */
        struct bmi160_acc_high_g_int_cfg* high_g_int_cfg =
            &(int_config->int_type_cfg.acc_high_g_int);

        /* Enable the high-g interrupt */
        rslt = enable_high_g_int(high_g_int_cfg, dev);
        if(rslt == BMI160_OK) {
            /* Configure Interrupt pins */
            rslt = set_intr_pin_config(int_config, dev);
            if(rslt == BMI160_OK) {
                /* Map INT pin to high-g interrupt */
                rslt = map_feature_interrupt(int_config, dev);
                if(rslt == BMI160_OK) {
                    /* configure the data source
                     * for high-g interrupt*/
                    rslt = config_high_g_data_src(high_g_int_cfg, dev);
                    if(rslt == BMI160_OK) {
                        rslt = config_high_g_int_settg(high_g_int_cfg, dev);
                    }
                }
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API configures the pins to fire the
 * interrupt signal when it occurs.
 */
static int8_t
    set_intr_pin_config(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;

    /* configure the behavioural settings of interrupt pin */
    rslt = config_int_out_ctrl(int_config, dev);
    if(rslt == BMI160_OK) {
        rslt = config_int_latch(int_config, dev);
    }

    return rslt;
}

/*!
 * @brief This internal API is used to validate the device structure pointer for
 * null conditions.
 */
static int8_t null_ptr_check(const struct bmi160_dev* dev) {
    int8_t rslt;

    if((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_ms == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* Device structure is fine */
        rslt = BMI160_OK;
    }

    return rslt;
}

/*!
 * @brief This API sets the default configuration parameters of accel & gyro.
 * Also maintain the previous state of configurations.
 */
static void default_param_settg(struct bmi160_dev* dev) {
    /* Initializing accel and gyro params with
     * default values */
    dev->accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
    dev->accel_cfg.odr = BMI160_ACCEL_ODR_100HZ;
    dev->accel_cfg.power = BMI160_ACCEL_SUSPEND_MODE;
    dev->accel_cfg.range = BMI160_ACCEL_RANGE_2G;
    dev->gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;
    dev->gyro_cfg.odr = BMI160_GYRO_ODR_100HZ;
    dev->gyro_cfg.power = BMI160_GYRO_SUSPEND_MODE;
    dev->gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;

    /* To maintain the previous state of accel configuration */
    dev->prev_accel_cfg = dev->accel_cfg;

    /* To maintain the previous state of gyro configuration */
    dev->prev_gyro_cfg = dev->gyro_cfg;
}

/*!
 * @brief This API set the accel configuration.
 */
static int8_t set_accel_conf(struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data[2] = {0};

    rslt = check_accel_config(data, dev);
    if(rslt == BMI160_OK) {
        /* Write output data rate and bandwidth */
        rslt = bmi160_set_regs(BMI160_ACCEL_CONFIG_ADDR, &data[0], 1, dev);
        if(rslt == BMI160_OK) {
            dev->prev_accel_cfg.odr = dev->accel_cfg.odr;
            dev->prev_accel_cfg.bw = dev->accel_cfg.bw;

            /* write accel range */
            rslt = bmi160_set_regs(BMI160_ACCEL_RANGE_ADDR, &data[1], 1, dev);
            if(rslt == BMI160_OK) {
                dev->prev_accel_cfg.range = dev->accel_cfg.range;
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API gets the accel configuration.
 */
static int8_t get_accel_conf(struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data[2] = {0};

    /* Get accel configurations */
    rslt = bmi160_get_regs(BMI160_ACCEL_CONFIG_ADDR, data, 2, dev);
    if(rslt == BMI160_OK) {
        dev->accel_cfg.odr = (data[0] & BMI160_ACCEL_ODR_MASK);
        dev->accel_cfg.bw = (data[0] & BMI160_ACCEL_BW_MASK) >> BMI160_ACCEL_BW_POS;
        dev->accel_cfg.range = (data[1] & BMI160_ACCEL_RANGE_MASK);
    }

    return rslt;
}

/*!
 * @brief This API check the accel configuration.
 */
static int8_t check_accel_config(uint8_t* data, const struct bmi160_dev* dev) {
    int8_t rslt;

    /* read accel Output data rate and bandwidth */
    rslt = bmi160_get_regs(BMI160_ACCEL_CONFIG_ADDR, data, 2, dev);
    if(rslt == BMI160_OK) {
        rslt = process_accel_odr(&data[0], dev);
        if(rslt == BMI160_OK) {
            rslt = process_accel_bw(&data[0], dev);
            if(rslt == BMI160_OK) {
                rslt = process_accel_range(&data[1], dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API process the accel odr.
 */
static int8_t process_accel_odr(uint8_t* data, const struct bmi160_dev* dev) {
    int8_t rslt = 0;
    uint8_t temp = 0;
    uint8_t odr = 0;

    if(dev->accel_cfg.odr <= BMI160_ACCEL_ODR_1600HZ) {
        if(dev->accel_cfg.odr != dev->prev_accel_cfg.odr) {
            odr = (uint8_t)dev->accel_cfg.odr;
            temp = *data & ~BMI160_ACCEL_ODR_MASK;

            /* Adding output data rate */
            *data = temp | (odr & BMI160_ACCEL_ODR_MASK);
        }
    } else {
        rslt = BMI160_E_OUT_OF_RANGE;
    }

    return rslt;
}

/*!
 * @brief This API process the accel bandwidth.
 */
static int8_t process_accel_bw(uint8_t* data, const struct bmi160_dev* dev) {
    int8_t rslt = 0;
    uint8_t temp = 0;
    uint8_t bw = 0;

    if(dev->accel_cfg.bw <= BMI160_ACCEL_BW_RES_AVG128) {
        if(dev->accel_cfg.bw != dev->prev_accel_cfg.bw) {
            bw = (uint8_t)dev->accel_cfg.bw;
            temp = *data & ~BMI160_ACCEL_BW_MASK;

            /* Adding bandwidth */
            *data = temp | ((bw << 4) & BMI160_ACCEL_BW_MASK);
        }
    } else {
        rslt = BMI160_E_OUT_OF_RANGE;
    }

    return rslt;
}

/*!
 * @brief This API process the accel range.
 */
static int8_t process_accel_range(uint8_t* data, const struct bmi160_dev* dev) {
    int8_t rslt = 0;
    uint8_t temp = 0;
    uint8_t range = 0;

    if(dev->accel_cfg.range <= BMI160_ACCEL_RANGE_16G) {
        if(dev->accel_cfg.range != dev->prev_accel_cfg.range) {
            range = (uint8_t)dev->accel_cfg.range;
            temp = *data & ~BMI160_ACCEL_RANGE_MASK;

            /* Adding range */
            *data = temp | (range & BMI160_ACCEL_RANGE_MASK);
        }
    } else {
        rslt = BMI160_E_OUT_OF_RANGE;
    }

    return rslt;
}

/*!
 * @brief This API checks the invalid settings for ODR & Bw for
 * Accel and Gyro.
 */
static int8_t check_invalid_settg(const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;

    /* read the error reg */
    rslt = bmi160_get_regs(BMI160_ERROR_REG_ADDR, &data, 1, dev);
    data = data >> 1;
    data = data & BMI160_ERR_REG_MASK;
    if(data == 1) {
        rslt = BMI160_E_ACCEL_ODR_BW_INVALID;
    } else if(data == 2) {
        rslt = BMI160_E_GYRO_ODR_BW_INVALID;
    } else if(data == 3) {
        rslt = BMI160_E_LWP_PRE_FLTR_INT_INVALID;
    } else if(data == 7) {
        rslt = BMI160_E_LWP_PRE_FLTR_INVALID;
    }

    return rslt;
}
static int8_t set_gyro_conf(struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data[2] = {0};

    rslt = check_gyro_config(data, dev);
    if(rslt == BMI160_OK) {
        /* Write output data rate and bandwidth */
        rslt = bmi160_set_regs(BMI160_GYRO_CONFIG_ADDR, &data[0], 1, dev);
        if(rslt == BMI160_OK) {
            dev->prev_gyro_cfg.odr = dev->gyro_cfg.odr;
            dev->prev_gyro_cfg.bw = dev->gyro_cfg.bw;

            /* Write gyro range */
            rslt = bmi160_set_regs(BMI160_GYRO_RANGE_ADDR, &data[1], 1, dev);
            if(rslt == BMI160_OK) {
                dev->prev_gyro_cfg.range = dev->gyro_cfg.range;
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API gets the gyro configuration.
 */
static int8_t get_gyro_conf(struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data[2] = {0};

    /* Get accel configurations */
    rslt = bmi160_get_regs(BMI160_GYRO_CONFIG_ADDR, data, 2, dev);
    if(rslt == BMI160_OK) {
        dev->gyro_cfg.odr = (data[0] & BMI160_GYRO_ODR_MASK);
        dev->gyro_cfg.bw = (data[0] & BMI160_GYRO_BW_MASK) >> BMI160_GYRO_BW_POS;
        dev->gyro_cfg.range = (data[1] & BMI160_GYRO_RANGE_MASK);
    }

    return rslt;
}

/*!
 * @brief This API check the gyro configuration.
 */
static int8_t check_gyro_config(uint8_t* data, const struct bmi160_dev* dev) {
    int8_t rslt;

    /* read gyro Output data rate and bandwidth */
    rslt = bmi160_get_regs(BMI160_GYRO_CONFIG_ADDR, data, 2, dev);
    if(rslt == BMI160_OK) {
        rslt = process_gyro_odr(&data[0], dev);
        if(rslt == BMI160_OK) {
            rslt = process_gyro_bw(&data[0], dev);
            if(rslt == BMI160_OK) {
                rslt = process_gyro_range(&data[1], dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API process the gyro odr.
 */
static int8_t process_gyro_odr(uint8_t* data, const struct bmi160_dev* dev) {
    int8_t rslt = 0;
    uint8_t temp = 0;
    uint8_t odr = 0;

    if(dev->gyro_cfg.odr <= BMI160_GYRO_ODR_3200HZ) {
        if(dev->gyro_cfg.odr != dev->prev_gyro_cfg.odr) {
            odr = (uint8_t)dev->gyro_cfg.odr;
            temp = (*data & ~BMI160_GYRO_ODR_MASK);

            /* Adding output data rate */
            *data = temp | (odr & BMI160_GYRO_ODR_MASK);
        }
    } else {
        rslt = BMI160_E_OUT_OF_RANGE;
    }

    return rslt;
}

/*!
 * @brief This API process the gyro bandwidth.
 */
static int8_t process_gyro_bw(uint8_t* data, const struct bmi160_dev* dev) {
    int8_t rslt = 0;
    uint8_t temp = 0;
    uint8_t bw = 0;

    if(dev->gyro_cfg.bw <= BMI160_GYRO_BW_NORMAL_MODE) {
        bw = (uint8_t)dev->gyro_cfg.bw;
        temp = *data & ~BMI160_GYRO_BW_MASK;

        /* Adding bandwidth */
        *data = temp | ((bw << 4) & BMI160_GYRO_BW_MASK);
    } else {
        rslt = BMI160_E_OUT_OF_RANGE;
    }

    return rslt;
}

/*!
 * @brief This API process the gyro range.
 */
static int8_t process_gyro_range(uint8_t* data, const struct bmi160_dev* dev) {
    int8_t rslt = 0;
    uint8_t temp = 0;
    uint8_t range = 0;

    if(dev->gyro_cfg.range <= BMI160_GYRO_RANGE_125_DPS) {
        if(dev->gyro_cfg.range != dev->prev_gyro_cfg.range) {
            range = (uint8_t)dev->gyro_cfg.range;
            temp = *data & ~BMI160_GYRO_RANGE_MASK;

            /* Adding range */
            *data = temp | (range & BMI160_GYRO_RANGE_MASK);
        }
    } else {
        rslt = BMI160_E_OUT_OF_RANGE;
    }

    return rslt;
}

/*!
 * @brief This API sets the accel power.
 */
static int8_t set_accel_pwr(struct bmi160_dev* dev) {
    int8_t rslt = 0;
    uint8_t data = 0;

    if((dev->accel_cfg.power >= BMI160_ACCEL_SUSPEND_MODE) &&
       (dev->accel_cfg.power <= BMI160_ACCEL_LOWPOWER_MODE)) {
        if(dev->accel_cfg.power != dev->prev_accel_cfg.power) {
            rslt = process_under_sampling(&data, dev);
            if(rslt == BMI160_OK) {
                /* Write accel power */
                rslt = bmi160_set_regs(BMI160_COMMAND_REG_ADDR, &dev->accel_cfg.power, 1, dev);

                /* Add delay of 3.8 ms - refer data sheet table 24*/
                if(dev->prev_accel_cfg.power == BMI160_ACCEL_SUSPEND_MODE) {
                    dev->delay_ms(BMI160_ACCEL_DELAY_MS);
                }

                dev->prev_accel_cfg.power = dev->accel_cfg.power;
            }
        }
    } else {
        rslt = BMI160_E_INVALID_CONFIG;
    }

    return rslt;
}

/*!
 * @brief This API process the undersampling setting of Accel.
 */
static int8_t process_under_sampling(uint8_t* data, const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t temp = 0;
    uint8_t pre_filter[2] = {0};

    rslt = bmi160_get_regs(BMI160_ACCEL_CONFIG_ADDR, data, 1, dev);
    if(rslt == BMI160_OK) {
        if(dev->accel_cfg.power == BMI160_ACCEL_LOWPOWER_MODE) {
            temp = *data & ~BMI160_ACCEL_UNDERSAMPLING_MASK;

            /* Set under-sampling parameter */
            *data = temp | ((1 << 7) & BMI160_ACCEL_UNDERSAMPLING_MASK);

            /* Write data */
            rslt = bmi160_set_regs(BMI160_ACCEL_CONFIG_ADDR, data, 1, dev);

            /* Disable the pre-filter data in low power mode */
            if(rslt == BMI160_OK) {
                /* Disable the Pre-filter data*/
                rslt = bmi160_set_regs(BMI160_INT_DATA_0_ADDR, pre_filter, 2, dev);
            }
        } else if(*data & BMI160_ACCEL_UNDERSAMPLING_MASK) {
            temp = *data & ~BMI160_ACCEL_UNDERSAMPLING_MASK;

            /* Disable under-sampling parameter if already enabled */
            *data = temp;

            /* Write data */
            rslt = bmi160_set_regs(BMI160_ACCEL_CONFIG_ADDR, data, 1, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the gyro power mode.
 */
static int8_t set_gyro_pwr(struct bmi160_dev* dev) {
    int8_t rslt = 0;

    if((dev->gyro_cfg.power == BMI160_GYRO_SUSPEND_MODE) ||
       (dev->gyro_cfg.power == BMI160_GYRO_NORMAL_MODE) ||
       (dev->gyro_cfg.power == BMI160_GYRO_FASTSTARTUP_MODE)) {
        if(dev->gyro_cfg.power != dev->prev_gyro_cfg.power) {
            /* Write gyro power */
            rslt = bmi160_set_regs(BMI160_COMMAND_REG_ADDR, &dev->gyro_cfg.power, 1, dev);
            if(dev->prev_gyro_cfg.power == BMI160_GYRO_SUSPEND_MODE) {
                /* Delay of 80 ms - datasheet Table 24 */
                dev->delay_ms(BMI160_GYRO_DELAY_MS);
            } else if(
                (dev->prev_gyro_cfg.power == BMI160_GYRO_FASTSTARTUP_MODE) &&
                (dev->gyro_cfg.power == BMI160_GYRO_NORMAL_MODE)) {
                /* This delay is required for transition from
                 * fast-startup mode to normal mode - datasheet Table 3 */
                dev->delay_ms(10);
            } else {
                /* do nothing */
            }

            dev->prev_gyro_cfg.power = dev->gyro_cfg.power;
        }
    } else {
        rslt = BMI160_E_INVALID_CONFIG;
    }

    return rslt;
}

/*!
 * @brief This API reads accel data along with sensor time if time is requested
 * by user. Kindly refer the user guide(README.md) for more info.
 */
static int8_t
    get_accel_data(uint8_t len, struct bmi160_sensor_data* accel, const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t idx = 0;
    uint8_t data_array[9] = {0};
    uint8_t time_0 = 0;
    uint16_t time_1 = 0;
    uint32_t time_2 = 0;
    uint8_t lsb;
    uint8_t msb;
    int16_t msblsb;

    /* read accel sensor data along with time if requested */
    rslt = bmi160_get_regs(BMI160_ACCEL_DATA_ADDR, data_array, 6 + len, dev);
    if(rslt == BMI160_OK) {
        /* Accel Data */
        lsb = data_array[idx++];
        msb = data_array[idx++];
        msblsb = (int16_t)((msb << 8) | lsb);
        accel->x = msblsb; /* Data in X axis */
        lsb = data_array[idx++];
        msb = data_array[idx++];
        msblsb = (int16_t)((msb << 8) | lsb);
        accel->y = msblsb; /* Data in Y axis */
        lsb = data_array[idx++];
        msb = data_array[idx++];
        msblsb = (int16_t)((msb << 8) | lsb);
        accel->z = msblsb; /* Data in Z axis */
        if(len == 3) {
            time_0 = data_array[idx++];
            time_1 = (uint16_t)(data_array[idx++] << 8);
            time_2 = (uint32_t)(data_array[idx++] << 16);
            accel->sensortime = (uint32_t)(time_2 | time_1 | time_0);
        } else {
            accel->sensortime = 0;
        }
    } else {
        rslt = BMI160_E_COM_FAIL;
    }

    return rslt;
}

/*!
 * @brief This API reads accel data along with sensor time if time is requested
 * by user. Kindly refer the user guide(README.md) for more info.
 */
static int8_t
    get_gyro_data(uint8_t len, struct bmi160_sensor_data* gyro, const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t idx = 0;
    uint8_t data_array[15] = {0};
    uint8_t time_0 = 0;
    uint16_t time_1 = 0;
    uint32_t time_2 = 0;
    uint8_t lsb;
    uint8_t msb;
    int16_t msblsb;

    if(len == 0) {
        /* read gyro data only */
        rslt = bmi160_get_regs(BMI160_GYRO_DATA_ADDR, data_array, 6, dev);
        if(rslt == BMI160_OK) {
            /* Gyro Data */
            lsb = data_array[idx++];
            msb = data_array[idx++];
            msblsb = (int16_t)((msb << 8) | lsb);
            gyro->x = msblsb; /* Data in X axis */
            lsb = data_array[idx++];
            msb = data_array[idx++];
            msblsb = (int16_t)((msb << 8) | lsb);
            gyro->y = msblsb; /* Data in Y axis */
            lsb = data_array[idx++];
            msb = data_array[idx++];
            msblsb = (int16_t)((msb << 8) | lsb);
            gyro->z = msblsb; /* Data in Z axis */
            gyro->sensortime = 0;
        } else {
            rslt = BMI160_E_COM_FAIL;
        }
    } else {
        /* read gyro sensor data along with time */
        rslt = bmi160_get_regs(BMI160_GYRO_DATA_ADDR, data_array, 12 + len, dev);
        if(rslt == BMI160_OK) {
            /* Gyro Data */
            lsb = data_array[idx++];
            msb = data_array[idx++];
            msblsb = (int16_t)((msb << 8) | lsb);
            gyro->x = msblsb; /* gyro X axis data */
            lsb = data_array[idx++];
            msb = data_array[idx++];
            msblsb = (int16_t)((msb << 8) | lsb);
            gyro->y = msblsb; /* gyro Y axis data */
            lsb = data_array[idx++];
            msb = data_array[idx++];
            msblsb = (int16_t)((msb << 8) | lsb);
            gyro->z = msblsb; /* gyro Z axis data */
            idx = idx + 6;
            time_0 = data_array[idx++];
            time_1 = (uint16_t)(data_array[idx++] << 8);
            time_2 = (uint32_t)(data_array[idx++] << 16);
            gyro->sensortime = (uint32_t)(time_2 | time_1 | time_0);
        } else {
            rslt = BMI160_E_COM_FAIL;
        }
    }

    return rslt;
}

/*!
 * @brief This API reads accel and gyro data along with sensor time
 * if time is requested by user.
 *  Kindly refer the user guide(README.md) for more info.
 */
static int8_t get_accel_gyro_data(
    uint8_t len,
    struct bmi160_sensor_data* accel,
    struct bmi160_sensor_data* gyro,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t idx = 0;
    uint8_t data_array[15] = {0};
    uint8_t time_0 = 0;
    uint16_t time_1 = 0;
    uint32_t time_2 = 0;
    uint8_t lsb;
    uint8_t msb;
    int16_t msblsb;

    /* read both accel and gyro sensor data
     * along with time if requested */
    rslt = bmi160_get_regs(BMI160_GYRO_DATA_ADDR, data_array, 12 + len, dev);
    if(rslt == BMI160_OK) {
        /* Gyro Data */
        lsb = data_array[idx++];
        msb = data_array[idx++];
        msblsb = (int16_t)((msb << 8) | lsb);
        gyro->x = msblsb; /* gyro X axis data */
        lsb = data_array[idx++];
        msb = data_array[idx++];
        msblsb = (int16_t)((msb << 8) | lsb);
        gyro->y = msblsb; /* gyro Y axis data */
        lsb = data_array[idx++];
        msb = data_array[idx++];
        msblsb = (int16_t)((msb << 8) | lsb);
        gyro->z = msblsb; /* gyro Z axis data */
        /* Accel Data */
        lsb = data_array[idx++];
        msb = data_array[idx++];
        msblsb = (int16_t)((msb << 8) | lsb);
        accel->x = (int16_t)msblsb; /* accel X axis data */
        lsb = data_array[idx++];
        msb = data_array[idx++];
        msblsb = (int16_t)((msb << 8) | lsb);
        accel->y = (int16_t)msblsb; /* accel Y axis data */
        lsb = data_array[idx++];
        msb = data_array[idx++];
        msblsb = (int16_t)((msb << 8) | lsb);
        accel->z = (int16_t)msblsb; /* accel Z axis data */
        if(len == 3) {
            time_0 = data_array[idx++];
            time_1 = (uint16_t)(data_array[idx++] << 8);
            time_2 = (uint32_t)(data_array[idx++] << 16);
            accel->sensortime = (uint32_t)(time_2 | time_1 | time_0);
            gyro->sensortime = (uint32_t)(time_2 | time_1 | time_0);
        } else {
            accel->sensortime = 0;
            gyro->sensortime = 0;
        }
    } else {
        rslt = BMI160_E_COM_FAIL;
    }

    return rslt;
}

/*!
 * @brief This API enables the any-motion interrupt for accel.
 */
static int8_t enable_accel_any_motion_int(
    const struct bmi160_acc_any_mot_int_cfg* any_motion_int_cfg,
    struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Enable any motion x, any motion y, any motion z
     * in Int Enable 0 register */
    rslt = bmi160_get_regs(BMI160_INT_ENABLE_0_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        if(any_motion_int_cfg->anymotion_en == BMI160_ENABLE) {
            temp = data & ~BMI160_ANY_MOTION_X_INT_EN_MASK;

            /* Adding Any_motion x axis */
            data = temp | (any_motion_int_cfg->anymotion_x & BMI160_ANY_MOTION_X_INT_EN_MASK);
            temp = data & ~BMI160_ANY_MOTION_Y_INT_EN_MASK;

            /* Adding Any_motion y axis */
            data = temp |
                   ((any_motion_int_cfg->anymotion_y << 1) & BMI160_ANY_MOTION_Y_INT_EN_MASK);
            temp = data & ~BMI160_ANY_MOTION_Z_INT_EN_MASK;

            /* Adding Any_motion z axis */
            data = temp |
                   ((any_motion_int_cfg->anymotion_z << 2) & BMI160_ANY_MOTION_Z_INT_EN_MASK);

            /* any-motion feature selected*/
            dev->any_sig_sel = BMI160_ANY_MOTION_ENABLED;
        } else {
            data = data & ~BMI160_ANY_MOTION_ALL_INT_EN_MASK;

            /* neither any-motion feature nor sig-motion selected */
            dev->any_sig_sel = BMI160_BOTH_ANY_SIG_MOTION_DISABLED;
        }

        /* write data to Int Enable 0 register */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_0_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API disable the sig-motion interrupt.
 */
static int8_t disable_sig_motion_int(const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Disabling Significant motion interrupt if enabled */
    rslt = bmi160_get_regs(BMI160_INT_MOTION_3_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = (data & BMI160_SIG_MOTION_SEL_MASK);
        if(temp) {
            temp = data & ~BMI160_SIG_MOTION_SEL_MASK;
            data = temp;

            /* Write data to register */
            rslt = bmi160_set_regs(BMI160_INT_MOTION_3_ADDR, &data, 1, dev);
        }
    }

    return rslt;
}

/*!
 *  @brief This API is used to map/unmap the Any/Sig motion, Step det/Low-g,
 *  Double tap, Single tap, Orientation, Flat, High-G, Nomotion interrupt pins.
 */
static int8_t
    map_feature_interrupt(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data[3] = {0, 0, 0};
    uint8_t temp[3] = {0, 0, 0};

    rslt = bmi160_get_regs(BMI160_INT_MAP_0_ADDR, data, 3, dev);
    if(rslt == BMI160_OK) {
        temp[0] = data[0] & ~int_mask_lookup_table[int_config->int_type];
        temp[2] = data[2] & ~int_mask_lookup_table[int_config->int_type];
        switch(int_config->int_channel) {
        case BMI160_INT_CHANNEL_NONE:
            data[0] = temp[0];
            data[2] = temp[2];
            break;
        case BMI160_INT_CHANNEL_1:
            data[0] = temp[0] | int_mask_lookup_table[int_config->int_type];
            data[2] = temp[2];
            break;
        case BMI160_INT_CHANNEL_2:
            data[2] = temp[2] | int_mask_lookup_table[int_config->int_type];
            data[0] = temp[0];
            break;
        case BMI160_INT_CHANNEL_BOTH:
            data[0] = temp[0] | int_mask_lookup_table[int_config->int_type];
            data[2] = temp[2] | int_mask_lookup_table[int_config->int_type];
            break;
        default:
            rslt = BMI160_E_OUT_OF_RANGE;
        }
        if(rslt == BMI160_OK) {
            rslt = bmi160_set_regs(BMI160_INT_MAP_0_ADDR, data, 3, dev);
        }
    }

    return rslt;
}

/*!
 *  @brief This API is used to map/unmap the Dataready(Accel & Gyro), FIFO full
 *  and FIFO watermark interrupt.
 */
static int8_t map_hardware_interrupt(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    rslt = bmi160_get_regs(BMI160_INT_MAP_1_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~int_mask_lookup_table[int_config->int_type];
        temp = temp & ~((uint8_t)(int_mask_lookup_table[int_config->int_type] << 4));
        switch(int_config->int_channel) {
        case BMI160_INT_CHANNEL_NONE:
            data = temp;
            break;
        case BMI160_INT_CHANNEL_1:
            data = temp | (uint8_t)((int_mask_lookup_table[int_config->int_type]) << 4);
            break;
        case BMI160_INT_CHANNEL_2:
            data = temp | int_mask_lookup_table[int_config->int_type];
            break;
        case BMI160_INT_CHANNEL_BOTH:
            data = temp | int_mask_lookup_table[int_config->int_type];
            data = data | (uint8_t)((int_mask_lookup_table[int_config->int_type]) << 4);
            break;
        default:
            rslt = BMI160_E_OUT_OF_RANGE;
        }
        if(rslt == BMI160_OK) {
            rslt = bmi160_set_regs(BMI160_INT_MAP_1_ADDR, &data, 1, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API configure the source of data(filter & pre-filter)
 * for any-motion interrupt.
 */
static int8_t config_any_motion_src(
    const struct bmi160_acc_any_mot_int_cfg* any_motion_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Configure Int data 1 register to add source of interrupt */
    rslt = bmi160_get_regs(BMI160_INT_DATA_1_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_MOTION_SRC_INT_MASK;
        data = temp | ((any_motion_int_cfg->anymotion_data_src << 7) & BMI160_MOTION_SRC_INT_MASK);

        /* Write data to DATA 1 address */
        rslt = bmi160_set_regs(BMI160_INT_DATA_1_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the duration and threshold of
 * any-motion interrupt.
 */
static int8_t config_any_dur_threshold(
    const struct bmi160_acc_any_mot_int_cfg* any_motion_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;
    uint8_t data_array[2] = {0};
    uint8_t dur;

    /* Configure Int Motion 0 register */
    rslt = bmi160_get_regs(BMI160_INT_MOTION_0_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        /* slope duration */
        dur = (uint8_t)any_motion_int_cfg->anymotion_dur;
        temp = data & ~BMI160_SLOPE_INT_DUR_MASK;
        data = temp | (dur & BMI160_MOTION_SRC_INT_MASK);
        data_array[0] = data;

        /* add slope threshold */
        data_array[1] = any_motion_int_cfg->anymotion_thr;

        /* INT MOTION 0 and INT MOTION 1 address lie consecutively,
         * hence writing data to respective registers at one go */

        /* Writing to Int_motion 0 and
         * Int_motion 1 Address simultaneously */
        rslt = bmi160_set_regs(BMI160_INT_MOTION_0_ADDR, data_array, 2, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure necessary setting of any-motion interrupt.
 */
static int8_t config_any_motion_int_settg(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_any_mot_int_cfg* any_motion_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Configure Interrupt pins */
    rslt = set_intr_pin_config(int_config, dev);
    if(rslt == BMI160_OK) {
        rslt = disable_sig_motion_int(dev);
        if(rslt == BMI160_OK) {
            rslt = map_feature_interrupt(int_config, dev);
            if(rslt == BMI160_OK) {
                rslt = config_any_motion_src(any_motion_int_cfg, dev);
                if(rslt == BMI160_OK) {
                    rslt = config_any_dur_threshold(any_motion_int_cfg, dev);
                }
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API enable the data ready interrupt.
 */
static int8_t enable_data_ready_int(const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Enable data ready interrupt in Int Enable 1 register */
    rslt = bmi160_get_regs(BMI160_INT_ENABLE_1_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_DATA_RDY_INT_EN_MASK;
        data = temp | ((1 << 4) & BMI160_DATA_RDY_INT_EN_MASK);

        /* Writing data to INT ENABLE 1 Address */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_1_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API enables the no motion/slow motion interrupt.
 */
static int8_t enable_no_motion_int(
    const struct bmi160_acc_no_motion_int_cfg* no_mot_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Enable no motion x, no motion y, no motion z
     * in Int Enable 2 register */
    rslt = bmi160_get_regs(BMI160_INT_ENABLE_2_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        if(no_mot_int_cfg->no_motion_x == 1) {
            temp = data & ~BMI160_NO_MOTION_X_INT_EN_MASK;

            /* Adding No_motion x axis */
            data = temp | (1 & BMI160_NO_MOTION_X_INT_EN_MASK);
        }

        if(no_mot_int_cfg->no_motion_y == 1) {
            temp = data & ~BMI160_NO_MOTION_Y_INT_EN_MASK;

            /* Adding No_motion x axis */
            data = temp | ((1 << 1) & BMI160_NO_MOTION_Y_INT_EN_MASK);
        }

        if(no_mot_int_cfg->no_motion_z == 1) {
            temp = data & ~BMI160_NO_MOTION_Z_INT_EN_MASK;

            /* Adding No_motion x axis */
            data = temp | ((1 << 2) & BMI160_NO_MOTION_Z_INT_EN_MASK);
        }

        /* write data to Int Enable 2 register */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_2_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the interrupt PIN setting for
 * no motion/slow motion interrupt.
 */
static int8_t config_no_motion_int_settg(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_no_motion_int_cfg* no_mot_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Configure Interrupt pins */
    rslt = set_intr_pin_config(int_config, dev);
    if(rslt == BMI160_OK) {
        rslt = map_feature_interrupt(int_config, dev);
        if(rslt == BMI160_OK) {
            rslt = config_no_motion_data_src(no_mot_int_cfg, dev);
            if(rslt == BMI160_OK) {
                rslt = config_no_motion_dur_thr(no_mot_int_cfg, dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API configure the source of interrupt for no motion.
 */
static int8_t config_no_motion_data_src(
    const struct bmi160_acc_no_motion_int_cfg* no_mot_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Configure Int data 1 register to add source of interrupt */
    rslt = bmi160_get_regs(BMI160_INT_DATA_1_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_MOTION_SRC_INT_MASK;
        data = temp | ((no_mot_int_cfg->no_motion_src << 7) & BMI160_MOTION_SRC_INT_MASK);

        /* Write data to DATA 1 address */
        rslt = bmi160_set_regs(BMI160_INT_DATA_1_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the duration and threshold of
 * no motion/slow motion interrupt along with selection of no/slow motion.
 */
static int8_t config_no_motion_dur_thr(
    const struct bmi160_acc_no_motion_int_cfg* no_mot_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;
    uint8_t temp_1 = 0;
    uint8_t reg_addr;
    uint8_t data_array[2] = {0};

    /* Configuring INT_MOTION register */
    reg_addr = BMI160_INT_MOTION_0_ADDR;
    rslt = bmi160_get_regs(reg_addr, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_NO_MOTION_INT_DUR_MASK;

        /* Adding no_motion duration */
        data = temp | ((no_mot_int_cfg->no_motion_dur << 2) & BMI160_NO_MOTION_INT_DUR_MASK);

        /* Write data to NO_MOTION 0 address */
        rslt = bmi160_set_regs(reg_addr, &data, 1, dev);
        if(rslt == BMI160_OK) {
            reg_addr = BMI160_INT_MOTION_3_ADDR;
            rslt = bmi160_get_regs(reg_addr, &data, 1, dev);
            if(rslt == BMI160_OK) {
                temp = data & ~BMI160_NO_MOTION_SEL_BIT_MASK;

                /* Adding no_motion_sel bit */
                temp_1 = (no_mot_int_cfg->no_motion_sel & BMI160_NO_MOTION_SEL_BIT_MASK);
                data = (temp | temp_1);
                data_array[1] = data;

                /* Adding no motion threshold */
                data_array[0] = no_mot_int_cfg->no_motion_thres;
                reg_addr = BMI160_INT_MOTION_2_ADDR;

                /* writing data to INT_MOTION 2 and INT_MOTION 3
                 * address simultaneously */
                rslt = bmi160_set_regs(reg_addr, data_array, 2, dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API enables the sig-motion motion interrupt.
 */
static int8_t enable_sig_motion_int(
    const struct bmi160_acc_sig_mot_int_cfg* sig_mot_int_cfg,
    struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* For significant motion,enable any motion x,any motion y,
     * any motion z in Int Enable 0 register */
    rslt = bmi160_get_regs(BMI160_INT_ENABLE_0_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        if(sig_mot_int_cfg->sig_en == BMI160_ENABLE) {
            temp = data & ~BMI160_SIG_MOTION_INT_EN_MASK;
            data = temp | (7 & BMI160_SIG_MOTION_INT_EN_MASK);

            /* sig-motion feature selected*/
            dev->any_sig_sel = BMI160_SIG_MOTION_ENABLED;
        } else {
            data = data & ~BMI160_SIG_MOTION_INT_EN_MASK;

            /* neither any-motion feature nor sig-motion selected */
            dev->any_sig_sel = BMI160_BOTH_ANY_SIG_MOTION_DISABLED;
        }

        /* write data to Int Enable 0 register */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_0_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the interrupt PIN setting for
 * significant motion interrupt.
 */
static int8_t config_sig_motion_int_settg(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_sig_mot_int_cfg* sig_mot_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Configure Interrupt pins */
    rslt = set_intr_pin_config(int_config, dev);
    if(rslt == BMI160_OK) {
        rslt = map_feature_interrupt(int_config, dev);
        if(rslt == BMI160_OK) {
            rslt = config_sig_motion_data_src(sig_mot_int_cfg, dev);
            if(rslt == BMI160_OK) {
                rslt = config_sig_dur_threshold(sig_mot_int_cfg, dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API configure the source of data(filter & pre-filter)
 * for sig motion interrupt.
 */
static int8_t config_sig_motion_data_src(
    const struct bmi160_acc_sig_mot_int_cfg* sig_mot_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Configure Int data 1 register to add source of interrupt */
    rslt = bmi160_get_regs(BMI160_INT_DATA_1_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_MOTION_SRC_INT_MASK;
        data = temp | ((sig_mot_int_cfg->sig_data_src << 7) & BMI160_MOTION_SRC_INT_MASK);

        /* Write data to DATA 1 address */
        rslt = bmi160_set_regs(BMI160_INT_DATA_1_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the threshold, skip and proof time of
 * sig motion interrupt.
 */
static int8_t config_sig_dur_threshold(
    const struct bmi160_acc_sig_mot_int_cfg* sig_mot_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data;
    uint8_t temp = 0;

    /* Configuring INT_MOTION registers */

    /* Write significant motion threshold.
     * This threshold is same as any motion threshold */
    data = sig_mot_int_cfg->sig_mot_thres;

    /* Write data to INT_MOTION 1 address */
    rslt = bmi160_set_regs(BMI160_INT_MOTION_1_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        rslt = bmi160_get_regs(BMI160_INT_MOTION_3_ADDR, &data, 1, dev);
        if(rslt == BMI160_OK) {
            temp = data & ~BMI160_SIG_MOTION_SKIP_MASK;

            /* adding skip time of sig_motion interrupt*/
            data = temp | ((sig_mot_int_cfg->sig_mot_skip << 2) & BMI160_SIG_MOTION_SKIP_MASK);
            temp = data & ~BMI160_SIG_MOTION_PROOF_MASK;

            /* adding proof time of sig_motion interrupt */
            data = temp | ((sig_mot_int_cfg->sig_mot_proof << 4) & BMI160_SIG_MOTION_PROOF_MASK);

            /* configure the int_sig_mot_sel bit to select
             * significant motion interrupt */
            temp = data & ~BMI160_SIG_MOTION_SEL_MASK;
            data = temp | ((sig_mot_int_cfg->sig_en << 1) & BMI160_SIG_MOTION_SEL_MASK);
            rslt = bmi160_set_regs(BMI160_INT_MOTION_3_ADDR, &data, 1, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API enables the step detector interrupt.
 */
static int8_t enable_step_detect_int(
    const struct bmi160_acc_step_detect_int_cfg* step_detect_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Enable data ready interrupt in Int Enable 2 register */
    rslt = bmi160_get_regs(BMI160_INT_ENABLE_2_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_STEP_DETECT_INT_EN_MASK;
        data = temp |
               ((step_detect_int_cfg->step_detector_en << 3) & BMI160_STEP_DETECT_INT_EN_MASK);

        /* Writing data to INT ENABLE 2 Address */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_2_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the step detector parameter.
 */
static int8_t config_step_detect(
    const struct bmi160_acc_step_detect_int_cfg* step_detect_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t temp = 0;
    uint8_t data_array[2] = {0};

    if(step_detect_int_cfg->step_detector_mode == BMI160_STEP_DETECT_NORMAL) {
        /* Normal mode setting */
        data_array[0] = 0x15;
        data_array[1] = 0x03;
    } else if(step_detect_int_cfg->step_detector_mode == BMI160_STEP_DETECT_SENSITIVE) {
        /* Sensitive mode setting */
        data_array[0] = 0x2D;
        data_array[1] = 0x00;
    } else if(step_detect_int_cfg->step_detector_mode == BMI160_STEP_DETECT_ROBUST) {
        /* Robust mode setting */
        data_array[0] = 0x1D;
        data_array[1] = 0x07;
    } else if(step_detect_int_cfg->step_detector_mode == BMI160_STEP_DETECT_USER_DEFINE) {
        /* Non recommended User defined setting */
        /* Configuring STEP_CONFIG register */
        rslt = bmi160_get_regs(BMI160_INT_STEP_CONFIG_0_ADDR, &data_array[0], 2, dev);
        if(rslt == BMI160_OK) {
            temp = data_array[0] & ~BMI160_STEP_DETECT_MIN_THRES_MASK;

            /* Adding min_threshold */
            data_array[0] = temp | ((step_detect_int_cfg->min_threshold << 3) &
                                    BMI160_STEP_DETECT_MIN_THRES_MASK);
            temp = data_array[0] & ~BMI160_STEP_DETECT_STEPTIME_MIN_MASK;

            /* Adding steptime_min */
            data_array[0] = temp | ((step_detect_int_cfg->steptime_min) &
                                    BMI160_STEP_DETECT_STEPTIME_MIN_MASK);
            temp = data_array[1] & ~BMI160_STEP_MIN_BUF_MASK;

            /* Adding steptime_min */
            data_array[1] = temp |
                            ((step_detect_int_cfg->step_min_buf) & BMI160_STEP_MIN_BUF_MASK);
        }
    }

    /* Write data to STEP_CONFIG register */
    rslt = bmi160_set_regs(BMI160_INT_STEP_CONFIG_0_ADDR, data_array, 2, dev);

    return rslt;
}

/*!
 * @brief This API enables the single/double tap interrupt.
 */
static int8_t enable_tap_int(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_tap_int_cfg* tap_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Enable single tap or double tap interrupt in Int Enable 0 register */
    rslt = bmi160_get_regs(BMI160_INT_ENABLE_0_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        if(int_config->int_type == BMI160_ACC_SINGLE_TAP_INT) {
            temp = data & ~BMI160_SINGLE_TAP_INT_EN_MASK;
            data = temp | ((tap_int_cfg->tap_en << 5) & BMI160_SINGLE_TAP_INT_EN_MASK);
        } else {
            temp = data & ~BMI160_DOUBLE_TAP_INT_EN_MASK;
            data = temp | ((tap_int_cfg->tap_en << 4) & BMI160_DOUBLE_TAP_INT_EN_MASK);
        }

        /* Write to Enable 0 Address */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_0_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the interrupt PIN setting for
 * tap interrupt.
 */
static int8_t config_tap_int_settg(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_tap_int_cfg* tap_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Configure Interrupt pins */
    rslt = set_intr_pin_config(int_config, dev);
    if(rslt == BMI160_OK) {
        rslt = map_feature_interrupt(int_config, dev);
        if(rslt == BMI160_OK) {
            rslt = config_tap_data_src(tap_int_cfg, dev);
            if(rslt == BMI160_OK) {
                rslt = config_tap_param(int_config, tap_int_cfg, dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API configure the source of data(filter & pre-filter)
 * for tap interrupt.
 */
static int8_t config_tap_data_src(
    const struct bmi160_acc_tap_int_cfg* tap_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Configure Int data 0 register to add source of interrupt */
    rslt = bmi160_get_regs(BMI160_INT_DATA_0_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_TAP_SRC_INT_MASK;
        data = temp | ((tap_int_cfg->tap_data_src << 3) & BMI160_TAP_SRC_INT_MASK);

        /* Write data to Data 0 address */
        rslt = bmi160_set_regs(BMI160_INT_DATA_0_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the  parameters of tap interrupt.
 * Threshold, quite, shock, and duration.
 */
static int8_t config_tap_param(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_acc_tap_int_cfg* tap_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t temp = 0;
    uint8_t data = 0;
    uint8_t data_array[2] = {0};
    uint8_t count = 0;
    uint8_t dur, shock, quiet, thres;

    /* Configure tap 0 register for tap shock,tap quiet duration
     * in case of single tap interrupt */
    rslt = bmi160_get_regs(BMI160_INT_TAP_0_ADDR, data_array, 2, dev);
    if(rslt == BMI160_OK) {
        data = data_array[count];
        if(int_config->int_type == BMI160_ACC_DOUBLE_TAP_INT) {
            dur = (uint8_t)tap_int_cfg->tap_dur;
            temp = (data & ~BMI160_TAP_DUR_MASK);

            /* Add tap duration data in case of
             * double tap interrupt */
            data = temp | (dur & BMI160_TAP_DUR_MASK);
        }

        shock = (uint8_t)tap_int_cfg->tap_shock;
        temp = data & ~BMI160_TAP_SHOCK_DUR_MASK;
        data = temp | ((shock << 6) & BMI160_TAP_SHOCK_DUR_MASK);
        quiet = (uint8_t)tap_int_cfg->tap_quiet;
        temp = data & ~BMI160_TAP_QUIET_DUR_MASK;
        data = temp | ((quiet << 7) & BMI160_TAP_QUIET_DUR_MASK);
        data_array[count++] = data;
        data = data_array[count];
        thres = (uint8_t)tap_int_cfg->tap_thr;
        temp = data & ~BMI160_TAP_THRES_MASK;
        data = temp | (thres & BMI160_TAP_THRES_MASK);
        data_array[count++] = data;

        /* TAP 0 and TAP 1 address lie consecutively,
         * hence writing data to respective registers at one go */

        /* Writing to Tap 0 and Tap 1 Address simultaneously */
        rslt = bmi160_set_regs(BMI160_INT_TAP_0_ADDR, data_array, count, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the secondary interface.
 */
static int8_t config_sec_if(const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t if_conf = 0;
    uint8_t cmd = BMI160_AUX_NORMAL_MODE;

    /* set the aux power mode to normal*/
    rslt = bmi160_set_regs(BMI160_COMMAND_REG_ADDR, &cmd, 1, dev);
    if(rslt == BMI160_OK) {
        /* 0.5ms delay - refer datasheet table 24*/
        dev->delay_ms(1);
        rslt = bmi160_get_regs(BMI160_IF_CONF_ADDR, &if_conf, 1, dev);
        if_conf |= (uint8_t)(1 << 5);
        if(rslt == BMI160_OK) {
            /*enable the secondary interface also*/
            rslt = bmi160_set_regs(BMI160_IF_CONF_ADDR, &if_conf, 1, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API configure the ODR of the auxiliary sensor.
 */
static int8_t config_aux_odr(const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t aux_odr;

    rslt = bmi160_get_regs(BMI160_AUX_ODR_ADDR, &aux_odr, 1, dev);
    if(rslt == BMI160_OK) {
        aux_odr = (uint8_t)(dev->aux_cfg.aux_odr);

        /* Set the secondary interface ODR
         * i.e polling rate of secondary sensor */
        rslt = bmi160_set_regs(BMI160_AUX_ODR_ADDR, &aux_odr, 1, dev);
        dev->delay_ms(BMI160_AUX_COM_DELAY);
    }

    return rslt;
}

/*!
 * @brief This API maps the actual burst read length set by user.
 */
static int8_t map_read_len(uint16_t* len, const struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;

    switch(dev->aux_cfg.aux_rd_burst_len) {
    case BMI160_AUX_READ_LEN_0:
        *len = 1;
        break;
    case BMI160_AUX_READ_LEN_1:
        *len = 2;
        break;
    case BMI160_AUX_READ_LEN_2:
        *len = 6;
        break;
    case BMI160_AUX_READ_LEN_3:
        *len = 8;
        break;
    default:
        rslt = BMI160_E_INVALID_INPUT;
        break;
    }

    return rslt;
}

/*!
 * @brief This API configure the settings of auxiliary sensor.
 */
static int8_t config_aux_settg(const struct bmi160_dev* dev) {
    int8_t rslt;

    rslt = config_sec_if(dev);
    if(rslt == BMI160_OK) {
        /* Configures the auxiliary interface settings */
        rslt = bmi160_config_aux_mode(dev);
    }

    return rslt;
}

/*!
 * @brief This API extract the read data from auxiliary sensor.
 */
static int8_t extract_aux_read(
    uint16_t map_len,
    uint8_t reg_addr,
    uint8_t* aux_data,
    uint16_t len,
    const struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;
    uint8_t data[8] = {
        0,
    };
    uint8_t read_addr = BMI160_AUX_DATA_ADDR;
    uint8_t count = 0;
    uint8_t read_count;
    uint8_t read_len = (uint8_t)map_len;

    for(; count < len;) {
        /* set address to read */
        rslt = bmi160_set_regs(BMI160_AUX_IF_2_ADDR, &reg_addr, 1, dev);
        dev->delay_ms(BMI160_AUX_COM_DELAY);
        if(rslt == BMI160_OK) {
            rslt = bmi160_get_regs(read_addr, data, map_len, dev);
            if(rslt == BMI160_OK) {
                read_count = 0;

                /* if read len is less the burst read len
                 * mention by user*/
                if(len < map_len) {
                    read_len = (uint8_t)len;
                } else if((len - count) < map_len) {
                    read_len = (uint8_t)(len - count);
                }

                for(; read_count < read_len; read_count++) {
                    aux_data[count + read_count] = data[read_count];
                }

                reg_addr += (uint8_t)map_len;
                count += (uint8_t)map_len;
            } else {
                rslt = BMI160_E_COM_FAIL;
                break;
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API enables the orient interrupt.
 */
static int8_t enable_orient_int(
    const struct bmi160_acc_orient_int_cfg* orient_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Enable data ready interrupt in Int Enable 0 register */
    rslt = bmi160_get_regs(BMI160_INT_ENABLE_0_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_ORIENT_INT_EN_MASK;
        data = temp | ((orient_int_cfg->orient_en << 6) & BMI160_ORIENT_INT_EN_MASK);

        /* write data to Int Enable 0 register */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_0_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the necessary setting of orientation interrupt.
 */
static int8_t config_orient_int_settg(
    const struct bmi160_acc_orient_int_cfg* orient_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;
    uint8_t data_array[2] = {0, 0};

    /* Configuring INT_ORIENT registers */
    rslt = bmi160_get_regs(BMI160_INT_ORIENT_0_ADDR, data_array, 2, dev);
    if(rslt == BMI160_OK) {
        data = data_array[0];
        temp = data & ~BMI160_ORIENT_MODE_MASK;

        /* Adding Orientation mode */
        data = temp | ((orient_int_cfg->orient_mode) & BMI160_ORIENT_MODE_MASK);
        temp = data & ~BMI160_ORIENT_BLOCK_MASK;

        /* Adding Orientation blocking */
        data = temp | ((orient_int_cfg->orient_blocking << 2) & BMI160_ORIENT_BLOCK_MASK);
        temp = data & ~BMI160_ORIENT_HYST_MASK;

        /* Adding Orientation hysteresis */
        data = temp | ((orient_int_cfg->orient_hyst << 4) & BMI160_ORIENT_HYST_MASK);
        data_array[0] = data;
        data = data_array[1];
        temp = data & ~BMI160_ORIENT_THETA_MASK;

        /* Adding Orientation threshold */
        data = temp | ((orient_int_cfg->orient_theta) & BMI160_ORIENT_THETA_MASK);
        temp = data & ~BMI160_ORIENT_UD_ENABLE;

        /* Adding Orient_ud_en */
        data = temp | ((orient_int_cfg->orient_ud_en << 6) & BMI160_ORIENT_UD_ENABLE);
        temp = data & ~BMI160_AXES_EN_MASK;

        /* Adding axes_en */
        data = temp | ((orient_int_cfg->axes_ex << 7) & BMI160_AXES_EN_MASK);
        data_array[1] = data;

        /* Writing data to INT_ORIENT 0 and INT_ORIENT 1
         * registers simultaneously */
        rslt = bmi160_set_regs(BMI160_INT_ORIENT_0_ADDR, data_array, 2, dev);
    }

    return rslt;
}

/*!
 * @brief This API enables the flat interrupt.
 */
static int8_t enable_flat_int(
    const struct bmi160_acc_flat_detect_int_cfg* flat_int,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Enable flat interrupt in Int Enable 0 register */
    rslt = bmi160_get_regs(BMI160_INT_ENABLE_0_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_FLAT_INT_EN_MASK;
        data = temp | ((flat_int->flat_en << 7) & BMI160_FLAT_INT_EN_MASK);

        /* write data to Int Enable 0 register */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_0_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the necessary setting of flat interrupt.
 */
static int8_t config_flat_int_settg(
    const struct bmi160_acc_flat_detect_int_cfg* flat_int,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;
    uint8_t data_array[2] = {0, 0};

    /* Configuring INT_FLAT register */
    rslt = bmi160_get_regs(BMI160_INT_FLAT_0_ADDR, data_array, 2, dev);
    if(rslt == BMI160_OK) {
        data = data_array[0];
        temp = data & ~BMI160_FLAT_THRES_MASK;

        /* Adding flat theta */
        data = temp | ((flat_int->flat_theta) & BMI160_FLAT_THRES_MASK);
        data_array[0] = data;
        data = data_array[1];
        temp = data & ~BMI160_FLAT_HOLD_TIME_MASK;

        /* Adding flat hold time */
        data = temp | ((flat_int->flat_hold_time << 4) & BMI160_FLAT_HOLD_TIME_MASK);
        temp = data & ~BMI160_FLAT_HYST_MASK;

        /* Adding flat hysteresis */
        data = temp | ((flat_int->flat_hy) & BMI160_FLAT_HYST_MASK);
        data_array[1] = data;

        /* Writing data to INT_FLAT 0 and INT_FLAT 1
         * registers simultaneously */
        rslt = bmi160_set_regs(BMI160_INT_FLAT_0_ADDR, data_array, 2, dev);
    }

    return rslt;
}

/*!
 * @brief This API enables the Low-g interrupt.
 */
static int8_t enable_low_g_int(
    const struct bmi160_acc_low_g_int_cfg* low_g_int,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Enable low-g interrupt in Int Enable 1 register */
    rslt = bmi160_get_regs(BMI160_INT_ENABLE_1_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_LOW_G_INT_EN_MASK;
        data = temp | ((low_g_int->low_en << 3) & BMI160_LOW_G_INT_EN_MASK);

        /* write data to Int Enable 0 register */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_1_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the source of data(filter & pre-filter)
 * for low-g interrupt.
 */
static int8_t config_low_g_data_src(
    const struct bmi160_acc_low_g_int_cfg* low_g_int,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Configure Int data 0 register to add source of interrupt */
    rslt = bmi160_get_regs(BMI160_INT_DATA_0_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_LOW_HIGH_SRC_INT_MASK;
        data = temp | ((low_g_int->low_data_src << 7) & BMI160_LOW_HIGH_SRC_INT_MASK);

        /* Write data to Data 0 address */
        rslt = bmi160_set_regs(BMI160_INT_DATA_0_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the necessary setting of low-g interrupt.
 */
static int8_t config_low_g_int_settg(
    const struct bmi160_acc_low_g_int_cfg* low_g_int,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t temp = 0;
    uint8_t data_array[3] = {0, 0, 0};

    /* Configuring INT_LOWHIGH register for low-g interrupt */
    rslt = bmi160_get_regs(BMI160_INT_LOWHIGH_2_ADDR, &data_array[2], 1, dev);
    if(rslt == BMI160_OK) {
        temp = data_array[2] & ~BMI160_LOW_G_HYST_MASK;

        /* Adding low-g hysteresis */
        data_array[2] = temp | (low_g_int->low_hyst & BMI160_LOW_G_HYST_MASK);
        temp = data_array[2] & ~BMI160_LOW_G_LOW_MODE_MASK;

        /* Adding low-mode */
        data_array[2] = temp | ((low_g_int->low_mode << 2) & BMI160_LOW_G_LOW_MODE_MASK);

        /* Adding low-g threshold */
        data_array[1] = low_g_int->low_thres;

        /* Adding low-g interrupt delay */
        data_array[0] = low_g_int->low_dur;

        /* Writing data to INT_LOWHIGH 0,1,2 registers simultaneously*/
        rslt = bmi160_set_regs(BMI160_INT_LOWHIGH_0_ADDR, data_array, 3, dev);
    }

    return rslt;
}

/*!
 * @brief This API enables the high-g interrupt.
 */
static int8_t enable_high_g_int(
    const struct bmi160_acc_high_g_int_cfg* high_g_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Enable low-g interrupt in Int Enable 1 register */
    rslt = bmi160_get_regs(BMI160_INT_ENABLE_1_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        /* Adding high-g X-axis */
        temp = data & ~BMI160_HIGH_G_X_INT_EN_MASK;
        data = temp | (high_g_int_cfg->high_g_x & BMI160_HIGH_G_X_INT_EN_MASK);

        /* Adding high-g Y-axis */
        temp = data & ~BMI160_HIGH_G_Y_INT_EN_MASK;
        data = temp | ((high_g_int_cfg->high_g_y << 1) & BMI160_HIGH_G_Y_INT_EN_MASK);

        /* Adding high-g Z-axis */
        temp = data & ~BMI160_HIGH_G_Z_INT_EN_MASK;
        data = temp | ((high_g_int_cfg->high_g_z << 2) & BMI160_HIGH_G_Z_INT_EN_MASK);

        /* write data to Int Enable 0 register */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_1_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the source of data(filter & pre-filter)
 * for high-g interrupt.
 */
static int8_t config_high_g_data_src(
    const struct bmi160_acc_high_g_int_cfg* high_g_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;
    uint8_t temp = 0;

    /* Configure Int data 0 register to add source of interrupt */
    rslt = bmi160_get_regs(BMI160_INT_DATA_0_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        temp = data & ~BMI160_LOW_HIGH_SRC_INT_MASK;
        data = temp | ((high_g_int_cfg->high_data_src << 7) & BMI160_LOW_HIGH_SRC_INT_MASK);

        /* Write data to Data 0 address */
        rslt = bmi160_set_regs(BMI160_INT_DATA_0_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the necessary setting of high-g interrupt.
 */
static int8_t config_high_g_int_settg(
    const struct bmi160_acc_high_g_int_cfg* high_g_int_cfg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t temp = 0;
    uint8_t data_array[3] = {0, 0, 0};

    rslt = bmi160_get_regs(BMI160_INT_LOWHIGH_2_ADDR, &data_array[0], 1, dev);
    if(rslt == BMI160_OK) {
        temp = data_array[0] & ~BMI160_HIGH_G_HYST_MASK;

        /* Adding high-g hysteresis */
        data_array[0] = temp | ((high_g_int_cfg->high_hy << 6) & BMI160_HIGH_G_HYST_MASK);

        /* Adding high-g duration */
        data_array[1] = high_g_int_cfg->high_dur;

        /* Adding high-g threshold */
        data_array[2] = high_g_int_cfg->high_thres;
        rslt = bmi160_set_regs(BMI160_INT_LOWHIGH_2_ADDR, data_array, 3, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the behavioural setting of interrupt pin.
 */
static int8_t
    config_int_out_ctrl(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t temp = 0;
    uint8_t data = 0;

    /* Configuration of output interrupt signals on pins INT1 and INT2 are
     * done in BMI160_INT_OUT_CTRL_ADDR register*/
    rslt = bmi160_get_regs(BMI160_INT_OUT_CTRL_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        /* updating the interrupt pin structure to local structure */
        const struct bmi160_int_pin_settg* intr_pin_sett = &(int_config->int_pin_settg);

        /* Configuring channel 1 */
        if(int_config->int_channel == BMI160_INT_CHANNEL_1) {
            /* Output enable */
            temp = data & ~BMI160_INT1_OUTPUT_EN_MASK;
            data = temp | ((intr_pin_sett->output_en << 3) & BMI160_INT1_OUTPUT_EN_MASK);

            /* Output mode */
            temp = data & ~BMI160_INT1_OUTPUT_MODE_MASK;
            data = temp | ((intr_pin_sett->output_mode << 2) & BMI160_INT1_OUTPUT_MODE_MASK);

            /* Output type */
            temp = data & ~BMI160_INT1_OUTPUT_TYPE_MASK;
            data = temp | ((intr_pin_sett->output_type << 1) & BMI160_INT1_OUTPUT_TYPE_MASK);

            /* edge control */
            temp = data & ~BMI160_INT1_EDGE_CTRL_MASK;
            data = temp | ((intr_pin_sett->edge_ctrl) & BMI160_INT1_EDGE_CTRL_MASK);
        } else {
            /* Configuring channel 2 */
            /* Output enable */
            temp = data & ~BMI160_INT2_OUTPUT_EN_MASK;
            data = temp | ((intr_pin_sett->output_en << 7) & BMI160_INT2_OUTPUT_EN_MASK);

            /* Output mode */
            temp = data & ~BMI160_INT2_OUTPUT_MODE_MASK;
            data = temp | ((intr_pin_sett->output_mode << 6) & BMI160_INT2_OUTPUT_MODE_MASK);

            /* Output type */
            temp = data & ~BMI160_INT2_OUTPUT_TYPE_MASK;
            data = temp | ((intr_pin_sett->output_type << 5) & BMI160_INT2_OUTPUT_TYPE_MASK);

            /* edge control */
            temp = data & ~BMI160_INT2_EDGE_CTRL_MASK;
            data = temp | ((intr_pin_sett->edge_ctrl << 4) & BMI160_INT2_EDGE_CTRL_MASK);
        }

        rslt = bmi160_set_regs(BMI160_INT_OUT_CTRL_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API configure the mode(input enable, latch or non-latch) of interrupt pin.
 */
static int8_t
    config_int_latch(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t temp = 0;
    uint8_t data = 0;

    /* Configuration of latch on pins INT1 and INT2 are done in
     * BMI160_INT_LATCH_ADDR register*/
    rslt = bmi160_get_regs(BMI160_INT_LATCH_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        /* updating the interrupt pin structure to local structure */
        const struct bmi160_int_pin_settg* intr_pin_sett = &(int_config->int_pin_settg);
        if(int_config->int_channel == BMI160_INT_CHANNEL_1) {
            /* Configuring channel 1 */
            /* Input enable */
            temp = data & ~BMI160_INT1_INPUT_EN_MASK;
            data = temp | ((intr_pin_sett->input_en << 4) & BMI160_INT1_INPUT_EN_MASK);
        } else {
            /* Configuring channel 2 */
            /* Input enable */
            temp = data & ~BMI160_INT2_INPUT_EN_MASK;
            data = temp | ((intr_pin_sett->input_en << 5) & BMI160_INT2_INPUT_EN_MASK);
        }

        /* In case of latch interrupt,update the latch duration */

        /* Latching holds the interrupt for the amount of latch
         * duration time */
        temp = data & ~BMI160_INT_LATCH_MASK;
        data = temp | (intr_pin_sett->latch_dur & BMI160_INT_LATCH_MASK);

        /* OUT_CTRL_INT and LATCH_INT address lie consecutively,
         * hence writing data to respective registers at one go */
        rslt = bmi160_set_regs(BMI160_INT_LATCH_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API performs the self test for accelerometer of BMI160
 */
static int8_t perform_accel_self_test(struct bmi160_dev* dev) {
    int8_t rslt;
    struct bmi160_sensor_data accel_pos, accel_neg;

    /* Enable Gyro self test bit */
    rslt = enable_accel_self_test(dev);
    if(rslt == BMI160_OK) {
        /* Perform accel self test with positive excitation */
        rslt = accel_self_test_positive_excitation(&accel_pos, dev);
        if(rslt == BMI160_OK) {
            /* Perform accel self test with negative excitation */
            rslt = accel_self_test_negative_excitation(&accel_neg, dev);
            if(rslt == BMI160_OK) {
                /* Validate the self test result */
                rslt = validate_accel_self_test(&accel_pos, &accel_neg);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API enables to perform the accel self test by setting proper
 * configurations to facilitate accel self test
 */
static int8_t enable_accel_self_test(struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t reg_data;

    /* Set the Accel power mode as normal mode */
    dev->accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    /* Set the sensor range configuration as 8G */
    dev->accel_cfg.range = BMI160_ACCEL_RANGE_8G;
    rslt = bmi160_set_sens_conf(dev);
    if(rslt == BMI160_OK) {
        /* Accel configurations are set to facilitate self test
         * acc_odr - 1600Hz ; acc_bwp = 2 ; acc_us = 0 */
        reg_data = BMI160_ACCEL_SELF_TEST_CONFIG;
        rslt = bmi160_set_regs(BMI160_ACCEL_CONFIG_ADDR, &reg_data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API performs accel self test with positive excitation
 */
static int8_t accel_self_test_positive_excitation(
    struct bmi160_sensor_data* accel_pos,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t reg_data;

    /* Enable accel self test with positive self-test excitation
     * and with amplitude of deflection set as high */
    reg_data = BMI160_ACCEL_SELF_TEST_POSITIVE_EN;
    rslt = bmi160_set_regs(BMI160_SELF_TEST_ADDR, &reg_data, 1, dev);
    if(rslt == BMI160_OK) {
        /* Read the data after a delay of 50ms - refer datasheet  2.8.1 accel self test*/
        dev->delay_ms(BMI160_ACCEL_SELF_TEST_DELAY);
        rslt = bmi160_get_sensor_data(BMI160_ACCEL_ONLY, accel_pos, NULL, dev);
    }

    return rslt;
}

/*!
 * @brief This API performs accel self test with negative excitation
 */
static int8_t accel_self_test_negative_excitation(
    struct bmi160_sensor_data* accel_neg,
    const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t reg_data;

    /* Enable accel self test with negative self-test excitation
     * and with amplitude of deflection set as high */
    reg_data = BMI160_ACCEL_SELF_TEST_NEGATIVE_EN;
    rslt = bmi160_set_regs(BMI160_SELF_TEST_ADDR, &reg_data, 1, dev);
    if(rslt == BMI160_OK) {
        /* Read the data after a delay of 50ms */
        dev->delay_ms(BMI160_ACCEL_SELF_TEST_DELAY);
        rslt = bmi160_get_sensor_data(BMI160_ACCEL_ONLY, accel_neg, NULL, dev);
    }

    return rslt;
}

/*!
 * @brief This API validates the accel self test results
 */
static int8_t validate_accel_self_test(
    const struct bmi160_sensor_data* accel_pos,
    const struct bmi160_sensor_data* accel_neg) {
    int8_t rslt;

    /* Validate the results of self test */
    if(((accel_neg->x - accel_pos->x) > BMI160_ACCEL_SELF_TEST_LIMIT) &&
       ((accel_neg->y - accel_pos->y) > BMI160_ACCEL_SELF_TEST_LIMIT) &&
       ((accel_neg->z - accel_pos->z) > BMI160_ACCEL_SELF_TEST_LIMIT)) {
        /* Self test pass condition */
        rslt = BMI160_OK;
    } else {
        rslt = BMI160_W_ACCEl_SELF_TEST_FAIL;
    }

    return rslt;
}

/*!
 * @brief This API performs the self test for gyroscope of BMI160
 */
static int8_t perform_gyro_self_test(const struct bmi160_dev* dev) {
    int8_t rslt;

    /* Enable Gyro self test bit */
    rslt = enable_gyro_self_test(dev);
    if(rslt == BMI160_OK) {
        /* Validate the gyro self test a delay of 50ms */
        dev->delay_ms(50);

        /* Validate the gyro self test results */
        rslt = validate_gyro_self_test(dev);
    }

    return rslt;
}

/*!
 * @brief This API enables the self test bit to trigger self test for Gyro
 */
static int8_t enable_gyro_self_test(const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t reg_data;

    /* Enable the Gyro self test bit to trigger the self test */
    rslt = bmi160_get_regs(BMI160_SELF_TEST_ADDR, &reg_data, 1, dev);
    if(rslt == BMI160_OK) {
        reg_data = BMI160_SET_BITS(reg_data, BMI160_GYRO_SELF_TEST, 1);
        rslt = bmi160_set_regs(BMI160_SELF_TEST_ADDR, &reg_data, 1, dev);
        if(rslt == BMI160_OK) {
            /* Delay to enable gyro self test */
            dev->delay_ms(15);
        }
    }

    return rslt;
}

/*!
 * @brief This API validates the self test results of Gyro
 */
static int8_t validate_gyro_self_test(const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t reg_data;

    /* Validate the Gyro self test result */
    rslt = bmi160_get_regs(BMI160_STATUS_ADDR, &reg_data, 1, dev);
    if(rslt == BMI160_OK) {
        reg_data = BMI160_GET_BITS(reg_data, BMI160_GYRO_SELF_TEST_STATUS);
        if(reg_data == BMI160_ENABLE) {
            /* Gyro self test success case */
            rslt = BMI160_OK;
        } else {
            rslt = BMI160_W_GYRO_SELF_TEST_FAIL;
        }
    }

    return rslt;
}

/*!
 *  @brief This API sets FIFO full interrupt of the sensor.This interrupt
 *  occurs when the FIFO is full and the next full data sample would cause
 *  a FIFO overflow, which may delete the old samples.
 */
static int8_t
    set_fifo_full_int(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;

    /* Null-pointer check */
    if((dev == NULL) || (dev->delay_ms == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /*enable the fifo full interrupt */
        rslt = enable_fifo_full_int(int_config, dev);
        if(rslt == BMI160_OK) {
            /* Configure Interrupt pins */
            rslt = set_intr_pin_config(int_config, dev);
            if(rslt == BMI160_OK) {
                rslt = map_hardware_interrupt(int_config, dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This enable the FIFO full interrupt engine.
 */
static int8_t
    enable_fifo_full_int(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;

    rslt = bmi160_get_regs(BMI160_INT_ENABLE_1_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        data = BMI160_SET_BITS(data, BMI160_FIFO_FULL_INT, int_config->fifo_full_int_en);

        /* Writing data to INT ENABLE 1 Address */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_1_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 *  @brief This API sets FIFO watermark interrupt of the sensor.The FIFO
 *  watermark interrupt is fired, when the FIFO fill level is above a fifo
 *  watermark.
 */
static int8_t set_fifo_watermark_int(
    const struct bmi160_int_settg* int_config,
    const struct bmi160_dev* dev) {
    int8_t rslt = BMI160_OK;

    if((dev == NULL) || (dev->delay_ms == NULL)) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* Enable fifo-watermark interrupt in Int Enable 1 register */
        rslt = enable_fifo_wtm_int(int_config, dev);
        if(rslt == BMI160_OK) {
            /* Configure Interrupt pins */
            rslt = set_intr_pin_config(int_config, dev);
            if(rslt == BMI160_OK) {
                rslt = map_hardware_interrupt(int_config, dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This enable the FIFO watermark interrupt engine.
 */
static int8_t
    enable_fifo_wtm_int(const struct bmi160_int_settg* int_config, const struct bmi160_dev* dev) {
    int8_t rslt;
    uint8_t data = 0;

    rslt = bmi160_get_regs(BMI160_INT_ENABLE_1_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        data = BMI160_SET_BITS(data, BMI160_FIFO_WTM_INT, int_config->fifo_wtm_int_en);

        /* Writing data to INT ENABLE 1 Address */
        rslt = bmi160_set_regs(BMI160_INT_ENABLE_1_ADDR, &data, 1, dev);
    }

    return rslt;
}

/*!
 *  @brief This API is used to reset the FIFO related configurations
 *  in the fifo_frame structure.
 */
static void reset_fifo_data_structure(const struct bmi160_dev* dev) {
    /*Prepare for next FIFO read by resetting FIFO's
     * internal data structures*/
    dev->fifo->accel_byte_start_idx = 0;
    dev->fifo->gyro_byte_start_idx = 0;
    dev->fifo->aux_byte_start_idx = 0;
    dev->fifo->sensor_time = 0;
    dev->fifo->skipped_frame_count = 0;
}

/*!
 *  @brief This API is used to read fifo_byte_counter value (i.e)
 *  current fill-level in Fifo buffer.
 */
static int8_t get_fifo_byte_counter(uint16_t* bytes_to_read, struct bmi160_dev const* dev) {
    int8_t rslt = 0;
    uint8_t data[2];
    uint8_t addr = BMI160_FIFO_LENGTH_ADDR;

    rslt |= bmi160_get_regs(addr, data, 2, dev);
    data[1] = data[1] & BMI160_FIFO_BYTE_COUNTER_MASK;

    /* Available data in FIFO is stored in bytes_to_read*/
    *bytes_to_read = (((uint16_t)data[1] << 8) | ((uint16_t)data[0]));

    return rslt;
}

/*!
 *  @brief This API is used to compute the number of bytes of accel FIFO data
 *  which is to be parsed in header-less mode
 */
static void get_accel_len_to_parse(
    uint16_t* data_index,
    uint16_t* data_read_length,
    const uint8_t* acc_frame_count,
    const struct bmi160_dev* dev) {
    /* Data start index */
    *data_index = dev->fifo->accel_byte_start_idx;
    if(dev->fifo->fifo_data_enable == BMI160_FIFO_A_ENABLE) {
        *data_read_length = (*acc_frame_count) * BMI160_FIFO_A_LENGTH;
    } else if(dev->fifo->fifo_data_enable == BMI160_FIFO_G_A_ENABLE) {
        *data_read_length = (*acc_frame_count) * BMI160_FIFO_GA_LENGTH;
    } else if(dev->fifo->fifo_data_enable == BMI160_FIFO_M_A_ENABLE) {
        *data_read_length = (*acc_frame_count) * BMI160_FIFO_MA_LENGTH;
    } else if(dev->fifo->fifo_data_enable == BMI160_FIFO_M_G_A_ENABLE) {
        *data_read_length = (*acc_frame_count) * BMI160_FIFO_MGA_LENGTH;
    } else {
        /* When accel is not enabled ,there will be no accel data.
         * so we update the data index as complete */
        *data_index = dev->fifo->length;
    }

    if(*data_read_length > dev->fifo->length) {
        /* Handling the case where more data is requested
         * than that is available*/
        *data_read_length = dev->fifo->length;
    }
}

/*!
 *  @brief This API is used to parse the accelerometer data from the
 *  FIFO data in both header mode and header-less mode.
 *  It updates the idx value which is used to store the index of
 *  the current data byte which is parsed.
 */
static void unpack_accel_frame(
    struct bmi160_sensor_data* acc,
    uint16_t* idx,
    uint8_t* acc_idx,
    uint8_t frame_info,
    const struct bmi160_dev* dev) {
    switch(frame_info) {
    case BMI160_FIFO_HEAD_A:
    case BMI160_FIFO_A_ENABLE:

        /*Partial read, then skip the data*/
        if((*idx + BMI160_FIFO_A_LENGTH) > dev->fifo->length) {
            /*Update the data index as complete*/
            *idx = dev->fifo->length;
            break;
        }

        /*Unpack the data array into the structure instance "acc" */
        unpack_accel_data(&acc[*acc_idx], *idx, dev);

        /*Move the data index*/
        *idx = *idx + BMI160_FIFO_A_LENGTH;
        (*acc_idx)++;
        break;
    case BMI160_FIFO_HEAD_G_A:
    case BMI160_FIFO_G_A_ENABLE:

        /*Partial read, then skip the data*/
        if((*idx + BMI160_FIFO_GA_LENGTH) > dev->fifo->length) {
            /*Update the data index as complete*/
            *idx = dev->fifo->length;
            break;
        }

        /*Unpack the data array into structure instance "acc"*/
        unpack_accel_data(&acc[*acc_idx], *idx + BMI160_FIFO_G_LENGTH, dev);

        /*Move the data index*/
        *idx = *idx + BMI160_FIFO_GA_LENGTH;
        (*acc_idx)++;
        break;
    case BMI160_FIFO_HEAD_M_A:
    case BMI160_FIFO_M_A_ENABLE:

        /*Partial read, then skip the data*/
        if((*idx + BMI160_FIFO_MA_LENGTH) > dev->fifo->length) {
            /*Update the data index as complete*/
            *idx = dev->fifo->length;
            break;
        }

        /*Unpack the data array into structure instance "acc"*/
        unpack_accel_data(&acc[*acc_idx], *idx + BMI160_FIFO_M_LENGTH, dev);

        /*Move the data index*/
        *idx = *idx + BMI160_FIFO_MA_LENGTH;
        (*acc_idx)++;
        break;
    case BMI160_FIFO_HEAD_M_G_A:
    case BMI160_FIFO_M_G_A_ENABLE:

        /*Partial read, then skip the data*/
        if((*idx + BMI160_FIFO_MGA_LENGTH) > dev->fifo->length) {
            /*Update the data index as complete*/
            *idx = dev->fifo->length;
            break;
        }

        /*Unpack the data array into structure instance "acc"*/
        unpack_accel_data(&acc[*acc_idx], *idx + BMI160_FIFO_MG_LENGTH, dev);

        /*Move the data index*/
        *idx = *idx + BMI160_FIFO_MGA_LENGTH;
        (*acc_idx)++;
        break;
    case BMI160_FIFO_HEAD_M:
    case BMI160_FIFO_M_ENABLE:
        (*idx) = (*idx) + BMI160_FIFO_M_LENGTH;
        break;
    case BMI160_FIFO_HEAD_G:
    case BMI160_FIFO_G_ENABLE:
        (*idx) = (*idx) + BMI160_FIFO_G_LENGTH;
        break;
    case BMI160_FIFO_HEAD_M_G:
    case BMI160_FIFO_M_G_ENABLE:
        (*idx) = (*idx) + BMI160_FIFO_MG_LENGTH;
        break;
    default:
        break;
    }
}

/*!
 *  @brief This API is used to parse the accelerometer data from the
 *  FIFO data and store it in the instance of the structure bmi160_sensor_data.
 */
static void unpack_accel_data(
    struct bmi160_sensor_data* accel_data,
    uint16_t data_start_index,
    const struct bmi160_dev* dev) {
    uint16_t data_lsb;
    uint16_t data_msb;

    /* Accel raw x data */
    data_lsb = dev->fifo->data[data_start_index++];
    data_msb = dev->fifo->data[data_start_index++];
    accel_data->x = (int16_t)((data_msb << 8) | data_lsb);

    /* Accel raw y data */
    data_lsb = dev->fifo->data[data_start_index++];
    data_msb = dev->fifo->data[data_start_index++];
    accel_data->y = (int16_t)((data_msb << 8) | data_lsb);

    /* Accel raw z data */
    data_lsb = dev->fifo->data[data_start_index++];
    data_msb = dev->fifo->data[data_start_index++];
    accel_data->z = (int16_t)((data_msb << 8) | data_lsb);
}

/*!
 *  @brief This API is used to parse the accelerometer data from the
 *  FIFO data in header mode.
 */
static void extract_accel_header_mode(
    struct bmi160_sensor_data* accel_data,
    uint8_t* accel_length,
    const struct bmi160_dev* dev) {
    uint8_t frame_header = 0;
    uint16_t data_index;
    uint8_t accel_index = 0;

    for(data_index = dev->fifo->accel_byte_start_idx; data_index < dev->fifo->length;) {
        /* extracting Frame header */
        frame_header = (dev->fifo->data[data_index] & BMI160_FIFO_TAG_INTR_MASK);

        /*Index is moved to next byte where the data is starting*/
        data_index++;
        switch(frame_header) {
        /* Accel frame */
        case BMI160_FIFO_HEAD_A:
        case BMI160_FIFO_HEAD_M_A:
        case BMI160_FIFO_HEAD_G_A:
        case BMI160_FIFO_HEAD_M_G_A:
            unpack_accel_frame(accel_data, &data_index, &accel_index, frame_header, dev);
            break;
        case BMI160_FIFO_HEAD_M:
            move_next_frame(&data_index, BMI160_FIFO_M_LENGTH, dev);
            break;
        case BMI160_FIFO_HEAD_G:
            move_next_frame(&data_index, BMI160_FIFO_G_LENGTH, dev);
            break;
        case BMI160_FIFO_HEAD_M_G:
            move_next_frame(&data_index, BMI160_FIFO_MG_LENGTH, dev);
            break;

        /* Sensor time frame */
        case BMI160_FIFO_HEAD_SENSOR_TIME:
            unpack_sensortime_frame(&data_index, dev);
            break;

        /* Skip frame */
        case BMI160_FIFO_HEAD_SKIP_FRAME:
            unpack_skipped_frame(&data_index, dev);
            break;

        /* Input config frame */
        case BMI160_FIFO_HEAD_INPUT_CONFIG:
            move_next_frame(&data_index, 1, dev);
            break;
        case BMI160_FIFO_HEAD_OVER_READ:

            /* Update the data index as complete in case of Over read */
            data_index = dev->fifo->length;
            break;
        default:
            break;
        }
        if(*accel_length == accel_index) {
            /* Number of frames to read completed */
            break;
        }
    }

    /*Update number of accel data read*/
    *accel_length = accel_index;

    /*Update the accel frame index*/
    dev->fifo->accel_byte_start_idx = data_index;
}

/*!
 *  @brief This API computes the number of bytes of gyro FIFO data
 *  which is to be parsed in header-less mode
 */
static void get_gyro_len_to_parse(
    uint16_t* data_index,
    uint16_t* data_read_length,
    const uint8_t* gyro_frame_count,
    const struct bmi160_dev* dev) {
    /* Data start index */
    *data_index = dev->fifo->gyro_byte_start_idx;
    if(dev->fifo->fifo_data_enable == BMI160_FIFO_G_ENABLE) {
        *data_read_length = (*gyro_frame_count) * BMI160_FIFO_G_LENGTH;
    } else if(dev->fifo->fifo_data_enable == BMI160_FIFO_G_A_ENABLE) {
        *data_read_length = (*gyro_frame_count) * BMI160_FIFO_GA_LENGTH;
    } else if(dev->fifo->fifo_data_enable == BMI160_FIFO_M_G_ENABLE) {
        *data_read_length = (*gyro_frame_count) * BMI160_FIFO_MG_LENGTH;
    } else if(dev->fifo->fifo_data_enable == BMI160_FIFO_M_G_A_ENABLE) {
        *data_read_length = (*gyro_frame_count) * BMI160_FIFO_MGA_LENGTH;
    } else {
        /* When gyro is not enabled ,there will be no gyro data.
         * so we update the data index as complete */
        *data_index = dev->fifo->length;
    }

    if(*data_read_length > dev->fifo->length) {
        /* Handling the case where more data is requested
         * than that is available*/
        *data_read_length = dev->fifo->length;
    }
}

/*!
 *  @brief This API is used to parse the gyroscope's data from the
 *  FIFO data in both header mode and header-less mode.
 *  It updates the idx value which is used to store the index of
 *  the current data byte which is parsed.
 */
static void unpack_gyro_frame(
    struct bmi160_sensor_data* gyro,
    uint16_t* idx,
    uint8_t* gyro_idx,
    uint8_t frame_info,
    const struct bmi160_dev* dev) {
    switch(frame_info) {
    case BMI160_FIFO_HEAD_G:
    case BMI160_FIFO_G_ENABLE:

        /*Partial read, then skip the data*/
        if((*idx + BMI160_FIFO_G_LENGTH) > dev->fifo->length) {
            /*Update the data index as complete*/
            *idx = dev->fifo->length;
            break;
        }

        /*Unpack the data array into structure instance "gyro"*/
        unpack_gyro_data(&gyro[*gyro_idx], *idx, dev);

        /*Move the data index*/
        (*idx) = (*idx) + BMI160_FIFO_G_LENGTH;
        (*gyro_idx)++;
        break;
    case BMI160_FIFO_HEAD_G_A:
    case BMI160_FIFO_G_A_ENABLE:

        /*Partial read, then skip the data*/
        if((*idx + BMI160_FIFO_GA_LENGTH) > dev->fifo->length) {
            /*Update the data index as complete*/
            *idx = dev->fifo->length;
            break;
        }

        /* Unpack the data array into structure instance "gyro" */
        unpack_gyro_data(&gyro[*gyro_idx], *idx, dev);

        /* Move the data index */
        *idx = *idx + BMI160_FIFO_GA_LENGTH;
        (*gyro_idx)++;
        break;
    case BMI160_FIFO_HEAD_M_G_A:
    case BMI160_FIFO_M_G_A_ENABLE:

        /*Partial read, then skip the data*/
        if((*idx + BMI160_FIFO_MGA_LENGTH) > dev->fifo->length) {
            /*Update the data index as complete*/
            *idx = dev->fifo->length;
            break;
        }

        /*Unpack the data array into structure instance "gyro"*/
        unpack_gyro_data(&gyro[*gyro_idx], *idx + BMI160_FIFO_M_LENGTH, dev);

        /*Move the data index*/
        *idx = *idx + BMI160_FIFO_MGA_LENGTH;
        (*gyro_idx)++;
        break;
    case BMI160_FIFO_HEAD_M_A:
    case BMI160_FIFO_M_A_ENABLE:

        /* Move the data index */
        *idx = *idx + BMI160_FIFO_MA_LENGTH;
        break;
    case BMI160_FIFO_HEAD_M:
    case BMI160_FIFO_M_ENABLE:
        (*idx) = (*idx) + BMI160_FIFO_M_LENGTH;
        break;
    case BMI160_FIFO_HEAD_M_G:
    case BMI160_FIFO_M_G_ENABLE:

        /*Partial read, then skip the data*/
        if((*idx + BMI160_FIFO_MG_LENGTH) > dev->fifo->length) {
            /*Update the data index as complete*/
            *idx = dev->fifo->length;
            break;
        }

        /*Unpack the data array into structure instance "gyro"*/
        unpack_gyro_data(&gyro[*gyro_idx], *idx + BMI160_FIFO_M_LENGTH, dev);

        /*Move the data index*/
        (*idx) = (*idx) + BMI160_FIFO_MG_LENGTH;
        (*gyro_idx)++;
        break;
    case BMI160_FIFO_HEAD_A:
    case BMI160_FIFO_A_ENABLE:

        /*Move the data index*/
        *idx = *idx + BMI160_FIFO_A_LENGTH;
        break;
    default:
        break;
    }
}

/*!
 *  @brief This API is used to parse the gyro data from the
 *  FIFO data and store it in the instance of the structure bmi160_sensor_data.
 */
static void unpack_gyro_data(
    struct bmi160_sensor_data* gyro_data,
    uint16_t data_start_index,
    const struct bmi160_dev* dev) {
    uint16_t data_lsb;
    uint16_t data_msb;

    /* Gyro raw x data */
    data_lsb = dev->fifo->data[data_start_index++];
    data_msb = dev->fifo->data[data_start_index++];
    gyro_data->x = (int16_t)((data_msb << 8) | data_lsb);

    /* Gyro raw y data */
    data_lsb = dev->fifo->data[data_start_index++];
    data_msb = dev->fifo->data[data_start_index++];
    gyro_data->y = (int16_t)((data_msb << 8) | data_lsb);

    /* Gyro raw z data */
    data_lsb = dev->fifo->data[data_start_index++];
    data_msb = dev->fifo->data[data_start_index++];
    gyro_data->z = (int16_t)((data_msb << 8) | data_lsb);
}

/*!
 *  @brief This API is used to parse the gyro data from the
 *  FIFO data in header mode.
 */
static void extract_gyro_header_mode(
    struct bmi160_sensor_data* gyro_data,
    uint8_t* gyro_length,
    const struct bmi160_dev* dev) {
    uint8_t frame_header = 0;
    uint16_t data_index;
    uint8_t gyro_index = 0;

    for(data_index = dev->fifo->gyro_byte_start_idx; data_index < dev->fifo->length;) {
        /* extracting Frame header */
        frame_header = (dev->fifo->data[data_index] & BMI160_FIFO_TAG_INTR_MASK);

        /*Index is moved to next byte where the data is starting*/
        data_index++;
        switch(frame_header) {
        /* GYRO frame */
        case BMI160_FIFO_HEAD_G:
        case BMI160_FIFO_HEAD_G_A:
        case BMI160_FIFO_HEAD_M_G:
        case BMI160_FIFO_HEAD_M_G_A:
            unpack_gyro_frame(gyro_data, &data_index, &gyro_index, frame_header, dev);
            break;
        case BMI160_FIFO_HEAD_A:
            move_next_frame(&data_index, BMI160_FIFO_A_LENGTH, dev);
            break;
        case BMI160_FIFO_HEAD_M:
            move_next_frame(&data_index, BMI160_FIFO_M_LENGTH, dev);
            break;
        case BMI160_FIFO_HEAD_M_A:
            move_next_frame(&data_index, BMI160_FIFO_M_LENGTH, dev);
            break;

        /* Sensor time frame */
        case BMI160_FIFO_HEAD_SENSOR_TIME:
            unpack_sensortime_frame(&data_index, dev);
            break;

        /* Skip frame */
        case BMI160_FIFO_HEAD_SKIP_FRAME:
            unpack_skipped_frame(&data_index, dev);
            break;

        /* Input config frame */
        case BMI160_FIFO_HEAD_INPUT_CONFIG:
            move_next_frame(&data_index, 1, dev);
            break;
        case BMI160_FIFO_HEAD_OVER_READ:

            /* Update the data index as complete in case of over read */
            data_index = dev->fifo->length;
            break;
        default:
            break;
        }
        if(*gyro_length == gyro_index) {
            /*Number of frames to read completed*/
            break;
        }
    }

    /*Update number of gyro data read*/
    *gyro_length = gyro_index;

    /*Update the gyro frame index*/
    dev->fifo->gyro_byte_start_idx = data_index;
}

/*!
 *  @brief This API computes the number of bytes of aux FIFO data
 *  which is to be parsed in header-less mode
 */
static void get_aux_len_to_parse(
    uint16_t* data_index,
    uint16_t* data_read_length,
    const uint8_t* aux_frame_count,
    const struct bmi160_dev* dev) {
    /* Data start index */
    *data_index = dev->fifo->gyro_byte_start_idx;
    if(dev->fifo->fifo_data_enable == BMI160_FIFO_M_ENABLE) {
        *data_read_length = (*aux_frame_count) * BMI160_FIFO_M_LENGTH;
    } else if(dev->fifo->fifo_data_enable == BMI160_FIFO_M_A_ENABLE) {
        *data_read_length = (*aux_frame_count) * BMI160_FIFO_MA_LENGTH;
    } else if(dev->fifo->fifo_data_enable == BMI160_FIFO_M_G_ENABLE) {
        *data_read_length = (*aux_frame_count) * BMI160_FIFO_MG_LENGTH;
    } else if(dev->fifo->fifo_data_enable == BMI160_FIFO_M_G_A_ENABLE) {
        *data_read_length = (*aux_frame_count) * BMI160_FIFO_MGA_LENGTH;
    } else {
        /* When aux is not enabled ,there will be no aux data.
         * so we update the data index as complete */
        *data_index = dev->fifo->length;
    }

    if(*data_read_length > dev->fifo->length) {
        /* Handling the case where more data is requested
         * than that is available */
        *data_read_length = dev->fifo->length;
    }
}

/*!
 *  @brief This API is used to parse the aux's data from the
 *  FIFO data in both header mode and header-less mode.
 *  It updates the idx value which is used to store the index of
 *  the current data byte which is parsed
 */
static void unpack_aux_frame(
    struct bmi160_aux_data* aux_data,
    uint16_t* idx,
    uint8_t* aux_index,
    uint8_t frame_info,
    const struct bmi160_dev* dev) {
    switch(frame_info) {
    case BMI160_FIFO_HEAD_M:
    case BMI160_FIFO_M_ENABLE:

        /* Partial read, then skip the data */
        if((*idx + BMI160_FIFO_M_LENGTH) > dev->fifo->length) {
            /* Update the data index as complete */
            *idx = dev->fifo->length;
            break;
        }

        /* Unpack the data array into structure instance */
        unpack_aux_data(&aux_data[*aux_index], *idx, dev);

        /* Move the data index */
        *idx = *idx + BMI160_FIFO_M_LENGTH;
        (*aux_index)++;
        break;
    case BMI160_FIFO_HEAD_M_A:
    case BMI160_FIFO_M_A_ENABLE:

        /* Partial read, then skip the data */
        if((*idx + BMI160_FIFO_MA_LENGTH) > dev->fifo->length) {
            /* Update the data index as complete */
            *idx = dev->fifo->length;
            break;
        }

        /* Unpack the data array into structure instance */
        unpack_aux_data(&aux_data[*aux_index], *idx, dev);

        /* Move the data index */
        *idx = *idx + BMI160_FIFO_MA_LENGTH;
        (*aux_index)++;
        break;
    case BMI160_FIFO_HEAD_M_G:
    case BMI160_FIFO_M_G_ENABLE:

        /* Partial read, then skip the data */
        if((*idx + BMI160_FIFO_MG_LENGTH) > dev->fifo->length) {
            /* Update the data index as complete */
            *idx = dev->fifo->length;
            break;
        }

        /* Unpack the data array into structure instance */
        unpack_aux_data(&aux_data[*aux_index], *idx, dev);

        /* Move the data index */
        (*idx) = (*idx) + BMI160_FIFO_MG_LENGTH;
        (*aux_index)++;
        break;
    case BMI160_FIFO_HEAD_M_G_A:
    case BMI160_FIFO_M_G_A_ENABLE:

        /*Partial read, then skip the data*/
        if((*idx + BMI160_FIFO_MGA_LENGTH) > dev->fifo->length) {
            /* Update the data index as complete */
            *idx = dev->fifo->length;
            break;
        }

        /* Unpack the data array into structure instance */
        unpack_aux_data(&aux_data[*aux_index], *idx, dev);

        /*Move the data index*/
        *idx = *idx + BMI160_FIFO_MGA_LENGTH;
        (*aux_index)++;
        break;
    case BMI160_FIFO_HEAD_G:
    case BMI160_FIFO_G_ENABLE:

        /* Move the data index */
        (*idx) = (*idx) + BMI160_FIFO_G_LENGTH;
        break;
    case BMI160_FIFO_HEAD_G_A:
    case BMI160_FIFO_G_A_ENABLE:

        /* Move the data index */
        *idx = *idx + BMI160_FIFO_GA_LENGTH;
        break;
    case BMI160_FIFO_HEAD_A:
    case BMI160_FIFO_A_ENABLE:

        /* Move the data index */
        *idx = *idx + BMI160_FIFO_A_LENGTH;
        break;
    default:
        break;
    }
}

/*!
 *  @brief This API is used to parse the aux data from the
 *  FIFO data and store it in the instance of the structure bmi160_aux_data.
 */
static void unpack_aux_data(
    struct bmi160_aux_data* aux_data,
    uint16_t data_start_index,
    const struct bmi160_dev* dev) {
    /* Aux data bytes */
    aux_data->data[0] = dev->fifo->data[data_start_index++];
    aux_data->data[1] = dev->fifo->data[data_start_index++];
    aux_data->data[2] = dev->fifo->data[data_start_index++];
    aux_data->data[3] = dev->fifo->data[data_start_index++];
    aux_data->data[4] = dev->fifo->data[data_start_index++];
    aux_data->data[5] = dev->fifo->data[data_start_index++];
    aux_data->data[6] = dev->fifo->data[data_start_index++];
    aux_data->data[7] = dev->fifo->data[data_start_index++];
}

/*!
 *  @brief This API is used to parse the aux data from the
 *  FIFO data in header mode.
 */
static void extract_aux_header_mode(
    struct bmi160_aux_data* aux_data,
    uint8_t* aux_length,
    const struct bmi160_dev* dev) {
    uint8_t frame_header = 0;
    uint16_t data_index;
    uint8_t aux_index = 0;

    for(data_index = dev->fifo->aux_byte_start_idx; data_index < dev->fifo->length;) {
        /* extracting Frame header */
        frame_header = (dev->fifo->data[data_index] & BMI160_FIFO_TAG_INTR_MASK);

        /*Index is moved to next byte where the data is starting*/
        data_index++;
        switch(frame_header) {
        /* Aux frame */
        case BMI160_FIFO_HEAD_M:
        case BMI160_FIFO_HEAD_M_A:
        case BMI160_FIFO_HEAD_M_G:
        case BMI160_FIFO_HEAD_M_G_A:
            unpack_aux_frame(aux_data, &data_index, &aux_index, frame_header, dev);
            break;
        case BMI160_FIFO_HEAD_G:
            move_next_frame(&data_index, BMI160_FIFO_G_LENGTH, dev);
            break;
        case BMI160_FIFO_HEAD_G_A:
            move_next_frame(&data_index, BMI160_FIFO_GA_LENGTH, dev);
            break;
        case BMI160_FIFO_HEAD_A:
            move_next_frame(&data_index, BMI160_FIFO_A_LENGTH, dev);
            break;

        /* Sensor time frame */
        case BMI160_FIFO_HEAD_SENSOR_TIME:
            unpack_sensortime_frame(&data_index, dev);
            break;

        /* Skip frame */
        case BMI160_FIFO_HEAD_SKIP_FRAME:
            unpack_skipped_frame(&data_index, dev);
            break;

        /* Input config frame */
        case BMI160_FIFO_HEAD_INPUT_CONFIG:
            move_next_frame(&data_index, 1, dev);
            break;
        case BMI160_FIFO_HEAD_OVER_READ:

            /* Update the data index as complete in case
                 * of over read */
            data_index = dev->fifo->length;
            break;
        default:

            /* Update the data index as complete in case of
                 * getting other headers like 0x00 */
            data_index = dev->fifo->length;
            break;
        }
        if(*aux_length == aux_index) {
            /*Number of frames to read completed*/
            break;
        }
    }

    /* Update number of aux data read */
    *aux_length = aux_index;

    /* Update the aux frame index */
    dev->fifo->aux_byte_start_idx = data_index;
}

/*!
 *  @brief This API checks the presence of non-valid frames in the read fifo data.
 */
static void check_frame_validity(uint16_t* data_index, const struct bmi160_dev* dev) {
    if((*data_index + 2) < dev->fifo->length) {
        /* Check if FIFO is empty */
        if((dev->fifo->data[*data_index] == FIFO_CONFIG_MSB_CHECK) &&
           (dev->fifo->data[*data_index + 1] == FIFO_CONFIG_LSB_CHECK)) {
            /*Update the data index as complete*/
            *data_index = dev->fifo->length;
        }
    }
}

/*!
 *  @brief This API is used to move the data index ahead of the
 *  current_frame_length parameter when unnecessary FIFO data appears while
 *  extracting the user specified data.
 */
static void move_next_frame(
    uint16_t* data_index,
    uint8_t current_frame_length,
    const struct bmi160_dev* dev) {
    /*Partial read, then move the data index to last data*/
    if((*data_index + current_frame_length) > dev->fifo->length) {
        /*Update the data index as complete*/
        *data_index = dev->fifo->length;
    } else {
        /*Move the data index to next frame*/
        *data_index = *data_index + current_frame_length;
    }
}

/*!
 *  @brief This API is used to parse and store the sensor time from the
 *  FIFO data in the structure instance dev.
 */
static void unpack_sensortime_frame(uint16_t* data_index, const struct bmi160_dev* dev) {
    uint32_t sensor_time_byte3 = 0;
    uint16_t sensor_time_byte2 = 0;
    uint8_t sensor_time_byte1 = 0;

    /*Partial read, then move the data index to last data*/
    if((*data_index + BMI160_SENSOR_TIME_LENGTH) > dev->fifo->length) {
        /*Update the data index as complete*/
        *data_index = dev->fifo->length;
    } else {
        sensor_time_byte3 = dev->fifo->data[(*data_index) + BMI160_SENSOR_TIME_MSB_BYTE] << 16;
        sensor_time_byte2 = dev->fifo->data[(*data_index) + BMI160_SENSOR_TIME_XLSB_BYTE] << 8;
        sensor_time_byte1 = dev->fifo->data[(*data_index)];

        /* Sensor time */
        dev->fifo->sensor_time =
            (uint32_t)(sensor_time_byte3 | sensor_time_byte2 | sensor_time_byte1);
        *data_index = (*data_index) + BMI160_SENSOR_TIME_LENGTH;
    }
}

/*!
 *  @brief This API is used to parse and store the skipped_frame_count from
 *  the FIFO data in the structure instance dev.
 */
static void unpack_skipped_frame(uint16_t* data_index, const struct bmi160_dev* dev) {
    /*Partial read, then move the data index to last data*/
    if(*data_index >= dev->fifo->length) {
        /*Update the data index as complete*/
        *data_index = dev->fifo->length;
    } else {
        dev->fifo->skipped_frame_count = dev->fifo->data[*data_index];

        /*Move the data index*/
        *data_index = (*data_index) + 1;
    }
}

/*!
 *  @brief This API is used to get the FOC status from the sensor
 */
static int8_t get_foc_status(uint8_t* foc_status, struct bmi160_dev const* dev) {
    int8_t rslt;
    uint8_t data;

    /* Read the FOC status from sensor */
    rslt = bmi160_get_regs(BMI160_STATUS_ADDR, &data, 1, dev);
    if(rslt == BMI160_OK) {
        /* Get the foc_status bit */
        *foc_status = BMI160_GET_BITS(data, BMI160_FOC_STATUS);
    }

    return rslt;
}

/*!
 *  @brief This API is used to configure the offset enable bits in the sensor
 */
static int8_t
    configure_offset_enable(const struct bmi160_foc_conf* foc_conf, struct bmi160_dev const* dev) {
    int8_t rslt;
    uint8_t data;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);
    if(rslt != BMI160_OK) {
        rslt = BMI160_E_NULL_PTR;
    } else {
        /* Read the FOC config from the sensor */
        rslt = bmi160_get_regs(BMI160_OFFSET_CONF_ADDR, &data, 1, dev);
        if(rslt == BMI160_OK) {
            /* Set the offset enable/disable for gyro */
            data = BMI160_SET_BITS(data, BMI160_GYRO_OFFSET_EN, foc_conf->gyro_off_en);

            /* Set the offset enable/disable for accel */
            data = BMI160_SET_BITS(data, BMI160_ACCEL_OFFSET_EN, foc_conf->acc_off_en);

            /* Set the offset config in the sensor */
            rslt = bmi160_set_regs(BMI160_OFFSET_CONF_ADDR, &data, 1, dev);
        }
    }

    return rslt;
}

static int8_t trigger_foc(struct bmi160_offsets* offset, struct bmi160_dev const* dev) {
    int8_t rslt;
    uint8_t foc_status = BMI160_ENABLE;
    uint8_t cmd = BMI160_START_FOC_CMD;
    uint8_t timeout = 0;
    uint8_t data_array[20];

    /* Start the FOC process */
    rslt = bmi160_set_regs(BMI160_COMMAND_REG_ADDR, &cmd, 1, dev);
    if(rslt == BMI160_OK) {
        /* Check the FOC status*/
        rslt = get_foc_status(&foc_status, dev);

        if((rslt != BMI160_OK) || (foc_status != BMI160_ENABLE)) {
            while((foc_status != BMI160_ENABLE) && (timeout < 11)) {
                /* Maximum time of 250ms is given in 10
                 * steps of 25ms each - 250ms refer datasheet 2.9.1 */
                dev->delay_ms(25);

                /* Check the FOC status*/
                rslt = get_foc_status(&foc_status, dev);
                timeout++;
            }

            if((rslt == BMI160_OK) && (foc_status == BMI160_ENABLE)) {
                /* Get offset values from sensor */
                rslt = bmi160_get_offsets(offset, dev);
            } else {
                /* FOC failure case */
                rslt = BMI160_E_FOC_FAILURE;
            }
        }

        if(rslt == BMI160_OK) {
            /* Read registers 0x04-0x17 */
            rslt = bmi160_get_regs(BMI160_GYRO_DATA_ADDR, data_array, 20, dev);
        }
    }

    return rslt;
}
