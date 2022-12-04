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
* @file       bmi160.h
* @date       2021-10-05
* @version    v3.9.2
*
*/

/*!
 * @defgroup bmi160 BMI160
 */

#ifndef BMI160_H_
#define BMI160_H_

/*************************** C++ guard macro *****************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "bmi160_defs.h"
#ifdef __KERNEL__
#include <bmi160_math.h>
#else
#include <math.h>
#include <string.h>
#include <stdlib.h>
#endif

/*********************** User function prototypes ************************/

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiInit Initialization
 * @brief Initialize the sensor and device structure
 */

/*!
 * \ingroup bmi160ApiInit
 * \page bmi160_api_bmi160_init bmi160_init
 * \code
 * int8_t bmi160_init(struct bmi160_dev *dev);
 * \endcode
 * @details This API is the entry point for sensor.It performs
 *  the selection of I2C/SPI read mechanism according to the
 *  selected interface and reads the chip-id of bmi160 sensor.
 *
 *  @param[in,out] dev : Structure instance of bmi160_dev
 *  @note : Refer user guide for detailed info.
 *
 *  @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_init(struct bmi160_dev* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiRegs Registers
 * @brief Read data from the given register address of sensor
 */

/*!
 * \ingroup bmi160ApiRegs
 * \page bmi160_api_bmi160_get_regs bmi160_get_regs
 * \code
 * int8_t bmi160_get_regs(uint8_t reg_addr, uint8_t *data, uint16_t len, const struct bmi160_dev *dev);
 * \endcode
 * @details This API reads the data from the given register address of sensor.
 *
 * @param[in] reg_addr  : Register address from where the data to be read
 * @param[out] data     : Pointer to data buffer to store the read data.
 * @param[in] len       : No of bytes of data to be read.
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @note For most of the registers auto address increment applies, with the
 * exception of a few special registers, which trap the address. For e.g.,
 * Register address - 0x24(BMI160_FIFO_DATA_ADDR)
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t
    bmi160_get_regs(uint8_t reg_addr, uint8_t* data, uint16_t len, const struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiRegs
 * \page bmi160_api_bmi160_set_regs bmi160_set_regs
 * \code
 * int8_t bmi160_set_regs(uint8_t reg_addr, uint8_t *data, uint16_t len, const struct bmi160_dev *dev);
 * \endcode
 * @details This API writes the given data to the register address
 * of sensor.
 *
 * @param[in] reg_addr  : Register address from where the data to be written.
 * @param[in] data      : Pointer to data buffer which is to be written
 * in the sensor.
 * @param[in] len       : No of bytes of data to write..
 * @param[in] dev       : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t
    bmi160_set_regs(uint8_t reg_addr, uint8_t* data, uint16_t len, const struct bmi160_dev* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiSoftreset Soft reset
 * @brief Perform soft reset of the sensor
 */

/*!
 * \ingroup bmi160ApiSoftreset
 * \page bmi160_api_bmi160_soft_reset bmi160_soft_reset
 * \code
 * int8_t bmi160_soft_reset(struct bmi160_dev *dev);
 * \endcode
 * @details This API resets and restarts the device.
 * All register values are overwritten with default parameters.
 *
 * @param[in] dev  : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_soft_reset(struct bmi160_dev* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiConfig Configuration
 * @brief Configuration of the sensor
 */

/*!
 * \ingroup bmi160ApiConfig
 * \page bmi160_api_bmi160_set_sens_conf bmi160_set_sens_conf
 * \code
 * int8_t bmi160_set_sens_conf(struct bmi160_dev *dev);
 * \endcode
 * @details This API configures the power mode, range and bandwidth
 * of sensor.
 *
 * @param[in] dev    : Structure instance of bmi160_dev.
 * @note : Refer user guide for detailed info.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_set_sens_conf(struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiConfig
 * \page bmi160_api_bmi160_get_sens_conf bmi160_get_sens_conf
 * \code
 * int8_t bmi160_get_sens_conf(struct bmi160_dev *dev);
 * \endcode
 * @details This API gets accel and gyro configurations.
 *
 * @param[out] dev    : Structure instance of bmi160_dev.
 * @note : Refer user guide for detailed info.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_get_sens_conf(struct bmi160_dev* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiPowermode Power mode
 * @brief Set / Get power mode of the sensor
 */

