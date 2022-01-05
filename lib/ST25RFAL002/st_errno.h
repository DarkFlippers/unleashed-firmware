
/******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2018 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty
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
 *      PROJECT:   STxxxx firmware
 *      LANGUAGE:  ISO C99
 */

/*! \file st_errno.h
 *
 *  \author 
 *
 *  \brief Main error codes
 *
 */

#ifndef ST_ERRNO_H
#define ST_ERRNO_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/

#include <stdint.h>

/*
******************************************************************************
* GLOBAL DATA TYPES
******************************************************************************
*/

typedef uint16_t ReturnCode; /*!< Standard Return Code type from function. */

/*
******************************************************************************
* DEFINES
******************************************************************************
*/

/*
 * Error codes to be used within the application.
 * They are represented by an uint8_t
 */
enum {
    ERR_NONE = 0, /*!< no error occurred */
    ERR_NOMEM = 1, /*!< not enough memory to perform the requested operation */
    ERR_BUSY = 2, /*!< device or resource busy */
    ERR_IO = 3, /*!< generic IO error */
    ERR_TIMEOUT = 4, /*!< error due to timeout */
    ERR_REQUEST = 5, /*!< invalid request or requested function can't be executed at the moment */
    ERR_NOMSG = 6, /*!< No message of desired type */
    ERR_PARAM = 7, /*!< Parameter error */
    ERR_SYSTEM = 8, /*!< System error */
    ERR_FRAMING = 9, /*!< Framing error */
    ERR_OVERRUN = 10, /*!< lost one or more received bytes */
    ERR_PROTO = 11, /*!< protocol error */
    ERR_INTERNAL = 12, /*!< Internal Error */
    ERR_AGAIN = 13, /*!< Call again */
    ERR_MEM_CORRUPT = 14, /*!< memory corruption */
    ERR_NOT_IMPLEMENTED = 15, /*!< not implemented */
    ERR_PC_CORRUPT =
        16, /*!< Program Counter has been manipulated or spike/noise trigger illegal operation */
    ERR_SEND = 17, /*!< error sending*/
    ERR_IGNORE = 18, /*!< indicates error detected but to be ignored */
    ERR_SEMANTIC = 19, /*!< indicates error in state machine (unexpected cmd) */
    ERR_SYNTAX = 20, /*!< indicates error in state machine (unknown cmd) */
    ERR_CRC = 21, /*!< crc error */
    ERR_NOTFOUND = 22, /*!< transponder not found */
    ERR_NOTUNIQUE = 23, /*!< transponder not unique - more than one transponder in field */
    ERR_NOTSUPP = 24, /*!< requested operation not supported */
    ERR_WRITE = 25, /*!< write error */
    ERR_FIFO = 26, /*!< fifo over or underflow error */
    ERR_PAR = 27, /*!< parity error */
    ERR_DONE = 28, /*!< transfer has already finished */
    ERR_RF_COLLISION =
        29, /*!< collision error (Bit Collision or during RF Collision avoidance ) */
    ERR_HW_OVERRUN = 30, /*!< lost one or more received bytes */
    ERR_RELEASE_REQ = 31, /*!< device requested release */
    ERR_SLEEP_REQ = 32, /*!< device requested sleep */
    ERR_WRONG_STATE = 33, /*!< incorrent state for requested operation */
    ERR_MAX_RERUNS = 34, /*!< blocking procedure reached maximum runs */
    ERR_DISABLED = 35, /*!< operation aborted due to disabled configuration */
    ERR_HW_MISMATCH = 36, /*!< expected hw do not match  */
    ERR_LINK_LOSS =
        37, /*!< Other device's field didn't behave as expected: turned off by Initiator in Passive mode, or AP2P did not turn on field */
    ERR_INVALID_HANDLE = 38, /*!< invalid or not initalized device handle */

    ERR_INCOMPLETE_BYTE = 40, /*!< Incomplete byte rcvd         */
    ERR_INCOMPLETE_BYTE_01 = 41, /*!< Incomplete byte rcvd - 1 bit */
    ERR_INCOMPLETE_BYTE_02 = 42, /*!< Incomplete byte rcvd - 2 bit */
    ERR_INCOMPLETE_BYTE_03 = 43, /*!< Incomplete byte rcvd - 3 bit */
    ERR_INCOMPLETE_BYTE_04 = 44, /*!< Incomplete byte rcvd - 4 bit */
    ERR_INCOMPLETE_BYTE_05 = 45, /*!< Incomplete byte rcvd - 5 bit */
    ERR_INCOMPLETE_BYTE_06 = 46, /*!< Incomplete byte rcvd - 6 bit */
    ERR_INCOMPLETE_BYTE_07 = 47, /*!< Incomplete byte rcvd - 7 bit */
};

/* General Sub-category number */
#define ERR_GENERIC_GRP (0x0000) /*!< Reserved value for generic error no */
#define ERR_WARN_GRP (0x0100) /*!< Errors which are not expected in normal operation */
#define ERR_PROCESS_GRP (0x0200) /*!< Processes management errors */
#define ERR_SIO_GRP (0x0800) /*!< SIO errors due to logging */
#define ERR_RINGBUF_GRP (0x0900) /*!< Ring Buffer errors */
#define ERR_MQ_GRP (0x0A00) /*!< MQ errors */
#define ERR_TIMER_GRP (0x0B00) /*!< Timer errors */
#define ERR_RFAL_GRP (0x0C00) /*!< RFAL errors */
#define ERR_UART_GRP (0x0D00) /*!< UART errors */
#define ERR_SPI_GRP (0x0E00) /*!< SPI errors */
#define ERR_I2C_GRP (0x0F00) /*!< I2c errors */

#define ERR_INSERT_SIO_GRP(x) (ERR_SIO_GRP | x) /*!< Insert the SIO grp */
#define ERR_INSERT_RINGBUF_GRP(x) (ERR_RINGBUF_GRP | x) /*!< Insert the Ring Buffer grp */
#define ERR_INSERT_RFAL_GRP(x) (ERR_RFAL_GRP | x) /*!< Insert the RFAL grp */
#define ERR_INSERT_SPI_GRP(x) (ERR_SPI_GRP | x) /*!< Insert the spi grp */
#define ERR_INSERT_I2C_GRP(x) (ERR_I2C_GRP | x) /*!< Insert the i2c grp */
#define ERR_INSERT_UART_GRP(x) (ERR_UART_GRP | x) /*!< Insert the uart grp */
#define ERR_INSERT_TIMER_GRP(x) (ERR_TIMER_GRP | x) /*!< Insert the timer grp */
#define ERR_INSERT_MQ_GRP(x) (ERR_MQ_GRP | x) /*!< Insert the mq grp */
#define ERR_INSERT_PROCESS_GRP(x) (ERR_PROCESS_GRP | x) /*!< Insert the process grp */
#define ERR_INSERT_WARN_GRP(x) (ERR_WARN_GRP | x) /*!< Insert the i2c grp */
#define ERR_INSERT_GENERIC_GRP(x) (ERR_GENERIC_GRP | x) /*!< Insert the generic grp */

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

#define ERR_NO_MASK(x) (x & 0x00FF) /*!< Mask the error number */

/*! Common code to exit a function with the error if function f return error */
#define EXIT_ON_ERR(r, f)     \
    if(ERR_NONE != (r = f)) { \
        return r;             \
    }

#endif /* ST_ERRNO_H */
