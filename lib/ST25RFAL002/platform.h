#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "timer.h"
#include "math.h"
#include <furi_hal_gpio.h>
#include <furi_hal_light.h>
#include <furi_hal_spi.h>

typedef void (*PlatformIrqCallback)();
void platformSetIrqCallback(PlatformIrqCallback cb);
void platformEnableIrqCallback();
void platformDisableIrqCallback();

bool platformSpiTxRx(const uint8_t* txBuf, uint8_t* rxBuf, uint16_t len);
void platformProtectST25RComm();
void platformUnprotectST25RComm();
void rfal_platform_spi_acquire();
void rfal_platform_spi_release();

#define ST25R_SS_PIN NFC_CS_Pin
#define ST25R_SS_PORT NFC_CS_GPIO_Port

#define ST25R_INT_PIN NFC_IRQ_Pin
#define ST25R_INT_PORT NFC_IRQ_GPIO_Port

#define RFAL_ANALOG_CONFIG_CUSTOM \
    true /*!< Enable/Disable RFAL custom analog configuration                           */

#define RFAL_FEATURE_LISTEN_MODE \
    true /*!< Enable/Disable RFAL support for Listen Mode                               */
#define RFAL_FEATURE_WAKEUP_MODE \
    true /*!< Enable/Disable RFAL support for the Wake-Up mode                          */
#define RFAL_FEATURE_LOWPOWER_MODE \
    true /*!< Enable/Disable RFAL support for the Low Power mode                        */
#define RFAL_FEATURE_NFCA \
    true /*!< Enable/Disable RFAL support for NFC-A (ISO14443A)                         */
#define RFAL_FEATURE_NFCB \
    true /*!< Enable/Disable RFAL support for NFC-B (ISO14443B)                         */
#define RFAL_FEATURE_NFCF \
    true /*!< Enable/Disable RFAL support for NFC-F (FeliCa)                            */
#define RFAL_FEATURE_NFCV \
    true /*!< Enable/Disable RFAL support for NFC-V (ISO15693)                          */
#define RFAL_FEATURE_T1T \
    true /*!< Enable/Disable RFAL support for T1T (Topaz)                               */
#define RFAL_FEATURE_T2T \
    true /*!< Enable/Disable RFAL support for T2T (MIFARE Ultralight)                   */
#define RFAL_FEATURE_T4T \
    true /*!< Enable/Disable RFAL support for T4T                                       */
#define RFAL_FEATURE_ST25TB \
    true /*!< Enable/Disable RFAL support for ST25TB                                    */
#define RFAL_FEATURE_ST25xV \
    true /*!< Enable/Disable RFAL support for ST25TV/ST25DV                             */
#define RFAL_FEATURE_DYNAMIC_ANALOG_CONFIG \
    false /*!< Enable/Disable Analog Configs to be dynamically updated (RAM)             */
#define RFAL_FEATURE_DPO \
    false /*!< Enable/Disable RFAL Dynamic Power Output upport                           */
#define RFAL_FEATURE_ISO_DEP \
    true /*!< Enable/Disable RFAL support for ISO-DEP (ISO14443-4)                      */
#define RFAL_FEATURE_ISO_DEP_POLL \
    true /*!< Enable/Disable RFAL support for Poller mode (PCD) ISO-DEP (ISO14443-4)    */
#define RFAL_FEATURE_ISO_DEP_LISTEN \
    true /*!< Enable/Disable RFAL support for Listen mode (PICC) ISO-DEP (ISO14443-4)   */
#define RFAL_FEATURE_NFC_DEP \
    true /*!< Enable/Disable RFAL support for NFC-DEP (NFCIP1/P2P)                      */

#define RFAL_FEATURE_ISO_DEP_IBLOCK_MAX_LEN \
    256U /*!< ISO-DEP I-Block max length. Please use values as defined by rfalIsoDepFSx */
#define RFAL_FEATURE_NFC_DEP_BLOCK_MAX_LEN \
    254U /*!< NFC-DEP Block/Payload length. Allowed values: 64, 128, 192, 254           */
#define RFAL_FEATURE_NFC_RF_BUF_LEN \
    256U /*!< RF buffer length used by RFAL NFC layer                                   */

#define RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN \
    512U /*!< ISO-DEP APDU max length.                                                  */
#define RFAL_FEATURE_NFC_DEP_PDU_MAX_LEN \
    512U /*!< NFC-DEP PDU max length.                                                   */

#define platformIrqST25RSetCallback(cb) platformSetIrqCallback(cb)

#define platformProtectST25RIrqStatus() \
    platformProtectST25RComm() /*!< Protect unique access to IRQ status var - IRQ disable on single thread environment (MCU) ; Mutex lock on a multi thread environment */
#define platformUnprotectST25RIrqStatus() \
    platformUnprotectST25RComm() /*!< Unprotect the IRQ status var - IRQ enable on a single thread environment (MCU) ; Mutex unlock on a multi thread environment         */