/*!
 * \ingroup bmi160ApiPowermode
 * \page bmi160_api_bmi160_set_power_mode bmi160_set_power_mode
 * \code
 * int8_t bmi160_set_power_mode(struct bmi160_dev *dev);
 * \endcode
 * @details This API sets the power mode of the sensor.
 *
 * @param[in] dev  : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_set_power_mode(struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiPowermode
 * \page bmi160_api_bmi160_get_power_mode bmi160_get_power_mode
 * \code
 * int8_t bmi160_get_power_mode(struct bmi160_dev *dev);
 * \endcode
 * @details This API gets the power mode of the sensor.
 *
 * @param[in] dev         : Structure instance of bmi160_dev
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_get_power_mode(struct bmi160_dev* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiData Sensor Data
 * @brief Read sensor data
 */

/*!
 * \ingroup bmi160ApiData
 * \page bmi160_api_bmi160_get_sensor_data bmi160_get_sensor_data
 * \code
 * int8_t bmi160_get_sensor_data(uint8_t select_sensor,
 *                             struct bmi160_sensor_data *accel,
 *                             struct bmi160_sensor_data *gyro,
 *                             const struct bmi160_dev *dev);
 *
 * \endcode
 * @details This API reads sensor data, stores it in
 * the bmi160_sensor_data structure pointer passed by the user.
 * The user can ask for accel data ,gyro data or both sensor
 * data using bmi160_select_sensor enum
 *
 * @param[in] select_sensor    : enum to choose accel,gyro or both sensor data
 * @param[out] accel    : Structure pointer to store accel data
 * @param[out] gyro     : Structure pointer to store gyro data
 * @param[in] dev       : Structure instance of bmi160_dev.
 * @note : Refer user guide for detailed info.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_get_sensor_data(
    uint8_t select_sensor,
    struct bmi160_sensor_data* accel,
    struct bmi160_sensor_data* gyro,
    const struct bmi160_dev* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiInt Interrupt configuration
 * @brief Set interrupt configuration of the sensor
 */

/*!
 * \ingroup bmi160ApiInt
 * \page bmi160_api_bmi160_set_int_config bmi160_set_int_config
 * \code
 * int8_t bmi160_set_int_config(struct bmi160_int_settg *int_config, struct bmi160_dev *dev);
 * \endcode
 * @details This API configures the necessary interrupt based on
 *  the user settings in the bmi160_int_settg structure instance.
 *
 * @param[in] int_config  : Structure instance of bmi160_int_settg.
 * @param[in] dev         : Structure instance of bmi160_dev.
 * @note : Refer user guide for detailed info.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_set_int_config(struct bmi160_int_settg* int_config, struct bmi160_dev* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiStepC Step counter
 * @brief Step counter operations
 */

/*!
 * \ingroup bmi160ApiStepC
 * \page bmi160_api_bmi160_set_step_counter bmi160_set_step_counter
 * \code
 * int8_t bmi160_set_step_counter(uint8_t step_cnt_enable, const struct bmi160_dev *dev);
 * \endcode
 * @details This API enables the step counter feature.
 *
 * @param[in] step_cnt_enable   : value to enable or disable
 * @param[in] dev       : Structure instance of bmi160_dev.
 * @note : Refer user guide for detailed info.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_set_step_counter(uint8_t step_cnt_enable, const struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiStepC
 * \page bmi160_api_bmi160_read_step_counter bmi160_read_step_counter
 * \code
 * int8_t bmi160_read_step_counter(uint16_t *step_val, const struct bmi160_dev *dev);
 * \endcode
 * @details This API reads the step counter value.
 *
 * @param[in] step_val    : Pointer to store the step counter value.
 * @param[in] dev         : Structure instance of bmi160_dev.
 * @note : Refer user guide for detailed info.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_read_step_counter(uint16_t* step_val, const struct bmi160_dev* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiAux Auxiliary sensor
 * @brief Auxiliary sensor operations
 */

/*!
 * \ingroup bmi160ApiAux
 * \page bmi160_api_bmi160_aux_read bmi160_aux_read
 * \code
 * int8_t bmi160_aux_read(uint8_t reg_addr, uint8_t *aux_data, uint16_t len, const struct bmi160_dev *dev);
 * \endcode
 * @details This API reads the mention no of byte of data from the given
 * register address of auxiliary sensor.
 *
 * @param[in] reg_addr    : Address of register to read.
 * @param[in] aux_data    : Pointer to store the read data.
 * @param[in] len     : No of bytes to read.
 * @param[in] dev         : Structure instance of bmi160_dev.
 * @note : Refer user guide for detailed info.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_aux_read(
    uint8_t reg_addr,
    uint8_t* aux_data,
    uint16_t len,
    const struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiAux
 * \page bmi160_api_bmi160_aux_write bmi160_aux_write
 * \code
 * int8_t bmi160_aux_write(uint8_t reg_addr, uint8_t *aux_data, uint16_t len, const struct bmi160_dev *dev);
 * \endcode
 * @details This API writes the mention no of byte of data to the given
 * register address of auxiliary sensor.
 *
 * @param[in] reg_addr    : Address of register to write.
 * @param[in] aux_data    : Pointer to write data.
 * @param[in] len     : No of bytes to write.
 * @param[in] dev         : Structure instance of bmi160_dev.
 * @note : Refer user guide for detailed info.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_aux_write(
    uint8_t reg_addr,
    uint8_t* aux_data,
    uint16_t len,
    const struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiAux
 * \page bmi160_api_bmi160_aux_init bmi160_aux_init
 * \code
 * int8_t bmi160_aux_init(const struct bmi160_dev *dev);
 * \endcode
 * @details This API initialize the auxiliary sensor
 * in order to access it.
 *
 * @param[in] dev         : Structure instance of bmi160_dev.
 * @note : Refer user guide for detailed info.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_aux_init(const struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiAux
 * \page bmi160_api_bmi160_set_aux_auto_mode bmi160_set_aux_auto_mode
 * \code
 * int8_t bmi160_set_aux_auto_mode(uint8_t *data_addr, struct bmi160_dev *dev);
 * \endcode
 * @details This API is used to setup the auxiliary sensor of bmi160 in auto mode
 * Thus enabling the auto update of 8 bytes of data from auxiliary sensor
 * to BMI160 register address 0x04 to 0x0B
 *
 * @param[in] data_addr    : Starting address of aux. sensor's data register
 *                           (BMI160 registers 0x04 to 0x0B will be updated
 *                           with 8 bytes of data from auxiliary sensor
 *                           starting from this register address.)
 * @param[in] dev      : Structure instance of bmi160_dev.
 *
 * @note : Set the value of auxiliary polling rate by setting
 *         dev->aux_cfg.aux_odr to the required value from the table
 *         before calling this API
 *
 *@verbatim
 *   dev->aux_cfg.aux_odr  |   Auxiliary ODR (Hz)
 *  -----------------------|-----------------------
 *  BMI160_AUX_ODR_0_78HZ  |        25/32
 *  BMI160_AUX_ODR_1_56HZ  |        25/16
 *  BMI160_AUX_ODR_3_12HZ  |        25/8
 *  BMI160_AUX_ODR_6_25HZ  |        25/4
 *  BMI160_AUX_ODR_12_5HZ  |        25/2
 *  BMI160_AUX_ODR_25HZ    |        25
 *  BMI160_AUX_ODR_50HZ    |        50
 *  BMI160_AUX_ODR_100HZ   |        100
 *  BMI160_AUX_ODR_200HZ   |        200
 *  BMI160_AUX_ODR_400HZ   |        400
 *  BMI160_AUX_ODR_800HZ   |        800
 *@endverbatim
 *
 * @note : Other values of  dev->aux_cfg.aux_odr are reserved and not for use
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_set_aux_auto_mode(uint8_t* data_addr, struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiAux
 * \page bmi160_api_bmi160_config_aux_mode bmi160_config_aux_mode
 * \code
 * int8_t bmi160_config_aux_mode(const struct bmi160_dev *dev);
 * \endcode
 * @details This API configures the 0x4C register and settings like
 * Auxiliary sensor manual enable/ disable and aux burst read length.
 *
 * @param[in] dev    : Structure instance of bmi160_dev.
 *
 * @return Result of API execution status
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_config_aux_mode(const struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiAux
 * \page bmi160_api_bmi160_read_aux_data_auto_mode bmi160_read_aux_data_auto_mode
 * \code
 * int8_t bmi160_read_aux_data_auto_mode(uint8_t *aux_data, const struct bmi160_dev *dev);
 * \endcode
 * @details This API is used to read the raw uncompensated auxiliary sensor
 * data of 8 bytes from BMI160 register address 0x04 to 0x0B
 *
 * @param[in] aux_data       : Pointer to user array of length 8 bytes
 *                             Ensure that the aux_data array is of
 *                             length 8 bytes
 * @param[in] dev        : Structure instance of bmi160_dev
 *
 * @retval zero -> Success  / -ve value -> Error
 * @retval Zero Success
 * @retval Negative Error
 */