#define platformGpioSet(port, pin) \
    furi_hal_gpio_write_port_pin(  \
        port, pin, true) /*!< Turns the given GPIO High                   */
#define platformGpioClear(port, pin) \
    furi_hal_gpio_write_port_pin(    \
        port, pin, false) /*!< Turns the given GPIO Low                    */

#define platformGpioIsHigh(port, pin)          \
    (furi_hal_gpio_read_port_pin(port, pin) == \
     true) /*!< Checks if the given LED is High             */
#define platformGpioIsLow(port, pin) \
    (!platformGpioIsHigh(port, pin)) /*!< Checks if the given LED is Low              */

#define platformTimerCreate(t) \
    timerCalculateTimer(t) /*!< Create a timer with the given time (ms)     */
#define platformTimerIsExpired(timer) \
    timerIsExpired(timer) /*!< Checks if the given timer is expired        */
#define platformDelay(t) furi_delay_ms(t) /*!< Performs a delay for the given time (ms)    */

#define platformGetSysTick() furi_get_tick() /*!< Get System Tick (1 tick = 1 ms)             */

#define platformAssert(exp) assert_param(exp) /*!< Asserts whether the given expression is true*/

#define platformSpiSelect() \
    platformGpioClear(      \
        ST25R_SS_PORT, ST25R_SS_PIN) /*!< SPI SS\CS: Chip|Slave Select                */
#define platformSpiDeselect() \
    platformGpioSet(          \
        ST25R_SS_PORT, ST25R_SS_PIN) /*!< SPI SS\CS: Chip|Slave Deselect              */

#define platformI2CTx(txBuf, len, last, txOnly) /*!< I2C Transmit                                */
#define platformI2CRx(txBuf, len) /*!< I2C Receive                                 */
#define platformI2CStart() /*!< I2C Start condition                         */
#define platformI2CStop() /*!< I2C Stop condition                          */
#define platformI2CRepeatStart() /*!< I2C Repeat Start                            */
#define platformI2CSlaveAddrWR(add) /*!< I2C Slave address for Write operation       */
#define platformI2CSlaveAddrRD(add) /*!< I2C Slave address for Read operation        */

#define platformLog(...) /*!< Log  method                                 */

/*
 ******************************************************************************
 * RFAL OPTIONAL MACROS            (Do not change)
 ******************************************************************************
 */
#ifndef platformProtectST25RIrqStatus
#define platformProtectST25RIrqStatus() /*!< Protect unique access to IRQ status var - IRQ disable on single thread environment (MCU) ; Mutex lock on a multi thread environment */
#endif /* platformProtectST25RIrqStatus */

#ifndef platformUnprotectST25RIrqStatus
#define platformUnprotectST25RIrqStatus() /*!< Unprotect the IRQ status var - IRQ enable on a single thread environment (MCU) ; Mutex unlock on a multi thread environment         */
#endif /* platformUnprotectST25RIrqStatus */

#ifndef platformProtectWorker
#define platformProtectWorker() /* Protect RFAL Worker/Task/Process from concurrent execution on multi thread platforms   */
#endif /* platformProtectWorker */

#ifndef platformUnprotectWorker
#define platformUnprotectWorker() /* Unprotect RFAL Worker/Task/Process from concurrent execution on multi thread platforms */
#endif /* platformUnprotectWorker */

#ifndef platformIrqST25RPinInitialize
#define platformIrqST25RPinInitialize() /*!< Initializes ST25R IRQ pin                    */
#endif /* platformIrqST25RPinInitialize */

#ifndef platformIrqST25RSetCallback
#define platformIrqST25RSetCallback(cb) /*!< Sets ST25R ISR callback                      */
#endif /* platformIrqST25RSetCallback */

#ifndef platformLedsInitialize
#define platformLedsInitialize() /*!< Initializes the pins used as LEDs to outputs */
#endif /* platformLedsInitialize */

#ifndef platformLedOff
#define platformLedOff(port, pin) /*!< Turns the given LED Off                      */
#endif /* platformLedOff */

#ifndef platformLedOn
#define platformLedOn(port, pin) /*!< Turns the given LED On                       */
#endif /* platformLedOn */

#ifndef platformLedToogle
#define platformLedToogle(port, pin) /*!< Toggles the given LED                        */
#endif /* platformLedToogle */

#ifndef platformGetSysTick
#define platformGetSysTick() /*!< Get System Tick (1 tick = 1 ms)              */
#endif /* platformGetSysTick */

#ifndef platformTimerDestroy
#define platformTimerDestroy(timer) /*!< Stops and released the given timer           */
#endif /* platformTimerDestroy */

#ifndef platformAssert
#define platformAssert(exp) /*!< Asserts whether the given expression is true */
#endif /* platformAssert */

#ifndef platformErrorHandle
#define platformErrorHandle() /*!< Global error handler or trap                 */
#endif /* platformErrorHandle */