int8_t bmi160_read_aux_data_auto_mode(uint8_t* aux_data, const struct bmi160_dev* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiSelfTest Self test
 * @brief Perform self test of the sensor
 */

/*!
 * \ingroup bmi160ApiSelfTest
 * \page bmi160_api_bmi160_perform_self_test bmi160_perform_self_test
 * \code
 * int8_t bmi160_perform_self_test(uint8_t select_sensor, struct bmi160_dev *dev);
 * \endcode
 * @details This is used to perform self test of accel/gyro of the BMI160 sensor
 *
 * @param[in] select_sensor  : enum to choose accel or gyro for self test
 * @param[in] dev            : Structure instance of bmi160_dev
 *
 * @note self test can be performed either for accel/gyro at any instant.
 *
 *@verbatim
 *     value of select_sensor       |  Inference
 *----------------------------------|--------------------------------
 *   BMI160_ACCEL_ONLY              | Accel self test enabled
 *   BMI160_GYRO_ONLY               | Gyro self test enabled
 *   BMI160_BOTH_ACCEL_AND_GYRO     | NOT TO BE USED
 *@endverbatim
 *
 * @note The return value of this API gives us the result of self test.
 *
 * @note Performing self test does soft reset of the sensor, User can
 * set the desired settings after performing the self test.
 *
 * @return Result of API execution status
 * @retval  BMI160_OK                       Self test success
 * @retval  BMI160_W_GYRO_SELF_TEST_FAIL    Gyro self test fail
 * @retval  BMI160_W_ACCEl_SELF_TEST_FAIL   Accel self test fail
 */
int8_t bmi160_perform_self_test(uint8_t select_sensor, struct bmi160_dev* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiFIFO FIFO
 * @brief FIFO operations of the sensor
 */

/*!
 * \ingroup bmi160ApiFIFO
 * \page bmi160_api_bmi160_get_fifo_data bmi160_get_fifo_data
 * \code
 * int8_t bmi160_get_fifo_data(struct bmi160_dev const *dev);
 * \endcode
 * @details This API reads data from the fifo buffer.
 *
 *  @note User has to allocate the FIFO buffer along with
 *  corresponding fifo length from his side before calling this API
 *  as mentioned in the readme.md
 *
 *  @note User must specify the number of bytes to read from the FIFO in
 *  dev->fifo->length , It will be updated by the number of bytes actually
 *  read from FIFO after calling this API
 *
 *  @param[in] dev     : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval Zero Success
 *  @retval Negative Error
 */
int8_t bmi160_get_fifo_data(struct bmi160_dev const* dev);

/*!
 * \ingroup bmi160ApiFIFO
 * \page bmi160_api_bmi160_set_fifo_flush bmi160_set_fifo_flush
 * \code
 * int8_t bmi160_set_fifo_flush(const struct bmi160_dev *dev);
 * \endcode
 * @details This API writes fifo_flush command to command register.This
 *  action clears all data in the Fifo without changing fifo configuration
 *  settings.
 *
 *  @param[in] dev     : Structure instance of bmi160_dev
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 *
 */
int8_t bmi160_set_fifo_flush(const struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiFIFO
 * \page bmi160_api_bmi160_set_fifo_config bmi160_set_fifo_config
 * \code
 * int8_t bmi160_set_fifo_config(uint8_t config, uint8_t enable, struct bmi160_dev const *dev);
 * \endcode
 * @details This API sets the FIFO configuration in the sensor.
 *
 *  @param[in] config : variable used to specify the FIFO
 *  configurations which are to be enabled or disabled in the sensor.
 *
 *  @note : User can set either set one or more or all FIFO configurations
 *  by ORing the below mentioned macros.
 *
 *@verbatim
 *      config                  |   Value
 *      ------------------------|---------------------------
 *      BMI160_FIFO_TIME        |   0x02
 *      BMI160_FIFO_TAG_INT2    |   0x04
 *      BMI160_FIFO_TAG_INT1    |   0x08
 *      BMI160_FIFO_HEADER      |   0x10
 *      BMI160_FIFO_AUX         |   0x20
 *      BMI160_FIFO_ACCEL       |   0x40
 *      BMI160_FIFO_GYRO        |   0x80
 *@endverbatim
 *
 *  @param[in] enable : Parameter used to enable or disable the above
 *  FIFO configuration
 *  @param[in] dev : Structure instance of bmi160_dev.
 *
 *  @return status of bus communication result
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 *
 */
int8_t bmi160_set_fifo_config(uint8_t config, uint8_t enable, struct bmi160_dev const* dev);

/*!
 * \ingroup bmi160ApiFIFO
 * \page bmi160_api_bmi160_set_fifo_down bmi160_set_fifo_down
 * \code
 * int8_t bmi160_set_fifo_down(uint8_t fifo_down, const struct bmi160_dev *dev);
 * \endcode
 * @details This API is used to configure the down sampling ratios of
 *  the accel and gyro data for FIFO.Also, it configures filtered or
 *  pre-filtered data for the fifo for accel and gyro.
 *
 *  @param[in] fifo_down : variable used to specify the FIFO down
 *  configurations which are to be enabled or disabled in the sensor.
 *
 *  @note The user must select one among the following macros to
 *  select down-sampling ratio for accel
 *
 *@verbatim
 *      config                               |   Value
 *      -------------------------------------|---------------------------
 *      BMI160_ACCEL_FIFO_DOWN_ZERO          |   0x00
 *      BMI160_ACCEL_FIFO_DOWN_ONE           |   0x10
 *      BMI160_ACCEL_FIFO_DOWN_TWO           |   0x20
 *      BMI160_ACCEL_FIFO_DOWN_THREE         |   0x30
 *      BMI160_ACCEL_FIFO_DOWN_FOUR          |   0x40
 *      BMI160_ACCEL_FIFO_DOWN_FIVE          |   0x50
 *      BMI160_ACCEL_FIFO_DOWN_SIX           |   0x60
 *      BMI160_ACCEL_FIFO_DOWN_SEVEN         |   0x70
 *@endverbatim
 *
 *  @note The user must select one among the following macros to
 *  select down-sampling ratio for gyro
 *
 *@verbatim
 *      config                               |   Value
 *      -------------------------------------|---------------------------
 *      BMI160_GYRO_FIFO_DOWN_ZERO           |   0x00
 *      BMI160_GYRO_FIFO_DOWN_ONE            |   0x01
 *      BMI160_GYRO_FIFO_DOWN_TWO            |   0x02
 *      BMI160_GYRO_FIFO_DOWN_THREE          |   0x03
 *      BMI160_GYRO_FIFO_DOWN_FOUR           |   0x04
 *      BMI160_GYRO_FIFO_DOWN_FIVE           |   0x05
 *      BMI160_GYRO_FIFO_DOWN_SIX            |   0x06
 *      BMI160_GYRO_FIFO_DOWN_SEVEN          |   0x07
 *@endverbatim
 *
 *  @note The user can enable filtered accel data by the following macro
 *
 *@verbatim
 *      config                               |   Value
 *      -------------------------------------|---------------------------
 *      BMI160_ACCEL_FIFO_FILT_EN            |   0x80
 *@endverbatim
 *
 *  @note The user can enable filtered gyro data by the following macro
 *
 *@verbatim
 *      config                               |   Value
 *      -------------------------------------|---------------------------
 *      BMI160_GYRO_FIFO_FILT_EN             |   0x08
 *@endverbatim
 *
 *  @note : By ORing the above mentioned macros, the user can select
 *  the required FIFO down config settings
 *
 *  @param[in] dev : Structure instance of bmi160_dev.
 *
 *  @return status of bus communication result
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 *
 */
int8_t bmi160_set_fifo_down(uint8_t fifo_down, const struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiFIFO
 * \page bmi160_api_bmi160_set_fifo_wm bmi160_set_fifo_wm
 * \code
 * int8_t bmi160_set_fifo_wm(uint8_t fifo_wm, const struct bmi160_dev *dev);
 * \endcode
 * @details This API sets the FIFO watermark level in the sensor.
 *
 *  @note The FIFO watermark is issued when the FIFO fill level is
 *  equal or above the watermark level and units of watermark is 4 bytes.
 *
 *  @param[in]  fifo_wm        : Variable used to set the FIFO water mark level
 *  @param[in]  dev            : Structure instance of bmi160_dev
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 *
 */
int8_t bmi160_set_fifo_wm(uint8_t fifo_wm, const struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiFIFO
 * \page bmi160_api_bmi160_extract_accel bmi160_extract_accel
 * \code
 * int8_t bmi160_extract_accel(struct bmi160_sensor_data *accel_data, uint8_t *accel_length, struct bmi160_dev const
 **dev);
 * \endcode
 * @details This API parses and extracts the accelerometer frames from
 *  FIFO data read by the "bmi160_get_fifo_data" API and stores it in
 *  the "accel_data" structure instance.
 *
 *  @note The bmi160_extract_accel API should be called only after
 *  reading the FIFO data by calling the bmi160_get_fifo_data() API.
 *
 *  @param[out] accel_data    : Structure instance of bmi160_sensor_data
 *                              where the accelerometer data in FIFO is stored.
 *  @param[in,out] accel_length  : Number of valid accelerometer frames
 *                              (x,y,z axes data) read out from fifo.
 *  @param[in] dev            : Structure instance of bmi160_dev.
 *
 *  @note accel_length is updated with the number of valid accelerometer
 *  frames extracted from fifo (1 accel frame   = 6 bytes) at the end of
 *  execution of this API.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 *
 */
int8_t bmi160_extract_accel(
    struct bmi160_sensor_data* accel_data,
    uint8_t* accel_length,
    struct bmi160_dev const* dev);

/*!
 * \ingroup bmi160ApiFIFO
 * \page bmi160_api_bmi160_extract_gyro bmi160_extract_gyro
 * \code
 * int8_t bmi160_extract_gyro(struct bmi160_sensor_data *gyro_data, uint8_t *gyro_length, struct bmi160_dev const *dev);
 * \endcode
 * @details This API parses and extracts the gyro frames from
 *  FIFO data read by the "bmi160_get_fifo_data" API and stores it in
 *  the "gyro_data" structure instance.
 *
 *  @note The bmi160_extract_gyro API should be called only after
 *  reading the FIFO data by calling the bmi160_get_fifo_data() API.
 *
 *  @param[out] gyro_data    : Structure instance of bmi160_sensor_data
 *                             where the gyro data in FIFO is stored.
 *  @param[in,out] gyro_length  : Number of valid gyro frames
 *                             (x,y,z axes data) read out from fifo.
 *  @param[in] dev           : Structure instance of bmi160_dev.
 *
 *  @note gyro_length is updated with the number of valid gyro
 *  frames extracted from fifo (1 gyro frame   = 6 bytes) at the end of
 *  execution of this API.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 *
 */
int8_t bmi160_extract_gyro(
    struct bmi160_sensor_data* gyro_data,
    uint8_t* gyro_length,
    struct bmi160_dev const* dev);

/*!
 * \ingroup bmi160ApiFIFO
 * \page bmi160_api_bmi160_extract_aux bmi160_extract_aux
 * \code
 * int8_t bmi160_extract_aux(struct bmi160_aux_data *aux_data, uint8_t *aux_len, struct bmi160_dev const *dev);
 * \endcode
 * @details This API parses and extracts the aux frames from
 *  FIFO data read by the "bmi160_get_fifo_data" API and stores it in
 *  the bmi160_aux_data structure instance.
 *
 *  @note The bmi160_extract_aux API should be called only after
 *  reading the FIFO data by calling the bmi160_get_fifo_data() API.
 *
 *  @param[out] aux_data    : Structure instance of bmi160_aux_data
 *                            where the aux data in FIFO is stored.
 *  @param[in,out] aux_len  : Number of valid aux frames (8bytes)
 *                            read out from FIFO.
 *  @param[in] dev          : Structure instance of bmi160_dev.
 *
 *  @note aux_len is updated with the number of valid aux
 *  frames extracted from fifo (1 aux frame = 8 bytes) at the end of
 *  execution of this API.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 *
 */
int8_t bmi160_extract_aux(
    struct bmi160_aux_data* aux_data,
    uint8_t* aux_len,
    struct bmi160_dev const* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiFOC FOC
 * @brief Start FOC of accel and gyro sensors
 */

/*!
 * \ingroup bmi160ApiFOC
 * \page bmi160_api_bmi160_start_foc bmi160_start_foc
 * \code
 * int8_t bmi160_start_foc(const struct bmi160_foc_conf *foc_conf,
 * \endcode
 * @details This API starts the FOC of accel and gyro
 *
 *  @note FOC should not be used in low-power mode of sensor
 *
 *  @note Accel FOC targets values of +1g , 0g , -1g
 *  Gyro FOC always targets value of 0 dps
 *
 *  @param[in] foc_conf    : Structure instance of bmi160_foc_conf which
 *                                   has the FOC configuration
 *  @param[in,out] offset  : Structure instance to store Offset
 *                                   values read from sensor
 *  @param[in] dev         : Structure instance of bmi160_dev.
 *
 *  @note Pre-requisites for triggering FOC in accel , Set the following,
 *   Enable the acc_off_en
 *       Ex :  foc_conf.acc_off_en = BMI160_ENABLE;
 *
 *   Set the desired target values of FOC to each axes (x,y,z) by using the
 *   following macros
 *       - BMI160_FOC_ACCEL_DISABLED
 *       - BMI160_FOC_ACCEL_POSITIVE_G
 *       - BMI160_FOC_ACCEL_NEGATIVE_G
 *       - BMI160_FOC_ACCEL_0G
 *
 *   Ex : foc_conf.foc_acc_x  = BMI160_FOC_ACCEL_0G;
 *        foc_conf.foc_acc_y  = BMI160_FOC_ACCEL_0G;
 *        foc_conf.foc_acc_z  = BMI160_FOC_ACCEL_POSITIVE_G;
 *
 *  @note Pre-requisites for triggering FOC in gyro ,
 *  Set the following parameters,
 *
 *   Ex : foc_conf.foc_gyr_en = BMI160_ENABLE;
 *        foc_conf.gyro_off_en = BMI160_ENABLE;
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 */
int8_t bmi160_start_foc(
    const struct bmi160_foc_conf* foc_conf,
    struct bmi160_offsets* offset,
    struct bmi160_dev const* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiOffsets Offsets
 * @brief Set / Get offset values of accel and gyro sensors
 */

/*!
 * \ingroup bmi160ApiOffsets
 * \page bmi160_api_bmi160_get_offsets bmi160_get_offsets
 * \code
 * int8_t bmi160_get_offsets(struct bmi160_offsets *offset, const struct bmi160_dev *dev);
 * \endcode
 * @details This API reads and stores the offset values of accel and gyro
 *
 *  @param[in,out] offset : Structure instance of bmi160_offsets in which
 *                          the offset values are read and stored
 *  @param[in] dev        : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 */
int8_t bmi160_get_offsets(struct bmi160_offsets* offset, const struct bmi160_dev* dev);

/*!
 * \ingroup bmi160ApiOffsets
 * \page bmi160_api_bmi160_set_offsets bmi160_set_offsets
 * \code
 * int8_t bmi160_set_offsets(const struct bmi160_foc_conf *foc_conf,
 *                         const struct bmi160_offsets *offset,
 *                         struct bmi160_dev const *dev);
 * \endcode
 * @details This API writes the offset values of accel and gyro to
 *  the sensor but these values will be reset on POR or soft reset.
 *
 *  @param[in] foc_conf    : Structure instance of bmi160_foc_conf which
 *                                   has the FOC configuration
 *  @param[in] offset      : Structure instance in which user updates offset
 *                            values which are to be written in the sensor
 *  @param[in] dev         : Structure instance of bmi160_dev.
 *
 *  @note Offsets can be set by user like offset->off_acc_x = 10;
 *  where 1LSB = 3.9mg and for gyro 1LSB = 0.061degrees/second
 *
 * @note BMI160 offset values for xyz axes of accel should be within range of
 *  BMI160_ACCEL_MIN_OFFSET (-128) to BMI160_ACCEL_MAX_OFFSET (127)
 *
 * @note BMI160 offset values for xyz axes of gyro should be within range of
 *  BMI160_GYRO_MIN_OFFSET (-512) to BMI160_GYRO_MAX_OFFSET (511)
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 */
int8_t bmi160_set_offsets(
    const struct bmi160_foc_conf* foc_conf,
    const struct bmi160_offsets* offset,
    struct bmi160_dev const* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiNVM NVM
 * @brief Write image registers values to NVM
 */

/*!
 * \ingroup bmi160ApiNVM
 * \page bmi160_api_bmi160_update_nvm bmi160_update_nvm
 * \code
 * int8_t bmi160_update_nvm(struct bmi160_dev const *dev);
 * \endcode
 * @details This API writes the image registers values to NVM which is
 *  stored even after POR or soft reset
 *
 *  @param[in] dev         : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 */
int8_t bmi160_update_nvm(struct bmi160_dev const* dev);

/**
 * \ingroup bmi160
 * \defgroup bmi160ApiInts Interrupt status
 * @brief Read interrupt status from the sensor
 */

/*!
 * \ingroup bmi160ApiInts
 * \page bmi160_api_bmi160_get_int_status bmi160_get_int_status
 * \code
 * int8_t bmi160_get_int_status(enum bmi160_int_status_sel int_status_sel,
 *                            union bmi160_int_status *int_status,
 *                            struct bmi160_dev const *dev);
 * \endcode
 * @details This API gets the interrupt status from the sensor.
 *
 *  @param[in] int_status_sel       : Enum variable to select either individual or all the
 *  interrupt status bits.
 *  @param[in] int_status           : pointer variable to get the interrupt status
 *  from the sensor.
 *  param[in] dev                   : Structure instance of bmi160_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 */
int8_t bmi160_get_int_status(
    enum bmi160_int_status_sel int_status_sel,
    union bmi160_int_status* int_status,
    struct bmi160_dev const* dev);

/*************************** C++ guard macro *****************************/
#ifdef __cplusplus
}
#endif

#endif /* BMI160_H_ */
