/**
  ******************************************************************************
  * @file    stm32l422xx.h
  * @author  MCD Application Team
  * @brief   CMSIS STM32L422xx Device Peripheral Access Layer Header File.
  *
  *          This file contains:
  *           - Data structures and the address mapping for all peripherals
  *           - Peripheral's registers declarations and bits definition
  *           - Macros to access peripheral’s registers hardware
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/** @addtogroup CMSIS_Device
  * @{
  */

/** @addtogroup stm32l422xx
  * @{
  */

#ifndef __STM32L422xx_H
#define __STM32L422xx_H

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/** @addtogroup Configuration_section_for_CMSIS
  * @{
  */

/**
  * @brief Configuration of the Cortex-M4 Processor and Core Peripherals
   */
#define __CM4_REV                 0x0001  /*!< Cortex-M4 revision r0p1                       */
#define __MPU_PRESENT             1       /*!< STM32L4XX provides an MPU                     */
#define __NVIC_PRIO_BITS          4       /*!< STM32L4XX uses 4 Bits for the Priority Levels */
#define __Vendor_SysTickConfig    0       /*!< Set to 1 if different SysTick Config is used  */
#define __FPU_PRESENT             1       /*!< FPU present                                   */

/**
  * @}
  */

/** @addtogroup Peripheral_interrupt_number_definition
  * @{
  */

/**
 * @brief STM32L4XX Interrupt Number Definition, according to the selected device
 *        in @ref Library_configuration_section
 */
typedef enum
{
/******  Cortex-M4 Processor Exceptions Numbers ****************************************************************/
  NonMaskableInt_IRQn         = -14,    /*!< 2 Cortex-M4 Non Maskable Interrupt                                */
  HardFault_IRQn              = -13,    /*!< 3 Cortex-M4 Hard Fault Interrupt                                  */
  MemoryManagement_IRQn       = -12,    /*!< 4 Cortex-M4 Memory Management Interrupt                           */
  BusFault_IRQn               = -11,    /*!< 5 Cortex-M4 Bus Fault Interrupt                                   */
  UsageFault_IRQn             = -10,    /*!< 6 Cortex-M4 Usage Fault Interrupt                                 */
  SVCall_IRQn                 = -5,     /*!< 11 Cortex-M4 SV Call Interrupt                                    */
  DebugMonitor_IRQn           = -4,     /*!< 12 Cortex-M4 Debug Monitor Interrupt                              */
  PendSV_IRQn                 = -2,     /*!< 14 Cortex-M4 Pend SV Interrupt                                    */
  SysTick_IRQn                = -1,     /*!< 15 Cortex-M4 System Tick Interrupt                                */
/******  STM32 specific Interrupt Numbers **********************************************************************/
  WWDG_IRQn                   = 0,      /*!< Window WatchDog Interrupt                                         */
  PVD_PVM_IRQn                = 1,      /*!< PVD/PVM1/PVM3/PVM4 through EXTI Line detection Interrupts         */
  TAMP_STAMP_IRQn             = 2,      /*!< Tamper and TimeStamp interrupts through the EXTI line             */
  RTC_WKUP_IRQn               = 3,      /*!< RTC Wakeup interrupt through the EXTI line                        */
  FLASH_IRQn                  = 4,      /*!< FLASH global Interrupt                                            */
  RCC_IRQn                    = 5,      /*!< RCC global Interrupt                                              */
  EXTI0_IRQn                  = 6,      /*!< EXTI Line0 Interrupt                                              */
  EXTI1_IRQn                  = 7,      /*!< EXTI Line1 Interrupt                                              */
  EXTI2_IRQn                  = 8,      /*!< EXTI Line2 Interrupt                                              */
  EXTI3_IRQn                  = 9,      /*!< EXTI Line3 Interrupt                                              */
  EXTI4_IRQn                  = 10,     /*!< EXTI Line4 Interrupt                                              */
  DMA1_Channel1_IRQn          = 11,     /*!< DMA1 Channel 1 global Interrupt                                   */
  DMA1_Channel2_IRQn          = 12,     /*!< DMA1 Channel 2 global Interrupt                                   */
  DMA1_Channel3_IRQn          = 13,     /*!< DMA1 Channel 3 global Interrupt                                   */
  DMA1_Channel4_IRQn          = 14,     /*!< DMA1 Channel 4 global Interrupt                                   */
  DMA1_Channel5_IRQn          = 15,     /*!< DMA1 Channel 5 global Interrupt                                   */
  DMA1_Channel6_IRQn          = 16,     /*!< DMA1 Channel 6 global Interrupt                                   */
  DMA1_Channel7_IRQn          = 17,     /*!< DMA1 Channel 7 global Interrupt                                   */
  ADC1_2_IRQn                 = 18,     /*!< ADC1, ADC2 SAR global Interrupts                                  */
  EXTI9_5_IRQn                = 23,     /*!< External Line[9:5] Interrupts                                     */
  TIM1_BRK_TIM15_IRQn         = 24,     /*!< TIM1 Break interrupt and TIM15 global interrupt                   */
  TIM1_UP_TIM16_IRQn          = 25,     /*!< TIM1 Update Interrupt and TIM16 global interrupt                  */
  TIM1_TRG_COM_IRQn           = 26,     /*!< TIM1 Trigger and Commutation Interrupt                            */
  TIM1_CC_IRQn                = 27,     /*!< TIM1 Capture Compare Interrupt                                    */
  TIM2_IRQn                   = 28,     /*!< TIM2 global Interrupt                                             */
  I2C1_EV_IRQn                = 31,     /*!< I2C1 Event Interrupt                                              */
  I2C1_ER_IRQn                = 32,     /*!< I2C1 Error Interrupt                                              */
  I2C2_EV_IRQn                = 33,     /*!< I2C2 Event Interrupt                                              */
  I2C2_ER_IRQn                = 34,     /*!< I2C2 Error Interrupt                                              */
  SPI1_IRQn                   = 35,     /*!< SPI1 global Interrupt                                             */
  SPI2_IRQn                   = 36,     /*!< SPI2 global Interrupt                                             */
  USART1_IRQn                 = 37,     /*!< USART1 global Interrupt                                           */
  USART2_IRQn                 = 38,     /*!< USART2 global Interrupt                                           */
  USART3_IRQn                 = 39,     /*!< USART3 global Interrupt                                           */
  EXTI15_10_IRQn              = 40,     /*!< External Line[15:10] Interrupts                                   */
  RTC_Alarm_IRQn              = 41,     /*!< RTC Alarm (A and B) through EXTI Line Interrupt                   */
  TIM6_IRQn                   = 54,     /*!< TIM6 global interrupt                                             */
  DMA2_Channel1_IRQn          = 56,     /*!< DMA2 Channel 1 global Interrupt                                   */
  DMA2_Channel2_IRQn          = 57,     /*!< DMA2 Channel 2 global Interrupt                                   */
  DMA2_Channel3_IRQn          = 58,     /*!< DMA2 Channel 3 global Interrupt                                   */
  DMA2_Channel4_IRQn          = 59,     /*!< DMA2 Channel 4 global Interrupt                                   */
  DMA2_Channel5_IRQn          = 60,     /*!< DMA2 Channel 5 global Interrupt                                   */
  COMP_IRQn                   = 64,     /*!< COMP1 Interrupt                                                   */
  LPTIM1_IRQn                 = 65,     /*!< LP TIM1 interrupt                                                 */
  LPTIM2_IRQn                 = 66,     /*!< LP TIM2 interrupt                                                 */
  USB_IRQn                    = 67,     /*!< USB event Interrupt                                               */
  DMA2_Channel6_IRQn          = 68,     /*!< DMA2 Channel 6 global interrupt                                   */
  DMA2_Channel7_IRQn          = 69,     /*!< DMA2 Channel 7 global interrupt                                   */
  LPUART1_IRQn                = 70,     /*!< LP UART1 interrupt                                                */
  QUADSPI_IRQn                = 71,     /*!< Quad SPI global interrupt                                         */
  I2C3_EV_IRQn                = 72,     /*!< I2C3 event interrupt                                              */
  I2C3_ER_IRQn                = 73,     /*!< I2C3 error interrupt                                              */
  TSC_IRQn                    = 77,     /*!< Touch Sense Controller global interrupt                           */
  AES_IRQn                    = 79,     /*!< AES global interrupt                                              */
  RNG_IRQn                    = 80,     /*!< RNG global interrupt                                              */
  FPU_IRQn                    = 81,     /*!< FPU global interrupt                                              */
  CRS_IRQn                    = 82      /*!< CRS global interrupt                                              */
} IRQn_Type;

/**
  * @}
  */

#include "core_cm4.h"             /* Cortex-M4 processor and core peripherals */
#include "system_stm32l4xx.h"
#include <stdint.h>

/** @addtogroup Peripheral_registers_structures
  * @{
  */

/**
  * @brief Analog to Digital Converter
  */

typedef struct
{
  __IO uint32_t ISR;          /*!< ADC interrupt and status register,             Address offset: 0x00 */
  __IO uint32_t IER;          /*!< ADC interrupt enable register,                 Address offset: 0x04 */
  __IO uint32_t CR;           /*!< ADC control register,                          Address offset: 0x08 */
  __IO uint32_t CFGR;         /*!< ADC configuration register 1,                  Address offset: 0x0C */
  __IO uint32_t CFGR2;        /*!< ADC configuration register 2,                  Address offset: 0x10 */
  __IO uint32_t SMPR1;        /*!< ADC sampling time register 1,                  Address offset: 0x14 */
  __IO uint32_t SMPR2;        /*!< ADC sampling time register 2,                  Address offset: 0x18 */
       uint32_t RESERVED1;    /*!< Reserved,                                                      0x1C */
  __IO uint32_t TR1;          /*!< ADC analog watchdog 1 threshold register,      Address offset: 0x20 */
  __IO uint32_t TR2;          /*!< ADC analog watchdog 2 threshold register,      Address offset: 0x24 */
  __IO uint32_t TR3;          /*!< ADC analog watchdog 3 threshold register,      Address offset: 0x28 */
       uint32_t RESERVED2;    /*!< Reserved,                                                      0x2C */
  __IO uint32_t SQR1;         /*!< ADC group regular sequencer register 1,        Address offset: 0x30 */
  __IO uint32_t SQR2;         /*!< ADC group regular sequencer register 2,        Address offset: 0x34 */
  __IO uint32_t SQR3;         /*!< ADC group regular sequencer register 3,        Address offset: 0x38 */
  __IO uint32_t SQR4;         /*!< ADC group regular sequencer register 4,        Address offset: 0x3C */
  __IO uint32_t DR;           /*!< ADC group regular data register,               Address offset: 0x40 */
       uint32_t RESERVED3;    /*!< Reserved,                                                      0x44 */
       uint32_t RESERVED4;    /*!< Reserved,                                                      0x48 */
  __IO uint32_t JSQR;         /*!< ADC group injected sequencer register,         Address offset: 0x4C */
       uint32_t RESERVED5[4]; /*!< Reserved,                                               0x50 - 0x5C */
  __IO uint32_t OFR1;         /*!< ADC offset register 1,                         Address offset: 0x60 */
  __IO uint32_t OFR2;         /*!< ADC offset register 2,                         Address offset: 0x64 */
  __IO uint32_t OFR3;         /*!< ADC offset register 3,                         Address offset: 0x68 */
  __IO uint32_t OFR4;         /*!< ADC offset register 4,                         Address offset: 0x6C */
       uint32_t RESERVED6[4]; /*!< Reserved,                                               0x70 - 0x7C */
  __IO uint32_t JDR1;         /*!< ADC group injected rank 1 data register,       Address offset: 0x80 */
  __IO uint32_t JDR2;         /*!< ADC group injected rank 2 data register,       Address offset: 0x84 */
  __IO uint32_t JDR3;         /*!< ADC group injected rank 3 data register,       Address offset: 0x88 */
  __IO uint32_t JDR4;         /*!< ADC group injected rank 4 data register,       Address offset: 0x8C */
       uint32_t RESERVED7[4]; /*!< Reserved,                                             0x090 - 0x09C */
  __IO uint32_t AWD2CR;       /*!< ADC analog watchdog 1 configuration register,  Address offset: 0xA0 */
  __IO uint32_t AWD3CR;       /*!< ADC analog watchdog 3 Configuration Register,  Address offset: 0xA4 */
       uint32_t RESERVED8;    /*!< Reserved,                                                     0x0A8 */
       uint32_t RESERVED9;    /*!< Reserved,                                                     0x0AC */
  __IO uint32_t DIFSEL;       /*!< ADC differential mode selection register,      Address offset: 0xB0 */
  __IO uint32_t CALFACT;      /*!< ADC calibration factors,                       Address offset: 0xB4 */

} ADC_TypeDef;

typedef struct
{
  __IO uint32_t CSR;          /*!< ADC common status register,                    Address offset: ADC1 base address + 0x300 */
  uint32_t      RESERVED;     /*!< Reserved,                                      Address offset: ADC1 base address + 0x304 */
  __IO uint32_t CCR;          /*!< ADC common configuration register,             Address offset: ADC1 base address + 0x308 */
  __IO uint32_t CDR;          /*!< ADC common group regular data register         Address offset: ADC1 base address + 0x30C */
} ADC_Common_TypeDef;




/**
  * @brief Comparator
  */

typedef struct
{
  __IO uint32_t CSR;         /*!< COMP control and status register, Address offset: 0x00 */
} COMP_TypeDef;

typedef struct
{
  __IO uint32_t CSR;         /*!< COMP control and status register, used for bits common to several COMP instances, Address offset: 0x00 */
} COMP_Common_TypeDef;

/**
  * @brief CRC calculation unit
  */

typedef struct
{
  __IO uint32_t DR;          /*!< CRC Data register,                           Address offset: 0x00 */
  __IO uint8_t  IDR;         /*!< CRC Independent data register,               Address offset: 0x04 */
  uint8_t       RESERVED0;   /*!< Reserved,                                                    0x05 */
  uint16_t      RESERVED1;   /*!< Reserved,                                                    0x06 */
  __IO uint32_t CR;          /*!< CRC Control register,                        Address offset: 0x08 */
  uint32_t      RESERVED2;   /*!< Reserved,                                                    0x0C */
  __IO uint32_t INIT;        /*!< Initial CRC value register,                  Address offset: 0x10 */
  __IO uint32_t POL;         /*!< CRC polynomial register,                     Address offset: 0x14 */
} CRC_TypeDef;

/**
  * @brief Clock Recovery System
  */
typedef struct
{
__IO uint32_t CR;            /*!< CRS ccontrol register,              Address offset: 0x00 */
__IO uint32_t CFGR;          /*!< CRS configuration register,         Address offset: 0x04 */
__IO uint32_t ISR;           /*!< CRS interrupt and status register,  Address offset: 0x08 */
__IO uint32_t ICR;           /*!< CRS interrupt flag clear register,  Address offset: 0x0C */
} CRS_TypeDef;



/**
  * @brief Debug MCU
  */

typedef struct
{
  __IO uint32_t IDCODE;      /*!< MCU device ID code,                 Address offset: 0x00 */
  __IO uint32_t CR;          /*!< Debug MCU configuration register,   Address offset: 0x04 */
  __IO uint32_t APB1FZR1;    /*!< Debug MCU APB1 freeze register 1,   Address offset: 0x08 */
  __IO uint32_t APB1FZR2;    /*!< Debug MCU APB1 freeze register 2,   Address offset: 0x0C */
  __IO uint32_t APB2FZ;      /*!< Debug MCU APB2 freeze register,     Address offset: 0x10 */
} DBGMCU_TypeDef;


/**
  * @brief DMA Controller
  */

typedef struct
{
  __IO uint32_t CCR;         /*!< DMA channel x configuration register        */
  __IO uint32_t CNDTR;       /*!< DMA channel x number of data register       */
  __IO uint32_t CPAR;        /*!< DMA channel x peripheral address register   */
  __IO uint32_t CMAR;        /*!< DMA channel x memory address register       */
} DMA_Channel_TypeDef;

typedef struct
{
  __IO uint32_t ISR;         /*!< DMA interrupt status register,                 Address offset: 0x00 */
  __IO uint32_t IFCR;        /*!< DMA interrupt flag clear register,             Address offset: 0x04 */
} DMA_TypeDef;

typedef struct
{
  __IO uint32_t CSELR;       /*!< DMA channel selection register              */
} DMA_Request_TypeDef;

/* Legacy define */
#define DMA_request_TypeDef  DMA_Request_TypeDef


/**
  * @brief External Interrupt/Event Controller
  */

typedef struct
{
  __IO uint32_t IMR1;        /*!< EXTI Interrupt mask register 1,             Address offset: 0x00 */
  __IO uint32_t EMR1;        /*!< EXTI Event mask register 1,                 Address offset: 0x04 */
  __IO uint32_t RTSR1;       /*!< EXTI Rising trigger selection register 1,   Address offset: 0x08 */
  __IO uint32_t FTSR1;       /*!< EXTI Falling trigger selection register 1,  Address offset: 0x0C */
  __IO uint32_t SWIER1;      /*!< EXTI Software interrupt event register 1,   Address offset: 0x10 */
  __IO uint32_t PR1;         /*!< EXTI Pending register 1,                    Address offset: 0x14 */
  uint32_t      RESERVED1;   /*!< Reserved, 0x18                                                   */
  uint32_t      RESERVED2;   /*!< Reserved, 0x1C                                                   */
  __IO uint32_t IMR2;        /*!< EXTI Interrupt mask register 2,             Address offset: 0x20 */
  __IO uint32_t EMR2;        /*!< EXTI Event mask register 2,                 Address offset: 0x24 */
  __IO uint32_t RTSR2;       /*!< EXTI Rising trigger selection register 2,   Address offset: 0x28 */
  __IO uint32_t FTSR2;       /*!< EXTI Falling trigger selection register 2,  Address offset: 0x2C */
  __IO uint32_t SWIER2;      /*!< EXTI Software interrupt event register 2,   Address offset: 0x30 */
  __IO uint32_t PR2;         /*!< EXTI Pending register 2,                    Address offset: 0x34 */
} EXTI_TypeDef;


/**
  * @brief Firewall
  */

typedef struct
{
  __IO uint32_t CSSA;        /*!< Code Segment Start Address register,              Address offset: 0x00 */
  __IO uint32_t CSL;         /*!< Code Segment Length register,                      Address offset: 0x04 */
  __IO uint32_t NVDSSA;      /*!< NON volatile data Segment Start Address register,  Address offset: 0x08 */
  __IO uint32_t NVDSL;       /*!< NON volatile data Segment Length register,         Address offset: 0x0C */
  __IO uint32_t VDSSA ;      /*!< Volatile data Segment Start Address register,      Address offset: 0x10 */
  __IO uint32_t VDSL ;       /*!< Volatile data Segment Length register,             Address offset: 0x14 */
  uint32_t      RESERVED1;   /*!< Reserved1,                                         Address offset: 0x18 */
  uint32_t      RESERVED2;   /*!< Reserved2,                                         Address offset: 0x1C */
  __IO uint32_t CR ;         /*!< Configuration  register,                           Address offset: 0x20 */
} FIREWALL_TypeDef;


/**
  * @brief FLASH Registers
  */

typedef struct
{
  __IO uint32_t ACR;              /*!< FLASH access control register,            Address offset: 0x00 */
  __IO uint32_t PDKEYR;           /*!< FLASH power down key register,            Address offset: 0x04 */
  __IO uint32_t KEYR;             /*!< FLASH key register,                       Address offset: 0x08 */
  __IO uint32_t OPTKEYR;          /*!< FLASH option key register,                Address offset: 0x0C */
  __IO uint32_t SR;               /*!< FLASH status register,                    Address offset: 0x10 */
  __IO uint32_t CR;               /*!< FLASH control register,                   Address offset: 0x14 */
  __IO uint32_t ECCR;             /*!< FLASH ECC register,                       Address offset: 0x18 */
  __IO uint32_t RESERVED1;        /*!< Reserved1,                                Address offset: 0x1C */
  __IO uint32_t OPTR;             /*!< FLASH option register,                    Address offset: 0x20 */
  __IO uint32_t PCROP1SR;         /*!< FLASH bank1 PCROP start address register, Address offset: 0x24 */
  __IO uint32_t PCROP1ER;         /*!< FLASH bank1 PCROP end address register,   Address offset: 0x28 */
  __IO uint32_t WRP1AR;           /*!< FLASH bank1 WRP area A address register,  Address offset: 0x2C */
  __IO uint32_t WRP1BR;           /*!< FLASH bank1 WRP area B address register,  Address offset: 0x30 */
} FLASH_TypeDef;



/**
  * @brief General Purpose I/O
  */

typedef struct
{
  __IO uint32_t MODER;       /*!< GPIO port mode register,               Address offset: 0x00      */
  __IO uint32_t OTYPER;      /*!< GPIO port output type register,        Address offset: 0x04      */
  __IO uint32_t OSPEEDR;     /*!< GPIO port output speed register,       Address offset: 0x08      */
  __IO uint32_t PUPDR;       /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C      */
  __IO uint32_t IDR;         /*!< GPIO port input data register,         Address offset: 0x10      */
  __IO uint32_t ODR;         /*!< GPIO port output data register,        Address offset: 0x14      */
  __IO uint32_t BSRR;        /*!< GPIO port bit set/reset  register,     Address offset: 0x18      */
  __IO uint32_t LCKR;        /*!< GPIO port configuration lock register, Address offset: 0x1C      */
  __IO uint32_t AFR[2];      /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
  __IO uint32_t BRR;         /*!< GPIO Bit Reset register,               Address offset: 0x28      */

} GPIO_TypeDef;


/**
  * @brief Inter-integrated Circuit Interface
  */

typedef struct
{
  __IO uint32_t CR1;         /*!< I2C Control register 1,            Address offset: 0x00 */
  __IO uint32_t CR2;         /*!< I2C Control register 2,            Address offset: 0x04 */
  __IO uint32_t OAR1;        /*!< I2C Own address 1 register,        Address offset: 0x08 */
  __IO uint32_t OAR2;        /*!< I2C Own address 2 register,        Address offset: 0x0C */
  __IO uint32_t TIMINGR;     /*!< I2C Timing register,               Address offset: 0x10 */
  __IO uint32_t TIMEOUTR;    /*!< I2C Timeout register,              Address offset: 0x14 */
  __IO uint32_t ISR;         /*!< I2C Interrupt and status register, Address offset: 0x18 */
  __IO uint32_t ICR;         /*!< I2C Interrupt clear register,      Address offset: 0x1C */
  __IO uint32_t PECR;        /*!< I2C PEC register,                  Address offset: 0x20 */
  __IO uint32_t RXDR;        /*!< I2C Receive data register,         Address offset: 0x24 */
  __IO uint32_t TXDR;        /*!< I2C Transmit data register,        Address offset: 0x28 */
} I2C_TypeDef;

/**
  * @brief Independent WATCHDOG
  */

typedef struct
{
  __IO uint32_t KR;          /*!< IWDG Key register,       Address offset: 0x00 */
  __IO uint32_t PR;          /*!< IWDG Prescaler register, Address offset: 0x04 */
  __IO uint32_t RLR;         /*!< IWDG Reload register,    Address offset: 0x08 */
  __IO uint32_t SR;          /*!< IWDG Status register,    Address offset: 0x0C */
  __IO uint32_t WINR;        /*!< IWDG Window register,    Address offset: 0x10 */
} IWDG_TypeDef;

/**
  * @brief LPTIMER
  */
typedef struct
{
  __IO uint32_t ISR;         /*!< LPTIM Interrupt and Status register,                Address offset: 0x00 */
  __IO uint32_t ICR;         /*!< LPTIM Interrupt Clear register,                     Address offset: 0x04 */
  __IO uint32_t IER;         /*!< LPTIM Interrupt Enable register,                    Address offset: 0x08 */
  __IO uint32_t CFGR;        /*!< LPTIM Configuration register,                       Address offset: 0x0C */
  __IO uint32_t CR;          /*!< LPTIM Control register,                             Address offset: 0x10 */
  __IO uint32_t CMP;         /*!< LPTIM Compare register,                             Address offset: 0x14 */
  __IO uint32_t ARR;         /*!< LPTIM Autoreload register,                          Address offset: 0x18 */
  __IO uint32_t CNT;         /*!< LPTIM Counter register,                             Address offset: 0x1C */
  __IO uint32_t OR;          /*!< LPTIM Option register,                              Address offset: 0x20 */
  __IO uint32_t RESERVED;    /*!< Reserved,                                           Address offset: 0x24 */
  __IO uint32_t RCR;         /*!< LPTIM repetition counter register,                  Address offset: 0x28 */
} LPTIM_TypeDef;

/**
  * @brief Operational Amplifier (OPAMP)
  */

typedef struct
{
  __IO uint32_t CSR;         /*!< OPAMP control/status register,                     Address offset: 0x00 */
  __IO uint32_t OTR;         /*!< OPAMP offset trimming register for normal mode,    Address offset: 0x04 */
  __IO uint32_t LPOTR;       /*!< OPAMP offset trimming register for low power mode, Address offset: 0x08 */
} OPAMP_TypeDef;

typedef struct
{
  __IO uint32_t CSR;         /*!< OPAMP control/status register, used for bits common to several OPAMP instances, Address offset: 0x00 */
} OPAMP_Common_TypeDef;

/**
  * @brief Power Control
  */

typedef struct
{
  __IO uint32_t CR1;   /*!< PWR power control register 1,        Address offset: 0x00 */
  __IO uint32_t CR2;   /*!< PWR power control register 2,        Address offset: 0x04 */
  __IO uint32_t CR3;   /*!< PWR power control register 3,        Address offset: 0x08 */
  __IO uint32_t CR4;   /*!< PWR power control register 4,        Address offset: 0x0C */
  __IO uint32_t SR1;   /*!< PWR power status register 1,         Address offset: 0x10 */
  __IO uint32_t SR2;   /*!< PWR power status register 2,         Address offset: 0x14 */
  __IO uint32_t SCR;   /*!< PWR power status reset register,     Address offset: 0x18 */
  uint32_t RESERVED;   /*!< Reserved,                            Address offset: 0x1C */
  __IO uint32_t PUCRA; /*!< Pull_up control register of portA,   Address offset: 0x20 */
  __IO uint32_t PDCRA; /*!< Pull_Down control register of portA, Address offset: 0x24 */
  __IO uint32_t PUCRB; /*!< Pull_up control register of portB,   Address offset: 0x28 */
  __IO uint32_t PDCRB; /*!< Pull_Down control register of portB, Address offset: 0x2C */
  __IO uint32_t PUCRC; /*!< Pull_up control register of portC,   Address offset: 0x30 */
  __IO uint32_t PDCRC; /*!< Pull_Down control register of portC, Address offset: 0x34 */
  __IO uint32_t PUCRD; /*!< Pull_up control register of portD,   Address offset: 0x38 */
  __IO uint32_t PDCRD; /*!< Pull_Down control register of portD, Address offset: 0x3C */
  __IO uint32_t PUCRE; /*!< Pull_up control register of portE,   Address offset: 0x40 */
  __IO uint32_t PDCRE; /*!< Pull_Down control register of portE, Address offset: 0x44 */
  uint32_t RESERVED1;  /*!< Reserved,                            Address offset: 0x48 */
  uint32_t RESERVED2;  /*!< Reserved,                            Address offset: 0x4C */
  uint32_t RESERVED3;  /*!< Reserved,                            Address offset: 0x50 */
  uint32_t RESERVED4;  /*!< Reserved,                            Address offset: 0x54 */
  __IO uint32_t PUCRH; /*!< Pull_up control register of portH,   Address offset: 0x58 */
  __IO uint32_t PDCRH; /*!< Pull_Down control register of portH, Address offset: 0x5C */
} PWR_TypeDef;


/**
  * @brief QUAD Serial Peripheral Interface
  */

typedef struct
{
  __IO uint32_t CR;          /*!< QUADSPI Control register,                           Address offset: 0x00 */
  __IO uint32_t DCR;         /*!< QUADSPI Device Configuration register,              Address offset: 0x04 */
  __IO uint32_t SR;          /*!< QUADSPI Status register,                            Address offset: 0x08 */
  __IO uint32_t FCR;         /*!< QUADSPI Flag Clear register,                        Address offset: 0x0C */
  __IO uint32_t DLR;         /*!< QUADSPI Data Length register,                       Address offset: 0x10 */
  __IO uint32_t CCR;         /*!< QUADSPI Communication Configuration register,       Address offset: 0x14 */
  __IO uint32_t AR;          /*!< QUADSPI Address register,                           Address offset: 0x18 */
  __IO uint32_t ABR;         /*!< QUADSPI Alternate Bytes register,                   Address offset: 0x1C */
  __IO uint32_t DR;          /*!< QUADSPI Data register,                              Address offset: 0x20 */
  __IO uint32_t PSMKR;       /*!< QUADSPI Polling Status Mask register,               Address offset: 0x24 */
  __IO uint32_t PSMAR;       /*!< QUADSPI Polling Status Match register,              Address offset: 0x28 */
  __IO uint32_t PIR;         /*!< QUADSPI Polling Interval register,                  Address offset: 0x2C */
  __IO uint32_t LPTR;        /*!< QUADSPI Low Power Timeout register,                 Address offset: 0x30 */
} QUADSPI_TypeDef;


/**
  * @brief Reset and Clock Control
  */

typedef struct
{
  __IO uint32_t CR;          /*!< RCC clock control register,                                              Address offset: 0x00 */
  __IO uint32_t ICSCR;       /*!< RCC internal clock sources calibration register,                         Address offset: 0x04 */
  __IO uint32_t CFGR;        /*!< RCC clock configuration register,                                        Address offset: 0x08 */
  __IO uint32_t PLLCFGR;     /*!< RCC system PLL configuration register,                                   Address offset: 0x0C */
  uint32_t      RESERVED8;   /*!< Reserved,                                                                Address offset: 0x10 */
  uint32_t      RESERVED;    /*!< Reserved,                                                                Address offset: 0x14 */
  __IO uint32_t CIER;        /*!< RCC clock interrupt enable register,                                     Address offset: 0x18 */
  __IO uint32_t CIFR;        /*!< RCC clock interrupt flag register,                                       Address offset: 0x1C */
  __IO uint32_t CICR;        /*!< RCC clock interrupt clear register,                                      Address offset: 0x20 */
  uint32_t      RESERVED0;   /*!< Reserved,                                                                Address offset: 0x24 */
  __IO uint32_t AHB1RSTR;    /*!< RCC AHB1 peripheral reset register,                                      Address offset: 0x28 */
  __IO uint32_t AHB2RSTR;    /*!< RCC AHB2 peripheral reset register,                                      Address offset: 0x2C */
  __IO uint32_t AHB3RSTR;    /*!< RCC AHB3 peripheral reset register,                                      Address offset: 0x30 */
  uint32_t      RESERVED1;   /*!< Reserved,                                                                Address offset: 0x34 */
  __IO uint32_t APB1RSTR1;   /*!< RCC APB1 peripheral reset register 1,                                    Address offset: 0x38 */
  __IO uint32_t APB1RSTR2;   /*!< RCC APB1 peripheral reset register 2,                                    Address offset: 0x3C */
  __IO uint32_t APB2RSTR;    /*!< RCC APB2 peripheral reset register,                                      Address offset: 0x40 */
  uint32_t      RESERVED2;   /*!< Reserved,                                                                Address offset: 0x44 */
  __IO uint32_t AHB1ENR;     /*!< RCC AHB1 peripheral clocks enable register,                              Address offset: 0x48 */
  __IO uint32_t AHB2ENR;     /*!< RCC AHB2 peripheral clocks enable register,                              Address offset: 0x4C */
  __IO uint32_t AHB3ENR;     /*!< RCC AHB3 peripheral clocks enable register,                              Address offset: 0x50 */
  uint32_t      RESERVED3;   /*!< Reserved,                                                                Address offset: 0x54 */
  __IO uint32_t APB1ENR1;    /*!< RCC APB1 peripheral clocks enable register 1,                            Address offset: 0x58 */
  __IO uint32_t APB1ENR2;    /*!< RCC APB1 peripheral clocks enable register 2,                            Address offset: 0x5C */
  __IO uint32_t APB2ENR;     /*!< RCC APB2 peripheral clocks enable register,                              Address offset: 0x60 */
  uint32_t      RESERVED4;   /*!< Reserved,                                                                Address offset: 0x64 */
  __IO uint32_t AHB1SMENR;   /*!< RCC AHB1 peripheral clocks enable in sleep and stop modes register,      Address offset: 0x68 */
  __IO uint32_t AHB2SMENR;   /*!< RCC AHB2 peripheral clocks enable in sleep and stop modes register,      Address offset: 0x6C */
  __IO uint32_t AHB3SMENR;   /*!< RCC AHB3 peripheral clocks enable in sleep and stop modes register,      Address offset: 0x70 */
  uint32_t      RESERVED5;   /*!< Reserved,                                                                Address offset: 0x74 */
  __IO uint32_t APB1SMENR1;  /*!< RCC APB1 peripheral clocks enable in sleep mode and stop modes register 1, Address offset: 0x78 */
  __IO uint32_t APB1SMENR2;  /*!< RCC APB1 peripheral clocks enable in sleep mode and stop modes register 2, Address offset: 0x7C */
  __IO uint32_t APB2SMENR;   /*!< RCC APB2 peripheral clocks enable in sleep mode and stop modes register, Address offset: 0x80 */
  uint32_t      RESERVED6;   /*!< Reserved,                                                                Address offset: 0x84 */
  __IO uint32_t CCIPR;       /*!< RCC peripherals independent clock configuration register,                Address offset: 0x88 */
  uint32_t      RESERVED7;   /*!< Reserved,                                                                Address offset: 0x8C */
  __IO uint32_t BDCR;        /*!< RCC backup domain control register,                                      Address offset: 0x90 */
  __IO uint32_t CSR;         /*!< RCC clock control & status register,                                     Address offset: 0x94 */
  __IO uint32_t CRRCR;       /*!< RCC clock recovery RC register,                                          Address offset: 0x98 */
} RCC_TypeDef;

/**
  * @brief Real-Time Clock
  */

typedef struct
{
  __IO uint32_t TR;          /*!< RTC time register,                              Address offset: 0x00 */
  __IO uint32_t DR;          /*!< RTC date register,                              Address offset: 0x04 */
  __IO uint32_t SSR;         /*!< RTC sub second register,                        Address offset: 0x08 */
  __IO uint32_t ICSR;        /*!< RTC initialization control and status register, Address offset: 0x0C */
  __IO uint32_t PRER;        /*!< RTC prescaler register,                         Address offset: 0x10 */
  __IO uint32_t WUTR;        /*!< RTC wakeup timer register,                      Address offset: 0x14 */
  __IO uint32_t CR;          /*!< RTC control register,                           Address offset: 0x18 */
       uint32_t RESERVED0;   /*!< Reserved,                                       Address offset: 0x1C */
       uint32_t RESERVED1;   /*!< Reserved,                                       Address offset: 0x20 */
  __IO uint32_t WPR;         /*!< RTC write protection register,                  Address offset: 0x24 */
  __IO uint32_t CALR;        /*!< RTC calibration register,                       Address offset: 0x28 */
  __IO uint32_t SHIFTR;      /*!< RTC shift control register,                     Address offset: 0x2C */
  __IO uint32_t TSTR;        /*!< RTC time stamp time register,                   Address offset: 0x30 */
  __IO uint32_t TSDR;        /*!< RTC time stamp date register,                   Address offset: 0x34 */
  __IO uint32_t TSSSR;       /*!< RTC time-stamp sub second register,             Address offset: 0x38 */
       uint32_t RESERVED2;   /*!< Reserved,                                       Address offset: 0x3C */
  __IO uint32_t ALRMAR;      /*!< RTC alarm A register,                           Address offset: 0x40 */
  __IO uint32_t ALRMASSR;    /*!< RTC alarm A sub second register,                Address offset: 0x44 */
  __IO uint32_t ALRMBR;      /*!< RTC alarm B register,                           Address offset: 0x48 */
  __IO uint32_t ALRMBSSR;    /*!< RTC alarm B sub second register,                Address offset: 0x4C */
  __IO uint32_t SR;          /*!< RTC Status register,                            Address offset: 0x50 */
  __IO uint32_t MISR;        /*!< RTC masked interrupt status register,           Address offset: 0x54 */
       uint32_t RESERVED3;   /*!< Reserved,                                       Address offset: 0x58 */
  __IO uint32_t SCR;         /*!< RTC status Clear register,                      Address offset: 0x5C */
} RTC_TypeDef;

/**
  * @brief Tamper and backup registers
  */
typedef struct
{
  __IO uint32_t CR1;         /*!< TAMP configuration register 1,            Address offset: 0x00 */
  __IO uint32_t CR2;         /*!< TAMP configuration register 2,            Address offset: 0x04 */
       uint32_t RESERVED0;   /*!< Reserved,                                 Address offset: 0x08 */
  __IO uint32_t FLTCR;       /*!< TAMP filter control register,             Address offset: 0x0C */
       uint32_t RESERVED1[7];/*!< Reserved,                                 Address offset: 0x10 -- 0x28 */
  __IO uint32_t IER;         /*!< TAMP interrupt enable register,           Address offset: 0x2C */
  __IO uint32_t SR;          /*!< TAMP status register,                     Address offset: 0x30 */
  __IO uint32_t MISR;        /*!< TAMP masked interrupt status register,    Address offset: 0x34 */
       uint32_t RESERVED2;   /*!< Reserved,                                 Address offset: 0x38 */
  __IO uint32_t SCR;         /*!< TAMP status clear register,               Address offset: 0x3C */
       uint32_t RESERVED3[48];/*!< Reserved,                                Address offset: 0x40 -- 0xFC */
  __IO uint32_t BKP0R;       /*!< TAMP backup register 0,                   Address offset: 0x100 */
  __IO uint32_t BKP1R;       /*!< TAMP backup register 1,                   Address offset: 0x104 */
  __IO uint32_t BKP2R;       /*!< TAMP backup register 2,                   Address offset: 0x108 */
  __IO uint32_t BKP3R;       /*!< TAMP backup register 3,                   Address offset: 0x10C */
  __IO uint32_t BKP4R;       /*!< TAMP backup register 4,                   Address offset: 0x110 */
  __IO uint32_t BKP5R;       /*!< TAMP backup register 5,                   Address offset: 0x114 */
  __IO uint32_t BKP6R;       /*!< TAMP backup register 6,                   Address offset: 0x118 */
  __IO uint32_t BKP7R;       /*!< TAMP backup register 7,                   Address offset: 0x11C */
  __IO uint32_t BKP8R;       /*!< TAMP backup register 8,                   Address offset: 0x120 */
  __IO uint32_t BKP9R;       /*!< TAMP backup register 9,                   Address offset: 0x124 */
  __IO uint32_t BKP10R;      /*!< TAMP backup register 10,                  Address offset: 0x128 */
  __IO uint32_t BKP11R;      /*!< TAMP backup register 11,                  Address offset: 0x12C */
  __IO uint32_t BKP12R;      /*!< TAMP backup register 12,                  Address offset: 0x130 */
  __IO uint32_t BKP13R;      /*!< TAMP backup register 13,                  Address offset: 0x134 */
  __IO uint32_t BKP14R;      /*!< TAMP backup register 14,                  Address offset: 0x138 */
  __IO uint32_t BKP15R;      /*!< TAMP backup register 15,                  Address offset: 0x13C */
  __IO uint32_t BKP16R;      /*!< TAMP backup register 16,                  Address offset: 0x140 */
  __IO uint32_t BKP17R;      /*!< TAMP backup register 17,                  Address offset: 0x144 */
  __IO uint32_t BKP18R;      /*!< TAMP backup register 18,                  Address offset: 0x148 */
  __IO uint32_t BKP19R;      /*!< TAMP backup register 19,                  Address offset: 0x14C */
  __IO uint32_t BKP20R;      /*!< TAMP backup register 20,                  Address offset: 0x150 */
  __IO uint32_t BKP21R;      /*!< TAMP backup register 21,                  Address offset: 0x154 */
  __IO uint32_t BKP22R;      /*!< TAMP backup register 22,                  Address offset: 0x158 */
  __IO uint32_t BKP23R;      /*!< TAMP backup register 23,                  Address offset: 0x15C */
  __IO uint32_t BKP24R;      /*!< TAMP backup register 24,                  Address offset: 0x160 */
  __IO uint32_t BKP25R;      /*!< TAMP backup register 25,                  Address offset: 0x164 */
  __IO uint32_t BKP26R;      /*!< TAMP backup register 26,                  Address offset: 0x168 */
  __IO uint32_t BKP27R;      /*!< TAMP backup register 27,                  Address offset: 0x16C */
  __IO uint32_t BKP28R;      /*!< TAMP backup register 28,                  Address offset: 0x170 */
  __IO uint32_t BKP29R;      /*!< TAMP backup register 29,                  Address offset: 0x174 */
  __IO uint32_t BKP30R;      /*!< TAMP backup register 30,                  Address offset: 0x178 */
  __IO uint32_t BKP31R;      /*!< TAMP backup register 31,                  Address offset: 0x17C */
} TAMP_TypeDef;



/**
  * @brief Serial Peripheral Interface
  */

typedef struct
{
  __IO uint32_t CR1;         /*!< SPI Control register 1,                              Address offset: 0x00 */
  __IO uint32_t CR2;         /*!< SPI Control register 2,                              Address offset: 0x04 */
  __IO uint32_t SR;          /*!< SPI Status register,                                 Address offset: 0x08 */
  __IO uint32_t DR;          /*!< SPI data register,                                   Address offset: 0x0C */
  __IO uint32_t CRCPR;       /*!< SPI CRC polynomial register,                         Address offset: 0x10 */
  __IO uint32_t RXCRCR;      /*!< SPI Rx CRC register,                                 Address offset: 0x14 */
  __IO uint32_t TXCRCR;      /*!< SPI Tx CRC register,                                 Address offset: 0x18 */
} SPI_TypeDef;


/**
  * @brief System configuration controller
  */

typedef struct
{
  __IO uint32_t MEMRMP;      /*!< SYSCFG memory remap register,                      Address offset: 0x00      */
  __IO uint32_t CFGR1;       /*!< SYSCFG configuration register 1,                   Address offset: 0x04      */
  __IO uint32_t EXTICR[4];   /*!< SYSCFG external interrupt configuration registers, Address offset: 0x08-0x14 */
  __IO uint32_t SCSR;        /*!< SYSCFG SRAM2 control and status register,          Address offset: 0x18      */
  __IO uint32_t CFGR2;       /*!< SYSCFG configuration register 2,                   Address offset: 0x1C      */
  __IO uint32_t SWPR;        /*!< SYSCFG SRAM2 write protection register,            Address offset: 0x20      */
  __IO uint32_t SKR;         /*!< SYSCFG SRAM2 key register,                         Address offset: 0x24      */
} SYSCFG_TypeDef;


/**
  * @brief TIM
  */

typedef struct
{
  __IO uint32_t CR1;         /*!< TIM control register 1,                   Address offset: 0x00 */
  __IO uint32_t CR2;         /*!< TIM control register 2,                   Address offset: 0x04 */
  __IO uint32_t SMCR;        /*!< TIM slave mode control register,          Address offset: 0x08 */
  __IO uint32_t DIER;        /*!< TIM DMA/interrupt enable register,        Address offset: 0x0C */
  __IO uint32_t SR;          /*!< TIM status register,                      Address offset: 0x10 */
  __IO uint32_t EGR;         /*!< TIM event generation register,            Address offset: 0x14 */
  __IO uint32_t CCMR1;       /*!< TIM capture/compare mode register 1,      Address offset: 0x18 */
  __IO uint32_t CCMR2;       /*!< TIM capture/compare mode register 2,      Address offset: 0x1C */
  __IO uint32_t CCER;        /*!< TIM capture/compare enable register,      Address offset: 0x20 */
  __IO uint32_t CNT;         /*!< TIM counter register,                     Address offset: 0x24 */
  __IO uint32_t PSC;         /*!< TIM prescaler,                            Address offset: 0x28 */
  __IO uint32_t ARR;         /*!< TIM auto-reload register,                 Address offset: 0x2C */
  __IO uint32_t RCR;         /*!< TIM repetition counter register,          Address offset: 0x30 */
  __IO uint32_t CCR1;        /*!< TIM capture/compare register 1,           Address offset: 0x34 */
  __IO uint32_t CCR2;        /*!< TIM capture/compare register 2,           Address offset: 0x38 */
  __IO uint32_t CCR3;        /*!< TIM capture/compare register 3,           Address offset: 0x3C */
  __IO uint32_t CCR4;        /*!< TIM capture/compare register 4,           Address offset: 0x40 */
  __IO uint32_t BDTR;        /*!< TIM break and dead-time register,         Address offset: 0x44 */
  __IO uint32_t DCR;         /*!< TIM DMA control register,                 Address offset: 0x48 */
  __IO uint32_t DMAR;        /*!< TIM DMA address for full transfer,        Address offset: 0x4C */
  __IO uint32_t OR1;         /*!< TIM option register 1,                    Address offset: 0x50 */
  __IO uint32_t CCMR3;       /*!< TIM capture/compare mode register 3,      Address offset: 0x54 */
  __IO uint32_t CCR5;        /*!< TIM capture/compare register5,            Address offset: 0x58 */
  __IO uint32_t CCR6;        /*!< TIM capture/compare register6,            Address offset: 0x5C */
  __IO uint32_t OR2;         /*!< TIM option register 2,                    Address offset: 0x60 */
  __IO uint32_t OR3;         /*!< TIM option register 3,                    Address offset: 0x64 */
} TIM_TypeDef;


/**
  * @brief Touch Sensing Controller (TSC)
  */

typedef struct
{
  __IO uint32_t CR;            /*!< TSC control register,                                     Address offset: 0x00 */
  __IO uint32_t IER;           /*!< TSC interrupt enable register,                            Address offset: 0x04 */
  __IO uint32_t ICR;           /*!< TSC interrupt clear register,                             Address offset: 0x08 */
  __IO uint32_t ISR;           /*!< TSC interrupt status register,                            Address offset: 0x0C */
  __IO uint32_t IOHCR;         /*!< TSC I/O hysteresis control register,                      Address offset: 0x10 */
  uint32_t      RESERVED1;     /*!< Reserved,                                                 Address offset: 0x14 */
  __IO uint32_t IOASCR;        /*!< TSC I/O analog switch control register,                   Address offset: 0x18 */
  uint32_t      RESERVED2;     /*!< Reserved,                                                 Address offset: 0x1C */
  __IO uint32_t IOSCR;         /*!< TSC I/O sampling control register,                        Address offset: 0x20 */
  uint32_t      RESERVED3;     /*!< Reserved,                                                 Address offset: 0x24 */
  __IO uint32_t IOCCR;         /*!< TSC I/O channel control register,                         Address offset: 0x28 */
  uint32_t      RESERVED4;     /*!< Reserved,                                                 Address offset: 0x2C */
  __IO uint32_t IOGCSR;        /*!< TSC I/O group control status register,                    Address offset: 0x30 */
  __IO uint32_t IOGXCR[7];     /*!< TSC I/O group x counter register,                         Address offset: 0x34-4C */
} TSC_TypeDef;

/**
  * @brief Universal Synchronous Asynchronous Receiver Transmitter
  */

typedef struct
{
  __IO uint32_t CR1;         /*!< USART Control register 1,                 Address offset: 0x00 */
  __IO uint32_t CR2;         /*!< USART Control register 2,                 Address offset: 0x04 */
  __IO uint32_t CR3;         /*!< USART Control register 3,                 Address offset: 0x08 */
  __IO uint32_t BRR;         /*!< USART Baud rate register,                 Address offset: 0x0C */
  __IO uint16_t GTPR;        /*!< USART Guard time and prescaler register,  Address offset: 0x10 */
  uint16_t  RESERVED2;       /*!< Reserved, 0x12                                                 */
  __IO uint32_t RTOR;        /*!< USART Receiver Time Out register,         Address offset: 0x14 */
  __IO uint16_t RQR;         /*!< USART Request register,                   Address offset: 0x18 */
  uint16_t  RESERVED3;       /*!< Reserved, 0x1A                                                 */
  __IO uint32_t ISR;         /*!< USART Interrupt and status register,      Address offset: 0x1C */
  __IO uint32_t ICR;         /*!< USART Interrupt flag Clear register,      Address offset: 0x20 */
  __IO uint16_t RDR;         /*!< USART Receive Data register,              Address offset: 0x24 */
  uint16_t  RESERVED4;       /*!< Reserved, 0x26                                                 */
  __IO uint16_t TDR;         /*!< USART Transmit Data register,             Address offset: 0x28 */
  uint16_t  RESERVED5;       /*!< Reserved, 0x2A                                                 */
} USART_TypeDef;

/**
  * @brief Universal Serial Bus Full Speed Device
  */

typedef struct
{
  __IO uint16_t EP0R;            /*!< USB Endpoint 0 register,                Address offset: 0x00 */
  __IO uint16_t RESERVED0;       /*!< Reserved */
  __IO uint16_t EP1R;            /*!< USB Endpoint 1 register,                Address offset: 0x04 */
  __IO uint16_t RESERVED1;       /*!< Reserved */
  __IO uint16_t EP2R;            /*!< USB Endpoint 2 register,                Address offset: 0x08 */
  __IO uint16_t RESERVED2;       /*!< Reserved */
  __IO uint16_t EP3R;            /*!< USB Endpoint 3 register,                Address offset: 0x0C */
  __IO uint16_t RESERVED3;       /*!< Reserved */
  __IO uint16_t EP4R;            /*!< USB Endpoint 4 register,                Address offset: 0x10 */
  __IO uint16_t RESERVED4;       /*!< Reserved */
  __IO uint16_t EP5R;            /*!< USB Endpoint 5 register,                Address offset: 0x14 */
  __IO uint16_t RESERVED5;       /*!< Reserved */
  __IO uint16_t EP6R;            /*!< USB Endpoint 6 register,                Address offset: 0x18 */
  __IO uint16_t RESERVED6;       /*!< Reserved */
  __IO uint16_t EP7R;            /*!< USB Endpoint 7 register,                Address offset: 0x1C */
  __IO uint16_t RESERVED7[17];   /*!< Reserved */
  __IO uint16_t CNTR;            /*!< Control register,                       Address offset: 0x40 */
  __IO uint16_t RESERVED8;       /*!< Reserved */
  __IO uint16_t ISTR;            /*!< Interrupt status register,              Address offset: 0x44 */
  __IO uint16_t RESERVED9;       /*!< Reserved */
  __IO uint16_t FNR;             /*!< Frame number register,                  Address offset: 0x48 */
  __IO uint16_t RESERVEDA;       /*!< Reserved */
  __IO uint16_t DADDR;           /*!< Device address register,                Address offset: 0x4C */
  __IO uint16_t RESERVEDB;       /*!< Reserved */
  __IO uint16_t BTABLE;          /*!< Buffer Table address register,          Address offset: 0x50 */
  __IO uint16_t RESERVEDC;       /*!< Reserved */
  __IO uint16_t LPMCSR;          /*!< LPM Control and Status register,        Address offset: 0x54 */
  __IO uint16_t RESERVEDD;       /*!< Reserved */
  __IO uint16_t BCDR;            /*!< Battery Charging detector register,     Address offset: 0x58 */
  __IO uint16_t RESERVEDE;       /*!< Reserved */
} USB_TypeDef;


/**
  * @brief Window WATCHDOG
  */

typedef struct
{
  __IO uint32_t CR;          /*!< WWDG Control register,       Address offset: 0x00 */
  __IO uint32_t CFR;         /*!< WWDG Configuration register, Address offset: 0x04 */
  __IO uint32_t SR;          /*!< WWDG Status register,        Address offset: 0x08 */
} WWDG_TypeDef;

/**
  * @brief AES hardware accelerator
  */

typedef struct
{
  __IO uint32_t CR;          /*!< AES control register,                        Address offset: 0x00 */
  __IO uint32_t SR;          /*!< AES status register,                         Address offset: 0x04 */
  __IO uint32_t DINR;        /*!< AES data input register,                     Address offset: 0x08 */
  __IO uint32_t DOUTR;       /*!< AES data output register,                    Address offset: 0x0C */
  __IO uint32_t KEYR0;       /*!< AES key register 0,                          Address offset: 0x10 */
  __IO uint32_t KEYR1;       /*!< AES key register 1,                          Address offset: 0x14 */
  __IO uint32_t KEYR2;       /*!< AES key register 2,                          Address offset: 0x18 */
  __IO uint32_t KEYR3;       /*!< AES key register 3,                          Address offset: 0x1C */
  __IO uint32_t IVR0;        /*!< AES initialization vector register 0,        Address offset: 0x20 */
  __IO uint32_t IVR1;        /*!< AES initialization vector register 1,        Address offset: 0x24 */
  __IO uint32_t IVR2;        /*!< AES initialization vector register 2,        Address offset: 0x28 */
  __IO uint32_t IVR3;        /*!< AES initialization vector register 3,        Address offset: 0x2C */
  __IO uint32_t KEYR4;       /*!< AES key register 4,                          Address offset: 0x30 */
  __IO uint32_t KEYR5;       /*!< AES key register 5,                          Address offset: 0x34 */
  __IO uint32_t KEYR6;       /*!< AES key register 6,                          Address offset: 0x38 */
  __IO uint32_t KEYR7;       /*!< AES key register 7,                          Address offset: 0x3C */
  __IO uint32_t SUSP0R;      /*!< AES Suspend register 0,                      Address offset: 0x40 */
  __IO uint32_t SUSP1R;      /*!< AES Suspend register 1,                      Address offset: 0x44 */
  __IO uint32_t SUSP2R;      /*!< AES Suspend register 2,                      Address offset: 0x48 */
  __IO uint32_t SUSP3R;      /*!< AES Suspend register 3,                      Address offset: 0x4C */
  __IO uint32_t SUSP4R;      /*!< AES Suspend register 4,                      Address offset: 0x50 */
  __IO uint32_t SUSP5R;      /*!< AES Suspend register 5,                      Address offset: 0x54 */
  __IO uint32_t SUSP6R;      /*!< AES Suspend register 6,                      Address offset: 0x58 */
  __IO uint32_t SUSP7R;      /*!< AES Suspend register 7,                      Address offset: 0x6C */
} AES_TypeDef;

/**
  * @brief RNG
  */

typedef struct
{
  __IO uint32_t CR;  /*!< RNG control register, Address offset: 0x00 */
  __IO uint32_t SR;  /*!< RNG status register,  Address offset: 0x04 */
  __IO uint32_t DR;  /*!< RNG data register,    Address offset: 0x08 */
} RNG_TypeDef;

/**
  * @}
  */

/** @addtogroup Peripheral_memory_map
  * @{
  */
#define FLASH_BASE            (0x08000000UL) /*!< FLASH(up to 128 KB) base address */
#define SRAM1_BASE            (0x20000000UL) /*!< SRAM1(up to 32 KB) base address */
#define SRAM2_BASE            (0x10000000UL) /*!< SRAM2(8 KB) base address */
#define PERIPH_BASE           (0x40000000UL) /*!< Peripheral base address */
#define QSPI_BASE             (0x90000000UL) /*!< QUADSPI memories accessible over AHB base address */

#define QSPI_R_BASE           (0xA0001000UL) /*!< QUADSPI control registers base address */
#define SRAM1_BB_BASE         (0x22000000UL) /*!< SRAM1(96 KB) base address in the bit-band region */
#define PERIPH_BB_BASE        (0x42000000UL) /*!< Peripheral base address in the bit-band region */

/* Legacy defines */
#define SRAM_BASE             SRAM1_BASE
#define SRAM_BB_BASE          SRAM1_BB_BASE

#define SRAM1_SIZE_MAX        (0x00008000UL) /*!< maximum SRAM1 size (up to 32 KBytes) */
#define SRAM2_SIZE            (0x00002000UL) /*!< SRAM2 size (8 KBytes) */

/*!< Peripheral memory map */
#define APB1PERIPH_BASE        PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000UL)
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000UL)
#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x08000000UL)


/*!< APB1 peripherals */
#define TIM2_BASE             (APB1PERIPH_BASE + 0x0000UL)
#define TIM6_BASE             (APB1PERIPH_BASE + 0x1000UL)
#define RTC_BASE              (APB1PERIPH_BASE + 0x2800UL)
#define WWDG_BASE             (APB1PERIPH_BASE + 0x2C00UL)
#define IWDG_BASE             (APB1PERIPH_BASE + 0x3000UL)
#define TAMP_BASE             (APB1PERIPH_BASE + 0x3400U)
#define SPI2_BASE             (APB1PERIPH_BASE + 0x3800UL)
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400UL)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800UL)
#define I2C1_BASE             (APB1PERIPH_BASE + 0x5400UL)
#define I2C2_BASE             (APB1PERIPH_BASE + 0x5800UL)
#define I2C3_BASE             (APB1PERIPH_BASE + 0x5C00UL)
#define CRS_BASE              (APB1PERIPH_BASE + 0x6000UL)
#define USB_BASE              (APB1PERIPH_BASE + 0x6800UL)  /*!< USB_IP Peripheral Registers base address */
#define USB_PMAADDR           (APB1PERIPH_BASE + 0x6C00UL)  /*!< USB_IP Packet Memory Area base address */
#define PWR_BASE              (APB1PERIPH_BASE + 0x7000UL)
#define OPAMP_BASE            (APB1PERIPH_BASE + 0x7800UL)
#define OPAMP1_BASE           (APB1PERIPH_BASE + 0x7800UL)
#define LPTIM1_BASE           (APB1PERIPH_BASE + 0x7C00UL)
#define LPUART1_BASE          (APB1PERIPH_BASE + 0x8000UL)
#define LPTIM2_BASE           (APB1PERIPH_BASE + 0x9400UL)


/*!< APB2 peripherals */
#define SYSCFG_BASE           (APB2PERIPH_BASE + 0x0000UL)
#define COMP1_BASE            (APB2PERIPH_BASE + 0x0200UL)
#define EXTI_BASE             (APB2PERIPH_BASE + 0x0400UL)
#define FIREWALL_BASE         (APB2PERIPH_BASE + 0x1C00UL)
#define TIM1_BASE             (APB2PERIPH_BASE + 0x2C00UL)
#define SPI1_BASE             (APB2PERIPH_BASE + 0x3000UL)
#define USART1_BASE           (APB2PERIPH_BASE + 0x3800UL)
#define TIM15_BASE            (APB2PERIPH_BASE + 0x4000UL)
#define TIM16_BASE            (APB2PERIPH_BASE + 0x4400UL)

/*!< AHB1 peripherals */
#define DMA1_BASE             (AHB1PERIPH_BASE)
#define DMA2_BASE             (AHB1PERIPH_BASE + 0x0400UL)
#define RCC_BASE              (AHB1PERIPH_BASE + 0x1000UL)
#define FLASH_R_BASE          (AHB1PERIPH_BASE + 0x2000UL)
#define CRC_BASE              (AHB1PERIPH_BASE + 0x3000UL)
#define TSC_BASE              (AHB1PERIPH_BASE + 0x4000UL)


#define DMA1_Channel1_BASE    (DMA1_BASE + 0x0008UL)
#define DMA1_Channel2_BASE    (DMA1_BASE + 0x001CUL)
#define DMA1_Channel3_BASE    (DMA1_BASE + 0x0030UL)
#define DMA1_Channel4_BASE    (DMA1_BASE + 0x0044UL)
#define DMA1_Channel5_BASE    (DMA1_BASE + 0x0058UL)
#define DMA1_Channel6_BASE    (DMA1_BASE + 0x006CUL)
#define DMA1_Channel7_BASE    (DMA1_BASE + 0x0080UL)
#define DMA1_CSELR_BASE       (DMA1_BASE + 0x00A8UL)


#define DMA2_Channel1_BASE    (DMA2_BASE + 0x0008UL)
#define DMA2_Channel2_BASE    (DMA2_BASE + 0x001CUL)
#define DMA2_Channel3_BASE    (DMA2_BASE + 0x0030UL)
#define DMA2_Channel4_BASE    (DMA2_BASE + 0x0044UL)
#define DMA2_Channel5_BASE    (DMA2_BASE + 0x0058UL)
#define DMA2_Channel6_BASE    (DMA2_BASE + 0x006CUL)
#define DMA2_Channel7_BASE    (DMA2_BASE + 0x0080UL)
#define DMA2_CSELR_BASE       (DMA2_BASE + 0x00A8UL)


/*!< AHB2 peripherals */
#define GPIOA_BASE            (AHB2PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE            (AHB2PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE            (AHB2PERIPH_BASE + 0x0800UL)
#define GPIOD_BASE            (AHB2PERIPH_BASE + 0x0C00UL)
#define GPIOH_BASE            (AHB2PERIPH_BASE + 0x1C00UL)


#define ADC1_BASE             (AHB2PERIPH_BASE + 0x08040000UL)
#define ADC2_BASE             (AHB2PERIPH_BASE + 0x08040100UL)
#define ADC12_COMMON_BASE     (AHB2PERIPH_BASE + 0x08040300UL)


#define AES_BASE              (AHB2PERIPH_BASE + 0x08060000UL)
#define RNG_BASE              (AHB2PERIPH_BASE + 0x08060800UL)



/* Debug MCU registers base address */
#define DBGMCU_BASE           (0xE0042000UL)


#define PACKAGE_BASE          (0x1FFF7500UL)        /*!< Package data register base address     */
#define UID_BASE              (0x1FFF7590UL)        /*!< Unique device ID register base address */
#define FLASHSIZE_BASE        (0x1FFF75E0UL)        /*!< Flash size data register base address  */
/**
  * @}
  */

/** @addtogroup Peripheral_declaration
  * @{
  */
#define TIM2                ((TIM_TypeDef *) TIM2_BASE)
#define TIM6                ((TIM_TypeDef *) TIM6_BASE)
#define RTC                 ((RTC_TypeDef *) RTC_BASE)
#define WWDG                ((WWDG_TypeDef *) WWDG_BASE)
#define IWDG                ((IWDG_TypeDef *) IWDG_BASE)
#define TAMP                ((TAMP_TypeDef *) TAMP_BASE)
#define SPI2                ((SPI_TypeDef *) SPI2_BASE)
#define USART2              ((USART_TypeDef *) USART2_BASE)
#define USART3              ((USART_TypeDef *) USART3_BASE)
#define I2C1                ((I2C_TypeDef *) I2C1_BASE)
#define I2C2                ((I2C_TypeDef *) I2C2_BASE)
#define I2C3                ((I2C_TypeDef *) I2C3_BASE)
#define CRS                 ((CRS_TypeDef *) CRS_BASE)
#define USB                 ((USB_TypeDef *) USB_BASE)
#define PWR                 ((PWR_TypeDef *) PWR_BASE)
#define OPAMP               ((OPAMP_TypeDef *) OPAMP_BASE)
#define OPAMP1              ((OPAMP_TypeDef *) OPAMP1_BASE)
#define OPAMP1_COMMON       ((OPAMP_Common_TypeDef *) OPAMP1_BASE)
#define LPTIM1              ((LPTIM_TypeDef *) LPTIM1_BASE)
#define LPUART1             ((USART_TypeDef *) LPUART1_BASE)
#define LPTIM2              ((LPTIM_TypeDef *) LPTIM2_BASE)

#define SYSCFG              ((SYSCFG_TypeDef *) SYSCFG_BASE)
#define COMP1               ((COMP_TypeDef *) COMP1_BASE)
#define EXTI                ((EXTI_TypeDef *) EXTI_BASE)
#define FIREWALL            ((FIREWALL_TypeDef *) FIREWALL_BASE)
#define TIM1                ((TIM_TypeDef *) TIM1_BASE)
#define SPI1                ((SPI_TypeDef *) SPI1_BASE)
#define USART1              ((USART_TypeDef *) USART1_BASE)
#define TIM15               ((TIM_TypeDef *) TIM15_BASE)
#define TIM16               ((TIM_TypeDef *) TIM16_BASE)
#define DMA1                ((DMA_TypeDef *) DMA1_BASE)
#define DMA2                ((DMA_TypeDef *) DMA2_BASE)
#define RCC                 ((RCC_TypeDef *) RCC_BASE)
#define FLASH               ((FLASH_TypeDef *) FLASH_R_BASE)
#define CRC                 ((CRC_TypeDef *) CRC_BASE)
#define TSC                 ((TSC_TypeDef *) TSC_BASE)

#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD               ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOH               ((GPIO_TypeDef *) GPIOH_BASE)
#define ADC1                ((ADC_TypeDef *) ADC1_BASE)
#define ADC2                ((ADC_TypeDef *) ADC2_BASE)
#define ADC12_COMMON        ((ADC_Common_TypeDef *) ADC12_COMMON_BASE)
#define AES                 ((AES_TypeDef *) AES_BASE)
#define RNG                 ((RNG_TypeDef *) RNG_BASE)


#define DMA1_Channel1       ((DMA_Channel_TypeDef *) DMA1_Channel1_BASE)
#define DMA1_Channel2       ((DMA_Channel_TypeDef *) DMA1_Channel2_BASE)
#define DMA1_Channel3       ((DMA_Channel_TypeDef *) DMA1_Channel3_BASE)
#define DMA1_Channel4       ((DMA_Channel_TypeDef *) DMA1_Channel4_BASE)
#define DMA1_Channel5       ((DMA_Channel_TypeDef *) DMA1_Channel5_BASE)
#define DMA1_Channel6       ((DMA_Channel_TypeDef *) DMA1_Channel6_BASE)
#define DMA1_Channel7       ((DMA_Channel_TypeDef *) DMA1_Channel7_BASE)
#define DMA1_CSELR          ((DMA_Request_TypeDef *) DMA1_CSELR_BASE)


#define DMA2_Channel1       ((DMA_Channel_TypeDef *) DMA2_Channel1_BASE)
#define DMA2_Channel2       ((DMA_Channel_TypeDef *) DMA2_Channel2_BASE)
#define DMA2_Channel3       ((DMA_Channel_TypeDef *) DMA2_Channel3_BASE)
#define DMA2_Channel4       ((DMA_Channel_TypeDef *) DMA2_Channel4_BASE)
#define DMA2_Channel5       ((DMA_Channel_TypeDef *) DMA2_Channel5_BASE)
#define DMA2_Channel6       ((DMA_Channel_TypeDef *) DMA2_Channel6_BASE)
#define DMA2_Channel7       ((DMA_Channel_TypeDef *) DMA2_Channel7_BASE)
#define DMA2_CSELR          ((DMA_Request_TypeDef *) DMA2_CSELR_BASE)



#define QUADSPI             ((QUADSPI_TypeDef *) QSPI_R_BASE)

#define DBGMCU              ((DBGMCU_TypeDef *) DBGMCU_BASE)

/**
  * @}
  */

/** @addtogroup Exported_constants
  * @{
  */

/** @addtogroup Peripheral_Registers_Bits_Definition
  * @{
  */

/******************************************************************************/
/*                         Peripheral Registers_Bits_Definition               */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                        Analog to Digital Converter                         */
/*                                                                            */
/******************************************************************************/

/*
 * @brief Specific device feature definitions (not present on all devices in the STM32L4 serie)
 */
#define ADC_MULTIMODE_SUPPORT                          /*!< ADC feature available only on specific devices: multimode available on devices with several ADC instances */

/********************  Bit definition for ADC_ISR register  *******************/
#define ADC_ISR_ADRDY_Pos              (0U)
#define ADC_ISR_ADRDY_Msk              (0x1UL << ADC_ISR_ADRDY_Pos)            /*!< 0x00000001 */
#define ADC_ISR_ADRDY                  ADC_ISR_ADRDY_Msk                       /*!< ADC ready flag */
#define ADC_ISR_EOSMP_Pos              (1U)
#define ADC_ISR_EOSMP_Msk              (0x1UL << ADC_ISR_EOSMP_Pos)            /*!< 0x00000002 */
#define ADC_ISR_EOSMP                  ADC_ISR_EOSMP_Msk                       /*!< ADC group regular end of sampling flag */
#define ADC_ISR_EOC_Pos                (2U)
#define ADC_ISR_EOC_Msk                (0x1UL << ADC_ISR_EOC_Pos)              /*!< 0x00000004 */
#define ADC_ISR_EOC                    ADC_ISR_EOC_Msk                         /*!< ADC group regular end of unitary conversion flag */
#define ADC_ISR_EOS_Pos                (3U)
#define ADC_ISR_EOS_Msk                (0x1UL << ADC_ISR_EOS_Pos)              /*!< 0x00000008 */
#define ADC_ISR_EOS                    ADC_ISR_EOS_Msk                         /*!< ADC group regular end of sequence conversions flag */
#define ADC_ISR_OVR_Pos                (4U)
#define ADC_ISR_OVR_Msk                (0x1UL << ADC_ISR_OVR_Pos)              /*!< 0x00000010 */
#define ADC_ISR_OVR                    ADC_ISR_OVR_Msk                         /*!< ADC group regular overrun flag */
#define ADC_ISR_JEOC_Pos               (5U)
#define ADC_ISR_JEOC_Msk               (0x1UL << ADC_ISR_JEOC_Pos)             /*!< 0x00000020 */
#define ADC_ISR_JEOC                   ADC_ISR_JEOC_Msk                        /*!< ADC group injected end of unitary conversion flag */
#define ADC_ISR_JEOS_Pos               (6U)
#define ADC_ISR_JEOS_Msk               (0x1UL << ADC_ISR_JEOS_Pos)             /*!< 0x00000040 */
#define ADC_ISR_JEOS                   ADC_ISR_JEOS_Msk                        /*!< ADC group injected end of sequence conversions flag */
#define ADC_ISR_AWD1_Pos               (7U)
#define ADC_ISR_AWD1_Msk               (0x1UL << ADC_ISR_AWD1_Pos)             /*!< 0x00000080 */
#define ADC_ISR_AWD1                   ADC_ISR_AWD1_Msk                        /*!< ADC analog watchdog 1 flag */
#define ADC_ISR_AWD2_Pos               (8U)
#define ADC_ISR_AWD2_Msk               (0x1UL << ADC_ISR_AWD2_Pos)             /*!< 0x00000100 */
#define ADC_ISR_AWD2                   ADC_ISR_AWD2_Msk                        /*!< ADC analog watchdog 2 flag */
#define ADC_ISR_AWD3_Pos               (9U)
#define ADC_ISR_AWD3_Msk               (0x1UL << ADC_ISR_AWD3_Pos)             /*!< 0x00000200 */
#define ADC_ISR_AWD3                   ADC_ISR_AWD3_Msk                        /*!< ADC analog watchdog 3 flag */
#define ADC_ISR_JQOVF_Pos              (10U)
#define ADC_ISR_JQOVF_Msk              (0x1UL << ADC_ISR_JQOVF_Pos)            /*!< 0x00000400 */
#define ADC_ISR_JQOVF                  ADC_ISR_JQOVF_Msk                       /*!< ADC group injected contexts queue overflow flag */

/********************  Bit definition for ADC_IER register  *******************/
#define ADC_IER_ADRDYIE_Pos            (0U)
#define ADC_IER_ADRDYIE_Msk            (0x1UL << ADC_IER_ADRDYIE_Pos)          /*!< 0x00000001 */
#define ADC_IER_ADRDYIE                ADC_IER_ADRDYIE_Msk                     /*!< ADC ready interrupt */
#define ADC_IER_EOSMPIE_Pos            (1U)
#define ADC_IER_EOSMPIE_Msk            (0x1UL << ADC_IER_EOSMPIE_Pos)          /*!< 0x00000002 */
#define ADC_IER_EOSMPIE                ADC_IER_EOSMPIE_Msk                     /*!< ADC group regular end of sampling interrupt */
#define ADC_IER_EOCIE_Pos              (2U)
#define ADC_IER_EOCIE_Msk              (0x1UL << ADC_IER_EOCIE_Pos)            /*!< 0x00000004 */
#define ADC_IER_EOCIE                  ADC_IER_EOCIE_Msk                       /*!< ADC group regular end of unitary conversion interrupt */
#define ADC_IER_EOSIE_Pos              (3U)
#define ADC_IER_EOSIE_Msk              (0x1UL << ADC_IER_EOSIE_Pos)            /*!< 0x00000008 */
#define ADC_IER_EOSIE                  ADC_IER_EOSIE_Msk                       /*!< ADC group regular end of sequence conversions interrupt */
#define ADC_IER_OVRIE_Pos              (4U)
#define ADC_IER_OVRIE_Msk              (0x1UL << ADC_IER_OVRIE_Pos)            /*!< 0x00000010 */
#define ADC_IER_OVRIE                  ADC_IER_OVRIE_Msk                       /*!< ADC group regular overrun interrupt */
#define ADC_IER_JEOCIE_Pos             (5U)
#define ADC_IER_JEOCIE_Msk             (0x1UL << ADC_IER_JEOCIE_Pos)           /*!< 0x00000020 */
#define ADC_IER_JEOCIE                 ADC_IER_JEOCIE_Msk                      /*!< ADC group injected end of unitary conversion interrupt */
#define ADC_IER_JEOSIE_Pos             (6U)
#define ADC_IER_JEOSIE_Msk             (0x1UL << ADC_IER_JEOSIE_Pos)           /*!< 0x00000040 */
#define ADC_IER_JEOSIE                 ADC_IER_JEOSIE_Msk                      /*!< ADC group injected end of sequence conversions interrupt */
#define ADC_IER_AWD1IE_Pos             (7U)
#define ADC_IER_AWD1IE_Msk             (0x1UL << ADC_IER_AWD1IE_Pos)           /*!< 0x00000080 */
#define ADC_IER_AWD1IE                 ADC_IER_AWD1IE_Msk                      /*!< ADC analog watchdog 1 interrupt */
#define ADC_IER_AWD2IE_Pos             (8U)
#define ADC_IER_AWD2IE_Msk             (0x1UL << ADC_IER_AWD2IE_Pos)           /*!< 0x00000100 */
#define ADC_IER_AWD2IE                 ADC_IER_AWD2IE_Msk                      /*!< ADC analog watchdog 2 interrupt */
#define ADC_IER_AWD3IE_Pos             (9U)
#define ADC_IER_AWD3IE_Msk             (0x1UL << ADC_IER_AWD3IE_Pos)           /*!< 0x00000200 */
#define ADC_IER_AWD3IE                 ADC_IER_AWD3IE_Msk                      /*!< ADC analog watchdog 3 interrupt */
#define ADC_IER_JQOVFIE_Pos            (10U)
#define ADC_IER_JQOVFIE_Msk            (0x1UL << ADC_IER_JQOVFIE_Pos)          /*!< 0x00000400 */
#define ADC_IER_JQOVFIE                ADC_IER_JQOVFIE_Msk                     /*!< ADC group injected contexts queue overflow interrupt */

/* Legacy defines */
#define ADC_IER_ADRDY           (ADC_IER_ADRDYIE)
#define ADC_IER_EOSMP           (ADC_IER_EOSMPIE)
#define ADC_IER_EOC             (ADC_IER_EOCIE)
#define ADC_IER_EOS             (ADC_IER_EOSIE)
#define ADC_IER_OVR             (ADC_IER_OVRIE)
#define ADC_IER_JEOC            (ADC_IER_JEOCIE)
#define ADC_IER_JEOS            (ADC_IER_JEOSIE)
#define ADC_IER_AWD1            (ADC_IER_AWD1IE)
#define ADC_IER_AWD2            (ADC_IER_AWD2IE)
#define ADC_IER_AWD3            (ADC_IER_AWD3IE)
#define ADC_IER_JQOVF           (ADC_IER_JQOVFIE)

/********************  Bit definition for ADC_CR register  ********************/
#define ADC_CR_ADEN_Pos                (0U)
#define ADC_CR_ADEN_Msk                (0x1UL << ADC_CR_ADEN_Pos)              /*!< 0x00000001 */
#define ADC_CR_ADEN                    ADC_CR_ADEN_Msk                         /*!< ADC enable */
#define ADC_CR_ADDIS_Pos               (1U)
#define ADC_CR_ADDIS_Msk               (0x1UL << ADC_CR_ADDIS_Pos)             /*!< 0x00000002 */
#define ADC_CR_ADDIS                   ADC_CR_ADDIS_Msk                        /*!< ADC disable */
#define ADC_CR_ADSTART_Pos             (2U)
#define ADC_CR_ADSTART_Msk             (0x1UL << ADC_CR_ADSTART_Pos)           /*!< 0x00000004 */
#define ADC_CR_ADSTART                 ADC_CR_ADSTART_Msk                      /*!< ADC group regular conversion start */
#define ADC_CR_JADSTART_Pos            (3U)
#define ADC_CR_JADSTART_Msk            (0x1UL << ADC_CR_JADSTART_Pos)          /*!< 0x00000008 */
#define ADC_CR_JADSTART                ADC_CR_JADSTART_Msk                     /*!< ADC group injected conversion start */
#define ADC_CR_ADSTP_Pos               (4U)
#define ADC_CR_ADSTP_Msk               (0x1UL << ADC_CR_ADSTP_Pos)             /*!< 0x00000010 */
#define ADC_CR_ADSTP                   ADC_CR_ADSTP_Msk                        /*!< ADC group regular conversion stop */
#define ADC_CR_JADSTP_Pos              (5U)
#define ADC_CR_JADSTP_Msk              (0x1UL << ADC_CR_JADSTP_Pos)            /*!< 0x00000020 */
#define ADC_CR_JADSTP                  ADC_CR_JADSTP_Msk                       /*!< ADC group injected conversion stop */
#define ADC_CR_ADVREGEN_Pos            (28U)
#define ADC_CR_ADVREGEN_Msk            (0x1UL << ADC_CR_ADVREGEN_Pos)          /*!< 0x10000000 */
#define ADC_CR_ADVREGEN                ADC_CR_ADVREGEN_Msk                     /*!< ADC voltage regulator enable */
#define ADC_CR_DEEPPWD_Pos             (29U)
#define ADC_CR_DEEPPWD_Msk             (0x1UL << ADC_CR_DEEPPWD_Pos)           /*!< 0x20000000 */
#define ADC_CR_DEEPPWD                 ADC_CR_DEEPPWD_Msk                      /*!< ADC deep power down enable */
#define ADC_CR_ADCALDIF_Pos            (30U)
#define ADC_CR_ADCALDIF_Msk            (0x1UL << ADC_CR_ADCALDIF_Pos)          /*!< 0x40000000 */
#define ADC_CR_ADCALDIF                ADC_CR_ADCALDIF_Msk                     /*!< ADC differential mode for calibration */
#define ADC_CR_ADCAL_Pos               (31U)
#define ADC_CR_ADCAL_Msk               (0x1UL << ADC_CR_ADCAL_Pos)             /*!< 0x80000000 */
#define ADC_CR_ADCAL                   ADC_CR_ADCAL_Msk                        /*!< ADC calibration */

/********************  Bit definition for ADC_CFGR register  ******************/
#define ADC_CFGR_DMAEN_Pos             (0U)
#define ADC_CFGR_DMAEN_Msk             (0x1UL << ADC_CFGR_DMAEN_Pos)           /*!< 0x00000001 */
#define ADC_CFGR_DMAEN                 ADC_CFGR_DMAEN_Msk                      /*!< ADC DMA transfer enable */
#define ADC_CFGR_DMACFG_Pos            (1U)
#define ADC_CFGR_DMACFG_Msk            (0x1UL << ADC_CFGR_DMACFG_Pos)          /*!< 0x00000002 */
#define ADC_CFGR_DMACFG                ADC_CFGR_DMACFG_Msk                     /*!< ADC DMA transfer configuration */

#define ADC_CFGR_RES_Pos               (3U)
#define ADC_CFGR_RES_Msk               (0x3UL << ADC_CFGR_RES_Pos)             /*!< 0x00000018 */
#define ADC_CFGR_RES                   ADC_CFGR_RES_Msk                        /*!< ADC data resolution */
#define ADC_CFGR_RES_0                 (0x1UL << ADC_CFGR_RES_Pos)             /*!< 0x00000008 */
#define ADC_CFGR_RES_1                 (0x2UL << ADC_CFGR_RES_Pos)             /*!< 0x00000010 */

#define ADC_CFGR_ALIGN_Pos             (5U)
#define ADC_CFGR_ALIGN_Msk             (0x1UL << ADC_CFGR_ALIGN_Pos)           /*!< 0x00000020 */
#define ADC_CFGR_ALIGN                 ADC_CFGR_ALIGN_Msk                      /*!< ADC data alignement */

#define ADC_CFGR_EXTSEL_Pos            (6U)
#define ADC_CFGR_EXTSEL_Msk            (0xFUL << ADC_CFGR_EXTSEL_Pos)          /*!< 0x000003C0 */
#define ADC_CFGR_EXTSEL                ADC_CFGR_EXTSEL_Msk                     /*!< ADC group regular external trigger source */
#define ADC_CFGR_EXTSEL_0              (0x1UL << ADC_CFGR_EXTSEL_Pos)          /*!< 0x00000040 */
#define ADC_CFGR_EXTSEL_1              (0x2UL << ADC_CFGR_EXTSEL_Pos)          /*!< 0x00000080 */
#define ADC_CFGR_EXTSEL_2              (0x4UL << ADC_CFGR_EXTSEL_Pos)          /*!< 0x00000100 */
#define ADC_CFGR_EXTSEL_3              (0x8UL << ADC_CFGR_EXTSEL_Pos)          /*!< 0x00000200 */

#define ADC_CFGR_EXTEN_Pos             (10U)
#define ADC_CFGR_EXTEN_Msk             (0x3UL << ADC_CFGR_EXTEN_Pos)           /*!< 0x00000C00 */
#define ADC_CFGR_EXTEN                 ADC_CFGR_EXTEN_Msk                      /*!< ADC group regular external trigger polarity */
#define ADC_CFGR_EXTEN_0               (0x1UL << ADC_CFGR_EXTEN_Pos)           /*!< 0x00000400 */
#define ADC_CFGR_EXTEN_1               (0x2UL << ADC_CFGR_EXTEN_Pos)           /*!< 0x00000800 */

#define ADC_CFGR_OVRMOD_Pos            (12U)
#define ADC_CFGR_OVRMOD_Msk            (0x1UL << ADC_CFGR_OVRMOD_Pos)          /*!< 0x00001000 */
#define ADC_CFGR_OVRMOD                ADC_CFGR_OVRMOD_Msk                     /*!< ADC group regular overrun configuration */
#define ADC_CFGR_CONT_Pos              (13U)
#define ADC_CFGR_CONT_Msk              (0x1UL << ADC_CFGR_CONT_Pos)            /*!< 0x00002000 */
#define ADC_CFGR_CONT                  ADC_CFGR_CONT_Msk                       /*!< ADC group regular continuous conversion mode */
#define ADC_CFGR_AUTDLY_Pos            (14U)
#define ADC_CFGR_AUTDLY_Msk            (0x1UL << ADC_CFGR_AUTDLY_Pos)          /*!< 0x00004000 */
#define ADC_CFGR_AUTDLY                ADC_CFGR_AUTDLY_Msk                     /*!< ADC low power auto wait */

#define ADC_CFGR_DISCEN_Pos            (16U)
#define ADC_CFGR_DISCEN_Msk            (0x1UL << ADC_CFGR_DISCEN_Pos)          /*!< 0x00010000 */
#define ADC_CFGR_DISCEN                ADC_CFGR_DISCEN_Msk                     /*!< ADC group regular sequencer discontinuous mode */

#define ADC_CFGR_DISCNUM_Pos           (17U)
#define ADC_CFGR_DISCNUM_Msk           (0x7UL << ADC_CFGR_DISCNUM_Pos)         /*!< 0x000E0000 */
#define ADC_CFGR_DISCNUM               ADC_CFGR_DISCNUM_Msk                    /*!< ADC group regular sequencer discontinuous number of ranks */
#define ADC_CFGR_DISCNUM_0             (0x1UL << ADC_CFGR_DISCNUM_Pos)         /*!< 0x00020000 */
#define ADC_CFGR_DISCNUM_1             (0x2UL << ADC_CFGR_DISCNUM_Pos)         /*!< 0x00040000 */
#define ADC_CFGR_DISCNUM_2             (0x4UL << ADC_CFGR_DISCNUM_Pos)         /*!< 0x00080000 */

#define ADC_CFGR_JDISCEN_Pos           (20U)
#define ADC_CFGR_JDISCEN_Msk           (0x1UL << ADC_CFGR_JDISCEN_Pos)         /*!< 0x00100000 */
#define ADC_CFGR_JDISCEN               ADC_CFGR_JDISCEN_Msk                    /*!< ADC group injected sequencer discontinuous mode */
#define ADC_CFGR_JQM_Pos               (21U)
#define ADC_CFGR_JQM_Msk               (0x1UL << ADC_CFGR_JQM_Pos)             /*!< 0x00200000 */
#define ADC_CFGR_JQM                   ADC_CFGR_JQM_Msk                        /*!< ADC group injected contexts queue mode */
#define ADC_CFGR_AWD1SGL_Pos           (22U)
#define ADC_CFGR_AWD1SGL_Msk           (0x1UL << ADC_CFGR_AWD1SGL_Pos)         /*!< 0x00400000 */
#define ADC_CFGR_AWD1SGL               ADC_CFGR_AWD1SGL_Msk                    /*!< ADC analog watchdog 1 monitoring a single channel or all channels */
#define ADC_CFGR_AWD1EN_Pos            (23U)
#define ADC_CFGR_AWD1EN_Msk            (0x1UL << ADC_CFGR_AWD1EN_Pos)          /*!< 0x00800000 */
#define ADC_CFGR_AWD1EN                ADC_CFGR_AWD1EN_Msk                     /*!< ADC analog watchdog 1 enable on scope ADC group regular */
#define ADC_CFGR_JAWD1EN_Pos           (24U)
#define ADC_CFGR_JAWD1EN_Msk           (0x1UL << ADC_CFGR_JAWD1EN_Pos)         /*!< 0x01000000 */
#define ADC_CFGR_JAWD1EN               ADC_CFGR_JAWD1EN_Msk                    /*!< ADC analog watchdog 1 enable on scope ADC group injected */
#define ADC_CFGR_JAUTO_Pos             (25U)
#define ADC_CFGR_JAUTO_Msk             (0x1UL << ADC_CFGR_JAUTO_Pos)           /*!< 0x02000000 */
#define ADC_CFGR_JAUTO                 ADC_CFGR_JAUTO_Msk                      /*!< ADC group injected automatic trigger mode */

#define ADC_CFGR_AWD1CH_Pos            (26U)
#define ADC_CFGR_AWD1CH_Msk            (0x1FUL << ADC_CFGR_AWD1CH_Pos)         /*!< 0x7C000000 */
#define ADC_CFGR_AWD1CH                ADC_CFGR_AWD1CH_Msk                     /*!< ADC analog watchdog 1 monitored channel selection */
#define ADC_CFGR_AWD1CH_0              (0x01UL << ADC_CFGR_AWD1CH_Pos)         /*!< 0x04000000 */
#define ADC_CFGR_AWD1CH_1              (0x02UL << ADC_CFGR_AWD1CH_Pos)         /*!< 0x08000000 */
#define ADC_CFGR_AWD1CH_2              (0x04UL << ADC_CFGR_AWD1CH_Pos)         /*!< 0x10000000 */
#define ADC_CFGR_AWD1CH_3              (0x08UL << ADC_CFGR_AWD1CH_Pos)         /*!< 0x20000000 */
#define ADC_CFGR_AWD1CH_4              (0x10UL << ADC_CFGR_AWD1CH_Pos)         /*!< 0x40000000 */

#define ADC_CFGR_JQDIS_Pos             (31U)
#define ADC_CFGR_JQDIS_Msk             (0x1UL << ADC_CFGR_JQDIS_Pos)           /*!< 0x80000000 */
#define ADC_CFGR_JQDIS                 ADC_CFGR_JQDIS_Msk                      /*!< ADC group injected contexts queue disable */

/********************  Bit definition for ADC_CFGR2 register  *****************/
#define ADC_CFGR2_ROVSE_Pos            (0U)
#define ADC_CFGR2_ROVSE_Msk            (0x1UL << ADC_CFGR2_ROVSE_Pos)          /*!< 0x00000001 */
#define ADC_CFGR2_ROVSE                ADC_CFGR2_ROVSE_Msk                     /*!< ADC oversampler enable on scope ADC group regular */
#define ADC_CFGR2_JOVSE_Pos            (1U)
#define ADC_CFGR2_JOVSE_Msk            (0x1UL << ADC_CFGR2_JOVSE_Pos)          /*!< 0x00000002 */
#define ADC_CFGR2_JOVSE                ADC_CFGR2_JOVSE_Msk                     /*!< ADC oversampler enable on scope ADC group injected */

#define ADC_CFGR2_OVSR_Pos             (2U)
#define ADC_CFGR2_OVSR_Msk             (0x7UL << ADC_CFGR2_OVSR_Pos)           /*!< 0x0000001C */
#define ADC_CFGR2_OVSR                 ADC_CFGR2_OVSR_Msk                      /*!< ADC oversampling ratio */
#define ADC_CFGR2_OVSR_0               (0x1UL << ADC_CFGR2_OVSR_Pos)           /*!< 0x00000004 */
#define ADC_CFGR2_OVSR_1               (0x2UL << ADC_CFGR2_OVSR_Pos)           /*!< 0x00000008 */
#define ADC_CFGR2_OVSR_2               (0x4UL << ADC_CFGR2_OVSR_Pos)           /*!< 0x00000010 */

#define ADC_CFGR2_OVSS_Pos             (5U)
#define ADC_CFGR2_OVSS_Msk             (0xFUL << ADC_CFGR2_OVSS_Pos)           /*!< 0x000001E0 */
#define ADC_CFGR2_OVSS                 ADC_CFGR2_OVSS_Msk                      /*!< ADC oversampling shift */
#define ADC_CFGR2_OVSS_0               (0x1UL << ADC_CFGR2_OVSS_Pos)           /*!< 0x00000020 */
#define ADC_CFGR2_OVSS_1               (0x2UL << ADC_CFGR2_OVSS_Pos)           /*!< 0x00000040 */
#define ADC_CFGR2_OVSS_2               (0x4UL << ADC_CFGR2_OVSS_Pos)           /*!< 0x00000080 */
#define ADC_CFGR2_OVSS_3               (0x8UL << ADC_CFGR2_OVSS_Pos)           /*!< 0x00000100 */

#define ADC_CFGR2_TROVS_Pos            (9U)
#define ADC_CFGR2_TROVS_Msk            (0x1UL << ADC_CFGR2_TROVS_Pos)          /*!< 0x00000200 */
#define ADC_CFGR2_TROVS                ADC_CFGR2_TROVS_Msk                     /*!< ADC oversampling discontinuous mode (triggered mode) for ADC group regular */
#define ADC_CFGR2_ROVSM_Pos            (10U)
#define ADC_CFGR2_ROVSM_Msk            (0x1UL << ADC_CFGR2_ROVSM_Pos)          /*!< 0x00000400 */
#define ADC_CFGR2_ROVSM                ADC_CFGR2_ROVSM_Msk                     /*!< ADC oversampling mode managing interlaced conversions of ADC group regular and group injected */

/********************  Bit definition for ADC_SMPR1 register  *****************/
#define ADC_SMPR1_SMP0_Pos             (0U)
#define ADC_SMPR1_SMP0_Msk             (0x7UL << ADC_SMPR1_SMP0_Pos)           /*!< 0x00000007 */
#define ADC_SMPR1_SMP0                 ADC_SMPR1_SMP0_Msk                      /*!< ADC channel 0 sampling time selection  */
#define ADC_SMPR1_SMP0_0               (0x1UL << ADC_SMPR1_SMP0_Pos)           /*!< 0x00000001 */
#define ADC_SMPR1_SMP0_1               (0x2UL << ADC_SMPR1_SMP0_Pos)           /*!< 0x00000002 */
#define ADC_SMPR1_SMP0_2               (0x4UL << ADC_SMPR1_SMP0_Pos)           /*!< 0x00000004 */

#define ADC_SMPR1_SMP1_Pos             (3U)
#define ADC_SMPR1_SMP1_Msk             (0x7UL << ADC_SMPR1_SMP1_Pos)           /*!< 0x00000038 */
#define ADC_SMPR1_SMP1                 ADC_SMPR1_SMP1_Msk                      /*!< ADC channel 1 sampling time selection  */
#define ADC_SMPR1_SMP1_0               (0x1UL << ADC_SMPR1_SMP1_Pos)           /*!< 0x00000008 */
#define ADC_SMPR1_SMP1_1               (0x2UL << ADC_SMPR1_SMP1_Pos)           /*!< 0x00000010 */
#define ADC_SMPR1_SMP1_2               (0x4UL << ADC_SMPR1_SMP1_Pos)           /*!< 0x00000020 */

#define ADC_SMPR1_SMP2_Pos             (6U)
#define ADC_SMPR1_SMP2_Msk             (0x7UL << ADC_SMPR1_SMP2_Pos)           /*!< 0x000001C0 */
#define ADC_SMPR1_SMP2                 ADC_SMPR1_SMP2_Msk                      /*!< ADC channel 2 sampling time selection  */
#define ADC_SMPR1_SMP2_0               (0x1UL << ADC_SMPR1_SMP2_Pos)           /*!< 0x00000040 */
#define ADC_SMPR1_SMP2_1               (0x2UL << ADC_SMPR1_SMP2_Pos)           /*!< 0x00000080 */
#define ADC_SMPR1_SMP2_2               (0x4UL << ADC_SMPR1_SMP2_Pos)           /*!< 0x00000100 */

#define ADC_SMPR1_SMP3_Pos             (9U)
#define ADC_SMPR1_SMP3_Msk             (0x7UL << ADC_SMPR1_SMP3_Pos)           /*!< 0x00000E00 */
#define ADC_SMPR1_SMP3                 ADC_SMPR1_SMP3_Msk                      /*!< ADC channel 3 sampling time selection  */
#define ADC_SMPR1_SMP3_0               (0x1UL << ADC_SMPR1_SMP3_Pos)           /*!< 0x00000200 */
#define ADC_SMPR1_SMP3_1               (0x2UL << ADC_SMPR1_SMP3_Pos)           /*!< 0x00000400 */
#define ADC_SMPR1_SMP3_2               (0x4UL << ADC_SMPR1_SMP3_Pos)           /*!< 0x00000800 */

#define ADC_SMPR1_SMP4_Pos             (12U)
#define ADC_SMPR1_SMP4_Msk             (0x7UL << ADC_SMPR1_SMP4_Pos)           /*!< 0x00007000 */
#define ADC_SMPR1_SMP4                 ADC_SMPR1_SMP4_Msk                      /*!< ADC channel 4 sampling time selection  */
#define ADC_SMPR1_SMP4_0               (0x1UL << ADC_SMPR1_SMP4_Pos)           /*!< 0x00001000 */
#define ADC_SMPR1_SMP4_1               (0x2UL << ADC_SMPR1_SMP4_Pos)           /*!< 0x00002000 */
#define ADC_SMPR1_SMP4_2               (0x4UL << ADC_SMPR1_SMP4_Pos)           /*!< 0x00004000 */

#define ADC_SMPR1_SMP5_Pos             (15U)
#define ADC_SMPR1_SMP5_Msk             (0x7UL << ADC_SMPR1_SMP5_Pos)           /*!< 0x00038000 */
#define ADC_SMPR1_SMP5                 ADC_SMPR1_SMP5_Msk                      /*!< ADC channel 5 sampling time selection  */
#define ADC_SMPR1_SMP5_0               (0x1UL << ADC_SMPR1_SMP5_Pos)           /*!< 0x00008000 */
#define ADC_SMPR1_SMP5_1               (0x2UL << ADC_SMPR1_SMP5_Pos)           /*!< 0x00010000 */
#define ADC_SMPR1_SMP5_2               (0x4UL << ADC_SMPR1_SMP5_Pos)           /*!< 0x00020000 */

#define ADC_SMPR1_SMP6_Pos             (18U)
#define ADC_SMPR1_SMP6_Msk             (0x7UL << ADC_SMPR1_SMP6_Pos)           /*!< 0x001C0000 */
#define ADC_SMPR1_SMP6                 ADC_SMPR1_SMP6_Msk                      /*!< ADC channel 6 sampling time selection  */
#define ADC_SMPR1_SMP6_0               (0x1UL << ADC_SMPR1_SMP6_Pos)           /*!< 0x00040000 */
#define ADC_SMPR1_SMP6_1               (0x2UL << ADC_SMPR1_SMP6_Pos)           /*!< 0x00080000 */
#define ADC_SMPR1_SMP6_2               (0x4UL << ADC_SMPR1_SMP6_Pos)           /*!< 0x00100000 */

#define ADC_SMPR1_SMP7_Pos             (21U)
#define ADC_SMPR1_SMP7_Msk             (0x7UL << ADC_SMPR1_SMP7_Pos)           /*!< 0x00E00000 */
#define ADC_SMPR1_SMP7                 ADC_SMPR1_SMP7_Msk                      /*!< ADC channel 7 sampling time selection  */
#define ADC_SMPR1_SMP7_0               (0x1UL << ADC_SMPR1_SMP7_Pos)           /*!< 0x00200000 */
#define ADC_SMPR1_SMP7_1               (0x2UL << ADC_SMPR1_SMP7_Pos)           /*!< 0x00400000 */
#define ADC_SMPR1_SMP7_2               (0x4UL << ADC_SMPR1_SMP7_Pos)           /*!< 0x00800000 */

#define ADC_SMPR1_SMP8_Pos             (24U)
#define ADC_SMPR1_SMP8_Msk             (0x7UL << ADC_SMPR1_SMP8_Pos)           /*!< 0x07000000 */
#define ADC_SMPR1_SMP8                 ADC_SMPR1_SMP8_Msk                      /*!< ADC channel 8 sampling time selection  */
#define ADC_SMPR1_SMP8_0               (0x1UL << ADC_SMPR1_SMP8_Pos)           /*!< 0x01000000 */
#define ADC_SMPR1_SMP8_1               (0x2UL << ADC_SMPR1_SMP8_Pos)           /*!< 0x02000000 */
#define ADC_SMPR1_SMP8_2               (0x4UL << ADC_SMPR1_SMP8_Pos)           /*!< 0x04000000 */

#define ADC_SMPR1_SMP9_Pos             (27U)
#define ADC_SMPR1_SMP9_Msk             (0x7UL << ADC_SMPR1_SMP9_Pos)           /*!< 0x38000000 */
#define ADC_SMPR1_SMP9                 ADC_SMPR1_SMP9_Msk                      /*!< ADC channel 9 sampling time selection  */
#define ADC_SMPR1_SMP9_0               (0x1UL << ADC_SMPR1_SMP9_Pos)           /*!< 0x08000000 */
#define ADC_SMPR1_SMP9_1               (0x2UL << ADC_SMPR1_SMP9_Pos)           /*!< 0x10000000 */
#define ADC_SMPR1_SMP9_2               (0x4UL << ADC_SMPR1_SMP9_Pos)           /*!< 0x20000000 */

#define ADC_SMPR1_SMPPLUS_Pos          (31U)
#define ADC_SMPR1_SMPPLUS_Msk          (0x1UL << ADC_SMPR1_SMPPLUS_Pos)        /*!< 0x80000000 */
#define ADC_SMPR1_SMPPLUS              ADC_SMPR1_SMPPLUS_Msk                   /*!< ADC channels sampling time additional setting */

/********************  Bit definition for ADC_SMPR2 register  *****************/
#define ADC_SMPR2_SMP10_Pos            (0U)
#define ADC_SMPR2_SMP10_Msk            (0x7UL << ADC_SMPR2_SMP10_Pos)          /*!< 0x00000007 */
#define ADC_SMPR2_SMP10                ADC_SMPR2_SMP10_Msk                     /*!< ADC channel 10 sampling time selection  */
#define ADC_SMPR2_SMP10_0              (0x1UL << ADC_SMPR2_SMP10_Pos)          /*!< 0x00000001 */
#define ADC_SMPR2_SMP10_1              (0x2UL << ADC_SMPR2_SMP10_Pos)          /*!< 0x00000002 */
#define ADC_SMPR2_SMP10_2              (0x4UL << ADC_SMPR2_SMP10_Pos)          /*!< 0x00000004 */

#define ADC_SMPR2_SMP11_Pos            (3U)
#define ADC_SMPR2_SMP11_Msk            (0x7UL << ADC_SMPR2_SMP11_Pos)          /*!< 0x00000038 */
#define ADC_SMPR2_SMP11                ADC_SMPR2_SMP11_Msk                     /*!< ADC channel 11 sampling time selection  */
#define ADC_SMPR2_SMP11_0              (0x1UL << ADC_SMPR2_SMP11_Pos)          /*!< 0x00000008 */
#define ADC_SMPR2_SMP11_1              (0x2UL << ADC_SMPR2_SMP11_Pos)          /*!< 0x00000010 */
#define ADC_SMPR2_SMP11_2              (0x4UL << ADC_SMPR2_SMP11_Pos)          /*!< 0x00000020 */

#define ADC_SMPR2_SMP12_Pos            (6U)
#define ADC_SMPR2_SMP12_Msk            (0x7UL << ADC_SMPR2_SMP12_Pos)          /*!< 0x000001C0 */
#define ADC_SMPR2_SMP12                ADC_SMPR2_SMP12_Msk                     /*!< ADC channel 12 sampling time selection  */
#define ADC_SMPR2_SMP12_0              (0x1UL << ADC_SMPR2_SMP12_Pos)          /*!< 0x00000040 */
#define ADC_SMPR2_SMP12_1              (0x2UL << ADC_SMPR2_SMP12_Pos)          /*!< 0x00000080 */
#define ADC_SMPR2_SMP12_2              (0x4UL << ADC_SMPR2_SMP12_Pos)          /*!< 0x00000100 */

#define ADC_SMPR2_SMP13_Pos            (9U)
#define ADC_SMPR2_SMP13_Msk            (0x7UL << ADC_SMPR2_SMP13_Pos)          /*!< 0x00000E00 */
#define ADC_SMPR2_SMP13                ADC_SMPR2_SMP13_Msk                     /*!< ADC channel 13 sampling time selection  */
#define ADC_SMPR2_SMP13_0              (0x1UL << ADC_SMPR2_SMP13_Pos)          /*!< 0x00000200 */
#define ADC_SMPR2_SMP13_1              (0x2UL << ADC_SMPR2_SMP13_Pos)          /*!< 0x00000400 */
#define ADC_SMPR2_SMP13_2              (0x4UL << ADC_SMPR2_SMP13_Pos)          /*!< 0x00000800 */

#define ADC_SMPR2_SMP14_Pos            (12U)
#define ADC_SMPR2_SMP14_Msk            (0x7UL << ADC_SMPR2_SMP14_Pos)          /*!< 0x00007000 */
#define ADC_SMPR2_SMP14                ADC_SMPR2_SMP14_Msk                     /*!< ADC channel 14 sampling time selection  */
#define ADC_SMPR2_SMP14_0              (0x1UL << ADC_SMPR2_SMP14_Pos)          /*!< 0x00001000 */
#define ADC_SMPR2_SMP14_1              (0x2UL << ADC_SMPR2_SMP14_Pos)          /*!< 0x00002000 */
#define ADC_SMPR2_SMP14_2              (0x4UL << ADC_SMPR2_SMP14_Pos)          /*!< 0x00004000 */

#define ADC_SMPR2_SMP15_Pos            (15U)
#define ADC_SMPR2_SMP15_Msk            (0x7UL << ADC_SMPR2_SMP15_Pos)          /*!< 0x00038000 */
#define ADC_SMPR2_SMP15                ADC_SMPR2_SMP15_Msk                     /*!< ADC channel 15 sampling time selection  */
#define ADC_SMPR2_SMP15_0              (0x1UL << ADC_SMPR2_SMP15_Pos)          /*!< 0x00008000 */
#define ADC_SMPR2_SMP15_1              (0x2UL << ADC_SMPR2_SMP15_Pos)          /*!< 0x00010000 */
#define ADC_SMPR2_SMP15_2              (0x4UL << ADC_SMPR2_SMP15_Pos)          /*!< 0x00020000 */

#define ADC_SMPR2_SMP16_Pos            (18U)
#define ADC_SMPR2_SMP16_Msk            (0x7UL << ADC_SMPR2_SMP16_Pos)          /*!< 0x001C0000 */
#define ADC_SMPR2_SMP16                ADC_SMPR2_SMP16_Msk                     /*!< ADC channel 16 sampling time selection  */
#define ADC_SMPR2_SMP16_0              (0x1UL << ADC_SMPR2_SMP16_Pos)          /*!< 0x00040000 */
#define ADC_SMPR2_SMP16_1              (0x2UL << ADC_SMPR2_SMP16_Pos)          /*!< 0x00080000 */
#define ADC_SMPR2_SMP16_2              (0x4UL << ADC_SMPR2_SMP16_Pos)          /*!< 0x00100000 */

#define ADC_SMPR2_SMP17_Pos            (21U)
#define ADC_SMPR2_SMP17_Msk            (0x7UL << ADC_SMPR2_SMP17_Pos)          /*!< 0x00E00000 */
#define ADC_SMPR2_SMP17                ADC_SMPR2_SMP17_Msk                     /*!< ADC channel 17 sampling time selection  */
#define ADC_SMPR2_SMP17_0              (0x1UL << ADC_SMPR2_SMP17_Pos)          /*!< 0x00200000 */
#define ADC_SMPR2_SMP17_1              (0x2UL << ADC_SMPR2_SMP17_Pos)          /*!< 0x00400000 */
#define ADC_SMPR2_SMP17_2              (0x4UL << ADC_SMPR2_SMP17_Pos)          /*!< 0x00800000 */

#define ADC_SMPR2_SMP18_Pos            (24U)
#define ADC_SMPR2_SMP18_Msk            (0x7UL << ADC_SMPR2_SMP18_Pos)          /*!< 0x07000000 */
#define ADC_SMPR2_SMP18                ADC_SMPR2_SMP18_Msk                     /*!< ADC channel 18 sampling time selection  */
#define ADC_SMPR2_SMP18_0              (0x1UL << ADC_SMPR2_SMP18_Pos)          /*!< 0x01000000 */
#define ADC_SMPR2_SMP18_1              (0x2UL << ADC_SMPR2_SMP18_Pos)          /*!< 0x02000000 */
#define ADC_SMPR2_SMP18_2              (0x4UL << ADC_SMPR2_SMP18_Pos)          /*!< 0x04000000 */

/********************  Bit definition for ADC_TR1 register  *******************/
#define ADC_TR1_LT1_Pos                (0U)
#define ADC_TR1_LT1_Msk                (0xFFFUL << ADC_TR1_LT1_Pos)            /*!< 0x00000FFF */
#define ADC_TR1_LT1                    ADC_TR1_LT1_Msk                         /*!< ADC analog watchdog 1 threshold low */
#define ADC_TR1_LT1_0                  (0x001UL << ADC_TR1_LT1_Pos)            /*!< 0x00000001 */
#define ADC_TR1_LT1_1                  (0x002UL << ADC_TR1_LT1_Pos)            /*!< 0x00000002 */
#define ADC_TR1_LT1_2                  (0x004UL << ADC_TR1_LT1_Pos)            /*!< 0x00000004 */
#define ADC_TR1_LT1_3                  (0x008UL << ADC_TR1_LT1_Pos)            /*!< 0x00000008 */
#define ADC_TR1_LT1_4                  (0x010UL << ADC_TR1_LT1_Pos)            /*!< 0x00000010 */
#define ADC_TR1_LT1_5                  (0x020UL << ADC_TR1_LT1_Pos)            /*!< 0x00000020 */
#define ADC_TR1_LT1_6                  (0x040UL << ADC_TR1_LT1_Pos)            /*!< 0x00000040 */
#define ADC_TR1_LT1_7                  (0x080UL << ADC_TR1_LT1_Pos)            /*!< 0x00000080 */
#define ADC_TR1_LT1_8                  (0x100UL << ADC_TR1_LT1_Pos)            /*!< 0x00000100 */
#define ADC_TR1_LT1_9                  (0x200UL << ADC_TR1_LT1_Pos)            /*!< 0x00000200 */
#define ADC_TR1_LT1_10                 (0x400UL << ADC_TR1_LT1_Pos)            /*!< 0x00000400 */
#define ADC_TR1_LT1_11                 (0x800UL << ADC_TR1_LT1_Pos)            /*!< 0x00000800 */

#define ADC_TR1_HT1_Pos                (16U)
#define ADC_TR1_HT1_Msk                (0xFFFUL << ADC_TR1_HT1_Pos)            /*!< 0x0FFF0000 */
#define ADC_TR1_HT1                    ADC_TR1_HT1_Msk                         /*!< ADC Analog watchdog 1 threshold high */
#define ADC_TR1_HT1_0                  (0x001UL << ADC_TR1_HT1_Pos)            /*!< 0x00010000 */
#define ADC_TR1_HT1_1                  (0x002UL << ADC_TR1_HT1_Pos)            /*!< 0x00020000 */
#define ADC_TR1_HT1_2                  (0x004UL << ADC_TR1_HT1_Pos)            /*!< 0x00040000 */
#define ADC_TR1_HT1_3                  (0x008UL << ADC_TR1_HT1_Pos)            /*!< 0x00080000 */
#define ADC_TR1_HT1_4                  (0x010UL << ADC_TR1_HT1_Pos)            /*!< 0x00100000 */
#define ADC_TR1_HT1_5                  (0x020UL << ADC_TR1_HT1_Pos)            /*!< 0x00200000 */
#define ADC_TR1_HT1_6                  (0x040UL << ADC_TR1_HT1_Pos)            /*!< 0x00400000 */
#define ADC_TR1_HT1_7                  (0x080UL << ADC_TR1_HT1_Pos)            /*!< 0x00800000 */
#define ADC_TR1_HT1_8                  (0x100UL << ADC_TR1_HT1_Pos)            /*!< 0x01000000 */
#define ADC_TR1_HT1_9                  (0x200UL << ADC_TR1_HT1_Pos)            /*!< 0x02000000 */
#define ADC_TR1_HT1_10                 (0x400UL << ADC_TR1_HT1_Pos)            /*!< 0x04000000 */
#define ADC_TR1_HT1_11                 (0x800UL << ADC_TR1_HT1_Pos)            /*!< 0x08000000 */

/********************  Bit definition for ADC_TR2 register  *******************/
#define ADC_TR2_LT2_Pos                (0U)
#define ADC_TR2_LT2_Msk                (0xFFUL << ADC_TR2_LT2_Pos)             /*!< 0x000000FF */
#define ADC_TR2_LT2                    ADC_TR2_LT2_Msk                         /*!< ADC analog watchdog 2 threshold low */
#define ADC_TR2_LT2_0                  (0x01UL << ADC_TR2_LT2_Pos)             /*!< 0x00000001 */
#define ADC_TR2_LT2_1                  (0x02UL << ADC_TR2_LT2_Pos)             /*!< 0x00000002 */
#define ADC_TR2_LT2_2                  (0x04UL << ADC_TR2_LT2_Pos)             /*!< 0x00000004 */
#define ADC_TR2_LT2_3                  (0x08UL << ADC_TR2_LT2_Pos)             /*!< 0x00000008 */
#define ADC_TR2_LT2_4                  (0x10UL << ADC_TR2_LT2_Pos)             /*!< 0x00000010 */
#define ADC_TR2_LT2_5                  (0x20UL << ADC_TR2_LT2_Pos)             /*!< 0x00000020 */
#define ADC_TR2_LT2_6                  (0x40UL << ADC_TR2_LT2_Pos)             /*!< 0x00000040 */
#define ADC_TR2_LT2_7                  (0x80UL << ADC_TR2_LT2_Pos)             /*!< 0x00000080 */

#define ADC_TR2_HT2_Pos                (16U)
#define ADC_TR2_HT2_Msk                (0xFFUL << ADC_TR2_HT2_Pos)             /*!< 0x00FF0000 */
#define ADC_TR2_HT2                    ADC_TR2_HT2_Msk                         /*!< ADC analog watchdog 2 threshold high */
#define ADC_TR2_HT2_0                  (0x01UL << ADC_TR2_HT2_Pos)             /*!< 0x00010000 */
#define ADC_TR2_HT2_1                  (0x02UL << ADC_TR2_HT2_Pos)             /*!< 0x00020000 */
#define ADC_TR2_HT2_2                  (0x04UL << ADC_TR2_HT2_Pos)             /*!< 0x00040000 */
#define ADC_TR2_HT2_3                  (0x08UL << ADC_TR2_HT2_Pos)             /*!< 0x00080000 */
#define ADC_TR2_HT2_4                  (0x10UL << ADC_TR2_HT2_Pos)             /*!< 0x00100000 */
#define ADC_TR2_HT2_5                  (0x20UL << ADC_TR2_HT2_Pos)             /*!< 0x00200000 */
#define ADC_TR2_HT2_6                  (0x40UL << ADC_TR2_HT2_Pos)             /*!< 0x00400000 */
#define ADC_TR2_HT2_7                  (0x80UL << ADC_TR2_HT2_Pos)             /*!< 0x00800000 */

/********************  Bit definition for ADC_TR3 register  *******************/
#define ADC_TR3_LT3_Pos                (0U)
#define ADC_TR3_LT3_Msk                (0xFFUL << ADC_TR3_LT3_Pos)             /*!< 0x000000FF */
#define ADC_TR3_LT3                    ADC_TR3_LT3_Msk                         /*!< ADC analog watchdog 3 threshold low */
#define ADC_TR3_LT3_0                  (0x01UL << ADC_TR3_LT3_Pos)             /*!< 0x00000001 */
#define ADC_TR3_LT3_1                  (0x02UL << ADC_TR3_LT3_Pos)             /*!< 0x00000002 */
#define ADC_TR3_LT3_2                  (0x04UL << ADC_TR3_LT3_Pos)             /*!< 0x00000004 */
#define ADC_TR3_LT3_3                  (0x08UL << ADC_TR3_LT3_Pos)             /*!< 0x00000008 */
#define ADC_TR3_LT3_4                  (0x10UL << ADC_TR3_LT3_Pos)             /*!< 0x00000010 */
#define ADC_TR3_LT3_5                  (0x20UL << ADC_TR3_LT3_Pos)             /*!< 0x00000020 */
#define ADC_TR3_LT3_6                  (0x40UL << ADC_TR3_LT3_Pos)             /*!< 0x00000040 */
#define ADC_TR3_LT3_7                  (0x80UL << ADC_TR3_LT3_Pos)             /*!< 0x00000080 */

#define ADC_TR3_HT3_Pos                (16U)
#define ADC_TR3_HT3_Msk                (0xFFUL << ADC_TR3_HT3_Pos)             /*!< 0x00FF0000 */
#define ADC_TR3_HT3                    ADC_TR3_HT3_Msk                         /*!< ADC analog watchdog 3 threshold high */
#define ADC_TR3_HT3_0                  (0x01UL << ADC_TR3_HT3_Pos)             /*!< 0x00010000 */
#define ADC_TR3_HT3_1                  (0x02UL << ADC_TR3_HT3_Pos)             /*!< 0x00020000 */
#define ADC_TR3_HT3_2                  (0x04UL << ADC_TR3_HT3_Pos)             /*!< 0x00040000 */
#define ADC_TR3_HT3_3                  (0x08UL << ADC_TR3_HT3_Pos)             /*!< 0x00080000 */
#define ADC_TR3_HT3_4                  (0x10UL << ADC_TR3_HT3_Pos)             /*!< 0x00100000 */
#define ADC_TR3_HT3_5                  (0x20UL << ADC_TR3_HT3_Pos)             /*!< 0x00200000 */
#define ADC_TR3_HT3_6                  (0x40UL << ADC_TR3_HT3_Pos)             /*!< 0x00400000 */
#define ADC_TR3_HT3_7                  (0x80UL << ADC_TR3_HT3_Pos)             /*!< 0x00800000 */

/********************  Bit definition for ADC_SQR1 register  ******************/
#define ADC_SQR1_L_Pos                 (0U)
#define ADC_SQR1_L_Msk                 (0xFUL << ADC_SQR1_L_Pos)               /*!< 0x0000000F */
#define ADC_SQR1_L                     ADC_SQR1_L_Msk                          /*!< ADC group regular sequencer scan length */
#define ADC_SQR1_L_0                   (0x1UL << ADC_SQR1_L_Pos)               /*!< 0x00000001 */
#define ADC_SQR1_L_1                   (0x2UL << ADC_SQR1_L_Pos)               /*!< 0x00000002 */
#define ADC_SQR1_L_2                   (0x4UL << ADC_SQR1_L_Pos)               /*!< 0x00000004 */
#define ADC_SQR1_L_3                   (0x8UL << ADC_SQR1_L_Pos)               /*!< 0x00000008 */

#define ADC_SQR1_SQ1_Pos               (6U)
#define ADC_SQR1_SQ1_Msk               (0x1FUL << ADC_SQR1_SQ1_Pos)            /*!< 0x000007C0 */
#define ADC_SQR1_SQ1                   ADC_SQR1_SQ1_Msk                        /*!< ADC group regular sequencer rank 1 */
#define ADC_SQR1_SQ1_0                 (0x01UL << ADC_SQR1_SQ1_Pos)            /*!< 0x00000040 */
#define ADC_SQR1_SQ1_1                 (0x02UL << ADC_SQR1_SQ1_Pos)            /*!< 0x00000080 */
#define ADC_SQR1_SQ1_2                 (0x04UL << ADC_SQR1_SQ1_Pos)            /*!< 0x00000100 */
#define ADC_SQR1_SQ1_3                 (0x08UL << ADC_SQR1_SQ1_Pos)            /*!< 0x00000200 */
#define ADC_SQR1_SQ1_4                 (0x10UL << ADC_SQR1_SQ1_Pos)            /*!< 0x00000400 */

#define ADC_SQR1_SQ2_Pos               (12U)
#define ADC_SQR1_SQ2_Msk               (0x1FUL << ADC_SQR1_SQ2_Pos)            /*!< 0x0001F000 */
#define ADC_SQR1_SQ2                   ADC_SQR1_SQ2_Msk                        /*!< ADC group regular sequencer rank 2 */
#define ADC_SQR1_SQ2_0                 (0x01UL << ADC_SQR1_SQ2_Pos)            /*!< 0x00001000 */
#define ADC_SQR1_SQ2_1                 (0x02UL << ADC_SQR1_SQ2_Pos)            /*!< 0x00002000 */
#define ADC_SQR1_SQ2_2                 (0x04UL << ADC_SQR1_SQ2_Pos)            /*!< 0x00004000 */
#define ADC_SQR1_SQ2_3                 (0x08UL << ADC_SQR1_SQ2_Pos)            /*!< 0x00008000 */
#define ADC_SQR1_SQ2_4                 (0x10UL << ADC_SQR1_SQ2_Pos)            /*!< 0x00010000 */

#define ADC_SQR1_SQ3_Pos               (18U)
#define ADC_SQR1_SQ3_Msk               (0x1FUL << ADC_SQR1_SQ3_Pos)            /*!< 0x007C0000 */
#define ADC_SQR1_SQ3                   ADC_SQR1_SQ3_Msk                        /*!< ADC group regular sequencer rank 3 */
#define ADC_SQR1_SQ3_0                 (0x01UL << ADC_SQR1_SQ3_Pos)            /*!< 0x00040000 */
#define ADC_SQR1_SQ3_1                 (0x02UL << ADC_SQR1_SQ3_Pos)            /*!< 0x00080000 */
#define ADC_SQR1_SQ3_2                 (0x04UL << ADC_SQR1_SQ3_Pos)            /*!< 0x00100000 */
#define ADC_SQR1_SQ3_3                 (0x08UL << ADC_SQR1_SQ3_Pos)            /*!< 0x00200000 */
#define ADC_SQR1_SQ3_4                 (0x10UL << ADC_SQR1_SQ3_Pos)            /*!< 0x00400000 */

#define ADC_SQR1_SQ4_Pos               (24U)
#define ADC_SQR1_SQ4_Msk               (0x1FUL << ADC_SQR1_SQ4_Pos)            /*!< 0x1F000000 */
#define ADC_SQR1_SQ4                   ADC_SQR1_SQ4_Msk                        /*!< ADC group regular sequencer rank 4 */
#define ADC_SQR1_SQ4_0                 (0x01UL << ADC_SQR1_SQ4_Pos)            /*!< 0x01000000 */
#define ADC_SQR1_SQ4_1                 (0x02UL << ADC_SQR1_SQ4_Pos)            /*!< 0x02000000 */
#define ADC_SQR1_SQ4_2                 (0x04UL << ADC_SQR1_SQ4_Pos)            /*!< 0x04000000 */
#define ADC_SQR1_SQ4_3                 (0x08UL << ADC_SQR1_SQ4_Pos)            /*!< 0x08000000 */
#define ADC_SQR1_SQ4_4                 (0x10UL << ADC_SQR1_SQ4_Pos)            /*!< 0x10000000 */

/********************  Bit definition for ADC_SQR2 register  ******************/
#define ADC_SQR2_SQ5_Pos               (0U)
#define ADC_SQR2_SQ5_Msk               (0x1FUL << ADC_SQR2_SQ5_Pos)            /*!< 0x0000001F */
#define ADC_SQR2_SQ5                   ADC_SQR2_SQ5_Msk                        /*!< ADC group regular sequencer rank 5 */
#define ADC_SQR2_SQ5_0                 (0x01UL << ADC_SQR2_SQ5_Pos)            /*!< 0x00000001 */
#define ADC_SQR2_SQ5_1                 (0x02UL << ADC_SQR2_SQ5_Pos)            /*!< 0x00000002 */
#define ADC_SQR2_SQ5_2                 (0x04UL << ADC_SQR2_SQ5_Pos)            /*!< 0x00000004 */
#define ADC_SQR2_SQ5_3                 (0x08UL << ADC_SQR2_SQ5_Pos)            /*!< 0x00000008 */
#define ADC_SQR2_SQ5_4                 (0x10UL << ADC_SQR2_SQ5_Pos)            /*!< 0x00000010 */

#define ADC_SQR2_SQ6_Pos               (6U)
#define ADC_SQR2_SQ6_Msk               (0x1FUL << ADC_SQR2_SQ6_Pos)            /*!< 0x000007C0 */
#define ADC_SQR2_SQ6                   ADC_SQR2_SQ6_Msk                        /*!< ADC group regular sequencer rank 6 */
#define ADC_SQR2_SQ6_0                 (0x01UL << ADC_SQR2_SQ6_Pos)            /*!< 0x00000040 */
#define ADC_SQR2_SQ6_1                 (0x02UL << ADC_SQR2_SQ6_Pos)            /*!< 0x00000080 */
#define ADC_SQR2_SQ6_2                 (0x04UL << ADC_SQR2_SQ6_Pos)            /*!< 0x00000100 */
#define ADC_SQR2_SQ6_3                 (0x08UL << ADC_SQR2_SQ6_Pos)            /*!< 0x00000200 */
#define ADC_SQR2_SQ6_4                 (0x10UL << ADC_SQR2_SQ6_Pos)            /*!< 0x00000400 */

#define ADC_SQR2_SQ7_Pos               (12U)
#define ADC_SQR2_SQ7_Msk               (0x1FUL << ADC_SQR2_SQ7_Pos)            /*!< 0x0001F000 */
#define ADC_SQR2_SQ7                   ADC_SQR2_SQ7_Msk                        /*!< ADC group regular sequencer rank 7 */
#define ADC_SQR2_SQ7_0                 (0x01UL << ADC_SQR2_SQ7_Pos)            /*!< 0x00001000 */
#define ADC_SQR2_SQ7_1                 (0x02UL << ADC_SQR2_SQ7_Pos)            /*!< 0x00002000 */
#define ADC_SQR2_SQ7_2                 (0x04UL << ADC_SQR2_SQ7_Pos)            /*!< 0x00004000 */
#define ADC_SQR2_SQ7_3                 (0x08UL << ADC_SQR2_SQ7_Pos)            /*!< 0x00008000 */
#define ADC_SQR2_SQ7_4                 (0x10UL << ADC_SQR2_SQ7_Pos)            /*!< 0x00010000 */

#define ADC_SQR2_SQ8_Pos               (18U)
#define ADC_SQR2_SQ8_Msk               (0x1FUL << ADC_SQR2_SQ8_Pos)            /*!< 0x007C0000 */
#define ADC_SQR2_SQ8                   ADC_SQR2_SQ8_Msk                        /*!< ADC group regular sequencer rank 8 */
#define ADC_SQR2_SQ8_0                 (0x01UL << ADC_SQR2_SQ8_Pos)            /*!< 0x00040000 */
#define ADC_SQR2_SQ8_1                 (0x02UL << ADC_SQR2_SQ8_Pos)            /*!< 0x00080000 */
#define ADC_SQR2_SQ8_2                 (0x04UL << ADC_SQR2_SQ8_Pos)            /*!< 0x00100000 */
#define ADC_SQR2_SQ8_3                 (0x08UL << ADC_SQR2_SQ8_Pos)            /*!< 0x00200000 */
#define ADC_SQR2_SQ8_4                 (0x10UL << ADC_SQR2_SQ8_Pos)            /*!< 0x00400000 */

#define ADC_SQR2_SQ9_Pos               (24U)
#define ADC_SQR2_SQ9_Msk               (0x1FUL << ADC_SQR2_SQ9_Pos)            /*!< 0x1F000000 */
#define ADC_SQR2_SQ9                   ADC_SQR2_SQ9_Msk                        /*!< ADC group regular sequencer rank 9 */
#define ADC_SQR2_SQ9_0                 (0x01UL << ADC_SQR2_SQ9_Pos)            /*!< 0x01000000 */
#define ADC_SQR2_SQ9_1                 (0x02UL << ADC_SQR2_SQ9_Pos)            /*!< 0x02000000 */
#define ADC_SQR2_SQ9_2                 (0x04UL << ADC_SQR2_SQ9_Pos)            /*!< 0x04000000 */
#define ADC_SQR2_SQ9_3                 (0x08UL << ADC_SQR2_SQ9_Pos)            /*!< 0x08000000 */
#define ADC_SQR2_SQ9_4                 (0x10UL << ADC_SQR2_SQ9_Pos)            /*!< 0x10000000 */

/********************  Bit definition for ADC_SQR3 register  ******************/
#define ADC_SQR3_SQ10_Pos              (0U)
#define ADC_SQR3_SQ10_Msk              (0x1FUL << ADC_SQR3_SQ10_Pos)           /*!< 0x0000001F */
#define ADC_SQR3_SQ10                  ADC_SQR3_SQ10_Msk                       /*!< ADC group regular sequencer rank 10 */
#define ADC_SQR3_SQ10_0                (0x01UL << ADC_SQR3_SQ10_Pos)           /*!< 0x00000001 */
#define ADC_SQR3_SQ10_1                (0x02UL << ADC_SQR3_SQ10_Pos)           /*!< 0x00000002 */
#define ADC_SQR3_SQ10_2                (0x04UL << ADC_SQR3_SQ10_Pos)           /*!< 0x00000004 */
#define ADC_SQR3_SQ10_3                (0x08UL << ADC_SQR3_SQ10_Pos)           /*!< 0x00000008 */
#define ADC_SQR3_SQ10_4                (0x10UL << ADC_SQR3_SQ10_Pos)           /*!< 0x00000010 */

#define ADC_SQR3_SQ11_Pos              (6U)
#define ADC_SQR3_SQ11_Msk              (0x1FUL << ADC_SQR3_SQ11_Pos)           /*!< 0x000007C0 */
#define ADC_SQR3_SQ11                  ADC_SQR3_SQ11_Msk                       /*!< ADC group regular sequencer rank 11 */
#define ADC_SQR3_SQ11_0                (0x01UL << ADC_SQR3_SQ11_Pos)           /*!< 0x00000040 */
#define ADC_SQR3_SQ11_1                (0x02UL << ADC_SQR3_SQ11_Pos)           /*!< 0x00000080 */
#define ADC_SQR3_SQ11_2                (0x04UL << ADC_SQR3_SQ11_Pos)           /*!< 0x00000100 */
#define ADC_SQR3_SQ11_3                (0x08UL << ADC_SQR3_SQ11_Pos)           /*!< 0x00000200 */
#define ADC_SQR3_SQ11_4                (0x10UL << ADC_SQR3_SQ11_Pos)           /*!< 0x00000400 */

#define ADC_SQR3_SQ12_Pos              (12U)
#define ADC_SQR3_SQ12_Msk              (0x1FUL << ADC_SQR3_SQ12_Pos)           /*!< 0x0001F000 */
#define ADC_SQR3_SQ12                  ADC_SQR3_SQ12_Msk                       /*!< ADC group regular sequencer rank 12 */
#define ADC_SQR3_SQ12_0                (0x01UL << ADC_SQR3_SQ12_Pos)           /*!< 0x00001000 */
#define ADC_SQR3_SQ12_1                (0x02UL << ADC_SQR3_SQ12_Pos)           /*!< 0x00002000 */
#define ADC_SQR3_SQ12_2                (0x04UL << ADC_SQR3_SQ12_Pos)           /*!< 0x00004000 */
#define ADC_SQR3_SQ12_3                (0x08UL << ADC_SQR3_SQ12_Pos)           /*!< 0x00008000 */
#define ADC_SQR3_SQ12_4                (0x10UL << ADC_SQR3_SQ12_Pos)           /*!< 0x00010000 */

#define ADC_SQR3_SQ13_Pos              (18U)
#define ADC_SQR3_SQ13_Msk              (0x1FUL << ADC_SQR3_SQ13_Pos)           /*!< 0x007C0000 */
#define ADC_SQR3_SQ13                  ADC_SQR3_SQ13_Msk                       /*!< ADC group regular sequencer rank 13 */
#define ADC_SQR3_SQ13_0                (0x01UL << ADC_SQR3_SQ13_Pos)           /*!< 0x00040000 */
#define ADC_SQR3_SQ13_1                (0x02UL << ADC_SQR3_SQ13_Pos)           /*!< 0x00080000 */
#define ADC_SQR3_SQ13_2                (0x04UL << ADC_SQR3_SQ13_Pos)           /*!< 0x00100000 */
#define ADC_SQR3_SQ13_3                (0x08UL << ADC_SQR3_SQ13_Pos)           /*!< 0x00200000 */
#define ADC_SQR3_SQ13_4                (0x10UL << ADC_SQR3_SQ13_Pos)           /*!< 0x00400000 */

#define ADC_SQR3_SQ14_Pos              (24U)
#define ADC_SQR3_SQ14_Msk              (0x1FUL << ADC_SQR3_SQ14_Pos)           /*!< 0x1F000000 */
#define ADC_SQR3_SQ14                  ADC_SQR3_SQ14_Msk                       /*!< ADC group regular sequencer rank 14 */
#define ADC_SQR3_SQ14_0                (0x01UL << ADC_SQR3_SQ14_Pos)           /*!< 0x01000000 */
#define ADC_SQR3_SQ14_1                (0x02UL << ADC_SQR3_SQ14_Pos)           /*!< 0x02000000 */
#define ADC_SQR3_SQ14_2                (0x04UL << ADC_SQR3_SQ14_Pos)           /*!< 0x04000000 */
#define ADC_SQR3_SQ14_3                (0x08UL << ADC_SQR3_SQ14_Pos)           /*!< 0x08000000 */
#define ADC_SQR3_SQ14_4                (0x10UL << ADC_SQR3_SQ14_Pos)           /*!< 0x10000000 */

/********************  Bit definition for ADC_SQR4 register  ******************/
#define ADC_SQR4_SQ15_Pos              (0U)
#define ADC_SQR4_SQ15_Msk              (0x1FUL << ADC_SQR4_SQ15_Pos)           /*!< 0x0000001F */
#define ADC_SQR4_SQ15                  ADC_SQR4_SQ15_Msk                       /*!< ADC group regular sequencer rank 15 */
#define ADC_SQR4_SQ15_0                (0x01UL << ADC_SQR4_SQ15_Pos)           /*!< 0x00000001 */
#define ADC_SQR4_SQ15_1                (0x02UL << ADC_SQR4_SQ15_Pos)           /*!< 0x00000002 */
#define ADC_SQR4_SQ15_2                (0x04UL << ADC_SQR4_SQ15_Pos)           /*!< 0x00000004 */
#define ADC_SQR4_SQ15_3                (0x08UL << ADC_SQR4_SQ15_Pos)           /*!< 0x00000008 */
#define ADC_SQR4_SQ15_4                (0x10UL << ADC_SQR4_SQ15_Pos)           /*!< 0x00000010 */

#define ADC_SQR4_SQ16_Pos              (6U)
#define ADC_SQR4_SQ16_Msk              (0x1FUL << ADC_SQR4_SQ16_Pos)           /*!< 0x000007C0 */
#define ADC_SQR4_SQ16                  ADC_SQR4_SQ16_Msk                       /*!< ADC group regular sequencer rank 16 */
#define ADC_SQR4_SQ16_0                (0x01UL << ADC_SQR4_SQ16_Pos)           /*!< 0x00000040 */
#define ADC_SQR4_SQ16_1                (0x02UL << ADC_SQR4_SQ16_Pos)           /*!< 0x00000080 */
#define ADC_SQR4_SQ16_2                (0x04UL << ADC_SQR4_SQ16_Pos)           /*!< 0x00000100 */
#define ADC_SQR4_SQ16_3                (0x08UL << ADC_SQR4_SQ16_Pos)           /*!< 0x00000200 */
#define ADC_SQR4_SQ16_4                (0x10UL << ADC_SQR4_SQ16_Pos)           /*!< 0x00000400 */

/********************  Bit definition for ADC_DR register  ********************/
#define ADC_DR_RDATA_Pos               (0U)
#define ADC_DR_RDATA_Msk               (0xFFFFUL << ADC_DR_RDATA_Pos)          /*!< 0x0000FFFF */
#define ADC_DR_RDATA                   ADC_DR_RDATA_Msk                        /*!< ADC group regular conversion data */
#define ADC_DR_RDATA_0                 (0x0001UL << ADC_DR_RDATA_Pos)          /*!< 0x00000001 */
#define ADC_DR_RDATA_1                 (0x0002UL << ADC_DR_RDATA_Pos)          /*!< 0x00000002 */
#define ADC_DR_RDATA_2                 (0x0004UL << ADC_DR_RDATA_Pos)          /*!< 0x00000004 */
#define ADC_DR_RDATA_3                 (0x0008UL << ADC_DR_RDATA_Pos)          /*!< 0x00000008 */
#define ADC_DR_RDATA_4                 (0x0010UL << ADC_DR_RDATA_Pos)          /*!< 0x00000010 */
#define ADC_DR_RDATA_5                 (0x0020UL << ADC_DR_RDATA_Pos)          /*!< 0x00000020 */
#define ADC_DR_RDATA_6                 (0x0040UL << ADC_DR_RDATA_Pos)          /*!< 0x00000040 */
#define ADC_DR_RDATA_7                 (0x0080UL << ADC_DR_RDATA_Pos)          /*!< 0x00000080 */
#define ADC_DR_RDATA_8                 (0x0100UL << ADC_DR_RDATA_Pos)          /*!< 0x00000100 */
#define ADC_DR_RDATA_9                 (0x0200UL << ADC_DR_RDATA_Pos)          /*!< 0x00000200 */
#define ADC_DR_RDATA_10                (0x0400UL << ADC_DR_RDATA_Pos)          /*!< 0x00000400 */
#define ADC_DR_RDATA_11                (0x0800UL << ADC_DR_RDATA_Pos)          /*!< 0x00000800 */
#define ADC_DR_RDATA_12                (0x1000UL << ADC_DR_RDATA_Pos)          /*!< 0x00001000 */
#define ADC_DR_RDATA_13                (0x2000UL << ADC_DR_RDATA_Pos)          /*!< 0x00002000 */
#define ADC_DR_RDATA_14                (0x4000UL << ADC_DR_RDATA_Pos)          /*!< 0x00004000 */
#define ADC_DR_RDATA_15                (0x8000UL << ADC_DR_RDATA_Pos)          /*!< 0x00008000 */

/********************  Bit definition for ADC_JSQR register  ******************/
#define ADC_JSQR_JL_Pos                (0U)
#define ADC_JSQR_JL_Msk                (0x3UL << ADC_JSQR_JL_Pos)              /*!< 0x00000003 */
#define ADC_JSQR_JL                    ADC_JSQR_JL_Msk                         /*!< ADC group injected sequencer scan length */
#define ADC_JSQR_JL_0                  (0x1UL << ADC_JSQR_JL_Pos)              /*!< 0x00000001 */
#define ADC_JSQR_JL_1                  (0x2UL << ADC_JSQR_JL_Pos)              /*!< 0x00000002 */

#define ADC_JSQR_JEXTSEL_Pos           (2U)
#define ADC_JSQR_JEXTSEL_Msk           (0xFUL << ADC_JSQR_JEXTSEL_Pos)         /*!< 0x0000003C */
#define ADC_JSQR_JEXTSEL               ADC_JSQR_JEXTSEL_Msk                    /*!< ADC group injected external trigger source */
#define ADC_JSQR_JEXTSEL_0             (0x1UL << ADC_JSQR_JEXTSEL_Pos)         /*!< 0x00000004 */
#define ADC_JSQR_JEXTSEL_1             (0x2UL << ADC_JSQR_JEXTSEL_Pos)         /*!< 0x00000008 */
#define ADC_JSQR_JEXTSEL_2             (0x4UL << ADC_JSQR_JEXTSEL_Pos)         /*!< 0x00000010 */
#define ADC_JSQR_JEXTSEL_3             (0x8UL << ADC_JSQR_JEXTSEL_Pos)         /*!< 0x00000020 */

#define ADC_JSQR_JEXTEN_Pos            (6U)
#define ADC_JSQR_JEXTEN_Msk            (0x3UL << ADC_JSQR_JEXTEN_Pos)          /*!< 0x000000C0 */
#define ADC_JSQR_JEXTEN                ADC_JSQR_JEXTEN_Msk                     /*!< ADC group injected external trigger polarity */
#define ADC_JSQR_JEXTEN_0              (0x1UL << ADC_JSQR_JEXTEN_Pos)          /*!< 0x00000040 */
#define ADC_JSQR_JEXTEN_1              (0x2UL << ADC_JSQR_JEXTEN_Pos)          /*!< 0x00000080 */

#define ADC_JSQR_JSQ1_Pos              (8U)
#define ADC_JSQR_JSQ1_Msk              (0x1FUL << ADC_JSQR_JSQ1_Pos)           /*!< 0x00001F00 */
#define ADC_JSQR_JSQ1                  ADC_JSQR_JSQ1_Msk                       /*!< ADC group injected sequencer rank 1 */
#define ADC_JSQR_JSQ1_0                (0x01UL << ADC_JSQR_JSQ1_Pos)           /*!< 0x00000100 */
#define ADC_JSQR_JSQ1_1                (0x02UL << ADC_JSQR_JSQ1_Pos)           /*!< 0x00000200 */
#define ADC_JSQR_JSQ1_2                (0x04UL << ADC_JSQR_JSQ1_Pos)           /*!< 0x00000400 */
#define ADC_JSQR_JSQ1_3                (0x08UL << ADC_JSQR_JSQ1_Pos)           /*!< 0x00000800 */
#define ADC_JSQR_JSQ1_4                (0x10UL << ADC_JSQR_JSQ1_Pos)           /*!< 0x00001000 */

#define ADC_JSQR_JSQ2_Pos              (14U)
#define ADC_JSQR_JSQ2_Msk              (0x1FUL << ADC_JSQR_JSQ2_Pos)           /*!< 0x0007C000 */
#define ADC_JSQR_JSQ2                  ADC_JSQR_JSQ2_Msk                       /*!< ADC group injected sequencer rank 2 */
#define ADC_JSQR_JSQ2_0                (0x01UL << ADC_JSQR_JSQ2_Pos)           /*!< 0x00004000 */
#define ADC_JSQR_JSQ2_1                (0x02UL << ADC_JSQR_JSQ2_Pos)           /*!< 0x00008000 */
#define ADC_JSQR_JSQ2_2                (0x04UL << ADC_JSQR_JSQ2_Pos)           /*!< 0x00010000 */
#define ADC_JSQR_JSQ2_3                (0x08UL << ADC_JSQR_JSQ2_Pos)           /*!< 0x00020000 */
#define ADC_JSQR_JSQ2_4                (0x10UL << ADC_JSQR_JSQ2_Pos)           /*!< 0x00040000 */

#define ADC_JSQR_JSQ3_Pos              (20U)
#define ADC_JSQR_JSQ3_Msk              (0x1FUL << ADC_JSQR_JSQ3_Pos)           /*!< 0x01F00000 */
#define ADC_JSQR_JSQ3                  ADC_JSQR_JSQ3_Msk                       /*!< ADC group injected sequencer rank 3 */
#define ADC_JSQR_JSQ3_0                (0x01UL << ADC_JSQR_JSQ3_Pos)           /*!< 0x00100000 */
#define ADC_JSQR_JSQ3_1                (0x02UL << ADC_JSQR_JSQ3_Pos)           /*!< 0x00200000 */
#define ADC_JSQR_JSQ3_2                (0x04UL << ADC_JSQR_JSQ3_Pos)           /*!< 0x00400000 */
#define ADC_JSQR_JSQ3_3                (0x08UL << ADC_JSQR_JSQ3_Pos)           /*!< 0x00800000 */
#define ADC_JSQR_JSQ3_4                (0x10UL << ADC_JSQR_JSQ3_Pos)           /*!< 0x01000000 */

#define ADC_JSQR_JSQ4_Pos              (26U)
#define ADC_JSQR_JSQ4_Msk              (0x1FUL << ADC_JSQR_JSQ4_Pos)           /*!< 0x7C000000 */
#define ADC_JSQR_JSQ4                  ADC_JSQR_JSQ4_Msk                       /*!< ADC group injected sequencer rank 4 */
#define ADC_JSQR_JSQ4_0                (0x01UL << ADC_JSQR_JSQ4_Pos)           /*!< 0x04000000 */
#define ADC_JSQR_JSQ4_1                (0x02UL << ADC_JSQR_JSQ4_Pos)           /*!< 0x08000000 */
#define ADC_JSQR_JSQ4_2                (0x04UL << ADC_JSQR_JSQ4_Pos)           /*!< 0x10000000 */
#define ADC_JSQR_JSQ4_3                (0x08UL << ADC_JSQR_JSQ4_Pos)           /*!< 0x20000000 */
#define ADC_JSQR_JSQ4_4                (0x10UL << ADC_JSQR_JSQ4_Pos)           /*!< 0x40000000 */

/********************  Bit definition for ADC_OFR1 register  ******************/
#define ADC_OFR1_OFFSET1_Pos           (0U)
#define ADC_OFR1_OFFSET1_Msk           (0xFFFUL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000FFF */
#define ADC_OFR1_OFFSET1               ADC_OFR1_OFFSET1_Msk                    /*!< ADC offset number 1 offset level */
#define ADC_OFR1_OFFSET1_0             (0x001UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000001 */
#define ADC_OFR1_OFFSET1_1             (0x002UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000002 */
#define ADC_OFR1_OFFSET1_2             (0x004UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000004 */
#define ADC_OFR1_OFFSET1_3             (0x008UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000008 */
#define ADC_OFR1_OFFSET1_4             (0x010UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000010 */
#define ADC_OFR1_OFFSET1_5             (0x020UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000020 */
#define ADC_OFR1_OFFSET1_6             (0x040UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000040 */
#define ADC_OFR1_OFFSET1_7             (0x080UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000080 */
#define ADC_OFR1_OFFSET1_8             (0x100UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000100 */
#define ADC_OFR1_OFFSET1_9             (0x200UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000200 */
#define ADC_OFR1_OFFSET1_10            (0x400UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000400 */
#define ADC_OFR1_OFFSET1_11            (0x800UL << ADC_OFR1_OFFSET1_Pos)       /*!< 0x00000800 */

#define ADC_OFR1_OFFSET1_CH_Pos        (26U)
#define ADC_OFR1_OFFSET1_CH_Msk        (0x1FUL << ADC_OFR1_OFFSET1_CH_Pos)     /*!< 0x7C000000 */
#define ADC_OFR1_OFFSET1_CH            ADC_OFR1_OFFSET1_CH_Msk                 /*!< ADC offset number 1 channel selection */
#define ADC_OFR1_OFFSET1_CH_0          (0x01UL << ADC_OFR1_OFFSET1_CH_Pos)     /*!< 0x04000000 */
#define ADC_OFR1_OFFSET1_CH_1          (0x02UL << ADC_OFR1_OFFSET1_CH_Pos)     /*!< 0x08000000 */
#define ADC_OFR1_OFFSET1_CH_2          (0x04UL << ADC_OFR1_OFFSET1_CH_Pos)     /*!< 0x10000000 */
#define ADC_OFR1_OFFSET1_CH_3          (0x08UL << ADC_OFR1_OFFSET1_CH_Pos)     /*!< 0x20000000 */
#define ADC_OFR1_OFFSET1_CH_4          (0x10UL << ADC_OFR1_OFFSET1_CH_Pos)     /*!< 0x40000000 */

#define ADC_OFR1_OFFSET1_EN_Pos        (31U)
#define ADC_OFR1_OFFSET1_EN_Msk        (0x1UL << ADC_OFR1_OFFSET1_EN_Pos)      /*!< 0x80000000 */
#define ADC_OFR1_OFFSET1_EN            ADC_OFR1_OFFSET1_EN_Msk                 /*!< ADC offset number 1 enable */

/********************  Bit definition for ADC_OFR2 register  ******************/
#define ADC_OFR2_OFFSET2_Pos           (0U)
#define ADC_OFR2_OFFSET2_Msk           (0xFFFUL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000FFF */
#define ADC_OFR2_OFFSET2               ADC_OFR2_OFFSET2_Msk                    /*!< ADC offset number 2 offset level */
#define ADC_OFR2_OFFSET2_0             (0x001UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000001 */
#define ADC_OFR2_OFFSET2_1             (0x002UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000002 */
#define ADC_OFR2_OFFSET2_2             (0x004UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000004 */
#define ADC_OFR2_OFFSET2_3             (0x008UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000008 */
#define ADC_OFR2_OFFSET2_4             (0x010UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000010 */
#define ADC_OFR2_OFFSET2_5             (0x020UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000020 */
#define ADC_OFR2_OFFSET2_6             (0x040UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000040 */
#define ADC_OFR2_OFFSET2_7             (0x080UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000080 */
#define ADC_OFR2_OFFSET2_8             (0x100UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000100 */
#define ADC_OFR2_OFFSET2_9             (0x200UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000200 */
#define ADC_OFR2_OFFSET2_10            (0x400UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000400 */
#define ADC_OFR2_OFFSET2_11            (0x800UL << ADC_OFR2_OFFSET2_Pos)       /*!< 0x00000800 */

#define ADC_OFR2_OFFSET2_CH_Pos        (26U)
#define ADC_OFR2_OFFSET2_CH_Msk        (0x1FUL << ADC_OFR2_OFFSET2_CH_Pos)     /*!< 0x7C000000 */
#define ADC_OFR2_OFFSET2_CH            ADC_OFR2_OFFSET2_CH_Msk                 /*!< ADC offset number 2 channel selection */
#define ADC_OFR2_OFFSET2_CH_0          (0x01UL << ADC_OFR2_OFFSET2_CH_Pos)     /*!< 0x04000000 */
#define ADC_OFR2_OFFSET2_CH_1          (0x02UL << ADC_OFR2_OFFSET2_CH_Pos)     /*!< 0x08000000 */
#define ADC_OFR2_OFFSET2_CH_2          (0x04UL << ADC_OFR2_OFFSET2_CH_Pos)     /*!< 0x10000000 */
#define ADC_OFR2_OFFSET2_CH_3          (0x08UL << ADC_OFR2_OFFSET2_CH_Pos)     /*!< 0x20000000 */
#define ADC_OFR2_OFFSET2_CH_4          (0x10UL << ADC_OFR2_OFFSET2_CH_Pos)     /*!< 0x40000000 */

#define ADC_OFR2_OFFSET2_EN_Pos        (31U)
#define ADC_OFR2_OFFSET2_EN_Msk        (0x1UL << ADC_OFR2_OFFSET2_EN_Pos)      /*!< 0x80000000 */
#define ADC_OFR2_OFFSET2_EN            ADC_OFR2_OFFSET2_EN_Msk                 /*!< ADC offset number 2 enable */

/********************  Bit definition for ADC_OFR3 register  ******************/
#define ADC_OFR3_OFFSET3_Pos           (0U)
#define ADC_OFR3_OFFSET3_Msk           (0xFFFUL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000FFF */
#define ADC_OFR3_OFFSET3               ADC_OFR3_OFFSET3_Msk                    /*!< ADC offset number 3 offset level */
#define ADC_OFR3_OFFSET3_0             (0x001UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000001 */
#define ADC_OFR3_OFFSET3_1             (0x002UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000002 */
#define ADC_OFR3_OFFSET3_2             (0x004UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000004 */
#define ADC_OFR3_OFFSET3_3             (0x008UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000008 */
#define ADC_OFR3_OFFSET3_4             (0x010UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000010 */
#define ADC_OFR3_OFFSET3_5             (0x020UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000020 */
#define ADC_OFR3_OFFSET3_6             (0x040UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000040 */
#define ADC_OFR3_OFFSET3_7             (0x080UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000080 */
#define ADC_OFR3_OFFSET3_8             (0x100UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000100 */
#define ADC_OFR3_OFFSET3_9             (0x200UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000200 */
#define ADC_OFR3_OFFSET3_10            (0x400UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000400 */
#define ADC_OFR3_OFFSET3_11            (0x800UL << ADC_OFR3_OFFSET3_Pos)       /*!< 0x00000800 */

#define ADC_OFR3_OFFSET3_CH_Pos        (26U)
#define ADC_OFR3_OFFSET3_CH_Msk        (0x1FUL << ADC_OFR3_OFFSET3_CH_Pos)     /*!< 0x7C000000 */
#define ADC_OFR3_OFFSET3_CH            ADC_OFR3_OFFSET3_CH_Msk                 /*!< ADC offset number 3 channel selection */
#define ADC_OFR3_OFFSET3_CH_0          (0x01UL << ADC_OFR3_OFFSET3_CH_Pos)     /*!< 0x04000000 */
#define ADC_OFR3_OFFSET3_CH_1          (0x02UL << ADC_OFR3_OFFSET3_CH_Pos)     /*!< 0x08000000 */
#define ADC_OFR3_OFFSET3_CH_2          (0x04UL << ADC_OFR3_OFFSET3_CH_Pos)     /*!< 0x10000000 */
#define ADC_OFR3_OFFSET3_CH_3          (0x08UL << ADC_OFR3_OFFSET3_CH_Pos)     /*!< 0x20000000 */
#define ADC_OFR3_OFFSET3_CH_4          (0x10UL << ADC_OFR3_OFFSET3_CH_Pos)     /*!< 0x40000000 */

#define ADC_OFR3_OFFSET3_EN_Pos        (31U)
#define ADC_OFR3_OFFSET3_EN_Msk        (0x1UL << ADC_OFR3_OFFSET3_EN_Pos)      /*!< 0x80000000 */
#define ADC_OFR3_OFFSET3_EN            ADC_OFR3_OFFSET3_EN_Msk                 /*!< ADC offset number 3 enable */

/********************  Bit definition for ADC_OFR4 register  ******************/
#define ADC_OFR4_OFFSET4_Pos           (0U)
#define ADC_OFR4_OFFSET4_Msk           (0xFFFUL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000FFF */
#define ADC_OFR4_OFFSET4               ADC_OFR4_OFFSET4_Msk                    /*!< ADC offset number 4 offset level */
#define ADC_OFR4_OFFSET4_0             (0x001UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000001 */
#define ADC_OFR4_OFFSET4_1             (0x002UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000002 */
#define ADC_OFR4_OFFSET4_2             (0x004UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000004 */
#define ADC_OFR4_OFFSET4_3             (0x008UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000008 */
#define ADC_OFR4_OFFSET4_4             (0x010UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000010 */
#define ADC_OFR4_OFFSET4_5             (0x020UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000020 */
#define ADC_OFR4_OFFSET4_6             (0x040UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000040 */
#define ADC_OFR4_OFFSET4_7             (0x080UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000080 */
#define ADC_OFR4_OFFSET4_8             (0x100UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000100 */
#define ADC_OFR4_OFFSET4_9             (0x200UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000200 */
#define ADC_OFR4_OFFSET4_10            (0x400UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000400 */
#define ADC_OFR4_OFFSET4_11            (0x800UL << ADC_OFR4_OFFSET4_Pos)       /*!< 0x00000800 */

#define ADC_OFR4_OFFSET4_CH_Pos        (26U)
#define ADC_OFR4_OFFSET4_CH_Msk        (0x1FUL << ADC_OFR4_OFFSET4_CH_Pos)     /*!< 0x7C000000 */
#define ADC_OFR4_OFFSET4_CH            ADC_OFR4_OFFSET4_CH_Msk                 /*!< ADC offset number 4 channel selection */
#define ADC_OFR4_OFFSET4_CH_0          (0x01UL << ADC_OFR4_OFFSET4_CH_Pos)     /*!< 0x04000000 */
#define ADC_OFR4_OFFSET4_CH_1          (0x02UL << ADC_OFR4_OFFSET4_CH_Pos)     /*!< 0x08000000 */
#define ADC_OFR4_OFFSET4_CH_2          (0x04UL << ADC_OFR4_OFFSET4_CH_Pos)     /*!< 0x10000000 */
#define ADC_OFR4_OFFSET4_CH_3          (0x08UL << ADC_OFR4_OFFSET4_CH_Pos)     /*!< 0x20000000 */
#define ADC_OFR4_OFFSET4_CH_4          (0x10UL << ADC_OFR4_OFFSET4_CH_Pos)     /*!< 0x40000000 */

#define ADC_OFR4_OFFSET4_EN_Pos        (31U)
#define ADC_OFR4_OFFSET4_EN_Msk        (0x1UL << ADC_OFR4_OFFSET4_EN_Pos)      /*!< 0x80000000 */
#define ADC_OFR4_OFFSET4_EN            ADC_OFR4_OFFSET4_EN_Msk                 /*!< ADC offset number 4 enable */

/********************  Bit definition for ADC_JDR1 register  ******************/
#define ADC_JDR1_JDATA_Pos             (0U)
#define ADC_JDR1_JDATA_Msk             (0xFFFFUL << ADC_JDR1_JDATA_Pos)        /*!< 0x0000FFFF */
#define ADC_JDR1_JDATA                 ADC_JDR1_JDATA_Msk                      /*!< ADC group injected sequencer rank 1 conversion data */
#define ADC_JDR1_JDATA_0               (0x0001UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000001 */
#define ADC_JDR1_JDATA_1               (0x0002UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000002 */
#define ADC_JDR1_JDATA_2               (0x0004UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000004 */
#define ADC_JDR1_JDATA_3               (0x0008UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000008 */
#define ADC_JDR1_JDATA_4               (0x0010UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000010 */
#define ADC_JDR1_JDATA_5               (0x0020UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000020 */
#define ADC_JDR1_JDATA_6               (0x0040UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000040 */
#define ADC_JDR1_JDATA_7               (0x0080UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000080 */
#define ADC_JDR1_JDATA_8               (0x0100UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000100 */
#define ADC_JDR1_JDATA_9               (0x0200UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000200 */
#define ADC_JDR1_JDATA_10              (0x0400UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000400 */
#define ADC_JDR1_JDATA_11              (0x0800UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00000800 */
#define ADC_JDR1_JDATA_12              (0x1000UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00001000 */
#define ADC_JDR1_JDATA_13              (0x2000UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00002000 */
#define ADC_JDR1_JDATA_14              (0x4000UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00004000 */
#define ADC_JDR1_JDATA_15              (0x8000UL << ADC_JDR1_JDATA_Pos)        /*!< 0x00008000 */

/********************  Bit definition for ADC_JDR2 register  ******************/
#define ADC_JDR2_JDATA_Pos             (0U)
#define ADC_JDR2_JDATA_Msk             (0xFFFFUL << ADC_JDR2_JDATA_Pos)        /*!< 0x0000FFFF */
#define ADC_JDR2_JDATA                 ADC_JDR2_JDATA_Msk                      /*!< ADC group injected sequencer rank 2 conversion data */
#define ADC_JDR2_JDATA_0               (0x0001UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000001 */
#define ADC_JDR2_JDATA_1               (0x0002UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000002 */
#define ADC_JDR2_JDATA_2               (0x0004UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000004 */
#define ADC_JDR2_JDATA_3               (0x0008UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000008 */
#define ADC_JDR2_JDATA_4               (0x0010UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000010 */
#define ADC_JDR2_JDATA_5               (0x0020UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000020 */
#define ADC_JDR2_JDATA_6               (0x0040UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000040 */
#define ADC_JDR2_JDATA_7               (0x0080UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000080 */
#define ADC_JDR2_JDATA_8               (0x0100UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000100 */
#define ADC_JDR2_JDATA_9               (0x0200UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000200 */
#define ADC_JDR2_JDATA_10              (0x0400UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000400 */
#define ADC_JDR2_JDATA_11              (0x0800UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00000800 */
#define ADC_JDR2_JDATA_12              (0x1000UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00001000 */
#define ADC_JDR2_JDATA_13              (0x2000UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00002000 */
#define ADC_JDR2_JDATA_14              (0x4000UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00004000 */
#define ADC_JDR2_JDATA_15              (0x8000UL << ADC_JDR2_JDATA_Pos)        /*!< 0x00008000 */

/********************  Bit definition for ADC_JDR3 register  ******************/
#define ADC_JDR3_JDATA_Pos             (0U)
#define ADC_JDR3_JDATA_Msk             (0xFFFFUL << ADC_JDR3_JDATA_Pos)        /*!< 0x0000FFFF */
#define ADC_JDR3_JDATA                 ADC_JDR3_JDATA_Msk                      /*!< ADC group injected sequencer rank 3 conversion data */
#define ADC_JDR3_JDATA_0               (0x0001UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000001 */
#define ADC_JDR3_JDATA_1               (0x0002UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000002 */
#define ADC_JDR3_JDATA_2               (0x0004UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000004 */
#define ADC_JDR3_JDATA_3               (0x0008UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000008 */
#define ADC_JDR3_JDATA_4               (0x0010UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000010 */
#define ADC_JDR3_JDATA_5               (0x0020UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000020 */
#define ADC_JDR3_JDATA_6               (0x0040UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000040 */
#define ADC_JDR3_JDATA_7               (0x0080UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000080 */
#define ADC_JDR3_JDATA_8               (0x0100UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000100 */
#define ADC_JDR3_JDATA_9               (0x0200UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000200 */
#define ADC_JDR3_JDATA_10              (0x0400UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000400 */
#define ADC_JDR3_JDATA_11              (0x0800UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00000800 */
#define ADC_JDR3_JDATA_12              (0x1000UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00001000 */
#define ADC_JDR3_JDATA_13              (0x2000UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00002000 */
#define ADC_JDR3_JDATA_14              (0x4000UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00004000 */
#define ADC_JDR3_JDATA_15              (0x8000UL << ADC_JDR3_JDATA_Pos)        /*!< 0x00008000 */

/********************  Bit definition for ADC_JDR4 register  ******************/
#define ADC_JDR4_JDATA_Pos             (0U)
#define ADC_JDR4_JDATA_Msk             (0xFFFFUL << ADC_JDR4_JDATA_Pos)        /*!< 0x0000FFFF */
#define ADC_JDR4_JDATA                 ADC_JDR4_JDATA_Msk                      /*!< ADC group injected sequencer rank 4 conversion data */
#define ADC_JDR4_JDATA_0               (0x0001UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000001 */
#define ADC_JDR4_JDATA_1               (0x0002UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000002 */
#define ADC_JDR4_JDATA_2               (0x0004UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000004 */
#define ADC_JDR4_JDATA_3               (0x0008UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000008 */
#define ADC_JDR4_JDATA_4               (0x0010UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000010 */
#define ADC_JDR4_JDATA_5               (0x0020UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000020 */
#define ADC_JDR4_JDATA_6               (0x0040UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000040 */
#define ADC_JDR4_JDATA_7               (0x0080UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000080 */
#define ADC_JDR4_JDATA_8               (0x0100UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000100 */
#define ADC_JDR4_JDATA_9               (0x0200UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000200 */
#define ADC_JDR4_JDATA_10              (0x0400UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000400 */
#define ADC_JDR4_JDATA_11              (0x0800UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00000800 */
#define ADC_JDR4_JDATA_12              (0x1000UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00001000 */
#define ADC_JDR4_JDATA_13              (0x2000UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00002000 */
#define ADC_JDR4_JDATA_14              (0x4000UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00004000 */
#define ADC_JDR4_JDATA_15              (0x8000UL << ADC_JDR4_JDATA_Pos)        /*!< 0x00008000 */

/********************  Bit definition for ADC_AWD2CR register  ****************/
#define ADC_AWD2CR_AWD2CH_Pos          (0U)
#define ADC_AWD2CR_AWD2CH_Msk          (0x7FFFFUL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x0007FFFF */
#define ADC_AWD2CR_AWD2CH              ADC_AWD2CR_AWD2CH_Msk                   /*!< ADC analog watchdog 2 monitored channel selection */
#define ADC_AWD2CR_AWD2CH_0            (0x00001UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000001 */
#define ADC_AWD2CR_AWD2CH_1            (0x00002UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000002 */
#define ADC_AWD2CR_AWD2CH_2            (0x00004UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000004 */
#define ADC_AWD2CR_AWD2CH_3            (0x00008UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000008 */
#define ADC_AWD2CR_AWD2CH_4            (0x00010UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000010 */
#define ADC_AWD2CR_AWD2CH_5            (0x00020UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000020 */
#define ADC_AWD2CR_AWD2CH_6            (0x00040UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000040 */
#define ADC_AWD2CR_AWD2CH_7            (0x00080UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000080 */
#define ADC_AWD2CR_AWD2CH_8            (0x00100UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000100 */
#define ADC_AWD2CR_AWD2CH_9            (0x00200UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000200 */
#define ADC_AWD2CR_AWD2CH_10           (0x00400UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000400 */
#define ADC_AWD2CR_AWD2CH_11           (0x00800UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00000800 */
#define ADC_AWD2CR_AWD2CH_12           (0x01000UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00001000 */
#define ADC_AWD2CR_AWD2CH_13           (0x02000UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00002000 */
#define ADC_AWD2CR_AWD2CH_14           (0x04000UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00004000 */
#define ADC_AWD2CR_AWD2CH_15           (0x08000UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00008000 */
#define ADC_AWD2CR_AWD2CH_16           (0x10000UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00010000 */
#define ADC_AWD2CR_AWD2CH_17           (0x20000UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00020000 */
#define ADC_AWD2CR_AWD2CH_18           (0x40000UL << ADC_AWD2CR_AWD2CH_Pos)    /*!< 0x00040000 */

/********************  Bit definition for ADC_AWD3CR register  ****************/
#define ADC_AWD3CR_AWD3CH_Pos          (0U)
#define ADC_AWD3CR_AWD3CH_Msk          (0x7FFFFUL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x0007FFFF */
#define ADC_AWD3CR_AWD3CH              ADC_AWD3CR_AWD3CH_Msk                   /*!< ADC analog watchdog 3 monitored channel selection */
#define ADC_AWD3CR_AWD3CH_0            (0x00001UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000001 */
#define ADC_AWD3CR_AWD3CH_1            (0x00002UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000002 */
#define ADC_AWD3CR_AWD3CH_2            (0x00004UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000004 */
#define ADC_AWD3CR_AWD3CH_3            (0x00008UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000008 */
#define ADC_AWD3CR_AWD3CH_4            (0x00010UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000010 */
#define ADC_AWD3CR_AWD3CH_5            (0x00020UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000020 */
#define ADC_AWD3CR_AWD3CH_6            (0x00040UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000040 */
#define ADC_AWD3CR_AWD3CH_7            (0x00080UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000080 */
#define ADC_AWD3CR_AWD3CH_8            (0x00100UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000100 */
#define ADC_AWD3CR_AWD3CH_9            (0x00200UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000200 */
#define ADC_AWD3CR_AWD3CH_10           (0x00400UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000400 */
#define ADC_AWD3CR_AWD3CH_11           (0x00800UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00000800 */
#define ADC_AWD3CR_AWD3CH_12           (0x01000UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00001000 */
#define ADC_AWD3CR_AWD3CH_13           (0x02000UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00002000 */
#define ADC_AWD3CR_AWD3CH_14           (0x04000UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00004000 */
#define ADC_AWD3CR_AWD3CH_15           (0x08000UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00008000 */
#define ADC_AWD3CR_AWD3CH_16           (0x10000UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00010000 */
#define ADC_AWD3CR_AWD3CH_17           (0x20000UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00020000 */
#define ADC_AWD3CR_AWD3CH_18           (0x40000UL << ADC_AWD3CR_AWD3CH_Pos)    /*!< 0x00040000 */

/********************  Bit definition for ADC_DIFSEL register  ****************/
#define ADC_DIFSEL_DIFSEL_Pos          (0U)
#define ADC_DIFSEL_DIFSEL_Msk          (0x7FFFFUL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x0007FFFF */
#define ADC_DIFSEL_DIFSEL              ADC_DIFSEL_DIFSEL_Msk                   /*!< ADC channel differential or single-ended mode */
#define ADC_DIFSEL_DIFSEL_0            (0x00001UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000001 */
#define ADC_DIFSEL_DIFSEL_1            (0x00002UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000002 */
#define ADC_DIFSEL_DIFSEL_2            (0x00004UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000004 */
#define ADC_DIFSEL_DIFSEL_3            (0x00008UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000008 */
#define ADC_DIFSEL_DIFSEL_4            (0x00010UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000010 */
#define ADC_DIFSEL_DIFSEL_5            (0x00020UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000020 */
#define ADC_DIFSEL_DIFSEL_6            (0x00040UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000040 */
#define ADC_DIFSEL_DIFSEL_7            (0x00080UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000080 */
#define ADC_DIFSEL_DIFSEL_8            (0x00100UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000100 */
#define ADC_DIFSEL_DIFSEL_9            (0x00200UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000200 */
#define ADC_DIFSEL_DIFSEL_10           (0x00400UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000400 */
#define ADC_DIFSEL_DIFSEL_11           (0x00800UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00000800 */
#define ADC_DIFSEL_DIFSEL_12           (0x01000UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00001000 */
#define ADC_DIFSEL_DIFSEL_13           (0x02000UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00002000 */
#define ADC_DIFSEL_DIFSEL_14           (0x04000UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00004000 */
#define ADC_DIFSEL_DIFSEL_15           (0x08000UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00008000 */
#define ADC_DIFSEL_DIFSEL_16           (0x10000UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00010000 */
#define ADC_DIFSEL_DIFSEL_17           (0x20000UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00020000 */
#define ADC_DIFSEL_DIFSEL_18           (0x40000UL << ADC_DIFSEL_DIFSEL_Pos)    /*!< 0x00040000 */

/********************  Bit definition for ADC_CALFACT register  ***************/
#define ADC_CALFACT_CALFACT_S_Pos      (0U)
#define ADC_CALFACT_CALFACT_S_Msk      (0x7FUL << ADC_CALFACT_CALFACT_S_Pos)   /*!< 0x0000007F */
#define ADC_CALFACT_CALFACT_S          ADC_CALFACT_CALFACT_S_Msk               /*!< ADC calibration factor in single-ended mode */
#define ADC_CALFACT_CALFACT_S_0        (0x01UL << ADC_CALFACT_CALFACT_S_Pos)   /*!< 0x00000001 */
#define ADC_CALFACT_CALFACT_S_1        (0x02UL << ADC_CALFACT_CALFACT_S_Pos)   /*!< 0x00000002 */
#define ADC_CALFACT_CALFACT_S_2        (0x04UL << ADC_CALFACT_CALFACT_S_Pos)   /*!< 0x00000004 */
#define ADC_CALFACT_CALFACT_S_3        (0x08UL << ADC_CALFACT_CALFACT_S_Pos)   /*!< 0x00000008 */
#define ADC_CALFACT_CALFACT_S_4        (0x10UL << ADC_CALFACT_CALFACT_S_Pos)   /*!< 0x00000010 */
#define ADC_CALFACT_CALFACT_S_5        (0x20UL << ADC_CALFACT_CALFACT_S_Pos)   /*!< 0x00000020 */
#define ADC_CALFACT_CALFACT_S_6        (0x40UL << ADC_CALFACT_CALFACT_S_Pos)   /*!< 0x00000040 */

#define ADC_CALFACT_CALFACT_D_Pos      (16U)
#define ADC_CALFACT_CALFACT_D_Msk      (0x7FUL << ADC_CALFACT_CALFACT_D_Pos)   /*!< 0x007F0000 */
#define ADC_CALFACT_CALFACT_D          ADC_CALFACT_CALFACT_D_Msk               /*!< ADC calibration factor in differential mode */
#define ADC_CALFACT_CALFACT_D_0        (0x01UL << ADC_CALFACT_CALFACT_D_Pos)   /*!< 0x00010000 */
#define ADC_CALFACT_CALFACT_D_1        (0x02UL << ADC_CALFACT_CALFACT_D_Pos)   /*!< 0x00020000 */
#define ADC_CALFACT_CALFACT_D_2        (0x04UL << ADC_CALFACT_CALFACT_D_Pos)   /*!< 0x00040000 */
#define ADC_CALFACT_CALFACT_D_3        (0x08UL << ADC_CALFACT_CALFACT_D_Pos)   /*!< 0x00080000 */
#define ADC_CALFACT_CALFACT_D_4        (0x10UL << ADC_CALFACT_CALFACT_D_Pos)   /*!< 0x00100000 */
#define ADC_CALFACT_CALFACT_D_5        (0x20UL << ADC_CALFACT_CALFACT_D_Pos)   /*!< 0x00200000 */
#define ADC_CALFACT_CALFACT_D_6        (0x40UL << ADC_CALFACT_CALFACT_D_Pos)   /*!< 0x00400000 */

/*************************  ADC Common registers  *****************************/
/********************  Bit definition for ADC_CSR register  *******************/
#define ADC_CSR_ADRDY_MST_Pos          (0U)
#define ADC_CSR_ADRDY_MST_Msk          (0x1UL << ADC_CSR_ADRDY_MST_Pos)        /*!< 0x00000001 */
#define ADC_CSR_ADRDY_MST              ADC_CSR_ADRDY_MST_Msk                   /*!< ADC multimode master ready flag */
#define ADC_CSR_EOSMP_MST_Pos          (1U)
#define ADC_CSR_EOSMP_MST_Msk          (0x1UL << ADC_CSR_EOSMP_MST_Pos)        /*!< 0x00000002 */
#define ADC_CSR_EOSMP_MST              ADC_CSR_EOSMP_MST_Msk                   /*!< ADC multimode master group regular end of sampling flag */
#define ADC_CSR_EOC_MST_Pos            (2U)
#define ADC_CSR_EOC_MST_Msk            (0x1UL << ADC_CSR_EOC_MST_Pos)          /*!< 0x00000004 */
#define ADC_CSR_EOC_MST                ADC_CSR_EOC_MST_Msk                     /*!< ADC multimode master group regular end of unitary conversion flag */
#define ADC_CSR_EOS_MST_Pos            (3U)
#define ADC_CSR_EOS_MST_Msk            (0x1UL << ADC_CSR_EOS_MST_Pos)          /*!< 0x00000008 */
#define ADC_CSR_EOS_MST                ADC_CSR_EOS_MST_Msk                     /*!< ADC multimode master group regular end of sequence conversions flag */
#define ADC_CSR_OVR_MST_Pos            (4U)
#define ADC_CSR_OVR_MST_Msk            (0x1UL << ADC_CSR_OVR_MST_Pos)          /*!< 0x00000010 */
#define ADC_CSR_OVR_MST                ADC_CSR_OVR_MST_Msk                     /*!< ADC multimode master group regular overrun flag */
#define ADC_CSR_JEOC_MST_Pos           (5U)
#define ADC_CSR_JEOC_MST_Msk           (0x1UL << ADC_CSR_JEOC_MST_Pos)         /*!< 0x00000020 */
#define ADC_CSR_JEOC_MST               ADC_CSR_JEOC_MST_Msk                    /*!< ADC multimode master group injected end of unitary conversion flag */
#define ADC_CSR_JEOS_MST_Pos           (6U)
#define ADC_CSR_JEOS_MST_Msk           (0x1UL << ADC_CSR_JEOS_MST_Pos)         /*!< 0x00000040 */
#define ADC_CSR_JEOS_MST               ADC_CSR_JEOS_MST_Msk                    /*!< ADC multimode master group injected end of sequence conversions flag */
#define ADC_CSR_AWD1_MST_Pos           (7U)
#define ADC_CSR_AWD1_MST_Msk           (0x1UL << ADC_CSR_AWD1_MST_Pos)         /*!< 0x00000080 */
#define ADC_CSR_AWD1_MST               ADC_CSR_AWD1_MST_Msk                    /*!< ADC multimode master analog watchdog 1 flag */
#define ADC_CSR_AWD2_MST_Pos           (8U)
#define ADC_CSR_AWD2_MST_Msk           (0x1UL << ADC_CSR_AWD2_MST_Pos)         /*!< 0x00000100 */
#define ADC_CSR_AWD2_MST               ADC_CSR_AWD2_MST_Msk                    /*!< ADC multimode master analog watchdog 2 flag */
#define ADC_CSR_AWD3_MST_Pos           (9U)
#define ADC_CSR_AWD3_MST_Msk           (0x1UL << ADC_CSR_AWD3_MST_Pos)         /*!< 0x00000200 */
#define ADC_CSR_AWD3_MST               ADC_CSR_AWD3_MST_Msk                    /*!< ADC multimode master analog watchdog 3 flag */
#define ADC_CSR_JQOVF_MST_Pos          (10U)
#define ADC_CSR_JQOVF_MST_Msk          (0x1UL << ADC_CSR_JQOVF_MST_Pos)        /*!< 0x00000400 */
#define ADC_CSR_JQOVF_MST              ADC_CSR_JQOVF_MST_Msk                   /*!< ADC multimode master group injected contexts queue overflow flag */

#define ADC_CSR_ADRDY_SLV_Pos          (16U)
#define ADC_CSR_ADRDY_SLV_Msk          (0x1UL << ADC_CSR_ADRDY_SLV_Pos)        /*!< 0x00010000 */
#define ADC_CSR_ADRDY_SLV              ADC_CSR_ADRDY_SLV_Msk                   /*!< ADC multimode slave ready flag */
#define ADC_CSR_EOSMP_SLV_Pos          (17U)
#define ADC_CSR_EOSMP_SLV_Msk          (0x1UL << ADC_CSR_EOSMP_SLV_Pos)        /*!< 0x00020000 */
#define ADC_CSR_EOSMP_SLV              ADC_CSR_EOSMP_SLV_Msk                   /*!< ADC multimode slave group regular end of sampling flag */
#define ADC_CSR_EOC_SLV_Pos            (18U)
#define ADC_CSR_EOC_SLV_Msk            (0x1UL << ADC_CSR_EOC_SLV_Pos)          /*!< 0x00040000 */
#define ADC_CSR_EOC_SLV                ADC_CSR_EOC_SLV_Msk                     /*!< ADC multimode slave group regular end of unitary conversion flag */
#define ADC_CSR_EOS_SLV_Pos            (19U)
#define ADC_CSR_EOS_SLV_Msk            (0x1UL << ADC_CSR_EOS_SLV_Pos)          /*!< 0x00080000 */
#define ADC_CSR_EOS_SLV                ADC_CSR_EOS_SLV_Msk                     /*!< ADC multimode slave group regular end of sequence conversions flag */
#define ADC_CSR_OVR_SLV_Pos            (20U)
#define ADC_CSR_OVR_SLV_Msk            (0x1UL << ADC_CSR_OVR_SLV_Pos)          /*!< 0x00100000 */
#define ADC_CSR_OVR_SLV                ADC_CSR_OVR_SLV_Msk                     /*!< ADC multimode slave group regular overrun flag */
#define ADC_CSR_JEOC_SLV_Pos           (21U)
#define ADC_CSR_JEOC_SLV_Msk           (0x1UL << ADC_CSR_JEOC_SLV_Pos)         /*!< 0x00200000 */
#define ADC_CSR_JEOC_SLV               ADC_CSR_JEOC_SLV_Msk                    /*!< ADC multimode slave group injected end of unitary conversion flag */
#define ADC_CSR_JEOS_SLV_Pos           (22U)
#define ADC_CSR_JEOS_SLV_Msk           (0x1UL << ADC_CSR_JEOS_SLV_Pos)         /*!< 0x00400000 */
#define ADC_CSR_JEOS_SLV               ADC_CSR_JEOS_SLV_Msk                    /*!< ADC multimode slave group injected end of sequence conversions flag */
#define ADC_CSR_AWD1_SLV_Pos           (23U)
#define ADC_CSR_AWD1_SLV_Msk           (0x1UL << ADC_CSR_AWD1_SLV_Pos)         /*!< 0x00800000 */
#define ADC_CSR_AWD1_SLV               ADC_CSR_AWD1_SLV_Msk                    /*!< ADC multimode slave analog watchdog 1 flag */
#define ADC_CSR_AWD2_SLV_Pos           (24U)
#define ADC_CSR_AWD2_SLV_Msk           (0x1UL << ADC_CSR_AWD2_SLV_Pos)         /*!< 0x01000000 */
#define ADC_CSR_AWD2_SLV               ADC_CSR_AWD2_SLV_Msk                    /*!< ADC multimode slave analog watchdog 2 flag */
#define ADC_CSR_AWD3_SLV_Pos           (25U)
#define ADC_CSR_AWD3_SLV_Msk           (0x1UL << ADC_CSR_AWD3_SLV_Pos)         /*!< 0x02000000 */
#define ADC_CSR_AWD3_SLV               ADC_CSR_AWD3_SLV_Msk                    /*!< ADC multimode slave analog watchdog 3 flag */
#define ADC_CSR_JQOVF_SLV_Pos          (26U)
#define ADC_CSR_JQOVF_SLV_Msk          (0x1UL << ADC_CSR_JQOVF_SLV_Pos)        /*!< 0x04000000 */
#define ADC_CSR_JQOVF_SLV              ADC_CSR_JQOVF_SLV_Msk                   /*!< ADC multimode slave group injected contexts queue overflow flag */

/********************  Bit definition for ADC_CCR register  *******************/
#define ADC_CCR_DUAL_Pos               (0U)
#define ADC_CCR_DUAL_Msk               (0x1FUL << ADC_CCR_DUAL_Pos)            /*!< 0x0000001F */
#define ADC_CCR_DUAL                   ADC_CCR_DUAL_Msk                        /*!< ADC multimode mode selection */
#define ADC_CCR_DUAL_0                 (0x01UL << ADC_CCR_DUAL_Pos)            /*!< 0x00000001 */
#define ADC_CCR_DUAL_1                 (0x02UL << ADC_CCR_DUAL_Pos)            /*!< 0x00000002 */
#define ADC_CCR_DUAL_2                 (0x04UL << ADC_CCR_DUAL_Pos)            /*!< 0x00000004 */
#define ADC_CCR_DUAL_3                 (0x08UL << ADC_CCR_DUAL_Pos)            /*!< 0x00000008 */
#define ADC_CCR_DUAL_4                 (0x10UL << ADC_CCR_DUAL_Pos)            /*!< 0x00000010 */

#define ADC_CCR_DELAY_Pos              (8U)
#define ADC_CCR_DELAY_Msk              (0xFUL << ADC_CCR_DELAY_Pos)            /*!< 0x00000F00 */
#define ADC_CCR_DELAY                  ADC_CCR_DELAY_Msk                       /*!< ADC multimode delay between 2 sampling phases */
#define ADC_CCR_DELAY_0                (0x1UL << ADC_CCR_DELAY_Pos)            /*!< 0x00000100 */
#define ADC_CCR_DELAY_1                (0x2UL << ADC_CCR_DELAY_Pos)            /*!< 0x00000200 */
#define ADC_CCR_DELAY_2                (0x4UL << ADC_CCR_DELAY_Pos)            /*!< 0x00000400 */
#define ADC_CCR_DELAY_3                (0x8UL << ADC_CCR_DELAY_Pos)            /*!< 0x00000800 */

#define ADC_CCR_DMACFG_Pos             (13U)
#define ADC_CCR_DMACFG_Msk             (0x1UL << ADC_CCR_DMACFG_Pos)           /*!< 0x00002000 */
#define ADC_CCR_DMACFG                 ADC_CCR_DMACFG_Msk                      /*!< ADC multimode DMA transfer configuration */

#define ADC_CCR_MDMA_Pos               (14U)
#define ADC_CCR_MDMA_Msk               (0x3UL << ADC_CCR_MDMA_Pos)             /*!< 0x0000C000 */
#define ADC_CCR_MDMA                   ADC_CCR_MDMA_Msk                        /*!< ADC multimode DMA transfer enable */
#define ADC_CCR_MDMA_0                 (0x1UL << ADC_CCR_MDMA_Pos)             /*!< 0x00004000 */
#define ADC_CCR_MDMA_1                 (0x2UL << ADC_CCR_MDMA_Pos)             /*!< 0x00008000 */

#define ADC_CCR_CKMODE_Pos             (16U)
#define ADC_CCR_CKMODE_Msk             (0x3UL << ADC_CCR_CKMODE_Pos)           /*!< 0x00030000 */
#define ADC_CCR_CKMODE                 ADC_CCR_CKMODE_Msk                      /*!< ADC common clock source and prescaler (prescaler only for clock source synchronous) */
#define ADC_CCR_CKMODE_0               (0x1UL << ADC_CCR_CKMODE_Pos)           /*!< 0x00010000 */
#define ADC_CCR_CKMODE_1               (0x2UL << ADC_CCR_CKMODE_Pos)           /*!< 0x00020000 */

#define ADC_CCR_PRESC_Pos              (18U)
#define ADC_CCR_PRESC_Msk              (0xFUL << ADC_CCR_PRESC_Pos)            /*!< 0x003C0000 */
#define ADC_CCR_PRESC                  ADC_CCR_PRESC_Msk                       /*!< ADC common clock prescaler, only for clock source asynchronous */
#define ADC_CCR_PRESC_0                (0x1UL << ADC_CCR_PRESC_Pos)            /*!< 0x00040000 */
#define ADC_CCR_PRESC_1                (0x2UL << ADC_CCR_PRESC_Pos)            /*!< 0x00080000 */
#define ADC_CCR_PRESC_2                (0x4UL << ADC_CCR_PRESC_Pos)            /*!< 0x00100000 */
#define ADC_CCR_PRESC_3                (0x8UL << ADC_CCR_PRESC_Pos)            /*!< 0x00200000 */

#define ADC_CCR_VREFEN_Pos             (22U)
#define ADC_CCR_VREFEN_Msk             (0x1UL << ADC_CCR_VREFEN_Pos)           /*!< 0x00400000 */
#define ADC_CCR_VREFEN                 ADC_CCR_VREFEN_Msk                      /*!< ADC internal path to VrefInt enable */
#define ADC_CCR_TSEN_Pos               (23U)
#define ADC_CCR_TSEN_Msk               (0x1UL << ADC_CCR_TSEN_Pos)             /*!< 0x00800000 */
#define ADC_CCR_TSEN                   ADC_CCR_TSEN_Msk                        /*!< ADC internal path to temperature sensor enable */
#define ADC_CCR_VBATEN_Pos             (24U)
#define ADC_CCR_VBATEN_Msk             (0x1UL << ADC_CCR_VBATEN_Pos)           /*!< 0x01000000 */
#define ADC_CCR_VBATEN                 ADC_CCR_VBATEN_Msk                      /*!< ADC internal path to battery voltage enable */

/********************  Bit definition for ADC_CDR register  *******************/
#define ADC_CDR_RDATA_MST_Pos          (0U)
#define ADC_CDR_RDATA_MST_Msk          (0xFFFFUL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x0000FFFF */
#define ADC_CDR_RDATA_MST              ADC_CDR_RDATA_MST_Msk                   /*!< ADC multimode master group regular conversion data */
#define ADC_CDR_RDATA_MST_0            (0x0001UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000001 */
#define ADC_CDR_RDATA_MST_1            (0x0002UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000002 */
#define ADC_CDR_RDATA_MST_2            (0x0004UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000004 */
#define ADC_CDR_RDATA_MST_3            (0x0008UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000008 */
#define ADC_CDR_RDATA_MST_4            (0x0010UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000010 */
#define ADC_CDR_RDATA_MST_5            (0x0020UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000020 */
#define ADC_CDR_RDATA_MST_6            (0x0040UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000040 */
#define ADC_CDR_RDATA_MST_7            (0x0080UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000080 */
#define ADC_CDR_RDATA_MST_8            (0x0100UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000100 */
#define ADC_CDR_RDATA_MST_9            (0x0200UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000200 */
#define ADC_CDR_RDATA_MST_10           (0x0400UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000400 */
#define ADC_CDR_RDATA_MST_11           (0x0800UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00000800 */
#define ADC_CDR_RDATA_MST_12           (0x1000UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00001000 */
#define ADC_CDR_RDATA_MST_13           (0x2000UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00002000 */
#define ADC_CDR_RDATA_MST_14           (0x4000UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00004000 */
#define ADC_CDR_RDATA_MST_15           (0x8000UL << ADC_CDR_RDATA_MST_Pos)     /*!< 0x00008000 */

#define ADC_CDR_RDATA_SLV_Pos          (16U)
#define ADC_CDR_RDATA_SLV_Msk          (0xFFFFUL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0xFFFF0000 */
#define ADC_CDR_RDATA_SLV              ADC_CDR_RDATA_SLV_Msk                   /*!< ADC multimode slave group regular conversion data */
#define ADC_CDR_RDATA_SLV_0            (0x0001UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x00010000 */
#define ADC_CDR_RDATA_SLV_1            (0x0002UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x00020000 */
#define ADC_CDR_RDATA_SLV_2            (0x0004UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x00040000 */
#define ADC_CDR_RDATA_SLV_3            (0x0008UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x00080000 */
#define ADC_CDR_RDATA_SLV_4            (0x0010UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x00100000 */
#define ADC_CDR_RDATA_SLV_5            (0x0020UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x00200000 */
#define ADC_CDR_RDATA_SLV_6            (0x0040UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x00400000 */
#define ADC_CDR_RDATA_SLV_7            (0x0080UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x00800000 */
#define ADC_CDR_RDATA_SLV_8            (0x0100UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x01000000 */
#define ADC_CDR_RDATA_SLV_9            (0x0200UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x02000000 */
#define ADC_CDR_RDATA_SLV_10           (0x0400UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x04000000 */
#define ADC_CDR_RDATA_SLV_11           (0x0800UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x08000000 */
#define ADC_CDR_RDATA_SLV_12           (0x1000UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x10000000 */
#define ADC_CDR_RDATA_SLV_13           (0x2000UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x20000000 */
#define ADC_CDR_RDATA_SLV_14           (0x4000UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x40000000 */
#define ADC_CDR_RDATA_SLV_15           (0x8000UL << ADC_CDR_RDATA_SLV_Pos)     /*!< 0x80000000 */


/******************************************************************************/
/*                                                                            */
/*                          CRC calculation unit                              */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for CRC_DR register  *********************/
#define CRC_DR_DR_Pos            (0U)
#define CRC_DR_DR_Msk            (0xFFFFFFFFUL << CRC_DR_DR_Pos)               /*!< 0xFFFFFFFF */
#define CRC_DR_DR                CRC_DR_DR_Msk                                 /*!< Data register bits */

/*******************  Bit definition for CRC_IDR register  ********************/
#define CRC_IDR_IDR_Pos          (0U)
#define CRC_IDR_IDR_Msk          (0xFFU << CRC_IDR_IDR_Pos)                    /*!< 0x000000FF */
#define CRC_IDR_IDR              CRC_IDR_IDR_Msk                               /*!< General-purpose 8-bit data register bits */

/********************  Bit definition for CRC_CR register  ********************/
#define CRC_CR_RESET_Pos         (0U)
#define CRC_CR_RESET_Msk         (0x1UL << CRC_CR_RESET_Pos)                   /*!< 0x00000001 */
#define CRC_CR_RESET             CRC_CR_RESET_Msk                              /*!< RESET the CRC computation unit bit */
#define CRC_CR_POLYSIZE_Pos      (3U)
#define CRC_CR_POLYSIZE_Msk      (0x3UL << CRC_CR_POLYSIZE_Pos)                /*!< 0x00000018 */
#define CRC_CR_POLYSIZE          CRC_CR_POLYSIZE_Msk                           /*!< Polynomial size bits */
#define CRC_CR_POLYSIZE_0        (0x1UL << CRC_CR_POLYSIZE_Pos)                /*!< 0x00000008 */
#define CRC_CR_POLYSIZE_1        (0x2UL << CRC_CR_POLYSIZE_Pos)                /*!< 0x00000010 */
#define CRC_CR_REV_IN_Pos        (5U)
#define CRC_CR_REV_IN_Msk        (0x3UL << CRC_CR_REV_IN_Pos)                  /*!< 0x00000060 */
#define CRC_CR_REV_IN            CRC_CR_REV_IN_Msk                             /*!< REV_IN Reverse Input Data bits */
#define CRC_CR_REV_IN_0          (0x1UL << CRC_CR_REV_IN_Pos)                  /*!< 0x00000020 */
#define CRC_CR_REV_IN_1          (0x2UL << CRC_CR_REV_IN_Pos)                  /*!< 0x00000040 */
#define CRC_CR_REV_OUT_Pos       (7U)
#define CRC_CR_REV_OUT_Msk       (0x1UL << CRC_CR_REV_OUT_Pos)                 /*!< 0x00000080 */
#define CRC_CR_REV_OUT           CRC_CR_REV_OUT_Msk                            /*!< REV_OUT Reverse Output Data bits */

/*******************  Bit definition for CRC_INIT register  *******************/
#define CRC_INIT_INIT_Pos        (0U)
#define CRC_INIT_INIT_Msk        (0xFFFFFFFFUL << CRC_INIT_INIT_Pos)           /*!< 0xFFFFFFFF */
#define CRC_INIT_INIT            CRC_INIT_INIT_Msk                             /*!< Initial CRC value bits */

/*******************  Bit definition for CRC_POL register  ********************/
#define CRC_POL_POL_Pos          (0U)
#define CRC_POL_POL_Msk          (0xFFFFFFFFUL << CRC_POL_POL_Pos)             /*!< 0xFFFFFFFF */
#define CRC_POL_POL              CRC_POL_POL_Msk                               /*!< Coefficients of the polynomial */

/******************************************************************************/
/*                                                                            */
/*                          CRS Clock Recovery System                         */
/******************************************************************************/

/*******************  Bit definition for CRS_CR register  *********************/
#define CRS_CR_SYNCOKIE_Pos       (0U)
#define CRS_CR_SYNCOKIE_Msk       (0x1UL << CRS_CR_SYNCOKIE_Pos)               /*!< 0x00000001 */
#define CRS_CR_SYNCOKIE           CRS_CR_SYNCOKIE_Msk                          /*!< SYNC event OK interrupt enable */
#define CRS_CR_SYNCWARNIE_Pos     (1U)
#define CRS_CR_SYNCWARNIE_Msk     (0x1UL << CRS_CR_SYNCWARNIE_Pos)             /*!< 0x00000002 */
#define CRS_CR_SYNCWARNIE         CRS_CR_SYNCWARNIE_Msk                        /*!< SYNC warning interrupt enable */
#define CRS_CR_ERRIE_Pos          (2U)
#define CRS_CR_ERRIE_Msk          (0x1UL << CRS_CR_ERRIE_Pos)                  /*!< 0x00000004 */
#define CRS_CR_ERRIE              CRS_CR_ERRIE_Msk                             /*!< SYNC error or trimming error interrupt enable */
#define CRS_CR_ESYNCIE_Pos        (3U)
#define CRS_CR_ESYNCIE_Msk        (0x1UL << CRS_CR_ESYNCIE_Pos)                /*!< 0x00000008 */
#define CRS_CR_ESYNCIE            CRS_CR_ESYNCIE_Msk                           /*!< Expected SYNC interrupt enable */
#define CRS_CR_CEN_Pos            (5U)
#define CRS_CR_CEN_Msk            (0x1UL << CRS_CR_CEN_Pos)                    /*!< 0x00000020 */
#define CRS_CR_CEN                CRS_CR_CEN_Msk                               /*!< Frequency error counter enable */
#define CRS_CR_AUTOTRIMEN_Pos     (6U)
#define CRS_CR_AUTOTRIMEN_Msk     (0x1UL << CRS_CR_AUTOTRIMEN_Pos)             /*!< 0x00000040 */
#define CRS_CR_AUTOTRIMEN         CRS_CR_AUTOTRIMEN_Msk                        /*!< Automatic trimming enable */
#define CRS_CR_SWSYNC_Pos         (7U)
#define CRS_CR_SWSYNC_Msk         (0x1UL << CRS_CR_SWSYNC_Pos)                 /*!< 0x00000080 */
#define CRS_CR_SWSYNC             CRS_CR_SWSYNC_Msk                            /*!< Generate software SYNC event */
#define CRS_CR_TRIM_Pos           (8U)
#define CRS_CR_TRIM_Msk           (0x7FUL << CRS_CR_TRIM_Pos)                  /*!< 0x00007F00 */
#define CRS_CR_TRIM               CRS_CR_TRIM_Msk                              /*!< HSI48 oscillator smooth trimming */

/*******************  Bit definition for CRS_CFGR register  *********************/
#define CRS_CFGR_RELOAD_Pos       (0U)
#define CRS_CFGR_RELOAD_Msk       (0xFFFFUL << CRS_CFGR_RELOAD_Pos)            /*!< 0x0000FFFF */
#define CRS_CFGR_RELOAD           CRS_CFGR_RELOAD_Msk                          /*!< Counter reload value */
#define CRS_CFGR_FELIM_Pos        (16U)
#define CRS_CFGR_FELIM_Msk        (0xFFUL << CRS_CFGR_FELIM_Pos)               /*!< 0x00FF0000 */
#define CRS_CFGR_FELIM            CRS_CFGR_FELIM_Msk                           /*!< Frequency error limit */

#define CRS_CFGR_SYNCDIV_Pos      (24U)
#define CRS_CFGR_SYNCDIV_Msk      (0x7UL << CRS_CFGR_SYNCDIV_Pos)              /*!< 0x07000000 */
#define CRS_CFGR_SYNCDIV          CRS_CFGR_SYNCDIV_Msk                         /*!< SYNC divider */
#define CRS_CFGR_SYNCDIV_0        (0x1UL << CRS_CFGR_SYNCDIV_Pos)              /*!< 0x01000000 */
#define CRS_CFGR_SYNCDIV_1        (0x2UL << CRS_CFGR_SYNCDIV_Pos)              /*!< 0x02000000 */
#define CRS_CFGR_SYNCDIV_2        (0x4UL << CRS_CFGR_SYNCDIV_Pos)              /*!< 0x04000000 */

#define CRS_CFGR_SYNCSRC_Pos      (28U)
#define CRS_CFGR_SYNCSRC_Msk      (0x3UL << CRS_CFGR_SYNCSRC_Pos)              /*!< 0x30000000 */
#define CRS_CFGR_SYNCSRC          CRS_CFGR_SYNCSRC_Msk                         /*!< SYNC signal source selection */
#define CRS_CFGR_SYNCSRC_0        (0x1UL << CRS_CFGR_SYNCSRC_Pos)              /*!< 0x10000000 */
#define CRS_CFGR_SYNCSRC_1        (0x2UL << CRS_CFGR_SYNCSRC_Pos)              /*!< 0x20000000 */

#define CRS_CFGR_SYNCPOL_Pos      (31U)
#define CRS_CFGR_SYNCPOL_Msk      (0x1UL << CRS_CFGR_SYNCPOL_Pos)              /*!< 0x80000000 */
#define CRS_CFGR_SYNCPOL          CRS_CFGR_SYNCPOL_Msk                         /*!< SYNC polarity selection */

/*******************  Bit definition for CRS_ISR register  *********************/
#define CRS_ISR_SYNCOKF_Pos       (0U)
#define CRS_ISR_SYNCOKF_Msk       (0x1UL << CRS_ISR_SYNCOKF_Pos)               /*!< 0x00000001 */
#define CRS_ISR_SYNCOKF           CRS_ISR_SYNCOKF_Msk                          /*!< SYNC event OK flag */
#define CRS_ISR_SYNCWARNF_Pos     (1U)
#define CRS_ISR_SYNCWARNF_Msk     (0x1UL << CRS_ISR_SYNCWARNF_Pos)             /*!< 0x00000002 */
#define CRS_ISR_SYNCWARNF         CRS_ISR_SYNCWARNF_Msk                        /*!< SYNC warning flag */
#define CRS_ISR_ERRF_Pos          (2U)
#define CRS_ISR_ERRF_Msk          (0x1UL << CRS_ISR_ERRF_Pos)                  /*!< 0x00000004 */
#define CRS_ISR_ERRF              CRS_ISR_ERRF_Msk                             /*!< Error flag */
#define CRS_ISR_ESYNCF_Pos        (3U)
#define CRS_ISR_ESYNCF_Msk        (0x1UL << CRS_ISR_ESYNCF_Pos)                /*!< 0x00000008 */
#define CRS_ISR_ESYNCF            CRS_ISR_ESYNCF_Msk                           /*!< Expected SYNC flag */
#define CRS_ISR_SYNCERR_Pos       (8U)
#define CRS_ISR_SYNCERR_Msk       (0x1UL << CRS_ISR_SYNCERR_Pos)               /*!< 0x00000100 */
#define CRS_ISR_SYNCERR           CRS_ISR_SYNCERR_Msk                          /*!< SYNC error */
#define CRS_ISR_SYNCMISS_Pos      (9U)
#define CRS_ISR_SYNCMISS_Msk      (0x1UL << CRS_ISR_SYNCMISS_Pos)              /*!< 0x00000200 */
#define CRS_ISR_SYNCMISS          CRS_ISR_SYNCMISS_Msk                         /*!< SYNC missed */
#define CRS_ISR_TRIMOVF_Pos       (10U)
#define CRS_ISR_TRIMOVF_Msk       (0x1UL << CRS_ISR_TRIMOVF_Pos)               /*!< 0x00000400 */
#define CRS_ISR_TRIMOVF           CRS_ISR_TRIMOVF_Msk                          /*!< Trimming overflow or underflow */
#define CRS_ISR_FEDIR_Pos         (15U)
#define CRS_ISR_FEDIR_Msk         (0x1UL << CRS_ISR_FEDIR_Pos)                 /*!< 0x00008000 */
#define CRS_ISR_FEDIR             CRS_ISR_FEDIR_Msk                            /*!< Frequency error direction */
#define CRS_ISR_FECAP_Pos         (16U)
#define CRS_ISR_FECAP_Msk         (0xFFFFUL << CRS_ISR_FECAP_Pos)              /*!< 0xFFFF0000 */
#define CRS_ISR_FECAP             CRS_ISR_FECAP_Msk                            /*!< Frequency error capture */

/*******************  Bit definition for CRS_ICR register  *********************/
#define CRS_ICR_SYNCOKC_Pos       (0U)
#define CRS_ICR_SYNCOKC_Msk       (0x1UL << CRS_ICR_SYNCOKC_Pos)               /*!< 0x00000001 */
#define CRS_ICR_SYNCOKC           CRS_ICR_SYNCOKC_Msk                          /*!< SYNC event OK clear flag */
#define CRS_ICR_SYNCWARNC_Pos     (1U)
#define CRS_ICR_SYNCWARNC_Msk     (0x1UL << CRS_ICR_SYNCWARNC_Pos)             /*!< 0x00000002 */
#define CRS_ICR_SYNCWARNC         CRS_ICR_SYNCWARNC_Msk                        /*!< SYNC warning clear flag */
#define CRS_ICR_ERRC_Pos          (2U)
#define CRS_ICR_ERRC_Msk          (0x1UL << CRS_ICR_ERRC_Pos)                  /*!< 0x00000004 */
#define CRS_ICR_ERRC              CRS_ICR_ERRC_Msk                             /*!< Error clear flag */
#define CRS_ICR_ESYNCC_Pos        (3U)
#define CRS_ICR_ESYNCC_Msk        (0x1UL << CRS_ICR_ESYNCC_Pos)                /*!< 0x00000008 */
#define CRS_ICR_ESYNCC            CRS_ICR_ESYNCC_Msk                           /*!< Expected SYNC clear flag */

/******************************************************************************/
/*                                                                            */
/*                       Advanced Encryption Standard (AES)                   */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for AES_CR register  *********************/
#define AES_CR_EN_Pos            (0U)
#define AES_CR_EN_Msk            (0x1UL << AES_CR_EN_Pos)                      /*!< 0x00000001 */
#define AES_CR_EN                AES_CR_EN_Msk                                 /*!< AES Enable */
#define AES_CR_DATATYPE_Pos      (1U)
#define AES_CR_DATATYPE_Msk      (0x3UL << AES_CR_DATATYPE_Pos)                /*!< 0x00000006 */
#define AES_CR_DATATYPE          AES_CR_DATATYPE_Msk                           /*!< Data type selection */
#define AES_CR_DATATYPE_0        (0x1UL << AES_CR_DATATYPE_Pos)                /*!< 0x00000002 */
#define AES_CR_DATATYPE_1        (0x2UL << AES_CR_DATATYPE_Pos)                /*!< 0x00000004 */

#define AES_CR_MODE_Pos          (3U)
#define AES_CR_MODE_Msk          (0x3UL << AES_CR_MODE_Pos)                    /*!< 0x00000018 */
#define AES_CR_MODE              AES_CR_MODE_Msk                               /*!< AES Mode Of Operation */
#define AES_CR_MODE_0            (0x1UL << AES_CR_MODE_Pos)                    /*!< 0x00000008 */
#define AES_CR_MODE_1            (0x2UL << AES_CR_MODE_Pos)                    /*!< 0x00000010 */

#define AES_CR_CHMOD_Pos         (5U)
#define AES_CR_CHMOD_Msk         (0x803UL << AES_CR_CHMOD_Pos)                 /*!< 0x00010060 */
#define AES_CR_CHMOD             AES_CR_CHMOD_Msk                              /*!< AES Chaining Mode */
#define AES_CR_CHMOD_0           (0x001UL << AES_CR_CHMOD_Pos)                 /*!< 0x00000020 */
#define AES_CR_CHMOD_1           (0x002UL << AES_CR_CHMOD_Pos)                 /*!< 0x00000040 */
#define AES_CR_CHMOD_2           (0x800UL << AES_CR_CHMOD_Pos)                 /*!< 0x00010000 */

#define AES_CR_CCFC_Pos          (7U)
#define AES_CR_CCFC_Msk          (0x1UL << AES_CR_CCFC_Pos)                    /*!< 0x00000080 */
#define AES_CR_CCFC              AES_CR_CCFC_Msk                               /*!< Computation Complete Flag Clear */
#define AES_CR_ERRC_Pos          (8U)
#define AES_CR_ERRC_Msk          (0x1UL << AES_CR_ERRC_Pos)                    /*!< 0x00000100 */
#define AES_CR_ERRC              AES_CR_ERRC_Msk                               /*!< Error Clear */
#define AES_CR_CCFIE_Pos         (9U)
#define AES_CR_CCFIE_Msk         (0x1UL << AES_CR_CCFIE_Pos)                   /*!< 0x00000200 */
#define AES_CR_CCFIE             AES_CR_CCFIE_Msk                              /*!< Computation Complete Flag Interrupt Enable */
#define AES_CR_ERRIE_Pos         (10U)
#define AES_CR_ERRIE_Msk         (0x1UL << AES_CR_ERRIE_Pos)                   /*!< 0x00000400 */
#define AES_CR_ERRIE             AES_CR_ERRIE_Msk                              /*!< Error Interrupt Enable */
#define AES_CR_DMAINEN_Pos       (11U)
#define AES_CR_DMAINEN_Msk       (0x1UL << AES_CR_DMAINEN_Pos)                 /*!< 0x00000800 */
#define AES_CR_DMAINEN           AES_CR_DMAINEN_Msk                            /*!< Enable data input phase DMA management  */
#define AES_CR_DMAOUTEN_Pos      (12U)
#define AES_CR_DMAOUTEN_Msk      (0x1UL << AES_CR_DMAOUTEN_Pos)                /*!< 0x00001000 */
#define AES_CR_DMAOUTEN          AES_CR_DMAOUTEN_Msk                           /*!< Enable data output phase DMA management */

#define AES_CR_GCMPH_Pos         (13U)
#define AES_CR_GCMPH_Msk         (0x3UL << AES_CR_GCMPH_Pos)                   /*!< 0x00006000 */
#define AES_CR_GCMPH             AES_CR_GCMPH_Msk                              /*!< GCM Phase */
#define AES_CR_GCMPH_0           (0x1UL << AES_CR_GCMPH_Pos)                   /*!< 0x00002000 */
#define AES_CR_GCMPH_1           (0x2UL << AES_CR_GCMPH_Pos)                   /*!< 0x00004000 */

#define AES_CR_KEYSIZE_Pos       (18U)
#define AES_CR_KEYSIZE_Msk       (0x1UL << AES_CR_KEYSIZE_Pos)                 /*!< 0x00040000 */
#define AES_CR_KEYSIZE           AES_CR_KEYSIZE_Msk                            /*!< Key size selection */

/*******************  Bit definition for AES_SR register  *********************/
#define AES_SR_CCF_Pos           (0U)
#define AES_SR_CCF_Msk           (0x1UL << AES_SR_CCF_Pos)                     /*!< 0x00000001 */
#define AES_SR_CCF               AES_SR_CCF_Msk                                /*!< Computation Complete Flag */
#define AES_SR_RDERR_Pos         (1U)
#define AES_SR_RDERR_Msk         (0x1UL << AES_SR_RDERR_Pos)                   /*!< 0x00000002 */
#define AES_SR_RDERR             AES_SR_RDERR_Msk                              /*!< Read Error Flag */
#define AES_SR_WRERR_Pos         (2U)
#define AES_SR_WRERR_Msk         (0x1UL << AES_SR_WRERR_Pos)                   /*!< 0x00000004 */
#define AES_SR_WRERR             AES_SR_WRERR_Msk                              /*!< Write Error Flag */
#define AES_SR_BUSY_Pos          (3U)
#define AES_SR_BUSY_Msk          (0x1UL << AES_SR_BUSY_Pos)                    /*!< 0x00000008 */
#define AES_SR_BUSY              AES_SR_BUSY_Msk                               /*!< Busy Flag */

/*******************  Bit definition for AES_DINR register  *******************/
#define AES_DINR_Pos             (0U)
#define AES_DINR_Msk             (0xFFFFFFFFUL << AES_DINR_Pos)                /*!< 0xFFFFFFFF */
#define AES_DINR                 AES_DINR_Msk                                  /*!< AES Data Input Register */

/*******************  Bit definition for AES_DOUTR register  ******************/
#define AES_DOUTR_Pos            (0U)
#define AES_DOUTR_Msk            (0xFFFFFFFFUL << AES_DOUTR_Pos)               /*!< 0xFFFFFFFF */
#define AES_DOUTR                AES_DOUTR_Msk                                 /*!< AES Data Output Register */

/*******************  Bit definition for AES_KEYR0 register  ******************/
#define AES_KEYR0_Pos            (0U)
#define AES_KEYR0_Msk            (0xFFFFFFFFUL << AES_KEYR0_Pos)               /*!< 0xFFFFFFFF */
#define AES_KEYR0                AES_KEYR0_Msk                                 /*!< AES Key Register 0 */

/*******************  Bit definition for AES_KEYR1 register  ******************/
#define AES_KEYR1_Pos            (0U)
#define AES_KEYR1_Msk            (0xFFFFFFFFUL << AES_KEYR1_Pos)               /*!< 0xFFFFFFFF */
#define AES_KEYR1                AES_KEYR1_Msk                                 /*!< AES Key Register 1 */

/*******************  Bit definition for AES_KEYR2 register  ******************/
#define AES_KEYR2_Pos            (0U)
#define AES_KEYR2_Msk            (0xFFFFFFFFUL << AES_KEYR2_Pos)               /*!< 0xFFFFFFFF */
#define AES_KEYR2                AES_KEYR2_Msk                                 /*!< AES Key Register 2 */

/*******************  Bit definition for AES_KEYR3 register  ******************/
#define AES_KEYR3_Pos            (0U)
#define AES_KEYR3_Msk            (0xFFFFFFFFUL << AES_KEYR3_Pos)               /*!< 0xFFFFFFFF */
#define AES_KEYR3                AES_KEYR3_Msk                                 /*!< AES Key Register 3 */

/*******************  Bit definition for AES_KEYR4 register  ******************/
#define AES_KEYR4_Pos            (0U)
#define AES_KEYR4_Msk            (0xFFFFFFFFUL << AES_KEYR4_Pos)               /*!< 0xFFFFFFFF */
#define AES_KEYR4                AES_KEYR4_Msk                                 /*!< AES Key Register 4 */

/*******************  Bit definition for AES_KEYR5 register  ******************/
#define AES_KEYR5_Pos            (0U)
#define AES_KEYR5_Msk            (0xFFFFFFFFUL << AES_KEYR5_Pos)               /*!< 0xFFFFFFFF */
#define AES_KEYR5                AES_KEYR5_Msk                                 /*!< AES Key Register 5 */

/*******************  Bit definition for AES_KEYR6 register  ******************/
#define AES_KEYR6_Pos            (0U)
#define AES_KEYR6_Msk            (0xFFFFFFFFUL << AES_KEYR6_Pos)               /*!< 0xFFFFFFFF */
#define AES_KEYR6                AES_KEYR6_Msk                                 /*!< AES Key Register 6 */

/*******************  Bit definition for AES_KEYR7 register  ******************/
#define AES_KEYR7_Pos            (0U)
#define AES_KEYR7_Msk            (0xFFFFFFFFUL << AES_KEYR7_Pos)               /*!< 0xFFFFFFFF */
#define AES_KEYR7                AES_KEYR7_Msk                                 /*!< AES Key Register 7 */

/*******************  Bit definition for AES_IVR0 register   ******************/
#define AES_IVR0_Pos             (0U)
#define AES_IVR0_Msk             (0xFFFFFFFFUL << AES_IVR0_Pos)                /*!< 0xFFFFFFFF */
#define AES_IVR0                 AES_IVR0_Msk                                  /*!< AES Initialization Vector Register 0 */

/*******************  Bit definition for AES_IVR1 register   ******************/
#define AES_IVR1_Pos             (0U)
#define AES_IVR1_Msk             (0xFFFFFFFFUL << AES_IVR1_Pos)                /*!< 0xFFFFFFFF */
#define AES_IVR1                 AES_IVR1_Msk                                  /*!< AES Initialization Vector Register 1 */

/*******************  Bit definition for AES_IVR2 register   ******************/
#define AES_IVR2_Pos             (0U)
#define AES_IVR2_Msk             (0xFFFFFFFFUL << AES_IVR2_Pos)                /*!< 0xFFFFFFFF */
#define AES_IVR2                 AES_IVR2_Msk                                  /*!< AES Initialization Vector Register 2 */

/*******************  Bit definition for AES_IVR3 register   ******************/
#define AES_IVR3_Pos             (0U)
#define AES_IVR3_Msk             (0xFFFFFFFFUL << AES_IVR3_Pos)                /*!< 0xFFFFFFFF */
#define AES_IVR3                 AES_IVR3_Msk                                  /*!< AES Initialization Vector Register 3 */

/*******************  Bit definition for AES_SUSP0R register  ******************/
#define AES_SUSP0R_Pos           (0U)
#define AES_SUSP0R_Msk           (0xFFFFFFFFUL << AES_SUSP0R_Pos)              /*!< 0xFFFFFFFF */
#define AES_SUSP0R               AES_SUSP0R_Msk                                /*!< AES Suspend registers 0 */

/*******************  Bit definition for AES_SUSP1R register  ******************/
#define AES_SUSP1R_Pos           (0U)
#define AES_SUSP1R_Msk           (0xFFFFFFFFUL << AES_SUSP1R_Pos)              /*!< 0xFFFFFFFF */
#define AES_SUSP1R               AES_SUSP1R_Msk                                /*!< AES Suspend registers 1 */

/*******************  Bit definition for AES_SUSP2R register  ******************/
#define AES_SUSP2R_Pos           (0U)
#define AES_SUSP2R_Msk           (0xFFFFFFFFUL << AES_SUSP2R_Pos)              /*!< 0xFFFFFFFF */
#define AES_SUSP2R               AES_SUSP2R_Msk                                /*!< AES Suspend registers 2 */

/*******************  Bit definition for AES_SUSP3R register  ******************/
#define AES_SUSP3R_Pos           (0U)
#define AES_SUSP3R_Msk           (0xFFFFFFFFUL << AES_SUSP3R_Pos)              /*!< 0xFFFFFFFF */
#define AES_SUSP3R               AES_SUSP3R_Msk                                /*!< AES Suspend registers 3 */

/*******************  Bit definition for AES_SUSP4R register  ******************/
#define AES_SUSP4R_Pos           (0U)
#define AES_SUSP4R_Msk           (0xFFFFFFFFUL << AES_SUSP4R_Pos)              /*!< 0xFFFFFFFF */
#define AES_SUSP4R               AES_SUSP4R_Msk                                /*!< AES Suspend registers 4 */

/*******************  Bit definition for AES_SUSP5R register  ******************/
#define AES_SUSP5R_Pos           (0U)
#define AES_SUSP5R_Msk           (0xFFFFFFFFUL << AES_SUSP5R_Pos)              /*!< 0xFFFFFFFF */
#define AES_SUSP5R               AES_SUSP5R_Msk                                /*!< AES Suspend registers 5 */

/*******************  Bit definition for AES_SUSP6R register  ******************/
#define AES_SUSP6R_Pos           (0U)
#define AES_SUSP6R_Msk           (0xFFFFFFFFUL << AES_SUSP6R_Pos)              /*!< 0xFFFFFFFF */
#define AES_SUSP6R               AES_SUSP6R_Msk                                /*!< AES Suspend registers 6 */

/*******************  Bit definition for AES_SUSP7R register  ******************/
#define AES_SUSP7R_Pos           (0U)
#define AES_SUSP7R_Msk           (0xFFFFFFFFUL << AES_SUSP7R_Pos)              /*!< 0xFFFFFFFF */
#define AES_SUSP7R               AES_SUSP7R_Msk                                /*!< AES Suspend registers 7 */


/******************************************************************************/
/*                                                                            */
/*                           DMA Controller (DMA)                             */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for DMA_ISR register  ********************/
#define DMA_ISR_GIF1_Pos       (0U)
#define DMA_ISR_GIF1_Msk       (0x1UL << DMA_ISR_GIF1_Pos)                     /*!< 0x00000001 */
#define DMA_ISR_GIF1           DMA_ISR_GIF1_Msk                                /*!< Channel 1 Global interrupt flag */
#define DMA_ISR_TCIF1_Pos      (1U)
#define DMA_ISR_TCIF1_Msk      (0x1UL << DMA_ISR_TCIF1_Pos)                    /*!< 0x00000002 */
#define DMA_ISR_TCIF1          DMA_ISR_TCIF1_Msk                               /*!< Channel 1 Transfer Complete flag */
#define DMA_ISR_HTIF1_Pos      (2U)
#define DMA_ISR_HTIF1_Msk      (0x1UL << DMA_ISR_HTIF1_Pos)                    /*!< 0x00000004 */
#define DMA_ISR_HTIF1          DMA_ISR_HTIF1_Msk                               /*!< Channel 1 Half Transfer flag */
#define DMA_ISR_TEIF1_Pos      (3U)
#define DMA_ISR_TEIF1_Msk      (0x1UL << DMA_ISR_TEIF1_Pos)                    /*!< 0x00000008 */
#define DMA_ISR_TEIF1          DMA_ISR_TEIF1_Msk                               /*!< Channel 1 Transfer Error flag */
#define DMA_ISR_GIF2_Pos       (4U)
#define DMA_ISR_GIF2_Msk       (0x1UL << DMA_ISR_GIF2_Pos)                     /*!< 0x00000010 */
#define DMA_ISR_GIF2           DMA_ISR_GIF2_Msk                                /*!< Channel 2 Global interrupt flag */
#define DMA_ISR_TCIF2_Pos      (5U)
#define DMA_ISR_TCIF2_Msk      (0x1UL << DMA_ISR_TCIF2_Pos)                    /*!< 0x00000020 */
#define DMA_ISR_TCIF2          DMA_ISR_TCIF2_Msk                               /*!< Channel 2 Transfer Complete flag */
#define DMA_ISR_HTIF2_Pos      (6U)
#define DMA_ISR_HTIF2_Msk      (0x1UL << DMA_ISR_HTIF2_Pos)                    /*!< 0x00000040 */
#define DMA_ISR_HTIF2          DMA_ISR_HTIF2_Msk                               /*!< Channel 2 Half Transfer flag */
#define DMA_ISR_TEIF2_Pos      (7U)
#define DMA_ISR_TEIF2_Msk      (0x1UL << DMA_ISR_TEIF2_Pos)                    /*!< 0x00000080 */
#define DMA_ISR_TEIF2          DMA_ISR_TEIF2_Msk                               /*!< Channel 2 Transfer Error flag */
#define DMA_ISR_GIF3_Pos       (8U)
#define DMA_ISR_GIF3_Msk       (0x1UL << DMA_ISR_GIF3_Pos)                     /*!< 0x00000100 */
#define DMA_ISR_GIF3           DMA_ISR_GIF3_Msk                                /*!< Channel 3 Global interrupt flag */
#define DMA_ISR_TCIF3_Pos      (9U)
#define DMA_ISR_TCIF3_Msk      (0x1UL << DMA_ISR_TCIF3_Pos)                    /*!< 0x00000200 */
#define DMA_ISR_TCIF3          DMA_ISR_TCIF3_Msk                               /*!< Channel 3 Transfer Complete flag */
#define DMA_ISR_HTIF3_Pos      (10U)
#define DMA_ISR_HTIF3_Msk      (0x1UL << DMA_ISR_HTIF3_Pos)                    /*!< 0x00000400 */
#define DMA_ISR_HTIF3          DMA_ISR_HTIF3_Msk                               /*!< Channel 3 Half Transfer flag */
#define DMA_ISR_TEIF3_Pos      (11U)
#define DMA_ISR_TEIF3_Msk      (0x1UL << DMA_ISR_TEIF3_Pos)                    /*!< 0x00000800 */
#define DMA_ISR_TEIF3          DMA_ISR_TEIF3_Msk                               /*!< Channel 3 Transfer Error flag */
#define DMA_ISR_GIF4_Pos       (12U)
#define DMA_ISR_GIF4_Msk       (0x1UL << DMA_ISR_GIF4_Pos)                     /*!< 0x00001000 */
#define DMA_ISR_GIF4           DMA_ISR_GIF4_Msk                                /*!< Channel 4 Global interrupt flag */
#define DMA_ISR_TCIF4_Pos      (13U)
#define DMA_ISR_TCIF4_Msk      (0x1UL << DMA_ISR_TCIF4_Pos)                    /*!< 0x00002000 */
#define DMA_ISR_TCIF4          DMA_ISR_TCIF4_Msk                               /*!< Channel 4 Transfer Complete flag */
#define DMA_ISR_HTIF4_Pos      (14U)
#define DMA_ISR_HTIF4_Msk      (0x1UL << DMA_ISR_HTIF4_Pos)                    /*!< 0x00004000 */
#define DMA_ISR_HTIF4          DMA_ISR_HTIF4_Msk                               /*!< Channel 4 Half Transfer flag */
#define DMA_ISR_TEIF4_Pos      (15U)
#define DMA_ISR_TEIF4_Msk      (0x1UL << DMA_ISR_TEIF4_Pos)                    /*!< 0x00008000 */
#define DMA_ISR_TEIF4          DMA_ISR_TEIF4_Msk                               /*!< Channel 4 Transfer Error flag */
#define DMA_ISR_GIF5_Pos       (16U)
#define DMA_ISR_GIF5_Msk       (0x1UL << DMA_ISR_GIF5_Pos)                     /*!< 0x00010000 */
#define DMA_ISR_GIF5           DMA_ISR_GIF5_Msk                                /*!< Channel 5 Global interrupt flag */
#define DMA_ISR_TCIF5_Pos      (17U)
#define DMA_ISR_TCIF5_Msk      (0x1UL << DMA_ISR_TCIF5_Pos)                    /*!< 0x00020000 */
#define DMA_ISR_TCIF5          DMA_ISR_TCIF5_Msk                               /*!< Channel 5 Transfer Complete flag */
#define DMA_ISR_HTIF5_Pos      (18U)
#define DMA_ISR_HTIF5_Msk      (0x1UL << DMA_ISR_HTIF5_Pos)                    /*!< 0x00040000 */
#define DMA_ISR_HTIF5          DMA_ISR_HTIF5_Msk                               /*!< Channel 5 Half Transfer flag */
#define DMA_ISR_TEIF5_Pos      (19U)
#define DMA_ISR_TEIF5_Msk      (0x1UL << DMA_ISR_TEIF5_Pos)                    /*!< 0x00080000 */
#define DMA_ISR_TEIF5          DMA_ISR_TEIF5_Msk                               /*!< Channel 5 Transfer Error flag */
#define DMA_ISR_GIF6_Pos       (20U)
#define DMA_ISR_GIF6_Msk       (0x1UL << DMA_ISR_GIF6_Pos)                     /*!< 0x00100000 */
#define DMA_ISR_GIF6           DMA_ISR_GIF6_Msk                                /*!< Channel 6 Global interrupt flag */
#define DMA_ISR_TCIF6_Pos      (21U)
#define DMA_ISR_TCIF6_Msk      (0x1UL << DMA_ISR_TCIF6_Pos)                    /*!< 0x00200000 */
#define DMA_ISR_TCIF6          DMA_ISR_TCIF6_Msk                               /*!< Channel 6 Transfer Complete flag */
#define DMA_ISR_HTIF6_Pos      (22U)
#define DMA_ISR_HTIF6_Msk      (0x1UL << DMA_ISR_HTIF6_Pos)                    /*!< 0x00400000 */
#define DMA_ISR_HTIF6          DMA_ISR_HTIF6_Msk                               /*!< Channel 6 Half Transfer flag */
#define DMA_ISR_TEIF6_Pos      (23U)
#define DMA_ISR_TEIF6_Msk      (0x1UL << DMA_ISR_TEIF6_Pos)                    /*!< 0x00800000 */
#define DMA_ISR_TEIF6          DMA_ISR_TEIF6_Msk                               /*!< Channel 6 Transfer Error flag */
#define DMA_ISR_GIF7_Pos       (24U)
#define DMA_ISR_GIF7_Msk       (0x1UL << DMA_ISR_GIF7_Pos)                     /*!< 0x01000000 */
#define DMA_ISR_GIF7           DMA_ISR_GIF7_Msk                                /*!< Channel 7 Global interrupt flag */
#define DMA_ISR_TCIF7_Pos      (25U)
#define DMA_ISR_TCIF7_Msk      (0x1UL << DMA_ISR_TCIF7_Pos)                    /*!< 0x02000000 */
#define DMA_ISR_TCIF7          DMA_ISR_TCIF7_Msk                               /*!< Channel 7 Transfer Complete flag */
#define DMA_ISR_HTIF7_Pos      (26U)
#define DMA_ISR_HTIF7_Msk      (0x1UL << DMA_ISR_HTIF7_Pos)                    /*!< 0x04000000 */
#define DMA_ISR_HTIF7          DMA_ISR_HTIF7_Msk                               /*!< Channel 7 Half Transfer flag */
#define DMA_ISR_TEIF7_Pos      (27U)
#define DMA_ISR_TEIF7_Msk      (0x1UL << DMA_ISR_TEIF7_Pos)                    /*!< 0x08000000 */
#define DMA_ISR_TEIF7          DMA_ISR_TEIF7_Msk                               /*!< Channel 7 Transfer Error flag */

/*******************  Bit definition for DMA_IFCR register  *******************/
#define DMA_IFCR_CGIF1_Pos     (0U)
#define DMA_IFCR_CGIF1_Msk     (0x1UL << DMA_IFCR_CGIF1_Pos)                   /*!< 0x00000001 */
#define DMA_IFCR_CGIF1         DMA_IFCR_CGIF1_Msk                              /*!< Channel 1 Global interrupt clearr */
#define DMA_IFCR_CTCIF1_Pos    (1U)
#define DMA_IFCR_CTCIF1_Msk    (0x1UL << DMA_IFCR_CTCIF1_Pos)                  /*!< 0x00000002 */
#define DMA_IFCR_CTCIF1        DMA_IFCR_CTCIF1_Msk                             /*!< Channel 1 Transfer Complete clear */
#define DMA_IFCR_CHTIF1_Pos    (2U)
#define DMA_IFCR_CHTIF1_Msk    (0x1UL << DMA_IFCR_CHTIF1_Pos)                  /*!< 0x00000004 */
#define DMA_IFCR_CHTIF1        DMA_IFCR_CHTIF1_Msk                             /*!< Channel 1 Half Transfer clear */
#define DMA_IFCR_CTEIF1_Pos    (3U)
#define DMA_IFCR_CTEIF1_Msk    (0x1UL << DMA_IFCR_CTEIF1_Pos)                  /*!< 0x00000008 */
#define DMA_IFCR_CTEIF1        DMA_IFCR_CTEIF1_Msk                             /*!< Channel 1 Transfer Error clear */
#define DMA_IFCR_CGIF2_Pos     (4U)
#define DMA_IFCR_CGIF2_Msk     (0x1UL << DMA_IFCR_CGIF2_Pos)                   /*!< 0x00000010 */
#define DMA_IFCR_CGIF2         DMA_IFCR_CGIF2_Msk                              /*!< Channel 2 Global interrupt clear */
#define DMA_IFCR_CTCIF2_Pos    (5U)
#define DMA_IFCR_CTCIF2_Msk    (0x1UL << DMA_IFCR_CTCIF2_Pos)                  /*!< 0x00000020 */
#define DMA_IFCR_CTCIF2        DMA_IFCR_CTCIF2_Msk                             /*!< Channel 2 Transfer Complete clear */
#define DMA_IFCR_CHTIF2_Pos    (6U)
#define DMA_IFCR_CHTIF2_Msk    (0x1UL << DMA_IFCR_CHTIF2_Pos)                  /*!< 0x00000040 */
#define DMA_IFCR_CHTIF2        DMA_IFCR_CHTIF2_Msk                             /*!< Channel 2 Half Transfer clear */
#define DMA_IFCR_CTEIF2_Pos    (7U)
#define DMA_IFCR_CTEIF2_Msk    (0x1UL << DMA_IFCR_CTEIF2_Pos)                  /*!< 0x00000080 */
#define DMA_IFCR_CTEIF2        DMA_IFCR_CTEIF2_Msk                             /*!< Channel 2 Transfer Error clear */
#define DMA_IFCR_CGIF3_Pos     (8U)
#define DMA_IFCR_CGIF3_Msk     (0x1UL << DMA_IFCR_CGIF3_Pos)                   /*!< 0x00000100 */
#define DMA_IFCR_CGIF3         DMA_IFCR_CGIF3_Msk                              /*!< Channel 3 Global interrupt clear */
#define DMA_IFCR_CTCIF3_Pos    (9U)
#define DMA_IFCR_CTCIF3_Msk    (0x1UL << DMA_IFCR_CTCIF3_Pos)                  /*!< 0x00000200 */
#define DMA_IFCR_CTCIF3        DMA_IFCR_CTCIF3_Msk                             /*!< Channel 3 Transfer Complete clear */
#define DMA_IFCR_CHTIF3_Pos    (10U)
#define DMA_IFCR_CHTIF3_Msk    (0x1UL << DMA_IFCR_CHTIF3_Pos)                  /*!< 0x00000400 */
#define DMA_IFCR_CHTIF3        DMA_IFCR_CHTIF3_Msk                             /*!< Channel 3 Half Transfer clear */
#define DMA_IFCR_CTEIF3_Pos    (11U)
#define DMA_IFCR_CTEIF3_Msk    (0x1UL << DMA_IFCR_CTEIF3_Pos)                  /*!< 0x00000800 */
#define DMA_IFCR_CTEIF3        DMA_IFCR_CTEIF3_Msk                             /*!< Channel 3 Transfer Error clear */
#define DMA_IFCR_CGIF4_Pos     (12U)
#define DMA_IFCR_CGIF4_Msk     (0x1UL << DMA_IFCR_CGIF4_Pos)                   /*!< 0x00001000 */
#define DMA_IFCR_CGIF4         DMA_IFCR_CGIF4_Msk                              /*!< Channel 4 Global interrupt clear */
#define DMA_IFCR_CTCIF4_Pos    (13U)
#define DMA_IFCR_CTCIF4_Msk    (0x1UL << DMA_IFCR_CTCIF4_Pos)                  /*!< 0x00002000 */
#define DMA_IFCR_CTCIF4        DMA_IFCR_CTCIF4_Msk                             /*!< Channel 4 Transfer Complete clear */
#define DMA_IFCR_CHTIF4_Pos    (14U)
#define DMA_IFCR_CHTIF4_Msk    (0x1UL << DMA_IFCR_CHTIF4_Pos)                  /*!< 0x00004000 */
#define DMA_IFCR_CHTIF4        DMA_IFCR_CHTIF4_Msk                             /*!< Channel 4 Half Transfer clear */
#define DMA_IFCR_CTEIF4_Pos    (15U)
#define DMA_IFCR_CTEIF4_Msk    (0x1UL << DMA_IFCR_CTEIF4_Pos)                  /*!< 0x00008000 */
#define DMA_IFCR_CTEIF4        DMA_IFCR_CTEIF4_Msk                             /*!< Channel 4 Transfer Error clear */
#define DMA_IFCR_CGIF5_Pos     (16U)
#define DMA_IFCR_CGIF5_Msk     (0x1UL << DMA_IFCR_CGIF5_Pos)                   /*!< 0x00010000 */
#define DMA_IFCR_CGIF5         DMA_IFCR_CGIF5_Msk                              /*!< Channel 5 Global interrupt clear */
#define DMA_IFCR_CTCIF5_Pos    (17U)
#define DMA_IFCR_CTCIF5_Msk    (0x1UL << DMA_IFCR_CTCIF5_Pos)                  /*!< 0x00020000 */
#define DMA_IFCR_CTCIF5        DMA_IFCR_CTCIF5_Msk                             /*!< Channel 5 Transfer Complete clear */
#define DMA_IFCR_CHTIF5_Pos    (18U)
#define DMA_IFCR_CHTIF5_Msk    (0x1UL << DMA_IFCR_CHTIF5_Pos)                  /*!< 0x00040000 */
#define DMA_IFCR_CHTIF5        DMA_IFCR_CHTIF5_Msk                             /*!< Channel 5 Half Transfer clear */
#define DMA_IFCR_CTEIF5_Pos    (19U)
#define DMA_IFCR_CTEIF5_Msk    (0x1UL << DMA_IFCR_CTEIF5_Pos)                  /*!< 0x00080000 */
#define DMA_IFCR_CTEIF5        DMA_IFCR_CTEIF5_Msk                             /*!< Channel 5 Transfer Error clear */
#define DMA_IFCR_CGIF6_Pos     (20U)
#define DMA_IFCR_CGIF6_Msk     (0x1UL << DMA_IFCR_CGIF6_Pos)                   /*!< 0x00100000 */
#define DMA_IFCR_CGIF6         DMA_IFCR_CGIF6_Msk                              /*!< Channel 6 Global interrupt clear */
#define DMA_IFCR_CTCIF6_Pos    (21U)
#define DMA_IFCR_CTCIF6_Msk    (0x1UL << DMA_IFCR_CTCIF6_Pos)                  /*!< 0x00200000 */
#define DMA_IFCR_CTCIF6        DMA_IFCR_CTCIF6_Msk                             /*!< Channel 6 Transfer Complete clear */
#define DMA_IFCR_CHTIF6_Pos    (22U)
#define DMA_IFCR_CHTIF6_Msk    (0x1UL << DMA_IFCR_CHTIF6_Pos)                  /*!< 0x00400000 */
#define DMA_IFCR_CHTIF6        DMA_IFCR_CHTIF6_Msk                             /*!< Channel 6 Half Transfer clear */
#define DMA_IFCR_CTEIF6_Pos    (23U)
#define DMA_IFCR_CTEIF6_Msk    (0x1UL << DMA_IFCR_CTEIF6_Pos)                  /*!< 0x00800000 */
#define DMA_IFCR_CTEIF6        DMA_IFCR_CTEIF6_Msk                             /*!< Channel 6 Transfer Error clear */
#define DMA_IFCR_CGIF7_Pos     (24U)
#define DMA_IFCR_CGIF7_Msk     (0x1UL << DMA_IFCR_CGIF7_Pos)                   /*!< 0x01000000 */
#define DMA_IFCR_CGIF7         DMA_IFCR_CGIF7_Msk                              /*!< Channel 7 Global interrupt clear */
#define DMA_IFCR_CTCIF7_Pos    (25U)
#define DMA_IFCR_CTCIF7_Msk    (0x1UL << DMA_IFCR_CTCIF7_Pos)                  /*!< 0x02000000 */
#define DMA_IFCR_CTCIF7        DMA_IFCR_CTCIF7_Msk                             /*!< Channel 7 Transfer Complete clear */
#define DMA_IFCR_CHTIF7_Pos    (26U)
#define DMA_IFCR_CHTIF7_Msk    (0x1UL << DMA_IFCR_CHTIF7_Pos)                  /*!< 0x04000000 */
#define DMA_IFCR_CHTIF7        DMA_IFCR_CHTIF7_Msk                             /*!< Channel 7 Half Transfer clear */
#define DMA_IFCR_CTEIF7_Pos    (27U)
#define DMA_IFCR_CTEIF7_Msk    (0x1UL << DMA_IFCR_CTEIF7_Pos)                  /*!< 0x08000000 */
#define DMA_IFCR_CTEIF7        DMA_IFCR_CTEIF7_Msk                             /*!< Channel 7 Transfer Error clear */

/*******************  Bit definition for DMA_CCR register  ********************/
#define DMA_CCR_EN_Pos         (0U)
#define DMA_CCR_EN_Msk         (0x1UL << DMA_CCR_EN_Pos)                       /*!< 0x00000001 */
#define DMA_CCR_EN             DMA_CCR_EN_Msk                                  /*!< Channel enable                      */
#define DMA_CCR_TCIE_Pos       (1U)
#define DMA_CCR_TCIE_Msk       (0x1UL << DMA_CCR_TCIE_Pos)                     /*!< 0x00000002 */
#define DMA_CCR_TCIE           DMA_CCR_TCIE_Msk                                /*!< Transfer complete interrupt enable  */
#define DMA_CCR_HTIE_Pos       (2U)
#define DMA_CCR_HTIE_Msk       (0x1UL << DMA_CCR_HTIE_Pos)                     /*!< 0x00000004 */
#define DMA_CCR_HTIE           DMA_CCR_HTIE_Msk                                /*!< Half Transfer interrupt enable      */
#define DMA_CCR_TEIE_Pos       (3U)
#define DMA_CCR_TEIE_Msk       (0x1UL << DMA_CCR_TEIE_Pos)                     /*!< 0x00000008 */
#define DMA_CCR_TEIE           DMA_CCR_TEIE_Msk                                /*!< Transfer error interrupt enable     */
#define DMA_CCR_DIR_Pos        (4U)
#define DMA_CCR_DIR_Msk        (0x1UL << DMA_CCR_DIR_Pos)                      /*!< 0x00000010 */
#define DMA_CCR_DIR            DMA_CCR_DIR_Msk                                 /*!< Data transfer direction             */
#define DMA_CCR_CIRC_Pos       (5U)
#define DMA_CCR_CIRC_Msk       (0x1UL << DMA_CCR_CIRC_Pos)                     /*!< 0x00000020 */
#define DMA_CCR_CIRC           DMA_CCR_CIRC_Msk                                /*!< Circular mode                       */
#define DMA_CCR_PINC_Pos       (6U)
#define DMA_CCR_PINC_Msk       (0x1UL << DMA_CCR_PINC_Pos)                     /*!< 0x00000040 */
#define DMA_CCR_PINC           DMA_CCR_PINC_Msk                                /*!< Peripheral increment mode           */
#define DMA_CCR_MINC_Pos       (7U)
#define DMA_CCR_MINC_Msk       (0x1UL << DMA_CCR_MINC_Pos)                     /*!< 0x00000080 */
#define DMA_CCR_MINC           DMA_CCR_MINC_Msk                                /*!< Memory increment mode               */

#define DMA_CCR_PSIZE_Pos      (8U)
#define DMA_CCR_PSIZE_Msk      (0x3UL << DMA_CCR_PSIZE_Pos)                    /*!< 0x00000300 */
#define DMA_CCR_PSIZE          DMA_CCR_PSIZE_Msk                               /*!< PSIZE[1:0] bits (Peripheral size)   */
#define DMA_CCR_PSIZE_0        (0x1UL << DMA_CCR_PSIZE_Pos)                    /*!< 0x00000100 */
#define DMA_CCR_PSIZE_1        (0x2UL << DMA_CCR_PSIZE_Pos)                    /*!< 0x00000200 */

#define DMA_CCR_MSIZE_Pos      (10U)
#define DMA_CCR_MSIZE_Msk      (0x3UL << DMA_CCR_MSIZE_Pos)                    /*!< 0x00000C00 */
#define DMA_CCR_MSIZE          DMA_CCR_MSIZE_Msk                               /*!< MSIZE[1:0] bits (Memory size)       */
#define DMA_CCR_MSIZE_0        (0x1UL << DMA_CCR_MSIZE_Pos)                    /*!< 0x00000400 */
#define DMA_CCR_MSIZE_1        (0x2UL << DMA_CCR_MSIZE_Pos)                    /*!< 0x00000800 */

#define DMA_CCR_PL_Pos         (12U)
#define DMA_CCR_PL_Msk         (0x3UL << DMA_CCR_PL_Pos)                       /*!< 0x00003000 */
#define DMA_CCR_PL             DMA_CCR_PL_Msk                                  /*!< PL[1:0] bits(Channel Priority level)*/
#define DMA_CCR_PL_0           (0x1UL << DMA_CCR_PL_Pos)                       /*!< 0x00001000 */
#define DMA_CCR_PL_1           (0x2UL << DMA_CCR_PL_Pos)                       /*!< 0x00002000 */

#define DMA_CCR_MEM2MEM_Pos    (14U)
#define DMA_CCR_MEM2MEM_Msk    (0x1UL << DMA_CCR_MEM2MEM_Pos)                  /*!< 0x00004000 */
#define DMA_CCR_MEM2MEM        DMA_CCR_MEM2MEM_Msk                             /*!< Memory to memory mode               */

/******************  Bit definition for DMA_CNDTR register  *******************/
#define DMA_CNDTR_NDT_Pos      (0U)
#define DMA_CNDTR_NDT_Msk      (0xFFFFUL << DMA_CNDTR_NDT_Pos)                 /*!< 0x0000FFFF */
#define DMA_CNDTR_NDT          DMA_CNDTR_NDT_Msk                               /*!< Number of data to Transfer          */

/******************  Bit definition for DMA_CPAR register  ********************/
#define DMA_CPAR_PA_Pos        (0U)
#define DMA_CPAR_PA_Msk        (0xFFFFFFFFUL << DMA_CPAR_PA_Pos)               /*!< 0xFFFFFFFF */
#define DMA_CPAR_PA            DMA_CPAR_PA_Msk                                 /*!< Peripheral Address                  */

/******************  Bit definition for DMA_CMAR register  ********************/
#define DMA_CMAR_MA_Pos        (0U)
#define DMA_CMAR_MA_Msk        (0xFFFFFFFFUL << DMA_CMAR_MA_Pos)               /*!< 0xFFFFFFFF */
#define DMA_CMAR_MA            DMA_CMAR_MA_Msk                                 /*!< Memory Address                      */


/*******************  Bit definition for DMA_CSELR register  *******************/
#define DMA_CSELR_C1S_Pos      (0U)
#define DMA_CSELR_C1S_Msk      (0xFUL << DMA_CSELR_C1S_Pos)                    /*!< 0x0000000F */
#define DMA_CSELR_C1S          DMA_CSELR_C1S_Msk                               /*!< Channel 1 Selection */
#define DMA_CSELR_C2S_Pos      (4U)
#define DMA_CSELR_C2S_Msk      (0xFUL << DMA_CSELR_C2S_Pos)                    /*!< 0x000000F0 */
#define DMA_CSELR_C2S          DMA_CSELR_C2S_Msk                               /*!< Channel 2 Selection */
#define DMA_CSELR_C3S_Pos      (8U)
#define DMA_CSELR_C3S_Msk      (0xFUL << DMA_CSELR_C3S_Pos)                    /*!< 0x00000F00 */
#define DMA_CSELR_C3S          DMA_CSELR_C3S_Msk                               /*!< Channel 3 Selection */
#define DMA_CSELR_C4S_Pos      (12U)
#define DMA_CSELR_C4S_Msk      (0xFUL << DMA_CSELR_C4S_Pos)                    /*!< 0x0000F000 */
#define DMA_CSELR_C4S          DMA_CSELR_C4S_Msk                               /*!< Channel 4 Selection */
#define DMA_CSELR_C5S_Pos      (16U)
#define DMA_CSELR_C5S_Msk      (0xFUL << DMA_CSELR_C5S_Pos)                    /*!< 0x000F0000 */
#define DMA_CSELR_C5S          DMA_CSELR_C5S_Msk                               /*!< Channel 5 Selection */
#define DMA_CSELR_C6S_Pos      (20U)
#define DMA_CSELR_C6S_Msk      (0xFUL << DMA_CSELR_C6S_Pos)                    /*!< 0x00F00000 */
#define DMA_CSELR_C6S          DMA_CSELR_C6S_Msk                               /*!< Channel 6 Selection */
#define DMA_CSELR_C7S_Pos      (24U)
#define DMA_CSELR_C7S_Msk      (0xFUL << DMA_CSELR_C7S_Pos)                    /*!< 0x0F000000 */
#define DMA_CSELR_C7S          DMA_CSELR_C7S_Msk                               /*!< Channel 7 Selection */

/******************************************************************************/
/*                                                                            */
/*                    External Interrupt/Event Controller                     */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for EXTI_IMR1 register  ******************/
#define EXTI_IMR1_IM0_Pos        (0U)
#define EXTI_IMR1_IM0_Msk        (0x1UL << EXTI_IMR1_IM0_Pos)                  /*!< 0x00000001 */
#define EXTI_IMR1_IM0            EXTI_IMR1_IM0_Msk                             /*!< Interrupt Mask on line 0 */
#define EXTI_IMR1_IM1_Pos        (1U)
#define EXTI_IMR1_IM1_Msk        (0x1UL << EXTI_IMR1_IM1_Pos)                  /*!< 0x00000002 */
#define EXTI_IMR1_IM1            EXTI_IMR1_IM1_Msk                             /*!< Interrupt Mask on line 1 */
#define EXTI_IMR1_IM2_Pos        (2U)
#define EXTI_IMR1_IM2_Msk        (0x1UL << EXTI_IMR1_IM2_Pos)                  /*!< 0x00000004 */
#define EXTI_IMR1_IM2            EXTI_IMR1_IM2_Msk                             /*!< Interrupt Mask on line 2 */
#define EXTI_IMR1_IM3_Pos        (3U)
#define EXTI_IMR1_IM3_Msk        (0x1UL << EXTI_IMR1_IM3_Pos)                  /*!< 0x00000008 */
#define EXTI_IMR1_IM3            EXTI_IMR1_IM3_Msk                             /*!< Interrupt Mask on line 3 */
#define EXTI_IMR1_IM4_Pos        (4U)
#define EXTI_IMR1_IM4_Msk        (0x1UL << EXTI_IMR1_IM4_Pos)                  /*!< 0x00000010 */
#define EXTI_IMR1_IM4            EXTI_IMR1_IM4_Msk                             /*!< Interrupt Mask on line 4 */
#define EXTI_IMR1_IM5_Pos        (5U)
#define EXTI_IMR1_IM5_Msk        (0x1UL << EXTI_IMR1_IM5_Pos)                  /*!< 0x00000020 */
#define EXTI_IMR1_IM5            EXTI_IMR1_IM5_Msk                             /*!< Interrupt Mask on line 5 */
#define EXTI_IMR1_IM6_Pos        (6U)
#define EXTI_IMR1_IM6_Msk        (0x1UL << EXTI_IMR1_IM6_Pos)                  /*!< 0x00000040 */
#define EXTI_IMR1_IM6            EXTI_IMR1_IM6_Msk                             /*!< Interrupt Mask on line 6 */
#define EXTI_IMR1_IM7_Pos        (7U)
#define EXTI_IMR1_IM7_Msk        (0x1UL << EXTI_IMR1_IM7_Pos)                  /*!< 0x00000080 */
#define EXTI_IMR1_IM7            EXTI_IMR1_IM7_Msk                             /*!< Interrupt Mask on line 7 */
#define EXTI_IMR1_IM8_Pos        (8U)
#define EXTI_IMR1_IM8_Msk        (0x1UL << EXTI_IMR1_IM8_Pos)                  /*!< 0x00000100 */
#define EXTI_IMR1_IM8            EXTI_IMR1_IM8_Msk                             /*!< Interrupt Mask on line 8 */
#define EXTI_IMR1_IM9_Pos        (9U)
#define EXTI_IMR1_IM9_Msk        (0x1UL << EXTI_IMR1_IM9_Pos)                  /*!< 0x00000200 */
#define EXTI_IMR1_IM9            EXTI_IMR1_IM9_Msk                             /*!< Interrupt Mask on line 9 */
#define EXTI_IMR1_IM10_Pos       (10U)
#define EXTI_IMR1_IM10_Msk       (0x1UL << EXTI_IMR1_IM10_Pos)                 /*!< 0x00000400 */
#define EXTI_IMR1_IM10           EXTI_IMR1_IM10_Msk                            /*!< Interrupt Mask on line 10 */
#define EXTI_IMR1_IM11_Pos       (11U)
#define EXTI_IMR1_IM11_Msk       (0x1UL << EXTI_IMR1_IM11_Pos)                 /*!< 0x00000800 */
#define EXTI_IMR1_IM11           EXTI_IMR1_IM11_Msk                            /*!< Interrupt Mask on line 11 */
#define EXTI_IMR1_IM12_Pos       (12U)
#define EXTI_IMR1_IM12_Msk       (0x1UL << EXTI_IMR1_IM12_Pos)                 /*!< 0x00001000 */
#define EXTI_IMR1_IM12           EXTI_IMR1_IM12_Msk                            /*!< Interrupt Mask on line 12 */
#define EXTI_IMR1_IM13_Pos       (13U)
#define EXTI_IMR1_IM13_Msk       (0x1UL << EXTI_IMR1_IM13_Pos)                 /*!< 0x00002000 */
#define EXTI_IMR1_IM13           EXTI_IMR1_IM13_Msk                            /*!< Interrupt Mask on line 13 */
#define EXTI_IMR1_IM14_Pos       (14U)
#define EXTI_IMR1_IM14_Msk       (0x1UL << EXTI_IMR1_IM14_Pos)                 /*!< 0x00004000 */
#define EXTI_IMR1_IM14           EXTI_IMR1_IM14_Msk                            /*!< Interrupt Mask on line 14 */
#define EXTI_IMR1_IM15_Pos       (15U)
#define EXTI_IMR1_IM15_Msk       (0x1UL << EXTI_IMR1_IM15_Pos)                 /*!< 0x00008000 */
#define EXTI_IMR1_IM15           EXTI_IMR1_IM15_Msk                            /*!< Interrupt Mask on line 15 */
#define EXTI_IMR1_IM16_Pos       (16U)
#define EXTI_IMR1_IM16_Msk       (0x1UL << EXTI_IMR1_IM16_Pos)                 /*!< 0x00010000 */
#define EXTI_IMR1_IM16           EXTI_IMR1_IM16_Msk                            /*!< Interrupt Mask on line 16 */
#define EXTI_IMR1_IM17_Pos       (17U)
#define EXTI_IMR1_IM17_Msk       (0x1UL << EXTI_IMR1_IM17_Pos)                 /*!< 0x00020000 */
#define EXTI_IMR1_IM17           EXTI_IMR1_IM17_Msk                            /*!< Interrupt Mask on line 17 */
#define EXTI_IMR1_IM18_Pos       (18U)
#define EXTI_IMR1_IM18_Msk       (0x1UL << EXTI_IMR1_IM18_Pos)                 /*!< 0x00040000 */
#define EXTI_IMR1_IM18           EXTI_IMR1_IM18_Msk                            /*!< Interrupt Mask on line 18 */
#define EXTI_IMR1_IM19_Pos       (19U)
#define EXTI_IMR1_IM19_Msk       (0x1UL << EXTI_IMR1_IM19_Pos)                 /*!< 0x00080000 */
#define EXTI_IMR1_IM19           EXTI_IMR1_IM19_Msk                            /*!< Interrupt Mask on line 19 */
#define EXTI_IMR1_IM20_Pos       (20U)
#define EXTI_IMR1_IM20_Msk       (0x1UL << EXTI_IMR1_IM20_Pos)                 /*!< 0x00100000 */
#define EXTI_IMR1_IM20           EXTI_IMR1_IM20_Msk                            /*!< Interrupt Mask on line 20 */
#define EXTI_IMR1_IM21_Pos       (21U)
#define EXTI_IMR1_IM21_Msk       (0x1UL << EXTI_IMR1_IM21_Pos)                 /*!< 0x00200000 */
#define EXTI_IMR1_IM21           EXTI_IMR1_IM21_Msk                            /*!< Interrupt Mask on line 21 */
#define EXTI_IMR1_IM23_Pos       (23U)
#define EXTI_IMR1_IM23_Msk       (0x1UL << EXTI_IMR1_IM23_Pos)                 /*!< 0x00800000 */
#define EXTI_IMR1_IM23           EXTI_IMR1_IM23_Msk                            /*!< Interrupt Mask on line 23 */
#define EXTI_IMR1_IM24_Pos       (24U)
#define EXTI_IMR1_IM24_Msk       (0x1UL << EXTI_IMR1_IM24_Pos)                 /*!< 0x01000000 */
#define EXTI_IMR1_IM24           EXTI_IMR1_IM24_Msk                            /*!< Interrupt Mask on line 24 */
#define EXTI_IMR1_IM25_Pos       (25U)
#define EXTI_IMR1_IM25_Msk       (0x1UL << EXTI_IMR1_IM25_Pos)                 /*!< 0x02000000 */
#define EXTI_IMR1_IM25           EXTI_IMR1_IM25_Msk                            /*!< Interrupt Mask on line 25 */
#define EXTI_IMR1_IM26_Pos       (26U)
#define EXTI_IMR1_IM26_Msk       (0x1UL << EXTI_IMR1_IM26_Pos)                 /*!< 0x04000000 */
#define EXTI_IMR1_IM26           EXTI_IMR1_IM26_Msk                            /*!< Interrupt Mask on line 26 */
#define EXTI_IMR1_IM27_Pos       (27U)
#define EXTI_IMR1_IM27_Msk       (0x1UL << EXTI_IMR1_IM27_Pos)                 /*!< 0x08000000 */
#define EXTI_IMR1_IM27           EXTI_IMR1_IM27_Msk                            /*!< Interrupt Mask on line 27 */
#define EXTI_IMR1_IM28_Pos       (28U)
#define EXTI_IMR1_IM28_Msk       (0x1UL << EXTI_IMR1_IM28_Pos)                 /*!< 0x10000000 */
#define EXTI_IMR1_IM28           EXTI_IMR1_IM28_Msk                            /*!< Interrupt Mask on line 28 */
#define EXTI_IMR1_IM29_Pos       (29U)
#define EXTI_IMR1_IM29_Msk       (0x1UL << EXTI_IMR1_IM29_Pos)                 /*!< 0x20000000 */
#define EXTI_IMR1_IM29           EXTI_IMR1_IM29_Msk                            /*!< Interrupt Mask on line 29 */
#define EXTI_IMR1_IM30_Pos       (30U)
#define EXTI_IMR1_IM30_Msk       (0x1UL << EXTI_IMR1_IM30_Pos)                 /*!< 0x40000000 */
#define EXTI_IMR1_IM30           EXTI_IMR1_IM30_Msk                            /*!< Interrupt Mask on line 30 */
#define EXTI_IMR1_IM31_Pos       (31U)
#define EXTI_IMR1_IM31_Msk       (0x1UL << EXTI_IMR1_IM31_Pos)                 /*!< 0x80000000 */
#define EXTI_IMR1_IM31           EXTI_IMR1_IM31_Msk                            /*!< Interrupt Mask on line 31 */
#define EXTI_IMR1_IM_Pos         (0U)
#define EXTI_IMR1_IM_Msk         (0x9FFFFFFFUL << EXTI_IMR1_IM_Pos)            /*!< 0x9FFFFFFF */
#define EXTI_IMR1_IM             EXTI_IMR1_IM_Msk                              /*!< Interrupt Mask All */

/*******************  Bit definition for EXTI_EMR1 register  ******************/
#define EXTI_EMR1_EM0_Pos        (0U)
#define EXTI_EMR1_EM0_Msk        (0x1UL << EXTI_EMR1_EM0_Pos)                  /*!< 0x00000001 */
#define EXTI_EMR1_EM0            EXTI_EMR1_EM0_Msk                             /*!< Event Mask on line 0 */
#define EXTI_EMR1_EM1_Pos        (1U)
#define EXTI_EMR1_EM1_Msk        (0x1UL << EXTI_EMR1_EM1_Pos)                  /*!< 0x00000002 */
#define EXTI_EMR1_EM1            EXTI_EMR1_EM1_Msk                             /*!< Event Mask on line 1 */
#define EXTI_EMR1_EM2_Pos        (2U)
#define EXTI_EMR1_EM2_Msk        (0x1UL << EXTI_EMR1_EM2_Pos)                  /*!< 0x00000004 */
#define EXTI_EMR1_EM2            EXTI_EMR1_EM2_Msk                             /*!< Event Mask on line 2 */
#define EXTI_EMR1_EM3_Pos        (3U)
#define EXTI_EMR1_EM3_Msk        (0x1UL << EXTI_EMR1_EM3_Pos)                  /*!< 0x00000008 */
#define EXTI_EMR1_EM3            EXTI_EMR1_EM3_Msk                             /*!< Event Mask on line 3 */
#define EXTI_EMR1_EM4_Pos        (4U)
#define EXTI_EMR1_EM4_Msk        (0x1UL << EXTI_EMR1_EM4_Pos)                  /*!< 0x00000010 */
#define EXTI_EMR1_EM4            EXTI_EMR1_EM4_Msk                             /*!< Event Mask on line 4 */
#define EXTI_EMR1_EM5_Pos        (5U)
#define EXTI_EMR1_EM5_Msk        (0x1UL << EXTI_EMR1_EM5_Pos)                  /*!< 0x00000020 */
#define EXTI_EMR1_EM5            EXTI_EMR1_EM5_Msk                             /*!< Event Mask on line 5 */
#define EXTI_EMR1_EM6_Pos        (6U)
#define EXTI_EMR1_EM6_Msk        (0x1UL << EXTI_EMR1_EM6_Pos)                  /*!< 0x00000040 */
#define EXTI_EMR1_EM6            EXTI_EMR1_EM6_Msk                             /*!< Event Mask on line 6 */
#define EXTI_EMR1_EM7_Pos        (7U)
#define EXTI_EMR1_EM7_Msk        (0x1UL << EXTI_EMR1_EM7_Pos)                  /*!< 0x00000080 */
#define EXTI_EMR1_EM7            EXTI_EMR1_EM7_Msk                             /*!< Event Mask on line 7 */
#define EXTI_EMR1_EM8_Pos        (8U)
#define EXTI_EMR1_EM8_Msk        (0x1UL << EXTI_EMR1_EM8_Pos)                  /*!< 0x00000100 */
#define EXTI_EMR1_EM8            EXTI_EMR1_EM8_Msk                             /*!< Event Mask on line 8 */
#define EXTI_EMR1_EM9_Pos        (9U)
#define EXTI_EMR1_EM9_Msk        (0x1UL << EXTI_EMR1_EM9_Pos)                  /*!< 0x00000200 */
#define EXTI_EMR1_EM9            EXTI_EMR1_EM9_Msk                             /*!< Event Mask on line 9 */
#define EXTI_EMR1_EM10_Pos       (10U)
#define EXTI_EMR1_EM10_Msk       (0x1UL << EXTI_EMR1_EM10_Pos)                 /*!< 0x00000400 */
#define EXTI_EMR1_EM10           EXTI_EMR1_EM10_Msk                            /*!< Event Mask on line 10 */
#define EXTI_EMR1_EM11_Pos       (11U)
#define EXTI_EMR1_EM11_Msk       (0x1UL << EXTI_EMR1_EM11_Pos)                 /*!< 0x00000800 */
#define EXTI_EMR1_EM11           EXTI_EMR1_EM11_Msk                            /*!< Event Mask on line 11 */
#define EXTI_EMR1_EM12_Pos       (12U)
#define EXTI_EMR1_EM12_Msk       (0x1UL << EXTI_EMR1_EM12_Pos)                 /*!< 0x00001000 */
#define EXTI_EMR1_EM12           EXTI_EMR1_EM12_Msk                            /*!< Event Mask on line 12 */
#define EXTI_EMR1_EM13_Pos       (13U)
#define EXTI_EMR1_EM13_Msk       (0x1UL << EXTI_EMR1_EM13_Pos)                 /*!< 0x00002000 */
#define EXTI_EMR1_EM13           EXTI_EMR1_EM13_Msk                            /*!< Event Mask on line 13 */
#define EXTI_EMR1_EM14_Pos       (14U)
#define EXTI_EMR1_EM14_Msk       (0x1UL << EXTI_EMR1_EM14_Pos)                 /*!< 0x00004000 */
#define EXTI_EMR1_EM14           EXTI_EMR1_EM14_Msk                            /*!< Event Mask on line 14 */
#define EXTI_EMR1_EM15_Pos       (15U)
#define EXTI_EMR1_EM15_Msk       (0x1UL << EXTI_EMR1_EM15_Pos)                 /*!< 0x00008000 */
#define EXTI_EMR1_EM15           EXTI_EMR1_EM15_Msk                            /*!< Event Mask on line 15 */
#define EXTI_EMR1_EM16_Pos       (16U)
#define EXTI_EMR1_EM16_Msk       (0x1UL << EXTI_EMR1_EM16_Pos)                 /*!< 0x00010000 */
#define EXTI_EMR1_EM16           EXTI_EMR1_EM16_Msk                            /*!< Event Mask on line 16 */
#define EXTI_EMR1_EM17_Pos       (17U)
#define EXTI_EMR1_EM17_Msk       (0x1UL << EXTI_EMR1_EM17_Pos)                 /*!< 0x00020000 */
#define EXTI_EMR1_EM17           EXTI_EMR1_EM17_Msk                            /*!< Event Mask on line 17 */
#define EXTI_EMR1_EM18_Pos       (18U)
#define EXTI_EMR1_EM18_Msk       (0x1UL << EXTI_EMR1_EM18_Pos)                 /*!< 0x00040000 */
#define EXTI_EMR1_EM18           EXTI_EMR1_EM18_Msk                            /*!< Event Mask on line 18 */
#define EXTI_EMR1_EM19_Pos       (19U)
#define EXTI_EMR1_EM19_Msk       (0x1UL << EXTI_EMR1_EM19_Pos)                 /*!< 0x00080000 */
#define EXTI_EMR1_EM19           EXTI_EMR1_EM19_Msk                            /*!< Event Mask on line 19 */
#define EXTI_EMR1_EM20_Pos       (20U)
#define EXTI_EMR1_EM20_Msk       (0x1UL << EXTI_EMR1_EM20_Pos)                 /*!< 0x00100000 */
#define EXTI_EMR1_EM20           EXTI_EMR1_EM20_Msk                            /*!< Event Mask on line 20 */
#define EXTI_EMR1_EM21_Pos       (21U)
#define EXTI_EMR1_EM21_Msk       (0x1UL << EXTI_EMR1_EM21_Pos)                 /*!< 0x00200000 */
#define EXTI_EMR1_EM21           EXTI_EMR1_EM21_Msk                            /*!< Event Mask on line 21 */
#define EXTI_EMR1_EM23_Pos       (23U)
#define EXTI_EMR1_EM23_Msk       (0x1UL << EXTI_EMR1_EM23_Pos)                 /*!< 0x00800000 */
#define EXTI_EMR1_EM23           EXTI_EMR1_EM23_Msk                            /*!< Event Mask on line 23 */
#define EXTI_EMR1_EM24_Pos       (24U)
#define EXTI_EMR1_EM24_Msk       (0x1UL << EXTI_EMR1_EM24_Pos)                 /*!< 0x01000000 */
#define EXTI_EMR1_EM24           EXTI_EMR1_EM24_Msk                            /*!< Event Mask on line 24 */
#define EXTI_EMR1_EM25_Pos       (25U)
#define EXTI_EMR1_EM25_Msk       (0x1UL << EXTI_EMR1_EM25_Pos)                 /*!< 0x02000000 */
#define EXTI_EMR1_EM25           EXTI_EMR1_EM25_Msk                            /*!< Event Mask on line 25 */
#define EXTI_EMR1_EM26_Pos       (26U)
#define EXTI_EMR1_EM26_Msk       (0x1UL << EXTI_EMR1_EM26_Pos)                 /*!< 0x04000000 */
#define EXTI_EMR1_EM26           EXTI_EMR1_EM26_Msk                            /*!< Event Mask on line 26 */
#define EXTI_EMR1_EM27_Pos       (27U)
#define EXTI_EMR1_EM27_Msk       (0x1UL << EXTI_EMR1_EM27_Pos)                 /*!< 0x08000000 */
#define EXTI_EMR1_EM27           EXTI_EMR1_EM27_Msk                            /*!< Event Mask on line 27 */
#define EXTI_EMR1_EM28_Pos       (28U)
#define EXTI_EMR1_EM28_Msk       (0x1UL << EXTI_EMR1_EM28_Pos)                 /*!< 0x10000000 */
#define EXTI_EMR1_EM28           EXTI_EMR1_EM28_Msk                            /*!< Event Mask on line 28 */
#define EXTI_EMR1_EM31_Pos       (31U)
#define EXTI_EMR1_EM31_Msk       (0x1UL << EXTI_EMR1_EM31_Pos)                 /*!< 0x80000000 */
#define EXTI_EMR1_EM31           EXTI_EMR1_EM31_Msk                            /*!< Event Mask on line 31 */

/******************  Bit definition for EXTI_RTSR1 register  ******************/
#define EXTI_RTSR1_RT0_Pos       (0U)
#define EXTI_RTSR1_RT0_Msk       (0x1UL << EXTI_RTSR1_RT0_Pos)                 /*!< 0x00000001 */
#define EXTI_RTSR1_RT0           EXTI_RTSR1_RT0_Msk                            /*!< Rising trigger event configuration bit of line 0 */
#define EXTI_RTSR1_RT1_Pos       (1U)
#define EXTI_RTSR1_RT1_Msk       (0x1UL << EXTI_RTSR1_RT1_Pos)                 /*!< 0x00000002 */
#define EXTI_RTSR1_RT1           EXTI_RTSR1_RT1_Msk                            /*!< Rising trigger event configuration bit of line 1 */
#define EXTI_RTSR1_RT2_Pos       (2U)
#define EXTI_RTSR1_RT2_Msk       (0x1UL << EXTI_RTSR1_RT2_Pos)                 /*!< 0x00000004 */
#define EXTI_RTSR1_RT2           EXTI_RTSR1_RT2_Msk                            /*!< Rising trigger event configuration bit of line 2 */
#define EXTI_RTSR1_RT3_Pos       (3U)
#define EXTI_RTSR1_RT3_Msk       (0x1UL << EXTI_RTSR1_RT3_Pos)                 /*!< 0x00000008 */
#define EXTI_RTSR1_RT3           EXTI_RTSR1_RT3_Msk                            /*!< Rising trigger event configuration bit of line 3 */
#define EXTI_RTSR1_RT4_Pos       (4U)
#define EXTI_RTSR1_RT4_Msk       (0x1UL << EXTI_RTSR1_RT4_Pos)                 /*!< 0x00000010 */
#define EXTI_RTSR1_RT4           EXTI_RTSR1_RT4_Msk                            /*!< Rising trigger event configuration bit of line 4 */
#define EXTI_RTSR1_RT5_Pos       (5U)
#define EXTI_RTSR1_RT5_Msk       (0x1UL << EXTI_RTSR1_RT5_Pos)                 /*!< 0x00000020 */
#define EXTI_RTSR1_RT5           EXTI_RTSR1_RT5_Msk                            /*!< Rising trigger event configuration bit of line 5 */
#define EXTI_RTSR1_RT6_Pos       (6U)
#define EXTI_RTSR1_RT6_Msk       (0x1UL << EXTI_RTSR1_RT6_Pos)                 /*!< 0x00000040 */
#define EXTI_RTSR1_RT6           EXTI_RTSR1_RT6_Msk                            /*!< Rising trigger event configuration bit of line 6 */
#define EXTI_RTSR1_RT7_Pos       (7U)
#define EXTI_RTSR1_RT7_Msk       (0x1UL << EXTI_RTSR1_RT7_Pos)                 /*!< 0x00000080 */
#define EXTI_RTSR1_RT7           EXTI_RTSR1_RT7_Msk                            /*!< Rising trigger event configuration bit of line 7 */
#define EXTI_RTSR1_RT8_Pos       (8U)
#define EXTI_RTSR1_RT8_Msk       (0x1UL << EXTI_RTSR1_RT8_Pos)                 /*!< 0x00000100 */
#define EXTI_RTSR1_RT8           EXTI_RTSR1_RT8_Msk                            /*!< Rising trigger event configuration bit of line 8 */
#define EXTI_RTSR1_RT9_Pos       (9U)
#define EXTI_RTSR1_RT9_Msk       (0x1UL << EXTI_RTSR1_RT9_Pos)                 /*!< 0x00000200 */
#define EXTI_RTSR1_RT9           EXTI_RTSR1_RT9_Msk                            /*!< Rising trigger event configuration bit of line 9 */
#define EXTI_RTSR1_RT10_Pos      (10U)
#define EXTI_RTSR1_RT10_Msk      (0x1UL << EXTI_RTSR1_RT10_Pos)                /*!< 0x00000400 */
#define EXTI_RTSR1_RT10          EXTI_RTSR1_RT10_Msk                           /*!< Rising trigger event configuration bit of line 10 */
#define EXTI_RTSR1_RT11_Pos      (11U)
#define EXTI_RTSR1_RT11_Msk      (0x1UL << EXTI_RTSR1_RT11_Pos)                /*!< 0x00000800 */
#define EXTI_RTSR1_RT11          EXTI_RTSR1_RT11_Msk                           /*!< Rising trigger event configuration bit of line 11 */
#define EXTI_RTSR1_RT12_Pos      (12U)
#define EXTI_RTSR1_RT12_Msk      (0x1UL << EXTI_RTSR1_RT12_Pos)                /*!< 0x00001000 */
#define EXTI_RTSR1_RT12          EXTI_RTSR1_RT12_Msk                           /*!< Rising trigger event configuration bit of line 12 */
#define EXTI_RTSR1_RT13_Pos      (13U)
#define EXTI_RTSR1_RT13_Msk      (0x1UL << EXTI_RTSR1_RT13_Pos)                /*!< 0x00002000 */
#define EXTI_RTSR1_RT13          EXTI_RTSR1_RT13_Msk                           /*!< Rising trigger event configuration bit of line 13 */
#define EXTI_RTSR1_RT14_Pos      (14U)
#define EXTI_RTSR1_RT14_Msk      (0x1UL << EXTI_RTSR1_RT14_Pos)                /*!< 0x00004000 */
#define EXTI_RTSR1_RT14          EXTI_RTSR1_RT14_Msk                           /*!< Rising trigger event configuration bit of line 14 */
#define EXTI_RTSR1_RT15_Pos      (15U)
#define EXTI_RTSR1_RT15_Msk      (0x1UL << EXTI_RTSR1_RT15_Pos)                /*!< 0x00008000 */
#define EXTI_RTSR1_RT15          EXTI_RTSR1_RT15_Msk                           /*!< Rising trigger event configuration bit of line 15 */
#define EXTI_RTSR1_RT16_Pos      (16U)
#define EXTI_RTSR1_RT16_Msk      (0x1UL << EXTI_RTSR1_RT16_Pos)                /*!< 0x00010000 */
#define EXTI_RTSR1_RT16          EXTI_RTSR1_RT16_Msk                           /*!< Rising trigger event configuration bit of line 16 */
#define EXTI_RTSR1_RT18_Pos      (18U)
#define EXTI_RTSR1_RT18_Msk      (0x1UL << EXTI_RTSR1_RT18_Pos)                /*!< 0x00040000 */
#define EXTI_RTSR1_RT18          EXTI_RTSR1_RT18_Msk                           /*!< Rising trigger event configuration bit of line 18 */
#define EXTI_RTSR1_RT19_Pos      (19U)
#define EXTI_RTSR1_RT19_Msk      (0x1UL << EXTI_RTSR1_RT19_Pos)                /*!< 0x00080000 */
#define EXTI_RTSR1_RT19          EXTI_RTSR1_RT19_Msk                           /*!< Rising trigger event configuration bit of line 19 */
#define EXTI_RTSR1_RT20_Pos      (20U)
#define EXTI_RTSR1_RT20_Msk      (0x1UL << EXTI_RTSR1_RT20_Pos)                /*!< 0x00100000 */
#define EXTI_RTSR1_RT20          EXTI_RTSR1_RT20_Msk                           /*!< Rising trigger event configuration bit of line 20 */
#define EXTI_RTSR1_RT21_Pos      (21U)
#define EXTI_RTSR1_RT21_Msk      (0x1UL << EXTI_RTSR1_RT21_Pos)                /*!< 0x00200000 */
#define EXTI_RTSR1_RT21          EXTI_RTSR1_RT21_Msk                           /*!< Rising trigger event configuration bit of line 21 */

/******************  Bit definition for EXTI_FTSR1 register  ******************/
#define EXTI_FTSR1_FT0_Pos       (0U)
#define EXTI_FTSR1_FT0_Msk       (0x1UL << EXTI_FTSR1_FT0_Pos)                 /*!< 0x00000001 */
#define EXTI_FTSR1_FT0           EXTI_FTSR1_FT0_Msk                            /*!< Falling trigger event configuration bit of line 0 */
#define EXTI_FTSR1_FT1_Pos       (1U)
#define EXTI_FTSR1_FT1_Msk       (0x1UL << EXTI_FTSR1_FT1_Pos)                 /*!< 0x00000002 */
#define EXTI_FTSR1_FT1           EXTI_FTSR1_FT1_Msk                            /*!< Falling trigger event configuration bit of line 1 */
#define EXTI_FTSR1_FT2_Pos       (2U)
#define EXTI_FTSR1_FT2_Msk       (0x1UL << EXTI_FTSR1_FT2_Pos)                 /*!< 0x00000004 */
#define EXTI_FTSR1_FT2           EXTI_FTSR1_FT2_Msk                            /*!< Falling trigger event configuration bit of line 2 */
#define EXTI_FTSR1_FT3_Pos       (3U)
#define EXTI_FTSR1_FT3_Msk       (0x1UL << EXTI_FTSR1_FT3_Pos)                 /*!< 0x00000008 */
#define EXTI_FTSR1_FT3           EXTI_FTSR1_FT3_Msk                            /*!< Falling trigger event configuration bit of line 3 */
#define EXTI_FTSR1_FT4_Pos       (4U)
#define EXTI_FTSR1_FT4_Msk       (0x1UL << EXTI_FTSR1_FT4_Pos)                 /*!< 0x00000010 */
#define EXTI_FTSR1_FT4           EXTI_FTSR1_FT4_Msk                            /*!< Falling trigger event configuration bit of line 4 */
#define EXTI_FTSR1_FT5_Pos       (5U)
#define EXTI_FTSR1_FT5_Msk       (0x1UL << EXTI_FTSR1_FT5_Pos)                 /*!< 0x00000020 */
#define EXTI_FTSR1_FT5           EXTI_FTSR1_FT5_Msk                            /*!< Falling trigger event configuration bit of line 5 */
#define EXTI_FTSR1_FT6_Pos       (6U)
#define EXTI_FTSR1_FT6_Msk       (0x1UL << EXTI_FTSR1_FT6_Pos)                 /*!< 0x00000040 */
#define EXTI_FTSR1_FT6           EXTI_FTSR1_FT6_Msk                            /*!< Falling trigger event configuration bit of line 6 */
#define EXTI_FTSR1_FT7_Pos       (7U)
#define EXTI_FTSR1_FT7_Msk       (0x1UL << EXTI_FTSR1_FT7_Pos)                 /*!< 0x00000080 */
#define EXTI_FTSR1_FT7           EXTI_FTSR1_FT7_Msk                            /*!< Falling trigger event configuration bit of line 7 */
#define EXTI_FTSR1_FT8_Pos       (8U)
#define EXTI_FTSR1_FT8_Msk       (0x1UL << EXTI_FTSR1_FT8_Pos)                 /*!< 0x00000100 */
#define EXTI_FTSR1_FT8           EXTI_FTSR1_FT8_Msk                            /*!< Falling trigger event configuration bit of line 8 */
#define EXTI_FTSR1_FT9_Pos       (9U)
#define EXTI_FTSR1_FT9_Msk       (0x1UL << EXTI_FTSR1_FT9_Pos)                 /*!< 0x00000200 */
#define EXTI_FTSR1_FT9           EXTI_FTSR1_FT9_Msk                            /*!< Falling trigger event configuration bit of line 9 */
#define EXTI_FTSR1_FT10_Pos      (10U)
#define EXTI_FTSR1_FT10_Msk      (0x1UL << EXTI_FTSR1_FT10_Pos)                /*!< 0x00000400 */
#define EXTI_FTSR1_FT10          EXTI_FTSR1_FT10_Msk                           /*!< Falling trigger event configuration bit of line 10 */
#define EXTI_FTSR1_FT11_Pos      (11U)
#define EXTI_FTSR1_FT11_Msk      (0x1UL << EXTI_FTSR1_FT11_Pos)                /*!< 0x00000800 */
#define EXTI_FTSR1_FT11          EXTI_FTSR1_FT11_Msk                           /*!< Falling trigger event configuration bit of line 11 */
#define EXTI_FTSR1_FT12_Pos      (12U)
#define EXTI_FTSR1_FT12_Msk      (0x1UL << EXTI_FTSR1_FT12_Pos)                /*!< 0x00001000 */
#define EXTI_FTSR1_FT12          EXTI_FTSR1_FT12_Msk                           /*!< Falling trigger event configuration bit of line 12 */
#define EXTI_FTSR1_FT13_Pos      (13U)
#define EXTI_FTSR1_FT13_Msk      (0x1UL << EXTI_FTSR1_FT13_Pos)                /*!< 0x00002000 */
#define EXTI_FTSR1_FT13          EXTI_FTSR1_FT13_Msk                           /*!< Falling trigger event configuration bit of line 13 */
#define EXTI_FTSR1_FT14_Pos      (14U)
#define EXTI_FTSR1_FT14_Msk      (0x1UL << EXTI_FTSR1_FT14_Pos)                /*!< 0x00004000 */
#define EXTI_FTSR1_FT14          EXTI_FTSR1_FT14_Msk                           /*!< Falling trigger event configuration bit of line 14 */
#define EXTI_FTSR1_FT15_Pos      (15U)
#define EXTI_FTSR1_FT15_Msk      (0x1UL << EXTI_FTSR1_FT15_Pos)                /*!< 0x00008000 */
#define EXTI_FTSR1_FT15          EXTI_FTSR1_FT15_Msk                           /*!< Falling trigger event configuration bit of line 15 */
#define EXTI_FTSR1_FT16_Pos      (16U)
#define EXTI_FTSR1_FT16_Msk      (0x1UL << EXTI_FTSR1_FT16_Pos)                /*!< 0x00010000 */
#define EXTI_FTSR1_FT16          EXTI_FTSR1_FT16_Msk                           /*!< Falling trigger event configuration bit of line 16 */
#define EXTI_FTSR1_FT18_Pos      (18U)
#define EXTI_FTSR1_FT18_Msk      (0x1UL << EXTI_FTSR1_FT18_Pos)                /*!< 0x00040000 */
#define EXTI_FTSR1_FT18          EXTI_FTSR1_FT18_Msk                           /*!< Falling trigger event configuration bit of line 18 */
#define EXTI_FTSR1_FT19_Pos      (19U)
#define EXTI_FTSR1_FT19_Msk      (0x1UL << EXTI_FTSR1_FT19_Pos)                /*!< 0x00080000 */
#define EXTI_FTSR1_FT19          EXTI_FTSR1_FT19_Msk                           /*!< Falling trigger event configuration bit of line 19 */
#define EXTI_FTSR1_FT20_Pos      (20U)
#define EXTI_FTSR1_FT20_Msk      (0x1UL << EXTI_FTSR1_FT20_Pos)                /*!< 0x00100000 */
#define EXTI_FTSR1_FT20          EXTI_FTSR1_FT20_Msk                           /*!< Falling trigger event configuration bit of line 20 */
#define EXTI_FTSR1_FT21_Pos      (21U)
#define EXTI_FTSR1_FT21_Msk      (0x1UL << EXTI_FTSR1_FT21_Pos)                /*!< 0x00200000 */
#define EXTI_FTSR1_FT21          EXTI_FTSR1_FT21_Msk                           /*!< Falling trigger event configuration bit of line 21 */

/******************  Bit definition for EXTI_SWIER1 register  *****************/
#define EXTI_SWIER1_SWI0_Pos     (0U)
#define EXTI_SWIER1_SWI0_Msk     (0x1UL << EXTI_SWIER1_SWI0_Pos)               /*!< 0x00000001 */
#define EXTI_SWIER1_SWI0         EXTI_SWIER1_SWI0_Msk                          /*!< Software Interrupt on line 0 */
#define EXTI_SWIER1_SWI1_Pos     (1U)
#define EXTI_SWIER1_SWI1_Msk     (0x1UL << EXTI_SWIER1_SWI1_Pos)               /*!< 0x00000002 */
#define EXTI_SWIER1_SWI1         EXTI_SWIER1_SWI1_Msk                          /*!< Software Interrupt on line 1 */
#define EXTI_SWIER1_SWI2_Pos     (2U)
#define EXTI_SWIER1_SWI2_Msk     (0x1UL << EXTI_SWIER1_SWI2_Pos)               /*!< 0x00000004 */
#define EXTI_SWIER1_SWI2         EXTI_SWIER1_SWI2_Msk                          /*!< Software Interrupt on line 2 */
#define EXTI_SWIER1_SWI3_Pos     (3U)
#define EXTI_SWIER1_SWI3_Msk     (0x1UL << EXTI_SWIER1_SWI3_Pos)               /*!< 0x00000008 */
#define EXTI_SWIER1_SWI3         EXTI_SWIER1_SWI3_Msk                          /*!< Software Interrupt on line 3 */
#define EXTI_SWIER1_SWI4_Pos     (4U)
#define EXTI_SWIER1_SWI4_Msk     (0x1UL << EXTI_SWIER1_SWI4_Pos)               /*!< 0x00000010 */
#define EXTI_SWIER1_SWI4         EXTI_SWIER1_SWI4_Msk                          /*!< Software Interrupt on line 4 */
#define EXTI_SWIER1_SWI5_Pos     (5U)
#define EXTI_SWIER1_SWI5_Msk     (0x1UL << EXTI_SWIER1_SWI5_Pos)               /*!< 0x00000020 */
#define EXTI_SWIER1_SWI5         EXTI_SWIER1_SWI5_Msk                          /*!< Software Interrupt on line 5 */
#define EXTI_SWIER1_SWI6_Pos     (6U)
#define EXTI_SWIER1_SWI6_Msk     (0x1UL << EXTI_SWIER1_SWI6_Pos)               /*!< 0x00000040 */
#define EXTI_SWIER1_SWI6         EXTI_SWIER1_SWI6_Msk                          /*!< Software Interrupt on line 6 */
#define EXTI_SWIER1_SWI7_Pos     (7U)
#define EXTI_SWIER1_SWI7_Msk     (0x1UL << EXTI_SWIER1_SWI7_Pos)               /*!< 0x00000080 */
#define EXTI_SWIER1_SWI7         EXTI_SWIER1_SWI7_Msk                          /*!< Software Interrupt on line 7 */
#define EXTI_SWIER1_SWI8_Pos     (8U)
#define EXTI_SWIER1_SWI8_Msk     (0x1UL << EXTI_SWIER1_SWI8_Pos)               /*!< 0x00000100 */
#define EXTI_SWIER1_SWI8         EXTI_SWIER1_SWI8_Msk                          /*!< Software Interrupt on line 8 */
#define EXTI_SWIER1_SWI9_Pos     (9U)
#define EXTI_SWIER1_SWI9_Msk     (0x1UL << EXTI_SWIER1_SWI9_Pos)               /*!< 0x00000200 */
#define EXTI_SWIER1_SWI9         EXTI_SWIER1_SWI9_Msk                          /*!< Software Interrupt on line 9 */
#define EXTI_SWIER1_SWI10_Pos    (10U)
#define EXTI_SWIER1_SWI10_Msk    (0x1UL << EXTI_SWIER1_SWI10_Pos)              /*!< 0x00000400 */
#define EXTI_SWIER1_SWI10        EXTI_SWIER1_SWI10_Msk                         /*!< Software Interrupt on line 10 */
#define EXTI_SWIER1_SWI11_Pos    (11U)
#define EXTI_SWIER1_SWI11_Msk    (0x1UL << EXTI_SWIER1_SWI11_Pos)              /*!< 0x00000800 */
#define EXTI_SWIER1_SWI11        EXTI_SWIER1_SWI11_Msk                         /*!< Software Interrupt on line 11 */
#define EXTI_SWIER1_SWI12_Pos    (12U)
#define EXTI_SWIER1_SWI12_Msk    (0x1UL << EXTI_SWIER1_SWI12_Pos)              /*!< 0x00001000 */
#define EXTI_SWIER1_SWI12        EXTI_SWIER1_SWI12_Msk                         /*!< Software Interrupt on line 12 */
#define EXTI_SWIER1_SWI13_Pos    (13U)
#define EXTI_SWIER1_SWI13_Msk    (0x1UL << EXTI_SWIER1_SWI13_Pos)              /*!< 0x00002000 */
#define EXTI_SWIER1_SWI13        EXTI_SWIER1_SWI13_Msk                         /*!< Software Interrupt on line 13 */
#define EXTI_SWIER1_SWI14_Pos    (14U)
#define EXTI_SWIER1_SWI14_Msk    (0x1UL << EXTI_SWIER1_SWI14_Pos)              /*!< 0x00004000 */
#define EXTI_SWIER1_SWI14        EXTI_SWIER1_SWI14_Msk                         /*!< Software Interrupt on line 14 */
#define EXTI_SWIER1_SWI15_Pos    (15U)
#define EXTI_SWIER1_SWI15_Msk    (0x1UL << EXTI_SWIER1_SWI15_Pos)              /*!< 0x00008000 */
#define EXTI_SWIER1_SWI15        EXTI_SWIER1_SWI15_Msk                         /*!< Software Interrupt on line 15 */
#define EXTI_SWIER1_SWI16_Pos    (16U)
#define EXTI_SWIER1_SWI16_Msk    (0x1UL << EXTI_SWIER1_SWI16_Pos)              /*!< 0x00010000 */
#define EXTI_SWIER1_SWI16        EXTI_SWIER1_SWI16_Msk                         /*!< Software Interrupt on line 16 */
#define EXTI_SWIER1_SWI18_Pos    (18U)
#define EXTI_SWIER1_SWI18_Msk    (0x1UL << EXTI_SWIER1_SWI18_Pos)              /*!< 0x00040000 */
#define EXTI_SWIER1_SWI18        EXTI_SWIER1_SWI18_Msk                         /*!< Software Interrupt on line 18 */
#define EXTI_SWIER1_SWI19_Pos    (19U)
#define EXTI_SWIER1_SWI19_Msk    (0x1UL << EXTI_SWIER1_SWI19_Pos)              /*!< 0x00080000 */
#define EXTI_SWIER1_SWI19        EXTI_SWIER1_SWI19_Msk                         /*!< Software Interrupt on line 19 */
#define EXTI_SWIER1_SWI20_Pos    (20U)
#define EXTI_SWIER1_SWI20_Msk    (0x1UL << EXTI_SWIER1_SWI20_Pos)              /*!< 0x00100000 */
#define EXTI_SWIER1_SWI20        EXTI_SWIER1_SWI20_Msk                         /*!< Software Interrupt on line 20 */
#define EXTI_SWIER1_SWI21_Pos    (21U)
#define EXTI_SWIER1_SWI21_Msk    (0x1UL << EXTI_SWIER1_SWI21_Pos)              /*!< 0x00200000 */
#define EXTI_SWIER1_SWI21        EXTI_SWIER1_SWI21_Msk                         /*!< Software Interrupt on line 21 */

/*******************  Bit definition for EXTI_PR1 register  *******************/
#define EXTI_PR1_PIF0_Pos        (0U)
#define EXTI_PR1_PIF0_Msk        (0x1UL << EXTI_PR1_PIF0_Pos)                  /*!< 0x00000001 */
#define EXTI_PR1_PIF0            EXTI_PR1_PIF0_Msk                             /*!< Pending bit for line 0 */
#define EXTI_PR1_PIF1_Pos        (1U)
#define EXTI_PR1_PIF1_Msk        (0x1UL << EXTI_PR1_PIF1_Pos)                  /*!< 0x00000002 */
#define EXTI_PR1_PIF1            EXTI_PR1_PIF1_Msk                             /*!< Pending bit for line 1 */
#define EXTI_PR1_PIF2_Pos        (2U)
#define EXTI_PR1_PIF2_Msk        (0x1UL << EXTI_PR1_PIF2_Pos)                  /*!< 0x00000004 */
#define EXTI_PR1_PIF2            EXTI_PR1_PIF2_Msk                             /*!< Pending bit for line 2 */
#define EXTI_PR1_PIF3_Pos        (3U)
#define EXTI_PR1_PIF3_Msk        (0x1UL << EXTI_PR1_PIF3_Pos)                  /*!< 0x00000008 */
#define EXTI_PR1_PIF3            EXTI_PR1_PIF3_Msk                             /*!< Pending bit for line 3 */
#define EXTI_PR1_PIF4_Pos        (4U)
#define EXTI_PR1_PIF4_Msk        (0x1UL << EXTI_PR1_PIF4_Pos)                  /*!< 0x00000010 */
#define EXTI_PR1_PIF4            EXTI_PR1_PIF4_Msk                             /*!< Pending bit for line 4 */
#define EXTI_PR1_PIF5_Pos        (5U)
#define EXTI_PR1_PIF5_Msk        (0x1UL << EXTI_PR1_PIF5_Pos)                  /*!< 0x00000020 */
#define EXTI_PR1_PIF5            EXTI_PR1_PIF5_Msk                             /*!< Pending bit for line 5 */
#define EXTI_PR1_PIF6_Pos        (6U)
#define EXTI_PR1_PIF6_Msk        (0x1UL << EXTI_PR1_PIF6_Pos)                  /*!< 0x00000040 */
#define EXTI_PR1_PIF6            EXTI_PR1_PIF6_Msk                             /*!< Pending bit for line 6 */
#define EXTI_PR1_PIF7_Pos        (7U)
#define EXTI_PR1_PIF7_Msk        (0x1UL << EXTI_PR1_PIF7_Pos)                  /*!< 0x00000080 */
#define EXTI_PR1_PIF7            EXTI_PR1_PIF7_Msk                             /*!< Pending bit for line 7 */
#define EXTI_PR1_PIF8_Pos        (8U)
#define EXTI_PR1_PIF8_Msk        (0x1UL << EXTI_PR1_PIF8_Pos)                  /*!< 0x00000100 */
#define EXTI_PR1_PIF8            EXTI_PR1_PIF8_Msk                             /*!< Pending bit for line 8 */
#define EXTI_PR1_PIF9_Pos        (9U)
#define EXTI_PR1_PIF9_Msk        (0x1UL << EXTI_PR1_PIF9_Pos)                  /*!< 0x00000200 */
#define EXTI_PR1_PIF9            EXTI_PR1_PIF9_Msk                             /*!< Pending bit for line 9 */
#define EXTI_PR1_PIF10_Pos       (10U)
#define EXTI_PR1_PIF10_Msk       (0x1UL << EXTI_PR1_PIF10_Pos)                 /*!< 0x00000400 */
#define EXTI_PR1_PIF10           EXTI_PR1_PIF10_Msk                            /*!< Pending bit for line 10 */
#define EXTI_PR1_PIF11_Pos       (11U)
#define EXTI_PR1_PIF11_Msk       (0x1UL << EXTI_PR1_PIF11_Pos)                 /*!< 0x00000800 */
#define EXTI_PR1_PIF11           EXTI_PR1_PIF11_Msk                            /*!< Pending bit for line 11 */
#define EXTI_PR1_PIF12_Pos       (12U)
#define EXTI_PR1_PIF12_Msk       (0x1UL << EXTI_PR1_PIF12_Pos)                 /*!< 0x00001000 */
#define EXTI_PR1_PIF12           EXTI_PR1_PIF12_Msk                            /*!< Pending bit for line 12 */
#define EXTI_PR1_PIF13_Pos       (13U)
#define EXTI_PR1_PIF13_Msk       (0x1UL << EXTI_PR1_PIF13_Pos)                 /*!< 0x00002000 */
#define EXTI_PR1_PIF13           EXTI_PR1_PIF13_Msk                            /*!< Pending bit for line 13 */
#define EXTI_PR1_PIF14_Pos       (14U)
#define EXTI_PR1_PIF14_Msk       (0x1UL << EXTI_PR1_PIF14_Pos)                 /*!< 0x00004000 */
#define EXTI_PR1_PIF14           EXTI_PR1_PIF14_Msk                            /*!< Pending bit for line 14 */
#define EXTI_PR1_PIF15_Pos       (15U)
#define EXTI_PR1_PIF15_Msk       (0x1UL << EXTI_PR1_PIF15_Pos)                 /*!< 0x00008000 */
#define EXTI_PR1_PIF15           EXTI_PR1_PIF15_Msk                            /*!< Pending bit for line 15 */
#define EXTI_PR1_PIF16_Pos       (16U)
#define EXTI_PR1_PIF16_Msk       (0x1UL << EXTI_PR1_PIF16_Pos)                 /*!< 0x00010000 */
#define EXTI_PR1_PIF16           EXTI_PR1_PIF16_Msk                            /*!< Pending bit for line 16 */
#define EXTI_PR1_PIF18_Pos       (18U)
#define EXTI_PR1_PIF18_Msk       (0x1UL << EXTI_PR1_PIF18_Pos)                 /*!< 0x00040000 */
#define EXTI_PR1_PIF18           EXTI_PR1_PIF18_Msk                            /*!< Pending bit for line 18 */
#define EXTI_PR1_PIF19_Pos       (19U)
#define EXTI_PR1_PIF19_Msk       (0x1UL << EXTI_PR1_PIF19_Pos)                 /*!< 0x00080000 */
#define EXTI_PR1_PIF19           EXTI_PR1_PIF19_Msk                            /*!< Pending bit for line 19 */
#define EXTI_PR1_PIF20_Pos       (20U)
#define EXTI_PR1_PIF20_Msk       (0x1UL << EXTI_PR1_PIF20_Pos)                 /*!< 0x00100000 */
#define EXTI_PR1_PIF20           EXTI_PR1_PIF20_Msk                            /*!< Pending bit for line 20 */
#define EXTI_PR1_PIF21_Pos       (21U)
#define EXTI_PR1_PIF21_Msk       (0x1UL << EXTI_PR1_PIF21_Pos)                 /*!< 0x00200000 */
#define EXTI_PR1_PIF21           EXTI_PR1_PIF21_Msk                            /*!< Pending bit for line 21 */

/*******************  Bit definition for EXTI_IMR2 register  ******************/
#define EXTI_IMR2_IM32_Pos       (0U)
#define EXTI_IMR2_IM32_Msk       (0x1UL << EXTI_IMR2_IM32_Pos)                 /*!< 0x00000001 */
#define EXTI_IMR2_IM32           EXTI_IMR2_IM32_Msk                            /*!< Interrupt Mask on line 32 */
#define EXTI_IMR2_IM33_Pos       (1U)
#define EXTI_IMR2_IM33_Msk       (0x1UL << EXTI_IMR2_IM33_Pos)                 /*!< 0x00000002 */
#define EXTI_IMR2_IM33           EXTI_IMR2_IM33_Msk                            /*!< Interrupt Mask on line 33 */
#define EXTI_IMR2_IM35_Pos       (3U)
#define EXTI_IMR2_IM35_Msk       (0x1UL << EXTI_IMR2_IM35_Pos)                 /*!< 0x00000008 */
#define EXTI_IMR2_IM35           EXTI_IMR2_IM35_Msk                            /*!< Interrupt Mask on line 35 */
#define EXTI_IMR2_IM37_Pos       (5U)
#define EXTI_IMR2_IM37_Msk       (0x1UL << EXTI_IMR2_IM37_Pos)                 /*!< 0x00000020 */
#define EXTI_IMR2_IM37           EXTI_IMR2_IM37_Msk                            /*!< Interrupt Mask on line 37 */
#define EXTI_IMR2_IM38_Pos       (6U)
#define EXTI_IMR2_IM38_Msk       (0x1UL << EXTI_IMR2_IM38_Pos)                 /*!< 0x00000040 */
#define EXTI_IMR2_IM38           EXTI_IMR2_IM38_Msk                            /*!< Interrupt Mask on line 38 */
#define EXTI_IMR2_IM_Pos         (0U)
#define EXTI_IMR2_IM_Msk         (0x6BUL << EXTI_IMR2_IM_Pos)                   /*!< 0x0000006B */
#define EXTI_IMR2_IM             EXTI_IMR2_IM_Msk                              /*!< Interrupt Mask all        */

/*******************  Bit definition for EXTI_EMR2 register  ******************/
#define EXTI_EMR2_EM32_Pos       (0U)
#define EXTI_EMR2_EM32_Msk       (0x1UL << EXTI_EMR2_EM32_Pos)                 /*!< 0x00000001 */
#define EXTI_EMR2_EM32           EXTI_EMR2_EM32_Msk                            /*!< Event Mask on line 32 */
#define EXTI_EMR2_EM33_Pos       (1U)
#define EXTI_EMR2_EM33_Msk       (0x1UL << EXTI_EMR2_EM33_Pos)                 /*!< 0x00000002 */
#define EXTI_EMR2_EM33           EXTI_EMR2_EM33_Msk                            /*!< Event Mask on line 33 */
#define EXTI_EMR2_EM35_Pos       (3U)
#define EXTI_EMR2_EM35_Msk       (0x1UL << EXTI_EMR2_EM35_Pos)                 /*!< 0x00000008 */
#define EXTI_EMR2_EM35           EXTI_EMR2_EM35_Msk                            /*!< Event Mask on line 35 */
#define EXTI_EMR2_EM37_Pos       (5U)
#define EXTI_EMR2_EM37_Msk       (0x1UL << EXTI_EMR2_EM37_Pos)                 /*!< 0x00000020 */
#define EXTI_EMR2_EM37           EXTI_EMR2_EM37_Msk                            /*!< Event Mask on line 37 */
#define EXTI_EMR2_EM38_Pos       (6U)
#define EXTI_EMR2_EM38_Msk       (0x1UL << EXTI_EMR2_EM38_Pos)                 /*!< 0x00000040 */
#define EXTI_EMR2_EM38           EXTI_EMR2_EM38_Msk                            /*!< Event Mask on line 38 */
#define EXTI_EMR2_EM_Pos         (0U)
#define EXTI_EMR2_EM_Msk         (0x6BUL << EXTI_EMR2_EM_Pos)                   /*!< 0x0000006B */
#define EXTI_EMR2_EM             EXTI_EMR2_EM_Msk                              /*!< Interrupt Mask all        */

/******************  Bit definition for EXTI_RTSR2 register  ******************/
#define EXTI_RTSR2_RT35_Pos      (3U)
#define EXTI_RTSR2_RT35_Msk      (0x1UL << EXTI_RTSR2_RT35_Pos)                /*!< 0x00000008 */
#define EXTI_RTSR2_RT35          EXTI_RTSR2_RT35_Msk                           /*!< Rising trigger event configuration bit of line 35 */
#define EXTI_RTSR2_RT37_Pos      (5U)
#define EXTI_RTSR2_RT37_Msk      (0x1UL << EXTI_RTSR2_RT37_Pos)                /*!< 0x00000020 */
#define EXTI_RTSR2_RT37          EXTI_RTSR2_RT37_Msk                           /*!< Rising trigger event configuration bit of line 37 */
#define EXTI_RTSR2_RT38_Pos      (6U)
#define EXTI_RTSR2_RT38_Msk      (0x1UL << EXTI_RTSR2_RT38_Pos)                /*!< 0x00000040 */
#define EXTI_RTSR2_RT38          EXTI_RTSR2_RT38_Msk                           /*!< Rising trigger event configuration bit of line 38 */

/******************  Bit definition for EXTI_FTSR2 register  ******************/
#define EXTI_FTSR2_FT35_Pos      (3U)
#define EXTI_FTSR2_FT35_Msk      (0x1UL << EXTI_FTSR2_FT35_Pos)                /*!< 0x00000008 */
#define EXTI_FTSR2_FT35          EXTI_FTSR2_FT35_Msk                           /*!< Falling trigger event configuration bit of line 35 */
#define EXTI_FTSR2_FT37_Pos      (5U)
#define EXTI_FTSR2_FT37_Msk      (0x1UL << EXTI_FTSR2_FT37_Pos)                /*!< 0x00000020 */
#define EXTI_FTSR2_FT37          EXTI_FTSR2_FT37_Msk                           /*!< Falling trigger event configuration bit of line 37 */
#define EXTI_FTSR2_FT38_Pos      (6U)
#define EXTI_FTSR2_FT38_Msk      (0x1UL << EXTI_FTSR2_FT38_Pos)                /*!< 0x00000040 */
#define EXTI_FTSR2_FT38          EXTI_FTSR2_FT38_Msk                           /*!< Falling trigger event configuration bit of line 38 */

/******************  Bit definition for EXTI_SWIER2 register  *****************/
#define EXTI_SWIER2_SWI35_Pos    (3U)
#define EXTI_SWIER2_SWI35_Msk    (0x1UL << EXTI_SWIER2_SWI35_Pos)              /*!< 0x00000008 */
#define EXTI_SWIER2_SWI35        EXTI_SWIER2_SWI35_Msk                         /*!< Software Interrupt on line 35 */
#define EXTI_SWIER2_SWI37_Pos    (5U)
#define EXTI_SWIER2_SWI37_Msk    (0x1UL << EXTI_SWIER2_SWI37_Pos)              /*!< 0x00000020 */
#define EXTI_SWIER2_SWI37        EXTI_SWIER2_SWI37_Msk                         /*!< Software Interrupt on line 37 */
#define EXTI_SWIER2_SWI38_Pos    (6U)
#define EXTI_SWIER2_SWI38_Msk    (0x1UL << EXTI_SWIER2_SWI38_Pos)              /*!< 0x00000040 */
#define EXTI_SWIER2_SWI38        EXTI_SWIER2_SWI38_Msk                         /*!< Software Interrupt on line 38 */

/*******************  Bit definition for EXTI_PR2 register  *******************/
#define EXTI_PR2_PIF35_Pos       (3U)
#define EXTI_PR2_PIF35_Msk       (0x1UL << EXTI_PR2_PIF35_Pos)                 /*!< 0x00000008 */
#define EXTI_PR2_PIF35           EXTI_PR2_PIF35_Msk                            /*!< Pending bit for line 35 */
#define EXTI_PR2_PIF37_Pos       (5U)
#define EXTI_PR2_PIF37_Msk       (0x1UL << EXTI_PR2_PIF37_Pos)                 /*!< 0x00000020 */
#define EXTI_PR2_PIF37           EXTI_PR2_PIF37_Msk                            /*!< Pending bit for line 37 */
#define EXTI_PR2_PIF38_Pos       (6U)
#define EXTI_PR2_PIF38_Msk       (0x1UL << EXTI_PR2_PIF38_Pos)                 /*!< 0x00000040 */
#define EXTI_PR2_PIF38           EXTI_PR2_PIF38_Msk                            /*!< Pending bit for line 38 */


/******************************************************************************/
/*                                                                            */
/*                                    FLASH                                   */
/*                                                                            */
/******************************************************************************/
/*******************  Bits definition for FLASH_ACR register  *****************/
#define FLASH_ACR_LATENCY_Pos             (0U)
#define FLASH_ACR_LATENCY_Msk             (0x7UL << FLASH_ACR_LATENCY_Pos)     /*!< 0x00000007 */
#define FLASH_ACR_LATENCY                 FLASH_ACR_LATENCY_Msk
#define FLASH_ACR_LATENCY_0WS             (0x00000000UL)
#define FLASH_ACR_LATENCY_1WS             (0x00000001UL)
#define FLASH_ACR_LATENCY_2WS             (0x00000002UL)
#define FLASH_ACR_LATENCY_3WS             (0x00000003UL)
#define FLASH_ACR_LATENCY_4WS             (0x00000004UL)
#define FLASH_ACR_PRFTEN_Pos              (8U)
#define FLASH_ACR_PRFTEN_Msk              (0x1UL << FLASH_ACR_PRFTEN_Pos)      /*!< 0x00000100 */
#define FLASH_ACR_PRFTEN                  FLASH_ACR_PRFTEN_Msk
#define FLASH_ACR_ICEN_Pos                (9U)
#define FLASH_ACR_ICEN_Msk                (0x1UL << FLASH_ACR_ICEN_Pos)        /*!< 0x00000200 */
#define FLASH_ACR_ICEN                    FLASH_ACR_ICEN_Msk
#define FLASH_ACR_DCEN_Pos                (10U)
#define FLASH_ACR_DCEN_Msk                (0x1UL << FLASH_ACR_DCEN_Pos)        /*!< 0x00000400 */
#define FLASH_ACR_DCEN                    FLASH_ACR_DCEN_Msk
#define FLASH_ACR_ICRST_Pos               (11U)
#define FLASH_ACR_ICRST_Msk               (0x1UL << FLASH_ACR_ICRST_Pos)       /*!< 0x00000800 */
#define FLASH_ACR_ICRST                   FLASH_ACR_ICRST_Msk
#define FLASH_ACR_DCRST_Pos               (12U)
#define FLASH_ACR_DCRST_Msk               (0x1UL << FLASH_ACR_DCRST_Pos)       /*!< 0x00001000 */
#define FLASH_ACR_DCRST                   FLASH_ACR_DCRST_Msk
#define FLASH_ACR_RUN_PD_Pos              (13U)
#define FLASH_ACR_RUN_PD_Msk              (0x1UL << FLASH_ACR_RUN_PD_Pos)      /*!< 0x00002000 */
#define FLASH_ACR_RUN_PD                  FLASH_ACR_RUN_PD_Msk                 /*!< Flash power down mode during run */
#define FLASH_ACR_SLEEP_PD_Pos            (14U)
#define FLASH_ACR_SLEEP_PD_Msk            (0x1UL << FLASH_ACR_SLEEP_PD_Pos)    /*!< 0x00004000 */
#define FLASH_ACR_SLEEP_PD                FLASH_ACR_SLEEP_PD_Msk               /*!< Flash power down mode during sleep */

/*******************  Bits definition for FLASH_SR register  ******************/
#define FLASH_SR_EOP_Pos                  (0U)
#define FLASH_SR_EOP_Msk                  (0x1UL << FLASH_SR_EOP_Pos)          /*!< 0x00000001 */
#define FLASH_SR_EOP                      FLASH_SR_EOP_Msk
#define FLASH_SR_OPERR_Pos                (1U)
#define FLASH_SR_OPERR_Msk                (0x1UL << FLASH_SR_OPERR_Pos)        /*!< 0x00000002 */
#define FLASH_SR_OPERR                    FLASH_SR_OPERR_Msk
#define FLASH_SR_PROGERR_Pos              (3U)
#define FLASH_SR_PROGERR_Msk              (0x1UL << FLASH_SR_PROGERR_Pos)      /*!< 0x00000008 */
#define FLASH_SR_PROGERR                  FLASH_SR_PROGERR_Msk
#define FLASH_SR_WRPERR_Pos               (4U)
#define FLASH_SR_WRPERR_Msk               (0x1UL << FLASH_SR_WRPERR_Pos)       /*!< 0x00000010 */
#define FLASH_SR_WRPERR                   FLASH_SR_WRPERR_Msk
#define FLASH_SR_PGAERR_Pos               (5U)
#define FLASH_SR_PGAERR_Msk               (0x1UL << FLASH_SR_PGAERR_Pos)       /*!< 0x00000020 */
#define FLASH_SR_PGAERR                   FLASH_SR_PGAERR_Msk
#define FLASH_SR_SIZERR_Pos               (6U)
#define FLASH_SR_SIZERR_Msk               (0x1UL << FLASH_SR_SIZERR_Pos)       /*!< 0x00000040 */
#define FLASH_SR_SIZERR                   FLASH_SR_SIZERR_Msk
#define FLASH_SR_PGSERR_Pos               (7U)
#define FLASH_SR_PGSERR_Msk               (0x1UL << FLASH_SR_PGSERR_Pos)       /*!< 0x00000080 */
#define FLASH_SR_PGSERR                   FLASH_SR_PGSERR_Msk
#define FLASH_SR_MISERR_Pos               (8U)
#define FLASH_SR_MISERR_Msk               (0x1UL << FLASH_SR_MISERR_Pos)       /*!< 0x00000100 */
#define FLASH_SR_MISERR                   FLASH_SR_MISERR_Msk
#define FLASH_SR_FASTERR_Pos              (9U)
#define FLASH_SR_FASTERR_Msk              (0x1UL << FLASH_SR_FASTERR_Pos)      /*!< 0x00000200 */
#define FLASH_SR_FASTERR                  FLASH_SR_FASTERR_Msk
#define FLASH_SR_RDERR_Pos                (14U)
#define FLASH_SR_RDERR_Msk                (0x1UL << FLASH_SR_RDERR_Pos)        /*!< 0x00004000 */
#define FLASH_SR_RDERR                    FLASH_SR_RDERR_Msk
#define FLASH_SR_OPTVERR_Pos              (15U)
#define FLASH_SR_OPTVERR_Msk              (0x1UL << FLASH_SR_OPTVERR_Pos)      /*!< 0x00008000 */
#define FLASH_SR_OPTVERR                  FLASH_SR_OPTVERR_Msk
#define FLASH_SR_BSY_Pos                  (16U)
#define FLASH_SR_BSY_Msk                  (0x1UL << FLASH_SR_BSY_Pos)          /*!< 0x00010000 */
#define FLASH_SR_BSY                      FLASH_SR_BSY_Msk
#define FLASH_SR_PEMPTY_Pos               (17U)
#define FLASH_SR_PEMPTY_Msk               (0x1UL << FLASH_SR_PEMPTY_Pos)       /*!< 0x00020000 */
#define FLASH_SR_PEMPTY                   FLASH_SR_PEMPTY_Msk

/*******************  Bits definition for FLASH_CR register  ******************/
#define FLASH_CR_PG_Pos                   (0U)
#define FLASH_CR_PG_Msk                   (0x1UL << FLASH_CR_PG_Pos)           /*!< 0x00000001 */
#define FLASH_CR_PG                       FLASH_CR_PG_Msk
#define FLASH_CR_PER_Pos                  (1U)
#define FLASH_CR_PER_Msk                  (0x1UL << FLASH_CR_PER_Pos)          /*!< 0x00000002 */
#define FLASH_CR_PER                      FLASH_CR_PER_Msk
#define FLASH_CR_MER1_Pos                 (2U)
#define FLASH_CR_MER1_Msk                 (0x1UL << FLASH_CR_MER1_Pos)         /*!< 0x00000004 */
#define FLASH_CR_MER1                     FLASH_CR_MER1_Msk
#define FLASH_CR_PNB_Pos                  (3U)
#define FLASH_CR_PNB_Msk                  (0x3FUL << FLASH_CR_PNB_Pos)         /*!< 0x000001F8 */
#define FLASH_CR_PNB                      FLASH_CR_PNB_Msk
#define FLASH_CR_STRT_Pos                 (16U)
#define FLASH_CR_STRT_Msk                 (0x1UL << FLASH_CR_STRT_Pos)         /*!< 0x00010000 */
#define FLASH_CR_STRT                     FLASH_CR_STRT_Msk
#define FLASH_CR_OPTSTRT_Pos              (17U)
#define FLASH_CR_OPTSTRT_Msk              (0x1UL << FLASH_CR_OPTSTRT_Pos)      /*!< 0x00020000 */
#define FLASH_CR_OPTSTRT                  FLASH_CR_OPTSTRT_Msk
#define FLASH_CR_FSTPG_Pos                (18U)
#define FLASH_CR_FSTPG_Msk                (0x1UL << FLASH_CR_FSTPG_Pos)        /*!< 0x00040000 */
#define FLASH_CR_FSTPG                    FLASH_CR_FSTPG_Msk
#define FLASH_CR_EOPIE_Pos                (24U)
#define FLASH_CR_EOPIE_Msk                (0x1UL << FLASH_CR_EOPIE_Pos)        /*!< 0x01000000 */
#define FLASH_CR_EOPIE                    FLASH_CR_EOPIE_Msk
#define FLASH_CR_ERRIE_Pos                (25U)
#define FLASH_CR_ERRIE_Msk                (0x1UL << FLASH_CR_ERRIE_Pos)        /*!< 0x02000000 */
#define FLASH_CR_ERRIE                    FLASH_CR_ERRIE_Msk
#define FLASH_CR_RDERRIE_Pos              (26U)
#define FLASH_CR_RDERRIE_Msk              (0x1UL << FLASH_CR_RDERRIE_Pos)      /*!< 0x04000000 */
#define FLASH_CR_RDERRIE                  FLASH_CR_RDERRIE_Msk
#define FLASH_CR_OBL_LAUNCH_Pos           (27U)
#define FLASH_CR_OBL_LAUNCH_Msk           (0x1UL << FLASH_CR_OBL_LAUNCH_Pos)   /*!< 0x08000000 */
#define FLASH_CR_OBL_LAUNCH               FLASH_CR_OBL_LAUNCH_Msk
#define FLASH_CR_OPTLOCK_Pos              (30U)
#define FLASH_CR_OPTLOCK_Msk              (0x1UL << FLASH_CR_OPTLOCK_Pos)      /*!< 0x40000000 */
#define FLASH_CR_OPTLOCK                  FLASH_CR_OPTLOCK_Msk
#define FLASH_CR_LOCK_Pos                 (31U)
#define FLASH_CR_LOCK_Msk                 (0x1UL << FLASH_CR_LOCK_Pos)         /*!< 0x80000000 */
#define FLASH_CR_LOCK                     FLASH_CR_LOCK_Msk

/*******************  Bits definition for FLASH_ECCR register  ***************/
#define FLASH_ECCR_ADDR_ECC_Pos           (0U)
#define FLASH_ECCR_ADDR_ECC_Msk           (0x7FFFFUL << FLASH_ECCR_ADDR_ECC_Pos) /*!< 0x0007FFFF */
#define FLASH_ECCR_ADDR_ECC               FLASH_ECCR_ADDR_ECC_Msk
#define FLASH_ECCR_SYSF_ECC_Pos           (20U)
#define FLASH_ECCR_SYSF_ECC_Msk           (0x1UL << FLASH_ECCR_SYSF_ECC_Pos)   /*!< 0x00100000 */
#define FLASH_ECCR_SYSF_ECC               FLASH_ECCR_SYSF_ECC_Msk
#define FLASH_ECCR_ECCIE_Pos              (24U)
#define FLASH_ECCR_ECCIE_Msk              (0x1UL << FLASH_ECCR_ECCIE_Pos)      /*!< 0x01000000 */
#define FLASH_ECCR_ECCIE                  FLASH_ECCR_ECCIE_Msk
#define FLASH_ECCR_ECCC_Pos               (30U)
#define FLASH_ECCR_ECCC_Msk               (0x1UL << FLASH_ECCR_ECCC_Pos)       /*!< 0x40000000 */
#define FLASH_ECCR_ECCC                   FLASH_ECCR_ECCC_Msk
#define FLASH_ECCR_ECCD_Pos               (31U)
#define FLASH_ECCR_ECCD_Msk               (0x1UL << FLASH_ECCR_ECCD_Pos)       /*!< 0x80000000 */
#define FLASH_ECCR_ECCD                   FLASH_ECCR_ECCD_Msk

/*******************  Bits definition for FLASH_OPTR register  ***************/
#define FLASH_OPTR_RDP_Pos                (0U)
#define FLASH_OPTR_RDP_Msk                (0xFFUL << FLASH_OPTR_RDP_Pos)       /*!< 0x000000FF */
#define FLASH_OPTR_RDP                    FLASH_OPTR_RDP_Msk
#define FLASH_OPTR_BOR_LEV_Pos            (8U)
#define FLASH_OPTR_BOR_LEV_Msk            (0x7UL << FLASH_OPTR_BOR_LEV_Pos)    /*!< 0x00000700 */
#define FLASH_OPTR_BOR_LEV                FLASH_OPTR_BOR_LEV_Msk
#define FLASH_OPTR_BOR_LEV_0              (0x0UL << FLASH_OPTR_BOR_LEV_Pos)    /*!< 0x00000000 */
#define FLASH_OPTR_BOR_LEV_1              (0x1UL << FLASH_OPTR_BOR_LEV_Pos)    /*!< 0x00000100 */
#define FLASH_OPTR_BOR_LEV_2              (0x2UL << FLASH_OPTR_BOR_LEV_Pos)    /*!< 0x00000200 */
#define FLASH_OPTR_BOR_LEV_3              (0x3UL << FLASH_OPTR_BOR_LEV_Pos)    /*!< 0x00000300 */
#define FLASH_OPTR_BOR_LEV_4              (0x4UL << FLASH_OPTR_BOR_LEV_Pos)    /*!< 0x00000400 */
#define FLASH_OPTR_nRST_STOP_Pos          (12U)
#define FLASH_OPTR_nRST_STOP_Msk          (0x1UL << FLASH_OPTR_nRST_STOP_Pos)  /*!< 0x00001000 */
#define FLASH_OPTR_nRST_STOP              FLASH_OPTR_nRST_STOP_Msk
#define FLASH_OPTR_nRST_STDBY_Pos         (13U)
#define FLASH_OPTR_nRST_STDBY_Msk         (0x1UL << FLASH_OPTR_nRST_STDBY_Pos) /*!< 0x00002000 */
#define FLASH_OPTR_nRST_STDBY             FLASH_OPTR_nRST_STDBY_Msk
#define FLASH_OPTR_nRST_SHDW_Pos          (14U)
#define FLASH_OPTR_nRST_SHDW_Msk          (0x1UL << FLASH_OPTR_nRST_SHDW_Pos)  /*!< 0x00004000 */
#define FLASH_OPTR_nRST_SHDW              FLASH_OPTR_nRST_SHDW_Msk
#define FLASH_OPTR_IWDG_SW_Pos            (16U)
#define FLASH_OPTR_IWDG_SW_Msk            (0x1UL << FLASH_OPTR_IWDG_SW_Pos)    /*!< 0x00010000 */
#define FLASH_OPTR_IWDG_SW                FLASH_OPTR_IWDG_SW_Msk
#define FLASH_OPTR_IWDG_STOP_Pos          (17U)
#define FLASH_OPTR_IWDG_STOP_Msk          (0x1UL << FLASH_OPTR_IWDG_STOP_Pos)  /*!< 0x00020000 */
#define FLASH_OPTR_IWDG_STOP              FLASH_OPTR_IWDG_STOP_Msk
#define FLASH_OPTR_IWDG_STDBY_Pos         (18U)
#define FLASH_OPTR_IWDG_STDBY_Msk         (0x1UL << FLASH_OPTR_IWDG_STDBY_Pos) /*!< 0x00040000 */
#define FLASH_OPTR_IWDG_STDBY             FLASH_OPTR_IWDG_STDBY_Msk
#define FLASH_OPTR_WWDG_SW_Pos            (19U)
#define FLASH_OPTR_WWDG_SW_Msk            (0x1UL << FLASH_OPTR_WWDG_SW_Pos)    /*!< 0x00080000 */
#define FLASH_OPTR_WWDG_SW                FLASH_OPTR_WWDG_SW_Msk
#define FLASH_OPTR_nBOOT1_Pos             (23U)
#define FLASH_OPTR_nBOOT1_Msk             (0x1UL << FLASH_OPTR_nBOOT1_Pos)     /*!< 0x00800000 */
#define FLASH_OPTR_nBOOT1                 FLASH_OPTR_nBOOT1_Msk
#define FLASH_OPTR_SRAM2_PE_Pos           (24U)
#define FLASH_OPTR_SRAM2_PE_Msk           (0x1UL << FLASH_OPTR_SRAM2_PE_Pos)   /*!< 0x01000000 */
#define FLASH_OPTR_SRAM2_PE               FLASH_OPTR_SRAM2_PE_Msk
#define FLASH_OPTR_SRAM2_RST_Pos          (25U)
#define FLASH_OPTR_SRAM2_RST_Msk          (0x1UL << FLASH_OPTR_SRAM2_RST_Pos)  /*!< 0x02000000 */
#define FLASH_OPTR_SRAM2_RST              FLASH_OPTR_SRAM2_RST_Msk
#define FLASH_OPTR_nSWBOOT0_Pos           (26U)
#define FLASH_OPTR_nSWBOOT0_Msk           (0x1UL << FLASH_OPTR_nSWBOOT0_Pos)   /*!< 0x04000000 */
#define FLASH_OPTR_nSWBOOT0               FLASH_OPTR_nSWBOOT0_Msk
#define FLASH_OPTR_nBOOT0_Pos             (27U)
#define FLASH_OPTR_nBOOT0_Msk             (0x1UL << FLASH_OPTR_nBOOT0_Pos)     /*!< 0x08000000 */
#define FLASH_OPTR_nBOOT0                 FLASH_OPTR_nBOOT0_Msk

/******************  Bits definition for FLASH_PCROP1SR register  **********/
#define FLASH_PCROP1SR_PCROP1_STRT_Pos    (0U)
#define FLASH_PCROP1SR_PCROP1_STRT_Msk    (0x3FFFUL << FLASH_PCROP1SR_PCROP1_STRT_Pos) /*!< 0x00003FFF */
#define FLASH_PCROP1SR_PCROP1_STRT        FLASH_PCROP1SR_PCROP1_STRT_Msk

/******************  Bits definition for FLASH_PCROP1ER register  ***********/
#define FLASH_PCROP1ER_PCROP1_END_Pos     (0U)
#define FLASH_PCROP1ER_PCROP1_END_Msk     (0x3FFFUL << FLASH_PCROP1ER_PCROP1_END_Pos) /*!< 0x00003FFF */
#define FLASH_PCROP1ER_PCROP1_END         FLASH_PCROP1ER_PCROP1_END_Msk
#define FLASH_PCROP1ER_PCROP_RDP_Pos      (31U)
#define FLASH_PCROP1ER_PCROP_RDP_Msk      (0x1UL << FLASH_PCROP1ER_PCROP_RDP_Pos) /*!< 0x80000000 */
#define FLASH_PCROP1ER_PCROP_RDP          FLASH_PCROP1ER_PCROP_RDP_Msk

/******************  Bits definition for FLASH_WRP1AR register  ***************/
#define FLASH_WRP1AR_WRP1A_STRT_Pos       (0U)
#define FLASH_WRP1AR_WRP1A_STRT_Msk       (0x3FUL << FLASH_WRP1AR_WRP1A_STRT_Pos) /*!< 0x0000003F */
#define FLASH_WRP1AR_WRP1A_STRT           FLASH_WRP1AR_WRP1A_STRT_Msk
#define FLASH_WRP1AR_WRP1A_END_Pos        (16U)
#define FLASH_WRP1AR_WRP1A_END_Msk        (0x3FUL << FLASH_WRP1AR_WRP1A_END_Pos)  /*!< 0x003F0000 */
#define FLASH_WRP1AR_WRP1A_END            FLASH_WRP1AR_WRP1A_END_Msk

/******************  Bits definition for FLASH_WRPB1R register  ***************/
#define FLASH_WRP1BR_WRP1B_STRT_Pos       (0U)
#define FLASH_WRP1BR_WRP1B_STRT_Msk       (0x3FUL << FLASH_WRP1BR_WRP1B_STRT_Pos) /*!< 0x0000003F */
#define FLASH_WRP1BR_WRP1B_STRT           FLASH_WRP1BR_WRP1B_STRT_Msk
#define FLASH_WRP1BR_WRP1B_END_Pos        (16U)
#define FLASH_WRP1BR_WRP1B_END_Msk        (0x3FUL << FLASH_WRP1BR_WRP1B_END_Pos)  /*!< 0x003F0000 */
#define FLASH_WRP1BR_WRP1B_END            FLASH_WRP1BR_WRP1B_END_Msk




/******************************************************************************/
/*                                                                            */
/*                       General Purpose IOs (GPIO)                           */
/*                                                                            */
/******************************************************************************/
/******************  Bits definition for GPIO_MODER register  *****************/
#define GPIO_MODER_MODE0_Pos           (0U)
#define GPIO_MODER_MODE0_Msk           (0x3UL << GPIO_MODER_MODE0_Pos)         /*!< 0x00000003 */
#define GPIO_MODER_MODE0               GPIO_MODER_MODE0_Msk
#define GPIO_MODER_MODE0_0             (0x1UL << GPIO_MODER_MODE0_Pos)         /*!< 0x00000001 */
#define GPIO_MODER_MODE0_1             (0x2UL << GPIO_MODER_MODE0_Pos)         /*!< 0x00000002 */
#define GPIO_MODER_MODE1_Pos           (2U)
#define GPIO_MODER_MODE1_Msk           (0x3UL << GPIO_MODER_MODE1_Pos)         /*!< 0x0000000C */
#define GPIO_MODER_MODE1               GPIO_MODER_MODE1_Msk
#define GPIO_MODER_MODE1_0             (0x1UL << GPIO_MODER_MODE1_Pos)         /*!< 0x00000004 */
#define GPIO_MODER_MODE1_1             (0x2UL << GPIO_MODER_MODE1_Pos)         /*!< 0x00000008 */
#define GPIO_MODER_MODE2_Pos           (4U)
#define GPIO_MODER_MODE2_Msk           (0x3UL << GPIO_MODER_MODE2_Pos)         /*!< 0x00000030 */
#define GPIO_MODER_MODE2               GPIO_MODER_MODE2_Msk
#define GPIO_MODER_MODE2_0             (0x1UL << GPIO_MODER_MODE2_Pos)         /*!< 0x00000010 */
#define GPIO_MODER_MODE2_1             (0x2UL << GPIO_MODER_MODE2_Pos)         /*!< 0x00000020 */
#define GPIO_MODER_MODE3_Pos           (6U)
#define GPIO_MODER_MODE3_Msk           (0x3UL << GPIO_MODER_MODE3_Pos)         /*!< 0x000000C0 */
#define GPIO_MODER_MODE3               GPIO_MODER_MODE3_Msk
#define GPIO_MODER_MODE3_0             (0x1UL << GPIO_MODER_MODE3_Pos)         /*!< 0x00000040 */
#define GPIO_MODER_MODE3_1             (0x2UL << GPIO_MODER_MODE3_Pos)         /*!< 0x00000080 */
#define GPIO_MODER_MODE4_Pos           (8U)
#define GPIO_MODER_MODE4_Msk           (0x3UL << GPIO_MODER_MODE4_Pos)         /*!< 0x00000300 */
#define GPIO_MODER_MODE4               GPIO_MODER_MODE4_Msk
#define GPIO_MODER_MODE4_0             (0x1UL << GPIO_MODER_MODE4_Pos)         /*!< 0x00000100 */
#define GPIO_MODER_MODE4_1             (0x2UL << GPIO_MODER_MODE4_Pos)         /*!< 0x00000200 */
#define GPIO_MODER_MODE5_Pos           (10U)
#define GPIO_MODER_MODE5_Msk           (0x3UL << GPIO_MODER_MODE5_Pos)         /*!< 0x00000C00 */
#define GPIO_MODER_MODE5               GPIO_MODER_MODE5_Msk
#define GPIO_MODER_MODE5_0             (0x1UL << GPIO_MODER_MODE5_Pos)         /*!< 0x00000400 */
#define GPIO_MODER_MODE5_1             (0x2UL << GPIO_MODER_MODE5_Pos)         /*!< 0x00000800 */
#define GPIO_MODER_MODE6_Pos           (12U)
#define GPIO_MODER_MODE6_Msk           (0x3UL << GPIO_MODER_MODE6_Pos)         /*!< 0x00003000 */
#define GPIO_MODER_MODE6               GPIO_MODER_MODE6_Msk
#define GPIO_MODER_MODE6_0             (0x1UL << GPIO_MODER_MODE6_Pos)         /*!< 0x00001000 */
#define GPIO_MODER_MODE6_1             (0x2UL << GPIO_MODER_MODE6_Pos)         /*!< 0x00002000 */
#define GPIO_MODER_MODE7_Pos           (14U)
#define GPIO_MODER_MODE7_Msk           (0x3UL << GPIO_MODER_MODE7_Pos)         /*!< 0x0000C000 */
#define GPIO_MODER_MODE7               GPIO_MODER_MODE7_Msk
#define GPIO_MODER_MODE7_0             (0x1UL << GPIO_MODER_MODE7_Pos)         /*!< 0x00004000 */
#define GPIO_MODER_MODE7_1             (0x2UL << GPIO_MODER_MODE7_Pos)         /*!< 0x00008000 */
#define GPIO_MODER_MODE8_Pos           (16U)
#define GPIO_MODER_MODE8_Msk           (0x3UL << GPIO_MODER_MODE8_Pos)         /*!< 0x00030000 */
#define GPIO_MODER_MODE8               GPIO_MODER_MODE8_Msk
#define GPIO_MODER_MODE8_0             (0x1UL << GPIO_MODER_MODE8_Pos)         /*!< 0x00010000 */
#define GPIO_MODER_MODE8_1             (0x2UL << GPIO_MODER_MODE8_Pos)         /*!< 0x00020000 */
#define GPIO_MODER_MODE9_Pos           (18U)
#define GPIO_MODER_MODE9_Msk           (0x3UL << GPIO_MODER_MODE9_Pos)         /*!< 0x000C0000 */
#define GPIO_MODER_MODE9               GPIO_MODER_MODE9_Msk
#define GPIO_MODER_MODE9_0             (0x1UL << GPIO_MODER_MODE9_Pos)         /*!< 0x00040000 */
#define GPIO_MODER_MODE9_1             (0x2UL << GPIO_MODER_MODE9_Pos)         /*!< 0x00080000 */
#define GPIO_MODER_MODE10_Pos          (20U)
#define GPIO_MODER_MODE10_Msk          (0x3UL << GPIO_MODER_MODE10_Pos)        /*!< 0x00300000 */
#define GPIO_MODER_MODE10              GPIO_MODER_MODE10_Msk
#define GPIO_MODER_MODE10_0            (0x1UL << GPIO_MODER_MODE10_Pos)        /*!< 0x00100000 */
#define GPIO_MODER_MODE10_1            (0x2UL << GPIO_MODER_MODE10_Pos)        /*!< 0x00200000 */
#define GPIO_MODER_MODE11_Pos          (22U)
#define GPIO_MODER_MODE11_Msk          (0x3UL << GPIO_MODER_MODE11_Pos)        /*!< 0x00C00000 */
#define GPIO_MODER_MODE11              GPIO_MODER_MODE11_Msk
#define GPIO_MODER_MODE11_0            (0x1UL << GPIO_MODER_MODE11_Pos)        /*!< 0x00400000 */
#define GPIO_MODER_MODE11_1            (0x2UL << GPIO_MODER_MODE11_Pos)        /*!< 0x00800000 */
#define GPIO_MODER_MODE12_Pos          (24U)
#define GPIO_MODER_MODE12_Msk          (0x3UL << GPIO_MODER_MODE12_Pos)        /*!< 0x03000000 */
#define GPIO_MODER_MODE12              GPIO_MODER_MODE12_Msk
#define GPIO_MODER_MODE12_0            (0x1UL << GPIO_MODER_MODE12_Pos)        /*!< 0x01000000 */
#define GPIO_MODER_MODE12_1            (0x2UL << GPIO_MODER_MODE12_Pos)        /*!< 0x02000000 */
#define GPIO_MODER_MODE13_Pos          (26U)
#define GPIO_MODER_MODE13_Msk          (0x3UL << GPIO_MODER_MODE13_Pos)        /*!< 0x0C000000 */
#define GPIO_MODER_MODE13              GPIO_MODER_MODE13_Msk
#define GPIO_MODER_MODE13_0            (0x1UL << GPIO_MODER_MODE13_Pos)        /*!< 0x04000000 */
#define GPIO_MODER_MODE13_1            (0x2UL << GPIO_MODER_MODE13_Pos)        /*!< 0x08000000 */
#define GPIO_MODER_MODE14_Pos          (28U)
#define GPIO_MODER_MODE14_Msk          (0x3UL << GPIO_MODER_MODE14_Pos)        /*!< 0x30000000 */
#define GPIO_MODER_MODE14              GPIO_MODER_MODE14_Msk
#define GPIO_MODER_MODE14_0            (0x1UL << GPIO_MODER_MODE14_Pos)        /*!< 0x10000000 */
#define GPIO_MODER_MODE14_1            (0x2UL << GPIO_MODER_MODE14_Pos)        /*!< 0x20000000 */
#define GPIO_MODER_MODE15_Pos          (30U)
#define GPIO_MODER_MODE15_Msk          (0x3UL << GPIO_MODER_MODE15_Pos)        /*!< 0xC0000000 */
#define GPIO_MODER_MODE15              GPIO_MODER_MODE15_Msk
#define GPIO_MODER_MODE15_0            (0x1UL << GPIO_MODER_MODE15_Pos)        /*!< 0x40000000 */
#define GPIO_MODER_MODE15_1            (0x2UL << GPIO_MODER_MODE15_Pos)        /*!< 0x80000000 */

/* Legacy defines */
#define GPIO_MODER_MODER0                   GPIO_MODER_MODE0
#define GPIO_MODER_MODER0_0                 GPIO_MODER_MODE0_0
#define GPIO_MODER_MODER0_1                 GPIO_MODER_MODE0_1
#define GPIO_MODER_MODER1                   GPIO_MODER_MODE1
#define GPIO_MODER_MODER1_0                 GPIO_MODER_MODE1_0
#define GPIO_MODER_MODER1_1                 GPIO_MODER_MODE1_1
#define GPIO_MODER_MODER2                   GPIO_MODER_MODE2
#define GPIO_MODER_MODER2_0                 GPIO_MODER_MODE2_0
#define GPIO_MODER_MODER2_1                 GPIO_MODER_MODE2_1
#define GPIO_MODER_MODER3                   GPIO_MODER_MODE3
#define GPIO_MODER_MODER3_0                 GPIO_MODER_MODE3_0
#define GPIO_MODER_MODER3_1                 GPIO_MODER_MODE3_1
#define GPIO_MODER_MODER4                   GPIO_MODER_MODE4
#define GPIO_MODER_MODER4_0                 GPIO_MODER_MODE4_0
#define GPIO_MODER_MODER4_1                 GPIO_MODER_MODE4_1
#define GPIO_MODER_MODER5                   GPIO_MODER_MODE5
#define GPIO_MODER_MODER5_0                 GPIO_MODER_MODE5_0
#define GPIO_MODER_MODER5_1                 GPIO_MODER_MODE5_1
#define GPIO_MODER_MODER6                   GPIO_MODER_MODE6
#define GPIO_MODER_MODER6_0                 GPIO_MODER_MODE6_0
#define GPIO_MODER_MODER6_1                 GPIO_MODER_MODE6_1
#define GPIO_MODER_MODER7                   GPIO_MODER_MODE7
#define GPIO_MODER_MODER7_0                 GPIO_MODER_MODE7_0
#define GPIO_MODER_MODER7_1                 GPIO_MODER_MODE7_1
#define GPIO_MODER_MODER8                   GPIO_MODER_MODE8
#define GPIO_MODER_MODER8_0                 GPIO_MODER_MODE8_0
#define GPIO_MODER_MODER8_1                 GPIO_MODER_MODE8_1
#define GPIO_MODER_MODER9                   GPIO_MODER_MODE9
#define GPIO_MODER_MODER9_0                 GPIO_MODER_MODE9_0
#define GPIO_MODER_MODER9_1                 GPIO_MODER_MODE9_1
#define GPIO_MODER_MODER10                  GPIO_MODER_MODE10
#define GPIO_MODER_MODER10_0                GPIO_MODER_MODE10_0
#define GPIO_MODER_MODER10_1                GPIO_MODER_MODE10_1
#define GPIO_MODER_MODER11                  GPIO_MODER_MODE11
#define GPIO_MODER_MODER11_0                GPIO_MODER_MODE11_0
#define GPIO_MODER_MODER11_1                GPIO_MODER_MODE11_1
#define GPIO_MODER_MODER12                  GPIO_MODER_MODE12
#define GPIO_MODER_MODER12_0                GPIO_MODER_MODE12_0
#define GPIO_MODER_MODER12_1                GPIO_MODER_MODE12_1
#define GPIO_MODER_MODER13                  GPIO_MODER_MODE13
#define GPIO_MODER_MODER13_0                GPIO_MODER_MODE13_0
#define GPIO_MODER_MODER13_1                GPIO_MODER_MODE13_1
#define GPIO_MODER_MODER14                  GPIO_MODER_MODE14
#define GPIO_MODER_MODER14_0                GPIO_MODER_MODE14_0
#define GPIO_MODER_MODER14_1                GPIO_MODER_MODE14_1
#define GPIO_MODER_MODER15                  GPIO_MODER_MODE15
#define GPIO_MODER_MODER15_0                GPIO_MODER_MODE15_0
#define GPIO_MODER_MODER15_1                GPIO_MODER_MODE15_1

/******************  Bits definition for GPIO_OTYPER register  ****************/
#define GPIO_OTYPER_OT0_Pos            (0U)
#define GPIO_OTYPER_OT0_Msk            (0x1UL << GPIO_OTYPER_OT0_Pos)          /*!< 0x00000001 */
#define GPIO_OTYPER_OT0                GPIO_OTYPER_OT0_Msk
#define GPIO_OTYPER_OT1_Pos            (1U)
#define GPIO_OTYPER_OT1_Msk            (0x1UL << GPIO_OTYPER_OT1_Pos)          /*!< 0x00000002 */
#define GPIO_OTYPER_OT1                GPIO_OTYPER_OT1_Msk
#define GPIO_OTYPER_OT2_Pos            (2U)
#define GPIO_OTYPER_OT2_Msk            (0x1UL << GPIO_OTYPER_OT2_Pos)          /*!< 0x00000004 */
#define GPIO_OTYPER_OT2                GPIO_OTYPER_OT2_Msk
#define GPIO_OTYPER_OT3_Pos            (3U)
#define GPIO_OTYPER_OT3_Msk            (0x1UL << GPIO_OTYPER_OT3_Pos)          /*!< 0x00000008 */
#define GPIO_OTYPER_OT3                GPIO_OTYPER_OT3_Msk
#define GPIO_OTYPER_OT4_Pos            (4U)
#define GPIO_OTYPER_OT4_Msk            (0x1UL << GPIO_OTYPER_OT4_Pos)          /*!< 0x00000010 */
#define GPIO_OTYPER_OT4                GPIO_OTYPER_OT4_Msk
#define GPIO_OTYPER_OT5_Pos            (5U)
#define GPIO_OTYPER_OT5_Msk            (0x1UL << GPIO_OTYPER_OT5_Pos)          /*!< 0x00000020 */
#define GPIO_OTYPER_OT5                GPIO_OTYPER_OT5_Msk
#define GPIO_OTYPER_OT6_Pos            (6U)
#define GPIO_OTYPER_OT6_Msk            (0x1UL << GPIO_OTYPER_OT6_Pos)          /*!< 0x00000040 */
#define GPIO_OTYPER_OT6                GPIO_OTYPER_OT6_Msk
#define GPIO_OTYPER_OT7_Pos            (7U)
#define GPIO_OTYPER_OT7_Msk            (0x1UL << GPIO_OTYPER_OT7_Pos)          /*!< 0x00000080 */
#define GPIO_OTYPER_OT7                GPIO_OTYPER_OT7_Msk
#define GPIO_OTYPER_OT8_Pos            (8U)
#define GPIO_OTYPER_OT8_Msk            (0x1UL << GPIO_OTYPER_OT8_Pos)          /*!< 0x00000100 */
#define GPIO_OTYPER_OT8                GPIO_OTYPER_OT8_Msk
#define GPIO_OTYPER_OT9_Pos            (9U)
#define GPIO_OTYPER_OT9_Msk            (0x1UL << GPIO_OTYPER_OT9_Pos)          /*!< 0x00000200 */
#define GPIO_OTYPER_OT9                GPIO_OTYPER_OT9_Msk
#define GPIO_OTYPER_OT10_Pos           (10U)
#define GPIO_OTYPER_OT10_Msk           (0x1UL << GPIO_OTYPER_OT10_Pos)         /*!< 0x00000400 */
#define GPIO_OTYPER_OT10               GPIO_OTYPER_OT10_Msk
#define GPIO_OTYPER_OT11_Pos           (11U)
#define GPIO_OTYPER_OT11_Msk           (0x1UL << GPIO_OTYPER_OT11_Pos)         /*!< 0x00000800 */
#define GPIO_OTYPER_OT11               GPIO_OTYPER_OT11_Msk
#define GPIO_OTYPER_OT12_Pos           (12U)
#define GPIO_OTYPER_OT12_Msk           (0x1UL << GPIO_OTYPER_OT12_Pos)         /*!< 0x00001000 */
#define GPIO_OTYPER_OT12               GPIO_OTYPER_OT12_Msk
#define GPIO_OTYPER_OT13_Pos           (13U)
#define GPIO_OTYPER_OT13_Msk           (0x1UL << GPIO_OTYPER_OT13_Pos)         /*!< 0x00002000 */
#define GPIO_OTYPER_OT13               GPIO_OTYPER_OT13_Msk
#define GPIO_OTYPER_OT14_Pos           (14U)
#define GPIO_OTYPER_OT14_Msk           (0x1UL << GPIO_OTYPER_OT14_Pos)         /*!< 0x00004000 */
#define GPIO_OTYPER_OT14               GPIO_OTYPER_OT14_Msk
#define GPIO_OTYPER_OT15_Pos           (15U)
#define GPIO_OTYPER_OT15_Msk           (0x1UL << GPIO_OTYPER_OT15_Pos)         /*!< 0x00008000 */
#define GPIO_OTYPER_OT15               GPIO_OTYPER_OT15_Msk

/* Legacy defines */
#define GPIO_OTYPER_OT_0                    GPIO_OTYPER_OT0
#define GPIO_OTYPER_OT_1                    GPIO_OTYPER_OT1
#define GPIO_OTYPER_OT_2                    GPIO_OTYPER_OT2
#define GPIO_OTYPER_OT_3                    GPIO_OTYPER_OT3
#define GPIO_OTYPER_OT_4                    GPIO_OTYPER_OT4
#define GPIO_OTYPER_OT_5                    GPIO_OTYPER_OT5
#define GPIO_OTYPER_OT_6                    GPIO_OTYPER_OT6
#define GPIO_OTYPER_OT_7                    GPIO_OTYPER_OT7
#define GPIO_OTYPER_OT_8                    GPIO_OTYPER_OT8
#define GPIO_OTYPER_OT_9                    GPIO_OTYPER_OT9
#define GPIO_OTYPER_OT_10                   GPIO_OTYPER_OT10
#define GPIO_OTYPER_OT_11                   GPIO_OTYPER_OT11
#define GPIO_OTYPER_OT_12                   GPIO_OTYPER_OT12
#define GPIO_OTYPER_OT_13                   GPIO_OTYPER_OT13
#define GPIO_OTYPER_OT_14                   GPIO_OTYPER_OT14
#define GPIO_OTYPER_OT_15                   GPIO_OTYPER_OT15

/******************  Bits definition for GPIO_OSPEEDR register  ***************/
#define GPIO_OSPEEDR_OSPEED0_Pos       (0U)
#define GPIO_OSPEEDR_OSPEED0_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED0_Pos)     /*!< 0x00000003 */
#define GPIO_OSPEEDR_OSPEED0           GPIO_OSPEEDR_OSPEED0_Msk
#define GPIO_OSPEEDR_OSPEED0_0         (0x1UL << GPIO_OSPEEDR_OSPEED0_Pos)     /*!< 0x00000001 */
#define GPIO_OSPEEDR_OSPEED0_1         (0x2UL << GPIO_OSPEEDR_OSPEED0_Pos)     /*!< 0x00000002 */
#define GPIO_OSPEEDR_OSPEED1_Pos       (2U)
#define GPIO_OSPEEDR_OSPEED1_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED1_Pos)     /*!< 0x0000000C */
#define GPIO_OSPEEDR_OSPEED1           GPIO_OSPEEDR_OSPEED1_Msk
#define GPIO_OSPEEDR_OSPEED1_0         (0x1UL << GPIO_OSPEEDR_OSPEED1_Pos)     /*!< 0x00000004 */
#define GPIO_OSPEEDR_OSPEED1_1         (0x2UL << GPIO_OSPEEDR_OSPEED1_Pos)     /*!< 0x00000008 */
#define GPIO_OSPEEDR_OSPEED2_Pos       (4U)
#define GPIO_OSPEEDR_OSPEED2_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED2_Pos)     /*!< 0x00000030 */
#define GPIO_OSPEEDR_OSPEED2           GPIO_OSPEEDR_OSPEED2_Msk
#define GPIO_OSPEEDR_OSPEED2_0         (0x1UL << GPIO_OSPEEDR_OSPEED2_Pos)     /*!< 0x00000010 */
#define GPIO_OSPEEDR_OSPEED2_1         (0x2UL << GPIO_OSPEEDR_OSPEED2_Pos)     /*!< 0x00000020 */
#define GPIO_OSPEEDR_OSPEED3_Pos       (6U)
#define GPIO_OSPEEDR_OSPEED3_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED3_Pos)     /*!< 0x000000C0 */
#define GPIO_OSPEEDR_OSPEED3           GPIO_OSPEEDR_OSPEED3_Msk
#define GPIO_OSPEEDR_OSPEED3_0         (0x1UL << GPIO_OSPEEDR_OSPEED3_Pos)     /*!< 0x00000040 */
#define GPIO_OSPEEDR_OSPEED3_1         (0x2UL << GPIO_OSPEEDR_OSPEED3_Pos)     /*!< 0x00000080 */
#define GPIO_OSPEEDR_OSPEED4_Pos       (8U)
#define GPIO_OSPEEDR_OSPEED4_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED4_Pos)     /*!< 0x00000300 */
#define GPIO_OSPEEDR_OSPEED4           GPIO_OSPEEDR_OSPEED4_Msk
#define GPIO_OSPEEDR_OSPEED4_0         (0x1UL << GPIO_OSPEEDR_OSPEED4_Pos)     /*!< 0x00000100 */
#define GPIO_OSPEEDR_OSPEED4_1         (0x2UL << GPIO_OSPEEDR_OSPEED4_Pos)     /*!< 0x00000200 */
#define GPIO_OSPEEDR_OSPEED5_Pos       (10U)
#define GPIO_OSPEEDR_OSPEED5_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED5_Pos)     /*!< 0x00000C00 */
#define GPIO_OSPEEDR_OSPEED5           GPIO_OSPEEDR_OSPEED5_Msk
#define GPIO_OSPEEDR_OSPEED5_0         (0x1UL << GPIO_OSPEEDR_OSPEED5_Pos)     /*!< 0x00000400 */
#define GPIO_OSPEEDR_OSPEED5_1         (0x2UL << GPIO_OSPEEDR_OSPEED5_Pos)     /*!< 0x00000800 */
#define GPIO_OSPEEDR_OSPEED6_Pos       (12U)
#define GPIO_OSPEEDR_OSPEED6_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED6_Pos)     /*!< 0x00003000 */
#define GPIO_OSPEEDR_OSPEED6           GPIO_OSPEEDR_OSPEED6_Msk
#define GPIO_OSPEEDR_OSPEED6_0         (0x1UL << GPIO_OSPEEDR_OSPEED6_Pos)     /*!< 0x00001000 */
#define GPIO_OSPEEDR_OSPEED6_1         (0x2UL << GPIO_OSPEEDR_OSPEED6_Pos)     /*!< 0x00002000 */
#define GPIO_OSPEEDR_OSPEED7_Pos       (14U)
#define GPIO_OSPEEDR_OSPEED7_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED7_Pos)     /*!< 0x0000C000 */
#define GPIO_OSPEEDR_OSPEED7           GPIO_OSPEEDR_OSPEED7_Msk
#define GPIO_OSPEEDR_OSPEED7_0         (0x1UL << GPIO_OSPEEDR_OSPEED7_Pos)     /*!< 0x00004000 */
#define GPIO_OSPEEDR_OSPEED7_1         (0x2UL << GPIO_OSPEEDR_OSPEED7_Pos)     /*!< 0x00008000 */
#define GPIO_OSPEEDR_OSPEED8_Pos       (16U)
#define GPIO_OSPEEDR_OSPEED8_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED8_Pos)     /*!< 0x00030000 */
#define GPIO_OSPEEDR_OSPEED8           GPIO_OSPEEDR_OSPEED8_Msk
#define GPIO_OSPEEDR_OSPEED8_0         (0x1UL << GPIO_OSPEEDR_OSPEED8_Pos)     /*!< 0x00010000 */
#define GPIO_OSPEEDR_OSPEED8_1         (0x2UL << GPIO_OSPEEDR_OSPEED8_Pos)     /*!< 0x00020000 */
#define GPIO_OSPEEDR_OSPEED9_Pos       (18U)
#define GPIO_OSPEEDR_OSPEED9_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED9_Pos)     /*!< 0x000C0000 */
#define GPIO_OSPEEDR_OSPEED9           GPIO_OSPEEDR_OSPEED9_Msk
#define GPIO_OSPEEDR_OSPEED9_0         (0x1UL << GPIO_OSPEEDR_OSPEED9_Pos)     /*!< 0x00040000 */
#define GPIO_OSPEEDR_OSPEED9_1         (0x2UL << GPIO_OSPEEDR_OSPEED9_Pos)     /*!< 0x00080000 */
#define GPIO_OSPEEDR_OSPEED10_Pos      (20U)
#define GPIO_OSPEEDR_OSPEED10_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED10_Pos)    /*!< 0x00300000 */
#define GPIO_OSPEEDR_OSPEED10          GPIO_OSPEEDR_OSPEED10_Msk
#define GPIO_OSPEEDR_OSPEED10_0        (0x1UL << GPIO_OSPEEDR_OSPEED10_Pos)    /*!< 0x00100000 */
#define GPIO_OSPEEDR_OSPEED10_1        (0x2UL << GPIO_OSPEEDR_OSPEED10_Pos)    /*!< 0x00200000 */
#define GPIO_OSPEEDR_OSPEED11_Pos      (22U)
#define GPIO_OSPEEDR_OSPEED11_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED11_Pos)    /*!< 0x00C00000 */
#define GPIO_OSPEEDR_OSPEED11          GPIO_OSPEEDR_OSPEED11_Msk
#define GPIO_OSPEEDR_OSPEED11_0        (0x1UL << GPIO_OSPEEDR_OSPEED11_Pos)    /*!< 0x00400000 */
#define GPIO_OSPEEDR_OSPEED11_1        (0x2UL << GPIO_OSPEEDR_OSPEED11_Pos)    /*!< 0x00800000 */
#define GPIO_OSPEEDR_OSPEED12_Pos      (24U)
#define GPIO_OSPEEDR_OSPEED12_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED12_Pos)    /*!< 0x03000000 */
#define GPIO_OSPEEDR_OSPEED12          GPIO_OSPEEDR_OSPEED12_Msk
#define GPIO_OSPEEDR_OSPEED12_0        (0x1UL << GPIO_OSPEEDR_OSPEED12_Pos)    /*!< 0x01000000 */
#define GPIO_OSPEEDR_OSPEED12_1        (0x2UL << GPIO_OSPEEDR_OSPEED12_Pos)    /*!< 0x02000000 */
#define GPIO_OSPEEDR_OSPEED13_Pos      (26U)
#define GPIO_OSPEEDR_OSPEED13_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED13_Pos)    /*!< 0x0C000000 */
#define GPIO_OSPEEDR_OSPEED13          GPIO_OSPEEDR_OSPEED13_Msk
#define GPIO_OSPEEDR_OSPEED13_0        (0x1UL << GPIO_OSPEEDR_OSPEED13_Pos)    /*!< 0x04000000 */
#define GPIO_OSPEEDR_OSPEED13_1        (0x2UL << GPIO_OSPEEDR_OSPEED13_Pos)    /*!< 0x08000000 */
#define GPIO_OSPEEDR_OSPEED14_Pos      (28U)
#define GPIO_OSPEEDR_OSPEED14_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED14_Pos)    /*!< 0x30000000 */
#define GPIO_OSPEEDR_OSPEED14          GPIO_OSPEEDR_OSPEED14_Msk
#define GPIO_OSPEEDR_OSPEED14_0        (0x1UL << GPIO_OSPEEDR_OSPEED14_Pos)    /*!< 0x10000000 */
#define GPIO_OSPEEDR_OSPEED14_1        (0x2UL << GPIO_OSPEEDR_OSPEED14_Pos)    /*!< 0x20000000 */
#define GPIO_OSPEEDR_OSPEED15_Pos      (30U)
#define GPIO_OSPEEDR_OSPEED15_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED15_Pos)    /*!< 0xC0000000 */
#define GPIO_OSPEEDR_OSPEED15          GPIO_OSPEEDR_OSPEED15_Msk
#define GPIO_OSPEEDR_OSPEED15_0        (0x1UL << GPIO_OSPEEDR_OSPEED15_Pos)    /*!< 0x40000000 */
#define GPIO_OSPEEDR_OSPEED15_1        (0x2UL << GPIO_OSPEEDR_OSPEED15_Pos)    /*!< 0x80000000 */

/* Legacy defines */
#define GPIO_OSPEEDER_OSPEEDR0              GPIO_OSPEEDR_OSPEED0
#define GPIO_OSPEEDER_OSPEEDR0_0            GPIO_OSPEEDR_OSPEED0_0
#define GPIO_OSPEEDER_OSPEEDR0_1            GPIO_OSPEEDR_OSPEED0_1
#define GPIO_OSPEEDER_OSPEEDR1              GPIO_OSPEEDR_OSPEED1
#define GPIO_OSPEEDER_OSPEEDR1_0            GPIO_OSPEEDR_OSPEED1_0
#define GPIO_OSPEEDER_OSPEEDR1_1            GPIO_OSPEEDR_OSPEED1_1
#define GPIO_OSPEEDER_OSPEEDR2              GPIO_OSPEEDR_OSPEED2
#define GPIO_OSPEEDER_OSPEEDR2_0            GPIO_OSPEEDR_OSPEED2_0
#define GPIO_OSPEEDER_OSPEEDR2_1            GPIO_OSPEEDR_OSPEED2_1
#define GPIO_OSPEEDER_OSPEEDR3              GPIO_OSPEEDR_OSPEED3
#define GPIO_OSPEEDER_OSPEEDR3_0            GPIO_OSPEEDR_OSPEED3_0
#define GPIO_OSPEEDER_OSPEEDR3_1            GPIO_OSPEEDR_OSPEED3_1
#define GPIO_OSPEEDER_OSPEEDR4              GPIO_OSPEEDR_OSPEED4
#define GPIO_OSPEEDER_OSPEEDR4_0            GPIO_OSPEEDR_OSPEED4_0
#define GPIO_OSPEEDER_OSPEEDR4_1            GPIO_OSPEEDR_OSPEED4_1
#define GPIO_OSPEEDER_OSPEEDR5              GPIO_OSPEEDR_OSPEED5
#define GPIO_OSPEEDER_OSPEEDR5_0            GPIO_OSPEEDR_OSPEED5_0
#define GPIO_OSPEEDER_OSPEEDR5_1            GPIO_OSPEEDR_OSPEED5_1
#define GPIO_OSPEEDER_OSPEEDR6              GPIO_OSPEEDR_OSPEED6
#define GPIO_OSPEEDER_OSPEEDR6_0            GPIO_OSPEEDR_OSPEED6_0
#define GPIO_OSPEEDER_OSPEEDR6_1            GPIO_OSPEEDR_OSPEED6_1
#define GPIO_OSPEEDER_OSPEEDR7              GPIO_OSPEEDR_OSPEED7
#define GPIO_OSPEEDER_OSPEEDR7_0            GPIO_OSPEEDR_OSPEED7_0
#define GPIO_OSPEEDER_OSPEEDR7_1            GPIO_OSPEEDR_OSPEED7_1
#define GPIO_OSPEEDER_OSPEEDR8              GPIO_OSPEEDR_OSPEED8
#define GPIO_OSPEEDER_OSPEEDR8_0            GPIO_OSPEEDR_OSPEED8_0
#define GPIO_OSPEEDER_OSPEEDR8_1            GPIO_OSPEEDR_OSPEED8_1
#define GPIO_OSPEEDER_OSPEEDR9              GPIO_OSPEEDR_OSPEED9
#define GPIO_OSPEEDER_OSPEEDR9_0            GPIO_OSPEEDR_OSPEED9_0
#define GPIO_OSPEEDER_OSPEEDR9_1            GPIO_OSPEEDR_OSPEED9_1
#define GPIO_OSPEEDER_OSPEEDR10             GPIO_OSPEEDR_OSPEED10
#define GPIO_OSPEEDER_OSPEEDR10_0           GPIO_OSPEEDR_OSPEED10_0
#define GPIO_OSPEEDER_OSPEEDR10_1           GPIO_OSPEEDR_OSPEED10_1
#define GPIO_OSPEEDER_OSPEEDR11             GPIO_OSPEEDR_OSPEED11
#define GPIO_OSPEEDER_OSPEEDR11_0           GPIO_OSPEEDR_OSPEED11_0
#define GPIO_OSPEEDER_OSPEEDR11_1           GPIO_OSPEEDR_OSPEED11_1
#define GPIO_OSPEEDER_OSPEEDR12             GPIO_OSPEEDR_OSPEED12
#define GPIO_OSPEEDER_OSPEEDR12_0           GPIO_OSPEEDR_OSPEED12_0
#define GPIO_OSPEEDER_OSPEEDR12_1           GPIO_OSPEEDR_OSPEED12_1
#define GPIO_OSPEEDER_OSPEEDR13             GPIO_OSPEEDR_OSPEED13
#define GPIO_OSPEEDER_OSPEEDR13_0           GPIO_OSPEEDR_OSPEED13_0
#define GPIO_OSPEEDER_OSPEEDR13_1           GPIO_OSPEEDR_OSPEED13_1
#define GPIO_OSPEEDER_OSPEEDR14             GPIO_OSPEEDR_OSPEED14
#define GPIO_OSPEEDER_OSPEEDR14_0           GPIO_OSPEEDR_OSPEED14_0
#define GPIO_OSPEEDER_OSPEEDR14_1           GPIO_OSPEEDR_OSPEED14_1
#define GPIO_OSPEEDER_OSPEEDR15             GPIO_OSPEEDR_OSPEED15
#define GPIO_OSPEEDER_OSPEEDR15_0           GPIO_OSPEEDR_OSPEED15_0
#define GPIO_OSPEEDER_OSPEEDR15_1           GPIO_OSPEEDR_OSPEED15_1

/******************  Bits definition for GPIO_PUPDR register  *****************/
#define GPIO_PUPDR_PUPD0_Pos           (0U)
#define GPIO_PUPDR_PUPD0_Msk           (0x3UL << GPIO_PUPDR_PUPD0_Pos)         /*!< 0x00000003 */
#define GPIO_PUPDR_PUPD0               GPIO_PUPDR_PUPD0_Msk
#define GPIO_PUPDR_PUPD0_0             (0x1UL << GPIO_PUPDR_PUPD0_Pos)         /*!< 0x00000001 */
#define GPIO_PUPDR_PUPD0_1             (0x2UL << GPIO_PUPDR_PUPD0_Pos)         /*!< 0x00000002 */
#define GPIO_PUPDR_PUPD1_Pos           (2U)
#define GPIO_PUPDR_PUPD1_Msk           (0x3UL << GPIO_PUPDR_PUPD1_Pos)         /*!< 0x0000000C */
#define GPIO_PUPDR_PUPD1               GPIO_PUPDR_PUPD1_Msk
#define GPIO_PUPDR_PUPD1_0             (0x1UL << GPIO_PUPDR_PUPD1_Pos)         /*!< 0x00000004 */
#define GPIO_PUPDR_PUPD1_1             (0x2UL << GPIO_PUPDR_PUPD1_Pos)         /*!< 0x00000008 */
#define GPIO_PUPDR_PUPD2_Pos           (4U)
#define GPIO_PUPDR_PUPD2_Msk           (0x3UL << GPIO_PUPDR_PUPD2_Pos)         /*!< 0x00000030 */
#define GPIO_PUPDR_PUPD2               GPIO_PUPDR_PUPD2_Msk
#define GPIO_PUPDR_PUPD2_0             (0x1UL << GPIO_PUPDR_PUPD2_Pos)         /*!< 0x00000010 */
#define GPIO_PUPDR_PUPD2_1             (0x2UL << GPIO_PUPDR_PUPD2_Pos)         /*!< 0x00000020 */
#define GPIO_PUPDR_PUPD3_Pos           (6U)
#define GPIO_PUPDR_PUPD3_Msk           (0x3UL << GPIO_PUPDR_PUPD3_Pos)         /*!< 0x000000C0 */
#define GPIO_PUPDR_PUPD3               GPIO_PUPDR_PUPD3_Msk
#define GPIO_PUPDR_PUPD3_0             (0x1UL << GPIO_PUPDR_PUPD3_Pos)         /*!< 0x00000040 */
#define GPIO_PUPDR_PUPD3_1             (0x2UL << GPIO_PUPDR_PUPD3_Pos)         /*!< 0x00000080 */
#define GPIO_PUPDR_PUPD4_Pos           (8U)
#define GPIO_PUPDR_PUPD4_Msk           (0x3UL << GPIO_PUPDR_PUPD4_Pos)         /*!< 0x00000300 */
#define GPIO_PUPDR_PUPD4               GPIO_PUPDR_PUPD4_Msk
#define GPIO_PUPDR_PUPD4_0             (0x1UL << GPIO_PUPDR_PUPD4_Pos)         /*!< 0x00000100 */
#define GPIO_PUPDR_PUPD4_1             (0x2UL << GPIO_PUPDR_PUPD4_Pos)         /*!< 0x00000200 */
#define GPIO_PUPDR_PUPD5_Pos           (10U)
#define GPIO_PUPDR_PUPD5_Msk           (0x3UL << GPIO_PUPDR_PUPD5_Pos)         /*!< 0x00000C00 */
#define GPIO_PUPDR_PUPD5               GPIO_PUPDR_PUPD5_Msk
#define GPIO_PUPDR_PUPD5_0             (0x1UL << GPIO_PUPDR_PUPD5_Pos)         /*!< 0x00000400 */
#define GPIO_PUPDR_PUPD5_1             (0x2UL << GPIO_PUPDR_PUPD5_Pos)         /*!< 0x00000800 */
#define GPIO_PUPDR_PUPD6_Pos           (12U)
#define GPIO_PUPDR_PUPD6_Msk           (0x3UL << GPIO_PUPDR_PUPD6_Pos)         /*!< 0x00003000 */
#define GPIO_PUPDR_PUPD6               GPIO_PUPDR_PUPD6_Msk
#define GPIO_PUPDR_PUPD6_0             (0x1UL << GPIO_PUPDR_PUPD6_Pos)         /*!< 0x00001000 */
#define GPIO_PUPDR_PUPD6_1             (0x2UL << GPIO_PUPDR_PUPD6_Pos)         /*!< 0x00002000 */
#define GPIO_PUPDR_PUPD7_Pos           (14U)
#define GPIO_PUPDR_PUPD7_Msk           (0x3UL << GPIO_PUPDR_PUPD7_Pos)         /*!< 0x0000C000 */
#define GPIO_PUPDR_PUPD7               GPIO_PUPDR_PUPD7_Msk
#define GPIO_PUPDR_PUPD7_0             (0x1UL << GPIO_PUPDR_PUPD7_Pos)         /*!< 0x00004000 */
#define GPIO_PUPDR_PUPD7_1             (0x2UL << GPIO_PUPDR_PUPD7_Pos)         /*!< 0x00008000 */
#define GPIO_PUPDR_PUPD8_Pos           (16U)
#define GPIO_PUPDR_PUPD8_Msk           (0x3UL << GPIO_PUPDR_PUPD8_Pos)         /*!< 0x00030000 */
#define GPIO_PUPDR_PUPD8               GPIO_PUPDR_PUPD8_Msk
#define GPIO_PUPDR_PUPD8_0             (0x1UL << GPIO_PUPDR_PUPD8_Pos)         /*!< 0x00010000 */
#define GPIO_PUPDR_PUPD8_1             (0x2UL << GPIO_PUPDR_PUPD8_Pos)         /*!< 0x00020000 */
#define GPIO_PUPDR_PUPD9_Pos           (18U)
#define GPIO_PUPDR_PUPD9_Msk           (0x3UL << GPIO_PUPDR_PUPD9_Pos)         /*!< 0x000C0000 */
#define GPIO_PUPDR_PUPD9               GPIO_PUPDR_PUPD9_Msk
#define GPIO_PUPDR_PUPD9_0             (0x1UL << GPIO_PUPDR_PUPD9_Pos)         /*!< 0x00040000 */
#define GPIO_PUPDR_PUPD9_1             (0x2UL << GPIO_PUPDR_PUPD9_Pos)         /*!< 0x00080000 */
#define GPIO_PUPDR_PUPD10_Pos          (20U)
#define GPIO_PUPDR_PUPD10_Msk          (0x3UL << GPIO_PUPDR_PUPD10_Pos)        /*!< 0x00300000 */
#define GPIO_PUPDR_PUPD10              GPIO_PUPDR_PUPD10_Msk
#define GPIO_PUPDR_PUPD10_0            (0x1UL << GPIO_PUPDR_PUPD10_Pos)        /*!< 0x00100000 */
#define GPIO_PUPDR_PUPD10_1            (0x2UL << GPIO_PUPDR_PUPD10_Pos)        /*!< 0x00200000 */
#define GPIO_PUPDR_PUPD11_Pos          (22U)
#define GPIO_PUPDR_PUPD11_Msk          (0x3UL << GPIO_PUPDR_PUPD11_Pos)        /*!< 0x00C00000 */
#define GPIO_PUPDR_PUPD11              GPIO_PUPDR_PUPD11_Msk
#define GPIO_PUPDR_PUPD11_0            (0x1UL << GPIO_PUPDR_PUPD11_Pos)        /*!< 0x00400000 */
#define GPIO_PUPDR_PUPD11_1            (0x2UL << GPIO_PUPDR_PUPD11_Pos)        /*!< 0x00800000 */
#define GPIO_PUPDR_PUPD12_Pos          (24U)
#define GPIO_PUPDR_PUPD12_Msk          (0x3UL << GPIO_PUPDR_PUPD12_Pos)        /*!< 0x03000000 */
#define GPIO_PUPDR_PUPD12              GPIO_PUPDR_PUPD12_Msk
#define GPIO_PUPDR_PUPD12_0            (0x1UL << GPIO_PUPDR_PUPD12_Pos)        /*!< 0x01000000 */
#define GPIO_PUPDR_PUPD12_1            (0x2UL << GPIO_PUPDR_PUPD12_Pos)        /*!< 0x02000000 */
#define GPIO_PUPDR_PUPD13_Pos          (26U)
#define GPIO_PUPDR_PUPD13_Msk          (0x3UL << GPIO_PUPDR_PUPD13_Pos)        /*!< 0x0C000000 */
#define GPIO_PUPDR_PUPD13              GPIO_PUPDR_PUPD13_Msk
#define GPIO_PUPDR_PUPD13_0            (0x1UL << GPIO_PUPDR_PUPD13_Pos)        /*!< 0x04000000 */
#define GPIO_PUPDR_PUPD13_1            (0x2UL << GPIO_PUPDR_PUPD13_Pos)        /*!< 0x08000000 */
#define GPIO_PUPDR_PUPD14_Pos          (28U)
#define GPIO_PUPDR_PUPD14_Msk          (0x3UL << GPIO_PUPDR_PUPD14_Pos)        /*!< 0x30000000 */
#define GPIO_PUPDR_PUPD14              GPIO_PUPDR_PUPD14_Msk
#define GPIO_PUPDR_PUPD14_0            (0x1UL << GPIO_PUPDR_PUPD14_Pos)        /*!< 0x10000000 */
#define GPIO_PUPDR_PUPD14_1            (0x2UL << GPIO_PUPDR_PUPD14_Pos)        /*!< 0x20000000 */
#define GPIO_PUPDR_PUPD15_Pos          (30U)
#define GPIO_PUPDR_PUPD15_Msk          (0x3UL << GPIO_PUPDR_PUPD15_Pos)        /*!< 0xC0000000 */
#define GPIO_PUPDR_PUPD15              GPIO_PUPDR_PUPD15_Msk
#define GPIO_PUPDR_PUPD15_0            (0x1UL << GPIO_PUPDR_PUPD15_Pos)        /*!< 0x40000000 */
#define GPIO_PUPDR_PUPD15_1            (0x2UL << GPIO_PUPDR_PUPD15_Pos)        /*!< 0x80000000 */

/* Legacy defines */
#define GPIO_PUPDR_PUPDR0                   GPIO_PUPDR_PUPD0
#define GPIO_PUPDR_PUPDR0_0                 GPIO_PUPDR_PUPD0_0
#define GPIO_PUPDR_PUPDR0_1                 GPIO_PUPDR_PUPD0_1
#define GPIO_PUPDR_PUPDR1                   GPIO_PUPDR_PUPD1
#define GPIO_PUPDR_PUPDR1_0                 GPIO_PUPDR_PUPD1_0
#define GPIO_PUPDR_PUPDR1_1                 GPIO_PUPDR_PUPD1_1
#define GPIO_PUPDR_PUPDR2                   GPIO_PUPDR_PUPD2
#define GPIO_PUPDR_PUPDR2_0                 GPIO_PUPDR_PUPD2_0
#define GPIO_PUPDR_PUPDR2_1                 GPIO_PUPDR_PUPD2_1
#define GPIO_PUPDR_PUPDR3                   GPIO_PUPDR_PUPD3
#define GPIO_PUPDR_PUPDR3_0                 GPIO_PUPDR_PUPD3_0
#define GPIO_PUPDR_PUPDR3_1                 GPIO_PUPDR_PUPD3_1
#define GPIO_PUPDR_PUPDR4                   GPIO_PUPDR_PUPD4
#define GPIO_PUPDR_PUPDR4_0                 GPIO_PUPDR_PUPD4_0
#define GPIO_PUPDR_PUPDR4_1                 GPIO_PUPDR_PUPD4_1
#define GPIO_PUPDR_PUPDR5                   GPIO_PUPDR_PUPD5
#define GPIO_PUPDR_PUPDR5_0                 GPIO_PUPDR_PUPD5_0
#define GPIO_PUPDR_PUPDR5_1                 GPIO_PUPDR_PUPD5_1
#define GPIO_PUPDR_PUPDR6                   GPIO_PUPDR_PUPD6
#define GPIO_PUPDR_PUPDR6_0                 GPIO_PUPDR_PUPD6_0
#define GPIO_PUPDR_PUPDR6_1                 GPIO_PUPDR_PUPD6_1
#define GPIO_PUPDR_PUPDR7                   GPIO_PUPDR_PUPD7
#define GPIO_PUPDR_PUPDR7_0                 GPIO_PUPDR_PUPD7_0
#define GPIO_PUPDR_PUPDR7_1                 GPIO_PUPDR_PUPD7_1
#define GPIO_PUPDR_PUPDR8                   GPIO_PUPDR_PUPD8
#define GPIO_PUPDR_PUPDR8_0                 GPIO_PUPDR_PUPD8_0
#define GPIO_PUPDR_PUPDR8_1                 GPIO_PUPDR_PUPD8_1
#define GPIO_PUPDR_PUPDR9                   GPIO_PUPDR_PUPD9
#define GPIO_PUPDR_PUPDR9_0                 GPIO_PUPDR_PUPD9_0
#define GPIO_PUPDR_PUPDR9_1                 GPIO_PUPDR_PUPD9_1
#define GPIO_PUPDR_PUPDR10                  GPIO_PUPDR_PUPD10
#define GPIO_PUPDR_PUPDR10_0                GPIO_PUPDR_PUPD10_0
#define GPIO_PUPDR_PUPDR10_1                GPIO_PUPDR_PUPD10_1
#define GPIO_PUPDR_PUPDR11                  GPIO_PUPDR_PUPD11
#define GPIO_PUPDR_PUPDR11_0                GPIO_PUPDR_PUPD11_0
#define GPIO_PUPDR_PUPDR11_1                GPIO_PUPDR_PUPD11_1
#define GPIO_PUPDR_PUPDR12                  GPIO_PUPDR_PUPD12
#define GPIO_PUPDR_PUPDR12_0                GPIO_PUPDR_PUPD12_0
#define GPIO_PUPDR_PUPDR12_1                GPIO_PUPDR_PUPD12_1
#define GPIO_PUPDR_PUPDR13                  GPIO_PUPDR_PUPD13
#define GPIO_PUPDR_PUPDR13_0                GPIO_PUPDR_PUPD13_0
#define GPIO_PUPDR_PUPDR13_1                GPIO_PUPDR_PUPD13_1
#define GPIO_PUPDR_PUPDR14                  GPIO_PUPDR_PUPD14
#define GPIO_PUPDR_PUPDR14_0                GPIO_PUPDR_PUPD14_0
#define GPIO_PUPDR_PUPDR14_1                GPIO_PUPDR_PUPD14_1
#define GPIO_PUPDR_PUPDR15                  GPIO_PUPDR_PUPD15
#define GPIO_PUPDR_PUPDR15_0                GPIO_PUPDR_PUPD15_0
#define GPIO_PUPDR_PUPDR15_1                GPIO_PUPDR_PUPD15_1

/******************  Bits definition for GPIO_IDR register  *******************/
#define GPIO_IDR_ID0_Pos               (0U)
#define GPIO_IDR_ID0_Msk               (0x1UL << GPIO_IDR_ID0_Pos)             /*!< 0x00000001 */
#define GPIO_IDR_ID0                   GPIO_IDR_ID0_Msk
#define GPIO_IDR_ID1_Pos               (1U)
#define GPIO_IDR_ID1_Msk               (0x1UL << GPIO_IDR_ID1_Pos)             /*!< 0x00000002 */
#define GPIO_IDR_ID1                   GPIO_IDR_ID1_Msk
#define GPIO_IDR_ID2_Pos               (2U)
#define GPIO_IDR_ID2_Msk               (0x1UL << GPIO_IDR_ID2_Pos)             /*!< 0x00000004 */
#define GPIO_IDR_ID2                   GPIO_IDR_ID2_Msk
#define GPIO_IDR_ID3_Pos               (3U)
#define GPIO_IDR_ID3_Msk               (0x1UL << GPIO_IDR_ID3_Pos)             /*!< 0x00000008 */
#define GPIO_IDR_ID3                   GPIO_IDR_ID3_Msk
#define GPIO_IDR_ID4_Pos               (4U)
#define GPIO_IDR_ID4_Msk               (0x1UL << GPIO_IDR_ID4_Pos)             /*!< 0x00000010 */
#define GPIO_IDR_ID4                   GPIO_IDR_ID4_Msk
#define GPIO_IDR_ID5_Pos               (5U)
#define GPIO_IDR_ID5_Msk               (0x1UL << GPIO_IDR_ID5_Pos)             /*!< 0x00000020 */
#define GPIO_IDR_ID5                   GPIO_IDR_ID5_Msk
#define GPIO_IDR_ID6_Pos               (6U)
#define GPIO_IDR_ID6_Msk               (0x1UL << GPIO_IDR_ID6_Pos)             /*!< 0x00000040 */
#define GPIO_IDR_ID6                   GPIO_IDR_ID6_Msk
#define GPIO_IDR_ID7_Pos               (7U)
#define GPIO_IDR_ID7_Msk               (0x1UL << GPIO_IDR_ID7_Pos)             /*!< 0x00000080 */
#define GPIO_IDR_ID7                   GPIO_IDR_ID7_Msk
#define GPIO_IDR_ID8_Pos               (8U)
#define GPIO_IDR_ID8_Msk               (0x1UL << GPIO_IDR_ID8_Pos)             /*!< 0x00000100 */
#define GPIO_IDR_ID8                   GPIO_IDR_ID8_Msk
#define GPIO_IDR_ID9_Pos               (9U)
#define GPIO_IDR_ID9_Msk               (0x1UL << GPIO_IDR_ID9_Pos)             /*!< 0x00000200 */
#define GPIO_IDR_ID9                   GPIO_IDR_ID9_Msk
#define GPIO_IDR_ID10_Pos              (10U)
#define GPIO_IDR_ID10_Msk              (0x1UL << GPIO_IDR_ID10_Pos)            /*!< 0x00000400 */
#define GPIO_IDR_ID10                  GPIO_IDR_ID10_Msk
#define GPIO_IDR_ID11_Pos              (11U)
#define GPIO_IDR_ID11_Msk              (0x1UL << GPIO_IDR_ID11_Pos)            /*!< 0x00000800 */
#define GPIO_IDR_ID11                  GPIO_IDR_ID11_Msk
#define GPIO_IDR_ID12_Pos              (12U)
#define GPIO_IDR_ID12_Msk              (0x1UL << GPIO_IDR_ID12_Pos)            /*!< 0x00001000 */
#define GPIO_IDR_ID12                  GPIO_IDR_ID12_Msk
#define GPIO_IDR_ID13_Pos              (13U)
#define GPIO_IDR_ID13_Msk              (0x1UL << GPIO_IDR_ID13_Pos)            /*!< 0x00002000 */
#define GPIO_IDR_ID13                  GPIO_IDR_ID13_Msk
#define GPIO_IDR_ID14_Pos              (14U)
#define GPIO_IDR_ID14_Msk              (0x1UL << GPIO_IDR_ID14_Pos)            /*!< 0x00004000 */
#define GPIO_IDR_ID14                  GPIO_IDR_ID14_Msk
#define GPIO_IDR_ID15_Pos              (15U)
#define GPIO_IDR_ID15_Msk              (0x1UL << GPIO_IDR_ID15_Pos)            /*!< 0x00008000 */
#define GPIO_IDR_ID15                  GPIO_IDR_ID15_Msk

/* Legacy defines */
#define GPIO_IDR_IDR_0                      GPIO_IDR_ID0
#define GPIO_IDR_IDR_1                      GPIO_IDR_ID1
#define GPIO_IDR_IDR_2                      GPIO_IDR_ID2
#define GPIO_IDR_IDR_3                      GPIO_IDR_ID3
#define GPIO_IDR_IDR_4                      GPIO_IDR_ID4
#define GPIO_IDR_IDR_5                      GPIO_IDR_ID5
#define GPIO_IDR_IDR_6                      GPIO_IDR_ID6
#define GPIO_IDR_IDR_7                      GPIO_IDR_ID7
#define GPIO_IDR_IDR_8                      GPIO_IDR_ID8
#define GPIO_IDR_IDR_9                      GPIO_IDR_ID9
#define GPIO_IDR_IDR_10                     GPIO_IDR_ID10
#define GPIO_IDR_IDR_11                     GPIO_IDR_ID11
#define GPIO_IDR_IDR_12                     GPIO_IDR_ID12
#define GPIO_IDR_IDR_13                     GPIO_IDR_ID13
#define GPIO_IDR_IDR_14                     GPIO_IDR_ID14
#define GPIO_IDR_IDR_15                     GPIO_IDR_ID15

/* Old GPIO_IDR register bits definition, maintained for legacy purpose */
#define GPIO_OTYPER_IDR_0                   GPIO_IDR_ID0
#define GPIO_OTYPER_IDR_1                   GPIO_IDR_ID1
#define GPIO_OTYPER_IDR_2                   GPIO_IDR_ID2
#define GPIO_OTYPER_IDR_3                   GPIO_IDR_ID3
#define GPIO_OTYPER_IDR_4                   GPIO_IDR_ID4
#define GPIO_OTYPER_IDR_5                   GPIO_IDR_ID5
#define GPIO_OTYPER_IDR_6                   GPIO_IDR_ID6
#define GPIO_OTYPER_IDR_7                   GPIO_IDR_ID7
#define GPIO_OTYPER_IDR_8                   GPIO_IDR_ID8
#define GPIO_OTYPER_IDR_9                   GPIO_IDR_ID9
#define GPIO_OTYPER_IDR_10                  GPIO_IDR_ID10
#define GPIO_OTYPER_IDR_11                  GPIO_IDR_ID11
#define GPIO_OTYPER_IDR_12                  GPIO_IDR_ID12
#define GPIO_OTYPER_IDR_13                  GPIO_IDR_ID13
#define GPIO_OTYPER_IDR_14                  GPIO_IDR_ID14
#define GPIO_OTYPER_IDR_15                  GPIO_IDR_ID15

/******************  Bits definition for GPIO_ODR register  *******************/
#define GPIO_ODR_OD0_Pos               (0U)
#define GPIO_ODR_OD0_Msk               (0x1UL << GPIO_ODR_OD0_Pos)             /*!< 0x00000001 */
#define GPIO_ODR_OD0                   GPIO_ODR_OD0_Msk
#define GPIO_ODR_OD1_Pos               (1U)
#define GPIO_ODR_OD1_Msk               (0x1UL << GPIO_ODR_OD1_Pos)             /*!< 0x00000002 */
#define GPIO_ODR_OD1                   GPIO_ODR_OD1_Msk
#define GPIO_ODR_OD2_Pos               (2U)
#define GPIO_ODR_OD2_Msk               (0x1UL << GPIO_ODR_OD2_Pos)             /*!< 0x00000004 */
#define GPIO_ODR_OD2                   GPIO_ODR_OD2_Msk
#define GPIO_ODR_OD3_Pos               (3U)
#define GPIO_ODR_OD3_Msk               (0x1UL << GPIO_ODR_OD3_Pos)             /*!< 0x00000008 */
#define GPIO_ODR_OD3                   GPIO_ODR_OD3_Msk
#define GPIO_ODR_OD4_Pos               (4U)
#define GPIO_ODR_OD4_Msk               (0x1UL << GPIO_ODR_OD4_Pos)             /*!< 0x00000010 */
#define GPIO_ODR_OD4                   GPIO_ODR_OD4_Msk
#define GPIO_ODR_OD5_Pos               (5U)
#define GPIO_ODR_OD5_Msk               (0x1UL << GPIO_ODR_OD5_Pos)             /*!< 0x00000020 */
#define GPIO_ODR_OD5                   GPIO_ODR_OD5_Msk
#define GPIO_ODR_OD6_Pos               (6U)
#define GPIO_ODR_OD6_Msk               (0x1UL << GPIO_ODR_OD6_Pos)             /*!< 0x00000040 */
#define GPIO_ODR_OD6                   GPIO_ODR_OD6_Msk
#define GPIO_ODR_OD7_Pos               (7U)
#define GPIO_ODR_OD7_Msk               (0x1UL << GPIO_ODR_OD7_Pos)             /*!< 0x00000080 */
#define GPIO_ODR_OD7                   GPIO_ODR_OD7_Msk
#define GPIO_ODR_OD8_Pos               (8U)
#define GPIO_ODR_OD8_Msk               (0x1UL << GPIO_ODR_OD8_Pos)             /*!< 0x00000100 */
#define GPIO_ODR_OD8                   GPIO_ODR_OD8_Msk
#define GPIO_ODR_OD9_Pos               (9U)
#define GPIO_ODR_OD9_Msk               (0x1UL << GPIO_ODR_OD9_Pos)             /*!< 0x00000200 */
#define GPIO_ODR_OD9                   GPIO_ODR_OD9_Msk
#define GPIO_ODR_OD10_Pos              (10U)
#define GPIO_ODR_OD10_Msk              (0x1UL << GPIO_ODR_OD10_Pos)            /*!< 0x00000400 */
#define GPIO_ODR_OD10                  GPIO_ODR_OD10_Msk
#define GPIO_ODR_OD11_Pos              (11U)
#define GPIO_ODR_OD11_Msk              (0x1UL << GPIO_ODR_OD11_Pos)            /*!< 0x00000800 */
#define GPIO_ODR_OD11                  GPIO_ODR_OD11_Msk
#define GPIO_ODR_OD12_Pos              (12U)
#define GPIO_ODR_OD12_Msk              (0x1UL << GPIO_ODR_OD12_Pos)            /*!< 0x00001000 */
#define GPIO_ODR_OD12                  GPIO_ODR_OD12_Msk
#define GPIO_ODR_OD13_Pos              (13U)
#define GPIO_ODR_OD13_Msk              (0x1UL << GPIO_ODR_OD13_Pos)            /*!< 0x00002000 */
#define GPIO_ODR_OD13                  GPIO_ODR_OD13_Msk
#define GPIO_ODR_OD14_Pos              (14U)
#define GPIO_ODR_OD14_Msk              (0x1UL << GPIO_ODR_OD14_Pos)            /*!< 0x00004000 */
#define GPIO_ODR_OD14                  GPIO_ODR_OD14_Msk
#define GPIO_ODR_OD15_Pos              (15U)
#define GPIO_ODR_OD15_Msk              (0x1UL << GPIO_ODR_OD15_Pos)            /*!< 0x00008000 */
#define GPIO_ODR_OD15                  GPIO_ODR_OD15_Msk

/* Legacy defines */
#define GPIO_ODR_ODR_0                      GPIO_ODR_OD0
#define GPIO_ODR_ODR_1                      GPIO_ODR_OD1
#define GPIO_ODR_ODR_2                      GPIO_ODR_OD2
#define GPIO_ODR_ODR_3                      GPIO_ODR_OD3
#define GPIO_ODR_ODR_4                      GPIO_ODR_OD4
#define GPIO_ODR_ODR_5                      GPIO_ODR_OD5
#define GPIO_ODR_ODR_6                      GPIO_ODR_OD6
#define GPIO_ODR_ODR_7                      GPIO_ODR_OD7
#define GPIO_ODR_ODR_8                      GPIO_ODR_OD8
#define GPIO_ODR_ODR_9                      GPIO_ODR_OD9
#define GPIO_ODR_ODR_10                     GPIO_ODR_OD10
#define GPIO_ODR_ODR_11                     GPIO_ODR_OD11
#define GPIO_ODR_ODR_12                     GPIO_ODR_OD12
#define GPIO_ODR_ODR_13                     GPIO_ODR_OD13
#define GPIO_ODR_ODR_14                     GPIO_ODR_OD14
#define GPIO_ODR_ODR_15                     GPIO_ODR_OD15

/* Old GPIO_ODR register bits definition, maintained for legacy purpose */
#define GPIO_OTYPER_ODR_0                   GPIO_ODR_OD0
#define GPIO_OTYPER_ODR_1                   GPIO_ODR_OD1
#define GPIO_OTYPER_ODR_2                   GPIO_ODR_OD2
#define GPIO_OTYPER_ODR_3                   GPIO_ODR_OD3
#define GPIO_OTYPER_ODR_4                   GPIO_ODR_OD4
#define GPIO_OTYPER_ODR_5                   GPIO_ODR_OD5
#define GPIO_OTYPER_ODR_6                   GPIO_ODR_OD6
#define GPIO_OTYPER_ODR_7                   GPIO_ODR_OD7
#define GPIO_OTYPER_ODR_8                   GPIO_ODR_OD8
#define GPIO_OTYPER_ODR_9                   GPIO_ODR_OD9
#define GPIO_OTYPER_ODR_10                  GPIO_ODR_OD10
#define GPIO_OTYPER_ODR_11                  GPIO_ODR_OD11
#define GPIO_OTYPER_ODR_12                  GPIO_ODR_OD12
#define GPIO_OTYPER_ODR_13                  GPIO_ODR_OD13
#define GPIO_OTYPER_ODR_14                  GPIO_ODR_OD14
#define GPIO_OTYPER_ODR_15                  GPIO_ODR_OD15

/******************  Bits definition for GPIO_BSRR register  ******************/
#define GPIO_BSRR_BS0_Pos              (0U)
#define GPIO_BSRR_BS0_Msk              (0x1UL << GPIO_BSRR_BS0_Pos)            /*!< 0x00000001 */
#define GPIO_BSRR_BS0                  GPIO_BSRR_BS0_Msk
#define GPIO_BSRR_BS1_Pos              (1U)
#define GPIO_BSRR_BS1_Msk              (0x1UL << GPIO_BSRR_BS1_Pos)            /*!< 0x00000002 */
#define GPIO_BSRR_BS1                  GPIO_BSRR_BS1_Msk
#define GPIO_BSRR_BS2_Pos              (2U)
#define GPIO_BSRR_BS2_Msk              (0x1UL << GPIO_BSRR_BS2_Pos)            /*!< 0x00000004 */
#define GPIO_BSRR_BS2                  GPIO_BSRR_BS2_Msk
#define GPIO_BSRR_BS3_Pos              (3U)
#define GPIO_BSRR_BS3_Msk              (0x1UL << GPIO_BSRR_BS3_Pos)            /*!< 0x00000008 */
#define GPIO_BSRR_BS3                  GPIO_BSRR_BS3_Msk
#define GPIO_BSRR_BS4_Pos              (4U)
#define GPIO_BSRR_BS4_Msk              (0x1UL << GPIO_BSRR_BS4_Pos)            /*!< 0x00000010 */
#define GPIO_BSRR_BS4                  GPIO_BSRR_BS4_Msk
#define GPIO_BSRR_BS5_Pos              (5U)
#define GPIO_BSRR_BS5_Msk              (0x1UL << GPIO_BSRR_BS5_Pos)            /*!< 0x00000020 */
#define GPIO_BSRR_BS5                  GPIO_BSRR_BS5_Msk
#define GPIO_BSRR_BS6_Pos              (6U)
#define GPIO_BSRR_BS6_Msk              (0x1UL << GPIO_BSRR_BS6_Pos)            /*!< 0x00000040 */
#define GPIO_BSRR_BS6                  GPIO_BSRR_BS6_Msk
#define GPIO_BSRR_BS7_Pos              (7U)
#define GPIO_BSRR_BS7_Msk              (0x1UL << GPIO_BSRR_BS7_Pos)            /*!< 0x00000080 */
#define GPIO_BSRR_BS7                  GPIO_BSRR_BS7_Msk
#define GPIO_BSRR_BS8_Pos              (8U)
#define GPIO_BSRR_BS8_Msk              (0x1UL << GPIO_BSRR_BS8_Pos)            /*!< 0x00000100 */
#define GPIO_BSRR_BS8                  GPIO_BSRR_BS8_Msk
#define GPIO_BSRR_BS9_Pos              (9U)
#define GPIO_BSRR_BS9_Msk              (0x1UL << GPIO_BSRR_BS9_Pos)            /*!< 0x00000200 */
#define GPIO_BSRR_BS9                  GPIO_BSRR_BS9_Msk
#define GPIO_BSRR_BS10_Pos             (10U)
#define GPIO_BSRR_BS10_Msk             (0x1UL << GPIO_BSRR_BS10_Pos)           /*!< 0x00000400 */
#define GPIO_BSRR_BS10                 GPIO_BSRR_BS10_Msk
#define GPIO_BSRR_BS11_Pos             (11U)
#define GPIO_BSRR_BS11_Msk             (0x1UL << GPIO_BSRR_BS11_Pos)           /*!< 0x00000800 */
#define GPIO_BSRR_BS11                 GPIO_BSRR_BS11_Msk
#define GPIO_BSRR_BS12_Pos             (12U)
#define GPIO_BSRR_BS12_Msk             (0x1UL << GPIO_BSRR_BS12_Pos)           /*!< 0x00001000 */
#define GPIO_BSRR_BS12                 GPIO_BSRR_BS12_Msk
#define GPIO_BSRR_BS13_Pos             (13U)
#define GPIO_BSRR_BS13_Msk             (0x1UL << GPIO_BSRR_BS13_Pos)           /*!< 0x00002000 */
#define GPIO_BSRR_BS13                 GPIO_BSRR_BS13_Msk
#define GPIO_BSRR_BS14_Pos             (14U)
#define GPIO_BSRR_BS14_Msk             (0x1UL << GPIO_BSRR_BS14_Pos)           /*!< 0x00004000 */
#define GPIO_BSRR_BS14                 GPIO_BSRR_BS14_Msk
#define GPIO_BSRR_BS15_Pos             (15U)
#define GPIO_BSRR_BS15_Msk             (0x1UL << GPIO_BSRR_BS15_Pos)           /*!< 0x00008000 */
#define GPIO_BSRR_BS15                 GPIO_BSRR_BS15_Msk
#define GPIO_BSRR_BR0_Pos              (16U)
#define GPIO_BSRR_BR0_Msk              (0x1UL << GPIO_BSRR_BR0_Pos)            /*!< 0x00010000 */
#define GPIO_BSRR_BR0                  GPIO_BSRR_BR0_Msk
#define GPIO_BSRR_BR1_Pos              (17U)
#define GPIO_BSRR_BR1_Msk              (0x1UL << GPIO_BSRR_BR1_Pos)            /*!< 0x00020000 */
#define GPIO_BSRR_BR1                  GPIO_BSRR_BR1_Msk
#define GPIO_BSRR_BR2_Pos              (18U)
#define GPIO_BSRR_BR2_Msk              (0x1UL << GPIO_BSRR_BR2_Pos)            /*!< 0x00040000 */
#define GPIO_BSRR_BR2                  GPIO_BSRR_BR2_Msk
#define GPIO_BSRR_BR3_Pos              (19U)
#define GPIO_BSRR_BR3_Msk              (0x1UL << GPIO_BSRR_BR3_Pos)            /*!< 0x00080000 */
#define GPIO_BSRR_BR3                  GPIO_BSRR_BR3_Msk
#define GPIO_BSRR_BR4_Pos              (20U)
#define GPIO_BSRR_BR4_Msk              (0x1UL << GPIO_BSRR_BR4_Pos)            /*!< 0x00100000 */
#define GPIO_BSRR_BR4                  GPIO_BSRR_BR4_Msk
#define GPIO_BSRR_BR5_Pos              (21U)
#define GPIO_BSRR_BR5_Msk              (0x1UL << GPIO_BSRR_BR5_Pos)            /*!< 0x00200000 */
#define GPIO_BSRR_BR5                  GPIO_BSRR_BR5_Msk
#define GPIO_BSRR_BR6_Pos              (22U)
#define GPIO_BSRR_BR6_Msk              (0x1UL << GPIO_BSRR_BR6_Pos)            /*!< 0x00400000 */
#define GPIO_BSRR_BR6                  GPIO_BSRR_BR6_Msk
#define GPIO_BSRR_BR7_Pos              (23U)
#define GPIO_BSRR_BR7_Msk              (0x1UL << GPIO_BSRR_BR7_Pos)            /*!< 0x00800000 */
#define GPIO_BSRR_BR7                  GPIO_BSRR_BR7_Msk
#define GPIO_BSRR_BR8_Pos              (24U)
#define GPIO_BSRR_BR8_Msk              (0x1UL << GPIO_BSRR_BR8_Pos)            /*!< 0x01000000 */
#define GPIO_BSRR_BR8                  GPIO_BSRR_BR8_Msk
#define GPIO_BSRR_BR9_Pos              (25U)
#define GPIO_BSRR_BR9_Msk              (0x1UL << GPIO_BSRR_BR9_Pos)            /*!< 0x02000000 */
#define GPIO_BSRR_BR9                  GPIO_BSRR_BR9_Msk
#define GPIO_BSRR_BR10_Pos             (26U)
#define GPIO_BSRR_BR10_Msk             (0x1UL << GPIO_BSRR_BR10_Pos)           /*!< 0x04000000 */
#define GPIO_BSRR_BR10                 GPIO_BSRR_BR10_Msk
#define GPIO_BSRR_BR11_Pos             (27U)
#define GPIO_BSRR_BR11_Msk             (0x1UL << GPIO_BSRR_BR11_Pos)           /*!< 0x08000000 */
#define GPIO_BSRR_BR11                 GPIO_BSRR_BR11_Msk
#define GPIO_BSRR_BR12_Pos             (28U)
#define GPIO_BSRR_BR12_Msk             (0x1UL << GPIO_BSRR_BR12_Pos)           /*!< 0x10000000 */
#define GPIO_BSRR_BR12                 GPIO_BSRR_BR12_Msk
#define GPIO_BSRR_BR13_Pos             (29U)
#define GPIO_BSRR_BR13_Msk             (0x1UL << GPIO_BSRR_BR13_Pos)           /*!< 0x20000000 */
#define GPIO_BSRR_BR13                 GPIO_BSRR_BR13_Msk
#define GPIO_BSRR_BR14_Pos             (30U)
#define GPIO_BSRR_BR14_Msk             (0x1UL << GPIO_BSRR_BR14_Pos)           /*!< 0x40000000 */
#define GPIO_BSRR_BR14                 GPIO_BSRR_BR14_Msk
#define GPIO_BSRR_BR15_Pos             (31U)
#define GPIO_BSRR_BR15_Msk             (0x1UL << GPIO_BSRR_BR15_Pos)           /*!< 0x80000000 */
#define GPIO_BSRR_BR15                 GPIO_BSRR_BR15_Msk

/* Legacy defines */
#define GPIO_BSRR_BS_0                      GPIO_BSRR_BS0
#define GPIO_BSRR_BS_1                      GPIO_BSRR_BS1
#define GPIO_BSRR_BS_2                      GPIO_BSRR_BS2
#define GPIO_BSRR_BS_3                      GPIO_BSRR_BS3
#define GPIO_BSRR_BS_4                      GPIO_BSRR_BS4
#define GPIO_BSRR_BS_5                      GPIO_BSRR_BS5
#define GPIO_BSRR_BS_6                      GPIO_BSRR_BS6
#define GPIO_BSRR_BS_7                      GPIO_BSRR_BS7
#define GPIO_BSRR_BS_8                      GPIO_BSRR_BS8
#define GPIO_BSRR_BS_9                      GPIO_BSRR_BS9
#define GPIO_BSRR_BS_10                     GPIO_BSRR_BS10
#define GPIO_BSRR_BS_11                     GPIO_BSRR_BS11
#define GPIO_BSRR_BS_12                     GPIO_BSRR_BS12
#define GPIO_BSRR_BS_13                     GPIO_BSRR_BS13
#define GPIO_BSRR_BS_14                     GPIO_BSRR_BS14
#define GPIO_BSRR_BS_15                     GPIO_BSRR_BS15
#define GPIO_BSRR_BR_0                      GPIO_BSRR_BR0
#define GPIO_BSRR_BR_1                      GPIO_BSRR_BR1
#define GPIO_BSRR_BR_2                      GPIO_BSRR_BR2
#define GPIO_BSRR_BR_3                      GPIO_BSRR_BR3
#define GPIO_BSRR_BR_4                      GPIO_BSRR_BR4
#define GPIO_BSRR_BR_5                      GPIO_BSRR_BR5
#define GPIO_BSRR_BR_6                      GPIO_BSRR_BR6
#define GPIO_BSRR_BR_7                      GPIO_BSRR_BR7
#define GPIO_BSRR_BR_8                      GPIO_BSRR_BR8
#define GPIO_BSRR_BR_9                      GPIO_BSRR_BR9
#define GPIO_BSRR_BR_10                     GPIO_BSRR_BR10
#define GPIO_BSRR_BR_11                     GPIO_BSRR_BR11
#define GPIO_BSRR_BR_12                     GPIO_BSRR_BR12
#define GPIO_BSRR_BR_13                     GPIO_BSRR_BR13
#define GPIO_BSRR_BR_14                     GPIO_BSRR_BR14
#define GPIO_BSRR_BR_15                     GPIO_BSRR_BR15

/****************** Bit definition for GPIO_LCKR register *********************/
#define GPIO_LCKR_LCK0_Pos             (0U)
#define GPIO_LCKR_LCK0_Msk             (0x1UL << GPIO_LCKR_LCK0_Pos)           /*!< 0x00000001 */
#define GPIO_LCKR_LCK0                 GPIO_LCKR_LCK0_Msk
#define GPIO_LCKR_LCK1_Pos             (1U)
#define GPIO_LCKR_LCK1_Msk             (0x1UL << GPIO_LCKR_LCK1_Pos)           /*!< 0x00000002 */
#define GPIO_LCKR_LCK1                 GPIO_LCKR_LCK1_Msk
#define GPIO_LCKR_LCK2_Pos             (2U)
#define GPIO_LCKR_LCK2_Msk             (0x1UL << GPIO_LCKR_LCK2_Pos)           /*!< 0x00000004 */
#define GPIO_LCKR_LCK2                 GPIO_LCKR_LCK2_Msk
#define GPIO_LCKR_LCK3_Pos             (3U)
#define GPIO_LCKR_LCK3_Msk             (0x1UL << GPIO_LCKR_LCK3_Pos)           /*!< 0x00000008 */
#define GPIO_LCKR_LCK3                 GPIO_LCKR_LCK3_Msk
#define GPIO_LCKR_LCK4_Pos             (4U)
#define GPIO_LCKR_LCK4_Msk             (0x1UL << GPIO_LCKR_LCK4_Pos)           /*!< 0x00000010 */
#define GPIO_LCKR_LCK4                 GPIO_LCKR_LCK4_Msk
#define GPIO_LCKR_LCK5_Pos             (5U)
#define GPIO_LCKR_LCK5_Msk             (0x1UL << GPIO_LCKR_LCK5_Pos)           /*!< 0x00000020 */
#define GPIO_LCKR_LCK5                 GPIO_LCKR_LCK5_Msk
#define GPIO_LCKR_LCK6_Pos             (6U)
#define GPIO_LCKR_LCK6_Msk             (0x1UL << GPIO_LCKR_LCK6_Pos)           /*!< 0x00000040 */
#define GPIO_LCKR_LCK6                 GPIO_LCKR_LCK6_Msk
#define GPIO_LCKR_LCK7_Pos             (7U)
#define GPIO_LCKR_LCK7_Msk             (0x1UL << GPIO_LCKR_LCK7_Pos)           /*!< 0x00000080 */
#define GPIO_LCKR_LCK7                 GPIO_LCKR_LCK7_Msk
#define GPIO_LCKR_LCK8_Pos             (8U)
#define GPIO_LCKR_LCK8_Msk             (0x1UL << GPIO_LCKR_LCK8_Pos)           /*!< 0x00000100 */
#define GPIO_LCKR_LCK8                 GPIO_LCKR_LCK8_Msk
#define GPIO_LCKR_LCK9_Pos             (9U)
#define GPIO_LCKR_LCK9_Msk             (0x1UL << GPIO_LCKR_LCK9_Pos)           /*!< 0x00000200 */
#define GPIO_LCKR_LCK9                 GPIO_LCKR_LCK9_Msk
#define GPIO_LCKR_LCK10_Pos            (10U)
#define GPIO_LCKR_LCK10_Msk            (0x1UL << GPIO_LCKR_LCK10_Pos)          /*!< 0x00000400 */
#define GPIO_LCKR_LCK10                GPIO_LCKR_LCK10_Msk
#define GPIO_LCKR_LCK11_Pos            (11U)
#define GPIO_LCKR_LCK11_Msk            (0x1UL << GPIO_LCKR_LCK11_Pos)          /*!< 0x00000800 */
#define GPIO_LCKR_LCK11                GPIO_LCKR_LCK11_Msk
#define GPIO_LCKR_LCK12_Pos            (12U)
#define GPIO_LCKR_LCK12_Msk            (0x1UL << GPIO_LCKR_LCK12_Pos)          /*!< 0x00001000 */
#define GPIO_LCKR_LCK12                GPIO_LCKR_LCK12_Msk
#define GPIO_LCKR_LCK13_Pos            (13U)
#define GPIO_LCKR_LCK13_Msk            (0x1UL << GPIO_LCKR_LCK13_Pos)          /*!< 0x00002000 */
#define GPIO_LCKR_LCK13                GPIO_LCKR_LCK13_Msk
#define GPIO_LCKR_LCK14_Pos            (14U)
#define GPIO_LCKR_LCK14_Msk            (0x1UL << GPIO_LCKR_LCK14_Pos)          /*!< 0x00004000 */
#define GPIO_LCKR_LCK14                GPIO_LCKR_LCK14_Msk
#define GPIO_LCKR_LCK15_Pos            (15U)
#define GPIO_LCKR_LCK15_Msk            (0x1UL << GPIO_LCKR_LCK15_Pos)          /*!< 0x00008000 */
#define GPIO_LCKR_LCK15                GPIO_LCKR_LCK15_Msk
#define GPIO_LCKR_LCKK_Pos             (16U)
#define GPIO_LCKR_LCKK_Msk             (0x1UL << GPIO_LCKR_LCKK_Pos)           /*!< 0x00010000 */
#define GPIO_LCKR_LCKK                 GPIO_LCKR_LCKK_Msk

/****************** Bit definition for GPIO_AFRL register *********************/
#define GPIO_AFRL_AFSEL0_Pos           (0U)
#define GPIO_AFRL_AFSEL0_Msk           (0xFUL << GPIO_AFRL_AFSEL0_Pos)         /*!< 0x0000000F */
#define GPIO_AFRL_AFSEL0               GPIO_AFRL_AFSEL0_Msk
#define GPIO_AFRL_AFSEL0_0             (0x1UL << GPIO_AFRL_AFSEL0_Pos)         /*!< 0x00000001 */
#define GPIO_AFRL_AFSEL0_1             (0x2UL << GPIO_AFRL_AFSEL0_Pos)         /*!< 0x00000002 */
#define GPIO_AFRL_AFSEL0_2             (0x4UL << GPIO_AFRL_AFSEL0_Pos)         /*!< 0x00000004 */
#define GPIO_AFRL_AFSEL0_3             (0x8UL << GPIO_AFRL_AFSEL0_Pos)         /*!< 0x00000008 */
#define GPIO_AFRL_AFSEL1_Pos           (4U)
#define GPIO_AFRL_AFSEL1_Msk           (0xFUL << GPIO_AFRL_AFSEL1_Pos)         /*!< 0x000000F0 */
#define GPIO_AFRL_AFSEL1               GPIO_AFRL_AFSEL1_Msk
#define GPIO_AFRL_AFSEL1_0             (0x1UL << GPIO_AFRL_AFSEL1_Pos)         /*!< 0x00000010 */
#define GPIO_AFRL_AFSEL1_1             (0x2UL << GPIO_AFRL_AFSEL1_Pos)         /*!< 0x00000020 */
#define GPIO_AFRL_AFSEL1_2             (0x4UL << GPIO_AFRL_AFSEL1_Pos)         /*!< 0x00000040 */
#define GPIO_AFRL_AFSEL1_3             (0x8UL << GPIO_AFRL_AFSEL1_Pos)         /*!< 0x00000080 */
#define GPIO_AFRL_AFSEL2_Pos           (8U)
#define GPIO_AFRL_AFSEL2_Msk           (0xFUL << GPIO_AFRL_AFSEL2_Pos)         /*!< 0x00000F00 */
#define GPIO_AFRL_AFSEL2               GPIO_AFRL_AFSEL2_Msk
#define GPIO_AFRL_AFSEL2_0             (0x1UL << GPIO_AFRL_AFSEL2_Pos)         /*!< 0x00000100 */
#define GPIO_AFRL_AFSEL2_1             (0x2UL << GPIO_AFRL_AFSEL2_Pos)         /*!< 0x00000200 */
#define GPIO_AFRL_AFSEL2_2             (0x4UL << GPIO_AFRL_AFSEL2_Pos)         /*!< 0x00000400 */
#define GPIO_AFRL_AFSEL2_3             (0x8UL << GPIO_AFRL_AFSEL2_Pos)         /*!< 0x00000800 */
#define GPIO_AFRL_AFSEL3_Pos           (12U)
#define GPIO_AFRL_AFSEL3_Msk           (0xFUL << GPIO_AFRL_AFSEL3_Pos)         /*!< 0x0000F000 */
#define GPIO_AFRL_AFSEL3               GPIO_AFRL_AFSEL3_Msk
#define GPIO_AFRL_AFSEL3_0             (0x1UL << GPIO_AFRL_AFSEL3_Pos)         /*!< 0x00001000 */
#define GPIO_AFRL_AFSEL3_1             (0x2UL << GPIO_AFRL_AFSEL3_Pos)         /*!< 0x00002000 */
#define GPIO_AFRL_AFSEL3_2             (0x4UL << GPIO_AFRL_AFSEL3_Pos)         /*!< 0x00004000 */
#define GPIO_AFRL_AFSEL3_3             (0x8UL << GPIO_AFRL_AFSEL3_Pos)         /*!< 0x00008000 */
#define GPIO_AFRL_AFSEL4_Pos           (16U)
#define GPIO_AFRL_AFSEL4_Msk           (0xFUL << GPIO_AFRL_AFSEL4_Pos)         /*!< 0x000F0000 */
#define GPIO_AFRL_AFSEL4               GPIO_AFRL_AFSEL4_Msk
#define GPIO_AFRL_AFSEL4_0             (0x1UL << GPIO_AFRL_AFSEL4_Pos)         /*!< 0x00010000 */
#define GPIO_AFRL_AFSEL4_1             (0x2UL << GPIO_AFRL_AFSEL4_Pos)         /*!< 0x00020000 */
#define GPIO_AFRL_AFSEL4_2             (0x4UL << GPIO_AFRL_AFSEL4_Pos)         /*!< 0x00040000 */
#define GPIO_AFRL_AFSEL4_3             (0x8UL << GPIO_AFRL_AFSEL4_Pos)         /*!< 0x00080000 */
#define GPIO_AFRL_AFSEL5_Pos           (20U)
#define GPIO_AFRL_AFSEL5_Msk           (0xFUL << GPIO_AFRL_AFSEL5_Pos)         /*!< 0x00F00000 */
#define GPIO_AFRL_AFSEL5               GPIO_AFRL_AFSEL5_Msk
#define GPIO_AFRL_AFSEL5_0             (0x1UL << GPIO_AFRL_AFSEL5_Pos)         /*!< 0x00100000 */
#define GPIO_AFRL_AFSEL5_1             (0x2UL << GPIO_AFRL_AFSEL5_Pos)         /*!< 0x00200000 */
#define GPIO_AFRL_AFSEL5_2             (0x4UL << GPIO_AFRL_AFSEL5_Pos)         /*!< 0x00400000 */
#define GPIO_AFRL_AFSEL5_3             (0x8UL << GPIO_AFRL_AFSEL5_Pos)         /*!< 0x00800000 */
#define GPIO_AFRL_AFSEL6_Pos           (24U)
#define GPIO_AFRL_AFSEL6_Msk           (0xFUL << GPIO_AFRL_AFSEL6_Pos)         /*!< 0x0F000000 */
#define GPIO_AFRL_AFSEL6               GPIO_AFRL_AFSEL6_Msk
#define GPIO_AFRL_AFSEL6_0             (0x1UL << GPIO_AFRL_AFSEL6_Pos)         /*!< 0x01000000 */
#define GPIO_AFRL_AFSEL6_1             (0x2UL << GPIO_AFRL_AFSEL6_Pos)         /*!< 0x02000000 */
#define GPIO_AFRL_AFSEL6_2             (0x4UL << GPIO_AFRL_AFSEL6_Pos)         /*!< 0x04000000 */
#define GPIO_AFRL_AFSEL6_3             (0x8UL << GPIO_AFRL_AFSEL6_Pos)         /*!< 0x08000000 */
#define GPIO_AFRL_AFSEL7_Pos           (28U)
#define GPIO_AFRL_AFSEL7_Msk           (0xFUL << GPIO_AFRL_AFSEL7_Pos)         /*!< 0xF0000000 */
#define GPIO_AFRL_AFSEL7               GPIO_AFRL_AFSEL7_Msk
#define GPIO_AFRL_AFSEL7_0             (0x1UL << GPIO_AFRL_AFSEL7_Pos)         /*!< 0x10000000 */
#define GPIO_AFRL_AFSEL7_1             (0x2UL << GPIO_AFRL_AFSEL7_Pos)         /*!< 0x20000000 */
#define GPIO_AFRL_AFSEL7_2             (0x4UL << GPIO_AFRL_AFSEL7_Pos)         /*!< 0x40000000 */
#define GPIO_AFRL_AFSEL7_3             (0x8UL << GPIO_AFRL_AFSEL7_Pos)         /*!< 0x80000000 */

/* Legacy defines */
#define GPIO_AFRL_AFRL0                      GPIO_AFRL_AFSEL0
#define GPIO_AFRL_AFRL1                      GPIO_AFRL_AFSEL1
#define GPIO_AFRL_AFRL2                      GPIO_AFRL_AFSEL2
#define GPIO_AFRL_AFRL3                      GPIO_AFRL_AFSEL3
#define GPIO_AFRL_AFRL4                      GPIO_AFRL_AFSEL4
#define GPIO_AFRL_AFRL5                      GPIO_AFRL_AFSEL5
#define GPIO_AFRL_AFRL6                      GPIO_AFRL_AFSEL6
#define GPIO_AFRL_AFRL7                      GPIO_AFRL_AFSEL7

/****************** Bit definition for GPIO_AFRH register *********************/
#define GPIO_AFRH_AFSEL8_Pos           (0U)
#define GPIO_AFRH_AFSEL8_Msk           (0xFUL << GPIO_AFRH_AFSEL8_Pos)         /*!< 0x0000000F */
#define GPIO_AFRH_AFSEL8               GPIO_AFRH_AFSEL8_Msk
#define GPIO_AFRH_AFSEL8_0             (0x1UL << GPIO_AFRH_AFSEL8_Pos)         /*!< 0x00000001 */
#define GPIO_AFRH_AFSEL8_1             (0x2UL << GPIO_AFRH_AFSEL8_Pos)         /*!< 0x00000002 */
#define GPIO_AFRH_AFSEL8_2             (0x4UL << GPIO_AFRH_AFSEL8_Pos)         /*!< 0x00000004 */
#define GPIO_AFRH_AFSEL8_3             (0x8UL << GPIO_AFRH_AFSEL8_Pos)         /*!< 0x00000008 */
#define GPIO_AFRH_AFSEL9_Pos           (4U)
#define GPIO_AFRH_AFSEL9_Msk           (0xFUL << GPIO_AFRH_AFSEL9_Pos)         /*!< 0x000000F0 */
#define GPIO_AFRH_AFSEL9               GPIO_AFRH_AFSEL9_Msk
#define GPIO_AFRH_AFSEL9_0             (0x1UL << GPIO_AFRH_AFSEL9_Pos)         /*!< 0x00000010 */
#define GPIO_AFRH_AFSEL9_1             (0x2UL << GPIO_AFRH_AFSEL9_Pos)         /*!< 0x00000020 */
#define GPIO_AFRH_AFSEL9_2             (0x4UL << GPIO_AFRH_AFSEL9_Pos)         /*!< 0x00000040 */
#define GPIO_AFRH_AFSEL9_3             (0x8UL << GPIO_AFRH_AFSEL9_Pos)         /*!< 0x00000080 */
#define GPIO_AFRH_AFSEL10_Pos          (8U)
#define GPIO_AFRH_AFSEL10_Msk          (0xFUL << GPIO_AFRH_AFSEL10_Pos)        /*!< 0x00000F00 */
#define GPIO_AFRH_AFSEL10              GPIO_AFRH_AFSEL10_Msk
#define GPIO_AFRH_AFSEL10_0            (0x1UL << GPIO_AFRH_AFSEL10_Pos)        /*!< 0x00000100 */
#define GPIO_AFRH_AFSEL10_1            (0x2UL << GPIO_AFRH_AFSEL10_Pos)        /*!< 0x00000200 */
#define GPIO_AFRH_AFSEL10_2            (0x4UL << GPIO_AFRH_AFSEL10_Pos)        /*!< 0x00000400 */
#define GPIO_AFRH_AFSEL10_3            (0x8UL << GPIO_AFRH_AFSEL10_Pos)        /*!< 0x00000800 */
#define GPIO_AFRH_AFSEL11_Pos          (12U)
#define GPIO_AFRH_AFSEL11_Msk          (0xFUL << GPIO_AFRH_AFSEL11_Pos)        /*!< 0x0000F000 */
#define GPIO_AFRH_AFSEL11              GPIO_AFRH_AFSEL11_Msk
#define GPIO_AFRH_AFSEL11_0            (0x1UL << GPIO_AFRH_AFSEL11_Pos)        /*!< 0x00001000 */
#define GPIO_AFRH_AFSEL11_1            (0x2UL << GPIO_AFRH_AFSEL11_Pos)        /*!< 0x00002000 */
#define GPIO_AFRH_AFSEL11_2            (0x4UL << GPIO_AFRH_AFSEL11_Pos)        /*!< 0x00004000 */
#define GPIO_AFRH_AFSEL11_3            (0x8UL << GPIO_AFRH_AFSEL11_Pos)        /*!< 0x00008000 */
#define GPIO_AFRH_AFSEL12_Pos          (16U)
#define GPIO_AFRH_AFSEL12_Msk          (0xFUL << GPIO_AFRH_AFSEL12_Pos)        /*!< 0x000F0000 */
#define GPIO_AFRH_AFSEL12              GPIO_AFRH_AFSEL12_Msk
#define GPIO_AFRH_AFSEL12_0            (0x1UL << GPIO_AFRH_AFSEL12_Pos)        /*!< 0x00010000 */
#define GPIO_AFRH_AFSEL12_1            (0x2UL << GPIO_AFRH_AFSEL12_Pos)        /*!< 0x00020000 */
#define GPIO_AFRH_AFSEL12_2            (0x4UL << GPIO_AFRH_AFSEL12_Pos)        /*!< 0x00040000 */
#define GPIO_AFRH_AFSEL12_3            (0x8UL << GPIO_AFRH_AFSEL12_Pos)        /*!< 0x00080000 */
#define GPIO_AFRH_AFSEL13_Pos          (20U)
#define GPIO_AFRH_AFSEL13_Msk          (0xFUL << GPIO_AFRH_AFSEL13_Pos)        /*!< 0x00F00000 */
#define GPIO_AFRH_AFSEL13              GPIO_AFRH_AFSEL13_Msk
#define GPIO_AFRH_AFSEL13_0            (0x1UL << GPIO_AFRH_AFSEL13_Pos)        /*!< 0x00100000 */
#define GPIO_AFRH_AFSEL13_1            (0x2UL << GPIO_AFRH_AFSEL13_Pos)        /*!< 0x00200000 */
#define GPIO_AFRH_AFSEL13_2            (0x4UL << GPIO_AFRH_AFSEL13_Pos)        /*!< 0x00400000 */
#define GPIO_AFRH_AFSEL13_3            (0x8UL << GPIO_AFRH_AFSEL13_Pos)        /*!< 0x00800000 */
#define GPIO_AFRH_AFSEL14_Pos          (24U)
#define GPIO_AFRH_AFSEL14_Msk          (0xFUL << GPIO_AFRH_AFSEL14_Pos)        /*!< 0x0F000000 */
#define GPIO_AFRH_AFSEL14              GPIO_AFRH_AFSEL14_Msk
#define GPIO_AFRH_AFSEL14_0            (0x1UL << GPIO_AFRH_AFSEL14_Pos)        /*!< 0x01000000 */
#define GPIO_AFRH_AFSEL14_1            (0x2UL << GPIO_AFRH_AFSEL14_Pos)        /*!< 0x02000000 */
#define GPIO_AFRH_AFSEL14_2            (0x4UL << GPIO_AFRH_AFSEL14_Pos)        /*!< 0x04000000 */
#define GPIO_AFRH_AFSEL14_3            (0x8UL << GPIO_AFRH_AFSEL14_Pos)        /*!< 0x08000000 */
#define GPIO_AFRH_AFSEL15_Pos          (28U)
#define GPIO_AFRH_AFSEL15_Msk          (0xFUL << GPIO_AFRH_AFSEL15_Pos)        /*!< 0xF0000000 */
#define GPIO_AFRH_AFSEL15              GPIO_AFRH_AFSEL15_Msk
#define GPIO_AFRH_AFSEL15_0            (0x1UL << GPIO_AFRH_AFSEL15_Pos)        /*!< 0x10000000 */
#define GPIO_AFRH_AFSEL15_1            (0x2UL << GPIO_AFRH_AFSEL15_Pos)        /*!< 0x20000000 */
#define GPIO_AFRH_AFSEL15_2            (0x4UL << GPIO_AFRH_AFSEL15_Pos)        /*!< 0x40000000 */
#define GPIO_AFRH_AFSEL15_3            (0x8UL << GPIO_AFRH_AFSEL15_Pos)        /*!< 0x80000000 */

/* Legacy defines */
#define GPIO_AFRH_AFRH0                      GPIO_AFRH_AFSEL8
#define GPIO_AFRH_AFRH1                      GPIO_AFRH_AFSEL9
#define GPIO_AFRH_AFRH2                      GPIO_AFRH_AFSEL10
#define GPIO_AFRH_AFRH3                      GPIO_AFRH_AFSEL11
#define GPIO_AFRH_AFRH4                      GPIO_AFRH_AFSEL12
#define GPIO_AFRH_AFRH5                      GPIO_AFRH_AFSEL13
#define GPIO_AFRH_AFRH6                      GPIO_AFRH_AFSEL14
#define GPIO_AFRH_AFRH7                      GPIO_AFRH_AFSEL15

/******************  Bits definition for GPIO_BRR register  ******************/
#define GPIO_BRR_BR0_Pos               (0U)
#define GPIO_BRR_BR0_Msk               (0x1UL << GPIO_BRR_BR0_Pos)             /*!< 0x00000001 */
#define GPIO_BRR_BR0                   GPIO_BRR_BR0_Msk
#define GPIO_BRR_BR1_Pos               (1U)
#define GPIO_BRR_BR1_Msk               (0x1UL << GPIO_BRR_BR1_Pos)             /*!< 0x00000002 */
#define GPIO_BRR_BR1                   GPIO_BRR_BR1_Msk
#define GPIO_BRR_BR2_Pos               (2U)
#define GPIO_BRR_BR2_Msk               (0x1UL << GPIO_BRR_BR2_Pos)             /*!< 0x00000004 */
#define GPIO_BRR_BR2                   GPIO_BRR_BR2_Msk
#define GPIO_BRR_BR3_Pos               (3U)
#define GPIO_BRR_BR3_Msk               (0x1UL << GPIO_BRR_BR3_Pos)             /*!< 0x00000008 */
#define GPIO_BRR_BR3                   GPIO_BRR_BR3_Msk
#define GPIO_BRR_BR4_Pos               (4U)
#define GPIO_BRR_BR4_Msk               (0x1UL << GPIO_BRR_BR4_Pos)             /*!< 0x00000010 */
#define GPIO_BRR_BR4                   GPIO_BRR_BR4_Msk
#define GPIO_BRR_BR5_Pos               (5U)
#define GPIO_BRR_BR5_Msk               (0x1UL << GPIO_BRR_BR5_Pos)             /*!< 0x00000020 */
#define GPIO_BRR_BR5                   GPIO_BRR_BR5_Msk
#define GPIO_BRR_BR6_Pos               (6U)
#define GPIO_BRR_BR6_Msk               (0x1UL << GPIO_BRR_BR6_Pos)             /*!< 0x00000040 */
#define GPIO_BRR_BR6                   GPIO_BRR_BR6_Msk
#define GPIO_BRR_BR7_Pos               (7U)
#define GPIO_BRR_BR7_Msk               (0x1UL << GPIO_BRR_BR7_Pos)             /*!< 0x00000080 */
#define GPIO_BRR_BR7                   GPIO_BRR_BR7_Msk
#define GPIO_BRR_BR8_Pos               (8U)
#define GPIO_BRR_BR8_Msk               (0x1UL << GPIO_BRR_BR8_Pos)             /*!< 0x00000100 */
#define GPIO_BRR_BR8                   GPIO_BRR_BR8_Msk
#define GPIO_BRR_BR9_Pos               (9U)
#define GPIO_BRR_BR9_Msk               (0x1UL << GPIO_BRR_BR9_Pos)             /*!< 0x00000200 */
#define GPIO_BRR_BR9                   GPIO_BRR_BR9_Msk
#define GPIO_BRR_BR10_Pos              (10U)
#define GPIO_BRR_BR10_Msk              (0x1UL << GPIO_BRR_BR10_Pos)            /*!< 0x00000400 */
#define GPIO_BRR_BR10                  GPIO_BRR_BR10_Msk
#define GPIO_BRR_BR11_Pos              (11U)
#define GPIO_BRR_BR11_Msk              (0x1UL << GPIO_BRR_BR11_Pos)            /*!< 0x00000800 */
#define GPIO_BRR_BR11                  GPIO_BRR_BR11_Msk
#define GPIO_BRR_BR12_Pos              (12U)
#define GPIO_BRR_BR12_Msk              (0x1UL << GPIO_BRR_BR12_Pos)            /*!< 0x00001000 */
#define GPIO_BRR_BR12                  GPIO_BRR_BR12_Msk
#define GPIO_BRR_BR13_Pos              (13U)
#define GPIO_BRR_BR13_Msk              (0x1UL << GPIO_BRR_BR13_Pos)            /*!< 0x00002000 */
#define GPIO_BRR_BR13                  GPIO_BRR_BR13_Msk
#define GPIO_BRR_BR14_Pos              (14U)
#define GPIO_BRR_BR14_Msk              (0x1UL << GPIO_BRR_BR14_Pos)            /*!< 0x00004000 */
#define GPIO_BRR_BR14                  GPIO_BRR_BR14_Msk
#define GPIO_BRR_BR15_Pos              (15U)
#define GPIO_BRR_BR15_Msk              (0x1UL << GPIO_BRR_BR15_Pos)            /*!< 0x00008000 */
#define GPIO_BRR_BR15                  GPIO_BRR_BR15_Msk

/* Legacy defines */
#define GPIO_BRR_BR_0                       GPIO_BRR_BR0
#define GPIO_BRR_BR_1                       GPIO_BRR_BR1
#define GPIO_BRR_BR_2                       GPIO_BRR_BR2
#define GPIO_BRR_BR_3                       GPIO_BRR_BR3
#define GPIO_BRR_BR_4                       GPIO_BRR_BR4
#define GPIO_BRR_BR_5                       GPIO_BRR_BR5
#define GPIO_BRR_BR_6                       GPIO_BRR_BR6
#define GPIO_BRR_BR_7                       GPIO_BRR_BR7
#define GPIO_BRR_BR_8                       GPIO_BRR_BR8
#define GPIO_BRR_BR_9                       GPIO_BRR_BR9
#define GPIO_BRR_BR_10                      GPIO_BRR_BR10
#define GPIO_BRR_BR_11                      GPIO_BRR_BR11
#define GPIO_BRR_BR_12                      GPIO_BRR_BR12
#define GPIO_BRR_BR_13                      GPIO_BRR_BR13
#define GPIO_BRR_BR_14                      GPIO_BRR_BR14
#define GPIO_BRR_BR_15                      GPIO_BRR_BR15



/******************************************************************************/
/*                                                                            */
/*                      Inter-integrated Circuit Interface (I2C)              */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for I2C_CR1 register  *******************/
#define I2C_CR1_PE_Pos               (0U)
#define I2C_CR1_PE_Msk               (0x1UL << I2C_CR1_PE_Pos)                 /*!< 0x00000001 */
#define I2C_CR1_PE                   I2C_CR1_PE_Msk                            /*!< Peripheral enable                   */
#define I2C_CR1_TXIE_Pos             (1U)
#define I2C_CR1_TXIE_Msk             (0x1UL << I2C_CR1_TXIE_Pos)               /*!< 0x00000002 */
#define I2C_CR1_TXIE                 I2C_CR1_TXIE_Msk                          /*!< TX interrupt enable                 */
#define I2C_CR1_RXIE_Pos             (2U)
#define I2C_CR1_RXIE_Msk             (0x1UL << I2C_CR1_RXIE_Pos)               /*!< 0x00000004 */
#define I2C_CR1_RXIE                 I2C_CR1_RXIE_Msk                          /*!< RX interrupt enable                 */
#define I2C_CR1_ADDRIE_Pos           (3U)
#define I2C_CR1_ADDRIE_Msk           (0x1UL << I2C_CR1_ADDRIE_Pos)             /*!< 0x00000008 */
#define I2C_CR1_ADDRIE               I2C_CR1_ADDRIE_Msk                        /*!< Address match interrupt enable      */
#define I2C_CR1_NACKIE_Pos           (4U)
#define I2C_CR1_NACKIE_Msk           (0x1UL << I2C_CR1_NACKIE_Pos)             /*!< 0x00000010 */
#define I2C_CR1_NACKIE               I2C_CR1_NACKIE_Msk                        /*!< NACK received interrupt enable      */
#define I2C_CR1_STOPIE_Pos           (5U)
#define I2C_CR1_STOPIE_Msk           (0x1UL << I2C_CR1_STOPIE_Pos)             /*!< 0x00000020 */
#define I2C_CR1_STOPIE               I2C_CR1_STOPIE_Msk                        /*!< STOP detection interrupt enable     */
#define I2C_CR1_TCIE_Pos             (6U)
#define I2C_CR1_TCIE_Msk             (0x1UL << I2C_CR1_TCIE_Pos)               /*!< 0x00000040 */
#define I2C_CR1_TCIE                 I2C_CR1_TCIE_Msk                          /*!< Transfer complete interrupt enable  */
#define I2C_CR1_ERRIE_Pos            (7U)
#define I2C_CR1_ERRIE_Msk            (0x1UL << I2C_CR1_ERRIE_Pos)              /*!< 0x00000080 */
#define I2C_CR1_ERRIE                I2C_CR1_ERRIE_Msk                         /*!< Errors interrupt enable             */
#define I2C_CR1_DNF_Pos              (8U)
#define I2C_CR1_DNF_Msk              (0xFUL << I2C_CR1_DNF_Pos)                /*!< 0x00000F00 */
#define I2C_CR1_DNF                  I2C_CR1_DNF_Msk                           /*!< Digital noise filter                */
#define I2C_CR1_ANFOFF_Pos           (12U)
#define I2C_CR1_ANFOFF_Msk           (0x1UL << I2C_CR1_ANFOFF_Pos)             /*!< 0x00001000 */
#define I2C_CR1_ANFOFF               I2C_CR1_ANFOFF_Msk                        /*!< Analog noise filter OFF             */
#define I2C_CR1_SWRST_Pos            (13U)
#define I2C_CR1_SWRST_Msk            (0x1UL << I2C_CR1_SWRST_Pos)              /*!< 0x00002000 */
#define I2C_CR1_SWRST                I2C_CR1_SWRST_Msk                         /*!< Software reset                      */
#define I2C_CR1_TXDMAEN_Pos          (14U)
#define I2C_CR1_TXDMAEN_Msk          (0x1UL << I2C_CR1_TXDMAEN_Pos)            /*!< 0x00004000 */
#define I2C_CR1_TXDMAEN              I2C_CR1_TXDMAEN_Msk                       /*!< DMA transmission requests enable    */
#define I2C_CR1_RXDMAEN_Pos          (15U)
#define I2C_CR1_RXDMAEN_Msk          (0x1UL << I2C_CR1_RXDMAEN_Pos)            /*!< 0x00008000 */
#define I2C_CR1_RXDMAEN              I2C_CR1_RXDMAEN_Msk                       /*!< DMA reception requests enable       */
#define I2C_CR1_SBC_Pos              (16U)
#define I2C_CR1_SBC_Msk              (0x1UL << I2C_CR1_SBC_Pos)                /*!< 0x00010000 */
#define I2C_CR1_SBC                  I2C_CR1_SBC_Msk                           /*!< Slave byte control                  */
#define I2C_CR1_NOSTRETCH_Pos        (17U)
#define I2C_CR1_NOSTRETCH_Msk        (0x1UL << I2C_CR1_NOSTRETCH_Pos)          /*!< 0x00020000 */
#define I2C_CR1_NOSTRETCH            I2C_CR1_NOSTRETCH_Msk                     /*!< Clock stretching disable            */
#define I2C_CR1_WUPEN_Pos            (18U)
#define I2C_CR1_WUPEN_Msk            (0x1UL << I2C_CR1_WUPEN_Pos)              /*!< 0x00040000 */
#define I2C_CR1_WUPEN                I2C_CR1_WUPEN_Msk                         /*!< Wakeup from STOP enable             */
#define I2C_CR1_GCEN_Pos             (19U)
#define I2C_CR1_GCEN_Msk             (0x1UL << I2C_CR1_GCEN_Pos)               /*!< 0x00080000 */
#define I2C_CR1_GCEN                 I2C_CR1_GCEN_Msk                          /*!< General call enable                 */
#define I2C_CR1_SMBHEN_Pos           (20U)
#define I2C_CR1_SMBHEN_Msk           (0x1UL << I2C_CR1_SMBHEN_Pos)             /*!< 0x00100000 */
#define I2C_CR1_SMBHEN               I2C_CR1_SMBHEN_Msk                        /*!< SMBus host address enable           */
#define I2C_CR1_SMBDEN_Pos           (21U)
#define I2C_CR1_SMBDEN_Msk           (0x1UL << I2C_CR1_SMBDEN_Pos)             /*!< 0x00200000 */
#define I2C_CR1_SMBDEN               I2C_CR1_SMBDEN_Msk                        /*!< SMBus device default address enable */
#define I2C_CR1_ALERTEN_Pos          (22U)
#define I2C_CR1_ALERTEN_Msk          (0x1UL << I2C_CR1_ALERTEN_Pos)            /*!< 0x00400000 */
#define I2C_CR1_ALERTEN              I2C_CR1_ALERTEN_Msk                       /*!< SMBus alert enable                  */
#define I2C_CR1_PECEN_Pos            (23U)
#define I2C_CR1_PECEN_Msk            (0x1UL << I2C_CR1_PECEN_Pos)              /*!< 0x00800000 */
#define I2C_CR1_PECEN                I2C_CR1_PECEN_Msk                         /*!< PEC enable                          */

/******************  Bit definition for I2C_CR2 register  ********************/
#define I2C_CR2_SADD_Pos             (0U)
#define I2C_CR2_SADD_Msk             (0x3FFUL << I2C_CR2_SADD_Pos)             /*!< 0x000003FF */
#define I2C_CR2_SADD                 I2C_CR2_SADD_Msk                          /*!< Slave address (master mode)                             */
#define I2C_CR2_RD_WRN_Pos           (10U)
#define I2C_CR2_RD_WRN_Msk           (0x1UL << I2C_CR2_RD_WRN_Pos)             /*!< 0x00000400 */
#define I2C_CR2_RD_WRN               I2C_CR2_RD_WRN_Msk                        /*!< Transfer direction (master mode)                        */
#define I2C_CR2_ADD10_Pos            (11U)
#define I2C_CR2_ADD10_Msk            (0x1UL << I2C_CR2_ADD10_Pos)              /*!< 0x00000800 */
#define I2C_CR2_ADD10                I2C_CR2_ADD10_Msk                         /*!< 10-bit addressing mode (master mode)                    */
#define I2C_CR2_HEAD10R_Pos          (12U)
#define I2C_CR2_HEAD10R_Msk          (0x1UL << I2C_CR2_HEAD10R_Pos)            /*!< 0x00001000 */
#define I2C_CR2_HEAD10R              I2C_CR2_HEAD10R_Msk                       /*!< 10-bit address header only read direction (master mode) */
#define I2C_CR2_START_Pos            (13U)
#define I2C_CR2_START_Msk            (0x1UL << I2C_CR2_START_Pos)              /*!< 0x00002000 */
#define I2C_CR2_START                I2C_CR2_START_Msk                         /*!< START generation                                        */
#define I2C_CR2_STOP_Pos             (14U)
#define I2C_CR2_STOP_Msk             (0x1UL << I2C_CR2_STOP_Pos)               /*!< 0x00004000 */
#define I2C_CR2_STOP                 I2C_CR2_STOP_Msk                          /*!< STOP generation (master mode)                           */
#define I2C_CR2_NACK_Pos             (15U)
#define I2C_CR2_NACK_Msk             (0x1UL << I2C_CR2_NACK_Pos)               /*!< 0x00008000 */
#define I2C_CR2_NACK                 I2C_CR2_NACK_Msk                          /*!< NACK generation (slave mode)                            */
#define I2C_CR2_NBYTES_Pos           (16U)
#define I2C_CR2_NBYTES_Msk           (0xFFUL << I2C_CR2_NBYTES_Pos)            /*!< 0x00FF0000 */
#define I2C_CR2_NBYTES               I2C_CR2_NBYTES_Msk                        /*!< Number of bytes                                         */
#define I2C_CR2_RELOAD_Pos           (24U)
#define I2C_CR2_RELOAD_Msk           (0x1UL << I2C_CR2_RELOAD_Pos)             /*!< 0x01000000 */
#define I2C_CR2_RELOAD               I2C_CR2_RELOAD_Msk                        /*!< NBYTES reload mode                                      */
#define I2C_CR2_AUTOEND_Pos          (25U)
#define I2C_CR2_AUTOEND_Msk          (0x1UL << I2C_CR2_AUTOEND_Pos)            /*!< 0x02000000 */
#define I2C_CR2_AUTOEND              I2C_CR2_AUTOEND_Msk                       /*!< Automatic end mode (master mode)                        */
#define I2C_CR2_PECBYTE_Pos          (26U)
#define I2C_CR2_PECBYTE_Msk          (0x1UL << I2C_CR2_PECBYTE_Pos)            /*!< 0x04000000 */
#define I2C_CR2_PECBYTE              I2C_CR2_PECBYTE_Msk                       /*!< Packet error checking byte                              */

/*******************  Bit definition for I2C_OAR1 register  ******************/
#define I2C_OAR1_OA1_Pos             (0U)
#define I2C_OAR1_OA1_Msk             (0x3FFUL << I2C_OAR1_OA1_Pos)             /*!< 0x000003FF */
#define I2C_OAR1_OA1                 I2C_OAR1_OA1_Msk                          /*!< Interface own address 1   */
#define I2C_OAR1_OA1MODE_Pos         (10U)
#define I2C_OAR1_OA1MODE_Msk         (0x1UL << I2C_OAR1_OA1MODE_Pos)           /*!< 0x00000400 */
#define I2C_OAR1_OA1MODE             I2C_OAR1_OA1MODE_Msk                      /*!< Own address 1 10-bit mode */
#define I2C_OAR1_OA1EN_Pos           (15U)
#define I2C_OAR1_OA1EN_Msk           (0x1UL << I2C_OAR1_OA1EN_Pos)             /*!< 0x00008000 */
#define I2C_OAR1_OA1EN               I2C_OAR1_OA1EN_Msk                        /*!< Own address 1 enable      */

/*******************  Bit definition for I2C_OAR2 register  ******************/
#define I2C_OAR2_OA2_Pos             (1U)
#define I2C_OAR2_OA2_Msk             (0x7FUL << I2C_OAR2_OA2_Pos)              /*!< 0x000000FE */
#define I2C_OAR2_OA2                 I2C_OAR2_OA2_Msk                          /*!< Interface own address 2                        */
#define I2C_OAR2_OA2MSK_Pos          (8U)
#define I2C_OAR2_OA2MSK_Msk          (0x7UL << I2C_OAR2_OA2MSK_Pos)            /*!< 0x00000700 */
#define I2C_OAR2_OA2MSK              I2C_OAR2_OA2MSK_Msk                       /*!< Own address 2 masks                            */
#define I2C_OAR2_OA2NOMASK           (0x00000000UL)                            /*!< No mask                                        */
#define I2C_OAR2_OA2MASK01_Pos       (8U)
#define I2C_OAR2_OA2MASK01_Msk       (0x1UL << I2C_OAR2_OA2MASK01_Pos)         /*!< 0x00000100 */
#define I2C_OAR2_OA2MASK01           I2C_OAR2_OA2MASK01_Msk                    /*!< OA2[1] is masked, Only OA2[7:2] are compared   */
#define I2C_OAR2_OA2MASK02_Pos       (9U)
#define I2C_OAR2_OA2MASK02_Msk       (0x1UL << I2C_OAR2_OA2MASK02_Pos)         /*!< 0x00000200 */
#define I2C_OAR2_OA2MASK02           I2C_OAR2_OA2MASK02_Msk                    /*!< OA2[2:1] is masked, Only OA2[7:3] are compared */
#define I2C_OAR2_OA2MASK03_Pos       (8U)
#define I2C_OAR2_OA2MASK03_Msk       (0x3UL << I2C_OAR2_OA2MASK03_Pos)         /*!< 0x00000300 */
#define I2C_OAR2_OA2MASK03           I2C_OAR2_OA2MASK03_Msk                    /*!< OA2[3:1] is masked, Only OA2[7:4] are compared */
#define I2C_OAR2_OA2MASK04_Pos       (10U)
#define I2C_OAR2_OA2MASK04_Msk       (0x1UL << I2C_OAR2_OA2MASK04_Pos)         /*!< 0x00000400 */
#define I2C_OAR2_OA2MASK04           I2C_OAR2_OA2MASK04_Msk                    /*!< OA2[4:1] is masked, Only OA2[7:5] are compared */
#define I2C_OAR2_OA2MASK05_Pos       (8U)
#define I2C_OAR2_OA2MASK05_Msk       (0x5UL << I2C_OAR2_OA2MASK05_Pos)         /*!< 0x00000500 */
#define I2C_OAR2_OA2MASK05           I2C_OAR2_OA2MASK05_Msk                    /*!< OA2[5:1] is masked, Only OA2[7:6] are compared */
#define I2C_OAR2_OA2MASK06_Pos       (9U)
#define I2C_OAR2_OA2MASK06_Msk       (0x3UL << I2C_OAR2_OA2MASK06_Pos)         /*!< 0x00000600 */
#define I2C_OAR2_OA2MASK06           I2C_OAR2_OA2MASK06_Msk                    /*!< OA2[6:1] is masked, Only OA2[7] are compared   */
#define I2C_OAR2_OA2MASK07_Pos       (8U)
#define I2C_OAR2_OA2MASK07_Msk       (0x7UL << I2C_OAR2_OA2MASK07_Pos)         /*!< 0x00000700 */
#define I2C_OAR2_OA2MASK07           I2C_OAR2_OA2MASK07_Msk                    /*!< OA2[7:1] is masked, No comparison is done      */
#define I2C_OAR2_OA2EN_Pos           (15U)
#define I2C_OAR2_OA2EN_Msk           (0x1UL << I2C_OAR2_OA2EN_Pos)             /*!< 0x00008000 */
#define I2C_OAR2_OA2EN               I2C_OAR2_OA2EN_Msk                        /*!< Own address 2 enable                           */

/*******************  Bit definition for I2C_TIMINGR register *******************/
#define I2C_TIMINGR_SCLL_Pos         (0U)
#define I2C_TIMINGR_SCLL_Msk         (0xFFUL << I2C_TIMINGR_SCLL_Pos)          /*!< 0x000000FF */
#define I2C_TIMINGR_SCLL             I2C_TIMINGR_SCLL_Msk                      /*!< SCL low period (master mode)  */
#define I2C_TIMINGR_SCLH_Pos         (8U)
#define I2C_TIMINGR_SCLH_Msk         (0xFFUL << I2C_TIMINGR_SCLH_Pos)          /*!< 0x0000FF00 */
#define I2C_TIMINGR_SCLH             I2C_TIMINGR_SCLH_Msk                      /*!< SCL high period (master mode) */
#define I2C_TIMINGR_SDADEL_Pos       (16U)
#define I2C_TIMINGR_SDADEL_Msk       (0xFUL << I2C_TIMINGR_SDADEL_Pos)         /*!< 0x000F0000 */
#define I2C_TIMINGR_SDADEL           I2C_TIMINGR_SDADEL_Msk                    /*!< Data hold time                */
#define I2C_TIMINGR_SCLDEL_Pos       (20U)
#define I2C_TIMINGR_SCLDEL_Msk       (0xFUL << I2C_TIMINGR_SCLDEL_Pos)         /*!< 0x00F00000 */
#define I2C_TIMINGR_SCLDEL           I2C_TIMINGR_SCLDEL_Msk                    /*!< Data setup time               */
#define I2C_TIMINGR_PRESC_Pos        (28U)
#define I2C_TIMINGR_PRESC_Msk        (0xFUL << I2C_TIMINGR_PRESC_Pos)          /*!< 0xF0000000 */
#define I2C_TIMINGR_PRESC            I2C_TIMINGR_PRESC_Msk                     /*!< Timings prescaler             */

/******************* Bit definition for I2C_TIMEOUTR register *******************/
#define I2C_TIMEOUTR_TIMEOUTA_Pos    (0U)
#define I2C_TIMEOUTR_TIMEOUTA_Msk    (0xFFFUL << I2C_TIMEOUTR_TIMEOUTA_Pos)    /*!< 0x00000FFF */
#define I2C_TIMEOUTR_TIMEOUTA        I2C_TIMEOUTR_TIMEOUTA_Msk                 /*!< Bus timeout A                 */
#define I2C_TIMEOUTR_TIDLE_Pos       (12U)
#define I2C_TIMEOUTR_TIDLE_Msk       (0x1UL << I2C_TIMEOUTR_TIDLE_Pos)         /*!< 0x00001000 */
#define I2C_TIMEOUTR_TIDLE           I2C_TIMEOUTR_TIDLE_Msk                    /*!< Idle clock timeout detection  */
#define I2C_TIMEOUTR_TIMOUTEN_Pos    (15U)
#define I2C_TIMEOUTR_TIMOUTEN_Msk    (0x1UL << I2C_TIMEOUTR_TIMOUTEN_Pos)      /*!< 0x00008000 */
#define I2C_TIMEOUTR_TIMOUTEN        I2C_TIMEOUTR_TIMOUTEN_Msk                 /*!< Clock timeout enable          */
#define I2C_TIMEOUTR_TIMEOUTB_Pos    (16U)
#define I2C_TIMEOUTR_TIMEOUTB_Msk    (0xFFFUL << I2C_TIMEOUTR_TIMEOUTB_Pos)    /*!< 0x0FFF0000 */
#define I2C_TIMEOUTR_TIMEOUTB        I2C_TIMEOUTR_TIMEOUTB_Msk                 /*!< Bus timeout B                 */
#define I2C_TIMEOUTR_TEXTEN_Pos      (31U)
#define I2C_TIMEOUTR_TEXTEN_Msk      (0x1UL << I2C_TIMEOUTR_TEXTEN_Pos)        /*!< 0x80000000 */
#define I2C_TIMEOUTR_TEXTEN          I2C_TIMEOUTR_TEXTEN_Msk                   /*!< Extended clock timeout enable */

/******************  Bit definition for I2C_ISR register  *********************/
#define I2C_ISR_TXE_Pos              (0U)
#define I2C_ISR_TXE_Msk              (0x1UL << I2C_ISR_TXE_Pos)                /*!< 0x00000001 */
#define I2C_ISR_TXE                  I2C_ISR_TXE_Msk                           /*!< Transmit data register empty    */
#define I2C_ISR_TXIS_Pos             (1U)
#define I2C_ISR_TXIS_Msk             (0x1UL << I2C_ISR_TXIS_Pos)               /*!< 0x00000002 */
#define I2C_ISR_TXIS                 I2C_ISR_TXIS_Msk                          /*!< Transmit interrupt status       */
#define I2C_ISR_RXNE_Pos             (2U)
#define I2C_ISR_RXNE_Msk             (0x1UL << I2C_ISR_RXNE_Pos)               /*!< 0x00000004 */
#define I2C_ISR_RXNE                 I2C_ISR_RXNE_Msk                          /*!< Receive data register not empty */
#define I2C_ISR_ADDR_Pos             (3U)
#define I2C_ISR_ADDR_Msk             (0x1UL << I2C_ISR_ADDR_Pos)               /*!< 0x00000008 */
#define I2C_ISR_ADDR                 I2C_ISR_ADDR_Msk                          /*!< Address matched (slave mode)    */
#define I2C_ISR_NACKF_Pos            (4U)
#define I2C_ISR_NACKF_Msk            (0x1UL << I2C_ISR_NACKF_Pos)              /*!< 0x00000010 */
#define I2C_ISR_NACKF                I2C_ISR_NACKF_Msk                         /*!< NACK received flag              */
#define I2C_ISR_STOPF_Pos            (5U)
#define I2C_ISR_STOPF_Msk            (0x1UL << I2C_ISR_STOPF_Pos)              /*!< 0x00000020 */
#define I2C_ISR_STOPF                I2C_ISR_STOPF_Msk                         /*!< STOP detection flag             */
#define I2C_ISR_TC_Pos               (6U)
#define I2C_ISR_TC_Msk               (0x1UL << I2C_ISR_TC_Pos)                 /*!< 0x00000040 */
#define I2C_ISR_TC                   I2C_ISR_TC_Msk                            /*!< Transfer complete (master mode) */
#define I2C_ISR_TCR_Pos              (7U)
#define I2C_ISR_TCR_Msk              (0x1UL << I2C_ISR_TCR_Pos)                /*!< 0x00000080 */
#define I2C_ISR_TCR                  I2C_ISR_TCR_Msk                           /*!< Transfer complete reload        */
#define I2C_ISR_BERR_Pos             (8U)
#define I2C_ISR_BERR_Msk             (0x1UL << I2C_ISR_BERR_Pos)               /*!< 0x00000100 */
#define I2C_ISR_BERR                 I2C_ISR_BERR_Msk                          /*!< Bus error                       */
#define I2C_ISR_ARLO_Pos             (9U)
#define I2C_ISR_ARLO_Msk             (0x1UL << I2C_ISR_ARLO_Pos)               /*!< 0x00000200 */
#define I2C_ISR_ARLO                 I2C_ISR_ARLO_Msk                          /*!< Arbitration lost                */
#define I2C_ISR_OVR_Pos              (10U)
#define I2C_ISR_OVR_Msk              (0x1UL << I2C_ISR_OVR_Pos)                /*!< 0x00000400 */
#define I2C_ISR_OVR                  I2C_ISR_OVR_Msk                           /*!< Overrun/Underrun                */
#define I2C_ISR_PECERR_Pos           (11U)
#define I2C_ISR_PECERR_Msk           (0x1UL << I2C_ISR_PECERR_Pos)             /*!< 0x00000800 */
#define I2C_ISR_PECERR               I2C_ISR_PECERR_Msk                        /*!< PEC error in reception          */
#define I2C_ISR_TIMEOUT_Pos          (12U)
#define I2C_ISR_TIMEOUT_Msk          (0x1UL << I2C_ISR_TIMEOUT_Pos)            /*!< 0x00001000 */
#define I2C_ISR_TIMEOUT              I2C_ISR_TIMEOUT_Msk                       /*!< Timeout or Tlow detection flag  */
#define I2C_ISR_ALERT_Pos            (13U)
#define I2C_ISR_ALERT_Msk            (0x1UL << I2C_ISR_ALERT_Pos)              /*!< 0x00002000 */
#define I2C_ISR_ALERT                I2C_ISR_ALERT_Msk                         /*!< SMBus alert                     */
#define I2C_ISR_BUSY_Pos             (15U)
#define I2C_ISR_BUSY_Msk             (0x1UL << I2C_ISR_BUSY_Pos)               /*!< 0x00008000 */
#define I2C_ISR_BUSY                 I2C_ISR_BUSY_Msk                          /*!< Bus busy                        */
#define I2C_ISR_DIR_Pos              (16U)
#define I2C_ISR_DIR_Msk              (0x1UL << I2C_ISR_DIR_Pos)                /*!< 0x00010000 */
#define I2C_ISR_DIR                  I2C_ISR_DIR_Msk                           /*!< Transfer direction (slave mode) */
#define I2C_ISR_ADDCODE_Pos          (17U)
#define I2C_ISR_ADDCODE_Msk          (0x7FUL << I2C_ISR_ADDCODE_Pos)           /*!< 0x00FE0000 */
#define I2C_ISR_ADDCODE              I2C_ISR_ADDCODE_Msk                       /*!< Address match code (slave mode) */

/******************  Bit definition for I2C_ICR register  *********************/
#define I2C_ICR_ADDRCF_Pos           (3U)
#define I2C_ICR_ADDRCF_Msk           (0x1UL << I2C_ICR_ADDRCF_Pos)             /*!< 0x00000008 */
#define I2C_ICR_ADDRCF               I2C_ICR_ADDRCF_Msk                        /*!< Address matched clear flag  */
#define I2C_ICR_NACKCF_Pos           (4U)
#define I2C_ICR_NACKCF_Msk           (0x1UL << I2C_ICR_NACKCF_Pos)             /*!< 0x00000010 */
#define I2C_ICR_NACKCF               I2C_ICR_NACKCF_Msk                        /*!< NACK clear flag             */
#define I2C_ICR_STOPCF_Pos           (5U)
#define I2C_ICR_STOPCF_Msk           (0x1UL << I2C_ICR_STOPCF_Pos)             /*!< 0x00000020 */
#define I2C_ICR_STOPCF               I2C_ICR_STOPCF_Msk                        /*!< STOP detection clear flag   */
#define I2C_ICR_BERRCF_Pos           (8U)
#define I2C_ICR_BERRCF_Msk           (0x1UL << I2C_ICR_BERRCF_Pos)             /*!< 0x00000100 */
#define I2C_ICR_BERRCF               I2C_ICR_BERRCF_Msk                        /*!< Bus error clear flag        */
#define I2C_ICR_ARLOCF_Pos           (9U)
#define I2C_ICR_ARLOCF_Msk           (0x1UL << I2C_ICR_ARLOCF_Pos)             /*!< 0x00000200 */
#define I2C_ICR_ARLOCF               I2C_ICR_ARLOCF_Msk                        /*!< Arbitration lost clear flag */
#define I2C_ICR_OVRCF_Pos            (10U)
#define I2C_ICR_OVRCF_Msk            (0x1UL << I2C_ICR_OVRCF_Pos)              /*!< 0x00000400 */
#define I2C_ICR_OVRCF                I2C_ICR_OVRCF_Msk                         /*!< Overrun/Underrun clear flag */
#define I2C_ICR_PECCF_Pos            (11U)
#define I2C_ICR_PECCF_Msk            (0x1UL << I2C_ICR_PECCF_Pos)              /*!< 0x00000800 */
#define I2C_ICR_PECCF                I2C_ICR_PECCF_Msk                         /*!< PAC error clear flag        */
#define I2C_ICR_TIMOUTCF_Pos         (12U)
#define I2C_ICR_TIMOUTCF_Msk         (0x1UL << I2C_ICR_TIMOUTCF_Pos)           /*!< 0x00001000 */
#define I2C_ICR_TIMOUTCF             I2C_ICR_TIMOUTCF_Msk                      /*!< Timeout clear flag          */
#define I2C_ICR_ALERTCF_Pos          (13U)
#define I2C_ICR_ALERTCF_Msk          (0x1UL << I2C_ICR_ALERTCF_Pos)            /*!< 0x00002000 */
#define I2C_ICR_ALERTCF              I2C_ICR_ALERTCF_Msk                       /*!< Alert clear flag            */

/******************  Bit definition for I2C_PECR register  *********************/
#define I2C_PECR_PEC_Pos             (0U)
#define I2C_PECR_PEC_Msk             (0xFFUL << I2C_PECR_PEC_Pos)              /*!< 0x000000FF */
#define I2C_PECR_PEC                 I2C_PECR_PEC_Msk                          /*!< PEC register */

/******************  Bit definition for I2C_RXDR register  *********************/
#define I2C_RXDR_RXDATA_Pos          (0U)
#define I2C_RXDR_RXDATA_Msk          (0xFFUL << I2C_RXDR_RXDATA_Pos)           /*!< 0x000000FF */
#define I2C_RXDR_RXDATA              I2C_RXDR_RXDATA_Msk                       /*!< 8-bit receive data */

/******************  Bit definition for I2C_TXDR register  *********************/
#define I2C_TXDR_TXDATA_Pos          (0U)
#define I2C_TXDR_TXDATA_Msk          (0xFFUL << I2C_TXDR_TXDATA_Pos)           /*!< 0x000000FF */
#define I2C_TXDR_TXDATA              I2C_TXDR_TXDATA_Msk                       /*!< 8-bit transmit data */

/******************************************************************************/
/*                                                                            */
/*                           Independent WATCHDOG                             */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for IWDG_KR register  ********************/
#define IWDG_KR_KEY_Pos      (0U)
#define IWDG_KR_KEY_Msk      (0xFFFFUL << IWDG_KR_KEY_Pos)                     /*!< 0x0000FFFF */
#define IWDG_KR_KEY          IWDG_KR_KEY_Msk                                   /*!<Key value (write only, read 0000h)  */

/*******************  Bit definition for IWDG_PR register  ********************/
#define IWDG_PR_PR_Pos       (0U)
#define IWDG_PR_PR_Msk       (0x7UL << IWDG_PR_PR_Pos)                         /*!< 0x00000007 */
#define IWDG_PR_PR           IWDG_PR_PR_Msk                                    /*!<PR[2:0] (Prescaler divider)         */
#define IWDG_PR_PR_0         (0x1UL << IWDG_PR_PR_Pos)                         /*!< 0x00000001 */
#define IWDG_PR_PR_1         (0x2UL << IWDG_PR_PR_Pos)                         /*!< 0x00000002 */
#define IWDG_PR_PR_2         (0x4UL << IWDG_PR_PR_Pos)                         /*!< 0x00000004 */

/*******************  Bit definition for IWDG_RLR register  *******************/
#define IWDG_RLR_RL_Pos      (0U)
#define IWDG_RLR_RL_Msk      (0xFFFUL << IWDG_RLR_RL_Pos)                      /*!< 0x00000FFF */
#define IWDG_RLR_RL          IWDG_RLR_RL_Msk                                   /*!<Watchdog counter reload value        */

/*******************  Bit definition for IWDG_SR register  ********************/
#define IWDG_SR_PVU_Pos      (0U)
#define IWDG_SR_PVU_Msk      (0x1UL << IWDG_SR_PVU_Pos)                        /*!< 0x00000001 */
#define IWDG_SR_PVU          IWDG_SR_PVU_Msk                                   /*!< Watchdog prescaler value update */
#define IWDG_SR_RVU_Pos      (1U)
#define IWDG_SR_RVU_Msk      (0x1UL << IWDG_SR_RVU_Pos)                        /*!< 0x00000002 */
#define IWDG_SR_RVU          IWDG_SR_RVU_Msk                                   /*!< Watchdog counter reload value update */
#define IWDG_SR_WVU_Pos      (2U)
#define IWDG_SR_WVU_Msk      (0x1UL << IWDG_SR_WVU_Pos)                        /*!< 0x00000004 */
#define IWDG_SR_WVU          IWDG_SR_WVU_Msk                                   /*!< Watchdog counter window value update */

/*******************  Bit definition for IWDG_KR register  ********************/
#define IWDG_WINR_WIN_Pos    (0U)
#define IWDG_WINR_WIN_Msk    (0xFFFUL << IWDG_WINR_WIN_Pos)                    /*!< 0x00000FFF */
#define IWDG_WINR_WIN        IWDG_WINR_WIN_Msk                                 /*!< Watchdog counter window value */

/******************************************************************************/
/*                                                                            */
/*                                     Firewall                               */
/*                                                                            */
/******************************************************************************/

/*******Bit definition for CSSA;CSL;NVDSSA;NVDSL;VDSSA;VDSL register          */
#define FW_CSSA_ADD_Pos      (8U)
#define FW_CSSA_ADD_Msk      (0xFFFFUL << FW_CSSA_ADD_Pos)                     /*!< 0x00FFFF00 */
#define FW_CSSA_ADD          FW_CSSA_ADD_Msk                                   /*!< Code Segment Start Address */
#define FW_CSL_LENG_Pos      (8U)
#define FW_CSL_LENG_Msk      (0x3FFFUL << FW_CSL_LENG_Pos)                     /*!< 0x003FFF00 */
#define FW_CSL_LENG          FW_CSL_LENG_Msk                                   /*!< Code Segment Length        */
#define FW_NVDSSA_ADD_Pos    (8U)
#define FW_NVDSSA_ADD_Msk    (0xFFFFUL << FW_NVDSSA_ADD_Pos)                   /*!< 0x00FFFF00 */
#define FW_NVDSSA_ADD        FW_NVDSSA_ADD_Msk                                 /*!< Non Volatile Dat Segment Start Address */
#define FW_NVDSL_LENG_Pos    (8U)
#define FW_NVDSL_LENG_Msk    (0x3FFFUL << FW_NVDSL_LENG_Pos)                   /*!< 0x003FFF00 */
#define FW_NVDSL_LENG        FW_NVDSL_LENG_Msk                                 /*!< Non Volatile Data Segment Length */
#define FW_VDSSA_ADD_Pos     (6U)
#define FW_VDSSA_ADD_Msk     (0x7FFUL << FW_VDSSA_ADD_Pos)                     /*!< 0x0001FFC0 */
#define FW_VDSSA_ADD         FW_VDSSA_ADD_Msk                                  /*!< Volatile Data Segment Start Address */
#define FW_VDSL_LENG_Pos     (6U)
#define FW_VDSL_LENG_Msk     (0x7FFUL << FW_VDSL_LENG_Pos)                     /*!< 0x0001FFC0 */
#define FW_VDSL_LENG         FW_VDSL_LENG_Msk                                  /*!< Volatile Data Segment Length */

/**************************Bit definition for CR register *********************/
#define FW_CR_FPA_Pos        (0U)
#define FW_CR_FPA_Msk        (0x1UL << FW_CR_FPA_Pos)                          /*!< 0x00000001 */
#define FW_CR_FPA            FW_CR_FPA_Msk                                     /*!< Firewall Pre Arm*/
#define FW_CR_VDS_Pos        (1U)
#define FW_CR_VDS_Msk        (0x1UL << FW_CR_VDS_Pos)                          /*!< 0x00000002 */
#define FW_CR_VDS            FW_CR_VDS_Msk                                     /*!< Volatile Data Sharing*/
#define FW_CR_VDE_Pos        (2U)
#define FW_CR_VDE_Msk        (0x1UL << FW_CR_VDE_Pos)                          /*!< 0x00000004 */
#define FW_CR_VDE            FW_CR_VDE_Msk                                     /*!< Volatile Data Execution*/

/******************************************************************************/
/*                                                                            */
/*                             Power Control                                  */
/*                                                                            */
/******************************************************************************/

/********************  Bit definition for PWR_CR1 register  ********************/

#define PWR_CR1_LPR_Pos              (14U)
#define PWR_CR1_LPR_Msk              (0x1UL << PWR_CR1_LPR_Pos)                /*!< 0x00004000 */
#define PWR_CR1_LPR                  PWR_CR1_LPR_Msk                           /*!< Regulator low-power mode */
#define PWR_CR1_VOS_Pos              (9U)
#define PWR_CR1_VOS_Msk              (0x3UL << PWR_CR1_VOS_Pos)                /*!< 0x00000600 */
#define PWR_CR1_VOS                  PWR_CR1_VOS_Msk                           /*!< VOS[1:0] bits (Regulator voltage scaling output selection) */
#define PWR_CR1_VOS_0                (0x1UL << PWR_CR1_VOS_Pos)                /*!< 0x00000200 */
#define PWR_CR1_VOS_1                (0x2UL << PWR_CR1_VOS_Pos)                /*!< 0x00000400 */
#define PWR_CR1_DBP_Pos              (8U)
#define PWR_CR1_DBP_Msk              (0x1UL << PWR_CR1_DBP_Pos)                /*!< 0x00000100 */
#define PWR_CR1_DBP                  PWR_CR1_DBP_Msk                           /*!< Disable Back-up domain Protection */
#define PWR_CR1_LPMS_Pos             (0U)
#define PWR_CR1_LPMS_Msk             (0x7UL << PWR_CR1_LPMS_Pos)               /*!< 0x00000007 */
#define PWR_CR1_LPMS                 PWR_CR1_LPMS_Msk                          /*!< Low-power mode selection field */
#define PWR_CR1_LPMS_STOP0           (0x00000000UL)                            /*!< Stop 0 mode */
#define PWR_CR1_LPMS_STOP1_Pos       (0U)
#define PWR_CR1_LPMS_STOP1_Msk       (0x1UL << PWR_CR1_LPMS_STOP1_Pos)         /*!< 0x00000001 */
#define PWR_CR1_LPMS_STOP1           PWR_CR1_LPMS_STOP1_Msk                    /*!< Stop 1 mode */
#define PWR_CR1_LPMS_STOP2_Pos       (1U)
#define PWR_CR1_LPMS_STOP2_Msk       (0x1UL << PWR_CR1_LPMS_STOP2_Pos)         /*!< 0x00000002 */
#define PWR_CR1_LPMS_STOP2           PWR_CR1_LPMS_STOP2_Msk                    /*!< Stop 2 mode */
#define PWR_CR1_LPMS_STANDBY_Pos     (0U)
#define PWR_CR1_LPMS_STANDBY_Msk     (0x3UL << PWR_CR1_LPMS_STANDBY_Pos)       /*!< 0x00000003 */
#define PWR_CR1_LPMS_STANDBY         PWR_CR1_LPMS_STANDBY_Msk                  /*!< Stand-by mode */
#define PWR_CR1_LPMS_SHUTDOWN_Pos    (2U)
#define PWR_CR1_LPMS_SHUTDOWN_Msk    (0x1UL << PWR_CR1_LPMS_SHUTDOWN_Pos)      /*!< 0x00000004 */
#define PWR_CR1_LPMS_SHUTDOWN        PWR_CR1_LPMS_SHUTDOWN_Msk                 /*!< Shut-down mode */


/********************  Bit definition for PWR_CR2 register  ********************/
#define PWR_CR2_USV_Pos              (10U)
#define PWR_CR2_USV_Msk              (0x1UL << PWR_CR2_USV_Pos)                /*!< 0x00000400 */
#define PWR_CR2_USV                  PWR_CR2_USV_Msk                           /*!< VDD USB Supply Valid */
/*!< PVME  Peripheral Voltage Monitor Enable */
#define PWR_CR2_PVME_Pos             (4U)
#define PWR_CR2_PVME_Msk             (0xDUL << PWR_CR2_PVME_Pos)               /*!< 0x000000D0 */
#define PWR_CR2_PVME                 PWR_CR2_PVME_Msk                          /*!< PVM bits field */
#define PWR_CR2_PVME4_Pos            (7U)
#define PWR_CR2_PVME4_Msk            (0x1UL << PWR_CR2_PVME4_Pos)              /*!< 0x00000080 */
#define PWR_CR2_PVME4                PWR_CR2_PVME4_Msk                         /*!< PVM 4 Enable */
#define PWR_CR2_PVME3_Pos            (6U)
#define PWR_CR2_PVME3_Msk            (0x1UL << PWR_CR2_PVME3_Pos)              /*!< 0x00000040 */
#define PWR_CR2_PVME3                PWR_CR2_PVME3_Msk                         /*!< PVM 3 Enable */
#define PWR_CR2_PVME1_Pos            (4U)
#define PWR_CR2_PVME1_Msk            (0x1UL << PWR_CR2_PVME1_Pos)              /*!< 0x00000010 */
#define PWR_CR2_PVME1                PWR_CR2_PVME1_Msk                         /*!< PVM 1 Enable */
/*!< PVD level configuration */
#define PWR_CR2_PLS_Pos              (1U)
#define PWR_CR2_PLS_Msk              (0x7UL << PWR_CR2_PLS_Pos)                /*!< 0x0000000E */
#define PWR_CR2_PLS                  PWR_CR2_PLS_Msk                           /*!< PVD level selection */
#define PWR_CR2_PLS_LEV0             (0x00000000UL)                            /*!< PVD level 0 */
#define PWR_CR2_PLS_LEV1_Pos         (1U)
#define PWR_CR2_PLS_LEV1_Msk         (0x1UL << PWR_CR2_PLS_LEV1_Pos)           /*!< 0x00000002 */
#define PWR_CR2_PLS_LEV1             PWR_CR2_PLS_LEV1_Msk                      /*!< PVD level 1 */
#define PWR_CR2_PLS_LEV2_Pos         (2U)
#define PWR_CR2_PLS_LEV2_Msk         (0x1UL << PWR_CR2_PLS_LEV2_Pos)           /*!< 0x00000004 */
#define PWR_CR2_PLS_LEV2             PWR_CR2_PLS_LEV2_Msk                      /*!< PVD level 2 */
#define PWR_CR2_PLS_LEV3_Pos         (1U)
#define PWR_CR2_PLS_LEV3_Msk         (0x3UL << PWR_CR2_PLS_LEV3_Pos)           /*!< 0x00000006 */
#define PWR_CR2_PLS_LEV3             PWR_CR2_PLS_LEV3_Msk                      /*!< PVD level 3 */
#define PWR_CR2_PLS_LEV4_Pos         (3U)
#define PWR_CR2_PLS_LEV4_Msk         (0x1UL << PWR_CR2_PLS_LEV4_Pos)           /*!< 0x00000008 */
#define PWR_CR2_PLS_LEV4             PWR_CR2_PLS_LEV4_Msk                      /*!< PVD level 4 */
#define PWR_CR2_PLS_LEV5_Pos         (1U)
#define PWR_CR2_PLS_LEV5_Msk         (0x5UL << PWR_CR2_PLS_LEV5_Pos)           /*!< 0x0000000A */
#define PWR_CR2_PLS_LEV5             PWR_CR2_PLS_LEV5_Msk                      /*!< PVD level 5 */
#define PWR_CR2_PLS_LEV6_Pos         (2U)
#define PWR_CR2_PLS_LEV6_Msk         (0x3UL << PWR_CR2_PLS_LEV6_Pos)           /*!< 0x0000000C */
#define PWR_CR2_PLS_LEV6             PWR_CR2_PLS_LEV6_Msk                      /*!< PVD level 6 */
#define PWR_CR2_PLS_LEV7_Pos         (1U)
#define PWR_CR2_PLS_LEV7_Msk         (0x7UL << PWR_CR2_PLS_LEV7_Pos)           /*!< 0x0000000E */
#define PWR_CR2_PLS_LEV7             PWR_CR2_PLS_LEV7_Msk                      /*!< PVD level 7 */
#define PWR_CR2_PVDE_Pos             (0U)
#define PWR_CR2_PVDE_Msk             (0x1UL << PWR_CR2_PVDE_Pos)               /*!< 0x00000001 */
#define PWR_CR2_PVDE                 PWR_CR2_PVDE_Msk                          /*!< Power Voltage Detector Enable */

/********************  Bit definition for PWR_CR3 register  ********************/
#define PWR_CR3_EIWUL_Pos            (15U)
#define PWR_CR3_EIWUL_Msk            (0x1UL << PWR_CR3_EIWUL_Pos)              /*!< 0x00008000 */
#define PWR_CR3_EIWUL                PWR_CR3_EIWUL_Msk                         /*!< Enable Internal Wake-up line */
#define PWR_CR3_ENULP_Pos            (11U)
#define PWR_CR3_ENULP_Msk            (0x1UL << PWR_CR3_ENULP_Pos)              /*!< 0x00000800 */
#define PWR_CR3_ENULP                PWR_CR3_ENULP_Msk                         /*!< Enable ULP BORL, BORH and PVD for STOP2 and Standby modes */
#define PWR_CR3_APC_Pos              (10U)
#define PWR_CR3_APC_Msk              (0x1UL << PWR_CR3_APC_Pos)                /*!< 0x00000400 */
#define PWR_CR3_APC                  PWR_CR3_APC_Msk                           /*!< Apply pull-up and pull-down configuration */
#define PWR_CR3_RRS_Pos              (8U)
#define PWR_CR3_RRS_Msk              (0x1UL << PWR_CR3_RRS_Pos)                /*!< 0x00000100 */
#define PWR_CR3_RRS                  PWR_CR3_RRS_Msk                           /*!< SRAM2 Retention in Stand-by mode */
#define PWR_CR3_EWUP5_Pos            (4U)
#define PWR_CR3_EWUP5_Msk            (0x1UL << PWR_CR3_EWUP5_Pos)              /*!< 0x00000010 */
#define PWR_CR3_EWUP5                PWR_CR3_EWUP5_Msk                         /*!< Enable Wake-Up Pin 5 */
#define PWR_CR3_EWUP4_Pos            (3U)
#define PWR_CR3_EWUP4_Msk            (0x1UL << PWR_CR3_EWUP4_Pos)              /*!< 0x00000008 */
#define PWR_CR3_EWUP4                PWR_CR3_EWUP4_Msk                         /*!< Enable Wake-Up Pin 4 */
#define PWR_CR3_EWUP3_Pos            (2U)
#define PWR_CR3_EWUP3_Msk            (0x1UL << PWR_CR3_EWUP3_Pos)              /*!< 0x00000004 */
#define PWR_CR3_EWUP3                PWR_CR3_EWUP3_Msk                         /*!< Enable Wake-Up Pin 3 */
#define PWR_CR3_EWUP2_Pos            (1U)
#define PWR_CR3_EWUP2_Msk            (0x1UL << PWR_CR3_EWUP2_Pos)              /*!< 0x00000002 */
#define PWR_CR3_EWUP2                PWR_CR3_EWUP2_Msk                         /*!< Enable Wake-Up Pin 2 */
#define PWR_CR3_EWUP1_Pos            (0U)
#define PWR_CR3_EWUP1_Msk            (0x1UL << PWR_CR3_EWUP1_Pos)              /*!< 0x00000001 */
#define PWR_CR3_EWUP1                PWR_CR3_EWUP1_Msk                         /*!< Enable Wake-Up Pin 1 */
#define PWR_CR3_EWUP_Pos             (0U)
#define PWR_CR3_EWUP_Msk             (0x1FUL << PWR_CR3_EWUP_Pos)              /*!< 0x0000001F */
#define PWR_CR3_EWUP                 PWR_CR3_EWUP_Msk                          /*!< Enable Wake-Up Pins  */

/* Legacy defines */
#define PWR_CR3_EIWF_Pos             PWR_CR3_EIWUL_Pos
#define PWR_CR3_EIWF_Msk             PWR_CR3_EIWUL_Msk
#define PWR_CR3_EIWF                 PWR_CR3_EIWUL


/********************  Bit definition for PWR_CR4 register  ********************/
#define PWR_CR4_EXT_SMPS_ON_Pos      (13U)
#define PWR_CR4_EXT_SMPS_ON_Msk      (0x1UL << PWR_CR4_EXT_SMPS_ON_Pos)         /*!< 0x00002000 */
#define PWR_CR4_EXT_SMPS_ON          PWR_CR4_EXT_SMPS_ON_Msk                   /*!< Inform the internal regulator on external SMPS switch status */
#define PWR_CR4_VBRS_Pos             (9U)
#define PWR_CR4_VBRS_Msk             (0x1UL << PWR_CR4_VBRS_Pos)               /*!< 0x00000200 */
#define PWR_CR4_VBRS                 PWR_CR4_VBRS_Msk                          /*!< VBAT Battery charging Resistor Selection */
#define PWR_CR4_VBE_Pos              (8U)
#define PWR_CR4_VBE_Msk              (0x1UL << PWR_CR4_VBE_Pos)                /*!< 0x00000100 */
#define PWR_CR4_VBE                  PWR_CR4_VBE_Msk                           /*!< VBAT Battery charging Enable  */
#define PWR_CR4_WP5_Pos              (4U)
#define PWR_CR4_WP5_Msk              (0x1UL << PWR_CR4_WP5_Pos)                /*!< 0x00000010 */
#define PWR_CR4_WP5                  PWR_CR4_WP5_Msk                           /*!< Wake-Up Pin 5 polarity */
#define PWR_CR4_WP4_Pos              (3U)
#define PWR_CR4_WP4_Msk              (0x1UL << PWR_CR4_WP4_Pos)                /*!< 0x00000008 */
#define PWR_CR4_WP4                  PWR_CR4_WP4_Msk                           /*!< Wake-Up Pin 4 polarity */
#define PWR_CR4_WP3_Pos              (2U)
#define PWR_CR4_WP3_Msk              (0x1UL << PWR_CR4_WP3_Pos)                /*!< 0x00000004 */
#define PWR_CR4_WP3                  PWR_CR4_WP3_Msk                           /*!< Wake-Up Pin 3 polarity */
#define PWR_CR4_WP2_Pos              (1U)
#define PWR_CR4_WP2_Msk              (0x1UL << PWR_CR4_WP2_Pos)                /*!< 0x00000002 */
#define PWR_CR4_WP2                  PWR_CR4_WP2_Msk                           /*!< Wake-Up Pin 2 polarity */
#define PWR_CR4_WP1_Pos              (0U)
#define PWR_CR4_WP1_Msk              (0x1UL << PWR_CR4_WP1_Pos)                /*!< 0x00000001 */
#define PWR_CR4_WP1                  PWR_CR4_WP1_Msk                           /*!< Wake-Up Pin 1 polarity */

/********************  Bit definition for PWR_SR1 register  ********************/
#define PWR_SR1_WUFI_Pos             (15U)
#define PWR_SR1_WUFI_Msk             (0x1UL << PWR_SR1_WUFI_Pos)               /*!< 0x00008000 */
#define PWR_SR1_WUFI                 PWR_SR1_WUFI_Msk                          /*!< Wake-Up Flag Internal */
#define PWR_SR1_EXT_SMPS_RDY_Pos     (13U)
#define PWR_SR1_EXT_SMPS_RDY_Msk     (0x1UL << PWR_SR1_EXT_SMPS_RDY_Pos)        /*!< 0x00002000 */
#define PWR_SR1_EXT_SMPS_RDY         PWR_SR1_EXT_SMPS_RDY_Msk                  /*!< Switching to external SMPS Ready Flag */
#define PWR_SR1_SBF_Pos              (8U)
#define PWR_SR1_SBF_Msk              (0x1UL << PWR_SR1_SBF_Pos)                /*!< 0x00000100 */
#define PWR_SR1_SBF                  PWR_SR1_SBF_Msk                           /*!< Stand-By Flag */
#define PWR_SR1_WUF_Pos              (0U)
#define PWR_SR1_WUF_Msk              (0x1FUL << PWR_SR1_WUF_Pos)               /*!< 0x0000001F */
#define PWR_SR1_WUF                  PWR_SR1_WUF_Msk                           /*!< Wake-up Flags */
#define PWR_SR1_WUF5_Pos             (4U)
#define PWR_SR1_WUF5_Msk             (0x1UL << PWR_SR1_WUF5_Pos)               /*!< 0x00000010 */
#define PWR_SR1_WUF5                 PWR_SR1_WUF5_Msk                          /*!< Wake-up Flag 5 */
#define PWR_SR1_WUF4_Pos             (3U)
#define PWR_SR1_WUF4_Msk             (0x1UL << PWR_SR1_WUF4_Pos)               /*!< 0x00000008 */
#define PWR_SR1_WUF4                 PWR_SR1_WUF4_Msk                          /*!< Wake-up Flag 4 */
#define PWR_SR1_WUF3_Pos             (2U)
#define PWR_SR1_WUF3_Msk             (0x1UL << PWR_SR1_WUF3_Pos)               /*!< 0x00000004 */
#define PWR_SR1_WUF3                 PWR_SR1_WUF3_Msk                          /*!< Wake-up Flag 3 */
#define PWR_SR1_WUF2_Pos             (1U)
#define PWR_SR1_WUF2_Msk             (0x1UL << PWR_SR1_WUF2_Pos)               /*!< 0x00000002 */
#define PWR_SR1_WUF2                 PWR_SR1_WUF2_Msk                          /*!< Wake-up Flag 2 */
#define PWR_SR1_WUF1_Pos             (0U)
#define PWR_SR1_WUF1_Msk             (0x1UL << PWR_SR1_WUF1_Pos)               /*!< 0x00000001 */
#define PWR_SR1_WUF1                 PWR_SR1_WUF1_Msk                          /*!< Wake-up Flag 1 */

/********************  Bit definition for PWR_SR2 register  ********************/
#define PWR_SR2_PVMO4_Pos            (15U)
#define PWR_SR2_PVMO4_Msk            (0x1UL << PWR_SR2_PVMO4_Pos)              /*!< 0x00008000 */
#define PWR_SR2_PVMO4                PWR_SR2_PVMO4_Msk                         /*!< Peripheral Voltage Monitoring Output 4 */
#define PWR_SR2_PVMO3_Pos            (14U)
#define PWR_SR2_PVMO3_Msk            (0x1UL << PWR_SR2_PVMO3_Pos)              /*!< 0x00004000 */
#define PWR_SR2_PVMO3                PWR_SR2_PVMO3_Msk                         /*!< Peripheral Voltage Monitoring Output 3 */
#define PWR_SR2_PVMO1_Pos            (12U)
#define PWR_SR2_PVMO1_Msk            (0x1UL << PWR_SR2_PVMO1_Pos)              /*!< 0x00001000 */
#define PWR_SR2_PVMO1                PWR_SR2_PVMO1_Msk                         /*!< Peripheral Voltage Monitoring Output 1 */
#define PWR_SR2_PVDO_Pos             (11U)
#define PWR_SR2_PVDO_Msk             (0x1UL << PWR_SR2_PVDO_Pos)               /*!< 0x00000800 */
#define PWR_SR2_PVDO                 PWR_SR2_PVDO_Msk                          /*!< Power Voltage Detector Output */
#define PWR_SR2_VOSF_Pos             (10U)
#define PWR_SR2_VOSF_Msk             (0x1UL << PWR_SR2_VOSF_Pos)               /*!< 0x00000400 */
#define PWR_SR2_VOSF                 PWR_SR2_VOSF_Msk                          /*!< Voltage Scaling Flag */
#define PWR_SR2_REGLPF_Pos           (9U)
#define PWR_SR2_REGLPF_Msk           (0x1UL << PWR_SR2_REGLPF_Pos)             /*!< 0x00000200 */
#define PWR_SR2_REGLPF               PWR_SR2_REGLPF_Msk                        /*!< Low-power Regulator Flag */
#define PWR_SR2_REGLPS_Pos           (8U)
#define PWR_SR2_REGLPS_Msk           (0x1UL << PWR_SR2_REGLPS_Pos)             /*!< 0x00000100 */
#define PWR_SR2_REGLPS               PWR_SR2_REGLPS_Msk                        /*!< Low-power Regulator Started */

/********************  Bit definition for PWR_SCR register  ********************/
#define PWR_SCR_CSBF_Pos             (8U)
#define PWR_SCR_CSBF_Msk             (0x1UL << PWR_SCR_CSBF_Pos)               /*!< 0x00000100 */
#define PWR_SCR_CSBF                 PWR_SCR_CSBF_Msk                          /*!< Clear Stand-By Flag */
#define PWR_SCR_CWUF_Pos             (0U)
#define PWR_SCR_CWUF_Msk             (0x1FUL << PWR_SCR_CWUF_Pos)              /*!< 0x0000001F */
#define PWR_SCR_CWUF                 PWR_SCR_CWUF_Msk                          /*!< Clear Wake-up Flags  */
#define PWR_SCR_CWUF5_Pos            (4U)
#define PWR_SCR_CWUF5_Msk            (0x1UL << PWR_SCR_CWUF5_Pos)              /*!< 0x00000010 */
#define PWR_SCR_CWUF5                PWR_SCR_CWUF5_Msk                         /*!< Clear Wake-up Flag 5 */
#define PWR_SCR_CWUF4_Pos            (3U)
#define PWR_SCR_CWUF4_Msk            (0x1UL << PWR_SCR_CWUF4_Pos)              /*!< 0x00000008 */
#define PWR_SCR_CWUF4                PWR_SCR_CWUF4_Msk                         /*!< Clear Wake-up Flag 4 */
#define PWR_SCR_CWUF3_Pos            (2U)
#define PWR_SCR_CWUF3_Msk            (0x1UL << PWR_SCR_CWUF3_Pos)              /*!< 0x00000004 */
#define PWR_SCR_CWUF3                PWR_SCR_CWUF3_Msk                         /*!< Clear Wake-up Flag 3 */
#define PWR_SCR_CWUF2_Pos            (1U)
#define PWR_SCR_CWUF2_Msk            (0x1UL << PWR_SCR_CWUF2_Pos)              /*!< 0x00000002 */
#define PWR_SCR_CWUF2                PWR_SCR_CWUF2_Msk                         /*!< Clear Wake-up Flag 2 */
#define PWR_SCR_CWUF1_Pos            (0U)
#define PWR_SCR_CWUF1_Msk            (0x1UL << PWR_SCR_CWUF1_Pos)              /*!< 0x00000001 */
#define PWR_SCR_CWUF1                PWR_SCR_CWUF1_Msk                         /*!< Clear Wake-up Flag 1 */

/********************  Bit definition for PWR_PUCRA register  ********************/
#define PWR_PUCRA_PA15_Pos           (15U)
#define PWR_PUCRA_PA15_Msk           (0x1UL << PWR_PUCRA_PA15_Pos)             /*!< 0x00008000 */
#define PWR_PUCRA_PA15               PWR_PUCRA_PA15_Msk                        /*!< Port PA15 Pull-Up set */
#define PWR_PUCRA_PA13_Pos           (13U)
#define PWR_PUCRA_PA13_Msk           (0x1UL << PWR_PUCRA_PA13_Pos)             /*!< 0x00002000 */
#define PWR_PUCRA_PA13               PWR_PUCRA_PA13_Msk                        /*!< Port PA13 Pull-Up set */
#define PWR_PUCRA_PA12_Pos           (12U)
#define PWR_PUCRA_PA12_Msk           (0x1UL << PWR_PUCRA_PA12_Pos)             /*!< 0x00001000 */
#define PWR_PUCRA_PA12               PWR_PUCRA_PA12_Msk                        /*!< Port PA12 Pull-Up set */
#define PWR_PUCRA_PA11_Pos           (11U)
#define PWR_PUCRA_PA11_Msk           (0x1UL << PWR_PUCRA_PA11_Pos)             /*!< 0x00000800 */
#define PWR_PUCRA_PA11               PWR_PUCRA_PA11_Msk                        /*!< Port PA11 Pull-Up set */
#define PWR_PUCRA_PA10_Pos           (10U)
#define PWR_PUCRA_PA10_Msk           (0x1UL << PWR_PUCRA_PA10_Pos)             /*!< 0x00000400 */
#define PWR_PUCRA_PA10               PWR_PUCRA_PA10_Msk                        /*!< Port PA10 Pull-Up set */
#define PWR_PUCRA_PA9_Pos            (9U)
#define PWR_PUCRA_PA9_Msk            (0x1UL << PWR_PUCRA_PA9_Pos)              /*!< 0x00000200 */
#define PWR_PUCRA_PA9                PWR_PUCRA_PA9_Msk                         /*!< Port PA9 Pull-Up set  */
#define PWR_PUCRA_PA8_Pos            (8U)
#define PWR_PUCRA_PA8_Msk            (0x1UL << PWR_PUCRA_PA8_Pos)              /*!< 0x00000100 */
#define PWR_PUCRA_PA8                PWR_PUCRA_PA8_Msk                         /*!< Port PA8 Pull-Up set  */
#define PWR_PUCRA_PA7_Pos            (7U)
#define PWR_PUCRA_PA7_Msk            (0x1UL << PWR_PUCRA_PA7_Pos)              /*!< 0x00000080 */
#define PWR_PUCRA_PA7                PWR_PUCRA_PA7_Msk                         /*!< Port PA7 Pull-Up set  */
#define PWR_PUCRA_PA6_Pos            (6U)
#define PWR_PUCRA_PA6_Msk            (0x1UL << PWR_PUCRA_PA6_Pos)              /*!< 0x00000040 */
#define PWR_PUCRA_PA6                PWR_PUCRA_PA6_Msk                         /*!< Port PA6 Pull-Up set  */
#define PWR_PUCRA_PA5_Pos            (5U)
#define PWR_PUCRA_PA5_Msk            (0x1UL << PWR_PUCRA_PA5_Pos)              /*!< 0x00000020 */
#define PWR_PUCRA_PA5                PWR_PUCRA_PA5_Msk                         /*!< Port PA5 Pull-Up set  */
#define PWR_PUCRA_PA4_Pos            (4U)
#define PWR_PUCRA_PA4_Msk            (0x1UL << PWR_PUCRA_PA4_Pos)              /*!< 0x00000010 */
#define PWR_PUCRA_PA4                PWR_PUCRA_PA4_Msk                         /*!< Port PA4 Pull-Up set  */
#define PWR_PUCRA_PA3_Pos            (3U)
#define PWR_PUCRA_PA3_Msk            (0x1UL << PWR_PUCRA_PA3_Pos)              /*!< 0x00000008 */
#define PWR_PUCRA_PA3                PWR_PUCRA_PA3_Msk                         /*!< Port PA3 Pull-Up set  */
#define PWR_PUCRA_PA2_Pos            (2U)
#define PWR_PUCRA_PA2_Msk            (0x1UL << PWR_PUCRA_PA2_Pos)              /*!< 0x00000004 */
#define PWR_PUCRA_PA2                PWR_PUCRA_PA2_Msk                         /*!< Port PA2 Pull-Up set  */
#define PWR_PUCRA_PA1_Pos            (1U)
#define PWR_PUCRA_PA1_Msk            (0x1UL << PWR_PUCRA_PA1_Pos)              /*!< 0x00000002 */
#define PWR_PUCRA_PA1                PWR_PUCRA_PA1_Msk                         /*!< Port PA1 Pull-Up set  */
#define PWR_PUCRA_PA0_Pos            (0U)
#define PWR_PUCRA_PA0_Msk            (0x1UL << PWR_PUCRA_PA0_Pos)              /*!< 0x00000001 */
#define PWR_PUCRA_PA0                PWR_PUCRA_PA0_Msk                         /*!< Port PA0 Pull-Up set  */

/********************  Bit definition for PWR_PDCRA register  ********************/
#define PWR_PDCRA_PA14_Pos           (14U)
#define PWR_PDCRA_PA14_Msk           (0x1UL << PWR_PDCRA_PA14_Pos)             /*!< 0x00004000 */
#define PWR_PDCRA_PA14               PWR_PDCRA_PA14_Msk                        /*!< Port PA14 Pull-Down set */
#define PWR_PDCRA_PA12_Pos           (12U)
#define PWR_PDCRA_PA12_Msk           (0x1UL << PWR_PDCRA_PA12_Pos)             /*!< 0x00001000 */
#define PWR_PDCRA_PA12               PWR_PDCRA_PA12_Msk                        /*!< Port PA12 Pull-Down set */
#define PWR_PDCRA_PA11_Pos           (11U)
#define PWR_PDCRA_PA11_Msk           (0x1UL << PWR_PDCRA_PA11_Pos)             /*!< 0x00000800 */
#define PWR_PDCRA_PA11               PWR_PDCRA_PA11_Msk                        /*!< Port PA11 Pull-Down set */
#define PWR_PDCRA_PA10_Pos           (10U)
#define PWR_PDCRA_PA10_Msk           (0x1UL << PWR_PDCRA_PA10_Pos)             /*!< 0x00000400 */
#define PWR_PDCRA_PA10               PWR_PDCRA_PA10_Msk                        /*!< Port PA10 Pull-Down set */
#define PWR_PDCRA_PA9_Pos            (9U)
#define PWR_PDCRA_PA9_Msk            (0x1UL << PWR_PDCRA_PA9_Pos)              /*!< 0x00000200 */
#define PWR_PDCRA_PA9                PWR_PDCRA_PA9_Msk                         /*!< Port PA9 Pull-Down set  */
#define PWR_PDCRA_PA8_Pos            (8U)
#define PWR_PDCRA_PA8_Msk            (0x1UL << PWR_PDCRA_PA8_Pos)              /*!< 0x00000100 */
#define PWR_PDCRA_PA8                PWR_PDCRA_PA8_Msk                         /*!< Port PA8 Pull-Down set  */
#define PWR_PDCRA_PA7_Pos            (7U)
#define PWR_PDCRA_PA7_Msk            (0x1UL << PWR_PDCRA_PA7_Pos)              /*!< 0x00000080 */
#define PWR_PDCRA_PA7                PWR_PDCRA_PA7_Msk                         /*!< Port PA7 Pull-Down set  */
#define PWR_PDCRA_PA6_Pos            (6U)
#define PWR_PDCRA_PA6_Msk            (0x1UL << PWR_PDCRA_PA6_Pos)              /*!< 0x00000040 */
#define PWR_PDCRA_PA6                PWR_PDCRA_PA6_Msk                         /*!< Port PA6 Pull-Down set  */
#define PWR_PDCRA_PA5_Pos            (5U)
#define PWR_PDCRA_PA5_Msk            (0x1UL << PWR_PDCRA_PA5_Pos)              /*!< 0x00000020 */
#define PWR_PDCRA_PA5                PWR_PDCRA_PA5_Msk                         /*!< Port PA5 Pull-Down set  */
#define PWR_PDCRA_PA4_Pos            (4U)
#define PWR_PDCRA_PA4_Msk            (0x1UL << PWR_PDCRA_PA4_Pos)              /*!< 0x00000010 */
#define PWR_PDCRA_PA4                PWR_PDCRA_PA4_Msk                         /*!< Port PA4 Pull-Down set  */
#define PWR_PDCRA_PA3_Pos            (3U)
#define PWR_PDCRA_PA3_Msk            (0x1UL << PWR_PDCRA_PA3_Pos)              /*!< 0x00000008 */
#define PWR_PDCRA_PA3                PWR_PDCRA_PA3_Msk                         /*!< Port PA3 Pull-Down set  */
#define PWR_PDCRA_PA2_Pos            (2U)
#define PWR_PDCRA_PA2_Msk            (0x1UL << PWR_PDCRA_PA2_Pos)              /*!< 0x00000004 */
#define PWR_PDCRA_PA2                PWR_PDCRA_PA2_Msk                         /*!< Port PA2 Pull-Down set  */
#define PWR_PDCRA_PA1_Pos            (1U)
#define PWR_PDCRA_PA1_Msk            (0x1UL << PWR_PDCRA_PA1_Pos)              /*!< 0x00000002 */
#define PWR_PDCRA_PA1                PWR_PDCRA_PA1_Msk                         /*!< Port PA1 Pull-Down set  */
#define PWR_PDCRA_PA0_Pos            (0U)
#define PWR_PDCRA_PA0_Msk            (0x1UL << PWR_PDCRA_PA0_Pos)              /*!< 0x00000001 */
#define PWR_PDCRA_PA0                PWR_PDCRA_PA0_Msk                         /*!< Port PA0 Pull-Down set  */

/********************  Bit definition for PWR_PUCRB register  ********************/
#define PWR_PUCRB_PB15_Pos           (15U)
#define PWR_PUCRB_PB15_Msk           (0x1UL << PWR_PUCRB_PB15_Pos)             /*!< 0x00008000 */
#define PWR_PUCRB_PB15               PWR_PUCRB_PB15_Msk                        /*!< Port PB15 Pull-Up set */
#define PWR_PUCRB_PB14_Pos           (14U)
#define PWR_PUCRB_PB14_Msk           (0x1UL << PWR_PUCRB_PB14_Pos)             /*!< 0x00004000 */
#define PWR_PUCRB_PB14               PWR_PUCRB_PB14_Msk                        /*!< Port PB14 Pull-Up set */
#define PWR_PUCRB_PB13_Pos           (13U)
#define PWR_PUCRB_PB13_Msk           (0x1UL << PWR_PUCRB_PB13_Pos)             /*!< 0x00002000 */
#define PWR_PUCRB_PB13               PWR_PUCRB_PB13_Msk                        /*!< Port PB13 Pull-Up set */
#define PWR_PUCRB_PB12_Pos           (12U)
#define PWR_PUCRB_PB12_Msk           (0x1UL << PWR_PUCRB_PB12_Pos)             /*!< 0x00001000 */
#define PWR_PUCRB_PB12               PWR_PUCRB_PB12_Msk                        /*!< Port PB12 Pull-Up set */
#define PWR_PUCRB_PB11_Pos           (11U)
#define PWR_PUCRB_PB11_Msk           (0x1UL << PWR_PUCRB_PB11_Pos)             /*!< 0x00000800 */
#define PWR_PUCRB_PB11               PWR_PUCRB_PB11_Msk                        /*!< Port PB11 Pull-Up set */
#define PWR_PUCRB_PB10_Pos           (10U)
#define PWR_PUCRB_PB10_Msk           (0x1UL << PWR_PUCRB_PB10_Pos)             /*!< 0x00000400 */
#define PWR_PUCRB_PB10               PWR_PUCRB_PB10_Msk                        /*!< Port PB10 Pull-Up set */
#define PWR_PUCRB_PB9_Pos            (9U)
#define PWR_PUCRB_PB9_Msk            (0x1UL << PWR_PUCRB_PB9_Pos)              /*!< 0x00000200 */
#define PWR_PUCRB_PB9                PWR_PUCRB_PB9_Msk                         /*!< Port PB9 Pull-Up set  */
#define PWR_PUCRB_PB8_Pos            (8U)
#define PWR_PUCRB_PB8_Msk            (0x1UL << PWR_PUCRB_PB8_Pos)              /*!< 0x00000100 */
#define PWR_PUCRB_PB8                PWR_PUCRB_PB8_Msk                         /*!< Port PB8 Pull-Up set  */
#define PWR_PUCRB_PB7_Pos            (7U)
#define PWR_PUCRB_PB7_Msk            (0x1UL << PWR_PUCRB_PB7_Pos)              /*!< 0x00000080 */
#define PWR_PUCRB_PB7                PWR_PUCRB_PB7_Msk                         /*!< Port PB7 Pull-Up set  */
#define PWR_PUCRB_PB6_Pos            (6U)
#define PWR_PUCRB_PB6_Msk            (0x1UL << PWR_PUCRB_PB6_Pos)              /*!< 0x00000040 */
#define PWR_PUCRB_PB6                PWR_PUCRB_PB6_Msk                         /*!< Port PB6 Pull-Up set  */
#define PWR_PUCRB_PB5_Pos            (5U)
#define PWR_PUCRB_PB5_Msk            (0x1UL << PWR_PUCRB_PB5_Pos)              /*!< 0x00000020 */
#define PWR_PUCRB_PB5                PWR_PUCRB_PB5_Msk                         /*!< Port PB5 Pull-Up set  */
#define PWR_PUCRB_PB4_Pos            (4U)
#define PWR_PUCRB_PB4_Msk            (0x1UL << PWR_PUCRB_PB4_Pos)              /*!< 0x00000010 */
#define PWR_PUCRB_PB4                PWR_PUCRB_PB4_Msk                         /*!< Port PB4 Pull-Up set  */
#define PWR_PUCRB_PB3_Pos            (3U)
#define PWR_PUCRB_PB3_Msk            (0x1UL << PWR_PUCRB_PB3_Pos)              /*!< 0x00000008 */
#define PWR_PUCRB_PB3                PWR_PUCRB_PB3_Msk                         /*!< Port PB3 Pull-Up set  */
#define PWR_PUCRB_PB2_Pos            (2U)
#define PWR_PUCRB_PB2_Msk            (0x1UL << PWR_PUCRB_PB2_Pos)              /*!< 0x00000004 */
#define PWR_PUCRB_PB2                PWR_PUCRB_PB2_Msk                         /*!< Port PB2 Pull-Up set  */
#define PWR_PUCRB_PB1_Pos            (1U)
#define PWR_PUCRB_PB1_Msk            (0x1UL << PWR_PUCRB_PB1_Pos)              /*!< 0x00000002 */
#define PWR_PUCRB_PB1                PWR_PUCRB_PB1_Msk                         /*!< Port PB1 Pull-Up set  */
#define PWR_PUCRB_PB0_Pos            (0U)
#define PWR_PUCRB_PB0_Msk            (0x1UL << PWR_PUCRB_PB0_Pos)              /*!< 0x00000001 */
#define PWR_PUCRB_PB0                PWR_PUCRB_PB0_Msk                         /*!< Port PB0 Pull-Up set  */

/********************  Bit definition for PWR_PDCRB register  ********************/
#define PWR_PDCRB_PB15_Pos           (15U)
#define PWR_PDCRB_PB15_Msk           (0x1UL << PWR_PDCRB_PB15_Pos)             /*!< 0x00008000 */
#define PWR_PDCRB_PB15               PWR_PDCRB_PB15_Msk                        /*!< Port PB15 Pull-Down set */
#define PWR_PDCRB_PB14_Pos           (14U)
#define PWR_PDCRB_PB14_Msk           (0x1UL << PWR_PDCRB_PB14_Pos)             /*!< 0x00004000 */
#define PWR_PDCRB_PB14               PWR_PDCRB_PB14_Msk                        /*!< Port PB14 Pull-Down set */
#define PWR_PDCRB_PB13_Pos           (13U)
#define PWR_PDCRB_PB13_Msk           (0x1UL << PWR_PDCRB_PB13_Pos)             /*!< 0x00002000 */
#define PWR_PDCRB_PB13               PWR_PDCRB_PB13_Msk                        /*!< Port PB13 Pull-Down set */
#define PWR_PDCRB_PB12_Pos           (12U)
#define PWR_PDCRB_PB12_Msk           (0x1UL << PWR_PDCRB_PB12_Pos)             /*!< 0x00001000 */
#define PWR_PDCRB_PB12               PWR_PDCRB_PB12_Msk                        /*!< Port PB12 Pull-Down set */
#define PWR_PDCRB_PB11_Pos           (11U)
#define PWR_PDCRB_PB11_Msk           (0x1UL << PWR_PDCRB_PB11_Pos)             /*!< 0x00000800 */
#define PWR_PDCRB_PB11               PWR_PDCRB_PB11_Msk                        /*!< Port PB11 Pull-Down set */
#define PWR_PDCRB_PB10_Pos           (10U)
#define PWR_PDCRB_PB10_Msk           (0x1UL << PWR_PDCRB_PB10_Pos)             /*!< 0x00000400 */
#define PWR_PDCRB_PB10               PWR_PDCRB_PB10_Msk                        /*!< Port PB10 Pull-Down set */
#define PWR_PDCRB_PB9_Pos            (9U)
#define PWR_PDCRB_PB9_Msk            (0x1UL << PWR_PDCRB_PB9_Pos)              /*!< 0x00000200 */
#define PWR_PDCRB_PB9                PWR_PDCRB_PB9_Msk                         /*!< Port PB9 Pull-Down set  */
#define PWR_PDCRB_PB8_Pos            (8U)
#define PWR_PDCRB_PB8_Msk            (0x1UL << PWR_PDCRB_PB8_Pos)              /*!< 0x00000100 */
#define PWR_PDCRB_PB8                PWR_PDCRB_PB8_Msk                         /*!< Port PB8 Pull-Down set  */
#define PWR_PDCRB_PB7_Pos            (7U)
#define PWR_PDCRB_PB7_Msk            (0x1UL << PWR_PDCRB_PB7_Pos)              /*!< 0x00000080 */
#define PWR_PDCRB_PB7                PWR_PDCRB_PB7_Msk                         /*!< Port PB7 Pull-Down set  */
#define PWR_PDCRB_PB6_Pos            (6U)
#define PWR_PDCRB_PB6_Msk            (0x1UL << PWR_PDCRB_PB6_Pos)              /*!< 0x00000040 */
#define PWR_PDCRB_PB6                PWR_PDCRB_PB6_Msk                         /*!< Port PB6 Pull-Down set  */
#define PWR_PDCRB_PB5_Pos            (5U)
#define PWR_PDCRB_PB5_Msk            (0x1UL << PWR_PDCRB_PB5_Pos)              /*!< 0x00000020 */
#define PWR_PDCRB_PB5                PWR_PDCRB_PB5_Msk                         /*!< Port PB5 Pull-Down set  */
#define PWR_PDCRB_PB3_Pos            (3U)
#define PWR_PDCRB_PB3_Msk            (0x1UL << PWR_PDCRB_PB3_Pos)              /*!< 0x00000008 */
#define PWR_PDCRB_PB3                PWR_PDCRB_PB3_Msk                         /*!< Port PB3 Pull-Down set  */
#define PWR_PDCRB_PB2_Pos            (2U)
#define PWR_PDCRB_PB2_Msk            (0x1UL << PWR_PDCRB_PB2_Pos)              /*!< 0x00000004 */
#define PWR_PDCRB_PB2                PWR_PDCRB_PB2_Msk                         /*!< Port PB2 Pull-Down set  */
#define PWR_PDCRB_PB1_Pos            (1U)
#define PWR_PDCRB_PB1_Msk            (0x1UL << PWR_PDCRB_PB1_Pos)              /*!< 0x00000002 */
#define PWR_PDCRB_PB1                PWR_PDCRB_PB1_Msk                         /*!< Port PB1 Pull-Down set  */
#define PWR_PDCRB_PB0_Pos            (0U)
#define PWR_PDCRB_PB0_Msk            (0x1UL << PWR_PDCRB_PB0_Pos)              /*!< 0x00000001 */
#define PWR_PDCRB_PB0                PWR_PDCRB_PB0_Msk                         /*!< Port PB0 Pull-Down set  */

/********************  Bit definition for PWR_PUCRC register  ********************/
#define PWR_PUCRC_PC15_Pos           (15U)
#define PWR_PUCRC_PC15_Msk           (0x1UL << PWR_PUCRC_PC15_Pos)             /*!< 0x00008000 */
#define PWR_PUCRC_PC15               PWR_PUCRC_PC15_Msk                        /*!< Port PC15 Pull-Up set */
#define PWR_PUCRC_PC14_Pos           (14U)
#define PWR_PUCRC_PC14_Msk           (0x1UL << PWR_PUCRC_PC14_Pos)             /*!< 0x00004000 */
#define PWR_PUCRC_PC14               PWR_PUCRC_PC14_Msk                        /*!< Port PC14 Pull-Up set */
#define PWR_PUCRC_PC13_Pos           (13U)
#define PWR_PUCRC_PC13_Msk           (0x1UL << PWR_PUCRC_PC13_Pos)             /*!< 0x00002000 */
#define PWR_PUCRC_PC13               PWR_PUCRC_PC13_Msk                        /*!< Port PC13 Pull-Up set */
#define PWR_PUCRC_PC12_Pos           (12U)
#define PWR_PUCRC_PC12_Msk           (0x1UL << PWR_PUCRC_PC12_Pos)             /*!< 0x00001000 */
#define PWR_PUCRC_PC12               PWR_PUCRC_PC12_Msk                        /*!< Port PC12 Pull-Up set */
#define PWR_PUCRC_PC11_Pos           (11U)
#define PWR_PUCRC_PC11_Msk           (0x1UL << PWR_PUCRC_PC11_Pos)             /*!< 0x00000800 */
#define PWR_PUCRC_PC11               PWR_PUCRC_PC11_Msk                        /*!< Port PC11 Pull-Up set */
#define PWR_PUCRC_PC10_Pos           (10U)
#define PWR_PUCRC_PC10_Msk           (0x1UL << PWR_PUCRC_PC10_Pos)             /*!< 0x00000400 */
#define PWR_PUCRC_PC10               PWR_PUCRC_PC10_Msk                        /*!< Port PC10 Pull-Up set */
#define PWR_PUCRC_PC9_Pos            (9U)
#define PWR_PUCRC_PC9_Msk            (0x1UL << PWR_PUCRC_PC9_Pos)              /*!< 0x00000200 */
#define PWR_PUCRC_PC9                PWR_PUCRC_PC9_Msk                         /*!< Port PC9 Pull-Up set  */
#define PWR_PUCRC_PC8_Pos            (8U)
#define PWR_PUCRC_PC8_Msk            (0x1UL << PWR_PUCRC_PC8_Pos)              /*!< 0x00000100 */
#define PWR_PUCRC_PC8                PWR_PUCRC_PC8_Msk                         /*!< Port PC8 Pull-Up set  */
#define PWR_PUCRC_PC7_Pos            (7U)
#define PWR_PUCRC_PC7_Msk            (0x1UL << PWR_PUCRC_PC7_Pos)              /*!< 0x00000080 */
#define PWR_PUCRC_PC7                PWR_PUCRC_PC7_Msk                         /*!< Port PC7 Pull-Up set  */
#define PWR_PUCRC_PC6_Pos            (6U)
#define PWR_PUCRC_PC6_Msk            (0x1UL << PWR_PUCRC_PC6_Pos)              /*!< 0x00000040 */
#define PWR_PUCRC_PC6                PWR_PUCRC_PC6_Msk                         /*!< Port PC6 Pull-Up set  */
#define PWR_PUCRC_PC5_Pos            (5U)
#define PWR_PUCRC_PC5_Msk            (0x1UL << PWR_PUCRC_PC5_Pos)              /*!< 0x00000020 */
#define PWR_PUCRC_PC5                PWR_PUCRC_PC5_Msk                         /*!< Port PC5 Pull-Up set  */
#define PWR_PUCRC_PC4_Pos            (4U)
#define PWR_PUCRC_PC4_Msk            (0x1UL << PWR_PUCRC_PC4_Pos)              /*!< 0x00000010 */
#define PWR_PUCRC_PC4                PWR_PUCRC_PC4_Msk                         /*!< Port PC4 Pull-Up set  */
#define PWR_PUCRC_PC3_Pos            (3U)
#define PWR_PUCRC_PC3_Msk            (0x1UL << PWR_PUCRC_PC3_Pos)              /*!< 0x00000008 */
#define PWR_PUCRC_PC3                PWR_PUCRC_PC3_Msk                         /*!< Port PC3 Pull-Up set  */
#define PWR_PUCRC_PC2_Pos            (2U)
#define PWR_PUCRC_PC2_Msk            (0x1UL << PWR_PUCRC_PC2_Pos)              /*!< 0x00000004 */
#define PWR_PUCRC_PC2                PWR_PUCRC_PC2_Msk                         /*!< Port PC2 Pull-Up set  */
#define PWR_PUCRC_PC1_Pos            (1U)
#define PWR_PUCRC_PC1_Msk            (0x1UL << PWR_PUCRC_PC1_Pos)              /*!< 0x00000002 */
#define PWR_PUCRC_PC1                PWR_PUCRC_PC1_Msk                         /*!< Port PC1 Pull-Up set  */
#define PWR_PUCRC_PC0_Pos            (0U)
#define PWR_PUCRC_PC0_Msk            (0x1UL << PWR_PUCRC_PC0_Pos)              /*!< 0x00000001 */
#define PWR_PUCRC_PC0                PWR_PUCRC_PC0_Msk                         /*!< Port PC0 Pull-Up set  */

/********************  Bit definition for PWR_PDCRC register  ********************/
#define PWR_PDCRC_PC15_Pos           (15U)
#define PWR_PDCRC_PC15_Msk           (0x1UL << PWR_PDCRC_PC15_Pos)             /*!< 0x00008000 */
#define PWR_PDCRC_PC15               PWR_PDCRC_PC15_Msk                        /*!< Port PC15 Pull-Down set */
#define PWR_PDCRC_PC14_Pos           (14U)
#define PWR_PDCRC_PC14_Msk           (0x1UL << PWR_PDCRC_PC14_Pos)             /*!< 0x00004000 */
#define PWR_PDCRC_PC14               PWR_PDCRC_PC14_Msk                        /*!< Port PC14 Pull-Down set */
#define PWR_PDCRC_PC13_Pos           (13U)
#define PWR_PDCRC_PC13_Msk           (0x1UL << PWR_PDCRC_PC13_Pos)             /*!< 0x00002000 */
#define PWR_PDCRC_PC13               PWR_PDCRC_PC13_Msk                        /*!< Port PC13 Pull-Down set */
#define PWR_PDCRC_PC12_Pos           (12U)
#define PWR_PDCRC_PC12_Msk           (0x1UL << PWR_PDCRC_PC12_Pos)             /*!< 0x00001000 */
#define PWR_PDCRC_PC12               PWR_PDCRC_PC12_Msk                        /*!< Port PC12 Pull-Down set */
#define PWR_PDCRC_PC11_Pos           (11U)
#define PWR_PDCRC_PC11_Msk           (0x1UL << PWR_PDCRC_PC11_Pos)             /*!< 0x00000800 */
#define PWR_PDCRC_PC11               PWR_PDCRC_PC11_Msk                        /*!< Port PC11 Pull-Down set */
#define PWR_PDCRC_PC10_Pos           (10U)
#define PWR_PDCRC_PC10_Msk           (0x1UL << PWR_PDCRC_PC10_Pos)             /*!< 0x00000400 */
#define PWR_PDCRC_PC10               PWR_PDCRC_PC10_Msk                        /*!< Port PC10 Pull-Down set */
#define PWR_PDCRC_PC9_Pos            (9U)
#define PWR_PDCRC_PC9_Msk            (0x1UL << PWR_PDCRC_PC9_Pos)              /*!< 0x00000200 */
#define PWR_PDCRC_PC9                PWR_PDCRC_PC9_Msk                         /*!< Port PC9 Pull-Down set  */
#define PWR_PDCRC_PC8_Pos            (8U)
#define PWR_PDCRC_PC8_Msk            (0x1UL << PWR_PDCRC_PC8_Pos)              /*!< 0x00000100 */
#define PWR_PDCRC_PC8                PWR_PDCRC_PC8_Msk                         /*!< Port PC8 Pull-Down set  */
#define PWR_PDCRC_PC7_Pos            (7U)
#define PWR_PDCRC_PC7_Msk            (0x1UL << PWR_PDCRC_PC7_Pos)              /*!< 0x00000080 */
#define PWR_PDCRC_PC7                PWR_PDCRC_PC7_Msk                         /*!< Port PC7 Pull-Down set  */
#define PWR_PDCRC_PC6_Pos            (6U)
#define PWR_PDCRC_PC6_Msk            (0x1UL << PWR_PDCRC_PC6_Pos)              /*!< 0x00000040 */
#define PWR_PDCRC_PC6                PWR_PDCRC_PC6_Msk                         /*!< Port PC6 Pull-Down set  */
#define PWR_PDCRC_PC5_Pos            (5U)
#define PWR_PDCRC_PC5_Msk            (0x1UL << PWR_PDCRC_PC5_Pos)              /*!< 0x00000020 */
#define PWR_PDCRC_PC5                PWR_PDCRC_PC5_Msk                         /*!< Port PC5 Pull-Down set  */
#define PWR_PDCRC_PC4_Pos            (4U)
#define PWR_PDCRC_PC4_Msk            (0x1UL << PWR_PDCRC_PC4_Pos)              /*!< 0x00000010 */
#define PWR_PDCRC_PC4                PWR_PDCRC_PC4_Msk                         /*!< Port PC4 Pull-Down set  */
#define PWR_PDCRC_PC3_Pos            (3U)
#define PWR_PDCRC_PC3_Msk            (0x1UL << PWR_PDCRC_PC3_Pos)              /*!< 0x00000008 */
#define PWR_PDCRC_PC3                PWR_PDCRC_PC3_Msk                         /*!< Port PC3 Pull-Down set  */
#define PWR_PDCRC_PC2_Pos            (2U)
#define PWR_PDCRC_PC2_Msk            (0x1UL << PWR_PDCRC_PC2_Pos)              /*!< 0x00000004 */
#define PWR_PDCRC_PC2                PWR_PDCRC_PC2_Msk                         /*!< Port PC2 Pull-Down set  */
#define PWR_PDCRC_PC1_Pos            (1U)
#define PWR_PDCRC_PC1_Msk            (0x1UL << PWR_PDCRC_PC1_Pos)              /*!< 0x00000002 */
#define PWR_PDCRC_PC1                PWR_PDCRC_PC1_Msk                         /*!< Port PC1 Pull-Down set  */
#define PWR_PDCRC_PC0_Pos            (0U)
#define PWR_PDCRC_PC0_Msk            (0x1UL << PWR_PDCRC_PC0_Pos)              /*!< 0x00000001 */
#define PWR_PDCRC_PC0                PWR_PDCRC_PC0_Msk                         /*!< Port PC0 Pull-Down set  */

/********************  Bit definition for PWR_PUCRD register  ********************/
#define PWR_PUCRD_PD15_Pos           (15U)
#define PWR_PUCRD_PD15_Msk           (0x1UL << PWR_PUCRD_PD15_Pos)             /*!< 0x00008000 */
#define PWR_PUCRD_PD15               PWR_PUCRD_PD15_Msk                        /*!< Port PD15 Pull-Up set */
#define PWR_PUCRD_PD14_Pos           (14U)
#define PWR_PUCRD_PD14_Msk           (0x1UL << PWR_PUCRD_PD14_Pos)             /*!< 0x00004000 */
#define PWR_PUCRD_PD14               PWR_PUCRD_PD14_Msk                        /*!< Port PD14 Pull-Up set */
#define PWR_PUCRD_PD13_Pos           (13U)
#define PWR_PUCRD_PD13_Msk           (0x1UL << PWR_PUCRD_PD13_Pos)             /*!< 0x00002000 */
#define PWR_PUCRD_PD13               PWR_PUCRD_PD13_Msk                        /*!< Port PD13 Pull-Up set */
#define PWR_PUCRD_PD12_Pos           (12U)
#define PWR_PUCRD_PD12_Msk           (0x1UL << PWR_PUCRD_PD12_Pos)             /*!< 0x00001000 */
#define PWR_PUCRD_PD12               PWR_PUCRD_PD12_Msk                        /*!< Port PD12 Pull-Up set */
#define PWR_PUCRD_PD11_Pos           (11U)
#define PWR_PUCRD_PD11_Msk           (0x1UL << PWR_PUCRD_PD11_Pos)             /*!< 0x00000800 */
#define PWR_PUCRD_PD11               PWR_PUCRD_PD11_Msk                        /*!< Port PD11 Pull-Up set */
#define PWR_PUCRD_PD10_Pos           (10U)
#define PWR_PUCRD_PD10_Msk           (0x1UL << PWR_PUCRD_PD10_Pos)             /*!< 0x00000400 */
#define PWR_PUCRD_PD10               PWR_PUCRD_PD10_Msk                        /*!< Port PD10 Pull-Up set */
#define PWR_PUCRD_PD9_Pos            (9U)
#define PWR_PUCRD_PD9_Msk            (0x1UL << PWR_PUCRD_PD9_Pos)              /*!< 0x00000200 */
#define PWR_PUCRD_PD9                PWR_PUCRD_PD9_Msk                         /*!< Port PD9 Pull-Up set  */
#define PWR_PUCRD_PD8_Pos            (8U)
#define PWR_PUCRD_PD8_Msk            (0x1UL << PWR_PUCRD_PD8_Pos)              /*!< 0x00000100 */
#define PWR_PUCRD_PD8                PWR_PUCRD_PD8_Msk                         /*!< Port PD8 Pull-Up set  */
#define PWR_PUCRD_PD7_Pos            (7U)
#define PWR_PUCRD_PD7_Msk            (0x1UL << PWR_PUCRD_PD7_Pos)              /*!< 0x00000080 */
#define PWR_PUCRD_PD7                PWR_PUCRD_PD7_Msk                         /*!< Port PD7 Pull-Up set  */
#define PWR_PUCRD_PD6_Pos            (6U)
#define PWR_PUCRD_PD6_Msk            (0x1UL << PWR_PUCRD_PD6_Pos)              /*!< 0x00000040 */
#define PWR_PUCRD_PD6                PWR_PUCRD_PD6_Msk                         /*!< Port PD6 Pull-Up set  */
#define PWR_PUCRD_PD5_Pos            (5U)
#define PWR_PUCRD_PD5_Msk            (0x1UL << PWR_PUCRD_PD5_Pos)              /*!< 0x00000020 */
#define PWR_PUCRD_PD5                PWR_PUCRD_PD5_Msk                         /*!< Port PD5 Pull-Up set  */
#define PWR_PUCRD_PD4_Pos            (4U)
#define PWR_PUCRD_PD4_Msk            (0x1UL << PWR_PUCRD_PD4_Pos)              /*!< 0x00000010 */
#define PWR_PUCRD_PD4                PWR_PUCRD_PD4_Msk                         /*!< Port PD4 Pull-Up set  */
#define PWR_PUCRD_PD3_Pos            (3U)
#define PWR_PUCRD_PD3_Msk            (0x1UL << PWR_PUCRD_PD3_Pos)              /*!< 0x00000008 */
#define PWR_PUCRD_PD3                PWR_PUCRD_PD3_Msk                         /*!< Port PD3 Pull-Up set  */
#define PWR_PUCRD_PD2_Pos            (2U)
#define PWR_PUCRD_PD2_Msk            (0x1UL << PWR_PUCRD_PD2_Pos)              /*!< 0x00000004 */
#define PWR_PUCRD_PD2                PWR_PUCRD_PD2_Msk                         /*!< Port PD2 Pull-Up set  */
#define PWR_PUCRD_PD1_Pos            (1U)
#define PWR_PUCRD_PD1_Msk            (0x1UL << PWR_PUCRD_PD1_Pos)              /*!< 0x00000002 */
#define PWR_PUCRD_PD1                PWR_PUCRD_PD1_Msk                         /*!< Port PD1 Pull-Up set  */
#define PWR_PUCRD_PD0_Pos            (0U)
#define PWR_PUCRD_PD0_Msk            (0x1UL << PWR_PUCRD_PD0_Pos)              /*!< 0x00000001 */
#define PWR_PUCRD_PD0                PWR_PUCRD_PD0_Msk                         /*!< Port PD0 Pull-Up set  */

/********************  Bit definition for PWR_PDCRD register  ********************/
#define PWR_PDCRD_PD15_Pos           (15U)
#define PWR_PDCRD_PD15_Msk           (0x1UL << PWR_PDCRD_PD15_Pos)             /*!< 0x00008000 */
#define PWR_PDCRD_PD15               PWR_PDCRD_PD15_Msk                        /*!< Port PD15 Pull-Down set */
#define PWR_PDCRD_PD14_Pos           (14U)
#define PWR_PDCRD_PD14_Msk           (0x1UL << PWR_PDCRD_PD14_Pos)             /*!< 0x00004000 */
#define PWR_PDCRD_PD14               PWR_PDCRD_PD14_Msk                        /*!< Port PD14 Pull-Down set */
#define PWR_PDCRD_PD13_Pos           (13U)
#define PWR_PDCRD_PD13_Msk           (0x1UL << PWR_PDCRD_PD13_Pos)             /*!< 0x00002000 */
#define PWR_PDCRD_PD13               PWR_PDCRD_PD13_Msk                        /*!< Port PD13 Pull-Down set */
#define PWR_PDCRD_PD12_Pos           (12U)
#define PWR_PDCRD_PD12_Msk           (0x1UL << PWR_PDCRD_PD12_Pos)             /*!< 0x00001000 */
#define PWR_PDCRD_PD12               PWR_PDCRD_PD12_Msk                        /*!< Port PD12 Pull-Down set */
#define PWR_PDCRD_PD11_Pos           (11U)
#define PWR_PDCRD_PD11_Msk           (0x1UL << PWR_PDCRD_PD11_Pos)             /*!< 0x00000800 */
#define PWR_PDCRD_PD11               PWR_PDCRD_PD11_Msk                        /*!< Port PD11 Pull-Down set */
#define PWR_PDCRD_PD10_Pos           (10U)
#define PWR_PDCRD_PD10_Msk           (0x1UL << PWR_PDCRD_PD10_Pos)             /*!< 0x00000400 */
#define PWR_PDCRD_PD10               PWR_PDCRD_PD10_Msk                        /*!< Port PD10 Pull-Down set */
#define PWR_PDCRD_PD9_Pos            (9U)
#define PWR_PDCRD_PD9_Msk            (0x1UL << PWR_PDCRD_PD9_Pos)              /*!< 0x00000200 */
#define PWR_PDCRD_PD9                PWR_PDCRD_PD9_Msk                         /*!< Port PD9 Pull-Down set  */
#define PWR_PDCRD_PD8_Pos            (8U)
#define PWR_PDCRD_PD8_Msk            (0x1UL << PWR_PDCRD_PD8_Pos)              /*!< 0x00000100 */
#define PWR_PDCRD_PD8                PWR_PDCRD_PD8_Msk                         /*!< Port PD8 Pull-Down set  */
#define PWR_PDCRD_PD7_Pos            (7U)
#define PWR_PDCRD_PD7_Msk            (0x1UL << PWR_PDCRD_PD7_Pos)              /*!< 0x00000080 */
#define PWR_PDCRD_PD7                PWR_PDCRD_PD7_Msk                         /*!< Port PD7 Pull-Down set  */
#define PWR_PDCRD_PD6_Pos            (6U)
#define PWR_PDCRD_PD6_Msk            (0x1UL << PWR_PDCRD_PD6_Pos)              /*!< 0x00000040 */
#define PWR_PDCRD_PD6                PWR_PDCRD_PD6_Msk                         /*!< Port PD6 Pull-Down set  */
#define PWR_PDCRD_PD5_Pos            (5U)
#define PWR_PDCRD_PD5_Msk            (0x1UL << PWR_PDCRD_PD5_Pos)              /*!< 0x00000020 */
#define PWR_PDCRD_PD5                PWR_PDCRD_PD5_Msk                         /*!< Port PD5 Pull-Down set  */
#define PWR_PDCRD_PD4_Pos            (4U)
#define PWR_PDCRD_PD4_Msk            (0x1UL << PWR_PDCRD_PD4_Pos)              /*!< 0x00000010 */
#define PWR_PDCRD_PD4                PWR_PDCRD_PD4_Msk                         /*!< Port PD4 Pull-Down set  */
#define PWR_PDCRD_PD3_Pos            (3U)
#define PWR_PDCRD_PD3_Msk            (0x1UL << PWR_PDCRD_PD3_Pos)              /*!< 0x00000008 */
#define PWR_PDCRD_PD3                PWR_PDCRD_PD3_Msk                         /*!< Port PD3 Pull-Down set  */
#define PWR_PDCRD_PD2_Pos            (2U)
#define PWR_PDCRD_PD2_Msk            (0x1UL << PWR_PDCRD_PD2_Pos)              /*!< 0x00000004 */
#define PWR_PDCRD_PD2                PWR_PDCRD_PD2_Msk                         /*!< Port PD2 Pull-Down set  */
#define PWR_PDCRD_PD1_Pos            (1U)
#define PWR_PDCRD_PD1_Msk            (0x1UL << PWR_PDCRD_PD1_Pos)              /*!< 0x00000002 */
#define PWR_PDCRD_PD1                PWR_PDCRD_PD1_Msk                         /*!< Port PD1 Pull-Down set  */
#define PWR_PDCRD_PD0_Pos            (0U)
#define PWR_PDCRD_PD0_Msk            (0x1UL << PWR_PDCRD_PD0_Pos)              /*!< 0x00000001 */
#define PWR_PDCRD_PD0                PWR_PDCRD_PD0_Msk                         /*!< Port PD0 Pull-Down set  */



/********************  Bit definition for PWR_PUCRH register  ********************/
#define PWR_PUCRH_PH3_Pos            (3U)
#define PWR_PUCRH_PH3_Msk            (0x1UL << PWR_PUCRH_PH3_Pos)              /*!< 0x00000008 */
#define PWR_PUCRH_PH3                PWR_PUCRH_PH3_Msk                         /*!< Port PH3 Pull-Up set  */
#define PWR_PUCRH_PH1_Pos            (1U)
#define PWR_PUCRH_PH1_Msk            (0x1UL << PWR_PUCRH_PH1_Pos)              /*!< 0x00000002 */
#define PWR_PUCRH_PH1                PWR_PUCRH_PH1_Msk                         /*!< Port PH1 Pull-Up set  */
#define PWR_PUCRH_PH0_Pos            (0U)
#define PWR_PUCRH_PH0_Msk            (0x1UL << PWR_PUCRH_PH0_Pos)              /*!< 0x00000001 */
#define PWR_PUCRH_PH0                PWR_PUCRH_PH0_Msk                         /*!< Port PH0 Pull-Up set  */

/********************  Bit definition for PWR_PDCRH register  ********************/
#define PWR_PDCRH_PH3_Pos            (3U)
#define PWR_PDCRH_PH3_Msk            (0x1UL << PWR_PDCRH_PH3_Pos)              /*!< 0x00000008 */
#define PWR_PDCRH_PH3                PWR_PDCRH_PH3_Msk                         /*!< Port PH3 Pull-Down set  */
#define PWR_PDCRH_PH1_Pos            (1U)
#define PWR_PDCRH_PH1_Msk            (0x1UL << PWR_PDCRH_PH1_Pos)              /*!< 0x00000002 */
#define PWR_PDCRH_PH1                PWR_PDCRH_PH1_Msk                         /*!< Port PH1 Pull-Down set  */
#define PWR_PDCRH_PH0_Pos            (0U)
#define PWR_PDCRH_PH0_Msk            (0x1UL << PWR_PDCRH_PH0_Pos)              /*!< 0x00000001 */
#define PWR_PDCRH_PH0                PWR_PDCRH_PH0_Msk                         /*!< Port PH0 Pull-Down set  */


/******************************************************************************/
/*                                                                            */
/*                         Reset and Clock Control                            */
/*                                                                            */
/******************************************************************************/
/*
* @brief Specific device feature definitions  (not present on all devices in the STM32L4 serie)
*/
#define RCC_HSI48_SUPPORT

/********************  Bit definition for RCC_CR register  ********************/
#define RCC_CR_MSION_Pos                     (0U)
#define RCC_CR_MSION_Msk                     (0x1UL << RCC_CR_MSION_Pos)       /*!< 0x00000001 */
#define RCC_CR_MSION                         RCC_CR_MSION_Msk                  /*!< Internal Multi Speed oscillator (MSI) clock enable */
#define RCC_CR_MSIRDY_Pos                    (1U)
#define RCC_CR_MSIRDY_Msk                    (0x1UL << RCC_CR_MSIRDY_Pos)      /*!< 0x00000002 */
#define RCC_CR_MSIRDY                        RCC_CR_MSIRDY_Msk                 /*!< Internal Multi Speed oscillator (MSI) clock ready flag */
#define RCC_CR_MSIPLLEN_Pos                  (2U)
#define RCC_CR_MSIPLLEN_Msk                  (0x1UL << RCC_CR_MSIPLLEN_Pos)    /*!< 0x00000004 */
#define RCC_CR_MSIPLLEN                      RCC_CR_MSIPLLEN_Msk               /*!< Internal Multi Speed oscillator (MSI) PLL enable */
#define RCC_CR_MSIRGSEL_Pos                  (3U)
#define RCC_CR_MSIRGSEL_Msk                  (0x1UL << RCC_CR_MSIRGSEL_Pos)    /*!< 0x00000008 */
#define RCC_CR_MSIRGSEL                      RCC_CR_MSIRGSEL_Msk               /*!< Internal Multi Speed oscillator (MSI) range selection */

/*!< MSIRANGE configuration : 12 frequency ranges available */
#define RCC_CR_MSIRANGE_Pos                  (4U)
#define RCC_CR_MSIRANGE_Msk                  (0xFUL << RCC_CR_MSIRANGE_Pos)    /*!< 0x000000F0 */
#define RCC_CR_MSIRANGE                      RCC_CR_MSIRANGE_Msk               /*!< Internal Multi Speed oscillator (MSI) clock Range */
#define RCC_CR_MSIRANGE_0                    (0x0UL << RCC_CR_MSIRANGE_Pos)    /*!< 0x00000000 */
#define RCC_CR_MSIRANGE_1                    (0x1UL << RCC_CR_MSIRANGE_Pos)    /*!< 0x00000010 */
#define RCC_CR_MSIRANGE_2                    (0x2UL << RCC_CR_MSIRANGE_Pos)    /*!< 0x00000020 */
#define RCC_CR_MSIRANGE_3                    (0x3UL << RCC_CR_MSIRANGE_Pos)    /*!< 0x00000030 */
#define RCC_CR_MSIRANGE_4                    (0x4UL << RCC_CR_MSIRANGE_Pos)    /*!< 0x00000040 */
#define RCC_CR_MSIRANGE_5                    (0x5UL << RCC_CR_MSIRANGE_Pos)    /*!< 0x00000050 */
#define RCC_CR_MSIRANGE_6                    (0x6UL << RCC_CR_MSIRANGE_Pos)    /*!< 0x00000060 */
#define RCC_CR_MSIRANGE_7                    (0x7UL << RCC_CR_MSIRANGE_Pos)    /*!< 0x00000070 */
#define RCC_CR_MSIRANGE_8                    (0x8UL << RCC_CR_MSIRANGE_Pos)    /*!< 0x00000080 */
#define RCC_CR_MSIRANGE_9                    (0x9UL << RCC_CR_MSIRANGE_Pos)    /*!< 0x00000090 */
#define RCC_CR_MSIRANGE_10                   (0xAUL << RCC_CR_MSIRANGE_Pos)    /*!< 0x000000A0 */
#define RCC_CR_MSIRANGE_11                   (0xBUL << RCC_CR_MSIRANGE_Pos)    /*!< 0x000000B0 */

#define RCC_CR_HSION_Pos                     (8U)
#define RCC_CR_HSION_Msk                     (0x1UL << RCC_CR_HSION_Pos)       /*!< 0x00000100 */
#define RCC_CR_HSION                         RCC_CR_HSION_Msk                  /*!< Internal High Speed oscillator (HSI16) clock enable */
#define RCC_CR_HSIKERON_Pos                  (9U)
#define RCC_CR_HSIKERON_Msk                  (0x1UL << RCC_CR_HSIKERON_Pos)    /*!< 0x00000200 */
#define RCC_CR_HSIKERON                      RCC_CR_HSIKERON_Msk               /*!< Internal High Speed oscillator (HSI16) clock enable for some IPs Kernel */
#define RCC_CR_HSIRDY_Pos                    (10U)
#define RCC_CR_HSIRDY_Msk                    (0x1UL << RCC_CR_HSIRDY_Pos)      /*!< 0x00000400 */
#define RCC_CR_HSIRDY                        RCC_CR_HSIRDY_Msk                 /*!< Internal High Speed oscillator (HSI16) clock ready flag */
#define RCC_CR_HSIASFS_Pos                   (11U)
#define RCC_CR_HSIASFS_Msk                   (0x1UL << RCC_CR_HSIASFS_Pos)     /*!< 0x00000800 */
#define RCC_CR_HSIASFS                       RCC_CR_HSIASFS_Msk                /*!< HSI16 Automatic Start from Stop */

#define RCC_CR_HSEON_Pos                     (16U)
#define RCC_CR_HSEON_Msk                     (0x1UL << RCC_CR_HSEON_Pos)       /*!< 0x00010000 */
#define RCC_CR_HSEON                         RCC_CR_HSEON_Msk                  /*!< External High Speed oscillator (HSE) clock enable */
#define RCC_CR_HSERDY_Pos                    (17U)
#define RCC_CR_HSERDY_Msk                    (0x1UL << RCC_CR_HSERDY_Pos)      /*!< 0x00020000 */
#define RCC_CR_HSERDY                        RCC_CR_HSERDY_Msk                 /*!< External High Speed oscillator (HSE) clock ready */
#define RCC_CR_HSEBYP_Pos                    (18U)
#define RCC_CR_HSEBYP_Msk                    (0x1UL << RCC_CR_HSEBYP_Pos)      /*!< 0x00040000 */
#define RCC_CR_HSEBYP                        RCC_CR_HSEBYP_Msk                 /*!< External High Speed oscillator (HSE) clock bypass */
#define RCC_CR_CSSON_Pos                     (19U)
#define RCC_CR_CSSON_Msk                     (0x1UL << RCC_CR_CSSON_Pos)       /*!< 0x00080000 */
#define RCC_CR_CSSON                         RCC_CR_CSSON_Msk                  /*!< HSE Clock Security System enable */

#define RCC_CR_PLLON_Pos                     (24U)
#define RCC_CR_PLLON_Msk                     (0x1UL << RCC_CR_PLLON_Pos)       /*!< 0x01000000 */
#define RCC_CR_PLLON                         RCC_CR_PLLON_Msk                  /*!< System PLL clock enable */
#define RCC_CR_PLLRDY_Pos                    (25U)
#define RCC_CR_PLLRDY_Msk                    (0x1UL << RCC_CR_PLLRDY_Pos)      /*!< 0x02000000 */
#define RCC_CR_PLLRDY                        RCC_CR_PLLRDY_Msk                 /*!< System PLL clock ready */

/********************  Bit definition for RCC_ICSCR register  ***************/
/*!< MSICAL configuration */
#define RCC_ICSCR_MSICAL_Pos                 (0U)
#define RCC_ICSCR_MSICAL_Msk                 (0xFFUL << RCC_ICSCR_MSICAL_Pos)  /*!< 0x000000FF */
#define RCC_ICSCR_MSICAL                     RCC_ICSCR_MSICAL_Msk              /*!< MSICAL[7:0] bits */
#define RCC_ICSCR_MSICAL_0                   (0x01UL << RCC_ICSCR_MSICAL_Pos)  /*!< 0x00000001 */
#define RCC_ICSCR_MSICAL_1                   (0x02UL << RCC_ICSCR_MSICAL_Pos)  /*!< 0x00000002 */
#define RCC_ICSCR_MSICAL_2                   (0x04UL << RCC_ICSCR_MSICAL_Pos)  /*!< 0x00000004 */
#define RCC_ICSCR_MSICAL_3                   (0x08UL << RCC_ICSCR_MSICAL_Pos)  /*!< 0x00000008 */
#define RCC_ICSCR_MSICAL_4                   (0x10UL << RCC_ICSCR_MSICAL_Pos)  /*!< 0x00000010 */
#define RCC_ICSCR_MSICAL_5                   (0x20UL << RCC_ICSCR_MSICAL_Pos)  /*!< 0x00000020 */
#define RCC_ICSCR_MSICAL_6                   (0x40UL << RCC_ICSCR_MSICAL_Pos)  /*!< 0x00000040 */
#define RCC_ICSCR_MSICAL_7                   (0x80UL << RCC_ICSCR_MSICAL_Pos)  /*!< 0x00000080 */

/*!< MSITRIM configuration */
#define RCC_ICSCR_MSITRIM_Pos                (8U)
#define RCC_ICSCR_MSITRIM_Msk                (0xFFUL << RCC_ICSCR_MSITRIM_Pos) /*!< 0x0000FF00 */
#define RCC_ICSCR_MSITRIM                    RCC_ICSCR_MSITRIM_Msk             /*!< MSITRIM[7:0] bits */
#define RCC_ICSCR_MSITRIM_0                  (0x01UL << RCC_ICSCR_MSITRIM_Pos) /*!< 0x00000100 */
#define RCC_ICSCR_MSITRIM_1                  (0x02UL << RCC_ICSCR_MSITRIM_Pos) /*!< 0x00000200 */
#define RCC_ICSCR_MSITRIM_2                  (0x04UL << RCC_ICSCR_MSITRIM_Pos) /*!< 0x00000400 */
#define RCC_ICSCR_MSITRIM_3                  (0x08UL << RCC_ICSCR_MSITRIM_Pos) /*!< 0x00000800 */
#define RCC_ICSCR_MSITRIM_4                  (0x10UL << RCC_ICSCR_MSITRIM_Pos) /*!< 0x00001000 */
#define RCC_ICSCR_MSITRIM_5                  (0x20UL << RCC_ICSCR_MSITRIM_Pos) /*!< 0x00002000 */
#define RCC_ICSCR_MSITRIM_6                  (0x40UL << RCC_ICSCR_MSITRIM_Pos) /*!< 0x00004000 */
#define RCC_ICSCR_MSITRIM_7                  (0x80UL << RCC_ICSCR_MSITRIM_Pos) /*!< 0x00008000 */

/*!< HSICAL configuration */
#define RCC_ICSCR_HSICAL_Pos                 (16U)
#define RCC_ICSCR_HSICAL_Msk                 (0xFFUL << RCC_ICSCR_HSICAL_Pos)  /*!< 0x00FF0000 */
#define RCC_ICSCR_HSICAL                     RCC_ICSCR_HSICAL_Msk              /*!< HSICAL[7:0] bits */
#define RCC_ICSCR_HSICAL_0                   (0x01UL << RCC_ICSCR_HSICAL_Pos)  /*!< 0x00010000 */
#define RCC_ICSCR_HSICAL_1                   (0x02UL << RCC_ICSCR_HSICAL_Pos)  /*!< 0x00020000 */
#define RCC_ICSCR_HSICAL_2                   (0x04UL << RCC_ICSCR_HSICAL_Pos)  /*!< 0x00040000 */
#define RCC_ICSCR_HSICAL_3                   (0x08UL << RCC_ICSCR_HSICAL_Pos)  /*!< 0x00080000 */
#define RCC_ICSCR_HSICAL_4                   (0x10UL << RCC_ICSCR_HSICAL_Pos)  /*!< 0x00100000 */
#define RCC_ICSCR_HSICAL_5                   (0x20UL << RCC_ICSCR_HSICAL_Pos)  /*!< 0x00200000 */
#define RCC_ICSCR_HSICAL_6                   (0x40UL << RCC_ICSCR_HSICAL_Pos)  /*!< 0x00400000 */
#define RCC_ICSCR_HSICAL_7                   (0x80UL << RCC_ICSCR_HSICAL_Pos)  /*!< 0x00800000 */

/*!< HSITRIM configuration */
#define RCC_ICSCR_HSITRIM_Pos                (24U)
#define RCC_ICSCR_HSITRIM_Msk                (0x7FUL << RCC_ICSCR_HSITRIM_Pos) /*!< 0x7F000000 */
#define RCC_ICSCR_HSITRIM                    RCC_ICSCR_HSITRIM_Msk             /*!< HSITRIM[6:0] bits */
#define RCC_ICSCR_HSITRIM_0                  (0x01UL << RCC_ICSCR_HSITRIM_Pos) /*!< 0x01000000 */
#define RCC_ICSCR_HSITRIM_1                  (0x02UL << RCC_ICSCR_HSITRIM_Pos) /*!< 0x02000000 */
#define RCC_ICSCR_HSITRIM_2                  (0x04UL << RCC_ICSCR_HSITRIM_Pos) /*!< 0x04000000 */
#define RCC_ICSCR_HSITRIM_3                  (0x08UL << RCC_ICSCR_HSITRIM_Pos) /*!< 0x08000000 */
#define RCC_ICSCR_HSITRIM_4                  (0x10UL << RCC_ICSCR_HSITRIM_Pos) /*!< 0x10000000 */
#define RCC_ICSCR_HSITRIM_5                  (0x20UL << RCC_ICSCR_HSITRIM_Pos) /*!< 0x20000000 */
#define RCC_ICSCR_HSITRIM_6                  (0x40UL << RCC_ICSCR_HSITRIM_Pos) /*!< 0x40000000 */

/********************  Bit definition for RCC_CFGR register  ******************/
/*!< SW configuration */
#define RCC_CFGR_SW_Pos                      (0U)
#define RCC_CFGR_SW_Msk                      (0x3UL << RCC_CFGR_SW_Pos)        /*!< 0x00000003 */
#define RCC_CFGR_SW                          RCC_CFGR_SW_Msk                   /*!< SW[1:0] bits (System clock Switch) */
#define RCC_CFGR_SW_0                        (0x1UL << RCC_CFGR_SW_Pos)        /*!< 0x00000001 */
#define RCC_CFGR_SW_1                        (0x2UL << RCC_CFGR_SW_Pos)        /*!< 0x00000002 */

#define RCC_CFGR_SW_MSI                      (0x00000000UL)                    /*!< MSI oscillator selection as system clock */
#define RCC_CFGR_SW_HSI                      (0x00000001UL)                    /*!< HSI16 oscillator selection as system clock */
#define RCC_CFGR_SW_HSE                      (0x00000002UL)                    /*!< HSE oscillator selection as system clock */
#define RCC_CFGR_SW_PLL                      (0x00000003UL)                    /*!< PLL selection as system clock */

/*!< SWS configuration */
#define RCC_CFGR_SWS_Pos                     (2U)
#define RCC_CFGR_SWS_Msk                     (0x3UL << RCC_CFGR_SWS_Pos)       /*!< 0x0000000C */
#define RCC_CFGR_SWS                         RCC_CFGR_SWS_Msk                  /*!< SWS[1:0] bits (System Clock Switch Status) */
#define RCC_CFGR_SWS_0                       (0x1UL << RCC_CFGR_SWS_Pos)       /*!< 0x00000004 */
#define RCC_CFGR_SWS_1                       (0x2UL << RCC_CFGR_SWS_Pos)       /*!< 0x00000008 */

#define RCC_CFGR_SWS_MSI                     (0x00000000UL)                    /*!< MSI oscillator used as system clock */
#define RCC_CFGR_SWS_HSI                     (0x00000004UL)                    /*!< HSI16 oscillator used as system clock */
#define RCC_CFGR_SWS_HSE                     (0x00000008UL)                    /*!< HSE oscillator used as system clock */
#define RCC_CFGR_SWS_PLL                     (0x0000000CUL)                    /*!< PLL used as system clock */

/*!< HPRE configuration */
#define RCC_CFGR_HPRE_Pos                    (4U)
#define RCC_CFGR_HPRE_Msk                    (0xFUL << RCC_CFGR_HPRE_Pos)      /*!< 0x000000F0 */
#define RCC_CFGR_HPRE                        RCC_CFGR_HPRE_Msk                 /*!< HPRE[3:0] bits (AHB prescaler) */
#define RCC_CFGR_HPRE_0                      (0x1UL << RCC_CFGR_HPRE_Pos)      /*!< 0x00000010 */
#define RCC_CFGR_HPRE_1                      (0x2UL << RCC_CFGR_HPRE_Pos)      /*!< 0x00000020 */
#define RCC_CFGR_HPRE_2                      (0x4UL << RCC_CFGR_HPRE_Pos)      /*!< 0x00000040 */
#define RCC_CFGR_HPRE_3                      (0x8UL << RCC_CFGR_HPRE_Pos)      /*!< 0x00000080 */

#define RCC_CFGR_HPRE_DIV1                   (0x00000000UL)                    /*!< SYSCLK not divided */
#define RCC_CFGR_HPRE_DIV2                   (0x00000080UL)                    /*!< SYSCLK divided by 2 */
#define RCC_CFGR_HPRE_DIV4                   (0x00000090UL)                    /*!< SYSCLK divided by 4 */
#define RCC_CFGR_HPRE_DIV8                   (0x000000A0UL)                    /*!< SYSCLK divided by 8 */
#define RCC_CFGR_HPRE_DIV16                  (0x000000B0UL)                    /*!< SYSCLK divided by 16 */
#define RCC_CFGR_HPRE_DIV64                  (0x000000C0UL)                    /*!< SYSCLK divided by 64 */
#define RCC_CFGR_HPRE_DIV128                 (0x000000D0UL)                    /*!< SYSCLK divided by 128 */
#define RCC_CFGR_HPRE_DIV256                 (0x000000E0UL)                    /*!< SYSCLK divided by 256 */
#define RCC_CFGR_HPRE_DIV512                 (0x000000F0UL)                    /*!< SYSCLK divided by 512 */

/*!< PPRE1 configuration */
#define RCC_CFGR_PPRE1_Pos                   (8U)
#define RCC_CFGR_PPRE1_Msk                   (0x7UL << RCC_CFGR_PPRE1_Pos)     /*!< 0x00000700 */
#define RCC_CFGR_PPRE1                       RCC_CFGR_PPRE1_Msk                /*!< PRE1[2:0] bits (APB2 prescaler) */
#define RCC_CFGR_PPRE1_0                     (0x1UL << RCC_CFGR_PPRE1_Pos)     /*!< 0x00000100 */
#define RCC_CFGR_PPRE1_1                     (0x2UL << RCC_CFGR_PPRE1_Pos)     /*!< 0x00000200 */
#define RCC_CFGR_PPRE1_2                     (0x4UL << RCC_CFGR_PPRE1_Pos)     /*!< 0x00000400 */

#define RCC_CFGR_PPRE1_DIV1                  (0x00000000UL)                    /*!< HCLK not divided */
#define RCC_CFGR_PPRE1_DIV2                  (0x00000400UL)                    /*!< HCLK divided by 2 */
#define RCC_CFGR_PPRE1_DIV4                  (0x00000500UL)                    /*!< HCLK divided by 4 */
#define RCC_CFGR_PPRE1_DIV8                  (0x00000600UL)                    /*!< HCLK divided by 8 */
#define RCC_CFGR_PPRE1_DIV16                 (0x00000700UL)                    /*!< HCLK divided by 16 */

/*!< PPRE2 configuration */
#define RCC_CFGR_PPRE2_Pos                   (11U)
#define RCC_CFGR_PPRE2_Msk                   (0x7UL << RCC_CFGR_PPRE2_Pos)     /*!< 0x00003800 */
#define RCC_CFGR_PPRE2                       RCC_CFGR_PPRE2_Msk                /*!< PRE2[2:0] bits (APB2 prescaler) */
#define RCC_CFGR_PPRE2_0                     (0x1UL << RCC_CFGR_PPRE2_Pos)     /*!< 0x00000800 */
#define RCC_CFGR_PPRE2_1                     (0x2UL << RCC_CFGR_PPRE2_Pos)     /*!< 0x00001000 */
#define RCC_CFGR_PPRE2_2                     (0x4UL << RCC_CFGR_PPRE2_Pos)     /*!< 0x00002000 */

#define RCC_CFGR_PPRE2_DIV1                  (0x00000000UL)                    /*!< HCLK not divided */
#define RCC_CFGR_PPRE2_DIV2                  (0x00002000UL)                    /*!< HCLK divided by 2 */
#define RCC_CFGR_PPRE2_DIV4                  (0x00002800UL)                    /*!< HCLK divided by 4 */
#define RCC_CFGR_PPRE2_DIV8                  (0x00003000UL)                    /*!< HCLK divided by 8 */
#define RCC_CFGR_PPRE2_DIV16                 (0x00003800UL)                    /*!< HCLK divided by 16 */

#define RCC_CFGR_STOPWUCK_Pos                (15U)
#define RCC_CFGR_STOPWUCK_Msk                (0x1UL << RCC_CFGR_STOPWUCK_Pos)  /*!< 0x00008000 */
#define RCC_CFGR_STOPWUCK                    RCC_CFGR_STOPWUCK_Msk             /*!< Wake Up from stop and CSS backup clock selection */

/*!< MCOSEL configuration */
#define RCC_CFGR_MCOSEL_Pos                  (24U)
#define RCC_CFGR_MCOSEL_Msk                  (0xFUL << RCC_CFGR_MCOSEL_Pos)    /*!< 0x0F000000 */
#define RCC_CFGR_MCOSEL                      RCC_CFGR_MCOSEL_Msk               /*!< MCOSEL [3:0] bits (Clock output selection) */
#define RCC_CFGR_MCOSEL_0                    (0x1UL << RCC_CFGR_MCOSEL_Pos)    /*!< 0x01000000 */
#define RCC_CFGR_MCOSEL_1                    (0x2UL << RCC_CFGR_MCOSEL_Pos)    /*!< 0x02000000 */
#define RCC_CFGR_MCOSEL_2                    (0x4UL << RCC_CFGR_MCOSEL_Pos)    /*!< 0x04000000 */
#define RCC_CFGR_MCOSEL_3                    (0x8UL << RCC_CFGR_MCOSEL_Pos)    /*!< 0x08000000 */

#define RCC_CFGR_MCOPRE_Pos                  (28U)
#define RCC_CFGR_MCOPRE_Msk                  (0x7UL << RCC_CFGR_MCOPRE_Pos)    /*!< 0x70000000 */
#define RCC_CFGR_MCOPRE                      RCC_CFGR_MCOPRE_Msk               /*!< MCO prescaler */
#define RCC_CFGR_MCOPRE_0                    (0x1UL << RCC_CFGR_MCOPRE_Pos)    /*!< 0x10000000 */
#define RCC_CFGR_MCOPRE_1                    (0x2UL << RCC_CFGR_MCOPRE_Pos)    /*!< 0x20000000 */
#define RCC_CFGR_MCOPRE_2                    (0x4UL << RCC_CFGR_MCOPRE_Pos)    /*!< 0x40000000 */

#define RCC_CFGR_MCOPRE_DIV1                 (0x00000000UL)                    /*!< MCO is divided by 1 */
#define RCC_CFGR_MCOPRE_DIV2                 (0x10000000UL)                    /*!< MCO is divided by 2 */
#define RCC_CFGR_MCOPRE_DIV4                 (0x20000000UL)                    /*!< MCO is divided by 4 */
#define RCC_CFGR_MCOPRE_DIV8                 (0x30000000UL)                    /*!< MCO is divided by 8 */
#define RCC_CFGR_MCOPRE_DIV16                (0x40000000UL)                    /*!< MCO is divided by 16 */

/* Legacy aliases */
#define RCC_CFGR_MCO_PRE                     RCC_CFGR_MCOPRE
#define RCC_CFGR_MCO_PRE_1                   RCC_CFGR_MCOPRE_DIV1
#define RCC_CFGR_MCO_PRE_2                   RCC_CFGR_MCOPRE_DIV2
#define RCC_CFGR_MCO_PRE_4                   RCC_CFGR_MCOPRE_DIV4
#define RCC_CFGR_MCO_PRE_8                   RCC_CFGR_MCOPRE_DIV8
#define RCC_CFGR_MCO_PRE_16                  RCC_CFGR_MCOPRE_DIV16

/********************  Bit definition for RCC_PLLCFGR register  ***************/
#define RCC_PLLCFGR_PLLSRC_Pos               (0U)
#define RCC_PLLCFGR_PLLSRC_Msk               (0x3UL << RCC_PLLCFGR_PLLSRC_Pos) /*!< 0x00000003 */
#define RCC_PLLCFGR_PLLSRC                   RCC_PLLCFGR_PLLSRC_Msk

#define RCC_PLLCFGR_PLLSRC_MSI_Pos           (0U)
#define RCC_PLLCFGR_PLLSRC_MSI_Msk           (0x1UL << RCC_PLLCFGR_PLLSRC_MSI_Pos) /*!< 0x00000001 */
#define RCC_PLLCFGR_PLLSRC_MSI               RCC_PLLCFGR_PLLSRC_MSI_Msk        /*!< MSI oscillator source clock selected */
#define RCC_PLLCFGR_PLLSRC_HSI_Pos           (1U)
#define RCC_PLLCFGR_PLLSRC_HSI_Msk           (0x1UL << RCC_PLLCFGR_PLLSRC_HSI_Pos) /*!< 0x00000002 */
#define RCC_PLLCFGR_PLLSRC_HSI               RCC_PLLCFGR_PLLSRC_HSI_Msk        /*!< HSI16 oscillator source clock selected */
#define RCC_PLLCFGR_PLLSRC_HSE_Pos           (0U)
#define RCC_PLLCFGR_PLLSRC_HSE_Msk           (0x3UL << RCC_PLLCFGR_PLLSRC_HSE_Pos) /*!< 0x00000003 */
#define RCC_PLLCFGR_PLLSRC_HSE               RCC_PLLCFGR_PLLSRC_HSE_Msk        /*!< HSE oscillator source clock selected */

#define RCC_PLLCFGR_PLLM_Pos                 (4U)
#define RCC_PLLCFGR_PLLM_Msk                 (0x7UL << RCC_PLLCFGR_PLLM_Pos)   /*!< 0x00000070 */
#define RCC_PLLCFGR_PLLM                     RCC_PLLCFGR_PLLM_Msk
#define RCC_PLLCFGR_PLLM_0                   (0x1UL << RCC_PLLCFGR_PLLM_Pos)   /*!< 0x00000010 */
#define RCC_PLLCFGR_PLLM_1                   (0x2UL << RCC_PLLCFGR_PLLM_Pos)   /*!< 0x00000020 */
#define RCC_PLLCFGR_PLLM_2                   (0x4UL << RCC_PLLCFGR_PLLM_Pos)   /*!< 0x00000040 */

#define RCC_PLLCFGR_PLLN_Pos                 (8U)
#define RCC_PLLCFGR_PLLN_Msk                 (0x7FUL << RCC_PLLCFGR_PLLN_Pos)  /*!< 0x00007F00 */
#define RCC_PLLCFGR_PLLN                     RCC_PLLCFGR_PLLN_Msk
#define RCC_PLLCFGR_PLLN_0                   (0x01UL << RCC_PLLCFGR_PLLN_Pos)  /*!< 0x00000100 */
#define RCC_PLLCFGR_PLLN_1                   (0x02UL << RCC_PLLCFGR_PLLN_Pos)  /*!< 0x00000200 */
#define RCC_PLLCFGR_PLLN_2                   (0x04UL << RCC_PLLCFGR_PLLN_Pos)  /*!< 0x00000400 */
#define RCC_PLLCFGR_PLLN_3                   (0x08UL << RCC_PLLCFGR_PLLN_Pos)  /*!< 0x00000800 */
#define RCC_PLLCFGR_PLLN_4                   (0x10UL << RCC_PLLCFGR_PLLN_Pos)  /*!< 0x00001000 */
#define RCC_PLLCFGR_PLLN_5                   (0x20UL << RCC_PLLCFGR_PLLN_Pos)  /*!< 0x00002000 */
#define RCC_PLLCFGR_PLLN_6                   (0x40UL << RCC_PLLCFGR_PLLN_Pos)  /*!< 0x00004000 */

#define RCC_PLLCFGR_PLLQEN_Pos               (20U)
#define RCC_PLLCFGR_PLLQEN_Msk               (0x1UL << RCC_PLLCFGR_PLLQEN_Pos) /*!< 0x00100000 */
#define RCC_PLLCFGR_PLLQEN                   RCC_PLLCFGR_PLLQEN_Msk

#define RCC_PLLCFGR_PLLQ_Pos                 (21U)
#define RCC_PLLCFGR_PLLQ_Msk                 (0x3UL << RCC_PLLCFGR_PLLQ_Pos)   /*!< 0x00600000 */
#define RCC_PLLCFGR_PLLQ                     RCC_PLLCFGR_PLLQ_Msk
#define RCC_PLLCFGR_PLLQ_0                   (0x1UL << RCC_PLLCFGR_PLLQ_Pos)   /*!< 0x00200000 */
#define RCC_PLLCFGR_PLLQ_1                   (0x2UL << RCC_PLLCFGR_PLLQ_Pos)   /*!< 0x00400000 */

#define RCC_PLLCFGR_PLLREN_Pos               (24U)
#define RCC_PLLCFGR_PLLREN_Msk               (0x1UL << RCC_PLLCFGR_PLLREN_Pos) /*!< 0x01000000 */
#define RCC_PLLCFGR_PLLREN                   RCC_PLLCFGR_PLLREN_Msk
#define RCC_PLLCFGR_PLLR_Pos                 (25U)
#define RCC_PLLCFGR_PLLR_Msk                 (0x3UL << RCC_PLLCFGR_PLLR_Pos)   /*!< 0x06000000 */
#define RCC_PLLCFGR_PLLR                     RCC_PLLCFGR_PLLR_Msk
#define RCC_PLLCFGR_PLLR_0                   (0x1UL << RCC_PLLCFGR_PLLR_Pos)   /*!< 0x02000000 */
#define RCC_PLLCFGR_PLLR_1                   (0x2UL << RCC_PLLCFGR_PLLR_Pos)   /*!< 0x04000000 */

/********************  Bit definition for RCC_CIER register  ******************/
#define RCC_CIER_LSIRDYIE_Pos                (0U)
#define RCC_CIER_LSIRDYIE_Msk                (0x1UL << RCC_CIER_LSIRDYIE_Pos)  /*!< 0x00000001 */
#define RCC_CIER_LSIRDYIE                    RCC_CIER_LSIRDYIE_Msk
#define RCC_CIER_LSERDYIE_Pos                (1U)
#define RCC_CIER_LSERDYIE_Msk                (0x1UL << RCC_CIER_LSERDYIE_Pos)  /*!< 0x00000002 */
#define RCC_CIER_LSERDYIE                    RCC_CIER_LSERDYIE_Msk
#define RCC_CIER_MSIRDYIE_Pos                (2U)
#define RCC_CIER_MSIRDYIE_Msk                (0x1UL << RCC_CIER_MSIRDYIE_Pos)  /*!< 0x00000004 */
#define RCC_CIER_MSIRDYIE                    RCC_CIER_MSIRDYIE_Msk
#define RCC_CIER_HSIRDYIE_Pos                (3U)
#define RCC_CIER_HSIRDYIE_Msk                (0x1UL << RCC_CIER_HSIRDYIE_Pos)  /*!< 0x00000008 */
#define RCC_CIER_HSIRDYIE                    RCC_CIER_HSIRDYIE_Msk
#define RCC_CIER_HSERDYIE_Pos                (4U)
#define RCC_CIER_HSERDYIE_Msk                (0x1UL << RCC_CIER_HSERDYIE_Pos)  /*!< 0x00000010 */
#define RCC_CIER_HSERDYIE                    RCC_CIER_HSERDYIE_Msk
#define RCC_CIER_PLLRDYIE_Pos                (5U)
#define RCC_CIER_PLLRDYIE_Msk                (0x1UL << RCC_CIER_PLLRDYIE_Pos)  /*!< 0x00000020 */
#define RCC_CIER_PLLRDYIE                    RCC_CIER_PLLRDYIE_Msk
#define RCC_CIER_LSECSSIE_Pos                (9U)
#define RCC_CIER_LSECSSIE_Msk                (0x1UL << RCC_CIER_LSECSSIE_Pos)  /*!< 0x00000200 */
#define RCC_CIER_LSECSSIE                    RCC_CIER_LSECSSIE_Msk
#define RCC_CIER_HSI48RDYIE_Pos              (10U)
#define RCC_CIER_HSI48RDYIE_Msk              (0x1UL << RCC_CIER_HSI48RDYIE_Pos) /*!< 0x00000400 */
#define RCC_CIER_HSI48RDYIE                  RCC_CIER_HSI48RDYIE_Msk

/********************  Bit definition for RCC_CIFR register  ******************/
#define RCC_CIFR_LSIRDYF_Pos                 (0U)
#define RCC_CIFR_LSIRDYF_Msk                 (0x1UL << RCC_CIFR_LSIRDYF_Pos)   /*!< 0x00000001 */
#define RCC_CIFR_LSIRDYF                     RCC_CIFR_LSIRDYF_Msk
#define RCC_CIFR_LSERDYF_Pos                 (1U)
#define RCC_CIFR_LSERDYF_Msk                 (0x1UL << RCC_CIFR_LSERDYF_Pos)   /*!< 0x00000002 */
#define RCC_CIFR_LSERDYF                     RCC_CIFR_LSERDYF_Msk
#define RCC_CIFR_MSIRDYF_Pos                 (2U)
#define RCC_CIFR_MSIRDYF_Msk                 (0x1UL << RCC_CIFR_MSIRDYF_Pos)   /*!< 0x00000004 */
#define RCC_CIFR_MSIRDYF                     RCC_CIFR_MSIRDYF_Msk
#define RCC_CIFR_HSIRDYF_Pos                 (3U)
#define RCC_CIFR_HSIRDYF_Msk                 (0x1UL << RCC_CIFR_HSIRDYF_Pos)   /*!< 0x00000008 */
#define RCC_CIFR_HSIRDYF                     RCC_CIFR_HSIRDYF_Msk
#define RCC_CIFR_HSERDYF_Pos                 (4U)
#define RCC_CIFR_HSERDYF_Msk                 (0x1UL << RCC_CIFR_HSERDYF_Pos)   /*!< 0x00000010 */
#define RCC_CIFR_HSERDYF                     RCC_CIFR_HSERDYF_Msk
#define RCC_CIFR_PLLRDYF_Pos                 (5U)
#define RCC_CIFR_PLLRDYF_Msk                 (0x1UL << RCC_CIFR_PLLRDYF_Pos)   /*!< 0x00000020 */
#define RCC_CIFR_PLLRDYF                     RCC_CIFR_PLLRDYF_Msk
#define RCC_CIFR_CSSF_Pos                    (8U)
#define RCC_CIFR_CSSF_Msk                    (0x1UL << RCC_CIFR_CSSF_Pos)      /*!< 0x00000100 */
#define RCC_CIFR_CSSF                        RCC_CIFR_CSSF_Msk
#define RCC_CIFR_LSECSSF_Pos                 (9U)
#define RCC_CIFR_LSECSSF_Msk                 (0x1UL << RCC_CIFR_LSECSSF_Pos)   /*!< 0x00000200 */
#define RCC_CIFR_LSECSSF                     RCC_CIFR_LSECSSF_Msk
#define RCC_CIFR_HSI48RDYF_Pos               (10U)
#define RCC_CIFR_HSI48RDYF_Msk               (0x1UL << RCC_CIFR_HSI48RDYF_Pos) /*!< 0x00000400 */
#define RCC_CIFR_HSI48RDYF                   RCC_CIFR_HSI48RDYF_Msk

/********************  Bit definition for RCC_CICR register  ******************/
#define RCC_CICR_LSIRDYC_Pos                 (0U)
#define RCC_CICR_LSIRDYC_Msk                 (0x1UL << RCC_CICR_LSIRDYC_Pos)   /*!< 0x00000001 */
#define RCC_CICR_LSIRDYC                     RCC_CICR_LSIRDYC_Msk
#define RCC_CICR_LSERDYC_Pos                 (1U)
#define RCC_CICR_LSERDYC_Msk                 (0x1UL << RCC_CICR_LSERDYC_Pos)   /*!< 0x00000002 */
#define RCC_CICR_LSERDYC                     RCC_CICR_LSERDYC_Msk
#define RCC_CICR_MSIRDYC_Pos                 (2U)
#define RCC_CICR_MSIRDYC_Msk                 (0x1UL << RCC_CICR_MSIRDYC_Pos)   /*!< 0x00000004 */
#define RCC_CICR_MSIRDYC                     RCC_CICR_MSIRDYC_Msk
#define RCC_CICR_HSIRDYC_Pos                 (3U)
#define RCC_CICR_HSIRDYC_Msk                 (0x1UL << RCC_CICR_HSIRDYC_Pos)   /*!< 0x00000008 */
#define RCC_CICR_HSIRDYC                     RCC_CICR_HSIRDYC_Msk
#define RCC_CICR_HSERDYC_Pos                 (4U)
#define RCC_CICR_HSERDYC_Msk                 (0x1UL << RCC_CICR_HSERDYC_Pos)   /*!< 0x00000010 */
#define RCC_CICR_HSERDYC                     RCC_CICR_HSERDYC_Msk
#define RCC_CICR_PLLRDYC_Pos                 (5U)
#define RCC_CICR_PLLRDYC_Msk                 (0x1UL << RCC_CICR_PLLRDYC_Pos)   /*!< 0x00000020 */
#define RCC_CICR_PLLRDYC                     RCC_CICR_PLLRDYC_Msk
#define RCC_CICR_CSSC_Pos                    (8U)
#define RCC_CICR_CSSC_Msk                    (0x1UL << RCC_CICR_CSSC_Pos)      /*!< 0x00000100 */
#define RCC_CICR_CSSC                        RCC_CICR_CSSC_Msk
#define RCC_CICR_LSECSSC_Pos                 (9U)
#define RCC_CICR_LSECSSC_Msk                 (0x1UL << RCC_CICR_LSECSSC_Pos)   /*!< 0x00000200 */
#define RCC_CICR_LSECSSC                     RCC_CICR_LSECSSC_Msk
#define RCC_CICR_HSI48RDYC_Pos               (10U)
#define RCC_CICR_HSI48RDYC_Msk               (0x1UL << RCC_CICR_HSI48RDYC_Pos) /*!< 0x00000400 */
#define RCC_CICR_HSI48RDYC                   RCC_CICR_HSI48RDYC_Msk

/********************  Bit definition for RCC_AHB1RSTR register  **************/
#define RCC_AHB1RSTR_DMA1RST_Pos             (0U)
#define RCC_AHB1RSTR_DMA1RST_Msk             (0x1UL << RCC_AHB1RSTR_DMA1RST_Pos) /*!< 0x00000001 */
#define RCC_AHB1RSTR_DMA1RST                 RCC_AHB1RSTR_DMA1RST_Msk
#define RCC_AHB1RSTR_DMA2RST_Pos             (1U)
#define RCC_AHB1RSTR_DMA2RST_Msk             (0x1UL << RCC_AHB1RSTR_DMA2RST_Pos) /*!< 0x00000002 */
#define RCC_AHB1RSTR_DMA2RST                 RCC_AHB1RSTR_DMA2RST_Msk
#define RCC_AHB1RSTR_FLASHRST_Pos            (8U)
#define RCC_AHB1RSTR_FLASHRST_Msk            (0x1UL << RCC_AHB1RSTR_FLASHRST_Pos) /*!< 0x00000100 */
#define RCC_AHB1RSTR_FLASHRST                RCC_AHB1RSTR_FLASHRST_Msk
#define RCC_AHB1RSTR_CRCRST_Pos              (12U)
#define RCC_AHB1RSTR_CRCRST_Msk              (0x1UL << RCC_AHB1RSTR_CRCRST_Pos) /*!< 0x00001000 */
#define RCC_AHB1RSTR_CRCRST                  RCC_AHB1RSTR_CRCRST_Msk
#define RCC_AHB1RSTR_TSCRST_Pos              (16U)
#define RCC_AHB1RSTR_TSCRST_Msk              (0x1UL << RCC_AHB1RSTR_TSCRST_Pos) /*!< 0x00010000 */
#define RCC_AHB1RSTR_TSCRST                  RCC_AHB1RSTR_TSCRST_Msk

/********************  Bit definition for RCC_AHB2RSTR register  **************/
#define RCC_AHB2RSTR_GPIOARST_Pos            (0U)
#define RCC_AHB2RSTR_GPIOARST_Msk            (0x1UL << RCC_AHB2RSTR_GPIOARST_Pos) /*!< 0x00000001 */
#define RCC_AHB2RSTR_GPIOARST                RCC_AHB2RSTR_GPIOARST_Msk
#define RCC_AHB2RSTR_GPIOBRST_Pos            (1U)
#define RCC_AHB2RSTR_GPIOBRST_Msk            (0x1UL << RCC_AHB2RSTR_GPIOBRST_Pos) /*!< 0x00000002 */
#define RCC_AHB2RSTR_GPIOBRST                RCC_AHB2RSTR_GPIOBRST_Msk
#define RCC_AHB2RSTR_GPIOCRST_Pos            (2U)
#define RCC_AHB2RSTR_GPIOCRST_Msk            (0x1UL << RCC_AHB2RSTR_GPIOCRST_Pos) /*!< 0x00000004 */
#define RCC_AHB2RSTR_GPIOCRST                RCC_AHB2RSTR_GPIOCRST_Msk
#define RCC_AHB2RSTR_GPIODRST_Pos            (3U)
#define RCC_AHB2RSTR_GPIODRST_Msk            (0x1UL << RCC_AHB2RSTR_GPIODRST_Pos) /*!< 0x00000008 */
#define RCC_AHB2RSTR_GPIODRST                RCC_AHB2RSTR_GPIODRST_Msk
#define RCC_AHB2RSTR_GPIOHRST_Pos            (7U)
#define RCC_AHB2RSTR_GPIOHRST_Msk            (0x1UL << RCC_AHB2RSTR_GPIOHRST_Pos) /*!< 0x00000080 */
#define RCC_AHB2RSTR_GPIOHRST                RCC_AHB2RSTR_GPIOHRST_Msk
#define RCC_AHB2RSTR_ADCRST_Pos              (13U)
#define RCC_AHB2RSTR_ADCRST_Msk              (0x1UL << RCC_AHB2RSTR_ADCRST_Pos) /*!< 0x00002000 */
#define RCC_AHB2RSTR_ADCRST                  RCC_AHB2RSTR_ADCRST_Msk
#define RCC_AHB2RSTR_AESRST_Pos              (16U)
#define RCC_AHB2RSTR_AESRST_Msk              (0x1UL << RCC_AHB2RSTR_AESRST_Pos) /*!< 0x00010000 */
#define RCC_AHB2RSTR_AESRST                  RCC_AHB2RSTR_AESRST_Msk
#define RCC_AHB2RSTR_RNGRST_Pos              (18U)
#define RCC_AHB2RSTR_RNGRST_Msk              (0x1UL << RCC_AHB2RSTR_RNGRST_Pos) /*!< 0x00040000 */
#define RCC_AHB2RSTR_RNGRST                  RCC_AHB2RSTR_RNGRST_Msk

/********************  Bit definition for RCC_AHB3RSTR register  **************/
#define RCC_AHB3RSTR_QSPIRST_Pos             (8U)
#define RCC_AHB3RSTR_QSPIRST_Msk             (0x1UL << RCC_AHB3RSTR_QSPIRST_Pos) /*!< 0x00000100 */
#define RCC_AHB3RSTR_QSPIRST                 RCC_AHB3RSTR_QSPIRST_Msk

/********************  Bit definition for RCC_APB1RSTR1 register  **************/
#define RCC_APB1RSTR1_TIM2RST_Pos            (0U)
#define RCC_APB1RSTR1_TIM2RST_Msk            (0x1UL << RCC_APB1RSTR1_TIM2RST_Pos) /*!< 0x00000001 */
#define RCC_APB1RSTR1_TIM2RST                RCC_APB1RSTR1_TIM2RST_Msk
#define RCC_APB1RSTR1_TIM6RST_Pos            (4U)
#define RCC_APB1RSTR1_TIM6RST_Msk            (0x1UL << RCC_APB1RSTR1_TIM6RST_Pos) /*!< 0x00000010 */
#define RCC_APB1RSTR1_TIM6RST                RCC_APB1RSTR1_TIM6RST_Msk
#define RCC_APB1RSTR1_SPI2RST_Pos            (14U)
#define RCC_APB1RSTR1_SPI2RST_Msk            (0x1UL << RCC_APB1RSTR1_SPI2RST_Pos) /*!< 0x00004000 */
#define RCC_APB1RSTR1_SPI2RST                RCC_APB1RSTR1_SPI2RST_Msk
#define RCC_APB1RSTR1_USART2RST_Pos          (17U)
#define RCC_APB1RSTR1_USART2RST_Msk          (0x1UL << RCC_APB1RSTR1_USART2RST_Pos) /*!< 0x00020000 */
#define RCC_APB1RSTR1_USART2RST              RCC_APB1RSTR1_USART2RST_Msk
#define RCC_APB1RSTR1_USART3RST_Pos          (18U)
#define RCC_APB1RSTR1_USART3RST_Msk          (0x1UL << RCC_APB1RSTR1_USART3RST_Pos) /*!< 0x00040000 */
#define RCC_APB1RSTR1_USART3RST              RCC_APB1RSTR1_USART3RST_Msk
#define RCC_APB1RSTR1_I2C1RST_Pos            (21U)
#define RCC_APB1RSTR1_I2C1RST_Msk            (0x1UL << RCC_APB1RSTR1_I2C1RST_Pos) /*!< 0x00200000 */
#define RCC_APB1RSTR1_I2C1RST                RCC_APB1RSTR1_I2C1RST_Msk
#define RCC_APB1RSTR1_I2C2RST_Pos            (22U)
#define RCC_APB1RSTR1_I2C2RST_Msk            (0x1UL << RCC_APB1RSTR1_I2C2RST_Pos) /*!< 0x00400000 */
#define RCC_APB1RSTR1_I2C2RST                RCC_APB1RSTR1_I2C2RST_Msk
#define RCC_APB1RSTR1_I2C3RST_Pos            (23U)
#define RCC_APB1RSTR1_I2C3RST_Msk            (0x1UL << RCC_APB1RSTR1_I2C3RST_Pos) /*!< 0x00800000 */
#define RCC_APB1RSTR1_I2C3RST                RCC_APB1RSTR1_I2C3RST_Msk
#define RCC_APB1RSTR1_CRSRST_Pos             (24U)
#define RCC_APB1RSTR1_CRSRST_Msk             (0x1UL << RCC_APB1RSTR1_CRSRST_Pos) /*!< 0x01000000 */
#define RCC_APB1RSTR1_CRSRST                 RCC_APB1RSTR1_CRSRST_Msk
#define RCC_APB1RSTR1_USBFSRST_Pos           (26U)
#define RCC_APB1RSTR1_USBFSRST_Msk           (0x1UL << RCC_APB1RSTR1_USBFSRST_Pos) /*!< 0x04000000 */
#define RCC_APB1RSTR1_USBFSRST               RCC_APB1RSTR1_USBFSRST_Msk
#define RCC_APB1RSTR1_PWRRST_Pos             (28U)
#define RCC_APB1RSTR1_PWRRST_Msk             (0x1UL << RCC_APB1RSTR1_PWRRST_Pos) /*!< 0x10000000 */
#define RCC_APB1RSTR1_PWRRST                 RCC_APB1RSTR1_PWRRST_Msk
#define RCC_APB1RSTR1_OPAMPRST_Pos           (30U)
#define RCC_APB1RSTR1_OPAMPRST_Msk           (0x1UL << RCC_APB1RSTR1_OPAMPRST_Pos) /*!< 0x40000000 */
#define RCC_APB1RSTR1_OPAMPRST               RCC_APB1RSTR1_OPAMPRST_Msk
#define RCC_APB1RSTR1_LPTIM1RST_Pos          (31U)
#define RCC_APB1RSTR1_LPTIM1RST_Msk          (0x1UL << RCC_APB1RSTR1_LPTIM1RST_Pos) /*!< 0x80000000 */
#define RCC_APB1RSTR1_LPTIM1RST              RCC_APB1RSTR1_LPTIM1RST_Msk

/********************  Bit definition for RCC_APB1RSTR2 register  **************/
#define RCC_APB1RSTR2_LPUART1RST_Pos         (0U)
#define RCC_APB1RSTR2_LPUART1RST_Msk         (0x1UL << RCC_APB1RSTR2_LPUART1RST_Pos) /*!< 0x00000001 */
#define RCC_APB1RSTR2_LPUART1RST             RCC_APB1RSTR2_LPUART1RST_Msk
#define RCC_APB1RSTR2_LPTIM2RST_Pos          (5U)
#define RCC_APB1RSTR2_LPTIM2RST_Msk          (0x1UL << RCC_APB1RSTR2_LPTIM2RST_Pos) /*!< 0x00000020 */
#define RCC_APB1RSTR2_LPTIM2RST              RCC_APB1RSTR2_LPTIM2RST_Msk

/********************  Bit definition for RCC_APB2RSTR register  **************/
#define RCC_APB2RSTR_SYSCFGRST_Pos           (0U)
#define RCC_APB2RSTR_SYSCFGRST_Msk           (0x1UL << RCC_APB2RSTR_SYSCFGRST_Pos) /*!< 0x00000001 */
#define RCC_APB2RSTR_SYSCFGRST               RCC_APB2RSTR_SYSCFGRST_Msk
#define RCC_APB2RSTR_TIM1RST_Pos             (11U)
#define RCC_APB2RSTR_TIM1RST_Msk             (0x1UL << RCC_APB2RSTR_TIM1RST_Pos) /*!< 0x00000800 */
#define RCC_APB2RSTR_TIM1RST                 RCC_APB2RSTR_TIM1RST_Msk
#define RCC_APB2RSTR_SPI1RST_Pos             (12U)
#define RCC_APB2RSTR_SPI1RST_Msk             (0x1UL << RCC_APB2RSTR_SPI1RST_Pos) /*!< 0x00001000 */
#define RCC_APB2RSTR_SPI1RST                 RCC_APB2RSTR_SPI1RST_Msk
#define RCC_APB2RSTR_USART1RST_Pos           (14U)
#define RCC_APB2RSTR_USART1RST_Msk           (0x1UL << RCC_APB2RSTR_USART1RST_Pos) /*!< 0x00004000 */
#define RCC_APB2RSTR_USART1RST               RCC_APB2RSTR_USART1RST_Msk
#define RCC_APB2RSTR_TIM15RST_Pos            (16U)
#define RCC_APB2RSTR_TIM15RST_Msk            (0x1UL << RCC_APB2RSTR_TIM15RST_Pos) /*!< 0x00010000 */
#define RCC_APB2RSTR_TIM15RST                RCC_APB2RSTR_TIM15RST_Msk
#define RCC_APB2RSTR_TIM16RST_Pos            (17U)
#define RCC_APB2RSTR_TIM16RST_Msk            (0x1UL << RCC_APB2RSTR_TIM16RST_Pos) /*!< 0x00020000 */
#define RCC_APB2RSTR_TIM16RST                RCC_APB2RSTR_TIM16RST_Msk

/********************  Bit definition for RCC_AHB1ENR register  ***************/
#define RCC_AHB1ENR_DMA1EN_Pos               (0U)
#define RCC_AHB1ENR_DMA1EN_Msk               (0x1UL << RCC_AHB1ENR_DMA1EN_Pos) /*!< 0x00000001 */
#define RCC_AHB1ENR_DMA1EN                   RCC_AHB1ENR_DMA1EN_Msk
#define RCC_AHB1ENR_DMA2EN_Pos               (1U)
#define RCC_AHB1ENR_DMA2EN_Msk               (0x1UL << RCC_AHB1ENR_DMA2EN_Pos) /*!< 0x00000002 */
#define RCC_AHB1ENR_DMA2EN                   RCC_AHB1ENR_DMA2EN_Msk
#define RCC_AHB1ENR_FLASHEN_Pos              (8U)
#define RCC_AHB1ENR_FLASHEN_Msk              (0x1UL << RCC_AHB1ENR_FLASHEN_Pos) /*!< 0x00000100 */
#define RCC_AHB1ENR_FLASHEN                  RCC_AHB1ENR_FLASHEN_Msk
#define RCC_AHB1ENR_CRCEN_Pos                (12U)
#define RCC_AHB1ENR_CRCEN_Msk                (0x1UL << RCC_AHB1ENR_CRCEN_Pos)  /*!< 0x00001000 */
#define RCC_AHB1ENR_CRCEN                    RCC_AHB1ENR_CRCEN_Msk
#define RCC_AHB1ENR_TSCEN_Pos                (16U)
#define RCC_AHB1ENR_TSCEN_Msk                (0x1UL << RCC_AHB1ENR_TSCEN_Pos)  /*!< 0x00010000 */
#define RCC_AHB1ENR_TSCEN                    RCC_AHB1ENR_TSCEN_Msk

/********************  Bit definition for RCC_AHB2ENR register  ***************/
#define RCC_AHB2ENR_GPIOAEN_Pos              (0U)
#define RCC_AHB2ENR_GPIOAEN_Msk              (0x1UL << RCC_AHB2ENR_GPIOAEN_Pos) /*!< 0x00000001 */
#define RCC_AHB2ENR_GPIOAEN                  RCC_AHB2ENR_GPIOAEN_Msk
#define RCC_AHB2ENR_GPIOBEN_Pos              (1U)
#define RCC_AHB2ENR_GPIOBEN_Msk              (0x1UL << RCC_AHB2ENR_GPIOBEN_Pos) /*!< 0x00000002 */
#define RCC_AHB2ENR_GPIOBEN                  RCC_AHB2ENR_GPIOBEN_Msk
#define RCC_AHB2ENR_GPIOCEN_Pos              (2U)
#define RCC_AHB2ENR_GPIOCEN_Msk              (0x1UL << RCC_AHB2ENR_GPIOCEN_Pos) /*!< 0x00000004 */
#define RCC_AHB2ENR_GPIOCEN                  RCC_AHB2ENR_GPIOCEN_Msk
#define RCC_AHB2ENR_GPIODEN_Pos              (3U)
#define RCC_AHB2ENR_GPIODEN_Msk              (0x1UL << RCC_AHB2ENR_GPIODEN_Pos) /*!< 0x00000008 */
#define RCC_AHB2ENR_GPIODEN                  RCC_AHB2ENR_GPIODEN_Msk
#define RCC_AHB2ENR_GPIOHEN_Pos              (7U)
#define RCC_AHB2ENR_GPIOHEN_Msk              (0x1UL << RCC_AHB2ENR_GPIOHEN_Pos) /*!< 0x00000080 */
#define RCC_AHB2ENR_GPIOHEN                  RCC_AHB2ENR_GPIOHEN_Msk
#define RCC_AHB2ENR_ADCEN_Pos                (13U)
#define RCC_AHB2ENR_ADCEN_Msk                (0x1UL << RCC_AHB2ENR_ADCEN_Pos)  /*!< 0x00002000 */
#define RCC_AHB2ENR_ADCEN                    RCC_AHB2ENR_ADCEN_Msk
#define RCC_AHB2ENR_AESEN_Pos                (16U)
#define RCC_AHB2ENR_AESEN_Msk                (0x1UL << RCC_AHB2ENR_AESEN_Pos)  /*!< 0x00010000 */
#define RCC_AHB2ENR_AESEN                    RCC_AHB2ENR_AESEN_Msk
#define RCC_AHB2ENR_RNGEN_Pos                (18U)
#define RCC_AHB2ENR_RNGEN_Msk                (0x1UL << RCC_AHB2ENR_RNGEN_Pos)  /*!< 0x00040000 */
#define RCC_AHB2ENR_RNGEN                    RCC_AHB2ENR_RNGEN_Msk

/********************  Bit definition for RCC_AHB3ENR register  ***************/
#define RCC_AHB3ENR_QSPIEN_Pos               (8U)
#define RCC_AHB3ENR_QSPIEN_Msk               (0x1UL << RCC_AHB3ENR_QSPIEN_Pos) /*!< 0x00000100 */
#define RCC_AHB3ENR_QSPIEN                   RCC_AHB3ENR_QSPIEN_Msk

/********************  Bit definition for RCC_APB1ENR1 register  ***************/
#define RCC_APB1ENR1_TIM2EN_Pos              (0U)
#define RCC_APB1ENR1_TIM2EN_Msk              (0x1UL << RCC_APB1ENR1_TIM2EN_Pos) /*!< 0x00000001 */
#define RCC_APB1ENR1_TIM2EN                  RCC_APB1ENR1_TIM2EN_Msk
#define RCC_APB1ENR1_TIM6EN_Pos              (4U)
#define RCC_APB1ENR1_TIM6EN_Msk              (0x1UL << RCC_APB1ENR1_TIM6EN_Pos) /*!< 0x00000010 */
#define RCC_APB1ENR1_TIM6EN                  RCC_APB1ENR1_TIM6EN_Msk
#define RCC_APB1ENR1_RTCAPBEN_Pos            (10U)
#define RCC_APB1ENR1_RTCAPBEN_Msk            (0x1UL << RCC_APB1ENR1_RTCAPBEN_Pos) /*!< 0x00000400 */
#define RCC_APB1ENR1_RTCAPBEN                RCC_APB1ENR1_RTCAPBEN_Msk
#define RCC_APB1ENR1_WWDGEN_Pos              (11U)
#define RCC_APB1ENR1_WWDGEN_Msk              (0x1UL << RCC_APB1ENR1_WWDGEN_Pos) /*!< 0x00000800 */
#define RCC_APB1ENR1_WWDGEN                  RCC_APB1ENR1_WWDGEN_Msk
#define RCC_APB1ENR1_SPI2EN_Pos              (14U)
#define RCC_APB1ENR1_SPI2EN_Msk              (0x1UL << RCC_APB1ENR1_SPI2EN_Pos) /*!< 0x00004000 */
#define RCC_APB1ENR1_SPI2EN                  RCC_APB1ENR1_SPI2EN_Msk
#define RCC_APB1ENR1_USART2EN_Pos            (17U)
#define RCC_APB1ENR1_USART2EN_Msk            (0x1UL << RCC_APB1ENR1_USART2EN_Pos) /*!< 0x00020000 */
#define RCC_APB1ENR1_USART2EN                RCC_APB1ENR1_USART2EN_Msk
#define RCC_APB1ENR1_USART3EN_Pos            (18U)
#define RCC_APB1ENR1_USART3EN_Msk            (0x1UL << RCC_APB1ENR1_USART3EN_Pos) /*!< 0x00040000 */
#define RCC_APB1ENR1_USART3EN                RCC_APB1ENR1_USART3EN_Msk
#define RCC_APB1ENR1_I2C1EN_Pos              (21U)
#define RCC_APB1ENR1_I2C1EN_Msk              (0x1UL << RCC_APB1ENR1_I2C1EN_Pos) /*!< 0x00200000 */
#define RCC_APB1ENR1_I2C1EN                  RCC_APB1ENR1_I2C1EN_Msk
#define RCC_APB1ENR1_I2C2EN_Pos              (22U)
#define RCC_APB1ENR1_I2C2EN_Msk              (0x1UL << RCC_APB1ENR1_I2C2EN_Pos) /*!< 0x00400000 */
#define RCC_APB1ENR1_I2C2EN                  RCC_APB1ENR1_I2C2EN_Msk
#define RCC_APB1ENR1_I2C3EN_Pos              (23U)
#define RCC_APB1ENR1_I2C3EN_Msk              (0x1UL << RCC_APB1ENR1_I2C3EN_Pos) /*!< 0x00800000 */
#define RCC_APB1ENR1_I2C3EN                  RCC_APB1ENR1_I2C3EN_Msk
#define RCC_APB1ENR1_CRSEN_Pos               (24U)
#define RCC_APB1ENR1_CRSEN_Msk               (0x1UL << RCC_APB1ENR1_CRSEN_Pos) /*!< 0x01000000 */
#define RCC_APB1ENR1_CRSEN                   RCC_APB1ENR1_CRSEN_Msk
#define RCC_APB1ENR1_USBFSEN_Pos             (26U)
#define RCC_APB1ENR1_USBFSEN_Msk             (0x1UL << RCC_APB1ENR1_USBFSEN_Pos) /*!< 0x04000000 */
#define RCC_APB1ENR1_USBFSEN                 RCC_APB1ENR1_USBFSEN_Msk
#define RCC_APB1ENR1_PWREN_Pos               (28U)
#define RCC_APB1ENR1_PWREN_Msk               (0x1UL << RCC_APB1ENR1_PWREN_Pos) /*!< 0x10000000 */
#define RCC_APB1ENR1_PWREN                   RCC_APB1ENR1_PWREN_Msk
#define RCC_APB1ENR1_OPAMPEN_Pos             (30U)
#define RCC_APB1ENR1_OPAMPEN_Msk             (0x1UL << RCC_APB1ENR1_OPAMPEN_Pos) /*!< 0x40000000 */
#define RCC_APB1ENR1_OPAMPEN                 RCC_APB1ENR1_OPAMPEN_Msk
#define RCC_APB1ENR1_LPTIM1EN_Pos            (31U)
#define RCC_APB1ENR1_LPTIM1EN_Msk            (0x1UL << RCC_APB1ENR1_LPTIM1EN_Pos) /*!< 0x80000000 */
#define RCC_APB1ENR1_LPTIM1EN                RCC_APB1ENR1_LPTIM1EN_Msk

/********************  Bit definition for RCC_APB1RSTR2 register  **************/
#define RCC_APB1ENR2_LPUART1EN_Pos           (0U)
#define RCC_APB1ENR2_LPUART1EN_Msk           (0x1UL << RCC_APB1ENR2_LPUART1EN_Pos) /*!< 0x00000001 */
#define RCC_APB1ENR2_LPUART1EN               RCC_APB1ENR2_LPUART1EN_Msk
#define RCC_APB1ENR2_LPTIM2EN_Pos            (5U)
#define RCC_APB1ENR2_LPTIM2EN_Msk            (0x1UL << RCC_APB1ENR2_LPTIM2EN_Pos) /*!< 0x00000020 */
#define RCC_APB1ENR2_LPTIM2EN                RCC_APB1ENR2_LPTIM2EN_Msk

/********************  Bit definition for RCC_APB2ENR register  ***************/
#define RCC_APB2ENR_SYSCFGEN_Pos             (0U)
#define RCC_APB2ENR_SYSCFGEN_Msk             (0x1UL << RCC_APB2ENR_SYSCFGEN_Pos) /*!< 0x00000001 */
#define RCC_APB2ENR_SYSCFGEN                 RCC_APB2ENR_SYSCFGEN_Msk
#define RCC_APB2ENR_FWEN_Pos                 (7U)
#define RCC_APB2ENR_FWEN_Msk                 (0x1UL << RCC_APB2ENR_FWEN_Pos)   /*!< 0x00000080 */
#define RCC_APB2ENR_FWEN                     RCC_APB2ENR_FWEN_Msk
#define RCC_APB2ENR_TIM1EN_Pos               (11U)
#define RCC_APB2ENR_TIM1EN_Msk               (0x1UL << RCC_APB2ENR_TIM1EN_Pos) /*!< 0x00000800 */
#define RCC_APB2ENR_TIM1EN                   RCC_APB2ENR_TIM1EN_Msk
#define RCC_APB2ENR_SPI1EN_Pos               (12U)
#define RCC_APB2ENR_SPI1EN_Msk               (0x1UL << RCC_APB2ENR_SPI1EN_Pos) /*!< 0x00001000 */
#define RCC_APB2ENR_SPI1EN                   RCC_APB2ENR_SPI1EN_Msk
#define RCC_APB2ENR_USART1EN_Pos             (14U)
#define RCC_APB2ENR_USART1EN_Msk             (0x1UL << RCC_APB2ENR_USART1EN_Pos) /*!< 0x00004000 */
#define RCC_APB2ENR_USART1EN                 RCC_APB2ENR_USART1EN_Msk
#define RCC_APB2ENR_TIM15EN_Pos              (16U)
#define RCC_APB2ENR_TIM15EN_Msk              (0x1UL << RCC_APB2ENR_TIM15EN_Pos) /*!< 0x00010000 */
#define RCC_APB2ENR_TIM15EN                  RCC_APB2ENR_TIM15EN_Msk
#define RCC_APB2ENR_TIM16EN_Pos              (17U)
#define RCC_APB2ENR_TIM16EN_Msk              (0x1UL << RCC_APB2ENR_TIM16EN_Pos) /*!< 0x00020000 */
#define RCC_APB2ENR_TIM16EN                  RCC_APB2ENR_TIM16EN_Msk

/********************  Bit definition for RCC_AHB1SMENR register  ***************/
#define RCC_AHB1SMENR_DMA1SMEN_Pos           (0U)
#define RCC_AHB1SMENR_DMA1SMEN_Msk           (0x1UL << RCC_AHB1SMENR_DMA1SMEN_Pos) /*!< 0x00000001 */
#define RCC_AHB1SMENR_DMA1SMEN               RCC_AHB1SMENR_DMA1SMEN_Msk
#define RCC_AHB1SMENR_DMA2SMEN_Pos           (1U)
#define RCC_AHB1SMENR_DMA2SMEN_Msk           (0x1UL << RCC_AHB1SMENR_DMA2SMEN_Pos) /*!< 0x00000002 */
#define RCC_AHB1SMENR_DMA2SMEN               RCC_AHB1SMENR_DMA2SMEN_Msk
#define RCC_AHB1SMENR_FLASHSMEN_Pos          (8U)
#define RCC_AHB1SMENR_FLASHSMEN_Msk          (0x1UL << RCC_AHB1SMENR_FLASHSMEN_Pos) /*!< 0x00000100 */
#define RCC_AHB1SMENR_FLASHSMEN              RCC_AHB1SMENR_FLASHSMEN_Msk
#define RCC_AHB1SMENR_SRAM1SMEN_Pos          (9U)
#define RCC_AHB1SMENR_SRAM1SMEN_Msk          (0x1UL << RCC_AHB1SMENR_SRAM1SMEN_Pos) /*!< 0x00000200 */
#define RCC_AHB1SMENR_SRAM1SMEN              RCC_AHB1SMENR_SRAM1SMEN_Msk
#define RCC_AHB1SMENR_CRCSMEN_Pos            (12U)
#define RCC_AHB1SMENR_CRCSMEN_Msk            (0x1UL << RCC_AHB1SMENR_CRCSMEN_Pos) /*!< 0x00001000 */
#define RCC_AHB1SMENR_CRCSMEN                RCC_AHB1SMENR_CRCSMEN_Msk
#define RCC_AHB1SMENR_TSCSMEN_Pos            (16U)
#define RCC_AHB1SMENR_TSCSMEN_Msk            (0x1UL << RCC_AHB1SMENR_TSCSMEN_Pos) /*!< 0x00010000 */
#define RCC_AHB1SMENR_TSCSMEN                RCC_AHB1SMENR_TSCSMEN_Msk

/********************  Bit definition for RCC_AHB2SMENR register  *************/
#define RCC_AHB2SMENR_GPIOASMEN_Pos          (0U)
#define RCC_AHB2SMENR_GPIOASMEN_Msk          (0x1UL << RCC_AHB2SMENR_GPIOASMEN_Pos) /*!< 0x00000001 */
#define RCC_AHB2SMENR_GPIOASMEN              RCC_AHB2SMENR_GPIOASMEN_Msk
#define RCC_AHB2SMENR_GPIOBSMEN_Pos          (1U)
#define RCC_AHB2SMENR_GPIOBSMEN_Msk          (0x1UL << RCC_AHB2SMENR_GPIOBSMEN_Pos) /*!< 0x00000002 */
#define RCC_AHB2SMENR_GPIOBSMEN              RCC_AHB2SMENR_GPIOBSMEN_Msk
#define RCC_AHB2SMENR_GPIOCSMEN_Pos          (2U)
#define RCC_AHB2SMENR_GPIOCSMEN_Msk          (0x1UL << RCC_AHB2SMENR_GPIOCSMEN_Pos) /*!< 0x00000004 */
#define RCC_AHB2SMENR_GPIOCSMEN              RCC_AHB2SMENR_GPIOCSMEN_Msk
#define RCC_AHB2SMENR_GPIODSMEN_Pos          (3U)
#define RCC_AHB2SMENR_GPIODSMEN_Msk          (0x1UL << RCC_AHB2SMENR_GPIODSMEN_Pos) /*!< 0x00000008 */
#define RCC_AHB2SMENR_GPIODSMEN              RCC_AHB2SMENR_GPIODSMEN_Msk
#define RCC_AHB2SMENR_GPIOHSMEN_Pos          (7U)
#define RCC_AHB2SMENR_GPIOHSMEN_Msk          (0x1UL << RCC_AHB2SMENR_GPIOHSMEN_Pos) /*!< 0x00000080 */
#define RCC_AHB2SMENR_GPIOHSMEN              RCC_AHB2SMENR_GPIOHSMEN_Msk
#define RCC_AHB2SMENR_SRAM2SMEN_Pos          (9U)
#define RCC_AHB2SMENR_SRAM2SMEN_Msk          (0x1UL << RCC_AHB2SMENR_SRAM2SMEN_Pos) /*!< 0x00000200 */
#define RCC_AHB2SMENR_SRAM2SMEN              RCC_AHB2SMENR_SRAM2SMEN_Msk
#define RCC_AHB2SMENR_ADCSMEN_Pos            (13U)
#define RCC_AHB2SMENR_ADCSMEN_Msk            (0x1UL << RCC_AHB2SMENR_ADCSMEN_Pos) /*!< 0x00002000 */
#define RCC_AHB2SMENR_ADCSMEN                RCC_AHB2SMENR_ADCSMEN_Msk
#define RCC_AHB2SMENR_AESSMEN_Pos            (16U)
#define RCC_AHB2SMENR_AESSMEN_Msk            (0x1UL << RCC_AHB2SMENR_AESSMEN_Pos) /*!< 0x00010000 */
#define RCC_AHB2SMENR_AESSMEN                RCC_AHB2SMENR_AESSMEN_Msk
#define RCC_AHB2SMENR_RNGSMEN_Pos            (18U)
#define RCC_AHB2SMENR_RNGSMEN_Msk            (0x1UL << RCC_AHB2SMENR_RNGSMEN_Pos) /*!< 0x00040000 */
#define RCC_AHB2SMENR_RNGSMEN                RCC_AHB2SMENR_RNGSMEN_Msk

/********************  Bit definition for RCC_AHB3SMENR register  *************/
#define RCC_AHB3SMENR_QSPISMEN_Pos           (8U)
#define RCC_AHB3SMENR_QSPISMEN_Msk           (0x1UL << RCC_AHB3SMENR_QSPISMEN_Pos) /*!< 0x00000100 */
#define RCC_AHB3SMENR_QSPISMEN               RCC_AHB3SMENR_QSPISMEN_Msk

/********************  Bit definition for RCC_APB1SMENR1 register  *************/
#define RCC_APB1SMENR1_TIM2SMEN_Pos          (0U)
#define RCC_APB1SMENR1_TIM2SMEN_Msk          (0x1UL << RCC_APB1SMENR1_TIM2SMEN_Pos) /*!< 0x00000001 */
#define RCC_APB1SMENR1_TIM2SMEN              RCC_APB1SMENR1_TIM2SMEN_Msk
#define RCC_APB1SMENR1_TIM6SMEN_Pos          (4U)
#define RCC_APB1SMENR1_TIM6SMEN_Msk          (0x1UL << RCC_APB1SMENR1_TIM6SMEN_Pos) /*!< 0x00000010 */
#define RCC_APB1SMENR1_TIM6SMEN              RCC_APB1SMENR1_TIM6SMEN_Msk
#define RCC_APB1SMENR1_RTCAPBSMEN_Pos        (10U)
#define RCC_APB1SMENR1_RTCAPBSMEN_Msk        (0x1UL << RCC_APB1SMENR1_RTCAPBSMEN_Pos) /*!< 0x00000400 */
#define RCC_APB1SMENR1_RTCAPBSMEN            RCC_APB1SMENR1_RTCAPBSMEN_Msk
#define RCC_APB1SMENR1_WWDGSMEN_Pos          (11U)
#define RCC_APB1SMENR1_WWDGSMEN_Msk          (0x1UL << RCC_APB1SMENR1_WWDGSMEN_Pos) /*!< 0x00000800 */
#define RCC_APB1SMENR1_WWDGSMEN              RCC_APB1SMENR1_WWDGSMEN_Msk
#define RCC_APB1SMENR1_SPI2SMEN_Pos          (14U)
#define RCC_APB1SMENR1_SPI2SMEN_Msk          (0x1UL << RCC_APB1SMENR1_SPI2SMEN_Pos) /*!< 0x00004000 */
#define RCC_APB1SMENR1_SPI2SMEN              RCC_APB1SMENR1_SPI2SMEN_Msk
#define RCC_APB1SMENR1_USART2SMEN_Pos        (17U)
#define RCC_APB1SMENR1_USART2SMEN_Msk        (0x1UL << RCC_APB1SMENR1_USART2SMEN_Pos) /*!< 0x00020000 */
#define RCC_APB1SMENR1_USART2SMEN            RCC_APB1SMENR1_USART2SMEN_Msk
#define RCC_APB1SMENR1_USART3SMEN_Pos        (18U)
#define RCC_APB1SMENR1_USART3SMEN_Msk        (0x1UL << RCC_APB1SMENR1_USART3SMEN_Pos) /*!< 0x00040000 */
#define RCC_APB1SMENR1_USART3SMEN            RCC_APB1SMENR1_USART3SMEN_Msk
#define RCC_APB1SMENR1_I2C1SMEN_Pos          (21U)
#define RCC_APB1SMENR1_I2C1SMEN_Msk          (0x1UL << RCC_APB1SMENR1_I2C1SMEN_Pos) /*!< 0x00200000 */
#define RCC_APB1SMENR1_I2C1SMEN              RCC_APB1SMENR1_I2C1SMEN_Msk
#define RCC_APB1SMENR1_I2C2SMEN_Pos          (22U)
#define RCC_APB1SMENR1_I2C2SMEN_Msk          (0x1UL << RCC_APB1SMENR1_I2C2SMEN_Pos) /*!< 0x00400000 */
#define RCC_APB1SMENR1_I2C2SMEN              RCC_APB1SMENR1_I2C2SMEN_Msk
#define RCC_APB1SMENR1_I2C3SMEN_Pos          (23U)
#define RCC_APB1SMENR1_I2C3SMEN_Msk          (0x1UL << RCC_APB1SMENR1_I2C3SMEN_Pos) /*!< 0x00800000 */
#define RCC_APB1SMENR1_I2C3SMEN              RCC_APB1SMENR1_I2C3SMEN_Msk
#define RCC_APB1SMENR1_CRSSMEN_Pos           (24U)
#define RCC_APB1SMENR1_CRSSMEN_Msk           (0x1UL << RCC_APB1SMENR1_CRSSMEN_Pos) /*!< 0x01000000 */
#define RCC_APB1SMENR1_CRSSMEN               RCC_APB1SMENR1_CRSSMEN_Msk
#define RCC_APB1SMENR1_USBFSSMEN_Pos         (26U)
#define RCC_APB1SMENR1_USBFSSMEN_Msk         (0x1UL << RCC_APB1SMENR1_USBFSSMEN_Pos) /*!< 0x04000000 */
#define RCC_APB1SMENR1_USBFSSMEN             RCC_APB1SMENR1_USBFSSMEN_Msk
#define RCC_APB1SMENR1_PWRSMEN_Pos           (28U)
#define RCC_APB1SMENR1_PWRSMEN_Msk           (0x1UL << RCC_APB1SMENR1_PWRSMEN_Pos) /*!< 0x10000000 */
#define RCC_APB1SMENR1_PWRSMEN               RCC_APB1SMENR1_PWRSMEN_Msk
#define RCC_APB1SMENR1_OPAMPSMEN_Pos         (30U)
#define RCC_APB1SMENR1_OPAMPSMEN_Msk         (0x1UL << RCC_APB1SMENR1_OPAMPSMEN_Pos) /*!< 0x40000000 */
#define RCC_APB1SMENR1_OPAMPSMEN             RCC_APB1SMENR1_OPAMPSMEN_Msk
#define RCC_APB1SMENR1_LPTIM1SMEN_Pos        (31U)
#define RCC_APB1SMENR1_LPTIM1SMEN_Msk        (0x1UL << RCC_APB1SMENR1_LPTIM1SMEN_Pos) /*!< 0x80000000 */
#define RCC_APB1SMENR1_LPTIM1SMEN            RCC_APB1SMENR1_LPTIM1SMEN_Msk

/********************  Bit definition for RCC_APB1SMENR2 register  *************/
#define RCC_APB1SMENR2_LPUART1SMEN_Pos       (0U)
#define RCC_APB1SMENR2_LPUART1SMEN_Msk       (0x1UL << RCC_APB1SMENR2_LPUART1SMEN_Pos) /*!< 0x00000001 */
#define RCC_APB1SMENR2_LPUART1SMEN           RCC_APB1SMENR2_LPUART1SMEN_Msk
#define RCC_APB1SMENR2_LPTIM2SMEN_Pos        (5U)
#define RCC_APB1SMENR2_LPTIM2SMEN_Msk        (0x1UL << RCC_APB1SMENR2_LPTIM2SMEN_Pos) /*!< 0x00000020 */
#define RCC_APB1SMENR2_LPTIM2SMEN            RCC_APB1SMENR2_LPTIM2SMEN_Msk

/********************  Bit definition for RCC_APB2SMENR register  *************/
#define RCC_APB2SMENR_SYSCFGSMEN_Pos         (0U)
#define RCC_APB2SMENR_SYSCFGSMEN_Msk         (0x1UL << RCC_APB2SMENR_SYSCFGSMEN_Pos) /*!< 0x00000001 */
#define RCC_APB2SMENR_SYSCFGSMEN             RCC_APB2SMENR_SYSCFGSMEN_Msk
#define RCC_APB2SMENR_TIM1SMEN_Pos           (11U)
#define RCC_APB2SMENR_TIM1SMEN_Msk           (0x1UL << RCC_APB2SMENR_TIM1SMEN_Pos) /*!< 0x00000800 */
#define RCC_APB2SMENR_TIM1SMEN               RCC_APB2SMENR_TIM1SMEN_Msk
#define RCC_APB2SMENR_SPI1SMEN_Pos           (12U)
#define RCC_APB2SMENR_SPI1SMEN_Msk           (0x1UL << RCC_APB2SMENR_SPI1SMEN_Pos) /*!< 0x00001000 */
#define RCC_APB2SMENR_SPI1SMEN               RCC_APB2SMENR_SPI1SMEN_Msk
#define RCC_APB2SMENR_USART1SMEN_Pos         (14U)
#define RCC_APB2SMENR_USART1SMEN_Msk         (0x1UL << RCC_APB2SMENR_USART1SMEN_Pos) /*!< 0x00004000 */
#define RCC_APB2SMENR_USART1SMEN             RCC_APB2SMENR_USART1SMEN_Msk
#define RCC_APB2SMENR_TIM15SMEN_Pos          (16U)
#define RCC_APB2SMENR_TIM15SMEN_Msk          (0x1UL << RCC_APB2SMENR_TIM15SMEN_Pos) /*!< 0x00010000 */
#define RCC_APB2SMENR_TIM15SMEN              RCC_APB2SMENR_TIM15SMEN_Msk
#define RCC_APB2SMENR_TIM16SMEN_Pos          (17U)
#define RCC_APB2SMENR_TIM16SMEN_Msk          (0x1UL << RCC_APB2SMENR_TIM16SMEN_Pos) /*!< 0x00020000 */
#define RCC_APB2SMENR_TIM16SMEN              RCC_APB2SMENR_TIM16SMEN_Msk

/********************  Bit definition for RCC_CCIPR register  ******************/
#define RCC_CCIPR_USART1SEL_Pos              (0U)
#define RCC_CCIPR_USART1SEL_Msk              (0x3UL << RCC_CCIPR_USART1SEL_Pos) /*!< 0x00000003 */
#define RCC_CCIPR_USART1SEL                  RCC_CCIPR_USART1SEL_Msk
#define RCC_CCIPR_USART1SEL_0                (0x1UL << RCC_CCIPR_USART1SEL_Pos) /*!< 0x00000001 */
#define RCC_CCIPR_USART1SEL_1                (0x2UL << RCC_CCIPR_USART1SEL_Pos) /*!< 0x00000002 */

#define RCC_CCIPR_USART2SEL_Pos              (2U)
#define RCC_CCIPR_USART2SEL_Msk              (0x3UL << RCC_CCIPR_USART2SEL_Pos) /*!< 0x0000000C */
#define RCC_CCIPR_USART2SEL                  RCC_CCIPR_USART2SEL_Msk
#define RCC_CCIPR_USART2SEL_0                (0x1UL << RCC_CCIPR_USART2SEL_Pos) /*!< 0x00000004 */
#define RCC_CCIPR_USART2SEL_1                (0x2UL << RCC_CCIPR_USART2SEL_Pos) /*!< 0x00000008 */

#define RCC_CCIPR_USART3SEL_Pos              (4U)
#define RCC_CCIPR_USART3SEL_Msk              (0x3UL << RCC_CCIPR_USART3SEL_Pos) /*!< 0x00000030 */
#define RCC_CCIPR_USART3SEL                  RCC_CCIPR_USART3SEL_Msk
#define RCC_CCIPR_USART3SEL_0                (0x1UL << RCC_CCIPR_USART3SEL_Pos) /*!< 0x00000010 */
#define RCC_CCIPR_USART3SEL_1                (0x2UL << RCC_CCIPR_USART3SEL_Pos) /*!< 0x00000020 */

#define RCC_CCIPR_LPUART1SEL_Pos             (10U)
#define RCC_CCIPR_LPUART1SEL_Msk             (0x3UL << RCC_CCIPR_LPUART1SEL_Pos) /*!< 0x00000C00 */
#define RCC_CCIPR_LPUART1SEL                 RCC_CCIPR_LPUART1SEL_Msk
#define RCC_CCIPR_LPUART1SEL_0               (0x1UL << RCC_CCIPR_LPUART1SEL_Pos) /*!< 0x00000400 */
#define RCC_CCIPR_LPUART1SEL_1               (0x2UL << RCC_CCIPR_LPUART1SEL_Pos) /*!< 0x00000800 */

#define RCC_CCIPR_I2C1SEL_Pos                (12U)
#define RCC_CCIPR_I2C1SEL_Msk                (0x3UL << RCC_CCIPR_I2C1SEL_Pos)  /*!< 0x00003000 */
#define RCC_CCIPR_I2C1SEL                    RCC_CCIPR_I2C1SEL_Msk
#define RCC_CCIPR_I2C1SEL_0                  (0x1UL << RCC_CCIPR_I2C1SEL_Pos)  /*!< 0x00001000 */
#define RCC_CCIPR_I2C1SEL_1                  (0x2UL << RCC_CCIPR_I2C1SEL_Pos)  /*!< 0x00002000 */

#define RCC_CCIPR_I2C2SEL_Pos                (14U)
#define RCC_CCIPR_I2C2SEL_Msk                (0x3UL << RCC_CCIPR_I2C2SEL_Pos)  /*!< 0x0000C000 */
#define RCC_CCIPR_I2C2SEL                    RCC_CCIPR_I2C2SEL_Msk
#define RCC_CCIPR_I2C2SEL_0                  (0x1UL << RCC_CCIPR_I2C2SEL_Pos)  /*!< 0x00004000 */
#define RCC_CCIPR_I2C2SEL_1                  (0x2UL << RCC_CCIPR_I2C2SEL_Pos)  /*!< 0x00008000 */

#define RCC_CCIPR_I2C3SEL_Pos                (16U)
#define RCC_CCIPR_I2C3SEL_Msk                (0x3UL << RCC_CCIPR_I2C3SEL_Pos)  /*!< 0x00030000 */
#define RCC_CCIPR_I2C3SEL                    RCC_CCIPR_I2C3SEL_Msk
#define RCC_CCIPR_I2C3SEL_0                  (0x1UL << RCC_CCIPR_I2C3SEL_Pos)  /*!< 0x00010000 */
#define RCC_CCIPR_I2C3SEL_1                  (0x2UL << RCC_CCIPR_I2C3SEL_Pos)  /*!< 0x00020000 */

#define RCC_CCIPR_LPTIM1SEL_Pos              (18U)
#define RCC_CCIPR_LPTIM1SEL_Msk              (0x3UL << RCC_CCIPR_LPTIM1SEL_Pos) /*!< 0x000C0000 */
#define RCC_CCIPR_LPTIM1SEL                  RCC_CCIPR_LPTIM1SEL_Msk
#define RCC_CCIPR_LPTIM1SEL_0                (0x1UL << RCC_CCIPR_LPTIM1SEL_Pos) /*!< 0x00040000 */
#define RCC_CCIPR_LPTIM1SEL_1                (0x2UL << RCC_CCIPR_LPTIM1SEL_Pos) /*!< 0x00080000 */

#define RCC_CCIPR_LPTIM2SEL_Pos              (20U)
#define RCC_CCIPR_LPTIM2SEL_Msk              (0x3UL << RCC_CCIPR_LPTIM2SEL_Pos) /*!< 0x00300000 */
#define RCC_CCIPR_LPTIM2SEL                  RCC_CCIPR_LPTIM2SEL_Msk
#define RCC_CCIPR_LPTIM2SEL_0                (0x1UL << RCC_CCIPR_LPTIM2SEL_Pos) /*!< 0x00100000 */
#define RCC_CCIPR_LPTIM2SEL_1                (0x2UL << RCC_CCIPR_LPTIM2SEL_Pos) /*!< 0x00200000 */

#define RCC_CCIPR_CLK48SEL_Pos               (26U)
#define RCC_CCIPR_CLK48SEL_Msk               (0x3UL << RCC_CCIPR_CLK48SEL_Pos) /*!< 0x0C000000 */
#define RCC_CCIPR_CLK48SEL                   RCC_CCIPR_CLK48SEL_Msk
#define RCC_CCIPR_CLK48SEL_0                 (0x1UL << RCC_CCIPR_CLK48SEL_Pos) /*!< 0x04000000 */
#define RCC_CCIPR_CLK48SEL_1                 (0x2UL << RCC_CCIPR_CLK48SEL_Pos) /*!< 0x08000000 */


/********************  Bit definition for RCC_BDCR register  ******************/
#define RCC_BDCR_LSEON_Pos                   (0U)
#define RCC_BDCR_LSEON_Msk                   (0x1UL << RCC_BDCR_LSEON_Pos)     /*!< 0x00000001 */
#define RCC_BDCR_LSEON                       RCC_BDCR_LSEON_Msk
#define RCC_BDCR_LSERDY_Pos                  (1U)
#define RCC_BDCR_LSERDY_Msk                  (0x1UL << RCC_BDCR_LSERDY_Pos)    /*!< 0x00000002 */
#define RCC_BDCR_LSERDY                      RCC_BDCR_LSERDY_Msk
#define RCC_BDCR_LSEBYP_Pos                  (2U)
#define RCC_BDCR_LSEBYP_Msk                  (0x1UL << RCC_BDCR_LSEBYP_Pos)    /*!< 0x00000004 */
#define RCC_BDCR_LSEBYP                      RCC_BDCR_LSEBYP_Msk

#define RCC_BDCR_LSEDRV_Pos                  (3U)
#define RCC_BDCR_LSEDRV_Msk                  (0x3UL << RCC_BDCR_LSEDRV_Pos)    /*!< 0x00000018 */
#define RCC_BDCR_LSEDRV                      RCC_BDCR_LSEDRV_Msk
#define RCC_BDCR_LSEDRV_0                    (0x1UL << RCC_BDCR_LSEDRV_Pos)    /*!< 0x00000008 */
#define RCC_BDCR_LSEDRV_1                    (0x2UL << RCC_BDCR_LSEDRV_Pos)    /*!< 0x00000010 */

#define RCC_BDCR_LSECSSON_Pos                (5U)
#define RCC_BDCR_LSECSSON_Msk                (0x1UL << RCC_BDCR_LSECSSON_Pos)  /*!< 0x00000020 */
#define RCC_BDCR_LSECSSON                    RCC_BDCR_LSECSSON_Msk
#define RCC_BDCR_LSECSSD_Pos                 (6U)
#define RCC_BDCR_LSECSSD_Msk                 (0x1UL << RCC_BDCR_LSECSSD_Pos)   /*!< 0x00000040 */
#define RCC_BDCR_LSECSSD                     RCC_BDCR_LSECSSD_Msk

#define RCC_BDCR_LSESYSDIS_Pos               (7U)
#define RCC_BDCR_LSESYSDIS_Msk               (0x1UL << RCC_BDCR_LSESYSDIS_Pos) /*!< 0x00000080 */
#define RCC_BDCR_LSESYSDIS                   RCC_BDCR_LSESYSDIS_Msk

#define RCC_BDCR_RTCSEL_Pos                  (8U)
#define RCC_BDCR_RTCSEL_Msk                  (0x3UL << RCC_BDCR_RTCSEL_Pos)    /*!< 0x00000300 */
#define RCC_BDCR_RTCSEL                      RCC_BDCR_RTCSEL_Msk
#define RCC_BDCR_RTCSEL_0                    (0x1UL << RCC_BDCR_RTCSEL_Pos)    /*!< 0x00000100 */
#define RCC_BDCR_RTCSEL_1                    (0x2UL << RCC_BDCR_RTCSEL_Pos)    /*!< 0x00000200 */

#define RCC_BDCR_RTCEN_Pos                   (15U)
#define RCC_BDCR_RTCEN_Msk                   (0x1UL << RCC_BDCR_RTCEN_Pos)     /*!< 0x00008000 */
#define RCC_BDCR_RTCEN                       RCC_BDCR_RTCEN_Msk
#define RCC_BDCR_BDRST_Pos                   (16U)
#define RCC_BDCR_BDRST_Msk                   (0x1UL << RCC_BDCR_BDRST_Pos)     /*!< 0x00010000 */
#define RCC_BDCR_BDRST                       RCC_BDCR_BDRST_Msk
#define RCC_BDCR_LSCOEN_Pos                  (24U)
#define RCC_BDCR_LSCOEN_Msk                  (0x1UL << RCC_BDCR_LSCOEN_Pos)    /*!< 0x01000000 */
#define RCC_BDCR_LSCOEN                      RCC_BDCR_LSCOEN_Msk
#define RCC_BDCR_LSCOSEL_Pos                 (25U)
#define RCC_BDCR_LSCOSEL_Msk                 (0x1UL << RCC_BDCR_LSCOSEL_Pos)   /*!< 0x02000000 */
#define RCC_BDCR_LSCOSEL                     RCC_BDCR_LSCOSEL_Msk

/********************  Bit definition for RCC_CSR register  *******************/
#define RCC_CSR_LSION_Pos                    (0U)
#define RCC_CSR_LSION_Msk                    (0x1UL << RCC_CSR_LSION_Pos)      /*!< 0x00000001 */
#define RCC_CSR_LSION                        RCC_CSR_LSION_Msk
#define RCC_CSR_LSIRDY_Pos                   (1U)
#define RCC_CSR_LSIRDY_Msk                   (0x1UL << RCC_CSR_LSIRDY_Pos)     /*!< 0x00000002 */
#define RCC_CSR_LSIRDY                       RCC_CSR_LSIRDY_Msk

#define RCC_CSR_LSIPREDIV_Pos                (4U)
#define RCC_CSR_LSIPREDIV_Msk                (0x1UL << RCC_CSR_LSIPREDIV_Pos)  /*!< 0x00000010 */
#define RCC_CSR_LSIPREDIV                    RCC_CSR_LSIPREDIV_Msk

#define RCC_CSR_MSISRANGE_Pos                (8U)
#define RCC_CSR_MSISRANGE_Msk                (0xFUL << RCC_CSR_MSISRANGE_Pos)  /*!< 0x00000F00 */
#define RCC_CSR_MSISRANGE                    RCC_CSR_MSISRANGE_Msk
#define RCC_CSR_MSISRANGE_1                  (0x4UL << RCC_CSR_MSISRANGE_Pos)  /*!< 0x00000400 */
#define RCC_CSR_MSISRANGE_2                  (0x5UL << RCC_CSR_MSISRANGE_Pos)  /*!< 0x00000500 */
#define RCC_CSR_MSISRANGE_4                  (0x6UL << RCC_CSR_MSISRANGE_Pos)  /*!< 0x00000600 */
#define RCC_CSR_MSISRANGE_8                  (0x7UL << RCC_CSR_MSISRANGE_Pos)  /*!< 0x00000700 */

#define RCC_CSR_RMVF_Pos                     (23U)
#define RCC_CSR_RMVF_Msk                     (0x1UL << RCC_CSR_RMVF_Pos)       /*!< 0x00800000 */
#define RCC_CSR_RMVF                         RCC_CSR_RMVF_Msk
#define RCC_CSR_FWRSTF_Pos                   (24U)
#define RCC_CSR_FWRSTF_Msk                   (0x1UL << RCC_CSR_FWRSTF_Pos)     /*!< 0x01000000 */
#define RCC_CSR_FWRSTF                       RCC_CSR_FWRSTF_Msk
#define RCC_CSR_OBLRSTF_Pos                  (25U)
#define RCC_CSR_OBLRSTF_Msk                  (0x1UL << RCC_CSR_OBLRSTF_Pos)    /*!< 0x02000000 */
#define RCC_CSR_OBLRSTF                      RCC_CSR_OBLRSTF_Msk
#define RCC_CSR_PINRSTF_Pos                  (26U)
#define RCC_CSR_PINRSTF_Msk                  (0x1UL << RCC_CSR_PINRSTF_Pos)    /*!< 0x04000000 */
#define RCC_CSR_PINRSTF                      RCC_CSR_PINRSTF_Msk
#define RCC_CSR_BORRSTF_Pos                  (27U)
#define RCC_CSR_BORRSTF_Msk                  (0x1UL << RCC_CSR_BORRSTF_Pos)    /*!< 0x08000000 */
#define RCC_CSR_BORRSTF                      RCC_CSR_BORRSTF_Msk
#define RCC_CSR_SFTRSTF_Pos                  (28U)
#define RCC_CSR_SFTRSTF_Msk                  (0x1UL << RCC_CSR_SFTRSTF_Pos)    /*!< 0x10000000 */
#define RCC_CSR_SFTRSTF                      RCC_CSR_SFTRSTF_Msk
#define RCC_CSR_IWDGRSTF_Pos                 (29U)
#define RCC_CSR_IWDGRSTF_Msk                 (0x1UL << RCC_CSR_IWDGRSTF_Pos)   /*!< 0x20000000 */
#define RCC_CSR_IWDGRSTF                     RCC_CSR_IWDGRSTF_Msk
#define RCC_CSR_WWDGRSTF_Pos                 (30U)
#define RCC_CSR_WWDGRSTF_Msk                 (0x1UL << RCC_CSR_WWDGRSTF_Pos)   /*!< 0x40000000 */
#define RCC_CSR_WWDGRSTF                     RCC_CSR_WWDGRSTF_Msk
#define RCC_CSR_LPWRRSTF_Pos                 (31U)
#define RCC_CSR_LPWRRSTF_Msk                 (0x1UL << RCC_CSR_LPWRRSTF_Pos)   /*!< 0x80000000 */
#define RCC_CSR_LPWRRSTF                     RCC_CSR_LPWRRSTF_Msk

/********************  Bit definition for RCC_CRRCR register  *****************/
#define RCC_CRRCR_HSI48ON_Pos                (0U)
#define RCC_CRRCR_HSI48ON_Msk                (0x1UL << RCC_CRRCR_HSI48ON_Pos)  /*!< 0x00000001 */
#define RCC_CRRCR_HSI48ON                    RCC_CRRCR_HSI48ON_Msk
#define RCC_CRRCR_HSI48RDY_Pos               (1U)
#define RCC_CRRCR_HSI48RDY_Msk               (0x1UL << RCC_CRRCR_HSI48RDY_Pos) /*!< 0x00000002 */
#define RCC_CRRCR_HSI48RDY                   RCC_CRRCR_HSI48RDY_Msk

/*!< HSI48CAL configuration */
#define RCC_CRRCR_HSI48CAL_Pos               (7U)
#define RCC_CRRCR_HSI48CAL_Msk               (0x1FFUL << RCC_CRRCR_HSI48CAL_Pos) /*!< 0x0000FF80 */
#define RCC_CRRCR_HSI48CAL                   RCC_CRRCR_HSI48CAL_Msk             /*!< HSI48CAL[8:0] bits */
#define RCC_CRRCR_HSI48CAL_0                 (0x001UL << RCC_CRRCR_HSI48CAL_Pos) /*!< 0x00000080 */
#define RCC_CRRCR_HSI48CAL_1                 (0x002UL << RCC_CRRCR_HSI48CAL_Pos) /*!< 0x00000100 */
#define RCC_CRRCR_HSI48CAL_2                 (0x004UL << RCC_CRRCR_HSI48CAL_Pos) /*!< 0x00000200 */
#define RCC_CRRCR_HSI48CAL_3                 (0x008UL << RCC_CRRCR_HSI48CAL_Pos) /*!< 0x00000400 */
#define RCC_CRRCR_HSI48CAL_4                 (0x010UL << RCC_CRRCR_HSI48CAL_Pos) /*!< 0x00000800 */
#define RCC_CRRCR_HSI48CAL_5                 (0x020UL << RCC_CRRCR_HSI48CAL_Pos) /*!< 0x00001000 */
#define RCC_CRRCR_HSI48CAL_6                 (0x040UL << RCC_CRRCR_HSI48CAL_Pos) /*!< 0x00002000 */
#define RCC_CRRCR_HSI48CAL_7                 (0x080UL << RCC_CRRCR_HSI48CAL_Pos) /*!< 0x00004000 */
#define RCC_CRRCR_HSI48CAL_8                 (0x100UL << RCC_CRRCR_HSI48CAL_Pos) /*!< 0x00008000 */

/******************************************************************************/
/*                                                                            */
/*                                    RNG                                     */
/*                                                                            */
/******************************************************************************/
/********************  Bits definition for RNG_CR register  *******************/
#define RNG_CR_RNGEN_Pos    (2U)
#define RNG_CR_RNGEN_Msk    (0x1UL << RNG_CR_RNGEN_Pos)                        /*!< 0x00000004 */
#define RNG_CR_RNGEN        RNG_CR_RNGEN_Msk
#define RNG_CR_IE_Pos       (3U)
#define RNG_CR_IE_Msk       (0x1UL << RNG_CR_IE_Pos)                           /*!< 0x00000008 */
#define RNG_CR_IE           RNG_CR_IE_Msk

/********************  Bits definition for RNG_SR register  *******************/
#define RNG_SR_DRDY_Pos     (0U)
#define RNG_SR_DRDY_Msk     (0x1UL << RNG_SR_DRDY_Pos)                         /*!< 0x00000001 */
#define RNG_SR_DRDY         RNG_SR_DRDY_Msk
#define RNG_SR_CECS_Pos     (1U)
#define RNG_SR_CECS_Msk     (0x1UL << RNG_SR_CECS_Pos)                         /*!< 0x00000002 */
#define RNG_SR_CECS         RNG_SR_CECS_Msk
#define RNG_SR_SECS_Pos     (2U)
#define RNG_SR_SECS_Msk     (0x1UL << RNG_SR_SECS_Pos)                         /*!< 0x00000004 */
#define RNG_SR_SECS         RNG_SR_SECS_Msk
#define RNG_SR_CEIS_Pos     (5U)
#define RNG_SR_CEIS_Msk     (0x1UL << RNG_SR_CEIS_Pos)                         /*!< 0x00000020 */
#define RNG_SR_CEIS         RNG_SR_CEIS_Msk
#define RNG_SR_SEIS_Pos     (6U)
#define RNG_SR_SEIS_Msk     (0x1UL << RNG_SR_SEIS_Pos)                         /*!< 0x00000040 */
#define RNG_SR_SEIS         RNG_SR_SEIS_Msk

/******************************************************************************/
/*                                                                            */
/*                           Real-Time Clock (RTC)                            */
/*                                                                            */
/******************************************************************************/
/*
* @brief Specific device feature definitions
*/
#define RTC_TAMPER1_SUPPORT
#define RTC_TAMPER2_SUPPORT
#define RTC_WAKEUP_SUPPORT
#define RTC_BACKUP_SUPPORT

/********************  Bits definition for RTC_TR register  *******************/
#define RTC_TR_PM_Pos                (22U)
#define RTC_TR_PM_Msk                (0x1UL << RTC_TR_PM_Pos)                   /*!< 0x00400000 */
#define RTC_TR_PM                    RTC_TR_PM_Msk
#define RTC_TR_HT_Pos                (20U)
#define RTC_TR_HT_Msk                (0x3UL << RTC_TR_HT_Pos)                   /*!< 0x00300000 */
#define RTC_TR_HT                    RTC_TR_HT_Msk
#define RTC_TR_HT_0                  (0x1UL << RTC_TR_HT_Pos)                   /*!< 0x00100000 */
#define RTC_TR_HT_1                  (0x2UL << RTC_TR_HT_Pos)                   /*!< 0x00200000 */
#define RTC_TR_HU_Pos                (16U)
#define RTC_TR_HU_Msk                (0xFUL << RTC_TR_HU_Pos)                   /*!< 0x000F0000 */
#define RTC_TR_HU                    RTC_TR_HU_Msk
#define RTC_TR_HU_0                  (0x1UL << RTC_TR_HU_Pos)                   /*!< 0x00010000 */
#define RTC_TR_HU_1                  (0x2UL << RTC_TR_HU_Pos)                   /*!< 0x00020000 */
#define RTC_TR_HU_2                  (0x4UL << RTC_TR_HU_Pos)                   /*!< 0x00040000 */
#define RTC_TR_HU_3                  (0x8UL << RTC_TR_HU_Pos)                   /*!< 0x00080000 */
#define RTC_TR_MNT_Pos               (12U)
#define RTC_TR_MNT_Msk               (0x7UL << RTC_TR_MNT_Pos)                  /*!< 0x00007000 */
#define RTC_TR_MNT                   RTC_TR_MNT_Msk
#define RTC_TR_MNT_0                 (0x1UL << RTC_TR_MNT_Pos)                  /*!< 0x00001000 */
#define RTC_TR_MNT_1                 (0x2UL << RTC_TR_MNT_Pos)                  /*!< 0x00002000 */
#define RTC_TR_MNT_2                 (0x4UL << RTC_TR_MNT_Pos)                  /*!< 0x00004000 */
#define RTC_TR_MNU_Pos               (8U)
#define RTC_TR_MNU_Msk               (0xFUL << RTC_TR_MNU_Pos)                  /*!< 0x00000F00 */
#define RTC_TR_MNU                   RTC_TR_MNU_Msk
#define RTC_TR_MNU_0                 (0x1UL << RTC_TR_MNU_Pos)                  /*!< 0x00000100 */
#define RTC_TR_MNU_1                 (0x2UL << RTC_TR_MNU_Pos)                  /*!< 0x00000200 */
#define RTC_TR_MNU_2                 (0x4UL << RTC_TR_MNU_Pos)                  /*!< 0x00000400 */
#define RTC_TR_MNU_3                 (0x8UL << RTC_TR_MNU_Pos)                  /*!< 0x00000800 */
#define RTC_TR_ST_Pos                (4U)
#define RTC_TR_ST_Msk                (0x7UL << RTC_TR_ST_Pos)                   /*!< 0x00000070 */
#define RTC_TR_ST                    RTC_TR_ST_Msk
#define RTC_TR_ST_0                  (0x1UL << RTC_TR_ST_Pos)                   /*!< 0x00000010 */
#define RTC_TR_ST_1                  (0x2UL << RTC_TR_ST_Pos)                   /*!< 0x00000020 */
#define RTC_TR_ST_2                  (0x4UL << RTC_TR_ST_Pos)                   /*!< 0x00000040 */
#define RTC_TR_SU_Pos                (0U)
#define RTC_TR_SU_Msk                (0xFUL << RTC_TR_SU_Pos)                   /*!< 0x0000000F */
#define RTC_TR_SU                    RTC_TR_SU_Msk
#define RTC_TR_SU_0                  (0x1UL << RTC_TR_SU_Pos)                   /*!< 0x00000001 */
#define RTC_TR_SU_1                  (0x2UL << RTC_TR_SU_Pos)                   /*!< 0x00000002 */
#define RTC_TR_SU_2                  (0x4UL << RTC_TR_SU_Pos)                   /*!< 0x00000004 */
#define RTC_TR_SU_3                  (0x8UL << RTC_TR_SU_Pos)                   /*!< 0x00000008 */

/********************  Bits definition for RTC_DR register  *******************/
#define RTC_DR_YT_Pos                (20U)
#define RTC_DR_YT_Msk                (0xFUL << RTC_DR_YT_Pos)                   /*!< 0x00F00000 */
#define RTC_DR_YT                    RTC_DR_YT_Msk
#define RTC_DR_YT_0                  (0x1UL << RTC_DR_YT_Pos)                   /*!< 0x00100000 */
#define RTC_DR_YT_1                  (0x2UL << RTC_DR_YT_Pos)                   /*!< 0x00200000 */
#define RTC_DR_YT_2                  (0x4UL << RTC_DR_YT_Pos)                   /*!< 0x00400000 */
#define RTC_DR_YT_3                  (0x8UL << RTC_DR_YT_Pos)                   /*!< 0x00800000 */
#define RTC_DR_YU_Pos                (16U)
#define RTC_DR_YU_Msk                (0xFUL << RTC_DR_YU_Pos)                   /*!< 0x000F0000 */
#define RTC_DR_YU                    RTC_DR_YU_Msk
#define RTC_DR_YU_0                  (0x1UL << RTC_DR_YU_Pos)                   /*!< 0x00010000 */
#define RTC_DR_YU_1                  (0x2UL << RTC_DR_YU_Pos)                   /*!< 0x00020000 */
#define RTC_DR_YU_2                  (0x4UL << RTC_DR_YU_Pos)                   /*!< 0x00040000 */
#define RTC_DR_YU_3                  (0x8UL << RTC_DR_YU_Pos)                   /*!< 0x00080000 */
#define RTC_DR_WDU_Pos               (13U)
#define RTC_DR_WDU_Msk               (0x7UL << RTC_DR_WDU_Pos)                  /*!< 0x0000E000 */
#define RTC_DR_WDU                   RTC_DR_WDU_Msk
#define RTC_DR_WDU_0                 (0x1UL << RTC_DR_WDU_Pos)                  /*!< 0x00002000 */
#define RTC_DR_WDU_1                 (0x2UL << RTC_DR_WDU_Pos)                  /*!< 0x00004000 */
#define RTC_DR_WDU_2                 (0x4UL << RTC_DR_WDU_Pos)                  /*!< 0x00008000 */
#define RTC_DR_MT_Pos                (12U)
#define RTC_DR_MT_Msk                (0x1UL << RTC_DR_MT_Pos)                   /*!< 0x00001000 */
#define RTC_DR_MT                    RTC_DR_MT_Msk
#define RTC_DR_MU_Pos                (8U)
#define RTC_DR_MU_Msk                (0xFUL << RTC_DR_MU_Pos)                   /*!< 0x00000F00 */
#define RTC_DR_MU                    RTC_DR_MU_Msk
#define RTC_DR_MU_0                  (0x1UL << RTC_DR_MU_Pos)                   /*!< 0x00000100 */
#define RTC_DR_MU_1                  (0x2UL << RTC_DR_MU_Pos)                   /*!< 0x00000200 */
#define RTC_DR_MU_2                  (0x4UL << RTC_DR_MU_Pos)                   /*!< 0x00000400 */
#define RTC_DR_MU_3                  (0x8UL << RTC_DR_MU_Pos)                   /*!< 0x00000800 */
#define RTC_DR_DT_Pos                (4U)
#define RTC_DR_DT_Msk                (0x3UL << RTC_DR_DT_Pos)                   /*!< 0x00000030 */
#define RTC_DR_DT                    RTC_DR_DT_Msk
#define RTC_DR_DT_0                  (0x1UL << RTC_DR_DT_Pos)                   /*!< 0x00000010 */
#define RTC_DR_DT_1                  (0x2UL << RTC_DR_DT_Pos)                   /*!< 0x00000020 */
#define RTC_DR_DU_Pos                (0U)
#define RTC_DR_DU_Msk                (0xFUL << RTC_DR_DU_Pos)                   /*!< 0x0000000F */
#define RTC_DR_DU                    RTC_DR_DU_Msk
#define RTC_DR_DU_0                  (0x1UL << RTC_DR_DU_Pos)                   /*!< 0x00000001 */
#define RTC_DR_DU_1                  (0x2UL << RTC_DR_DU_Pos)                   /*!< 0x00000002 */
#define RTC_DR_DU_2                  (0x4UL << RTC_DR_DU_Pos)                   /*!< 0x00000004 */
#define RTC_DR_DU_3                  (0x8UL << RTC_DR_DU_Pos)                   /*!< 0x00000008 */

/********************  Bits definition for RTC_SSR register  ******************/
#define RTC_SSR_SS_Pos               (0U)
#define RTC_SSR_SS_Msk               (0xFFFFUL << RTC_SSR_SS_Pos)               /*!< 0x0000FFFF */
#define RTC_SSR_SS                   RTC_SSR_SS_Msk

/********************  Bits definition for RTC_ICSR register  ******************/
#define RTC_ICSR_RECALPF_Pos         (16U)
#define RTC_ICSR_RECALPF_Msk         (0x1UL << RTC_ICSR_RECALPF_Pos)            /*!< 0x00010000 */
#define RTC_ICSR_RECALPF             RTC_ICSR_RECALPF_Msk
#define RTC_ICSR_INIT_Pos            (7U)
#define RTC_ICSR_INIT_Msk            (0x1UL << RTC_ICSR_INIT_Pos)               /*!< 0x00000080 */
#define RTC_ICSR_INIT                RTC_ICSR_INIT_Msk
#define RTC_ICSR_INITF_Pos           (6U)
#define RTC_ICSR_INITF_Msk           (0x1UL << RTC_ICSR_INITF_Pos)              /*!< 0x00000040 */
#define RTC_ICSR_INITF               RTC_ICSR_INITF_Msk
#define RTC_ICSR_RSF_Pos             (5U)
#define RTC_ICSR_RSF_Msk             (0x1UL << RTC_ICSR_RSF_Pos)                /*!< 0x00000020 */
#define RTC_ICSR_RSF                 RTC_ICSR_RSF_Msk
#define RTC_ICSR_INITS_Pos           (4U)
#define RTC_ICSR_INITS_Msk           (0x1UL << RTC_ICSR_INITS_Pos)              /*!< 0x00000010 */
#define RTC_ICSR_INITS               RTC_ICSR_INITS_Msk
#define RTC_ICSR_SHPF_Pos            (3U)
#define RTC_ICSR_SHPF_Msk            (0x1UL << RTC_ICSR_SHPF_Pos)               /*!< 0x00000008 */
#define RTC_ICSR_SHPF                RTC_ICSR_SHPF_Msk
#define RTC_ICSR_WUTWF_Pos           (2U)
#define RTC_ICSR_WUTWF_Msk           (0x1UL << RTC_ICSR_WUTWF_Pos)              /*!< 0x00000004 */
#define RTC_ICSR_WUTWF               RTC_ICSR_WUTWF_Msk
#define RTC_ICSR_ALRBWF_Pos          (1U)
#define RTC_ICSR_ALRBWF_Msk          (0x1UL << RTC_ICSR_ALRBWF_Pos)             /*!< 0x00000002 */
#define RTC_ICSR_ALRBWF              RTC_ICSR_ALRBWF_Msk
#define RTC_ICSR_ALRAWF_Pos          (0U)
#define RTC_ICSR_ALRAWF_Msk          (0x1UL << RTC_ICSR_ALRAWF_Pos)             /*!< 0x00000001 */
#define RTC_ICSR_ALRAWF              RTC_ICSR_ALRAWF_Msk

/********************  Bits definition for RTC_PRER register  *****************/
#define RTC_PRER_PREDIV_A_Pos        (16U)
#define RTC_PRER_PREDIV_A_Msk        (0x7FUL << RTC_PRER_PREDIV_A_Pos)          /*!< 0x007F0000 */
#define RTC_PRER_PREDIV_A            RTC_PRER_PREDIV_A_Msk
#define RTC_PRER_PREDIV_S_Pos        (0U)
#define RTC_PRER_PREDIV_S_Msk        (0x7FFFUL << RTC_PRER_PREDIV_S_Pos)        /*!< 0x00007FFF */
#define RTC_PRER_PREDIV_S            RTC_PRER_PREDIV_S_Msk

/********************  Bits definition for RTC_WUTR register  *****************/
#define RTC_WUTR_WUTOCLR_Pos         (16U)
#define RTC_WUTR_WUTOCLR_Msk         (0xFFFFUL << RTC_WUTR_WUTOCLR_Pos)         /*!< 0x0000FFFF */
#define RTC_WUTR_WUTOCLR             RTC_WUTR_WUTOCLR_Msk
#define RTC_WUTR_WUT_Pos             (0U)
#define RTC_WUTR_WUT_Msk             (0xFFFFUL << RTC_WUTR_WUT_Pos)             /*!< 0x0000FFFF */
#define RTC_WUTR_WUT                 RTC_WUTR_WUT_Msk

/********************  Bits definition for RTC_CR register  *******************/
#define RTC_CR_OUT2EN_Pos            (31U)
#define RTC_CR_OUT2EN_Msk            (0x1UL << RTC_CR_OUT2EN_Pos)               /*!< 0x80000000 */
#define RTC_CR_OUT2EN                RTC_CR_OUT2EN_Msk                          /*!<RTC_OUT2 output enable */
#define RTC_CR_TAMPALRM_TYPE_Pos     (30U)
#define RTC_CR_TAMPALRM_TYPE_Msk     (0x1UL << RTC_CR_TAMPALRM_TYPE_Pos)        /*!< 0x40000000 */
#define RTC_CR_TAMPALRM_TYPE         RTC_CR_TAMPALRM_TYPE_Msk                   /*!<TAMPALARM output type  */
#define RTC_CR_TAMPALRM_PU_Pos       (29U)
#define RTC_CR_TAMPALRM_PU_Msk       (0x1UL << RTC_CR_TAMPALRM_PU_Pos)          /*!< 0x20000000 */
#define RTC_CR_TAMPALRM_PU           RTC_CR_TAMPALRM_PU_Msk                     /*!<TAMPALARM output pull-up config */
#define RTC_CR_TAMPOE_Pos            (26U)
#define RTC_CR_TAMPOE_Msk            (0x1UL << RTC_CR_TAMPOE_Pos)               /*!< 0x04000000 */
#define RTC_CR_TAMPOE                RTC_CR_TAMPOE_Msk                          /*!<Tamper detection output enable on TAMPALARM  */
#define RTC_CR_TAMPTS_Pos            (25U)
#define RTC_CR_TAMPTS_Msk            (0x1UL << RTC_CR_TAMPTS_Pos)               /*!< 0x02000000 */
#define RTC_CR_TAMPTS                RTC_CR_TAMPTS_Msk                          /*!<Activate timestamp on tamper detection event  */
#define RTC_CR_ITSE_Pos              (24U)
#define RTC_CR_ITSE_Msk              (0x1UL << RTC_CR_ITSE_Pos)                 /*!< 0x01000000 */
#define RTC_CR_ITSE                  RTC_CR_ITSE_Msk                            /*!<Timestamp on internal event enable  */
#define RTC_CR_COE_Pos               (23U)
#define RTC_CR_COE_Msk               (0x1UL << RTC_CR_COE_Pos)                  /*!< 0x00800000 */
#define RTC_CR_COE                   RTC_CR_COE_Msk
#define RTC_CR_OSEL_Pos              (21U)
#define RTC_CR_OSEL_Msk              (0x3UL << RTC_CR_OSEL_Pos)                 /*!< 0x00600000 */
#define RTC_CR_OSEL                  RTC_CR_OSEL_Msk
#define RTC_CR_OSEL_0                (0x1UL << RTC_CR_OSEL_Pos)                 /*!< 0x00200000 */
#define RTC_CR_OSEL_1                (0x2UL << RTC_CR_OSEL_Pos)                 /*!< 0x00400000 */
#define RTC_CR_POL_Pos               (20U)
#define RTC_CR_POL_Msk               (0x1UL << RTC_CR_POL_Pos)                  /*!< 0x00100000 */
#define RTC_CR_POL                   RTC_CR_POL_Msk
#define RTC_CR_COSEL_Pos             (19U)
#define RTC_CR_COSEL_Msk             (0x1UL << RTC_CR_COSEL_Pos)                /*!< 0x00080000 */
#define RTC_CR_COSEL                 RTC_CR_COSEL_Msk
#define RTC_CR_BKP_Pos               (18U)
#define RTC_CR_BKP_Msk               (0x1UL << RTC_CR_BKP_Pos)                  /*!< 0x00040000 */
#define RTC_CR_BKP                   RTC_CR_BKP_Msk
#define RTC_CR_SUB1H_Pos             (17U)
#define RTC_CR_SUB1H_Msk             (0x1UL << RTC_CR_SUB1H_Pos)                /*!< 0x00020000 */
#define RTC_CR_SUB1H                 RTC_CR_SUB1H_Msk
#define RTC_CR_ADD1H_Pos             (16U)
#define RTC_CR_ADD1H_Msk             (0x1UL << RTC_CR_ADD1H_Pos)                /*!< 0x00010000 */
#define RTC_CR_ADD1H                 RTC_CR_ADD1H_Msk
#define RTC_CR_TSIE_Pos              (15U)
#define RTC_CR_TSIE_Msk              (0x1UL << RTC_CR_TSIE_Pos)                 /*!< 0x00008000 */
#define RTC_CR_TSIE                  RTC_CR_TSIE_Msk
#define RTC_CR_WUTIE_Pos             (14U)
#define RTC_CR_WUTIE_Msk             (0x1UL << RTC_CR_WUTIE_Pos)                /*!< 0x00004000 */
#define RTC_CR_WUTIE                 RTC_CR_WUTIE_Msk
#define RTC_CR_ALRBIE_Pos            (13U)
#define RTC_CR_ALRBIE_Msk            (0x1UL << RTC_CR_ALRBIE_Pos)               /*!< 0x00002000 */
#define RTC_CR_ALRBIE                RTC_CR_ALRBIE_Msk
#define RTC_CR_ALRAIE_Pos            (12U)
#define RTC_CR_ALRAIE_Msk            (0x1UL << RTC_CR_ALRAIE_Pos)               /*!< 0x00001000 */
#define RTC_CR_ALRAIE                RTC_CR_ALRAIE_Msk
#define RTC_CR_TSE_Pos               (11U)
#define RTC_CR_TSE_Msk               (0x1UL << RTC_CR_TSE_Pos)                  /*!< 0x00000800 */
#define RTC_CR_TSE                   RTC_CR_TSE_Msk
#define RTC_CR_WUTE_Pos              (10U)
#define RTC_CR_WUTE_Msk              (0x1UL << RTC_CR_WUTE_Pos)                 /*!< 0x00000400 */
#define RTC_CR_WUTE                  RTC_CR_WUTE_Msk
#define RTC_CR_ALRBE_Pos             (9U)
#define RTC_CR_ALRBE_Msk             (0x1UL << RTC_CR_ALRBE_Pos)                /*!< 0x00000200 */
#define RTC_CR_ALRBE                 RTC_CR_ALRBE_Msk
#define RTC_CR_ALRAE_Pos             (8U)
#define RTC_CR_ALRAE_Msk             (0x1UL << RTC_CR_ALRAE_Pos)                /*!< 0x00000100 */
#define RTC_CR_ALRAE                 RTC_CR_ALRAE_Msk
#define RTC_CR_FMT_Pos               (6U)
#define RTC_CR_FMT_Msk               (0x1UL << RTC_CR_FMT_Pos)                  /*!< 0x00000040 */
#define RTC_CR_FMT                   RTC_CR_FMT_Msk
#define RTC_CR_BYPSHAD_Pos           (5U)
#define RTC_CR_BYPSHAD_Msk           (0x1UL << RTC_CR_BYPSHAD_Pos)              /*!< 0x00000020 */
#define RTC_CR_BYPSHAD               RTC_CR_BYPSHAD_Msk
#define RTC_CR_REFCKON_Pos           (4U)
#define RTC_CR_REFCKON_Msk           (0x1UL << RTC_CR_REFCKON_Pos)              /*!< 0x00000010 */
#define RTC_CR_REFCKON               RTC_CR_REFCKON_Msk
#define RTC_CR_TSEDGE_Pos            (3U)
#define RTC_CR_TSEDGE_Msk            (0x1UL << RTC_CR_TSEDGE_Pos)               /*!< 0x00000008 */
#define RTC_CR_TSEDGE                RTC_CR_TSEDGE_Msk
#define RTC_CR_WUCKSEL_Pos           (0U)
#define RTC_CR_WUCKSEL_Msk           (0x7UL << RTC_CR_WUCKSEL_Pos)              /*!< 0x00000007 */
#define RTC_CR_WUCKSEL               RTC_CR_WUCKSEL_Msk
#define RTC_CR_WUCKSEL_0             (0x1UL << RTC_CR_WUCKSEL_Pos)              /*!< 0x00000001 */
#define RTC_CR_WUCKSEL_1             (0x2UL << RTC_CR_WUCKSEL_Pos)              /*!< 0x00000002 */
#define RTC_CR_WUCKSEL_2             (0x4UL << RTC_CR_WUCKSEL_Pos)              /*!< 0x00000004 */

/********************  Bits definition for RTC_WPR register  ******************/
#define RTC_WPR_KEY_Pos              (0U)
#define RTC_WPR_KEY_Msk              (0xFFUL << RTC_WPR_KEY_Pos)                /*!< 0x000000FF */
#define RTC_WPR_KEY                  RTC_WPR_KEY_Msk

/********************  Bits definition for RTC_CALR register  *****************/
#define RTC_CALR_CALP_Pos            (15U)
#define RTC_CALR_CALP_Msk            (0x1UL << RTC_CALR_CALP_Pos)               /*!< 0x00008000 */
#define RTC_CALR_CALP                RTC_CALR_CALP_Msk
#define RTC_CALR_CALW8_Pos           (14U)
#define RTC_CALR_CALW8_Msk           (0x1UL << RTC_CALR_CALW8_Pos)              /*!< 0x00004000 */
#define RTC_CALR_CALW8               RTC_CALR_CALW8_Msk
#define RTC_CALR_CALW16_Pos          (13U)
#define RTC_CALR_CALW16_Msk          (0x1UL << RTC_CALR_CALW16_Pos)             /*!< 0x00002000 */
#define RTC_CALR_LPCAL               RTC_CALR_LPCAL_Msk
#define RTC_CALR_LPCAL_Pos           (12U)
#define RTC_CALR_LPCAL_Msk           (0x1UL << RTC_CALR_LPCAL_Pos)              /*!< 0x00001000 */
#define RTC_CALR_CALW16              RTC_CALR_CALW16_Msk
#define RTC_CALR_CALM_Pos            (0U)
#define RTC_CALR_CALM_Msk            (0x1FFUL << RTC_CALR_CALM_Pos)             /*!< 0x000001FF */
#define RTC_CALR_CALM                RTC_CALR_CALM_Msk
#define RTC_CALR_CALM_0              (0x001UL << RTC_CALR_CALM_Pos)             /*!< 0x00000001 */
#define RTC_CALR_CALM_1              (0x002UL << RTC_CALR_CALM_Pos)             /*!< 0x00000002 */
#define RTC_CALR_CALM_2              (0x004UL << RTC_CALR_CALM_Pos)             /*!< 0x00000004 */
#define RTC_CALR_CALM_3              (0x008UL << RTC_CALR_CALM_Pos)             /*!< 0x00000008 */
#define RTC_CALR_CALM_4              (0x010UL << RTC_CALR_CALM_Pos)             /*!< 0x00000010 */
#define RTC_CALR_CALM_5              (0x020UL << RTC_CALR_CALM_Pos)             /*!< 0x00000020 */
#define RTC_CALR_CALM_6              (0x040UL << RTC_CALR_CALM_Pos)             /*!< 0x00000040 */
#define RTC_CALR_CALM_7              (0x080UL << RTC_CALR_CALM_Pos)             /*!< 0x00000080 */
#define RTC_CALR_CALM_8              (0x100UL << RTC_CALR_CALM_Pos)             /*!< 0x00000100 */

/********************  Bits definition for RTC_SHIFTR register  ***************/
#define RTC_SHIFTR_ADD1S_Pos         (31U)
#define RTC_SHIFTR_ADD1S_Msk         (0x1UL << RTC_SHIFTR_ADD1S_Pos)            /*!< 0x80000000 */
#define RTC_SHIFTR_ADD1S             RTC_SHIFTR_ADD1S_Msk
#define RTC_SHIFTR_SUBFS_Pos         (0U)
#define RTC_SHIFTR_SUBFS_Msk         (0x7FFFUL << RTC_SHIFTR_SUBFS_Pos)         /*!< 0x00007FFF */
#define RTC_SHIFTR_SUBFS             RTC_SHIFTR_SUBFS_Msk

/********************  Bits definition for RTC_TSTR register  *****************/
#define RTC_TSTR_PM_Pos              (22U)
#define RTC_TSTR_PM_Msk              (0x1UL << RTC_TSTR_PM_Pos)                 /*!< 0x00400000 */
#define RTC_TSTR_PM                  RTC_TSTR_PM_Msk
#define RTC_TSTR_HT_Pos              (20U)
#define RTC_TSTR_HT_Msk              (0x3UL << RTC_TSTR_HT_Pos)                 /*!< 0x00300000 */
#define RTC_TSTR_HT                  RTC_TSTR_HT_Msk
#define RTC_TSTR_HT_0                (0x1UL << RTC_TSTR_HT_Pos)                 /*!< 0x00100000 */
#define RTC_TSTR_HT_1                (0x2UL << RTC_TSTR_HT_Pos)                 /*!< 0x00200000 */
#define RTC_TSTR_HU_Pos              (16U)
#define RTC_TSTR_HU_Msk              (0xFUL << RTC_TSTR_HU_Pos)                 /*!< 0x000F0000 */
#define RTC_TSTR_HU                  RTC_TSTR_HU_Msk
#define RTC_TSTR_HU_0                (0x1UL << RTC_TSTR_HU_Pos)                 /*!< 0x00010000 */
#define RTC_TSTR_HU_1                (0x2UL << RTC_TSTR_HU_Pos)                 /*!< 0x00020000 */
#define RTC_TSTR_HU_2                (0x4UL << RTC_TSTR_HU_Pos)                 /*!< 0x00040000 */
#define RTC_TSTR_HU_3                (0x8UL << RTC_TSTR_HU_Pos)                 /*!< 0x00080000 */
#define RTC_TSTR_MNT_Pos             (12U)
#define RTC_TSTR_MNT_Msk             (0x7UL << RTC_TSTR_MNT_Pos)                /*!< 0x00007000 */
#define RTC_TSTR_MNT                 RTC_TSTR_MNT_Msk
#define RTC_TSTR_MNT_0               (0x1UL << RTC_TSTR_MNT_Pos)                /*!< 0x00001000 */
#define RTC_TSTR_MNT_1               (0x2UL << RTC_TSTR_MNT_Pos)                /*!< 0x00002000 */
#define RTC_TSTR_MNT_2               (0x4UL << RTC_TSTR_MNT_Pos)                /*!< 0x00004000 */
#define RTC_TSTR_MNU_Pos             (8U)
#define RTC_TSTR_MNU_Msk             (0xFUL << RTC_TSTR_MNU_Pos)                /*!< 0x00000F00 */
#define RTC_TSTR_MNU                 RTC_TSTR_MNU_Msk
#define RTC_TSTR_MNU_0               (0x1UL << RTC_TSTR_MNU_Pos)                /*!< 0x00000100 */
#define RTC_TSTR_MNU_1               (0x2UL << RTC_TSTR_MNU_Pos)                /*!< 0x00000200 */
#define RTC_TSTR_MNU_2               (0x4UL << RTC_TSTR_MNU_Pos)                /*!< 0x00000400 */
#define RTC_TSTR_MNU_3               (0x8UL << RTC_TSTR_MNU_Pos)                /*!< 0x00000800 */
#define RTC_TSTR_ST_Pos              (4U)
#define RTC_TSTR_ST_Msk              (0x7UL << RTC_TSTR_ST_Pos)                 /*!< 0x00000070 */
#define RTC_TSTR_ST                  RTC_TSTR_ST_Msk
#define RTC_TSTR_ST_0                (0x1UL << RTC_TSTR_ST_Pos)                 /*!< 0x00000010 */
#define RTC_TSTR_ST_1                (0x2UL << RTC_TSTR_ST_Pos)                 /*!< 0x00000020 */
#define RTC_TSTR_ST_2                (0x4UL << RTC_TSTR_ST_Pos)                 /*!< 0x00000040 */
#define RTC_TSTR_SU_Pos              (0U)
#define RTC_TSTR_SU_Msk              (0xFUL << RTC_TSTR_SU_Pos)                 /*!< 0x0000000F */
#define RTC_TSTR_SU                  RTC_TSTR_SU_Msk
#define RTC_TSTR_SU_0                (0x1UL << RTC_TSTR_SU_Pos)                 /*!< 0x00000001 */
#define RTC_TSTR_SU_1                (0x2UL << RTC_TSTR_SU_Pos)                 /*!< 0x00000002 */
#define RTC_TSTR_SU_2                (0x4UL << RTC_TSTR_SU_Pos)                 /*!< 0x00000004 */
#define RTC_TSTR_SU_3                (0x8UL << RTC_TSTR_SU_Pos)                 /*!< 0x00000008 */

/********************  Bits definition for RTC_TSDR register  *****************/
#define RTC_TSDR_WDU_Pos             (13U)
#define RTC_TSDR_WDU_Msk             (0x7UL << RTC_TSDR_WDU_Pos)                /*!< 0x0000E000 */
#define RTC_TSDR_WDU                 RTC_TSDR_WDU_Msk
#define RTC_TSDR_WDU_0               (0x1UL << RTC_TSDR_WDU_Pos)                /*!< 0x00002000 */
#define RTC_TSDR_WDU_1               (0x2UL << RTC_TSDR_WDU_Pos)                /*!< 0x00004000 */
#define RTC_TSDR_WDU_2               (0x4UL << RTC_TSDR_WDU_Pos)                /*!< 0x00008000 */
#define RTC_TSDR_MT_Pos              (12U)
#define RTC_TSDR_MT_Msk              (0x1UL << RTC_TSDR_MT_Pos)                 /*!< 0x00001000 */
#define RTC_TSDR_MT                  RTC_TSDR_MT_Msk
#define RTC_TSDR_MU_Pos              (8U)
#define RTC_TSDR_MU_Msk              (0xFUL << RTC_TSDR_MU_Pos)                 /*!< 0x00000F00 */
#define RTC_TSDR_MU                  RTC_TSDR_MU_Msk
#define RTC_TSDR_MU_0                (0x1UL << RTC_TSDR_MU_Pos)                 /*!< 0x00000100 */
#define RTC_TSDR_MU_1                (0x2UL << RTC_TSDR_MU_Pos)                 /*!< 0x00000200 */
#define RTC_TSDR_MU_2                (0x4UL << RTC_TSDR_MU_Pos)                 /*!< 0x00000400 */
#define RTC_TSDR_MU_3                (0x8UL << RTC_TSDR_MU_Pos)                 /*!< 0x00000800 */
#define RTC_TSDR_DT_Pos              (4U)
#define RTC_TSDR_DT_Msk              (0x3UL << RTC_TSDR_DT_Pos)                 /*!< 0x00000030 */
#define RTC_TSDR_DT                  RTC_TSDR_DT_Msk
#define RTC_TSDR_DT_0                (0x1UL << RTC_TSDR_DT_Pos)                 /*!< 0x00000010 */
#define RTC_TSDR_DT_1                (0x2UL << RTC_TSDR_DT_Pos)                 /*!< 0x00000020 */
#define RTC_TSDR_DU_Pos              (0U)
#define RTC_TSDR_DU_Msk              (0xFUL << RTC_TSDR_DU_Pos)                 /*!< 0x0000000F */
#define RTC_TSDR_DU                  RTC_TSDR_DU_Msk
#define RTC_TSDR_DU_0                (0x1UL << RTC_TSDR_DU_Pos)                 /*!< 0x00000001 */
#define RTC_TSDR_DU_1                (0x2UL << RTC_TSDR_DU_Pos)                 /*!< 0x00000002 */
#define RTC_TSDR_DU_2                (0x4UL << RTC_TSDR_DU_Pos)                 /*!< 0x00000004 */
#define RTC_TSDR_DU_3                (0x8UL << RTC_TSDR_DU_Pos)                 /*!< 0x00000008 */

/********************  Bits definition for RTC_TSSSR register  ****************/
#define RTC_TSSSR_SS_Pos             (0U)
#define RTC_TSSSR_SS_Msk             (0xFFFFUL << RTC_TSSSR_SS_Pos)             /*!< 0x0000FFFF */
#define RTC_TSSSR_SS                 RTC_TSSSR_SS_Msk

/********************  Bits definition for RTC_ALRMAR register  ***************/
#define RTC_ALRMAR_MSK4_Pos          (31U)
#define RTC_ALRMAR_MSK4_Msk          (0x1UL << RTC_ALRMAR_MSK4_Pos)             /*!< 0x80000000 */
#define RTC_ALRMAR_MSK4              RTC_ALRMAR_MSK4_Msk
#define RTC_ALRMAR_WDSEL_Pos         (30U)
#define RTC_ALRMAR_WDSEL_Msk         (0x1UL << RTC_ALRMAR_WDSEL_Pos)            /*!< 0x40000000 */
#define RTC_ALRMAR_WDSEL             RTC_ALRMAR_WDSEL_Msk
#define RTC_ALRMAR_DT_Pos            (28U)
#define RTC_ALRMAR_DT_Msk            (0x3UL << RTC_ALRMAR_DT_Pos)               /*!< 0x30000000 */
#define RTC_ALRMAR_DT                RTC_ALRMAR_DT_Msk
#define RTC_ALRMAR_DT_0              (0x1UL << RTC_ALRMAR_DT_Pos)               /*!< 0x10000000 */
#define RTC_ALRMAR_DT_1              (0x2UL << RTC_ALRMAR_DT_Pos)               /*!< 0x20000000 */
#define RTC_ALRMAR_DU_Pos            (24U)
#define RTC_ALRMAR_DU_Msk            (0xFUL << RTC_ALRMAR_DU_Pos)               /*!< 0x0F000000 */
#define RTC_ALRMAR_DU                RTC_ALRMAR_DU_Msk
#define RTC_ALRMAR_DU_0              (0x1UL << RTC_ALRMAR_DU_Pos)               /*!< 0x01000000 */
#define RTC_ALRMAR_DU_1              (0x2UL << RTC_ALRMAR_DU_Pos)               /*!< 0x02000000 */
#define RTC_ALRMAR_DU_2              (0x4UL << RTC_ALRMAR_DU_Pos)               /*!< 0x04000000 */
#define RTC_ALRMAR_DU_3              (0x8UL << RTC_ALRMAR_DU_Pos)               /*!< 0x08000000 */
#define RTC_ALRMAR_MSK3_Pos          (23U)
#define RTC_ALRMAR_MSK3_Msk          (0x1UL << RTC_ALRMAR_MSK3_Pos)             /*!< 0x00800000 */
#define RTC_ALRMAR_MSK3              RTC_ALRMAR_MSK3_Msk
#define RTC_ALRMAR_PM_Pos            (22U)
#define RTC_ALRMAR_PM_Msk            (0x1UL << RTC_ALRMAR_PM_Pos)               /*!< 0x00400000 */
#define RTC_ALRMAR_PM                RTC_ALRMAR_PM_Msk
#define RTC_ALRMAR_HT_Pos            (20U)
#define RTC_ALRMAR_HT_Msk            (0x3UL << RTC_ALRMAR_HT_Pos)               /*!< 0x00300000 */
#define RTC_ALRMAR_HT                RTC_ALRMAR_HT_Msk
#define RTC_ALRMAR_HT_0              (0x1UL << RTC_ALRMAR_HT_Pos)               /*!< 0x00100000 */
#define RTC_ALRMAR_HT_1              (0x2UL << RTC_ALRMAR_HT_Pos)               /*!< 0x00200000 */
#define RTC_ALRMAR_HU_Pos            (16U)
#define RTC_ALRMAR_HU_Msk            (0xFUL << RTC_ALRMAR_HU_Pos)               /*!< 0x000F0000 */
#define RTC_ALRMAR_HU                RTC_ALRMAR_HU_Msk
#define RTC_ALRMAR_HU_0              (0x1UL << RTC_ALRMAR_HU_Pos)               /*!< 0x00010000 */
#define RTC_ALRMAR_HU_1              (0x2UL << RTC_ALRMAR_HU_Pos)               /*!< 0x00020000 */
#define RTC_ALRMAR_HU_2              (0x4UL << RTC_ALRMAR_HU_Pos)               /*!< 0x00040000 */
#define RTC_ALRMAR_HU_3              (0x8UL << RTC_ALRMAR_HU_Pos)               /*!< 0x00080000 */
#define RTC_ALRMAR_MSK2_Pos          (15U)
#define RTC_ALRMAR_MSK2_Msk          (0x1UL << RTC_ALRMAR_MSK2_Pos)             /*!< 0x00008000 */
#define RTC_ALRMAR_MSK2              RTC_ALRMAR_MSK2_Msk
#define RTC_ALRMAR_MNT_Pos           (12U)
#define RTC_ALRMAR_MNT_Msk           (0x7UL << RTC_ALRMAR_MNT_Pos)              /*!< 0x00007000 */
#define RTC_ALRMAR_MNT               RTC_ALRMAR_MNT_Msk
#define RTC_ALRMAR_MNT_0             (0x1UL << RTC_ALRMAR_MNT_Pos)              /*!< 0x00001000 */
#define RTC_ALRMAR_MNT_1             (0x2UL << RTC_ALRMAR_MNT_Pos)              /*!< 0x00002000 */
#define RTC_ALRMAR_MNT_2             (0x4UL << RTC_ALRMAR_MNT_Pos)              /*!< 0x00004000 */
#define RTC_ALRMAR_MNU_Pos           (8U)
#define RTC_ALRMAR_MNU_Msk           (0xFUL << RTC_ALRMAR_MNU_Pos)              /*!< 0x00000F00 */
#define RTC_ALRMAR_MNU               RTC_ALRMAR_MNU_Msk
#define RTC_ALRMAR_MNU_0             (0x1UL << RTC_ALRMAR_MNU_Pos)              /*!< 0x00000100 */
#define RTC_ALRMAR_MNU_1             (0x2UL << RTC_ALRMAR_MNU_Pos)              /*!< 0x00000200 */
#define RTC_ALRMAR_MNU_2             (0x4UL << RTC_ALRMAR_MNU_Pos)              /*!< 0x00000400 */
#define RTC_ALRMAR_MNU_3             (0x8UL << RTC_ALRMAR_MNU_Pos)              /*!< 0x00000800 */
#define RTC_ALRMAR_MSK1_Pos          (7U)
#define RTC_ALRMAR_MSK1_Msk          (0x1UL << RTC_ALRMAR_MSK1_Pos)             /*!< 0x00000080 */
#define RTC_ALRMAR_MSK1              RTC_ALRMAR_MSK1_Msk
#define RTC_ALRMAR_ST_Pos            (4U)
#define RTC_ALRMAR_ST_Msk            (0x7UL << RTC_ALRMAR_ST_Pos)               /*!< 0x00000070 */
#define RTC_ALRMAR_ST                RTC_ALRMAR_ST_Msk
#define RTC_ALRMAR_ST_0              (0x1UL << RTC_ALRMAR_ST_Pos)               /*!< 0x00000010 */
#define RTC_ALRMAR_ST_1              (0x2UL << RTC_ALRMAR_ST_Pos)               /*!< 0x00000020 */
#define RTC_ALRMAR_ST_2              (0x4UL << RTC_ALRMAR_ST_Pos)               /*!< 0x00000040 */
#define RTC_ALRMAR_SU_Pos            (0U)
#define RTC_ALRMAR_SU_Msk            (0xFUL << RTC_ALRMAR_SU_Pos)               /*!< 0x0000000F */
#define RTC_ALRMAR_SU                RTC_ALRMAR_SU_Msk
#define RTC_ALRMAR_SU_0              (0x1UL << RTC_ALRMAR_SU_Pos)               /*!< 0x00000001 */
#define RTC_ALRMAR_SU_1              (0x2UL << RTC_ALRMAR_SU_Pos)               /*!< 0x00000002 */
#define RTC_ALRMAR_SU_2              (0x4UL << RTC_ALRMAR_SU_Pos)               /*!< 0x00000004 */
#define RTC_ALRMAR_SU_3              (0x8UL << RTC_ALRMAR_SU_Pos)               /*!< 0x00000008 */

/********************  Bits definition for RTC_ALRMASSR register  *************/
#define RTC_ALRMASSR_MASKSS_Pos      (24U)
#define RTC_ALRMASSR_MASKSS_Msk      (0xFUL << RTC_ALRMASSR_MASKSS_Pos)         /*!< 0x0F000000 */
#define RTC_ALRMASSR_MASKSS          RTC_ALRMASSR_MASKSS_Msk
#define RTC_ALRMASSR_MASKSS_0        (0x1UL << RTC_ALRMASSR_MASKSS_Pos)         /*!< 0x01000000 */
#define RTC_ALRMASSR_MASKSS_1        (0x2UL << RTC_ALRMASSR_MASKSS_Pos)         /*!< 0x02000000 */
#define RTC_ALRMASSR_MASKSS_2        (0x4UL << RTC_ALRMASSR_MASKSS_Pos)         /*!< 0x04000000 */
#define RTC_ALRMASSR_MASKSS_3        (0x8UL << RTC_ALRMASSR_MASKSS_Pos)         /*!< 0x08000000 */
#define RTC_ALRMASSR_SS_Pos          (0U)
#define RTC_ALRMASSR_SS_Msk          (0x7FFFUL << RTC_ALRMASSR_SS_Pos)          /*!< 0x00007FFF */
#define RTC_ALRMASSR_SS              RTC_ALRMASSR_SS_Msk

/********************  Bits definition for RTC_ALRMBR register  ***************/
#define RTC_ALRMBR_MSK4_Pos          (31U)
#define RTC_ALRMBR_MSK4_Msk          (0x1UL << RTC_ALRMBR_MSK4_Pos)             /*!< 0x80000000 */
#define RTC_ALRMBR_MSK4              RTC_ALRMBR_MSK4_Msk
#define RTC_ALRMBR_WDSEL_Pos         (30U)
#define RTC_ALRMBR_WDSEL_Msk         (0x1UL << RTC_ALRMBR_WDSEL_Pos)            /*!< 0x40000000 */
#define RTC_ALRMBR_WDSEL             RTC_ALRMBR_WDSEL_Msk
#define RTC_ALRMBR_DT_Pos            (28U)
#define RTC_ALRMBR_DT_Msk            (0x3UL << RTC_ALRMBR_DT_Pos)               /*!< 0x30000000 */
#define RTC_ALRMBR_DT                RTC_ALRMBR_DT_Msk
#define RTC_ALRMBR_DT_0              (0x1UL << RTC_ALRMBR_DT_Pos)               /*!< 0x10000000 */
#define RTC_ALRMBR_DT_1              (0x2UL << RTC_ALRMBR_DT_Pos)               /*!< 0x20000000 */
#define RTC_ALRMBR_DU_Pos            (24U)
#define RTC_ALRMBR_DU_Msk            (0xFUL << RTC_ALRMBR_DU_Pos)               /*!< 0x0F000000 */
#define RTC_ALRMBR_DU                RTC_ALRMBR_DU_Msk
#define RTC_ALRMBR_DU_0              (0x1UL << RTC_ALRMBR_DU_Pos)               /*!< 0x01000000 */
#define RTC_ALRMBR_DU_1              (0x2UL << RTC_ALRMBR_DU_Pos)               /*!< 0x02000000 */
#define RTC_ALRMBR_DU_2              (0x4UL << RTC_ALRMBR_DU_Pos)               /*!< 0x04000000 */
#define RTC_ALRMBR_DU_3              (0x8UL << RTC_ALRMBR_DU_Pos)               /*!< 0x08000000 */
#define RTC_ALRMBR_MSK3_Pos          (23U)
#define RTC_ALRMBR_MSK3_Msk          (0x1UL << RTC_ALRMBR_MSK3_Pos)             /*!< 0x00800000 */
#define RTC_ALRMBR_MSK3              RTC_ALRMBR_MSK3_Msk
#define RTC_ALRMBR_PM_Pos            (22U)
#define RTC_ALRMBR_PM_Msk            (0x1UL << RTC_ALRMBR_PM_Pos)               /*!< 0x00400000 */
#define RTC_ALRMBR_PM                RTC_ALRMBR_PM_Msk
#define RTC_ALRMBR_HT_Pos            (20U)
#define RTC_ALRMBR_HT_Msk            (0x3UL << RTC_ALRMBR_HT_Pos)               /*!< 0x00300000 */
#define RTC_ALRMBR_HT                RTC_ALRMBR_HT_Msk
#define RTC_ALRMBR_HT_0              (0x1UL << RTC_ALRMBR_HT_Pos)               /*!< 0x00100000 */
#define RTC_ALRMBR_HT_1              (0x2UL << RTC_ALRMBR_HT_Pos)               /*!< 0x00200000 */
#define RTC_ALRMBR_HU_Pos            (16U)
#define RTC_ALRMBR_HU_Msk            (0xFUL << RTC_ALRMBR_HU_Pos)               /*!< 0x000F0000 */
#define RTC_ALRMBR_HU                RTC_ALRMBR_HU_Msk
#define RTC_ALRMBR_HU_0              (0x1UL << RTC_ALRMBR_HU_Pos)               /*!< 0x00010000 */
#define RTC_ALRMBR_HU_1              (0x2UL << RTC_ALRMBR_HU_Pos)               /*!< 0x00020000 */
#define RTC_ALRMBR_HU_2              (0x4UL << RTC_ALRMBR_HU_Pos)               /*!< 0x00040000 */
#define RTC_ALRMBR_HU_3              (0x8UL << RTC_ALRMBR_HU_Pos)               /*!< 0x00080000 */
#define RTC_ALRMBR_MSK2_Pos          (15U)
#define RTC_ALRMBR_MSK2_Msk          (0x1UL << RTC_ALRMBR_MSK2_Pos)             /*!< 0x00008000 */
#define RTC_ALRMBR_MSK2              RTC_ALRMBR_MSK2_Msk
#define RTC_ALRMBR_MNT_Pos           (12U)
#define RTC_ALRMBR_MNT_Msk           (0x7UL << RTC_ALRMBR_MNT_Pos)              /*!< 0x00007000 */
#define RTC_ALRMBR_MNT               RTC_ALRMBR_MNT_Msk
#define RTC_ALRMBR_MNT_0             (0x1UL << RTC_ALRMBR_MNT_Pos)              /*!< 0x00001000 */
#define RTC_ALRMBR_MNT_1             (0x2UL << RTC_ALRMBR_MNT_Pos)              /*!< 0x00002000 */
#define RTC_ALRMBR_MNT_2             (0x4UL << RTC_ALRMBR_MNT_Pos)              /*!< 0x00004000 */
#define RTC_ALRMBR_MNU_Pos           (8U)
#define RTC_ALRMBR_MNU_Msk           (0xFUL << RTC_ALRMBR_MNU_Pos)              /*!< 0x00000F00 */
#define RTC_ALRMBR_MNU               RTC_ALRMBR_MNU_Msk
#define RTC_ALRMBR_MNU_0             (0x1UL << RTC_ALRMBR_MNU_Pos)              /*!< 0x00000100 */
#define RTC_ALRMBR_MNU_1             (0x2UL << RTC_ALRMBR_MNU_Pos)              /*!< 0x00000200 */
#define RTC_ALRMBR_MNU_2             (0x4UL << RTC_ALRMBR_MNU_Pos)              /*!< 0x00000400 */
#define RTC_ALRMBR_MNU_3             (0x8UL << RTC_ALRMBR_MNU_Pos)              /*!< 0x00000800 */
#define RTC_ALRMBR_MSK1_Pos          (7U)
#define RTC_ALRMBR_MSK1_Msk          (0x1UL << RTC_ALRMBR_MSK1_Pos)             /*!< 0x00000080 */
#define RTC_ALRMBR_MSK1              RTC_ALRMBR_MSK1_Msk
#define RTC_ALRMBR_ST_Pos            (4U)
#define RTC_ALRMBR_ST_Msk            (0x7UL << RTC_ALRMBR_ST_Pos)               /*!< 0x00000070 */
#define RTC_ALRMBR_ST                RTC_ALRMBR_ST_Msk
#define RTC_ALRMBR_ST_0              (0x1UL << RTC_ALRMBR_ST_Pos)               /*!< 0x00000010 */
#define RTC_ALRMBR_ST_1              (0x2UL << RTC_ALRMBR_ST_Pos)               /*!< 0x00000020 */
#define RTC_ALRMBR_ST_2              (0x4UL << RTC_ALRMBR_ST_Pos)               /*!< 0x00000040 */
#define RTC_ALRMBR_SU_Pos            (0U)
#define RTC_ALRMBR_SU_Msk            (0xFUL << RTC_ALRMBR_SU_Pos)               /*!< 0x0000000F */
#define RTC_ALRMBR_SU                RTC_ALRMBR_SU_Msk
#define RTC_ALRMBR_SU_0              (0x1UL << RTC_ALRMBR_SU_Pos)               /*!< 0x00000001 */
#define RTC_ALRMBR_SU_1              (0x2UL << RTC_ALRMBR_SU_Pos)               /*!< 0x00000002 */
#define RTC_ALRMBR_SU_2              (0x4UL << RTC_ALRMBR_SU_Pos)               /*!< 0x00000004 */
#define RTC_ALRMBR_SU_3              (0x8UL << RTC_ALRMBR_SU_Pos)               /*!< 0x00000008 */

/********************  Bits definition for RTC_ALRMBSSR register  *************/
#define RTC_ALRMBSSR_MASKSS_Pos      (24U)
#define RTC_ALRMBSSR_MASKSS_Msk      (0xFUL << RTC_ALRMBSSR_MASKSS_Pos)         /*!< 0x0F000000 */
#define RTC_ALRMBSSR_MASKSS          RTC_ALRMBSSR_MASKSS_Msk
#define RTC_ALRMBSSR_MASKSS_0        (0x1UL << RTC_ALRMBSSR_MASKSS_Pos)         /*!< 0x01000000 */
#define RTC_ALRMBSSR_MASKSS_1        (0x2UL << RTC_ALRMBSSR_MASKSS_Pos)         /*!< 0x02000000 */
#define RTC_ALRMBSSR_MASKSS_2        (0x4UL << RTC_ALRMBSSR_MASKSS_Pos)         /*!< 0x04000000 */
#define RTC_ALRMBSSR_MASKSS_3        (0x8UL << RTC_ALRMBSSR_MASKSS_Pos)         /*!< 0x08000000 */
#define RTC_ALRMBSSR_SS_Pos          (0U)
#define RTC_ALRMBSSR_SS_Msk          (0x7FFFUL << RTC_ALRMBSSR_SS_Pos)          /*!< 0x00007FFF */
#define RTC_ALRMBSSR_SS              RTC_ALRMBSSR_SS_Msk

/********************  Bits definition for RTC_SR register  *******************/
#define RTC_SR_ITSF_Pos              (5U)
#define RTC_SR_ITSF_Msk              (0x1UL << RTC_SR_ITSF_Pos)                 /*!< 0x00000020 */
#define RTC_SR_ITSF                  RTC_SR_ITSF_Msk
#define RTC_SR_TSOVF_Pos             (4U)
#define RTC_SR_TSOVF_Msk             (0x1UL << RTC_SR_TSOVF_Pos)                /*!< 0x00000010 */
#define RTC_SR_TSOVF                 RTC_SR_TSOVF_Msk
#define RTC_SR_TSF_Pos               (3U)
#define RTC_SR_TSF_Msk               (0x1UL << RTC_SR_TSF_Pos)                  /*!< 0x00000008 */
#define RTC_SR_TSF                   RTC_SR_TSF_Msk
#define RTC_SR_WUTF_Pos              (2U)
#define RTC_SR_WUTF_Msk              (0x1UL << RTC_SR_WUTF_Pos)                 /*!< 0x00000004 */
#define RTC_SR_WUTF                  RTC_SR_WUTF_Msk
#define RTC_SR_ALRBF_Pos             (1U)
#define RTC_SR_ALRBF_Msk             (0x1UL << RTC_SR_ALRBF_Pos)                /*!< 0x00000002 */
#define RTC_SR_ALRBF                 RTC_SR_ALRBF_Msk
#define RTC_SR_ALRAF_Pos             (0U)
#define RTC_SR_ALRAF_Msk             (0x1UL << RTC_SR_ALRAF_Pos)                /*!< 0x00000001 */
#define RTC_SR_ALRAF                 RTC_SR_ALRAF_Msk

/********************  Bits definition for RTC_MISR register  *****************/
#define RTC_MISR_ITSMF_Pos           (5U)
#define RTC_MISR_ITSMF_Msk           (0x1UL << RTC_MISR_ITSMF_Pos)              /*!< 0x00000020 */
#define RTC_MISR_ITSMF               RTC_MISR_ITSMF_Msk
#define RTC_MISR_TSOVMF_Pos          (4U)
#define RTC_MISR_TSOVMF_Msk          (0x1UL << RTC_MISR_TSOVMF_Pos)             /*!< 0x00000010 */
#define RTC_MISR_TSOVMF              RTC_MISR_TSOVMF_Msk
#define RTC_MISR_TSMF_Pos            (3U)
#define RTC_MISR_TSMF_Msk            (0x1UL << RTC_MISR_TSMF_Pos)               /*!< 0x00000008 */
#define RTC_MISR_TSMF                RTC_MISR_TSMF_Msk
#define RTC_MISR_WUTMF_Pos           (2U)
#define RTC_MISR_WUTMF_Msk           (0x1UL << RTC_MISR_WUTMF_Pos)              /*!< 0x00000004 */
#define RTC_MISR_WUTMF               RTC_MISR_WUTMF_Msk
#define RTC_MISR_ALRBMF_Pos          (1U)
#define RTC_MISR_ALRBMF_Msk          (0x1UL << RTC_MISR_ALRBMF_Pos)             /*!< 0x00000002 */
#define RTC_MISR_ALRBMF              RTC_MISR_ALRBMF_Msk
#define RTC_MISR_ALRAMF_Pos          (0U)
#define RTC_MISR_ALRAMF_Msk          (0x1UL << RTC_MISR_ALRAMF_Pos)             /*!< 0x00000001 */
#define RTC_MISR_ALRAMF              RTC_MISR_ALRAMF_Msk

/********************  Bits definition for RTC_SCR register  ******************/
#define RTC_SCR_CITSF_Pos            (5U)
#define RTC_SCR_CITSF_Msk            (0x1UL << RTC_SCR_CITSF_Pos)               /*!< 0x00000020 */
#define RTC_SCR_CITSF                RTC_SCR_CITSF_Msk
#define RTC_SCR_CTSOVF_Pos           (4U)
#define RTC_SCR_CTSOVF_Msk           (0x1UL << RTC_SCR_CTSOVF_Pos)              /*!< 0x00000010 */
#define RTC_SCR_CTSOVF               RTC_SCR_CTSOVF_Msk
#define RTC_SCR_CTSF_Pos             (3U)
#define RTC_SCR_CTSF_Msk             (0x1UL << RTC_SCR_CTSF_Pos)                /*!< 0x00000008 */
#define RTC_SCR_CTSF                 RTC_SCR_CTSF_Msk
#define RTC_SCR_CWUTF_Pos            (2U)
#define RTC_SCR_CWUTF_Msk            (0x1UL << RTC_SCR_CWUTF_Pos)               /*!< 0x00000004 */
#define RTC_SCR_CWUTF                RTC_SCR_CWUTF_Msk
#define RTC_SCR_CALRBF_Pos           (1U)
#define RTC_SCR_CALRBF_Msk           (0x1UL << RTC_SCR_CALRBF_Pos)              /*!< 0x00000002 */
#define RTC_SCR_CALRBF               RTC_SCR_CALRBF_Msk
#define RTC_SCR_CALRAF_Pos           (0U)
#define RTC_SCR_CALRAF_Msk           (0x1UL << RTC_SCR_CALRAF_Pos)              /*!< 0x00000001 */
#define RTC_SCR_CALRAF               RTC_SCR_CALRAF_Msk

/******************************************************************************/
/*                                                                            */
/*                     Tamper and backup register (TAMP)                      */
/*                                                                            */
/******************************************************************************/
/********************  Bits definition for TAMP_CR1 register  *****************/
#define TAMP_CR1_TAMP1E_Pos          (0U)
#define TAMP_CR1_TAMP1E_Msk          (0x1UL << TAMP_CR1_TAMP1E_Pos)             /*!< 0x00000001 */
#define TAMP_CR1_TAMP1E              TAMP_CR1_TAMP1E_Msk
#define TAMP_CR1_TAMP2E_Pos          (1U)
#define TAMP_CR1_TAMP2E_Msk          (0x1UL << TAMP_CR1_TAMP2E_Pos)             /*!< 0x00000002 */
#define TAMP_CR1_TAMP2E              TAMP_CR1_TAMP2E_Msk

/********************  Bits definition for TAMP_CR2 register  *****************/
#define TAMP_CR2_TAMP1NOERASE_Pos    (0U)
#define TAMP_CR2_TAMP1NOERASE_Msk    (0x1UL << TAMP_CR2_TAMP1NOERASE_Pos)       /*!< 0x00000001 */
#define TAMP_CR2_TAMP1NOERASE        TAMP_CR2_TAMP1NOERASE_Msk
#define TAMP_CR2_TAMP2NOERASE_Pos    (1U)
#define TAMP_CR2_TAMP2NOERASE_Msk    (0x1UL << TAMP_CR2_TAMP2NOERASE_Pos)       /*!< 0x00000002 */
#define TAMP_CR2_TAMP2NOERASE        TAMP_CR2_TAMP2NOERASE_Msk
#define TAMP_CR2_TAMP1MSK_Pos        (16U)
#define TAMP_CR2_TAMP1MSK_Msk        (0x1UL << TAMP_CR2_TAMP1MSK_Pos)           /*!< 0x00010000 */
#define TAMP_CR2_TAMP1MSK            TAMP_CR2_TAMP1MSK_Msk
#define TAMP_CR2_TAMP2MSK_Pos        (17U)
#define TAMP_CR2_TAMP2MSK_Msk        (0x1UL << TAMP_CR2_TAMP2MSK_Pos)           /*!< 0x00020000 */
#define TAMP_CR2_TAMP2MSK            TAMP_CR2_TAMP2MSK_Msk
#define TAMP_CR2_BKERASE_Pos         (23U)
#define TAMP_CR2_BKERASE_Msk         (0x1UL << TAMP_CR2_BKERASE_Pos)            /*!< 0x00800000 */
#define TAMP_CR2_BKERASE             TAMP_CR2_BKERASE_Msk
#define TAMP_CR2_TAMP1TRG_Pos        (24U)
#define TAMP_CR2_TAMP1TRG_Msk        (0x1UL << TAMP_CR2_TAMP1TRG_Pos)           /*!< 0x01000000 */
#define TAMP_CR2_TAMP1TRG            TAMP_CR2_TAMP1TRG_Msk
#define TAMP_CR2_TAMP2TRG_Pos        (25U)
#define TAMP_CR2_TAMP2TRG_Msk        (0x1UL << TAMP_CR2_TAMP2TRG_Pos)           /*!< 0x02000000 */
#define TAMP_CR2_TAMP2TRG            TAMP_CR2_TAMP2TRG_Msk

/********************  Bits definition for TAMP_FLTCR register  ***************/
#define TAMP_FLTCR_TAMPFREQ_Pos      (0U)
#define TAMP_FLTCR_TAMPFREQ_Msk      (0x7UL << TAMP_FLTCR_TAMPFREQ_Pos)         /*!< 0x00000007 */
#define TAMP_FLTCR_TAMPFREQ          TAMP_FLTCR_TAMPFREQ_Msk
#define TAMP_FLTCR_TAMPFREQ_0        (0x1UL << TAMP_FLTCR_TAMPFREQ_Pos)         /*!< 0x00000001 */
#define TAMP_FLTCR_TAMPFREQ_1        (0x2UL << TAMP_FLTCR_TAMPFREQ_Pos)         /*!< 0x00000002 */
#define TAMP_FLTCR_TAMPFREQ_2        (0x4UL << TAMP_FLTCR_TAMPFREQ_Pos)         /*!< 0x00000004 */
#define TAMP_FLTCR_TAMPFLT_Pos       (3U)
#define TAMP_FLTCR_TAMPFLT_Msk       (0x3UL << TAMP_FLTCR_TAMPFLT_Pos)          /*!< 0x00000018 */
#define TAMP_FLTCR_TAMPFLT           TAMP_FLTCR_TAMPFLT_Msk
#define TAMP_FLTCR_TAMPFLT_0         (0x1UL << TAMP_FLTCR_TAMPFLT_Pos)          /*!< 0x00000008 */
#define TAMP_FLTCR_TAMPFLT_1         (0x2UL << TAMP_FLTCR_TAMPFLT_Pos)          /*!< 0x00000010 */
#define TAMP_FLTCR_TAMPPRCH_Pos      (5U)
#define TAMP_FLTCR_TAMPPRCH_Msk      (0x3UL << TAMP_FLTCR_TAMPPRCH_Pos)         /*!< 0x00000060 */
#define TAMP_FLTCR_TAMPPRCH          TAMP_FLTCR_TAMPPRCH_Msk
#define TAMP_FLTCR_TAMPPRCH_0        (0x1UL << TAMP_FLTCR_TAMPPRCH_Pos)         /*!< 0x00000020 */
#define TAMP_FLTCR_TAMPPRCH_1        (0x2UL << TAMP_FLTCR_TAMPPRCH_Pos)         /*!< 0x00000040 */
#define TAMP_FLTCR_TAMPPUDIS_Pos     (7U)
#define TAMP_FLTCR_TAMPPUDIS_Msk     (0x1UL << TAMP_FLTCR_TAMPPUDIS_Pos)        /*!< 0x00000080 */
#define TAMP_FLTCR_TAMPPUDIS         TAMP_FLTCR_TAMPPUDIS_Msk

/********************  Bits definition for TAMP_IER register  *****************/
#define TAMP_IER_TAMP1IE_Pos         (0U)
#define TAMP_IER_TAMP1IE_Msk         (0x1UL << TAMP_IER_TAMP1IE_Pos)            /*!< 0x00000001 */
#define TAMP_IER_TAMP1IE             TAMP_IER_TAMP1IE_Msk
#define TAMP_IER_TAMP2IE_Pos         (1U)
#define TAMP_IER_TAMP2IE_Msk         (0x1UL << TAMP_IER_TAMP2IE_Pos)            /*!< 0x00000002 */
#define TAMP_IER_TAMP2IE             TAMP_IER_TAMP2IE_Msk

/********************  Bits definition for TAMP_SR register  *****************/
#define TAMP_SR_TAMP1F_Pos           (0U)
#define TAMP_SR_TAMP1F_Msk           (0x1UL << TAMP_SR_TAMP1F_Pos)              /*!< 0x00000001 */
#define TAMP_SR_TAMP1F               TAMP_SR_TAMP1F_Msk
#define TAMP_SR_TAMP2F_Pos           (1U)
#define TAMP_SR_TAMP2F_Msk           (0x1UL << TAMP_SR_TAMP2F_Pos)              /*!< 0x00000002 */
#define TAMP_SR_TAMP2F               TAMP_SR_TAMP2F_Msk

/********************  Bits definition for TAMP_MISR register  ************ *****/
#define TAMP_MISR_TAMP1MF_Pos        (0U)
#define TAMP_MISR_TAMP1MF_Msk        (0x1UL << TAMP_MISR_TAMP1MF_Pos)           /*!< 0x00000001 */
#define TAMP_MISR_TAMP1MF            TAMP_MISR_TAMP1MF_Msk
#define TAMP_MISR_TAMP2MF_Pos        (1U)
#define TAMP_MISR_TAMP2MF_Msk        (0x1UL << TAMP_MISR_TAMP2MF_Pos)           /*!< 0x00000002 */
#define TAMP_MISR_TAMP2MF            TAMP_MISR_TAMP2MF_Msk


/********************  Bits definition for TAMP_SCR register  *****************/
#define TAMP_SCR_CTAMP1F_Pos         (0U)
#define TAMP_SCR_CTAMP1F_Msk         (0x1UL << TAMP_SCR_CTAMP1F_Pos)            /*!< 0x00000001 */
#define TAMP_SCR_CTAMP1F             TAMP_SCR_CTAMP1F_Msk
#define TAMP_SCR_CTAMP2F_Pos         (1U)
#define TAMP_SCR_CTAMP2F_Msk         (0x1UL << TAMP_SCR_CTAMP2F_Pos)            /*!< 0x00000002 */
#define TAMP_SCR_CTAMP2F             TAMP_SCR_CTAMP2F_Msk


/********************  Bits definition for TAMP_COUNTR register  ***************/
#define TAMP_COUNTR_Pos               (16U)
#define TAMP_COUNTR_Msk               (0xFFFFUL << TAMP_COUNTR_Pos)             /*!< 0xFFFF0000 */
#define TAMP_COUNTR                   TAMP_COUNTR_Msk

/********************  Bits definition for TAMP_BKP0R register  ***************/
#define TAMP_BKP0R_Pos               (0U)
#define TAMP_BKP0R_Msk               (0xFFFFFFFFUL << TAMP_BKP0R_Pos)           /*!< 0xFFFFFFFF */
#define TAMP_BKP0R                   TAMP_BKP0R_Msk

/********************  Bits definition for TAMP_BKP1R register  ****************/
#define TAMP_BKP1R_Pos               (0U)
#define TAMP_BKP1R_Msk               (0xFFFFFFFFUL << TAMP_BKP1R_Pos)           /*!< 0xFFFFFFFF */
#define TAMP_BKP1R                   TAMP_BKP1R_Msk

/********************  Bits definition for TAMP_BKP2R register  ****************/
#define TAMP_BKP2R_Pos               (0U)
#define TAMP_BKP2R_Msk               (0xFFFFFFFFUL << TAMP_BKP2R_Pos)           /*!< 0xFFFFFFFF */
#define TAMP_BKP2R                   TAMP_BKP2R_Msk

/********************  Bits definition for TAMP_BKP3R register  ****************/
#define TAMP_BKP3R_Pos               (0U)
#define TAMP_BKP3R_Msk               (0xFFFFFFFFUL << TAMP_BKP3R_Pos)           /*!< 0xFFFFFFFF */
#define TAMP_BKP3R                   TAMP_BKP3R_Msk

/********************  Bits definition for TAMP_BKP4R register  ****************/
#define TAMP_BKP4R_Pos               (0U)
#define TAMP_BKP4R_Msk               (0xFFFFFFFFUL << TAMP_BKP4R_Pos)           /*!< 0xFFFFFFFF */
#define TAMP_BKP4R                   TAMP_BKP4R_Msk

/********************  Bits definition for TAMP_BKP5R register  ****************/
#define TAMP_BKP5R_Pos               (0U)
#define TAMP_BKP5R_Msk               (0xFFFFFFFFUL << TAMP_BKP5R_Pos)           /*!< 0xFFFFFFFF */
#define TAMP_BKP5R                   TAMP_BKP5R_Msk

/********************  Bits definition for TAMP_BKP6R register  ****************/
#define TAMP_BKP6R_Pos               (0U)
#define TAMP_BKP6R_Msk               (0xFFFFFFFFUL << TAMP_BKP6R_Pos)           /*!< 0xFFFFFFFF */
#define TAMP_BKP6R                   TAMP_BKP6R_Msk

/********************  Bits definition for TAMP_BKP7R register  ****************/
#define TAMP_BKP7R_Pos               (0U)
#define TAMP_BKP7R_Msk               (0xFFFFFFFFUL << TAMP_BKP7R_Pos)           /*!< 0xFFFFFFFF */
#define TAMP_BKP7R                   TAMP_BKP7R_Msk

/********************  Bits definition for TAMP_BKP8R register  ****************/
#define TAMP_BKP8R_Pos               (0U)
#define TAMP_BKP8R_Msk               (0xFFFFFFFFUL << TAMP_BKP8R_Pos)           /*!< 0xFFFFFFFF */
#define TAMP_BKP8R                   TAMP_BKP8R_Msk

/********************  Bits definition for TAMP_BKP9R register  ****************/
#define TAMP_BKP9R_Pos               (0U)
#define TAMP_BKP9R_Msk               (0xFFFFFFFFUL << TAMP_BKP9R_Pos)           /*!< 0xFFFFFFFF */
#define TAMP_BKP9R                   TAMP_BKP9R_Msk

/********************  Bits definition for TAMP_BKP10R register  ***************/
#define TAMP_BKP10R_Pos              (0U)
#define TAMP_BKP10R_Msk              (0xFFFFFFFFUL << TAMP_BKP10R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP10R                  TAMP_BKP10R_Msk

/********************  Bits definition for TAMP_BKP11R register  ***************/
#define TAMP_BKP11R_Pos              (0U)
#define TAMP_BKP11R_Msk              (0xFFFFFFFFUL << TAMP_BKP11R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP11R                  TAMP_BKP11R_Msk

/********************  Bits definition for TAMP_BKP12R register  ***************/
#define TAMP_BKP12R_Pos              (0U)
#define TAMP_BKP12R_Msk              (0xFFFFFFFFUL << TAMP_BKP12R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP12R                  TAMP_BKP12R_Msk

/********************  Bits definition for TAMP_BKP13R register  ***************/
#define TAMP_BKP13R_Pos              (0U)
#define TAMP_BKP13R_Msk              (0xFFFFFFFFUL << TAMP_BKP13R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP13R                  TAMP_BKP13R_Msk

/********************  Bits definition for TAMP_BKP14R register  ***************/
#define TAMP_BKP14R_Pos              (0U)
#define TAMP_BKP14R_Msk              (0xFFFFFFFFUL << TAMP_BKP14R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP14R                  TAMP_BKP14R_Msk

/********************  Bits definition for TAMP_BKP15R register  ***************/
#define TAMP_BKP15R_Pos              (0U)
#define TAMP_BKP15R_Msk              (0xFFFFFFFFUL << TAMP_BKP15R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP15R                  TAMP_BKP15R_Msk

/********************  Bits definition for TAMP_BKP16R register  ***************/
#define TAMP_BKP16R_Pos              (0U)
#define TAMP_BKP16R_Msk              (0xFFFFFFFFUL << TAMP_BKP16R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP16R                  TAMP_BKP16R_Msk

/********************  Bits definition for TAMP_BKP17R register  ***************/
#define TAMP_BKP17R_Pos              (0U)
#define TAMP_BKP17R_Msk              (0xFFFFFFFFUL << TAMP_BKP17R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP17R                  TAMP_BKP17R_Msk

/********************  Bits definition for TAMP_BKP18R register  ***************/
#define TAMP_BKP18R_Pos              (0U)
#define TAMP_BKP18R_Msk              (0xFFFFFFFFUL << TAMP_BKP18R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP18R                  TAMP_BKP18R_Msk

/********************  Bits definition for TAMP_BKP19R register  ***************/
#define TAMP_BKP19R_Pos              (0U)
#define TAMP_BKP19R_Msk              (0xFFFFFFFFUL << TAMP_BKP19R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP19R                  TAMP_BKP19R_Msk

/********************  Bits definition for TAMP_BKP20R register  ***************/
#define TAMP_BKP20R_Pos              (0U)
#define TAMP_BKP20R_Msk              (0xFFFFFFFFUL << TAMP_BKP20R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP20R                  TAMP_BKP20R_Msk

/********************  Bits definition for TAMP_BKP21R register  ***************/
#define TAMP_BKP21R_Pos              (0U)
#define TAMP_BKP21R_Msk              (0xFFFFFFFFUL << TAMP_BKP21R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP21R                  TAMP_BKP21R_Msk

/********************  Bits definition for TAMP_BKP22R register  ***************/
#define TAMP_BKP22R_Pos              (0U)
#define TAMP_BKP22R_Msk              (0xFFFFFFFFUL << TAMP_BKP22R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP22R                  TAMP_BKP22R_Msk

/********************  Bits definition for TAMP_BKP23R register  ***************/
#define TAMP_BKP23R_Pos              (0U)
#define TAMP_BKP23R_Msk              (0xFFFFFFFFUL << TAMP_BKP23R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP23R                  TAMP_BKP23R_Msk

/********************  Bits definition for TAMP_BKP24R register  ***************/
#define TAMP_BKP24R_Pos              (0U)
#define TAMP_BKP24R_Msk              (0xFFFFFFFFUL << TAMP_BKP24R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP24R                  TAMP_BKP24R_Msk

/********************  Bits definition for TAMP_BKP25R register  ***************/
#define TAMP_BKP25R_Pos              (0U)
#define TAMP_BKP25R_Msk              (0xFFFFFFFFUL << TAMP_BKP25R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP25R                  TAMP_BKP25R_Msk

/********************  Bits definition for TAMP_BKP26R register  ***************/
#define TAMP_BKP26R_Pos              (0U)
#define TAMP_BKP26R_Msk              (0xFFFFFFFFUL << TAMP_BKP26R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP26R                  TAMP_BKP26R_Msk

/********************  Bits definition for TAMP_BKP27R register  ***************/
#define TAMP_BKP27R_Pos              (0U)
#define TAMP_BKP27R_Msk              (0xFFFFFFFFUL << TAMP_BKP27R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP27R                  TAMP_BKP27R_Msk

/********************  Bits definition for TAMP_BKP28R register  ***************/
#define TAMP_BKP28R_Pos              (0U)
#define TAMP_BKP28R_Msk              (0xFFFFFFFFUL << TAMP_BKP28R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP28R                  TAMP_BKP28R_Msk

/********************  Bits definition for TAMP_BKP29R register  ***************/
#define TAMP_BKP29R_Pos              (0U)
#define TAMP_BKP29R_Msk              (0xFFFFFFFFUL << TAMP_BKP29R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP29R                  TAMP_BKP29R_Msk

/********************  Bits definition for TAMP_BKP30R register  ***************/
#define TAMP_BKP30R_Pos              (0U)
#define TAMP_BKP30R_Msk              (0xFFFFFFFFUL << TAMP_BKP30R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP30R                  TAMP_BKP30R_Msk

/********************  Bits definition for TAMP_BKP31R register  ***************/
#define TAMP_BKP31R_Pos              (0U)
#define TAMP_BKP31R_Msk              (0xFFFFFFFFUL << TAMP_BKP31R_Pos)          /*!< 0xFFFFFFFF */
#define TAMP_BKP31R                  TAMP_BKP31R_Msk

/******************** Number of backup registers ******************************/
#define RTC_BKP_NUMBER                       32U


/******************************************************************************/
/*                                                                            */
/*                        Serial Peripheral Interface (SPI)                   */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for SPI_CR1 register  ********************/
#define SPI_CR1_CPHA_Pos         (0U)
#define SPI_CR1_CPHA_Msk         (0x1UL << SPI_CR1_CPHA_Pos)                   /*!< 0x00000001 */
#define SPI_CR1_CPHA             SPI_CR1_CPHA_Msk                              /*!<Clock Phase      */
#define SPI_CR1_CPOL_Pos         (1U)
#define SPI_CR1_CPOL_Msk         (0x1UL << SPI_CR1_CPOL_Pos)                   /*!< 0x00000002 */
#define SPI_CR1_CPOL             SPI_CR1_CPOL_Msk                              /*!<Clock Polarity   */
#define SPI_CR1_MSTR_Pos         (2U)
#define SPI_CR1_MSTR_Msk         (0x1UL << SPI_CR1_MSTR_Pos)                   /*!< 0x00000004 */
#define SPI_CR1_MSTR             SPI_CR1_MSTR_Msk                              /*!<Master Selection */

#define SPI_CR1_BR_Pos           (3U)
#define SPI_CR1_BR_Msk           (0x7UL << SPI_CR1_BR_Pos)                     /*!< 0x00000038 */
#define SPI_CR1_BR               SPI_CR1_BR_Msk                                /*!<BR[2:0] bits (Baud Rate Control) */
#define SPI_CR1_BR_0             (0x1UL << SPI_CR1_BR_Pos)                     /*!< 0x00000008 */
#define SPI_CR1_BR_1             (0x2UL << SPI_CR1_BR_Pos)                     /*!< 0x00000010 */
#define SPI_CR1_BR_2             (0x4UL << SPI_CR1_BR_Pos)                     /*!< 0x00000020 */

#define SPI_CR1_SPE_Pos          (6U)
#define SPI_CR1_SPE_Msk          (0x1UL << SPI_CR1_SPE_Pos)                    /*!< 0x00000040 */
#define SPI_CR1_SPE              SPI_CR1_SPE_Msk                               /*!<SPI Enable                          */
#define SPI_CR1_LSBFIRST_Pos     (7U)
#define SPI_CR1_LSBFIRST_Msk     (0x1UL << SPI_CR1_LSBFIRST_Pos)               /*!< 0x00000080 */
#define SPI_CR1_LSBFIRST         SPI_CR1_LSBFIRST_Msk                          /*!<Frame Format                        */
#define SPI_CR1_SSI_Pos          (8U)
#define SPI_CR1_SSI_Msk          (0x1UL << SPI_CR1_SSI_Pos)                    /*!< 0x00000100 */
#define SPI_CR1_SSI              SPI_CR1_SSI_Msk                               /*!<Internal slave select               */
#define SPI_CR1_SSM_Pos          (9U)
#define SPI_CR1_SSM_Msk          (0x1UL << SPI_CR1_SSM_Pos)                    /*!< 0x00000200 */
#define SPI_CR1_SSM              SPI_CR1_SSM_Msk                               /*!<Software slave management           */
#define SPI_CR1_RXONLY_Pos       (10U)
#define SPI_CR1_RXONLY_Msk       (0x1UL << SPI_CR1_RXONLY_Pos)                 /*!< 0x00000400 */
#define SPI_CR1_RXONLY           SPI_CR1_RXONLY_Msk                            /*!<Receive only                        */
#define SPI_CR1_CRCL_Pos         (11U)
#define SPI_CR1_CRCL_Msk         (0x1UL << SPI_CR1_CRCL_Pos)                   /*!< 0x00000800 */
#define SPI_CR1_CRCL             SPI_CR1_CRCL_Msk                              /*!< CRC Length */
#define SPI_CR1_CRCNEXT_Pos      (12U)
#define SPI_CR1_CRCNEXT_Msk      (0x1UL << SPI_CR1_CRCNEXT_Pos)                /*!< 0x00001000 */
#define SPI_CR1_CRCNEXT          SPI_CR1_CRCNEXT_Msk                           /*!<Transmit CRC next                   */
#define SPI_CR1_CRCEN_Pos        (13U)
#define SPI_CR1_CRCEN_Msk        (0x1UL << SPI_CR1_CRCEN_Pos)                  /*!< 0x00002000 */
#define SPI_CR1_CRCEN            SPI_CR1_CRCEN_Msk                             /*!<Hardware CRC calculation enable     */
#define SPI_CR1_BIDIOE_Pos       (14U)
#define SPI_CR1_BIDIOE_Msk       (0x1UL << SPI_CR1_BIDIOE_Pos)                 /*!< 0x00004000 */
#define SPI_CR1_BIDIOE           SPI_CR1_BIDIOE_Msk                            /*!<Output enable in bidirectional mode */
#define SPI_CR1_BIDIMODE_Pos     (15U)
#define SPI_CR1_BIDIMODE_Msk     (0x1UL << SPI_CR1_BIDIMODE_Pos)               /*!< 0x00008000 */
#define SPI_CR1_BIDIMODE         SPI_CR1_BIDIMODE_Msk                          /*!<Bidirectional data mode enable      */

/*******************  Bit definition for SPI_CR2 register  ********************/
#define SPI_CR2_RXDMAEN_Pos      (0U)
#define SPI_CR2_RXDMAEN_Msk      (0x1UL << SPI_CR2_RXDMAEN_Pos)                /*!< 0x00000001 */
#define SPI_CR2_RXDMAEN          SPI_CR2_RXDMAEN_Msk                           /*!< Rx Buffer DMA Enable */
#define SPI_CR2_TXDMAEN_Pos      (1U)
#define SPI_CR2_TXDMAEN_Msk      (0x1UL << SPI_CR2_TXDMAEN_Pos)                /*!< 0x00000002 */
#define SPI_CR2_TXDMAEN          SPI_CR2_TXDMAEN_Msk                           /*!< Tx Buffer DMA Enable */
#define SPI_CR2_SSOE_Pos         (2U)
#define SPI_CR2_SSOE_Msk         (0x1UL << SPI_CR2_SSOE_Pos)                   /*!< 0x00000004 */
#define SPI_CR2_SSOE             SPI_CR2_SSOE_Msk                              /*!< SS Output Enable */
#define SPI_CR2_NSSP_Pos         (3U)
#define SPI_CR2_NSSP_Msk         (0x1UL << SPI_CR2_NSSP_Pos)                   /*!< 0x00000008 */
#define SPI_CR2_NSSP             SPI_CR2_NSSP_Msk                              /*!< NSS pulse management Enable */
#define SPI_CR2_FRF_Pos          (4U)
#define SPI_CR2_FRF_Msk          (0x1UL << SPI_CR2_FRF_Pos)                    /*!< 0x00000010 */
#define SPI_CR2_FRF              SPI_CR2_FRF_Msk                               /*!< Frame Format Enable */
#define SPI_CR2_ERRIE_Pos        (5U)
#define SPI_CR2_ERRIE_Msk        (0x1UL << SPI_CR2_ERRIE_Pos)                  /*!< 0x00000020 */
#define SPI_CR2_ERRIE            SPI_CR2_ERRIE_Msk                             /*!< Error Interrupt Enable */
#define SPI_CR2_RXNEIE_Pos       (6U)
#define SPI_CR2_RXNEIE_Msk       (0x1UL << SPI_CR2_RXNEIE_Pos)                 /*!< 0x00000040 */
#define SPI_CR2_RXNEIE           SPI_CR2_RXNEIE_Msk                            /*!< RX buffer Not Empty Interrupt Enable */
#define SPI_CR2_TXEIE_Pos        (7U)
#define SPI_CR2_TXEIE_Msk        (0x1UL << SPI_CR2_TXEIE_Pos)                  /*!< 0x00000080 */
#define SPI_CR2_TXEIE            SPI_CR2_TXEIE_Msk                             /*!< Tx buffer Empty Interrupt Enable */
#define SPI_CR2_DS_Pos           (8U)
#define SPI_CR2_DS_Msk           (0xFUL << SPI_CR2_DS_Pos)                     /*!< 0x00000F00 */
#define SPI_CR2_DS               SPI_CR2_DS_Msk                                /*!< DS[3:0] Data Size */
#define SPI_CR2_DS_0             (0x1UL << SPI_CR2_DS_Pos)                     /*!< 0x00000100 */
#define SPI_CR2_DS_1             (0x2UL << SPI_CR2_DS_Pos)                     /*!< 0x00000200 */
#define SPI_CR2_DS_2             (0x4UL << SPI_CR2_DS_Pos)                     /*!< 0x00000400 */
#define SPI_CR2_DS_3             (0x8UL << SPI_CR2_DS_Pos)                     /*!< 0x00000800 */
#define SPI_CR2_FRXTH_Pos        (12U)
#define SPI_CR2_FRXTH_Msk        (0x1UL << SPI_CR2_FRXTH_Pos)                  /*!< 0x00001000 */
#define SPI_CR2_FRXTH            SPI_CR2_FRXTH_Msk                             /*!< FIFO reception Threshold */
#define SPI_CR2_LDMARX_Pos       (13U)
#define SPI_CR2_LDMARX_Msk       (0x1UL << SPI_CR2_LDMARX_Pos)                 /*!< 0x00002000 */
#define SPI_CR2_LDMARX           SPI_CR2_LDMARX_Msk                            /*!< Last DMA transfer for reception */
#define SPI_CR2_LDMATX_Pos       (14U)
#define SPI_CR2_LDMATX_Msk       (0x1UL << SPI_CR2_LDMATX_Pos)                 /*!< 0x00004000 */
#define SPI_CR2_LDMATX           SPI_CR2_LDMATX_Msk                            /*!< Last DMA transfer for transmission */

/********************  Bit definition for SPI_SR register  ********************/
#define SPI_SR_RXNE_Pos          (0U)
#define SPI_SR_RXNE_Msk          (0x1UL << SPI_SR_RXNE_Pos)                    /*!< 0x00000001 */
#define SPI_SR_RXNE              SPI_SR_RXNE_Msk                               /*!< Receive buffer Not Empty */
#define SPI_SR_TXE_Pos           (1U)
#define SPI_SR_TXE_Msk           (0x1UL << SPI_SR_TXE_Pos)                     /*!< 0x00000002 */
#define SPI_SR_TXE               SPI_SR_TXE_Msk                                /*!< Transmit buffer Empty */
#define SPI_SR_CHSIDE_Pos        (2U)
#define SPI_SR_CHSIDE_Msk        (0x1UL << SPI_SR_CHSIDE_Pos)                  /*!< 0x00000004 */
#define SPI_SR_CHSIDE            SPI_SR_CHSIDE_Msk                             /*!< Channel side */
#define SPI_SR_UDR_Pos           (3U)
#define SPI_SR_UDR_Msk           (0x1UL << SPI_SR_UDR_Pos)                     /*!< 0x00000008 */
#define SPI_SR_UDR               SPI_SR_UDR_Msk                                /*!< Underrun flag */
#define SPI_SR_CRCERR_Pos        (4U)
#define SPI_SR_CRCERR_Msk        (0x1UL << SPI_SR_CRCERR_Pos)                  /*!< 0x00000010 */
#define SPI_SR_CRCERR            SPI_SR_CRCERR_Msk                             /*!< CRC Error flag */
#define SPI_SR_MODF_Pos          (5U)
#define SPI_SR_MODF_Msk          (0x1UL << SPI_SR_MODF_Pos)                    /*!< 0x00000020 */
#define SPI_SR_MODF              SPI_SR_MODF_Msk                               /*!< Mode fault */
#define SPI_SR_OVR_Pos           (6U)
#define SPI_SR_OVR_Msk           (0x1UL << SPI_SR_OVR_Pos)                     /*!< 0x00000040 */
#define SPI_SR_OVR               SPI_SR_OVR_Msk                                /*!< Overrun flag */
#define SPI_SR_BSY_Pos           (7U)
#define SPI_SR_BSY_Msk           (0x1UL << SPI_SR_BSY_Pos)                     /*!< 0x00000080 */
#define SPI_SR_BSY               SPI_SR_BSY_Msk                                /*!< Busy flag */
#define SPI_SR_FRE_Pos           (8U)
#define SPI_SR_FRE_Msk           (0x1UL << SPI_SR_FRE_Pos)                     /*!< 0x00000100 */
#define SPI_SR_FRE               SPI_SR_FRE_Msk                                /*!< TI frame format error */
#define SPI_SR_FRLVL_Pos         (9U)
#define SPI_SR_FRLVL_Msk         (0x3UL << SPI_SR_FRLVL_Pos)                   /*!< 0x00000600 */
#define SPI_SR_FRLVL             SPI_SR_FRLVL_Msk                              /*!< FIFO Reception Level */
#define SPI_SR_FRLVL_0           (0x1UL << SPI_SR_FRLVL_Pos)                   /*!< 0x00000200 */
#define SPI_SR_FRLVL_1           (0x2UL << SPI_SR_FRLVL_Pos)                   /*!< 0x00000400 */
#define SPI_SR_FTLVL_Pos         (11U)
#define SPI_SR_FTLVL_Msk         (0x3UL << SPI_SR_FTLVL_Pos)                   /*!< 0x00001800 */
#define SPI_SR_FTLVL             SPI_SR_FTLVL_Msk                              /*!< FIFO Transmission Level */
#define SPI_SR_FTLVL_0           (0x1UL << SPI_SR_FTLVL_Pos)                   /*!< 0x00000800 */
#define SPI_SR_FTLVL_1           (0x2UL << SPI_SR_FTLVL_Pos)                   /*!< 0x00001000 */

/********************  Bit definition for SPI_DR register  ********************/
#define SPI_DR_DR_Pos            (0U)
#define SPI_DR_DR_Msk            (0xFFFFUL << SPI_DR_DR_Pos)                   /*!< 0x0000FFFF */
#define SPI_DR_DR                SPI_DR_DR_Msk                                 /*!<Data Register           */

/*******************  Bit definition for SPI_CRCPR register  ******************/
#define SPI_CRCPR_CRCPOLY_Pos    (0U)
#define SPI_CRCPR_CRCPOLY_Msk    (0xFFFFUL << SPI_CRCPR_CRCPOLY_Pos)           /*!< 0x0000FFFF */
#define SPI_CRCPR_CRCPOLY        SPI_CRCPR_CRCPOLY_Msk                         /*!<CRC polynomial register */

/******************  Bit definition for SPI_RXCRCR register  ******************/
#define SPI_RXCRCR_RXCRC_Pos     (0U)
#define SPI_RXCRCR_RXCRC_Msk     (0xFFFFUL << SPI_RXCRCR_RXCRC_Pos)            /*!< 0x0000FFFF */
#define SPI_RXCRCR_RXCRC         SPI_RXCRCR_RXCRC_Msk                          /*!<Rx CRC Register         */

/******************  Bit definition for SPI_TXCRCR register  ******************/
#define SPI_TXCRCR_TXCRC_Pos     (0U)
#define SPI_TXCRCR_TXCRC_Msk     (0xFFFFUL << SPI_TXCRCR_TXCRC_Pos)            /*!< 0x0000FFFF */
#define SPI_TXCRCR_TXCRC         SPI_TXCRCR_TXCRC_Msk                          /*!<Tx CRC Register         */

/******************************************************************************/
/*                                                                            */
/*                                    QUADSPI                                 */
/*                                                                            */
/******************************************************************************/
/*****************  Bit definition for QUADSPI_CR register  *******************/
#define QUADSPI_CR_EN_Pos              (0U)
#define QUADSPI_CR_EN_Msk              (0x1UL << QUADSPI_CR_EN_Pos)            /*!< 0x00000001 */
#define QUADSPI_CR_EN                  QUADSPI_CR_EN_Msk                       /*!< Enable */
#define QUADSPI_CR_ABORT_Pos           (1U)
#define QUADSPI_CR_ABORT_Msk           (0x1UL << QUADSPI_CR_ABORT_Pos)         /*!< 0x00000002 */
#define QUADSPI_CR_ABORT               QUADSPI_CR_ABORT_Msk                    /*!< Abort request */
#define QUADSPI_CR_DMAEN_Pos           (2U)
#define QUADSPI_CR_DMAEN_Msk           (0x1UL << QUADSPI_CR_DMAEN_Pos)         /*!< 0x00000004 */
#define QUADSPI_CR_DMAEN               QUADSPI_CR_DMAEN_Msk                    /*!< DMA Enable */
#define QUADSPI_CR_TCEN_Pos            (3U)
#define QUADSPI_CR_TCEN_Msk            (0x1UL << QUADSPI_CR_TCEN_Pos)          /*!< 0x00000008 */
#define QUADSPI_CR_TCEN                QUADSPI_CR_TCEN_Msk                     /*!< Timeout Counter Enable */
#define QUADSPI_CR_SSHIFT_Pos          (4U)
#define QUADSPI_CR_SSHIFT_Msk          (0x1UL << QUADSPI_CR_SSHIFT_Pos)        /*!< 0x00000010 */
#define QUADSPI_CR_SSHIFT              QUADSPI_CR_SSHIFT_Msk                   /*!< Sample Shift */
#define QUADSPI_CR_DFM_Pos             (6U)
#define QUADSPI_CR_DFM_Msk             (0x1UL << QUADSPI_CR_DFM_Pos)           /*!< 0x00000040 */
#define QUADSPI_CR_DFM                 QUADSPI_CR_DFM_Msk                      /*!< Dual-flash mode */
#define QUADSPI_CR_FSEL_Pos            (7U)
#define QUADSPI_CR_FSEL_Msk            (0x1UL << QUADSPI_CR_FSEL_Pos)          /*!< 0x00000080 */
#define QUADSPI_CR_FSEL                QUADSPI_CR_FSEL_Msk                     /*!< Flash memory selection */
#define QUADSPI_CR_FTHRES_Pos          (8U)
#define QUADSPI_CR_FTHRES_Msk          (0xFUL << QUADSPI_CR_FTHRES_Pos)        /*!< 0x00000F00 */
#define QUADSPI_CR_FTHRES              QUADSPI_CR_FTHRES_Msk                   /*!< FTHRES[3:0] FIFO Level */
#define QUADSPI_CR_TEIE_Pos            (16U)
#define QUADSPI_CR_TEIE_Msk            (0x1UL << QUADSPI_CR_TEIE_Pos)          /*!< 0x00010000 */
#define QUADSPI_CR_TEIE                QUADSPI_CR_TEIE_Msk                     /*!< Transfer Error Interrupt Enable */
#define QUADSPI_CR_TCIE_Pos            (17U)
#define QUADSPI_CR_TCIE_Msk            (0x1UL << QUADSPI_CR_TCIE_Pos)          /*!< 0x00020000 */
#define QUADSPI_CR_TCIE                QUADSPI_CR_TCIE_Msk                     /*!< Transfer Complete Interrupt Enable */
#define QUADSPI_CR_FTIE_Pos            (18U)
#define QUADSPI_CR_FTIE_Msk            (0x1UL << QUADSPI_CR_FTIE_Pos)          /*!< 0x00040000 */
#define QUADSPI_CR_FTIE                QUADSPI_CR_FTIE_Msk                     /*!< FIFO Threshold Interrupt Enable */
#define QUADSPI_CR_SMIE_Pos            (19U)
#define QUADSPI_CR_SMIE_Msk            (0x1UL << QUADSPI_CR_SMIE_Pos)          /*!< 0x00080000 */
#define QUADSPI_CR_SMIE                QUADSPI_CR_SMIE_Msk                     /*!< Status Match Interrupt Enable */
#define QUADSPI_CR_TOIE_Pos            (20U)
#define QUADSPI_CR_TOIE_Msk            (0x1UL << QUADSPI_CR_TOIE_Pos)          /*!< 0x00100000 */
#define QUADSPI_CR_TOIE                QUADSPI_CR_TOIE_Msk                     /*!< TimeOut Interrupt Enable */
#define QUADSPI_CR_APMS_Pos            (22U)
#define QUADSPI_CR_APMS_Msk            (0x1UL << QUADSPI_CR_APMS_Pos)          /*!< 0x00400000 */
#define QUADSPI_CR_APMS                QUADSPI_CR_APMS_Msk                     /*!< Automatic Polling Mode Stop */
#define QUADSPI_CR_PMM_Pos             (23U)
#define QUADSPI_CR_PMM_Msk             (0x1UL << QUADSPI_CR_PMM_Pos)           /*!< 0x00800000 */
#define QUADSPI_CR_PMM                 QUADSPI_CR_PMM_Msk                      /*!< Polling Match Mode */
#define QUADSPI_CR_PRESCALER_Pos       (24U)
#define QUADSPI_CR_PRESCALER_Msk       (0xFFUL << QUADSPI_CR_PRESCALER_Pos)    /*!< 0xFF000000 */
#define QUADSPI_CR_PRESCALER           QUADSPI_CR_PRESCALER_Msk                /*!< PRESCALER[7:0] Clock prescaler */

/*****************  Bit definition for QUADSPI_DCR register  ******************/
#define QUADSPI_DCR_CKMODE_Pos         (0U)
#define QUADSPI_DCR_CKMODE_Msk         (0x1UL << QUADSPI_DCR_CKMODE_Pos)       /*!< 0x00000001 */
#define QUADSPI_DCR_CKMODE             QUADSPI_DCR_CKMODE_Msk                  /*!< Mode 0 / Mode 3 */
#define QUADSPI_DCR_CSHT_Pos           (8U)
#define QUADSPI_DCR_CSHT_Msk           (0x7UL << QUADSPI_DCR_CSHT_Pos)         /*!< 0x00000700 */
#define QUADSPI_DCR_CSHT               QUADSPI_DCR_CSHT_Msk                    /*!< CSHT[2:0]: ChipSelect High Time */
#define QUADSPI_DCR_CSHT_0             (0x1UL << QUADSPI_DCR_CSHT_Pos)         /*!< 0x00000100 */
#define QUADSPI_DCR_CSHT_1             (0x2UL << QUADSPI_DCR_CSHT_Pos)         /*!< 0x00000200 */
#define QUADSPI_DCR_CSHT_2             (0x4UL << QUADSPI_DCR_CSHT_Pos)         /*!< 0x00000400 */
#define QUADSPI_DCR_FSIZE_Pos          (16U)
#define QUADSPI_DCR_FSIZE_Msk          (0x1FUL << QUADSPI_DCR_FSIZE_Pos)       /*!< 0x001F0000 */
#define QUADSPI_DCR_FSIZE              QUADSPI_DCR_FSIZE_Msk                   /*!< FSIZE[4:0]: Flash Size */

/******************  Bit definition for QUADSPI_SR register  *******************/
#define QUADSPI_SR_TEF_Pos             (0U)
#define QUADSPI_SR_TEF_Msk             (0x1UL << QUADSPI_SR_TEF_Pos)           /*!< 0x00000001 */
#define QUADSPI_SR_TEF                 QUADSPI_SR_TEF_Msk                      /*!< Transfer Error Flag */
#define QUADSPI_SR_TCF_Pos             (1U)
#define QUADSPI_SR_TCF_Msk             (0x1UL << QUADSPI_SR_TCF_Pos)           /*!< 0x00000002 */
#define QUADSPI_SR_TCF                 QUADSPI_SR_TCF_Msk                      /*!< Transfer Complete Flag */
#define QUADSPI_SR_FTF_Pos             (2U)
#define QUADSPI_SR_FTF_Msk             (0x1UL << QUADSPI_SR_FTF_Pos)           /*!< 0x00000004 */
#define QUADSPI_SR_FTF                 QUADSPI_SR_FTF_Msk                      /*!< FIFO Threshlod Flag */
#define QUADSPI_SR_SMF_Pos             (3U)
#define QUADSPI_SR_SMF_Msk             (0x1UL << QUADSPI_SR_SMF_Pos)           /*!< 0x00000008 */
#define QUADSPI_SR_SMF                 QUADSPI_SR_SMF_Msk                      /*!< Status Match Flag */
#define QUADSPI_SR_TOF_Pos             (4U)
#define QUADSPI_SR_TOF_Msk             (0x1UL << QUADSPI_SR_TOF_Pos)           /*!< 0x00000010 */
#define QUADSPI_SR_TOF                 QUADSPI_SR_TOF_Msk                      /*!< Timeout Flag */
#define QUADSPI_SR_BUSY_Pos            (5U)
#define QUADSPI_SR_BUSY_Msk            (0x1UL << QUADSPI_SR_BUSY_Pos)          /*!< 0x00000020 */
#define QUADSPI_SR_BUSY                QUADSPI_SR_BUSY_Msk                     /*!< Busy */
#define QUADSPI_SR_FLEVEL_Pos          (8U)
#define QUADSPI_SR_FLEVEL_Msk          (0x1FUL << QUADSPI_SR_FLEVEL_Pos)       /*!< 0x00001F00 */
#define QUADSPI_SR_FLEVEL              QUADSPI_SR_FLEVEL_Msk                   /*!< FIFO Threshlod Flag */

/******************  Bit definition for QUADSPI_FCR register  ******************/
#define QUADSPI_FCR_CTEF_Pos           (0U)
#define QUADSPI_FCR_CTEF_Msk           (0x1UL << QUADSPI_FCR_CTEF_Pos)         /*!< 0x00000001 */
#define QUADSPI_FCR_CTEF               QUADSPI_FCR_CTEF_Msk                    /*!< Clear Transfer Error Flag */
#define QUADSPI_FCR_CTCF_Pos           (1U)
#define QUADSPI_FCR_CTCF_Msk           (0x1UL << QUADSPI_FCR_CTCF_Pos)         /*!< 0x00000002 */
#define QUADSPI_FCR_CTCF               QUADSPI_FCR_CTCF_Msk                    /*!< Clear Transfer Complete Flag */
#define QUADSPI_FCR_CSMF_Pos           (3U)
#define QUADSPI_FCR_CSMF_Msk           (0x1UL << QUADSPI_FCR_CSMF_Pos)         /*!< 0x00000008 */
#define QUADSPI_FCR_CSMF               QUADSPI_FCR_CSMF_Msk                    /*!< Clear Status Match Flag */
#define QUADSPI_FCR_CTOF_Pos           (4U)
#define QUADSPI_FCR_CTOF_Msk           (0x1UL << QUADSPI_FCR_CTOF_Pos)         /*!< 0x00000010 */
#define QUADSPI_FCR_CTOF               QUADSPI_FCR_CTOF_Msk                    /*!< Clear Timeout Flag */

/******************  Bit definition for QUADSPI_DLR register  ******************/
#define QUADSPI_DLR_DL_Pos             (0U)
#define QUADSPI_DLR_DL_Msk             (0xFFFFFFFFUL << QUADSPI_DLR_DL_Pos)    /*!< 0xFFFFFFFF */
#define QUADSPI_DLR_DL                 QUADSPI_DLR_DL_Msk                      /*!< DL[31:0]: Data Length */

/******************  Bit definition for QUADSPI_CCR register  ******************/
#define QUADSPI_CCR_INSTRUCTION_Pos    (0U)
#define QUADSPI_CCR_INSTRUCTION_Msk    (0xFFUL << QUADSPI_CCR_INSTRUCTION_Pos) /*!< 0x000000FF */
#define QUADSPI_CCR_INSTRUCTION        QUADSPI_CCR_INSTRUCTION_Msk             /*!< INSTRUCTION[7:0]: Instruction */
#define QUADSPI_CCR_IMODE_Pos          (8U)
#define QUADSPI_CCR_IMODE_Msk          (0x3UL << QUADSPI_CCR_IMODE_Pos)        /*!< 0x00000300 */
#define QUADSPI_CCR_IMODE              QUADSPI_CCR_IMODE_Msk                   /*!< IMODE[1:0]: Instruction Mode */
#define QUADSPI_CCR_IMODE_0            (0x1UL << QUADSPI_CCR_IMODE_Pos)        /*!< 0x00000100 */
#define QUADSPI_CCR_IMODE_1            (0x2UL << QUADSPI_CCR_IMODE_Pos)        /*!< 0x00000200 */
#define QUADSPI_CCR_ADMODE_Pos         (10U)
#define QUADSPI_CCR_ADMODE_Msk         (0x3UL << QUADSPI_CCR_ADMODE_Pos)       /*!< 0x00000C00 */
#define QUADSPI_CCR_ADMODE             QUADSPI_CCR_ADMODE_Msk                  /*!< ADMODE[1:0]: Address Mode */
#define QUADSPI_CCR_ADMODE_0           (0x1UL << QUADSPI_CCR_ADMODE_Pos)       /*!< 0x00000400 */
#define QUADSPI_CCR_ADMODE_1           (0x2UL << QUADSPI_CCR_ADMODE_Pos)       /*!< 0x00000800 */
#define QUADSPI_CCR_ADSIZE_Pos         (12U)
#define QUADSPI_CCR_ADSIZE_Msk         (0x3UL << QUADSPI_CCR_ADSIZE_Pos)       /*!< 0x00003000 */
#define QUADSPI_CCR_ADSIZE             QUADSPI_CCR_ADSIZE_Msk                  /*!< ADSIZE[1:0]: Address Size */
#define QUADSPI_CCR_ADSIZE_0           (0x1UL << QUADSPI_CCR_ADSIZE_Pos)       /*!< 0x00001000 */
#define QUADSPI_CCR_ADSIZE_1           (0x2UL << QUADSPI_CCR_ADSIZE_Pos)       /*!< 0x00002000 */
#define QUADSPI_CCR_ABMODE_Pos         (14U)
#define QUADSPI_CCR_ABMODE_Msk         (0x3UL << QUADSPI_CCR_ABMODE_Pos)       /*!< 0x0000C000 */
#define QUADSPI_CCR_ABMODE             QUADSPI_CCR_ABMODE_Msk                  /*!< ABMODE[1:0]: Alternate Bytes Mode */
#define QUADSPI_CCR_ABMODE_0           (0x1UL << QUADSPI_CCR_ABMODE_Pos)       /*!< 0x00004000 */
#define QUADSPI_CCR_ABMODE_1           (0x2UL << QUADSPI_CCR_ABMODE_Pos)       /*!< 0x00008000 */
#define QUADSPI_CCR_ABSIZE_Pos         (16U)
#define QUADSPI_CCR_ABSIZE_Msk         (0x3UL << QUADSPI_CCR_ABSIZE_Pos)       /*!< 0x00030000 */
#define QUADSPI_CCR_ABSIZE             QUADSPI_CCR_ABSIZE_Msk                  /*!< ABSIZE[1:0]: Instruction Mode */
#define QUADSPI_CCR_ABSIZE_0           (0x1UL << QUADSPI_CCR_ABSIZE_Pos)       /*!< 0x00010000 */
#define QUADSPI_CCR_ABSIZE_1           (0x2UL << QUADSPI_CCR_ABSIZE_Pos)       /*!< 0x00020000 */
#define QUADSPI_CCR_DCYC_Pos           (18U)
#define QUADSPI_CCR_DCYC_Msk           (0x1FUL << QUADSPI_CCR_DCYC_Pos)        /*!< 0x007C0000 */
#define QUADSPI_CCR_DCYC               QUADSPI_CCR_DCYC_Msk                    /*!< DCYC[4:0]: Dummy Cycles */
#define QUADSPI_CCR_DMODE_Pos          (24U)
#define QUADSPI_CCR_DMODE_Msk          (0x3UL << QUADSPI_CCR_DMODE_Pos)        /*!< 0x03000000 */
#define QUADSPI_CCR_DMODE              QUADSPI_CCR_DMODE_Msk                   /*!< DMODE[1:0]: Data Mode */
#define QUADSPI_CCR_DMODE_0            (0x1UL << QUADSPI_CCR_DMODE_Pos)        /*!< 0x01000000 */
#define QUADSPI_CCR_DMODE_1            (0x2UL << QUADSPI_CCR_DMODE_Pos)        /*!< 0x02000000 */
#define QUADSPI_CCR_FMODE_Pos          (26U)
#define QUADSPI_CCR_FMODE_Msk          (0x3UL << QUADSPI_CCR_FMODE_Pos)        /*!< 0x0C000000 */
#define QUADSPI_CCR_FMODE              QUADSPI_CCR_FMODE_Msk                   /*!< FMODE[1:0]: Functional Mode */
#define QUADSPI_CCR_FMODE_0            (0x1UL << QUADSPI_CCR_FMODE_Pos)        /*!< 0x04000000 */
#define QUADSPI_CCR_FMODE_1            (0x2UL << QUADSPI_CCR_FMODE_Pos)        /*!< 0x08000000 */
#define QUADSPI_CCR_SIOO_Pos           (28U)
#define QUADSPI_CCR_SIOO_Msk           (0x1UL << QUADSPI_CCR_SIOO_Pos)         /*!< 0x10000000 */
#define QUADSPI_CCR_SIOO               QUADSPI_CCR_SIOO_Msk                    /*!< SIOO: Send Instruction Only Once Mode */
#define QUADSPI_CCR_DHHC_Pos           (30U)
#define QUADSPI_CCR_DHHC_Msk           (0x1UL << QUADSPI_CCR_DHHC_Pos)         /*!< 0x40000000 */
#define QUADSPI_CCR_DHHC               QUADSPI_CCR_DHHC_Msk                    /*!< DHHC: DDR hold */
#define QUADSPI_CCR_DDRM_Pos           (31U)
#define QUADSPI_CCR_DDRM_Msk           (0x1UL << QUADSPI_CCR_DDRM_Pos)         /*!< 0x80000000 */
#define QUADSPI_CCR_DDRM               QUADSPI_CCR_DDRM_Msk                    /*!< DDRM: Double Data Rate Mode */

/******************  Bit definition for QUADSPI_AR register  *******************/
#define QUADSPI_AR_ADDRESS_Pos         (0U)
#define QUADSPI_AR_ADDRESS_Msk         (0xFFFFFFFFUL << QUADSPI_AR_ADDRESS_Pos) /*!< 0xFFFFFFFF */
#define QUADSPI_AR_ADDRESS             QUADSPI_AR_ADDRESS_Msk                  /*!< ADDRESS[31:0]: Address */

/******************  Bit definition for QUADSPI_ABR register  ******************/
#define QUADSPI_ABR_ALTERNATE_Pos      (0U)
#define QUADSPI_ABR_ALTERNATE_Msk      (0xFFFFFFFFUL << QUADSPI_ABR_ALTERNATE_Pos) /*!< 0xFFFFFFFF */
#define QUADSPI_ABR_ALTERNATE          QUADSPI_ABR_ALTERNATE_Msk               /*!< ALTERNATE[31:0]: Alternate Bytes */

/******************  Bit definition for QUADSPI_DR register  *******************/
#define QUADSPI_DR_DATA_Pos            (0U)
#define QUADSPI_DR_DATA_Msk            (0xFFFFFFFFUL << QUADSPI_DR_DATA_Pos)   /*!< 0xFFFFFFFF */
#define QUADSPI_DR_DATA                QUADSPI_DR_DATA_Msk                     /*!< DATA[31:0]: Data */

/******************  Bit definition for QUADSPI_PSMKR register  ****************/
#define QUADSPI_PSMKR_MASK_Pos         (0U)
#define QUADSPI_PSMKR_MASK_Msk         (0xFFFFFFFFUL << QUADSPI_PSMKR_MASK_Pos) /*!< 0xFFFFFFFF */
#define QUADSPI_PSMKR_MASK             QUADSPI_PSMKR_MASK_Msk                  /*!< MASK[31:0]: Status Mask */

/******************  Bit definition for QUADSPI_PSMAR register  ****************/
#define QUADSPI_PSMAR_MATCH_Pos        (0U)
#define QUADSPI_PSMAR_MATCH_Msk        (0xFFFFFFFFUL << QUADSPI_PSMAR_MATCH_Pos) /*!< 0xFFFFFFFF */
#define QUADSPI_PSMAR_MATCH            QUADSPI_PSMAR_MATCH_Msk                 /*!< MATCH[31:0]: Status Match */

/******************  Bit definition for QUADSPI_PIR register  *****************/
#define QUADSPI_PIR_INTERVAL_Pos       (0U)
#define QUADSPI_PIR_INTERVAL_Msk       (0xFFFFUL << QUADSPI_PIR_INTERVAL_Pos)  /*!< 0x0000FFFF */
#define QUADSPI_PIR_INTERVAL           QUADSPI_PIR_INTERVAL_Msk                /*!< INTERVAL[15:0]: Polling Interval */

/******************  Bit definition for QUADSPI_LPTR register  *****************/
#define QUADSPI_LPTR_TIMEOUT_Pos       (0U)
#define QUADSPI_LPTR_TIMEOUT_Msk       (0xFFFFUL << QUADSPI_LPTR_TIMEOUT_Pos)  /*!< 0x0000FFFF */
#define QUADSPI_LPTR_TIMEOUT           QUADSPI_LPTR_TIMEOUT_Msk                /*!< TIMEOUT[15:0]: Timeout period */

/******************************************************************************/
/*                                                                            */
/*                                 SYSCFG                                     */
/*                                                                            */
/******************************************************************************/
/******************  Bit definition for SYSCFG_MEMRMP register  ***************/
#define SYSCFG_MEMRMP_MEM_MODE_Pos      (0U)
#define SYSCFG_MEMRMP_MEM_MODE_Msk      (0x7UL << SYSCFG_MEMRMP_MEM_MODE_Pos)  /*!< 0x00000007 */
#define SYSCFG_MEMRMP_MEM_MODE          SYSCFG_MEMRMP_MEM_MODE_Msk             /*!< SYSCFG_Memory Remap Config */
#define SYSCFG_MEMRMP_MEM_MODE_0        (0x1UL << SYSCFG_MEMRMP_MEM_MODE_Pos)  /*!< 0x00000001 */
#define SYSCFG_MEMRMP_MEM_MODE_1        (0x2UL << SYSCFG_MEMRMP_MEM_MODE_Pos)  /*!< 0x00000002 */
#define SYSCFG_MEMRMP_MEM_MODE_2        (0x4UL << SYSCFG_MEMRMP_MEM_MODE_Pos)  /*!< 0x00000004 */

/******************  Bit definition for SYSCFG_CFGR1 register  ******************/
#define SYSCFG_CFGR1_FWDIS_Pos          (0U)
#define SYSCFG_CFGR1_FWDIS_Msk          (0x1UL << SYSCFG_CFGR1_FWDIS_Pos)      /*!< 0x00000001 */
#define SYSCFG_CFGR1_FWDIS              SYSCFG_CFGR1_FWDIS_Msk                 /*!< FIREWALL access enable*/
#define SYSCFG_CFGR1_BOOSTEN_Pos        (8U)
#define SYSCFG_CFGR1_BOOSTEN_Msk        (0x1UL << SYSCFG_CFGR1_BOOSTEN_Pos)    /*!< 0x00000100 */
#define SYSCFG_CFGR1_BOOSTEN            SYSCFG_CFGR1_BOOSTEN_Msk               /*!< I/O analog switch voltage booster enable */
#define SYSCFG_CFGR1_I2C_PB6_FMP_Pos    (16U)
#define SYSCFG_CFGR1_I2C_PB6_FMP_Msk    (0x1UL << SYSCFG_CFGR1_I2C_PB6_FMP_Pos) /*!< 0x00010000 */
#define SYSCFG_CFGR1_I2C_PB6_FMP        SYSCFG_CFGR1_I2C_PB6_FMP_Msk           /*!< I2C PB6 Fast mode plus */
#define SYSCFG_CFGR1_I2C_PB7_FMP_Pos    (17U)
#define SYSCFG_CFGR1_I2C_PB7_FMP_Msk    (0x1UL << SYSCFG_CFGR1_I2C_PB7_FMP_Pos) /*!< 0x00020000 */
#define SYSCFG_CFGR1_I2C_PB7_FMP        SYSCFG_CFGR1_I2C_PB7_FMP_Msk           /*!< I2C PB7 Fast mode plus */
#define SYSCFG_CFGR1_I2C_PB8_FMP_Pos    (18U)
#define SYSCFG_CFGR1_I2C_PB8_FMP_Msk    (0x1UL << SYSCFG_CFGR1_I2C_PB8_FMP_Pos) /*!< 0x00040000 */
#define SYSCFG_CFGR1_I2C_PB8_FMP        SYSCFG_CFGR1_I2C_PB8_FMP_Msk           /*!< I2C PB8 Fast mode plus */
#define SYSCFG_CFGR1_I2C_PB9_FMP_Pos    (19U)
#define SYSCFG_CFGR1_I2C_PB9_FMP_Msk    (0x1UL << SYSCFG_CFGR1_I2C_PB9_FMP_Pos) /*!< 0x00080000 */
#define SYSCFG_CFGR1_I2C_PB9_FMP        SYSCFG_CFGR1_I2C_PB9_FMP_Msk           /*!< I2C PB9 Fast mode plus */
#define SYSCFG_CFGR1_I2C1_FMP_Pos       (20U)
#define SYSCFG_CFGR1_I2C1_FMP_Msk       (0x1UL << SYSCFG_CFGR1_I2C1_FMP_Pos)   /*!< 0x00100000 */
#define SYSCFG_CFGR1_I2C1_FMP           SYSCFG_CFGR1_I2C1_FMP_Msk              /*!< I2C1 Fast mode plus */
#define SYSCFG_CFGR1_I2C2_FMP_Pos       (21U)
#define SYSCFG_CFGR1_I2C2_FMP_Msk       (0x1UL << SYSCFG_CFGR1_I2C2_FMP_Pos)   /*!< 0x00200000 */
#define SYSCFG_CFGR1_I2C2_FMP           SYSCFG_CFGR1_I2C2_FMP_Msk              /*!< I2C2 Fast mode plus */
#define SYSCFG_CFGR1_I2C3_FMP_Pos       (22U)
#define SYSCFG_CFGR1_I2C3_FMP_Msk       (0x1UL << SYSCFG_CFGR1_I2C3_FMP_Pos)   /*!< 0x00400000 */
#define SYSCFG_CFGR1_I2C3_FMP           SYSCFG_CFGR1_I2C3_FMP_Msk              /*!< I2C3 Fast mode plus */
#define SYSCFG_CFGR1_FPU_IE_0           (0x04000000UL)                         /*!<  Invalid operation Interrupt enable */
#define SYSCFG_CFGR1_FPU_IE_1           (0x08000000UL)                         /*!<  Divide-by-zero Interrupt enable */
#define SYSCFG_CFGR1_FPU_IE_2           (0x10000000UL)                         /*!<  Underflow Interrupt enable */
#define SYSCFG_CFGR1_FPU_IE_3           (0x20000000UL)                         /*!<  Overflow Interrupt enable */
#define SYSCFG_CFGR1_FPU_IE_4           (0x40000000UL)                         /*!<  Input denormal Interrupt enable */
#define SYSCFG_CFGR1_FPU_IE_5           (0x80000000UL)                         /*!<  Inexact Interrupt enable (interrupt disabled at reset) */

/*****************  Bit definition for SYSCFG_EXTICR1 register  ***************/
#define SYSCFG_EXTICR1_EXTI0_Pos        (0U)
#define SYSCFG_EXTICR1_EXTI0_Msk        (0x7UL << SYSCFG_EXTICR1_EXTI0_Pos)    /*!< 0x00000007 */
#define SYSCFG_EXTICR1_EXTI0            SYSCFG_EXTICR1_EXTI0_Msk               /*!<EXTI 0 configuration */
#define SYSCFG_EXTICR1_EXTI1_Pos        (4U)
#define SYSCFG_EXTICR1_EXTI1_Msk        (0x7UL << SYSCFG_EXTICR1_EXTI1_Pos)    /*!< 0x00000070 */
#define SYSCFG_EXTICR1_EXTI1            SYSCFG_EXTICR1_EXTI1_Msk               /*!<EXTI 1 configuration */
#define SYSCFG_EXTICR1_EXTI2_Pos        (8U)
#define SYSCFG_EXTICR1_EXTI2_Msk        (0x7UL << SYSCFG_EXTICR1_EXTI2_Pos)    /*!< 0x00000700 */
#define SYSCFG_EXTICR1_EXTI2            SYSCFG_EXTICR1_EXTI2_Msk               /*!<EXTI 2 configuration */
#define SYSCFG_EXTICR1_EXTI3_Pos        (12U)
#define SYSCFG_EXTICR1_EXTI3_Msk        (0x7UL << SYSCFG_EXTICR1_EXTI3_Pos)    /*!< 0x00007000 */
#define SYSCFG_EXTICR1_EXTI3            SYSCFG_EXTICR1_EXTI3_Msk               /*!<EXTI 3 configuration */

/**
  * @brief   EXTI0 configuration
  */
#define SYSCFG_EXTICR1_EXTI0_PA             (0x00000000UL)                     /*!<PA[0] pin */
#define SYSCFG_EXTICR1_EXTI0_PB             (0x00000001UL)                     /*!<PB[0] pin */
#define SYSCFG_EXTICR1_EXTI0_PC             (0x00000002UL)                     /*!<PC[0] pin */
#define SYSCFG_EXTICR1_EXTI0_PD             (0x00000003UL)                     /*!<PD[0] pin */
#define SYSCFG_EXTICR1_EXTI0_PH             (0x00000007UL)                     /*!<PH[0] pin */

/**
  * @brief   EXTI1 configuration
  */
#define SYSCFG_EXTICR1_EXTI1_PA             (0x00000000UL)                     /*!<PA[1] pin */
#define SYSCFG_EXTICR1_EXTI1_PB             (0x00000010UL)                     /*!<PB[1] pin */
#define SYSCFG_EXTICR1_EXTI1_PC             (0x00000020UL)                     /*!<PC[1] pin */
#define SYSCFG_EXTICR1_EXTI1_PD             (0x00000030UL)                     /*!<PD[1] pin */
#define SYSCFG_EXTICR1_EXTI1_PH             (0x00000070UL)                     /*!<PH[1] pin */

/**
  * @brief   EXTI2 configuration
  */
#define SYSCFG_EXTICR1_EXTI2_PA             (0x00000000UL)                     /*!<PA[2] pin */
#define SYSCFG_EXTICR1_EXTI2_PB             (0x00000100UL)                     /*!<PB[2] pin */
#define SYSCFG_EXTICR1_EXTI2_PC             (0x00000200UL)                     /*!<PC[2] pin */
#define SYSCFG_EXTICR1_EXTI2_PD             (0x00000300UL)                     /*!<PD[2] pin */

/**
  * @brief   EXTI3 configuration
  */
#define SYSCFG_EXTICR1_EXTI3_PA             (0x00000000UL)                     /*!<PA[3] pin */
#define SYSCFG_EXTICR1_EXTI3_PB             (0x00001000UL)                     /*!<PB[3] pin */
#define SYSCFG_EXTICR1_EXTI3_PC             (0x00002000UL)                     /*!<PC[3] pin */
#define SYSCFG_EXTICR1_EXTI3_PD             (0x00003000UL)                     /*!<PD[3] pin */
#define SYSCFG_EXTICR1_EXTI3_PH             (0x00007000UL)                     /*!<PH[3] pin */

/*****************  Bit definition for SYSCFG_EXTICR2 register  ***************/
#define SYSCFG_EXTICR2_EXTI4_Pos        (0U)
#define SYSCFG_EXTICR2_EXTI4_Msk        (0x7UL << SYSCFG_EXTICR2_EXTI4_Pos)    /*!< 0x00000007 */
#define SYSCFG_EXTICR2_EXTI4            SYSCFG_EXTICR2_EXTI4_Msk               /*!<EXTI 4 configuration */
#define SYSCFG_EXTICR2_EXTI5_Pos        (4U)
#define SYSCFG_EXTICR2_EXTI5_Msk        (0x7UL << SYSCFG_EXTICR2_EXTI5_Pos)    /*!< 0x00000070 */
#define SYSCFG_EXTICR2_EXTI5            SYSCFG_EXTICR2_EXTI5_Msk               /*!<EXTI 5 configuration */
#define SYSCFG_EXTICR2_EXTI6_Pos        (8U)
#define SYSCFG_EXTICR2_EXTI6_Msk        (0x7UL << SYSCFG_EXTICR2_EXTI6_Pos)    /*!< 0x00000700 */
#define SYSCFG_EXTICR2_EXTI6            SYSCFG_EXTICR2_EXTI6_Msk               /*!<EXTI 6 configuration */
#define SYSCFG_EXTICR2_EXTI7_Pos        (12U)
#define SYSCFG_EXTICR2_EXTI7_Msk        (0x7UL << SYSCFG_EXTICR2_EXTI7_Pos)    /*!< 0x00007000 */
#define SYSCFG_EXTICR2_EXTI7            SYSCFG_EXTICR2_EXTI7_Msk               /*!<EXTI 7 configuration */
/**
  * @brief   EXTI4 configuration
  */
#define SYSCFG_EXTICR2_EXTI4_PA             (0x00000000UL)                     /*!<PA[4] pin */
#define SYSCFG_EXTICR2_EXTI4_PB             (0x00000001UL)                     /*!<PB[4] pin */
#define SYSCFG_EXTICR2_EXTI4_PC             (0x00000002UL)                     /*!<PC[4] pin */
#define SYSCFG_EXTICR2_EXTI4_PD             (0x00000003UL)                     /*!<PD[4] pin */

/**
  * @brief   EXTI5 configuration
  */
#define SYSCFG_EXTICR2_EXTI5_PA             (0x00000000UL)                     /*!<PA[5] pin */
#define SYSCFG_EXTICR2_EXTI5_PB             (0x00000010UL)                     /*!<PB[5] pin */
#define SYSCFG_EXTICR2_EXTI5_PC             (0x00000020UL)                     /*!<PC[5] pin */
#define SYSCFG_EXTICR2_EXTI5_PD             (0x00000030UL)                     /*!<PD[5] pin */

/**
  * @brief   EXTI6 configuration
  */
#define SYSCFG_EXTICR2_EXTI6_PA             (0x00000000UL)                     /*!<PA[6] pin */
#define SYSCFG_EXTICR2_EXTI6_PB             (0x00000100UL)                     /*!<PB[6] pin */
#define SYSCFG_EXTICR2_EXTI6_PC             (0x00000200UL)                     /*!<PC[6] pin */
#define SYSCFG_EXTICR2_EXTI6_PD             (0x00000300UL)                     /*!<PD[6] pin */

/**
  * @brief   EXTI7 configuration
  */
#define SYSCFG_EXTICR2_EXTI7_PA             (0x00000000UL)                     /*!<PA[7] pin */
#define SYSCFG_EXTICR2_EXTI7_PB             (0x00001000UL)                     /*!<PB[7] pin */
#define SYSCFG_EXTICR2_EXTI7_PC             (0x00002000UL)                     /*!<PC[7] pin */
#define SYSCFG_EXTICR2_EXTI7_PD             (0x00003000UL)                     /*!<PD[7] pin */

/*****************  Bit definition for SYSCFG_EXTICR3 register  ***************/
#define SYSCFG_EXTICR3_EXTI8_Pos        (0U)
#define SYSCFG_EXTICR3_EXTI8_Msk        (0x7UL << SYSCFG_EXTICR3_EXTI8_Pos)    /*!< 0x00000007 */
#define SYSCFG_EXTICR3_EXTI8            SYSCFG_EXTICR3_EXTI8_Msk               /*!<EXTI 8 configuration */
#define SYSCFG_EXTICR3_EXTI9_Pos        (4U)
#define SYSCFG_EXTICR3_EXTI9_Msk        (0x7UL << SYSCFG_EXTICR3_EXTI9_Pos)    /*!< 0x00000070 */
#define SYSCFG_EXTICR3_EXTI9            SYSCFG_EXTICR3_EXTI9_Msk               /*!<EXTI 9 configuration */
#define SYSCFG_EXTICR3_EXTI10_Pos       (8U)
#define SYSCFG_EXTICR3_EXTI10_Msk       (0x7UL << SYSCFG_EXTICR3_EXTI10_Pos)   /*!< 0x00000700 */
#define SYSCFG_EXTICR3_EXTI10           SYSCFG_EXTICR3_EXTI10_Msk              /*!<EXTI 10 configuration */
#define SYSCFG_EXTICR3_EXTI11_Pos       (12U)
#define SYSCFG_EXTICR3_EXTI11_Msk       (0x7UL << SYSCFG_EXTICR3_EXTI11_Pos)   /*!< 0x00007000 */
#define SYSCFG_EXTICR3_EXTI11           SYSCFG_EXTICR3_EXTI11_Msk              /*!<EXTI 11 configuration */

/**
  * @brief   EXTI8 configuration
  */
#define SYSCFG_EXTICR3_EXTI8_PA             (0x00000000UL)                     /*!<PA[8] pin */
#define SYSCFG_EXTICR3_EXTI8_PB             (0x00000001UL)                     /*!<PB[8] pin */
#define SYSCFG_EXTICR3_EXTI8_PC             (0x00000002UL)                     /*!<PC[8] pin */
#define SYSCFG_EXTICR3_EXTI8_PD             (0x00000003UL)                     /*!<PD[8] pin */

/**
  * @brief   EXTI9 configuration
  */
#define SYSCFG_EXTICR3_EXTI9_PA             (0x00000000UL)                     /*!<PA[9] pin */
#define SYSCFG_EXTICR3_EXTI9_PB             (0x00000010UL)                     /*!<PB[9] pin */
#define SYSCFG_EXTICR3_EXTI9_PC             (0x00000020UL)                     /*!<PC[9] pin */
#define SYSCFG_EXTICR3_EXTI9_PD             (0x00000030UL)                     /*!<PD[9] pin */

/**
  * @brief   EXTI10 configuration
  */
#define SYSCFG_EXTICR3_EXTI10_PA            (0x00000000UL)                     /*!<PA[10] pin */
#define SYSCFG_EXTICR3_EXTI10_PB            (0x00000100UL)                     /*!<PB[10] pin */
#define SYSCFG_EXTICR3_EXTI10_PC            (0x00000200UL)                     /*!<PC[10] pin */
#define SYSCFG_EXTICR3_EXTI10_PD            (0x00000300UL)                     /*!<PD[10] pin */

/**
  * @brief   EXTI11 configuration
  */
#define SYSCFG_EXTICR3_EXTI11_PA            (0x00000000UL)                     /*!<PA[11] pin */
#define SYSCFG_EXTICR3_EXTI11_PB            (0x00001000UL)                     /*!<PB[11] pin */
#define SYSCFG_EXTICR3_EXTI11_PC            (0x00002000UL)                     /*!<PC[11] pin */
#define SYSCFG_EXTICR3_EXTI11_PD            (0x00003000UL)                     /*!<PD[11] pin */

/*****************  Bit definition for SYSCFG_EXTICR4 register  ***************/
#define SYSCFG_EXTICR4_EXTI12_Pos       (0U)
#define SYSCFG_EXTICR4_EXTI12_Msk       (0x7UL << SYSCFG_EXTICR4_EXTI12_Pos)   /*!< 0x00000007 */
#define SYSCFG_EXTICR4_EXTI12           SYSCFG_EXTICR4_EXTI12_Msk              /*!<EXTI 12 configuration */
#define SYSCFG_EXTICR4_EXTI13_Pos       (4U)
#define SYSCFG_EXTICR4_EXTI13_Msk       (0x7UL << SYSCFG_EXTICR4_EXTI13_Pos)   /*!< 0x00000070 */
#define SYSCFG_EXTICR4_EXTI13           SYSCFG_EXTICR4_EXTI13_Msk              /*!<EXTI 13 configuration */
#define SYSCFG_EXTICR4_EXTI14_Pos       (8U)
#define SYSCFG_EXTICR4_EXTI14_Msk       (0x7UL << SYSCFG_EXTICR4_EXTI14_Pos)   /*!< 0x00000700 */
#define SYSCFG_EXTICR4_EXTI14           SYSCFG_EXTICR4_EXTI14_Msk              /*!<EXTI 14 configuration */
#define SYSCFG_EXTICR4_EXTI15_Pos       (12U)
#define SYSCFG_EXTICR4_EXTI15_Msk       (0x7UL << SYSCFG_EXTICR4_EXTI15_Pos)   /*!< 0x00007000 */
#define SYSCFG_EXTICR4_EXTI15           SYSCFG_EXTICR4_EXTI15_Msk              /*!<EXTI 15 configuration */

/**
  * @brief   EXTI12 configuration
  */
#define SYSCFG_EXTICR4_EXTI12_PA            (0x00000000UL)                     /*!<PA[12] pin */
#define SYSCFG_EXTICR4_EXTI12_PB            (0x00000001UL)                     /*!<PB[12] pin */
#define SYSCFG_EXTICR4_EXTI12_PC            (0x00000002UL)                     /*!<PC[12] pin */
#define SYSCFG_EXTICR4_EXTI12_PD            (0x00000003UL)                     /*!<PD[12] pin */

/**
  * @brief   EXTI13 configuration
  */
#define SYSCFG_EXTICR4_EXTI13_PA            (0x00000000UL)                     /*!<PA[13] pin */
#define SYSCFG_EXTICR4_EXTI13_PB            (0x00000010UL)                     /*!<PB[13] pin */
#define SYSCFG_EXTICR4_EXTI13_PC            (0x00000020UL)                     /*!<PC[13] pin */
#define SYSCFG_EXTICR4_EXTI13_PD            (0x00000030UL)                     /*!<PD[13] pin */

/**
  * @brief   EXTI14 configuration
  */
#define SYSCFG_EXTICR4_EXTI14_PA            (0x00000000UL)                     /*!<PA[14] pin */
#define SYSCFG_EXTICR4_EXTI14_PB            (0x00000100UL)                     /*!<PB[14] pin */
#define SYSCFG_EXTICR4_EXTI14_PC            (0x00000200UL)                     /*!<PC[14] pin */
#define SYSCFG_EXTICR4_EXTI14_PD            (0x00000300UL)                     /*!<PD[14] pin */

/**
  * @brief   EXTI15 configuration
  */
#define SYSCFG_EXTICR4_EXTI15_PA            (0x00000000UL)                     /*!<PA[15] pin */
#define SYSCFG_EXTICR4_EXTI15_PB            (0x00001000UL)                     /*!<PB[15] pin */
#define SYSCFG_EXTICR4_EXTI15_PC            (0x00002000UL)                     /*!<PC[15] pin */
#define SYSCFG_EXTICR4_EXTI15_PD            (0x00003000UL)                     /*!<PD[15] pin */

/******************  Bit definition for SYSCFG_SCSR register  ****************/
#define SYSCFG_SCSR_SRAM2ER_Pos         (0U)
#define SYSCFG_SCSR_SRAM2ER_Msk         (0x1UL << SYSCFG_SCSR_SRAM2ER_Pos)     /*!< 0x00000001 */
#define SYSCFG_SCSR_SRAM2ER             SYSCFG_SCSR_SRAM2ER_Msk                /*!< SRAM2 Erase Request */
#define SYSCFG_SCSR_SRAM2BSY_Pos        (1U)
#define SYSCFG_SCSR_SRAM2BSY_Msk        (0x1UL << SYSCFG_SCSR_SRAM2BSY_Pos)    /*!< 0x00000002 */
#define SYSCFG_SCSR_SRAM2BSY            SYSCFG_SCSR_SRAM2BSY_Msk               /*!< SRAM2 Erase Ongoing */

/******************  Bit definition for SYSCFG_CFGR2 register  ****************/
#define SYSCFG_CFGR2_CLL_Pos            (0U)
#define SYSCFG_CFGR2_CLL_Msk            (0x1UL << SYSCFG_CFGR2_CLL_Pos)        /*!< 0x00000001 */
#define SYSCFG_CFGR2_CLL                SYSCFG_CFGR2_CLL_Msk                   /*!< Core Lockup Lock */
#define SYSCFG_CFGR2_SPL_Pos            (1U)
#define SYSCFG_CFGR2_SPL_Msk            (0x1UL << SYSCFG_CFGR2_SPL_Pos)        /*!< 0x00000002 */
#define SYSCFG_CFGR2_SPL                SYSCFG_CFGR2_SPL_Msk                   /*!< SRAM Parity Lock*/
#define SYSCFG_CFGR2_PVDL_Pos           (2U)
#define SYSCFG_CFGR2_PVDL_Msk           (0x1UL << SYSCFG_CFGR2_PVDL_Pos)       /*!< 0x00000004 */
#define SYSCFG_CFGR2_PVDL               SYSCFG_CFGR2_PVDL_Msk                  /*!<  PVD Lock */
#define SYSCFG_CFGR2_ECCL_Pos           (3U)
#define SYSCFG_CFGR2_ECCL_Msk           (0x1UL << SYSCFG_CFGR2_ECCL_Pos)       /*!< 0x00000008 */
#define SYSCFG_CFGR2_ECCL               SYSCFG_CFGR2_ECCL_Msk                  /*!< ECC Lock*/
#define SYSCFG_CFGR2_SPF_Pos            (8U)
#define SYSCFG_CFGR2_SPF_Msk            (0x1UL << SYSCFG_CFGR2_SPF_Pos)        /*!< 0x00000100 */
#define SYSCFG_CFGR2_SPF                SYSCFG_CFGR2_SPF_Msk                   /*!< SRAM Parity Flag */

/******************  Bit definition for SYSCFG_SWPR register  ****************/
#define SYSCFG_SWPR_PAGE0_Pos           (0U)
#define SYSCFG_SWPR_PAGE0_Msk           (0x1UL << SYSCFG_SWPR_PAGE0_Pos)       /*!< 0x00000001 */
#define SYSCFG_SWPR_PAGE0               SYSCFG_SWPR_PAGE0_Msk                  /*!< SRAM2 Write protection page 0 */
#define SYSCFG_SWPR_PAGE1_Pos           (1U)
#define SYSCFG_SWPR_PAGE1_Msk           (0x1UL << SYSCFG_SWPR_PAGE1_Pos)       /*!< 0x00000002 */
#define SYSCFG_SWPR_PAGE1               SYSCFG_SWPR_PAGE1_Msk                  /*!< SRAM2 Write protection page 1 */
#define SYSCFG_SWPR_PAGE2_Pos           (2U)
#define SYSCFG_SWPR_PAGE2_Msk           (0x1UL << SYSCFG_SWPR_PAGE2_Pos)       /*!< 0x00000004 */
#define SYSCFG_SWPR_PAGE2               SYSCFG_SWPR_PAGE2_Msk                  /*!< SRAM2 Write protection page 2 */
#define SYSCFG_SWPR_PAGE3_Pos           (3U)
#define SYSCFG_SWPR_PAGE3_Msk           (0x1UL << SYSCFG_SWPR_PAGE3_Pos)       /*!< 0x00000008 */
#define SYSCFG_SWPR_PAGE3               SYSCFG_SWPR_PAGE3_Msk                  /*!< SRAM2 Write protection page 3 */
#define SYSCFG_SWPR_PAGE4_Pos           (4U)
#define SYSCFG_SWPR_PAGE4_Msk           (0x1UL << SYSCFG_SWPR_PAGE4_Pos)       /*!< 0x00000010 */
#define SYSCFG_SWPR_PAGE4               SYSCFG_SWPR_PAGE4_Msk                  /*!< SRAM2 Write protection page 4 */
#define SYSCFG_SWPR_PAGE5_Pos           (5U)
#define SYSCFG_SWPR_PAGE5_Msk           (0x1UL << SYSCFG_SWPR_PAGE5_Pos)       /*!< 0x00000020 */
#define SYSCFG_SWPR_PAGE5               SYSCFG_SWPR_PAGE5_Msk                  /*!< SRAM2 Write protection page 5 */
#define SYSCFG_SWPR_PAGE6_Pos           (6U)
#define SYSCFG_SWPR_PAGE6_Msk           (0x1UL << SYSCFG_SWPR_PAGE6_Pos)       /*!< 0x00000040 */
#define SYSCFG_SWPR_PAGE6               SYSCFG_SWPR_PAGE6_Msk                  /*!< SRAM2 Write protection page 6 */
#define SYSCFG_SWPR_PAGE7_Pos           (7U)
#define SYSCFG_SWPR_PAGE7_Msk           (0x1UL << SYSCFG_SWPR_PAGE7_Pos)       /*!< 0x00000080 */
#define SYSCFG_SWPR_PAGE7               SYSCFG_SWPR_PAGE7_Msk                  /*!< SRAM2 Write protection page 7 */
#define SYSCFG_SWPR_PAGE8_Pos           (8U)
#define SYSCFG_SWPR_PAGE8_Msk           (0x1UL << SYSCFG_SWPR_PAGE8_Pos)       /*!< 0x00000100 */
#define SYSCFG_SWPR_PAGE8               SYSCFG_SWPR_PAGE8_Msk                  /*!< SRAM2 Write protection page 8 */
#define SYSCFG_SWPR_PAGE9_Pos           (9U)
#define SYSCFG_SWPR_PAGE9_Msk           (0x1UL << SYSCFG_SWPR_PAGE9_Pos)       /*!< 0x00000200 */
#define SYSCFG_SWPR_PAGE9               SYSCFG_SWPR_PAGE9_Msk                  /*!< SRAM2 Write protection page 9 */
#define SYSCFG_SWPR_PAGE10_Pos          (10U)
#define SYSCFG_SWPR_PAGE10_Msk          (0x1UL << SYSCFG_SWPR_PAGE10_Pos)      /*!< 0x00000400 */
#define SYSCFG_SWPR_PAGE10              SYSCFG_SWPR_PAGE10_Msk                 /*!< SRAM2 Write protection page 10*/
#define SYSCFG_SWPR_PAGE11_Pos          (11U)
#define SYSCFG_SWPR_PAGE11_Msk          (0x1UL << SYSCFG_SWPR_PAGE11_Pos)      /*!< 0x00000800 */
#define SYSCFG_SWPR_PAGE11              SYSCFG_SWPR_PAGE11_Msk                 /*!< SRAM2 Write protection page 11*/
#define SYSCFG_SWPR_PAGE12_Pos          (12U)
#define SYSCFG_SWPR_PAGE12_Msk          (0x1UL << SYSCFG_SWPR_PAGE12_Pos)      /*!< 0x00001000 */
#define SYSCFG_SWPR_PAGE12              SYSCFG_SWPR_PAGE12_Msk                 /*!< SRAM2 Write protection page 12*/
#define SYSCFG_SWPR_PAGE13_Pos          (13U)
#define SYSCFG_SWPR_PAGE13_Msk          (0x1UL << SYSCFG_SWPR_PAGE13_Pos)      /*!< 0x00002000 */
#define SYSCFG_SWPR_PAGE13              SYSCFG_SWPR_PAGE13_Msk                 /*!< SRAM2 Write protection page 13*/
#define SYSCFG_SWPR_PAGE14_Pos          (14U)
#define SYSCFG_SWPR_PAGE14_Msk          (0x1UL << SYSCFG_SWPR_PAGE14_Pos)      /*!< 0x00004000 */
#define SYSCFG_SWPR_PAGE14              SYSCFG_SWPR_PAGE14_Msk                 /*!< SRAM2 Write protection page 14*/
#define SYSCFG_SWPR_PAGE15_Pos          (15U)
#define SYSCFG_SWPR_PAGE15_Msk          (0x1UL << SYSCFG_SWPR_PAGE15_Pos)      /*!< 0x00008000 */
#define SYSCFG_SWPR_PAGE15              SYSCFG_SWPR_PAGE15_Msk                 /*!< SRAM2 Write protection page 15*/

/******************  Bit definition for SYSCFG_SKR register  ****************/
#define SYSCFG_SKR_KEY_Pos              (0U)
#define SYSCFG_SKR_KEY_Msk              (0xFFUL << SYSCFG_SKR_KEY_Pos)         /*!< 0x000000FF */
#define SYSCFG_SKR_KEY                  SYSCFG_SKR_KEY_Msk                     /*!<  SRAM2 write protection key for software erase  */




/******************************************************************************/
/*                                                                            */
/*                                    TIM                                     */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for TIM_CR1 register  ********************/
#define TIM_CR1_CEN_Pos           (0U)
#define TIM_CR1_CEN_Msk           (0x1UL << TIM_CR1_CEN_Pos)                   /*!< 0x00000001 */
#define TIM_CR1_CEN               TIM_CR1_CEN_Msk                              /*!<Counter enable */
#define TIM_CR1_UDIS_Pos          (1U)
#define TIM_CR1_UDIS_Msk          (0x1UL << TIM_CR1_UDIS_Pos)                  /*!< 0x00000002 */
#define TIM_CR1_UDIS              TIM_CR1_UDIS_Msk                             /*!<Update disable */
#define TIM_CR1_URS_Pos           (2U)
#define TIM_CR1_URS_Msk           (0x1UL << TIM_CR1_URS_Pos)                   /*!< 0x00000004 */
#define TIM_CR1_URS               TIM_CR1_URS_Msk                              /*!<Update request source */
#define TIM_CR1_OPM_Pos           (3U)
#define TIM_CR1_OPM_Msk           (0x1UL << TIM_CR1_OPM_Pos)                   /*!< 0x00000008 */
#define TIM_CR1_OPM               TIM_CR1_OPM_Msk                              /*!<One pulse mode */
#define TIM_CR1_DIR_Pos           (4U)
#define TIM_CR1_DIR_Msk           (0x1UL << TIM_CR1_DIR_Pos)                   /*!< 0x00000010 */
#define TIM_CR1_DIR               TIM_CR1_DIR_Msk                              /*!<Direction */

#define TIM_CR1_CMS_Pos           (5U)
#define TIM_CR1_CMS_Msk           (0x3UL << TIM_CR1_CMS_Pos)                   /*!< 0x00000060 */
#define TIM_CR1_CMS               TIM_CR1_CMS_Msk                              /*!<CMS[1:0] bits (Center-aligned mode selection) */
#define TIM_CR1_CMS_0             (0x1UL << TIM_CR1_CMS_Pos)                   /*!< 0x00000020 */
#define TIM_CR1_CMS_1             (0x2UL << TIM_CR1_CMS_Pos)                   /*!< 0x00000040 */

#define TIM_CR1_ARPE_Pos          (7U)
#define TIM_CR1_ARPE_Msk          (0x1UL << TIM_CR1_ARPE_Pos)                  /*!< 0x00000080 */
#define TIM_CR1_ARPE              TIM_CR1_ARPE_Msk                             /*!<Auto-reload preload enable */

#define TIM_CR1_CKD_Pos           (8U)
#define TIM_CR1_CKD_Msk           (0x3UL << TIM_CR1_CKD_Pos)                   /*!< 0x00000300 */
#define TIM_CR1_CKD               TIM_CR1_CKD_Msk                              /*!<CKD[1:0] bits (clock division) */
#define TIM_CR1_CKD_0             (0x1UL << TIM_CR1_CKD_Pos)                   /*!< 0x00000100 */
#define TIM_CR1_CKD_1             (0x2UL << TIM_CR1_CKD_Pos)                   /*!< 0x00000200 */

#define TIM_CR1_UIFREMAP_Pos      (11U)
#define TIM_CR1_UIFREMAP_Msk      (0x1UL << TIM_CR1_UIFREMAP_Pos)              /*!< 0x00000800 */
#define TIM_CR1_UIFREMAP          TIM_CR1_UIFREMAP_Msk                         /*!<Update interrupt flag remap */

/*******************  Bit definition for TIM_CR2 register  ********************/
#define TIM_CR2_CCPC_Pos          (0U)
#define TIM_CR2_CCPC_Msk          (0x1UL << TIM_CR2_CCPC_Pos)                  /*!< 0x00000001 */
#define TIM_CR2_CCPC              TIM_CR2_CCPC_Msk                             /*!<Capture/Compare Preloaded Control */
#define TIM_CR2_CCUS_Pos          (2U)
#define TIM_CR2_CCUS_Msk          (0x1UL << TIM_CR2_CCUS_Pos)                  /*!< 0x00000004 */
#define TIM_CR2_CCUS              TIM_CR2_CCUS_Msk                             /*!<Capture/Compare Control Update Selection */
#define TIM_CR2_CCDS_Pos          (3U)
#define TIM_CR2_CCDS_Msk          (0x1UL << TIM_CR2_CCDS_Pos)                  /*!< 0x00000008 */
#define TIM_CR2_CCDS              TIM_CR2_CCDS_Msk                             /*!<Capture/Compare DMA Selection */

#define TIM_CR2_MMS_Pos           (4U)
#define TIM_CR2_MMS_Msk           (0x7UL << TIM_CR2_MMS_Pos)                   /*!< 0x00000070 */
#define TIM_CR2_MMS               TIM_CR2_MMS_Msk                              /*!<MMS[2:0] bits (Master Mode Selection) */
#define TIM_CR2_MMS_0             (0x1UL << TIM_CR2_MMS_Pos)                   /*!< 0x00000010 */
#define TIM_CR2_MMS_1             (0x2UL << TIM_CR2_MMS_Pos)                   /*!< 0x00000020 */
#define TIM_CR2_MMS_2             (0x4UL << TIM_CR2_MMS_Pos)                   /*!< 0x00000040 */

#define TIM_CR2_TI1S_Pos          (7U)
#define TIM_CR2_TI1S_Msk          (0x1UL << TIM_CR2_TI1S_Pos)                  /*!< 0x00000080 */
#define TIM_CR2_TI1S              TIM_CR2_TI1S_Msk                             /*!<TI1 Selection */
#define TIM_CR2_OIS1_Pos          (8U)
#define TIM_CR2_OIS1_Msk          (0x1UL << TIM_CR2_OIS1_Pos)                  /*!< 0x00000100 */
#define TIM_CR2_OIS1              TIM_CR2_OIS1_Msk                             /*!<Output Idle state 1 (OC1 output) */
#define TIM_CR2_OIS1N_Pos         (9U)
#define TIM_CR2_OIS1N_Msk         (0x1UL << TIM_CR2_OIS1N_Pos)                 /*!< 0x00000200 */
#define TIM_CR2_OIS1N             TIM_CR2_OIS1N_Msk                            /*!<Output Idle state 1 (OC1N output) */
#define TIM_CR2_OIS2_Pos          (10U)
#define TIM_CR2_OIS2_Msk          (0x1UL << TIM_CR2_OIS2_Pos)                  /*!< 0x00000400 */
#define TIM_CR2_OIS2              TIM_CR2_OIS2_Msk                             /*!<Output Idle state 2 (OC2 output) */
#define TIM_CR2_OIS2N_Pos         (11U)
#define TIM_CR2_OIS2N_Msk         (0x1UL << TIM_CR2_OIS2N_Pos)                 /*!< 0x00000800 */
#define TIM_CR2_OIS2N             TIM_CR2_OIS2N_Msk                            /*!<Output Idle state 2 (OC2N output) */
#define TIM_CR2_OIS3_Pos          (12U)
#define TIM_CR2_OIS3_Msk          (0x1UL << TIM_CR2_OIS3_Pos)                  /*!< 0x00001000 */
#define TIM_CR2_OIS3              TIM_CR2_OIS3_Msk                             /*!<Output Idle state 3 (OC3 output) */
#define TIM_CR2_OIS3N_Pos         (13U)
#define TIM_CR2_OIS3N_Msk         (0x1UL << TIM_CR2_OIS3N_Pos)                 /*!< 0x00002000 */
#define TIM_CR2_OIS3N             TIM_CR2_OIS3N_Msk                            /*!<Output Idle state 3 (OC3N output) */
#define TIM_CR2_OIS4_Pos          (14U)
#define TIM_CR2_OIS4_Msk          (0x1UL << TIM_CR2_OIS4_Pos)                  /*!< 0x00004000 */
#define TIM_CR2_OIS4              TIM_CR2_OIS4_Msk                             /*!<Output Idle state 4 (OC4 output) */
#define TIM_CR2_OIS5_Pos          (16U)
#define TIM_CR2_OIS5_Msk          (0x1UL << TIM_CR2_OIS5_Pos)                  /*!< 0x00010000 */
#define TIM_CR2_OIS5              TIM_CR2_OIS5_Msk                             /*!<Output Idle state 5 (OC5 output) */
#define TIM_CR2_OIS6_Pos          (18U)
#define TIM_CR2_OIS6_Msk          (0x1UL << TIM_CR2_OIS6_Pos)                  /*!< 0x00040000 */
#define TIM_CR2_OIS6              TIM_CR2_OIS6_Msk                             /*!<Output Idle state 6 (OC6 output) */

#define TIM_CR2_MMS2_Pos          (20U)
#define TIM_CR2_MMS2_Msk          (0xFUL << TIM_CR2_MMS2_Pos)                  /*!< 0x00F00000 */
#define TIM_CR2_MMS2              TIM_CR2_MMS2_Msk                             /*!<MMS[2:0] bits (Master Mode Selection) */
#define TIM_CR2_MMS2_0            (0x1UL << TIM_CR2_MMS2_Pos)                  /*!< 0x00100000 */
#define TIM_CR2_MMS2_1            (0x2UL << TIM_CR2_MMS2_Pos)                  /*!< 0x00200000 */
#define TIM_CR2_MMS2_2            (0x4UL << TIM_CR2_MMS2_Pos)                  /*!< 0x00400000 */
#define TIM_CR2_MMS2_3            (0x8UL << TIM_CR2_MMS2_Pos)                  /*!< 0x00800000 */

/*******************  Bit definition for TIM_SMCR register  *******************/
#define TIM_SMCR_SMS_Pos          (0U)
#define TIM_SMCR_SMS_Msk          (0x10007UL << TIM_SMCR_SMS_Pos)              /*!< 0x00010007 */
#define TIM_SMCR_SMS              TIM_SMCR_SMS_Msk                             /*!<SMS[2:0] bits (Slave mode selection) */
#define TIM_SMCR_SMS_0            (0x00001UL << TIM_SMCR_SMS_Pos)              /*!< 0x00000001 */
#define TIM_SMCR_SMS_1            (0x00002UL << TIM_SMCR_SMS_Pos)              /*!< 0x00000002 */
#define TIM_SMCR_SMS_2            (0x00004UL << TIM_SMCR_SMS_Pos)              /*!< 0x00000004 */
#define TIM_SMCR_SMS_3            (0x10000UL << TIM_SMCR_SMS_Pos)              /*!< 0x00010000 */

#define TIM_SMCR_OCCS_Pos         (3U)
#define TIM_SMCR_OCCS_Msk         (0x1UL << TIM_SMCR_OCCS_Pos)                 /*!< 0x00000008 */
#define TIM_SMCR_OCCS             TIM_SMCR_OCCS_Msk                            /*!< OCREF clear selection */

#define TIM_SMCR_TS_Pos           (4U)
#define TIM_SMCR_TS_Msk           (0x7UL << TIM_SMCR_TS_Pos)                   /*!< 0x00000070 */
#define TIM_SMCR_TS               TIM_SMCR_TS_Msk                              /*!<TS[2:0] bits (Trigger selection) */
#define TIM_SMCR_TS_0             (0x1UL << TIM_SMCR_TS_Pos)                   /*!< 0x00000010 */
#define TIM_SMCR_TS_1             (0x2UL << TIM_SMCR_TS_Pos)                   /*!< 0x00000020 */
#define TIM_SMCR_TS_2             (0x4UL << TIM_SMCR_TS_Pos)                   /*!< 0x00000040 */

#define TIM_SMCR_MSM_Pos          (7U)
#define TIM_SMCR_MSM_Msk          (0x1UL << TIM_SMCR_MSM_Pos)                  /*!< 0x00000080 */
#define TIM_SMCR_MSM              TIM_SMCR_MSM_Msk                             /*!<Master/slave mode */

#define TIM_SMCR_ETF_Pos          (8U)
#define TIM_SMCR_ETF_Msk          (0xFUL << TIM_SMCR_ETF_Pos)                  /*!< 0x00000F00 */
#define TIM_SMCR_ETF              TIM_SMCR_ETF_Msk                             /*!<ETF[3:0] bits (External trigger filter) */
#define TIM_SMCR_ETF_0            (0x1UL << TIM_SMCR_ETF_Pos)                  /*!< 0x00000100 */
#define TIM_SMCR_ETF_1            (0x2UL << TIM_SMCR_ETF_Pos)                  /*!< 0x00000200 */
#define TIM_SMCR_ETF_2            (0x4UL << TIM_SMCR_ETF_Pos)                  /*!< 0x00000400 */
#define TIM_SMCR_ETF_3            (0x8UL << TIM_SMCR_ETF_Pos)                  /*!< 0x00000800 */

#define TIM_SMCR_ETPS_Pos         (12U)
#define TIM_SMCR_ETPS_Msk         (0x3UL << TIM_SMCR_ETPS_Pos)                 /*!< 0x00003000 */
#define TIM_SMCR_ETPS             TIM_SMCR_ETPS_Msk                            /*!<ETPS[1:0] bits (External trigger prescaler) */
#define TIM_SMCR_ETPS_0           (0x1UL << TIM_SMCR_ETPS_Pos)                 /*!< 0x00001000 */
#define TIM_SMCR_ETPS_1           (0x2UL << TIM_SMCR_ETPS_Pos)                 /*!< 0x00002000 */

#define TIM_SMCR_ECE_Pos          (14U)
#define TIM_SMCR_ECE_Msk          (0x1UL << TIM_SMCR_ECE_Pos)                  /*!< 0x00004000 */
#define TIM_SMCR_ECE              TIM_SMCR_ECE_Msk                             /*!<External clock enable */
#define TIM_SMCR_ETP_Pos          (15U)
#define TIM_SMCR_ETP_Msk          (0x1UL << TIM_SMCR_ETP_Pos)                  /*!< 0x00008000 */
#define TIM_SMCR_ETP              TIM_SMCR_ETP_Msk                             /*!<External trigger polarity */

/*******************  Bit definition for TIM_DIER register  *******************/
#define TIM_DIER_UIE_Pos          (0U)
#define TIM_DIER_UIE_Msk          (0x1UL << TIM_DIER_UIE_Pos)                  /*!< 0x00000001 */
#define TIM_DIER_UIE              TIM_DIER_UIE_Msk                             /*!<Update interrupt enable */
#define TIM_DIER_CC1IE_Pos        (1U)
#define TIM_DIER_CC1IE_Msk        (0x1UL << TIM_DIER_CC1IE_Pos)                /*!< 0x00000002 */
#define TIM_DIER_CC1IE            TIM_DIER_CC1IE_Msk                           /*!<Capture/Compare 1 interrupt enable */
#define TIM_DIER_CC2IE_Pos        (2U)
#define TIM_DIER_CC2IE_Msk        (0x1UL << TIM_DIER_CC2IE_Pos)                /*!< 0x00000004 */
#define TIM_DIER_CC2IE            TIM_DIER_CC2IE_Msk                           /*!<Capture/Compare 2 interrupt enable */
#define TIM_DIER_CC3IE_Pos        (3U)
#define TIM_DIER_CC3IE_Msk        (0x1UL << TIM_DIER_CC3IE_Pos)                /*!< 0x00000008 */
#define TIM_DIER_CC3IE            TIM_DIER_CC3IE_Msk                           /*!<Capture/Compare 3 interrupt enable */
#define TIM_DIER_CC4IE_Pos        (4U)
#define TIM_DIER_CC4IE_Msk        (0x1UL << TIM_DIER_CC4IE_Pos)                /*!< 0x00000010 */
#define TIM_DIER_CC4IE            TIM_DIER_CC4IE_Msk                           /*!<Capture/Compare 4 interrupt enable */
#define TIM_DIER_COMIE_Pos        (5U)
#define TIM_DIER_COMIE_Msk        (0x1UL << TIM_DIER_COMIE_Pos)                /*!< 0x00000020 */
#define TIM_DIER_COMIE            TIM_DIER_COMIE_Msk                           /*!<COM interrupt enable */
#define TIM_DIER_TIE_Pos          (6U)
#define TIM_DIER_TIE_Msk          (0x1UL << TIM_DIER_TIE_Pos)                  /*!< 0x00000040 */
#define TIM_DIER_TIE              TIM_DIER_TIE_Msk                             /*!<Trigger interrupt enable */
#define TIM_DIER_BIE_Pos          (7U)
#define TIM_DIER_BIE_Msk          (0x1UL << TIM_DIER_BIE_Pos)                  /*!< 0x00000080 */
#define TIM_DIER_BIE              TIM_DIER_BIE_Msk                             /*!<Break interrupt enable */
#define TIM_DIER_UDE_Pos          (8U)
#define TIM_DIER_UDE_Msk          (0x1UL << TIM_DIER_UDE_Pos)                  /*!< 0x00000100 */
#define TIM_DIER_UDE              TIM_DIER_UDE_Msk                             /*!<Update DMA request enable */
#define TIM_DIER_CC1DE_Pos        (9U)
#define TIM_DIER_CC1DE_Msk        (0x1UL << TIM_DIER_CC1DE_Pos)                /*!< 0x00000200 */
#define TIM_DIER_CC1DE            TIM_DIER_CC1DE_Msk                           /*!<Capture/Compare 1 DMA request enable */
#define TIM_DIER_CC2DE_Pos        (10U)
#define TIM_DIER_CC2DE_Msk        (0x1UL << TIM_DIER_CC2DE_Pos)                /*!< 0x00000400 */
#define TIM_DIER_CC2DE            TIM_DIER_CC2DE_Msk                           /*!<Capture/Compare 2 DMA request enable */
#define TIM_DIER_CC3DE_Pos        (11U)
#define TIM_DIER_CC3DE_Msk        (0x1UL << TIM_DIER_CC3DE_Pos)                /*!< 0x00000800 */
#define TIM_DIER_CC3DE            TIM_DIER_CC3DE_Msk                           /*!<Capture/Compare 3 DMA request enable */
#define TIM_DIER_CC4DE_Pos        (12U)
#define TIM_DIER_CC4DE_Msk        (0x1UL << TIM_DIER_CC4DE_Pos)                /*!< 0x00001000 */
#define TIM_DIER_CC4DE            TIM_DIER_CC4DE_Msk                           /*!<Capture/Compare 4 DMA request enable */
#define TIM_DIER_COMDE_Pos        (13U)
#define TIM_DIER_COMDE_Msk        (0x1UL << TIM_DIER_COMDE_Pos)                /*!< 0x00002000 */
#define TIM_DIER_COMDE            TIM_DIER_COMDE_Msk                           /*!<COM DMA request enable */
#define TIM_DIER_TDE_Pos          (14U)
#define TIM_DIER_TDE_Msk          (0x1UL << TIM_DIER_TDE_Pos)                  /*!< 0x00004000 */
#define TIM_DIER_TDE              TIM_DIER_TDE_Msk                             /*!<Trigger DMA request enable */

/********************  Bit definition for TIM_SR register  ********************/
#define TIM_SR_UIF_Pos            (0U)
#define TIM_SR_UIF_Msk            (0x1UL << TIM_SR_UIF_Pos)                    /*!< 0x00000001 */
#define TIM_SR_UIF                TIM_SR_UIF_Msk                               /*!<Update interrupt Flag */
#define TIM_SR_CC1IF_Pos          (1U)
#define TIM_SR_CC1IF_Msk          (0x1UL << TIM_SR_CC1IF_Pos)                  /*!< 0x00000002 */
#define TIM_SR_CC1IF              TIM_SR_CC1IF_Msk                             /*!<Capture/Compare 1 interrupt Flag */
#define TIM_SR_CC2IF_Pos          (2U)
#define TIM_SR_CC2IF_Msk          (0x1UL << TIM_SR_CC2IF_Pos)                  /*!< 0x00000004 */
#define TIM_SR_CC2IF              TIM_SR_CC2IF_Msk                             /*!<Capture/Compare 2 interrupt Flag */
#define TIM_SR_CC3IF_Pos          (3U)
#define TIM_SR_CC3IF_Msk          (0x1UL << TIM_SR_CC3IF_Pos)                  /*!< 0x00000008 */
#define TIM_SR_CC3IF              TIM_SR_CC3IF_Msk                             /*!<Capture/Compare 3 interrupt Flag */
#define TIM_SR_CC4IF_Pos          (4U)
#define TIM_SR_CC4IF_Msk          (0x1UL << TIM_SR_CC4IF_Pos)                  /*!< 0x00000010 */
#define TIM_SR_CC4IF              TIM_SR_CC4IF_Msk                             /*!<Capture/Compare 4 interrupt Flag */
#define TIM_SR_COMIF_Pos          (5U)
#define TIM_SR_COMIF_Msk          (0x1UL << TIM_SR_COMIF_Pos)                  /*!< 0x00000020 */
#define TIM_SR_COMIF              TIM_SR_COMIF_Msk                             /*!<COM interrupt Flag */
#define TIM_SR_TIF_Pos            (6U)
#define TIM_SR_TIF_Msk            (0x1UL << TIM_SR_TIF_Pos)                    /*!< 0x00000040 */
#define TIM_SR_TIF                TIM_SR_TIF_Msk                               /*!<Trigger interrupt Flag */
#define TIM_SR_BIF_Pos            (7U)
#define TIM_SR_BIF_Msk            (0x1UL << TIM_SR_BIF_Pos)                    /*!< 0x00000080 */
#define TIM_SR_BIF                TIM_SR_BIF_Msk                               /*!<Break interrupt Flag */
#define TIM_SR_B2IF_Pos           (8U)
#define TIM_SR_B2IF_Msk           (0x1UL << TIM_SR_B2IF_Pos)                   /*!< 0x00000100 */
#define TIM_SR_B2IF               TIM_SR_B2IF_Msk                              /*!<Break 2 interrupt Flag */
#define TIM_SR_CC1OF_Pos          (9U)
#define TIM_SR_CC1OF_Msk          (0x1UL << TIM_SR_CC1OF_Pos)                  /*!< 0x00000200 */
#define TIM_SR_CC1OF              TIM_SR_CC1OF_Msk                             /*!<Capture/Compare 1 Overcapture Flag */
#define TIM_SR_CC2OF_Pos          (10U)
#define TIM_SR_CC2OF_Msk          (0x1UL << TIM_SR_CC2OF_Pos)                  /*!< 0x00000400 */
#define TIM_SR_CC2OF              TIM_SR_CC2OF_Msk                             /*!<Capture/Compare 2 Overcapture Flag */
#define TIM_SR_CC3OF_Pos          (11U)
#define TIM_SR_CC3OF_Msk          (0x1UL << TIM_SR_CC3OF_Pos)                  /*!< 0x00000800 */
#define TIM_SR_CC3OF              TIM_SR_CC3OF_Msk                             /*!<Capture/Compare 3 Overcapture Flag */
#define TIM_SR_CC4OF_Pos          (12U)
#define TIM_SR_CC4OF_Msk          (0x1UL << TIM_SR_CC4OF_Pos)                  /*!< 0x00001000 */
#define TIM_SR_CC4OF              TIM_SR_CC4OF_Msk                             /*!<Capture/Compare 4 Overcapture Flag */
#define TIM_SR_SBIF_Pos           (13U)
#define TIM_SR_SBIF_Msk           (0x1UL << TIM_SR_SBIF_Pos)                   /*!< 0x00002000 */
#define TIM_SR_SBIF               TIM_SR_SBIF_Msk                              /*!<System Break interrupt Flag */
#define TIM_SR_CC5IF_Pos          (16U)
#define TIM_SR_CC5IF_Msk          (0x1UL << TIM_SR_CC5IF_Pos)                  /*!< 0x00010000 */
#define TIM_SR_CC5IF              TIM_SR_CC5IF_Msk                             /*!<Capture/Compare 5 interrupt Flag */
#define TIM_SR_CC6IF_Pos          (17U)
#define TIM_SR_CC6IF_Msk          (0x1UL << TIM_SR_CC6IF_Pos)                  /*!< 0x00020000 */
#define TIM_SR_CC6IF              TIM_SR_CC6IF_Msk                             /*!<Capture/Compare 6 interrupt Flag */


/*******************  Bit definition for TIM_EGR register  ********************/
#define TIM_EGR_UG_Pos            (0U)
#define TIM_EGR_UG_Msk            (0x1UL << TIM_EGR_UG_Pos)                    /*!< 0x00000001 */
#define TIM_EGR_UG                TIM_EGR_UG_Msk                               /*!<Update Generation */
#define TIM_EGR_CC1G_Pos          (1U)
#define TIM_EGR_CC1G_Msk          (0x1UL << TIM_EGR_CC1G_Pos)                  /*!< 0x00000002 */
#define TIM_EGR_CC1G              TIM_EGR_CC1G_Msk                             /*!<Capture/Compare 1 Generation */
#define TIM_EGR_CC2G_Pos          (2U)
#define TIM_EGR_CC2G_Msk          (0x1UL << TIM_EGR_CC2G_Pos)                  /*!< 0x00000004 */
#define TIM_EGR_CC2G              TIM_EGR_CC2G_Msk                             /*!<Capture/Compare 2 Generation */
#define TIM_EGR_CC3G_Pos          (3U)
#define TIM_EGR_CC3G_Msk          (0x1UL << TIM_EGR_CC3G_Pos)                  /*!< 0x00000008 */
#define TIM_EGR_CC3G              TIM_EGR_CC3G_Msk                             /*!<Capture/Compare 3 Generation */
#define TIM_EGR_CC4G_Pos          (4U)
#define TIM_EGR_CC4G_Msk          (0x1UL << TIM_EGR_CC4G_Pos)                  /*!< 0x00000010 */
#define TIM_EGR_CC4G              TIM_EGR_CC4G_Msk                             /*!<Capture/Compare 4 Generation */
#define TIM_EGR_COMG_Pos          (5U)
#define TIM_EGR_COMG_Msk          (0x1UL << TIM_EGR_COMG_Pos)                  /*!< 0x00000020 */
#define TIM_EGR_COMG              TIM_EGR_COMG_Msk                             /*!<Capture/Compare Control Update Generation */
#define TIM_EGR_TG_Pos            (6U)
#define TIM_EGR_TG_Msk            (0x1UL << TIM_EGR_TG_Pos)                    /*!< 0x00000040 */
#define TIM_EGR_TG                TIM_EGR_TG_Msk                               /*!<Trigger Generation */
#define TIM_EGR_BG_Pos            (7U)
#define TIM_EGR_BG_Msk            (0x1UL << TIM_EGR_BG_Pos)                    /*!< 0x00000080 */
#define TIM_EGR_BG                TIM_EGR_BG_Msk                               /*!<Break Generation */
#define TIM_EGR_B2G_Pos           (8U)
#define TIM_EGR_B2G_Msk           (0x1UL << TIM_EGR_B2G_Pos)                   /*!< 0x00000100 */
#define TIM_EGR_B2G               TIM_EGR_B2G_Msk                              /*!<Break 2 Generation */


/******************  Bit definition for TIM_CCMR1 register  *******************/
#define TIM_CCMR1_CC1S_Pos        (0U)
#define TIM_CCMR1_CC1S_Msk        (0x3UL << TIM_CCMR1_CC1S_Pos)                /*!< 0x00000003 */
#define TIM_CCMR1_CC1S            TIM_CCMR1_CC1S_Msk                           /*!<CC1S[1:0] bits (Capture/Compare 1 Selection) */
#define TIM_CCMR1_CC1S_0          (0x1UL << TIM_CCMR1_CC1S_Pos)                /*!< 0x00000001 */
#define TIM_CCMR1_CC1S_1          (0x2UL << TIM_CCMR1_CC1S_Pos)                /*!< 0x00000002 */

#define TIM_CCMR1_OC1FE_Pos       (2U)
#define TIM_CCMR1_OC1FE_Msk       (0x1UL << TIM_CCMR1_OC1FE_Pos)               /*!< 0x00000004 */
#define TIM_CCMR1_OC1FE           TIM_CCMR1_OC1FE_Msk                          /*!<Output Compare 1 Fast enable */
#define TIM_CCMR1_OC1PE_Pos       (3U)
#define TIM_CCMR1_OC1PE_Msk       (0x1UL << TIM_CCMR1_OC1PE_Pos)               /*!< 0x00000008 */
#define TIM_CCMR1_OC1PE           TIM_CCMR1_OC1PE_Msk                          /*!<Output Compare 1 Preload enable */

#define TIM_CCMR1_OC1M_Pos        (4U)
#define TIM_CCMR1_OC1M_Msk        (0x1007UL << TIM_CCMR1_OC1M_Pos)             /*!< 0x00010070 */
#define TIM_CCMR1_OC1M            TIM_CCMR1_OC1M_Msk                           /*!<OC1M[2:0] bits (Output Compare 1 Mode) */
#define TIM_CCMR1_OC1M_0          (0x0001UL << TIM_CCMR1_OC1M_Pos)             /*!< 0x00000010 */
#define TIM_CCMR1_OC1M_1          (0x0002UL << TIM_CCMR1_OC1M_Pos)             /*!< 0x00000020 */
#define TIM_CCMR1_OC1M_2          (0x0004UL << TIM_CCMR1_OC1M_Pos)             /*!< 0x00000040 */
#define TIM_CCMR1_OC1M_3          (0x1000UL << TIM_CCMR1_OC1M_Pos)             /*!< 0x00010000 */

#define TIM_CCMR1_OC1CE_Pos       (7U)
#define TIM_CCMR1_OC1CE_Msk       (0x1UL << TIM_CCMR1_OC1CE_Pos)               /*!< 0x00000080 */
#define TIM_CCMR1_OC1CE           TIM_CCMR1_OC1CE_Msk                          /*!<Output Compare 1 Clear Enable */

#define TIM_CCMR1_CC2S_Pos        (8U)
#define TIM_CCMR1_CC2S_Msk        (0x3UL << TIM_CCMR1_CC2S_Pos)                /*!< 0x00000300 */
#define TIM_CCMR1_CC2S            TIM_CCMR1_CC2S_Msk                           /*!<CC2S[1:0] bits (Capture/Compare 2 Selection) */
#define TIM_CCMR1_CC2S_0          (0x1UL << TIM_CCMR1_CC2S_Pos)                /*!< 0x00000100 */
#define TIM_CCMR1_CC2S_1          (0x2UL << TIM_CCMR1_CC2S_Pos)                /*!< 0x00000200 */

#define TIM_CCMR1_OC2FE_Pos       (10U)
#define TIM_CCMR1_OC2FE_Msk       (0x1UL << TIM_CCMR1_OC2FE_Pos)               /*!< 0x00000400 */
#define TIM_CCMR1_OC2FE           TIM_CCMR1_OC2FE_Msk                          /*!<Output Compare 2 Fast enable */
#define TIM_CCMR1_OC2PE_Pos       (11U)
#define TIM_CCMR1_OC2PE_Msk       (0x1UL << TIM_CCMR1_OC2PE_Pos)               /*!< 0x00000800 */
#define TIM_CCMR1_OC2PE           TIM_CCMR1_OC2PE_Msk                          /*!<Output Compare 2 Preload enable */

#define TIM_CCMR1_OC2M_Pos        (12U)
#define TIM_CCMR1_OC2M_Msk        (0x1007UL << TIM_CCMR1_OC2M_Pos)             /*!< 0x01007000 */
#define TIM_CCMR1_OC2M            TIM_CCMR1_OC2M_Msk                           /*!<OC2M[2:0] bits (Output Compare 2 Mode) */
#define TIM_CCMR1_OC2M_0          (0x0001UL << TIM_CCMR1_OC2M_Pos)             /*!< 0x00001000 */
#define TIM_CCMR1_OC2M_1          (0x0002UL << TIM_CCMR1_OC2M_Pos)             /*!< 0x00002000 */
#define TIM_CCMR1_OC2M_2          (0x0004UL << TIM_CCMR1_OC2M_Pos)             /*!< 0x00004000 */
#define TIM_CCMR1_OC2M_3          (0x1000UL << TIM_CCMR1_OC2M_Pos)             /*!< 0x01000000 */

#define TIM_CCMR1_OC2CE_Pos       (15U)
#define TIM_CCMR1_OC2CE_Msk       (0x1UL << TIM_CCMR1_OC2CE_Pos)               /*!< 0x00008000 */
#define TIM_CCMR1_OC2CE           TIM_CCMR1_OC2CE_Msk                          /*!<Output Compare 2 Clear Enable */

/*----------------------------------------------------------------------------*/
#define TIM_CCMR1_IC1PSC_Pos      (2U)
#define TIM_CCMR1_IC1PSC_Msk      (0x3UL << TIM_CCMR1_IC1PSC_Pos)              /*!< 0x0000000C */
#define TIM_CCMR1_IC1PSC          TIM_CCMR1_IC1PSC_Msk                         /*!<IC1PSC[1:0] bits (Input Capture 1 Prescaler) */
#define TIM_CCMR1_IC1PSC_0        (0x1UL << TIM_CCMR1_IC1PSC_Pos)              /*!< 0x00000004 */
#define TIM_CCMR1_IC1PSC_1        (0x2UL << TIM_CCMR1_IC1PSC_Pos)              /*!< 0x00000008 */

#define TIM_CCMR1_IC1F_Pos        (4U)
#define TIM_CCMR1_IC1F_Msk        (0xFUL << TIM_CCMR1_IC1F_Pos)                /*!< 0x000000F0 */
#define TIM_CCMR1_IC1F            TIM_CCMR1_IC1F_Msk                           /*!<IC1F[3:0] bits (Input Capture 1 Filter) */
#define TIM_CCMR1_IC1F_0          (0x1UL << TIM_CCMR1_IC1F_Pos)                /*!< 0x00000010 */
#define TIM_CCMR1_IC1F_1          (0x2UL << TIM_CCMR1_IC1F_Pos)                /*!< 0x00000020 */
#define TIM_CCMR1_IC1F_2          (0x4UL << TIM_CCMR1_IC1F_Pos)                /*!< 0x00000040 */
#define TIM_CCMR1_IC1F_3          (0x8UL << TIM_CCMR1_IC1F_Pos)                /*!< 0x00000080 */

#define TIM_CCMR1_IC2PSC_Pos      (10U)
#define TIM_CCMR1_IC2PSC_Msk      (0x3UL << TIM_CCMR1_IC2PSC_Pos)              /*!< 0x00000C00 */
#define TIM_CCMR1_IC2PSC          TIM_CCMR1_IC2PSC_Msk                         /*!<IC2PSC[1:0] bits (Input Capture 2 Prescaler) */
#define TIM_CCMR1_IC2PSC_0        (0x1UL << TIM_CCMR1_IC2PSC_Pos)              /*!< 0x00000400 */
#define TIM_CCMR1_IC2PSC_1        (0x2UL << TIM_CCMR1_IC2PSC_Pos)              /*!< 0x00000800 */

#define TIM_CCMR1_IC2F_Pos        (12U)
#define TIM_CCMR1_IC2F_Msk        (0xFUL << TIM_CCMR1_IC2F_Pos)                /*!< 0x0000F000 */
#define TIM_CCMR1_IC2F            TIM_CCMR1_IC2F_Msk                           /*!<IC2F[3:0] bits (Input Capture 2 Filter) */
#define TIM_CCMR1_IC2F_0          (0x1UL << TIM_CCMR1_IC2F_Pos)                /*!< 0x00001000 */
#define TIM_CCMR1_IC2F_1          (0x2UL << TIM_CCMR1_IC2F_Pos)                /*!< 0x00002000 */
#define TIM_CCMR1_IC2F_2          (0x4UL << TIM_CCMR1_IC2F_Pos)                /*!< 0x00004000 */
#define TIM_CCMR1_IC2F_3          (0x8UL << TIM_CCMR1_IC2F_Pos)                /*!< 0x00008000 */

/******************  Bit definition for TIM_CCMR2 register  *******************/
#define TIM_CCMR2_CC3S_Pos        (0U)
#define TIM_CCMR2_CC3S_Msk        (0x3UL << TIM_CCMR2_CC3S_Pos)                /*!< 0x00000003 */
#define TIM_CCMR2_CC3S            TIM_CCMR2_CC3S_Msk                           /*!<CC3S[1:0] bits (Capture/Compare 3 Selection) */
#define TIM_CCMR2_CC3S_0          (0x1UL << TIM_CCMR2_CC3S_Pos)                /*!< 0x00000001 */
#define TIM_CCMR2_CC3S_1          (0x2UL << TIM_CCMR2_CC3S_Pos)                /*!< 0x00000002 */

#define TIM_CCMR2_OC3FE_Pos       (2U)
#define TIM_CCMR2_OC3FE_Msk       (0x1UL << TIM_CCMR2_OC3FE_Pos)               /*!< 0x00000004 */
#define TIM_CCMR2_OC3FE           TIM_CCMR2_OC3FE_Msk                          /*!<Output Compare 3 Fast enable */
#define TIM_CCMR2_OC3PE_Pos       (3U)
#define TIM_CCMR2_OC3PE_Msk       (0x1UL << TIM_CCMR2_OC3PE_Pos)               /*!< 0x00000008 */
#define TIM_CCMR2_OC3PE           TIM_CCMR2_OC3PE_Msk                          /*!<Output Compare 3 Preload enable */

#define TIM_CCMR2_OC3M_Pos        (4U)
#define TIM_CCMR2_OC3M_Msk        (0x1007UL << TIM_CCMR2_OC3M_Pos)             /*!< 0x00010070 */
#define TIM_CCMR2_OC3M            TIM_CCMR2_OC3M_Msk                           /*!<OC3M[2:0] bits (Output Compare 3 Mode) */
#define TIM_CCMR2_OC3M_0          (0x0001UL << TIM_CCMR2_OC3M_Pos)             /*!< 0x00000010 */
#define TIM_CCMR2_OC3M_1          (0x0002UL << TIM_CCMR2_OC3M_Pos)             /*!< 0x00000020 */
#define TIM_CCMR2_OC3M_2          (0x0004UL << TIM_CCMR2_OC3M_Pos)             /*!< 0x00000040 */
#define TIM_CCMR2_OC3M_3          (0x1000UL << TIM_CCMR2_OC3M_Pos)             /*!< 0x00010000 */

#define TIM_CCMR2_OC3CE_Pos       (7U)
#define TIM_CCMR2_OC3CE_Msk       (0x1UL << TIM_CCMR2_OC3CE_Pos)               /*!< 0x00000080 */
#define TIM_CCMR2_OC3CE           TIM_CCMR2_OC3CE_Msk                          /*!<Output Compare 3 Clear Enable */

#define TIM_CCMR2_CC4S_Pos        (8U)
#define TIM_CCMR2_CC4S_Msk        (0x3UL << TIM_CCMR2_CC4S_Pos)                /*!< 0x00000300 */
#define TIM_CCMR2_CC4S            TIM_CCMR2_CC4S_Msk                           /*!<CC4S[1:0] bits (Capture/Compare 4 Selection) */
#define TIM_CCMR2_CC4S_0          (0x1UL << TIM_CCMR2_CC4S_Pos)                /*!< 0x00000100 */
#define TIM_CCMR2_CC4S_1          (0x2UL << TIM_CCMR2_CC4S_Pos)                /*!< 0x00000200 */

#define TIM_CCMR2_OC4FE_Pos       (10U)
#define TIM_CCMR2_OC4FE_Msk       (0x1UL << TIM_CCMR2_OC4FE_Pos)               /*!< 0x00000400 */
#define TIM_CCMR2_OC4FE           TIM_CCMR2_OC4FE_Msk                          /*!<Output Compare 4 Fast enable */
#define TIM_CCMR2_OC4PE_Pos       (11U)
#define TIM_CCMR2_OC4PE_Msk       (0x1UL << TIM_CCMR2_OC4PE_Pos)               /*!< 0x00000800 */
#define TIM_CCMR2_OC4PE           TIM_CCMR2_OC4PE_Msk                          /*!<Output Compare 4 Preload enable */

#define TIM_CCMR2_OC4M_Pos        (12U)
#define TIM_CCMR2_OC4M_Msk        (0x1007UL << TIM_CCMR2_OC4M_Pos)             /*!< 0x01007000 */
#define TIM_CCMR2_OC4M            TIM_CCMR2_OC4M_Msk                           /*!<OC4M[2:0] bits (Output Compare 4 Mode) */
#define TIM_CCMR2_OC4M_0          (0x0001UL << TIM_CCMR2_OC4M_Pos)             /*!< 0x00001000 */
#define TIM_CCMR2_OC4M_1          (0x0002UL << TIM_CCMR2_OC4M_Pos)             /*!< 0x00002000 */
#define TIM_CCMR2_OC4M_2          (0x0004UL << TIM_CCMR2_OC4M_Pos)             /*!< 0x00004000 */
#define TIM_CCMR2_OC4M_3          (0x1000UL << TIM_CCMR2_OC4M_Pos)             /*!< 0x01000000 */

#define TIM_CCMR2_OC4CE_Pos       (15U)
#define TIM_CCMR2_OC4CE_Msk       (0x1UL << TIM_CCMR2_OC4CE_Pos)               /*!< 0x00008000 */
#define TIM_CCMR2_OC4CE           TIM_CCMR2_OC4CE_Msk                          /*!<Output Compare 4 Clear Enable */

/*----------------------------------------------------------------------------*/
#define TIM_CCMR2_IC3PSC_Pos      (2U)
#define TIM_CCMR2_IC3PSC_Msk      (0x3UL << TIM_CCMR2_IC3PSC_Pos)              /*!< 0x0000000C */
#define TIM_CCMR2_IC3PSC          TIM_CCMR2_IC3PSC_Msk                         /*!<IC3PSC[1:0] bits (Input Capture 3 Prescaler) */
#define TIM_CCMR2_IC3PSC_0        (0x1UL << TIM_CCMR2_IC3PSC_Pos)              /*!< 0x00000004 */
#define TIM_CCMR2_IC3PSC_1        (0x2UL << TIM_CCMR2_IC3PSC_Pos)              /*!< 0x00000008 */

#define TIM_CCMR2_IC3F_Pos        (4U)
#define TIM_CCMR2_IC3F_Msk        (0xFUL << TIM_CCMR2_IC3F_Pos)                /*!< 0x000000F0 */
#define TIM_CCMR2_IC3F            TIM_CCMR2_IC3F_Msk                           /*!<IC3F[3:0] bits (Input Capture 3 Filter) */
#define TIM_CCMR2_IC3F_0          (0x1UL << TIM_CCMR2_IC3F_Pos)                /*!< 0x00000010 */
#define TIM_CCMR2_IC3F_1          (0x2UL << TIM_CCMR2_IC3F_Pos)                /*!< 0x00000020 */
#define TIM_CCMR2_IC3F_2          (0x4UL << TIM_CCMR2_IC3F_Pos)                /*!< 0x00000040 */
#define TIM_CCMR2_IC3F_3          (0x8UL << TIM_CCMR2_IC3F_Pos)                /*!< 0x00000080 */

#define TIM_CCMR2_IC4PSC_Pos      (10U)
#define TIM_CCMR2_IC4PSC_Msk      (0x3UL << TIM_CCMR2_IC4PSC_Pos)              /*!< 0x00000C00 */
#define TIM_CCMR2_IC4PSC          TIM_CCMR2_IC4PSC_Msk                         /*!<IC4PSC[1:0] bits (Input Capture 4 Prescaler) */
#define TIM_CCMR2_IC4PSC_0        (0x1UL << TIM_CCMR2_IC4PSC_Pos)              /*!< 0x00000400 */
#define TIM_CCMR2_IC4PSC_1        (0x2UL << TIM_CCMR2_IC4PSC_Pos)              /*!< 0x00000800 */

#define TIM_CCMR2_IC4F_Pos        (12U)
#define TIM_CCMR2_IC4F_Msk        (0xFUL << TIM_CCMR2_IC4F_Pos)                /*!< 0x0000F000 */
#define TIM_CCMR2_IC4F            TIM_CCMR2_IC4F_Msk                           /*!<IC4F[3:0] bits (Input Capture 4 Filter) */
#define TIM_CCMR2_IC4F_0          (0x1UL << TIM_CCMR2_IC4F_Pos)                /*!< 0x00001000 */
#define TIM_CCMR2_IC4F_1          (0x2UL << TIM_CCMR2_IC4F_Pos)                /*!< 0x00002000 */
#define TIM_CCMR2_IC4F_2          (0x4UL << TIM_CCMR2_IC4F_Pos)                /*!< 0x00004000 */
#define TIM_CCMR2_IC4F_3          (0x8UL << TIM_CCMR2_IC4F_Pos)                /*!< 0x00008000 */

/******************  Bit definition for TIM_CCMR3 register  *******************/
#define TIM_CCMR3_OC5FE_Pos       (2U)
#define TIM_CCMR3_OC5FE_Msk       (0x1UL << TIM_CCMR3_OC5FE_Pos)               /*!< 0x00000004 */
#define TIM_CCMR3_OC5FE           TIM_CCMR3_OC5FE_Msk                          /*!<Output Compare 5 Fast enable */
#define TIM_CCMR3_OC5PE_Pos       (3U)
#define TIM_CCMR3_OC5PE_Msk       (0x1UL << TIM_CCMR3_OC5PE_Pos)               /*!< 0x00000008 */
#define TIM_CCMR3_OC5PE           TIM_CCMR3_OC5PE_Msk                          /*!<Output Compare 5 Preload enable */

#define TIM_CCMR3_OC5M_Pos        (4U)
#define TIM_CCMR3_OC5M_Msk        (0x1007UL << TIM_CCMR3_OC5M_Pos)             /*!< 0x00010070 */
#define TIM_CCMR3_OC5M            TIM_CCMR3_OC5M_Msk                           /*!<OC5M[3:0] bits (Output Compare 5 Mode) */
#define TIM_CCMR3_OC5M_0          (0x0001UL << TIM_CCMR3_OC5M_Pos)             /*!< 0x00000010 */
#define TIM_CCMR3_OC5M_1          (0x0002UL << TIM_CCMR3_OC5M_Pos)             /*!< 0x00000020 */
#define TIM_CCMR3_OC5M_2          (0x0004UL << TIM_CCMR3_OC5M_Pos)             /*!< 0x00000040 */
#define TIM_CCMR3_OC5M_3          (0x1000UL << TIM_CCMR3_OC5M_Pos)             /*!< 0x00010000 */

#define TIM_CCMR3_OC5CE_Pos       (7U)
#define TIM_CCMR3_OC5CE_Msk       (0x1UL << TIM_CCMR3_OC5CE_Pos)               /*!< 0x00000080 */
#define TIM_CCMR3_OC5CE           TIM_CCMR3_OC5CE_Msk                          /*!<Output Compare 5 Clear Enable */

#define TIM_CCMR3_OC6FE_Pos       (10U)
#define TIM_CCMR3_OC6FE_Msk       (0x1UL << TIM_CCMR3_OC6FE_Pos)               /*!< 0x00000400 */
#define TIM_CCMR3_OC6FE           TIM_CCMR3_OC6FE_Msk                          /*!<Output Compare 6 Fast enable */
#define TIM_CCMR3_OC6PE_Pos       (11U)
#define TIM_CCMR3_OC6PE_Msk       (0x1UL << TIM_CCMR3_OC6PE_Pos)               /*!< 0x00000800 */
#define TIM_CCMR3_OC6PE           TIM_CCMR3_OC6PE_Msk                          /*!<Output Compare 6 Preload enable */

#define TIM_CCMR3_OC6M_Pos        (12U)
#define TIM_CCMR3_OC6M_Msk        (0x1007UL << TIM_CCMR3_OC6M_Pos)             /*!< 0x01007000 */
#define TIM_CCMR3_OC6M            TIM_CCMR3_OC6M_Msk                           /*!<OC6M[3:0] bits (Output Compare 6 Mode) */
#define TIM_CCMR3_OC6M_0          (0x0001UL << TIM_CCMR3_OC6M_Pos)             /*!< 0x00001000 */
#define TIM_CCMR3_OC6M_1          (0x0002UL << TIM_CCMR3_OC6M_Pos)             /*!< 0x00002000 */
#define TIM_CCMR3_OC6M_2          (0x0004UL << TIM_CCMR3_OC6M_Pos)             /*!< 0x00004000 */
#define TIM_CCMR3_OC6M_3          (0x1000UL << TIM_CCMR3_OC6M_Pos)             /*!< 0x01000000 */

#define TIM_CCMR3_OC6CE_Pos       (15U)
#define TIM_CCMR3_OC6CE_Msk       (0x1UL << TIM_CCMR3_OC6CE_Pos)               /*!< 0x00008000 */
#define TIM_CCMR3_OC6CE           TIM_CCMR3_OC6CE_Msk                          /*!<Output Compare 6 Clear Enable */

/*******************  Bit definition for TIM_CCER register  *******************/
#define TIM_CCER_CC1E_Pos         (0U)
#define TIM_CCER_CC1E_Msk         (0x1UL << TIM_CCER_CC1E_Pos)                 /*!< 0x00000001 */
#define TIM_CCER_CC1E             TIM_CCER_CC1E_Msk                            /*!<Capture/Compare 1 output enable */
#define TIM_CCER_CC1P_Pos         (1U)
#define TIM_CCER_CC1P_Msk         (0x1UL << TIM_CCER_CC1P_Pos)                 /*!< 0x00000002 */
#define TIM_CCER_CC1P             TIM_CCER_CC1P_Msk                            /*!<Capture/Compare 1 output Polarity */
#define TIM_CCER_CC1NE_Pos        (2U)
#define TIM_CCER_CC1NE_Msk        (0x1UL << TIM_CCER_CC1NE_Pos)                /*!< 0x00000004 */
#define TIM_CCER_CC1NE            TIM_CCER_CC1NE_Msk                           /*!<Capture/Compare 1 Complementary output enable */
#define TIM_CCER_CC1NP_Pos        (3U)
#define TIM_CCER_CC1NP_Msk        (0x1UL << TIM_CCER_CC1NP_Pos)                /*!< 0x00000008 */
#define TIM_CCER_CC1NP            TIM_CCER_CC1NP_Msk                           /*!<Capture/Compare 1 Complementary output Polarity */
#define TIM_CCER_CC2E_Pos         (4U)
#define TIM_CCER_CC2E_Msk         (0x1UL << TIM_CCER_CC2E_Pos)                 /*!< 0x00000010 */
#define TIM_CCER_CC2E             TIM_CCER_CC2E_Msk                            /*!<Capture/Compare 2 output enable */
#define TIM_CCER_CC2P_Pos         (5U)
#define TIM_CCER_CC2P_Msk         (0x1UL << TIM_CCER_CC2P_Pos)                 /*!< 0x00000020 */
#define TIM_CCER_CC2P             TIM_CCER_CC2P_Msk                            /*!<Capture/Compare 2 output Polarity */
#define TIM_CCER_CC2NE_Pos        (6U)
#define TIM_CCER_CC2NE_Msk        (0x1UL << TIM_CCER_CC2NE_Pos)                /*!< 0x00000040 */
#define TIM_CCER_CC2NE            TIM_CCER_CC2NE_Msk                           /*!<Capture/Compare 2 Complementary output enable */
#define TIM_CCER_CC2NP_Pos        (7U)
#define TIM_CCER_CC2NP_Msk        (0x1UL << TIM_CCER_CC2NP_Pos)                /*!< 0x00000080 */
#define TIM_CCER_CC2NP            TIM_CCER_CC2NP_Msk                           /*!<Capture/Compare 2 Complementary output Polarity */
#define TIM_CCER_CC3E_Pos         (8U)
#define TIM_CCER_CC3E_Msk         (0x1UL << TIM_CCER_CC3E_Pos)                 /*!< 0x00000100 */
#define TIM_CCER_CC3E             TIM_CCER_CC3E_Msk                            /*!<Capture/Compare 3 output enable */
#define TIM_CCER_CC3P_Pos         (9U)
#define TIM_CCER_CC3P_Msk         (0x1UL << TIM_CCER_CC3P_Pos)                 /*!< 0x00000200 */
#define TIM_CCER_CC3P             TIM_CCER_CC3P_Msk                            /*!<Capture/Compare 3 output Polarity */
#define TIM_CCER_CC3NE_Pos        (10U)
#define TIM_CCER_CC3NE_Msk        (0x1UL << TIM_CCER_CC3NE_Pos)                /*!< 0x00000400 */
#define TIM_CCER_CC3NE            TIM_CCER_CC3NE_Msk                           /*!<Capture/Compare 3 Complementary output enable */
#define TIM_CCER_CC3NP_Pos        (11U)
#define TIM_CCER_CC3NP_Msk        (0x1UL << TIM_CCER_CC3NP_Pos)                /*!< 0x00000800 */
#define TIM_CCER_CC3NP            TIM_CCER_CC3NP_Msk                           /*!<Capture/Compare 3 Complementary output Polarity */
#define TIM_CCER_CC4E_Pos         (12U)
#define TIM_CCER_CC4E_Msk         (0x1UL << TIM_CCER_CC4E_Pos)                 /*!< 0x00001000 */
#define TIM_CCER_CC4E             TIM_CCER_CC4E_Msk                            /*!<Capture/Compare 4 output enable */
#define TIM_CCER_CC4P_Pos         (13U)
#define TIM_CCER_CC4P_Msk         (0x1UL << TIM_CCER_CC4P_Pos)                 /*!< 0x00002000 */
#define TIM_CCER_CC4P             TIM_CCER_CC4P_Msk                            /*!<Capture/Compare 4 output Polarity */
#define TIM_CCER_CC4NP_Pos        (15U)
#define TIM_CCER_CC4NP_Msk        (0x1UL << TIM_CCER_CC4NP_Pos)                /*!< 0x00008000 */
#define TIM_CCER_CC4NP            TIM_CCER_CC4NP_Msk                           /*!<Capture/Compare 4 Complementary output Polarity */
#define TIM_CCER_CC5E_Pos         (16U)
#define TIM_CCER_CC5E_Msk         (0x1UL << TIM_CCER_CC5E_Pos)                 /*!< 0x00010000 */
#define TIM_CCER_CC5E             TIM_CCER_CC5E_Msk                            /*!<Capture/Compare 5 output enable */
#define TIM_CCER_CC5P_Pos         (17U)
#define TIM_CCER_CC5P_Msk         (0x1UL << TIM_CCER_CC5P_Pos)                 /*!< 0x00020000 */
#define TIM_CCER_CC5P             TIM_CCER_CC5P_Msk                            /*!<Capture/Compare 5 output Polarity */
#define TIM_CCER_CC6E_Pos         (20U)
#define TIM_CCER_CC6E_Msk         (0x1UL << TIM_CCER_CC6E_Pos)                 /*!< 0x00100000 */
#define TIM_CCER_CC6E             TIM_CCER_CC6E_Msk                            /*!<Capture/Compare 6 output enable */
#define TIM_CCER_CC6P_Pos         (21U)
#define TIM_CCER_CC6P_Msk         (0x1UL << TIM_CCER_CC6P_Pos)                 /*!< 0x00200000 */
#define TIM_CCER_CC6P             TIM_CCER_CC6P_Msk                            /*!<Capture/Compare 6 output Polarity */

/*******************  Bit definition for TIM_CNT register  ********************/
#define TIM_CNT_CNT_Pos           (0U)
#define TIM_CNT_CNT_Msk           (0xFFFFFFFFUL << TIM_CNT_CNT_Pos)            /*!< 0xFFFFFFFF */
#define TIM_CNT_CNT               TIM_CNT_CNT_Msk                              /*!<Counter Value */
#define TIM_CNT_UIFCPY_Pos        (31U)
#define TIM_CNT_UIFCPY_Msk        (0x1UL << TIM_CNT_UIFCPY_Pos)                /*!< 0x80000000 */
#define TIM_CNT_UIFCPY            TIM_CNT_UIFCPY_Msk                           /*!<Update interrupt flag copy (if UIFREMAP=1) */

/*******************  Bit definition for TIM_PSC register  ********************/
#define TIM_PSC_PSC_Pos           (0U)
#define TIM_PSC_PSC_Msk           (0xFFFFUL << TIM_PSC_PSC_Pos)                /*!< 0x0000FFFF */
#define TIM_PSC_PSC               TIM_PSC_PSC_Msk                              /*!<Prescaler Value */

/*******************  Bit definition for TIM_ARR register  ********************/
#define TIM_ARR_ARR_Pos           (0U)
#define TIM_ARR_ARR_Msk           (0xFFFFFFFFUL << TIM_ARR_ARR_Pos)            /*!< 0xFFFFFFFF */
#define TIM_ARR_ARR               TIM_ARR_ARR_Msk                              /*!<Actual auto-reload Value */

/*******************  Bit definition for TIM_RCR register  ********************/
#define TIM_RCR_REP_Pos           (0U)
#define TIM_RCR_REP_Msk           (0xFFFFUL << TIM_RCR_REP_Pos)                /*!< 0x0000FFFF */
#define TIM_RCR_REP               TIM_RCR_REP_Msk                              /*!<Repetition Counter Value */

/*******************  Bit definition for TIM_CCR1 register  *******************/
#define TIM_CCR1_CCR1_Pos         (0U)
#define TIM_CCR1_CCR1_Msk         (0xFFFFUL << TIM_CCR1_CCR1_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR1_CCR1             TIM_CCR1_CCR1_Msk                            /*!<Capture/Compare 1 Value */

/*******************  Bit definition for TIM_CCR2 register  *******************/
#define TIM_CCR2_CCR2_Pos         (0U)
#define TIM_CCR2_CCR2_Msk         (0xFFFFUL << TIM_CCR2_CCR2_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR2_CCR2             TIM_CCR2_CCR2_Msk                            /*!<Capture/Compare 2 Value */

/*******************  Bit definition for TIM_CCR3 register  *******************/
#define TIM_CCR3_CCR3_Pos         (0U)
#define TIM_CCR3_CCR3_Msk         (0xFFFFUL << TIM_CCR3_CCR3_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR3_CCR3             TIM_CCR3_CCR3_Msk                            /*!<Capture/Compare 3 Value */

/*******************  Bit definition for TIM_CCR4 register  *******************/
#define TIM_CCR4_CCR4_Pos         (0U)
#define TIM_CCR4_CCR4_Msk         (0xFFFFUL << TIM_CCR4_CCR4_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR4_CCR4             TIM_CCR4_CCR4_Msk                            /*!<Capture/Compare 4 Value */

/*******************  Bit definition for TIM_CCR5 register  *******************/
#define TIM_CCR5_CCR5_Pos         (0U)
#define TIM_CCR5_CCR5_Msk         (0xFFFFFFFFUL << TIM_CCR5_CCR5_Pos)          /*!< 0xFFFFFFFF */
#define TIM_CCR5_CCR5             TIM_CCR5_CCR5_Msk                            /*!<Capture/Compare 5 Value */
#define TIM_CCR5_GC5C1_Pos        (29U)
#define TIM_CCR5_GC5C1_Msk        (0x1UL << TIM_CCR5_GC5C1_Pos)                /*!< 0x20000000 */
#define TIM_CCR5_GC5C1            TIM_CCR5_GC5C1_Msk                           /*!<Group Channel 5 and Channel 1 */
#define TIM_CCR5_GC5C2_Pos        (30U)
#define TIM_CCR5_GC5C2_Msk        (0x1UL << TIM_CCR5_GC5C2_Pos)                /*!< 0x40000000 */
#define TIM_CCR5_GC5C2            TIM_CCR5_GC5C2_Msk                           /*!<Group Channel 5 and Channel 2 */
#define TIM_CCR5_GC5C3_Pos        (31U)
#define TIM_CCR5_GC5C3_Msk        (0x1UL << TIM_CCR5_GC5C3_Pos)                /*!< 0x80000000 */
#define TIM_CCR5_GC5C3            TIM_CCR5_GC5C3_Msk                           /*!<Group Channel 5 and Channel 3 */

/*******************  Bit definition for TIM_CCR6 register  *******************/
#define TIM_CCR6_CCR6_Pos         (0U)
#define TIM_CCR6_CCR6_Msk         (0xFFFFUL << TIM_CCR6_CCR6_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR6_CCR6             TIM_CCR6_CCR6_Msk                            /*!<Capture/Compare 6 Value */

/*******************  Bit definition for TIM_BDTR register  *******************/
#define TIM_BDTR_DTG_Pos          (0U)
#define TIM_BDTR_DTG_Msk          (0xFFUL << TIM_BDTR_DTG_Pos)                 /*!< 0x000000FF */
#define TIM_BDTR_DTG              TIM_BDTR_DTG_Msk                             /*!<DTG[0:7] bits (Dead-Time Generator set-up) */
#define TIM_BDTR_DTG_0            (0x01UL << TIM_BDTR_DTG_Pos)                 /*!< 0x00000001 */
#define TIM_BDTR_DTG_1            (0x02UL << TIM_BDTR_DTG_Pos)                 /*!< 0x00000002 */
#define TIM_BDTR_DTG_2            (0x04UL << TIM_BDTR_DTG_Pos)                 /*!< 0x00000004 */
#define TIM_BDTR_DTG_3            (0x08UL << TIM_BDTR_DTG_Pos)                 /*!< 0x00000008 */
#define TIM_BDTR_DTG_4            (0x10UL << TIM_BDTR_DTG_Pos)                 /*!< 0x00000010 */
#define TIM_BDTR_DTG_5            (0x20UL << TIM_BDTR_DTG_Pos)                 /*!< 0x00000020 */
#define TIM_BDTR_DTG_6            (0x40UL << TIM_BDTR_DTG_Pos)                 /*!< 0x00000040 */
#define TIM_BDTR_DTG_7            (0x80UL << TIM_BDTR_DTG_Pos)                 /*!< 0x00000080 */

#define TIM_BDTR_LOCK_Pos         (8U)
#define TIM_BDTR_LOCK_Msk         (0x3UL << TIM_BDTR_LOCK_Pos)                 /*!< 0x00000300 */
#define TIM_BDTR_LOCK             TIM_BDTR_LOCK_Msk                            /*!<LOCK[1:0] bits (Lock Configuration) */
#define TIM_BDTR_LOCK_0           (0x1UL << TIM_BDTR_LOCK_Pos)                 /*!< 0x00000100 */
#define TIM_BDTR_LOCK_1           (0x2UL << TIM_BDTR_LOCK_Pos)                 /*!< 0x00000200 */

#define TIM_BDTR_OSSI_Pos         (10U)
#define TIM_BDTR_OSSI_Msk         (0x1UL << TIM_BDTR_OSSI_Pos)                 /*!< 0x00000400 */
#define TIM_BDTR_OSSI             TIM_BDTR_OSSI_Msk                            /*!<Off-State Selection for Idle mode */
#define TIM_BDTR_OSSR_Pos         (11U)
#define TIM_BDTR_OSSR_Msk         (0x1UL << TIM_BDTR_OSSR_Pos)                 /*!< 0x00000800 */
#define TIM_BDTR_OSSR             TIM_BDTR_OSSR_Msk                            /*!<Off-State Selection for Run mode */
#define TIM_BDTR_BKE_Pos          (12U)
#define TIM_BDTR_BKE_Msk          (0x1UL << TIM_BDTR_BKE_Pos)                  /*!< 0x00001000 */
#define TIM_BDTR_BKE              TIM_BDTR_BKE_Msk                             /*!<Break enable for Break 1 */
#define TIM_BDTR_BKP_Pos          (13U)
#define TIM_BDTR_BKP_Msk          (0x1UL << TIM_BDTR_BKP_Pos)                  /*!< 0x00002000 */
#define TIM_BDTR_BKP              TIM_BDTR_BKP_Msk                             /*!<Break Polarity for Break 1 */
#define TIM_BDTR_AOE_Pos          (14U)
#define TIM_BDTR_AOE_Msk          (0x1UL << TIM_BDTR_AOE_Pos)                  /*!< 0x00004000 */
#define TIM_BDTR_AOE              TIM_BDTR_AOE_Msk                             /*!<Automatic Output enable */
#define TIM_BDTR_MOE_Pos          (15U)
#define TIM_BDTR_MOE_Msk          (0x1UL << TIM_BDTR_MOE_Pos)                  /*!< 0x00008000 */
#define TIM_BDTR_MOE              TIM_BDTR_MOE_Msk                             /*!<Main Output enable */

#define TIM_BDTR_BKF_Pos          (16U)
#define TIM_BDTR_BKF_Msk          (0xFUL << TIM_BDTR_BKF_Pos)                  /*!< 0x000F0000 */
#define TIM_BDTR_BKF              TIM_BDTR_BKF_Msk                             /*!<Break Filter for Break 1 */
#define TIM_BDTR_BK2F_Pos         (20U)
#define TIM_BDTR_BK2F_Msk         (0xFUL << TIM_BDTR_BK2F_Pos)                 /*!< 0x00F00000 */
#define TIM_BDTR_BK2F             TIM_BDTR_BK2F_Msk                            /*!<Break Filter for Break 2 */

#define TIM_BDTR_BK2E_Pos         (24U)
#define TIM_BDTR_BK2E_Msk         (0x1UL << TIM_BDTR_BK2E_Pos)                 /*!< 0x01000000 */
#define TIM_BDTR_BK2E             TIM_BDTR_BK2E_Msk                            /*!<Break enable for Break 2 */
#define TIM_BDTR_BK2P_Pos         (25U)
#define TIM_BDTR_BK2P_Msk         (0x1UL << TIM_BDTR_BK2P_Pos)                 /*!< 0x02000000 */
#define TIM_BDTR_BK2P             TIM_BDTR_BK2P_Msk                            /*!<Break Polarity for Break 2 */

/*******************  Bit definition for TIM_DCR register  ********************/
#define TIM_DCR_DBA_Pos           (0U)
#define TIM_DCR_DBA_Msk           (0x1FUL << TIM_DCR_DBA_Pos)                  /*!< 0x0000001F */
#define TIM_DCR_DBA               TIM_DCR_DBA_Msk                              /*!<DBA[4:0] bits (DMA Base Address) */
#define TIM_DCR_DBA_0             (0x01UL << TIM_DCR_DBA_Pos)                  /*!< 0x00000001 */
#define TIM_DCR_DBA_1             (0x02UL << TIM_DCR_DBA_Pos)                  /*!< 0x00000002 */
#define TIM_DCR_DBA_2             (0x04UL << TIM_DCR_DBA_Pos)                  /*!< 0x00000004 */
#define TIM_DCR_DBA_3             (0x08UL << TIM_DCR_DBA_Pos)                  /*!< 0x00000008 */
#define TIM_DCR_DBA_4             (0x10UL << TIM_DCR_DBA_Pos)                  /*!< 0x00000010 */

#define TIM_DCR_DBL_Pos           (8U)
#define TIM_DCR_DBL_Msk           (0x1FUL << TIM_DCR_DBL_Pos)                  /*!< 0x00001F00 */
#define TIM_DCR_DBL               TIM_DCR_DBL_Msk                              /*!<DBL[4:0] bits (DMA Burst Length) */
#define TIM_DCR_DBL_0             (0x01UL << TIM_DCR_DBL_Pos)                  /*!< 0x00000100 */
#define TIM_DCR_DBL_1             (0x02UL << TIM_DCR_DBL_Pos)                  /*!< 0x00000200 */
#define TIM_DCR_DBL_2             (0x04UL << TIM_DCR_DBL_Pos)                  /*!< 0x00000400 */
#define TIM_DCR_DBL_3             (0x08UL << TIM_DCR_DBL_Pos)                  /*!< 0x00000800 */
#define TIM_DCR_DBL_4             (0x10UL << TIM_DCR_DBL_Pos)                  /*!< 0x00001000 */

/*******************  Bit definition for TIM_DMAR register  *******************/
#define TIM_DMAR_DMAB_Pos         (0U)
#define TIM_DMAR_DMAB_Msk         (0xFFFFUL << TIM_DMAR_DMAB_Pos)              /*!< 0x0000FFFF */
#define TIM_DMAR_DMAB             TIM_DMAR_DMAB_Msk                            /*!<DMA register for burst accesses */

/*******************  Bit definition for TIM1_OR1 register  *******************/
#define TIM1_OR1_ETR_ADC1_RMP_Pos      (0U)
#define TIM1_OR1_ETR_ADC1_RMP_Msk      (0x3UL << TIM1_OR1_ETR_ADC1_RMP_Pos)    /*!< 0x00000003 */
#define TIM1_OR1_ETR_ADC1_RMP          TIM1_OR1_ETR_ADC1_RMP_Msk               /*!<ETR_ADC1_RMP[1:0] bits (TIM1 ETR remap on ADC1) */
#define TIM1_OR1_ETR_ADC1_RMP_0        (0x1UL << TIM1_OR1_ETR_ADC1_RMP_Pos)    /*!< 0x00000001 */
#define TIM1_OR1_ETR_ADC1_RMP_1        (0x2UL << TIM1_OR1_ETR_ADC1_RMP_Pos)    /*!< 0x00000002 */

#define TIM1_OR1_TI1_RMP_Pos           (4U)
#define TIM1_OR1_TI1_RMP_Msk           (0x1UL << TIM1_OR1_TI1_RMP_Pos)         /*!< 0x00000010 */
#define TIM1_OR1_TI1_RMP               TIM1_OR1_TI1_RMP_Msk                    /*!<TIM1 Input Capture 1 remap */

/*******************  Bit definition for TIM1_OR2 register  *******************/
#define TIM1_OR2_BKINE_Pos             (0U)
#define TIM1_OR2_BKINE_Msk             (0x1UL << TIM1_OR2_BKINE_Pos)           /*!< 0x00000001 */
#define TIM1_OR2_BKINE                 TIM1_OR2_BKINE_Msk                      /*!<BRK BKIN input enable */
#define TIM1_OR2_BKCMP1E_Pos           (1U)
#define TIM1_OR2_BKCMP1E_Msk           (0x1UL << TIM1_OR2_BKCMP1E_Pos)         /*!< 0x00000002 */
#define TIM1_OR2_BKCMP1E               TIM1_OR2_BKCMP1E_Msk                    /*!<BRK COMP1 enable */
#define TIM1_OR2_BKCMP2E_Pos           (2U)
#define TIM1_OR2_BKCMP2E_Msk           (0x1UL << TIM1_OR2_BKCMP2E_Pos)         /*!< 0x00000004 */
#define TIM1_OR2_BKCMP2E               TIM1_OR2_BKCMP2E_Msk                    /*!<BRK COMP2 enable */
#define TIM1_OR2_BKINP_Pos             (9U)
#define TIM1_OR2_BKINP_Msk             (0x1UL << TIM1_OR2_BKINP_Pos)           /*!< 0x00000200 */
#define TIM1_OR2_BKINP                 TIM1_OR2_BKINP_Msk                      /*!<BRK BKIN input polarity */
#define TIM1_OR2_BKCMP1P_Pos           (10U)
#define TIM1_OR2_BKCMP1P_Msk           (0x1UL << TIM1_OR2_BKCMP1P_Pos)         /*!< 0x00000400 */
#define TIM1_OR2_BKCMP1P               TIM1_OR2_BKCMP1P_Msk                    /*!<BRK COMP1 input polarity */
#define TIM1_OR2_BKCMP2P_Pos           (11U)
#define TIM1_OR2_BKCMP2P_Msk           (0x1UL << TIM1_OR2_BKCMP2P_Pos)         /*!< 0x00000800 */
#define TIM1_OR2_BKCMP2P               TIM1_OR2_BKCMP2P_Msk                    /*!<BRK COMP2 input polarity */

#define TIM1_OR2_ETRSEL_Pos            (14U)
#define TIM1_OR2_ETRSEL_Msk            (0x7UL << TIM1_OR2_ETRSEL_Pos)          /*!< 0x0001C000 */
#define TIM1_OR2_ETRSEL                TIM1_OR2_ETRSEL_Msk                     /*!<ETRSEL[2:0] bits (TIM1 ETR source selection) */
#define TIM1_OR2_ETRSEL_0              (0x1UL << TIM1_OR2_ETRSEL_Pos)          /*!< 0x00004000 */
#define TIM1_OR2_ETRSEL_1              (0x2UL << TIM1_OR2_ETRSEL_Pos)          /*!< 0x00008000 */
#define TIM1_OR2_ETRSEL_2              (0x4UL << TIM1_OR2_ETRSEL_Pos)          /*!< 0x00010000 */

/*******************  Bit definition for TIM1_OR3 register  *******************/
#define TIM1_OR3_BK2INE_Pos            (0U)
#define TIM1_OR3_BK2INE_Msk            (0x1UL << TIM1_OR3_BK2INE_Pos)          /*!< 0x00000001 */
#define TIM1_OR3_BK2INE                TIM1_OR3_BK2INE_Msk                     /*!<BRK2 BKIN2 input enable */
#define TIM1_OR3_BK2CMP1E_Pos          (1U)
#define TIM1_OR3_BK2CMP1E_Msk          (0x1UL << TIM1_OR3_BK2CMP1E_Pos)        /*!< 0x00000002 */
#define TIM1_OR3_BK2CMP1E              TIM1_OR3_BK2CMP1E_Msk                   /*!<BRK2 COMP1 enable */
#define TIM1_OR3_BK2CMP2E_Pos          (2U)
#define TIM1_OR3_BK2CMP2E_Msk          (0x1UL << TIM1_OR3_BK2CMP2E_Pos)        /*!< 0x00000004 */
#define TIM1_OR3_BK2CMP2E              TIM1_OR3_BK2CMP2E_Msk                   /*!<BRK2 COMP2 enable */
#define TIM1_OR3_BK2INP_Pos            (9U)
#define TIM1_OR3_BK2INP_Msk            (0x1UL << TIM1_OR3_BK2INP_Pos)          /*!< 0x00000200 */
#define TIM1_OR3_BK2INP                TIM1_OR3_BK2INP_Msk                     /*!<BRK2 BKIN2 input polarity */
#define TIM1_OR3_BK2CMP1P_Pos          (10U)
#define TIM1_OR3_BK2CMP1P_Msk          (0x1UL << TIM1_OR3_BK2CMP1P_Pos)        /*!< 0x00000400 */
#define TIM1_OR3_BK2CMP1P              TIM1_OR3_BK2CMP1P_Msk                   /*!<BRK2 COMP1 input polarity */
#define TIM1_OR3_BK2CMP2P_Pos          (11U)
#define TIM1_OR3_BK2CMP2P_Msk          (0x1UL << TIM1_OR3_BK2CMP2P_Pos)        /*!< 0x00000800 */
#define TIM1_OR3_BK2CMP2P              TIM1_OR3_BK2CMP2P_Msk                   /*!<BRK2 COMP2 input polarity */


/*******************  Bit definition for TIM2_OR1 register  *******************/
#define TIM2_OR1_ITR1_RMP_Pos     (0U)
#define TIM2_OR1_ITR1_RMP_Msk     (0x1UL << TIM2_OR1_ITR1_RMP_Pos)             /*!< 0x00000001 */
#define TIM2_OR1_ITR1_RMP         TIM2_OR1_ITR1_RMP_Msk                        /*!<TIM2 Internal trigger 1 remap */
#define TIM2_OR1_ETR1_RMP_Pos     (1U)
#define TIM2_OR1_ETR1_RMP_Msk     (0x1UL << TIM2_OR1_ETR1_RMP_Pos)             /*!< 0x00000002 */
#define TIM2_OR1_ETR1_RMP         TIM2_OR1_ETR1_RMP_Msk                        /*!<TIM2 External trigger 1 remap */

#define TIM2_OR1_TI4_RMP_Pos      (2U)
#define TIM2_OR1_TI4_RMP_Msk      (0x3UL << TIM2_OR1_TI4_RMP_Pos)              /*!< 0x0000000C */
#define TIM2_OR1_TI4_RMP          TIM2_OR1_TI4_RMP_Msk                         /*!<TI4_RMP[1:0] bits (TIM2 Input Capture 4 remap) */
#define TIM2_OR1_TI4_RMP_0        (0x1UL << TIM2_OR1_TI4_RMP_Pos)              /*!< 0x00000004 */
#define TIM2_OR1_TI4_RMP_1        (0x2UL << TIM2_OR1_TI4_RMP_Pos)              /*!< 0x00000008 */

/*******************  Bit definition for TIM2_OR2 register  *******************/
#define TIM2_OR2_ETRSEL_Pos       (14U)
#define TIM2_OR2_ETRSEL_Msk       (0x7UL << TIM2_OR2_ETRSEL_Pos)               /*!< 0x0001C000 */
#define TIM2_OR2_ETRSEL           TIM2_OR2_ETRSEL_Msk                          /*!<ETRSEL[2:0] bits (TIM2 ETR source selection) */
#define TIM2_OR2_ETRSEL_0         (0x1UL << TIM2_OR2_ETRSEL_Pos)               /*!< 0x00004000 */
#define TIM2_OR2_ETRSEL_1         (0x2UL << TIM2_OR2_ETRSEL_Pos)               /*!< 0x00008000 */
#define TIM2_OR2_ETRSEL_2         (0x4UL << TIM2_OR2_ETRSEL_Pos)               /*!< 0x00010000 */


/*******************  Bit definition for TIM15_OR1 register  ******************/
#define TIM15_OR1_TI1_RMP_Pos           (0U)
#define TIM15_OR1_TI1_RMP_Msk           (0x1UL << TIM15_OR1_TI1_RMP_Pos)       /*!< 0x00000001 */
#define TIM15_OR1_TI1_RMP               TIM15_OR1_TI1_RMP_Msk                  /*!<TIM15 Input Capture 1 remap */

#define TIM15_OR1_ENCODER_MODE_Pos      (1U)
#define TIM15_OR1_ENCODER_MODE_Msk      (0x3UL << TIM15_OR1_ENCODER_MODE_Pos)  /*!< 0x00000006 */
#define TIM15_OR1_ENCODER_MODE          TIM15_OR1_ENCODER_MODE_Msk             /*!<ENCODER_MODE[1:0] bits (TIM15 Encoder mode) */
#define TIM15_OR1_ENCODER_MODE_0        (0x1UL << TIM15_OR1_ENCODER_MODE_Pos)  /*!< 0x00000002 */
#define TIM15_OR1_ENCODER_MODE_1        (0x2UL << TIM15_OR1_ENCODER_MODE_Pos)  /*!< 0x00000004 */

/*******************  Bit definition for TIM15_OR2 register  ******************/
#define TIM15_OR2_BKINE_Pos             (0U)
#define TIM15_OR2_BKINE_Msk             (0x1UL << TIM15_OR2_BKINE_Pos)         /*!< 0x00000001 */
#define TIM15_OR2_BKINE                 TIM15_OR2_BKINE_Msk                    /*!<BRK BKIN input enable */
#define TIM15_OR2_BKCMP1E_Pos           (1U)
#define TIM15_OR2_BKCMP1E_Msk           (0x1UL << TIM15_OR2_BKCMP1E_Pos)       /*!< 0x00000002 */
#define TIM15_OR2_BKCMP1E               TIM15_OR2_BKCMP1E_Msk                  /*!<BRK COMP1 enable */
#define TIM15_OR2_BKINP_Pos             (9U)
#define TIM15_OR2_BKINP_Msk             (0x1UL << TIM15_OR2_BKINP_Pos)         /*!< 0x00000200 */
#define TIM15_OR2_BKINP                 TIM15_OR2_BKINP_Msk                    /*!<BRK BKIN input polarity */
#define TIM15_OR2_BKCMP1P_Pos           (10U)
#define TIM15_OR2_BKCMP1P_Msk           (0x1UL << TIM15_OR2_BKCMP1P_Pos)       /*!< 0x00000400 */
#define TIM15_OR2_BKCMP1P               TIM15_OR2_BKCMP1P_Msk                  /*!<BRK COMP1 input polarity */

/*******************  Bit definition for TIM16_OR1 register  ******************/
#define TIM16_OR1_TI1_RMP_Pos      (0U)
#define TIM16_OR1_TI1_RMP_Msk      (0x7UL << TIM16_OR1_TI1_RMP_Pos)            /*!< 0x00000007 */
#define TIM16_OR1_TI1_RMP          TIM16_OR1_TI1_RMP_Msk                       /*!<TI1_RMP[2:0] bits (TIM16 Input Capture 1 remap) */
#define TIM16_OR1_TI1_RMP_0        (0x1UL << TIM16_OR1_TI1_RMP_Pos)            /*!< 0x00000001 */
#define TIM16_OR1_TI1_RMP_1        (0x2UL << TIM16_OR1_TI1_RMP_Pos)            /*!< 0x00000002 */
#define TIM16_OR1_TI1_RMP_2        (0x4UL << TIM16_OR1_TI1_RMP_Pos)            /*!< 0x00000004 */

/*******************  Bit definition for TIM16_OR2 register  ******************/
#define TIM16_OR2_BKINE_Pos        (0U)
#define TIM16_OR2_BKINE_Msk        (0x1UL << TIM16_OR2_BKINE_Pos)              /*!< 0x00000001 */
#define TIM16_OR2_BKINE            TIM16_OR2_BKINE_Msk                         /*!<BRK BKIN input enable */
#define TIM16_OR2_BKCMP1E_Pos      (1U)
#define TIM16_OR2_BKCMP1E_Msk      (0x1UL << TIM16_OR2_BKCMP1E_Pos)            /*!< 0x00000002 */
#define TIM16_OR2_BKCMP1E          TIM16_OR2_BKCMP1E_Msk                       /*!<BRK COMP1 enable */
#define TIM16_OR2_BKINP_Pos        (9U)
#define TIM16_OR2_BKINP_Msk        (0x1UL << TIM16_OR2_BKINP_Pos)              /*!< 0x00000200 */
#define TIM16_OR2_BKINP            TIM16_OR2_BKINP_Msk                         /*!<BRK BKIN input polarity */
#define TIM16_OR2_BKCMP1P_Pos      (10U)
#define TIM16_OR2_BKCMP1P_Msk      (0x1UL << TIM16_OR2_BKCMP1P_Pos)            /*!< 0x00000400 */
#define TIM16_OR2_BKCMP1P          TIM16_OR2_BKCMP1P_Msk                       /*!<BRK COMP1 input polarity */


/******************************************************************************/
/*                                                                            */
/*                         Low Power Timer (LPTIM)                            */
/*                                                                            */
/******************************************************************************/
/******************  Bit definition for LPTIM_ISR register  *******************/
#define LPTIM_ISR_CMPM_Pos          (0U)
#define LPTIM_ISR_CMPM_Msk          (0x1UL << LPTIM_ISR_CMPM_Pos)              /*!< 0x00000001 */
#define LPTIM_ISR_CMPM              LPTIM_ISR_CMPM_Msk                         /*!< Compare match */
#define LPTIM_ISR_ARRM_Pos          (1U)
#define LPTIM_ISR_ARRM_Msk          (0x1UL << LPTIM_ISR_ARRM_Pos)              /*!< 0x00000002 */
#define LPTIM_ISR_ARRM              LPTIM_ISR_ARRM_Msk                         /*!< Autoreload match */
#define LPTIM_ISR_EXTTRIG_Pos       (2U)
#define LPTIM_ISR_EXTTRIG_Msk       (0x1UL << LPTIM_ISR_EXTTRIG_Pos)           /*!< 0x00000004 */
#define LPTIM_ISR_EXTTRIG           LPTIM_ISR_EXTTRIG_Msk                      /*!< External trigger edge event */
#define LPTIM_ISR_CMPOK_Pos         (3U)
#define LPTIM_ISR_CMPOK_Msk         (0x1UL << LPTIM_ISR_CMPOK_Pos)             /*!< 0x00000008 */
#define LPTIM_ISR_CMPOK             LPTIM_ISR_CMPOK_Msk                        /*!< Compare register update OK */
#define LPTIM_ISR_ARROK_Pos         (4U)
#define LPTIM_ISR_ARROK_Msk         (0x1UL << LPTIM_ISR_ARROK_Pos)             /*!< 0x00000010 */
#define LPTIM_ISR_ARROK             LPTIM_ISR_ARROK_Msk                        /*!< Autoreload register update OK */
#define LPTIM_ISR_UP_Pos            (5U)
#define LPTIM_ISR_UP_Msk            (0x1UL << LPTIM_ISR_UP_Pos)                /*!< 0x00000020 */
#define LPTIM_ISR_UP                LPTIM_ISR_UP_Msk                           /*!< Counter direction change down to up */
#define LPTIM_ISR_DOWN_Pos          (6U)
#define LPTIM_ISR_DOWN_Msk          (0x1UL << LPTIM_ISR_DOWN_Pos)              /*!< 0x00000040 */
#define LPTIM_ISR_DOWN              LPTIM_ISR_DOWN_Msk                         /*!< Counter direction change up to down */
#define LPTIM_ISR_UE_Pos            (7U)
#define LPTIM_ISR_UE_Msk            (0x1UL << LPTIM_ISR_UE_Pos)                /*!< 0x00000080 */
#define LPTIM_ISR_UE                LPTIM_ISR_UE_Msk                           /*!< Update event occurrence */
#define LPTIM_ISR_REPOK_Pos         (8U)
#define LPTIM_ISR_REPOK_Msk         (0x1UL << LPTIM_ISR_REPOK_Pos)             /*!< 0x00000100 */
#define LPTIM_ISR_REPOK             LPTIM_ISR_REPOK_Msk                        /*!< Repetition register update OK */

/******************  Bit definition for LPTIM_ICR register  *******************/
#define LPTIM_ICR_CMPMCF_Pos        (0U)
#define LPTIM_ICR_CMPMCF_Msk        (0x1UL << LPTIM_ICR_CMPMCF_Pos)            /*!< 0x00000001 */
#define LPTIM_ICR_CMPMCF            LPTIM_ICR_CMPMCF_Msk                       /*!< Compare match Clear Flag */
#define LPTIM_ICR_ARRMCF_Pos        (1U)
#define LPTIM_ICR_ARRMCF_Msk        (0x1UL << LPTIM_ICR_ARRMCF_Pos)            /*!< 0x00000002 */
#define LPTIM_ICR_ARRMCF            LPTIM_ICR_ARRMCF_Msk                       /*!< Autoreload match Clear Flag */
#define LPTIM_ICR_EXTTRIGCF_Pos     (2U)
#define LPTIM_ICR_EXTTRIGCF_Msk     (0x1UL << LPTIM_ICR_EXTTRIGCF_Pos)         /*!< 0x00000004 */
#define LPTIM_ICR_EXTTRIGCF         LPTIM_ICR_EXTTRIGCF_Msk                    /*!< External trigger edge event Clear Flag */
#define LPTIM_ICR_CMPOKCF_Pos       (3U)
#define LPTIM_ICR_CMPOKCF_Msk       (0x1UL << LPTIM_ICR_CMPOKCF_Pos)           /*!< 0x00000008 */
#define LPTIM_ICR_CMPOKCF           LPTIM_ICR_CMPOKCF_Msk                      /*!< Compare register update OK Clear Flag */
#define LPTIM_ICR_ARROKCF_Pos       (4U)
#define LPTIM_ICR_ARROKCF_Msk       (0x1UL << LPTIM_ICR_ARROKCF_Pos)           /*!< 0x00000010 */
#define LPTIM_ICR_ARROKCF           LPTIM_ICR_ARROKCF_Msk                      /*!< Autoreload register update OK Clear Flag */
#define LPTIM_ICR_UPCF_Pos          (5U)
#define LPTIM_ICR_UPCF_Msk          (0x1UL << LPTIM_ICR_UPCF_Pos)              /*!< 0x00000020 */
#define LPTIM_ICR_UPCF              LPTIM_ICR_UPCF_Msk                         /*!< Counter direction change down to up Clear Flag */
#define LPTIM_ICR_DOWNCF_Pos        (6U)
#define LPTIM_ICR_DOWNCF_Msk        (0x1UL << LPTIM_ICR_DOWNCF_Pos)            /*!< 0x00000040 */
#define LPTIM_ICR_DOWNCF            LPTIM_ICR_DOWNCF_Msk                       /*!< Counter direction change up to down Clear Flag */
#define LPTIM_ICR_UECF_Pos          (7U)
#define LPTIM_ICR_UECF_Msk          (0x1UL << LPTIM_ICR_UECF_Pos)               /*!< 0x00000080 */
#define LPTIM_ICR_UECF              LPTIM_ICR_UECF_Msk                         /*!< Update event Clear Flag */
#define LPTIM_ICR_REPOKCF_Pos       (8U)
#define LPTIM_ICR_REPOKCF_Msk       (0x1UL << LPTIM_ICR_REPOKCF_Pos)            /*!< 0x00000100 */
#define LPTIM_ICR_REPOKCF           LPTIM_ICR_REPOKCF_Msk                      /*!< Repetition register update OK Clear Flag */

/******************  Bit definition for LPTIM_IER register ********************/
#define LPTIM_IER_CMPMIE_Pos        (0U)
#define LPTIM_IER_CMPMIE_Msk        (0x1UL << LPTIM_IER_CMPMIE_Pos)            /*!< 0x00000001 */
#define LPTIM_IER_CMPMIE            LPTIM_IER_CMPMIE_Msk                       /*!< Compare match Interrupt Enable */
#define LPTIM_IER_ARRMIE_Pos        (1U)
#define LPTIM_IER_ARRMIE_Msk        (0x1UL << LPTIM_IER_ARRMIE_Pos)            /*!< 0x00000002 */
#define LPTIM_IER_ARRMIE            LPTIM_IER_ARRMIE_Msk                       /*!< Autoreload match Interrupt Enable */
#define LPTIM_IER_EXTTRIGIE_Pos     (2U)
#define LPTIM_IER_EXTTRIGIE_Msk     (0x1UL << LPTIM_IER_EXTTRIGIE_Pos)         /*!< 0x00000004 */
#define LPTIM_IER_EXTTRIGIE         LPTIM_IER_EXTTRIGIE_Msk                    /*!< External trigger edge event Interrupt Enable */
#define LPTIM_IER_CMPOKIE_Pos       (3U)
#define LPTIM_IER_CMPOKIE_Msk       (0x1UL << LPTIM_IER_CMPOKIE_Pos)           /*!< 0x00000008 */
#define LPTIM_IER_CMPOKIE           LPTIM_IER_CMPOKIE_Msk                      /*!< Compare register update OK Interrupt Enable */
#define LPTIM_IER_ARROKIE_Pos       (4U)
#define LPTIM_IER_ARROKIE_Msk       (0x1UL << LPTIM_IER_ARROKIE_Pos)           /*!< 0x00000010 */
#define LPTIM_IER_ARROKIE           LPTIM_IER_ARROKIE_Msk                      /*!< Autoreload register update OK Interrupt Enable */
#define LPTIM_IER_UPIE_Pos          (5U)
#define LPTIM_IER_UPIE_Msk          (0x1UL << LPTIM_IER_UPIE_Pos)              /*!< 0x00000020 */
#define LPTIM_IER_UPIE              LPTIM_IER_UPIE_Msk                         /*!< Counter direction change down to up Interrupt Enable */
#define LPTIM_IER_DOWNIE_Pos        (6U)
#define LPTIM_IER_DOWNIE_Msk        (0x1UL << LPTIM_IER_DOWNIE_Pos)            /*!< 0x00000040 */
#define LPTIM_IER_DOWNIE            LPTIM_IER_DOWNIE_Msk                       /*!< Counter direction change up to down Interrupt Enable */
#define LPTIM_IER_UEIE_Pos          (7U)
#define LPTIM_IER_UEIE_Msk          (0x1UL << LPTIM_IER_UEIE_Pos)              /*!< 0x00000080 */
#define LPTIM_IER_UEIE              LPTIM_IER_UEIE_Msk                         /*!< Update event Interrupt Enable */
#define LPTIM_IER_REPOKIE_Pos       (8U)
#define LPTIM_IER_REPOKIE_Msk       (0x1UL << LPTIM_IER_REPOKIE_Pos)           /*!< 0x00000100 */
#define LPTIM_IER_REPOKIE           LPTIM_IER_REPOKIE_Msk                      /*!< Repetition register update OK Interrupt Enable */

/******************  Bit definition for LPTIM_CFGR register *******************/
#define LPTIM_CFGR_CKSEL_Pos        (0U)
#define LPTIM_CFGR_CKSEL_Msk        (0x1UL << LPTIM_CFGR_CKSEL_Pos)            /*!< 0x00000001 */
#define LPTIM_CFGR_CKSEL            LPTIM_CFGR_CKSEL_Msk                       /*!< Clock selector */

#define LPTIM_CFGR_CKPOL_Pos        (1U)
#define LPTIM_CFGR_CKPOL_Msk        (0x3UL << LPTIM_CFGR_CKPOL_Pos)            /*!< 0x00000006 */
#define LPTIM_CFGR_CKPOL            LPTIM_CFGR_CKPOL_Msk                       /*!< CKPOL[1:0] bits (Clock polarity) */
#define LPTIM_CFGR_CKPOL_0          (0x1UL << LPTIM_CFGR_CKPOL_Pos)            /*!< 0x00000002 */
#define LPTIM_CFGR_CKPOL_1          (0x2UL << LPTIM_CFGR_CKPOL_Pos)            /*!< 0x00000004 */

#define LPTIM_CFGR_CKFLT_Pos        (3U)
#define LPTIM_CFGR_CKFLT_Msk        (0x3UL << LPTIM_CFGR_CKFLT_Pos)            /*!< 0x00000018 */
#define LPTIM_CFGR_CKFLT            LPTIM_CFGR_CKFLT_Msk                       /*!< CKFLT[1:0] bits (Configurable digital filter for external clock) */
#define LPTIM_CFGR_CKFLT_0          (0x1UL << LPTIM_CFGR_CKFLT_Pos)            /*!< 0x00000008 */
#define LPTIM_CFGR_CKFLT_1          (0x2UL << LPTIM_CFGR_CKFLT_Pos)            /*!< 0x00000010 */

#define LPTIM_CFGR_TRGFLT_Pos       (6U)
#define LPTIM_CFGR_TRGFLT_Msk       (0x3UL << LPTIM_CFGR_TRGFLT_Pos)           /*!< 0x000000C0 */
#define LPTIM_CFGR_TRGFLT           LPTIM_CFGR_TRGFLT_Msk                      /*!< TRGFLT[1:0] bits (Configurable digital filter for trigger) */
#define LPTIM_CFGR_TRGFLT_0         (0x1UL << LPTIM_CFGR_TRGFLT_Pos)           /*!< 0x00000040 */
#define LPTIM_CFGR_TRGFLT_1         (0x2UL << LPTIM_CFGR_TRGFLT_Pos)           /*!< 0x00000080 */

#define LPTIM_CFGR_PRESC_Pos        (9U)
#define LPTIM_CFGR_PRESC_Msk        (0x7UL << LPTIM_CFGR_PRESC_Pos)            /*!< 0x00000E00 */
#define LPTIM_CFGR_PRESC            LPTIM_CFGR_PRESC_Msk                       /*!< PRESC[2:0] bits (Clock prescaler) */
#define LPTIM_CFGR_PRESC_0          (0x1UL << LPTIM_CFGR_PRESC_Pos)            /*!< 0x00000200 */
#define LPTIM_CFGR_PRESC_1          (0x2UL << LPTIM_CFGR_PRESC_Pos)            /*!< 0x00000400 */
#define LPTIM_CFGR_PRESC_2          (0x4UL << LPTIM_CFGR_PRESC_Pos)            /*!< 0x00000800 */

#define LPTIM_CFGR_TRIGSEL_Pos      (13U)
#define LPTIM_CFGR_TRIGSEL_Msk      (0x7UL << LPTIM_CFGR_TRIGSEL_Pos)          /*!< 0x0000E000 */
#define LPTIM_CFGR_TRIGSEL          LPTIM_CFGR_TRIGSEL_Msk                     /*!< TRIGSEL[2:0]] bits (Trigger selector) */
#define LPTIM_CFGR_TRIGSEL_0        (0x1UL << LPTIM_CFGR_TRIGSEL_Pos)          /*!< 0x00002000 */
#define LPTIM_CFGR_TRIGSEL_1        (0x2UL << LPTIM_CFGR_TRIGSEL_Pos)          /*!< 0x00004000 */
#define LPTIM_CFGR_TRIGSEL_2        (0x4UL << LPTIM_CFGR_TRIGSEL_Pos)          /*!< 0x00008000 */

#define LPTIM_CFGR_TRIGEN_Pos       (17U)
#define LPTIM_CFGR_TRIGEN_Msk       (0x3UL << LPTIM_CFGR_TRIGEN_Pos)           /*!< 0x00060000 */
#define LPTIM_CFGR_TRIGEN           LPTIM_CFGR_TRIGEN_Msk                      /*!< TRIGEN[1:0] bits (Trigger enable and polarity) */
#define LPTIM_CFGR_TRIGEN_0         (0x1UL << LPTIM_CFGR_TRIGEN_Pos)           /*!< 0x00020000 */
#define LPTIM_CFGR_TRIGEN_1         (0x2UL << LPTIM_CFGR_TRIGEN_Pos)           /*!< 0x00040000 */

#define LPTIM_CFGR_TIMOUT_Pos       (19U)
#define LPTIM_CFGR_TIMOUT_Msk       (0x1UL << LPTIM_CFGR_TIMOUT_Pos)           /*!< 0x00080000 */
#define LPTIM_CFGR_TIMOUT           LPTIM_CFGR_TIMOUT_Msk                      /*!< Timout enable */
#define LPTIM_CFGR_WAVE_Pos         (20U)
#define LPTIM_CFGR_WAVE_Msk         (0x1UL << LPTIM_CFGR_WAVE_Pos)             /*!< 0x00100000 */
#define LPTIM_CFGR_WAVE             LPTIM_CFGR_WAVE_Msk                        /*!< Waveform shape */
#define LPTIM_CFGR_WAVPOL_Pos       (21U)
#define LPTIM_CFGR_WAVPOL_Msk       (0x1UL << LPTIM_CFGR_WAVPOL_Pos)           /*!< 0x00200000 */
#define LPTIM_CFGR_WAVPOL           LPTIM_CFGR_WAVPOL_Msk                      /*!< Waveform shape polarity */
#define LPTIM_CFGR_PRELOAD_Pos      (22U)
#define LPTIM_CFGR_PRELOAD_Msk      (0x1UL << LPTIM_CFGR_PRELOAD_Pos)          /*!< 0x00400000 */
#define LPTIM_CFGR_PRELOAD          LPTIM_CFGR_PRELOAD_Msk                     /*!< Reg update mode */
#define LPTIM_CFGR_COUNTMODE_Pos    (23U)
#define LPTIM_CFGR_COUNTMODE_Msk    (0x1UL << LPTIM_CFGR_COUNTMODE_Pos)        /*!< 0x00800000 */
#define LPTIM_CFGR_COUNTMODE        LPTIM_CFGR_COUNTMODE_Msk                   /*!< Counter mode enable */
#define LPTIM_CFGR_ENC_Pos          (24U)
#define LPTIM_CFGR_ENC_Msk          (0x1UL << LPTIM_CFGR_ENC_Pos)              /*!< 0x01000000 */
#define LPTIM_CFGR_ENC              LPTIM_CFGR_ENC_Msk                         /*!< Encoder mode enable */

/******************  Bit definition for LPTIM_CR register  ********************/
#define LPTIM_CR_ENABLE_Pos         (0U)
#define LPTIM_CR_ENABLE_Msk         (0x1UL << LPTIM_CR_ENABLE_Pos)             /*!< 0x00000001 */
#define LPTIM_CR_ENABLE             LPTIM_CR_ENABLE_Msk                        /*!< LPTIMer enable */
#define LPTIM_CR_SNGSTRT_Pos        (1U)
#define LPTIM_CR_SNGSTRT_Msk        (0x1UL << LPTIM_CR_SNGSTRT_Pos)            /*!< 0x00000002 */
#define LPTIM_CR_SNGSTRT            LPTIM_CR_SNGSTRT_Msk                       /*!< Timer start in single mode */
#define LPTIM_CR_CNTSTRT_Pos        (2U)
#define LPTIM_CR_CNTSTRT_Msk        (0x1UL << LPTIM_CR_CNTSTRT_Pos)            /*!< 0x00000004 */
#define LPTIM_CR_CNTSTRT            LPTIM_CR_CNTSTRT_Msk                       /*!< Timer start in continuous mode */
#define LPTIM_CR_COUNTRST_Pos       (3U)
#define LPTIM_CR_COUNTRST_Msk       (0x1UL << LPTIM_CR_COUNTRST_Pos)           /*!< 0x00000008 */
#define LPTIM_CR_COUNTRST           LPTIM_CR_COUNTRST_Msk                      /*!< Counter reset */
#define LPTIM_CR_RSTARE_Pos         (4U)
#define LPTIM_CR_RSTARE_Msk         (0x1UL << LPTIM_CR_RSTARE_Pos)             /*!< 0x00000010 */
#define LPTIM_CR_RSTARE             LPTIM_CR_RSTARE_Msk                        /*!< Reset after read enable */

/******************  Bit definition for LPTIM_CMP register  *******************/
#define LPTIM_CMP_CMP_Pos           (0U)
#define LPTIM_CMP_CMP_Msk           (0xFFFFUL << LPTIM_CMP_CMP_Pos)            /*!< 0x0000FFFF */
#define LPTIM_CMP_CMP               LPTIM_CMP_CMP_Msk                          /*!< Compare register */

/******************  Bit definition for LPTIM_ARR register  *******************/
#define LPTIM_ARR_ARR_Pos           (0U)
#define LPTIM_ARR_ARR_Msk           (0xFFFFUL << LPTIM_ARR_ARR_Pos)            /*!< 0x0000FFFF */
#define LPTIM_ARR_ARR               LPTIM_ARR_ARR_Msk                          /*!< Auto reload register */

/******************  Bit definition for LPTIM_CNT register  *******************/
#define LPTIM_CNT_CNT_Pos           (0U)
#define LPTIM_CNT_CNT_Msk           (0xFFFFUL << LPTIM_CNT_CNT_Pos)            /*!< 0x0000FFFF */
#define LPTIM_CNT_CNT               LPTIM_CNT_CNT_Msk                          /*!< Counter register */

/******************  Bit definition for LPTIM_OR register  ********************/
#define LPTIM_OR_OR_Pos             (0U)
#define LPTIM_OR_OR_Msk             (0x3UL << LPTIM_OR_OR_Pos)                 /*!< 0x00000003 */
#define LPTIM_OR_OR                 LPTIM_OR_OR_Msk                            /*!< OR[1:0] bits (Remap selection) */
#define LPTIM_OR_OR_0               (0x1UL << LPTIM_OR_OR_Pos)                 /*!< 0x00000001 */
#define LPTIM_OR_OR_1               (0x2UL << LPTIM_OR_OR_Pos)                 /*!< 0x00000002 */

/******************  Bit definition for LPTIM_RCR register  *******************/
#define LPTIM_RCR_REP_Pos           (0U)
#define LPTIM_RCR_REP_Msk           (0xFFUL << LPTIM_RCR_REP_Pos)              /*!< 0x000000FF */
#define LPTIM_RCR_REP               LPTIM_RCR_REP_Msk                          /*!<Repetition Counter Value */

/******************************************************************************/
/*                                                                            */
/*                      Analog Comparators (COMP)                             */
/*                                                                            */
/******************************************************************************/
/**********************  Bit definition for COMP_CSR register  ****************/
#define COMP_CSR_EN_Pos            (0U)
#define COMP_CSR_EN_Msk            (0x1UL << COMP_CSR_EN_Pos)                  /*!< 0x00000001 */
#define COMP_CSR_EN                COMP_CSR_EN_Msk                             /*!< Comparator enable */

#define COMP_CSR_PWRMODE_Pos       (2U)
#define COMP_CSR_PWRMODE_Msk       (0x3UL << COMP_CSR_PWRMODE_Pos)             /*!< 0x0000000C */
#define COMP_CSR_PWRMODE           COMP_CSR_PWRMODE_Msk                        /*!< Comparator power mode */
#define COMP_CSR_PWRMODE_0         (0x1UL << COMP_CSR_PWRMODE_Pos)             /*!< 0x00000004 */
#define COMP_CSR_PWRMODE_1         (0x2UL << COMP_CSR_PWRMODE_Pos)             /*!< 0x00000008 */

#define COMP_CSR_INMSEL_Pos        (4U)
#define COMP_CSR_INMSEL_Msk        (0x7UL << COMP_CSR_INMSEL_Pos)              /*!< 0x00000070 */
#define COMP_CSR_INMSEL            COMP_CSR_INMSEL_Msk                         /*!< Comparator input minus selection */
#define COMP_CSR_INMSEL_0          (0x1UL << COMP_CSR_INMSEL_Pos)              /*!< 0x00000010 */
#define COMP_CSR_INMSEL_1          (0x2UL << COMP_CSR_INMSEL_Pos)              /*!< 0x00000020 */
#define COMP_CSR_INMSEL_2          (0x4UL << COMP_CSR_INMSEL_Pos)              /*!< 0x00000040 */

#define COMP_CSR_INPSEL_Pos        (7U)
#define COMP_CSR_INPSEL_Msk        (0x3UL << COMP_CSR_INPSEL_Pos)              /*!< 0x00000180 */
#define COMP_CSR_INPSEL            COMP_CSR_INPSEL_Msk                         /*!< Comparator input plus selection */
#define COMP_CSR_INPSEL_0          (0x1UL << COMP_CSR_INPSEL_Pos)              /*!< 0x00000080 */
#define COMP_CSR_INPSEL_1          (0x2UL << COMP_CSR_INPSEL_Pos)              /*!< 0x00000100 */

#define COMP_CSR_POLARITY_Pos      (15U)
#define COMP_CSR_POLARITY_Msk      (0x1UL << COMP_CSR_POLARITY_Pos)            /*!< 0x00008000 */
#define COMP_CSR_POLARITY          COMP_CSR_POLARITY_Msk                       /*!< Comparator output polarity */

#define COMP_CSR_HYST_Pos          (16U)
#define COMP_CSR_HYST_Msk          (0x3UL << COMP_CSR_HYST_Pos)                /*!< 0x00030000 */
#define COMP_CSR_HYST              COMP_CSR_HYST_Msk                           /*!< Comparator hysteresis */
#define COMP_CSR_HYST_0            (0x1UL << COMP_CSR_HYST_Pos)                /*!< 0x00010000 */
#define COMP_CSR_HYST_1            (0x2UL << COMP_CSR_HYST_Pos)                /*!< 0x00020000 */

#define COMP_CSR_BLANKING_Pos      (18U)
#define COMP_CSR_BLANKING_Msk      (0x7UL << COMP_CSR_BLANKING_Pos)            /*!< 0x001C0000 */
#define COMP_CSR_BLANKING          COMP_CSR_BLANKING_Msk                       /*!< Comparator blanking source */
#define COMP_CSR_BLANKING_0        (0x1UL << COMP_CSR_BLANKING_Pos)            /*!< 0x00040000 */
#define COMP_CSR_BLANKING_1        (0x2UL << COMP_CSR_BLANKING_Pos)            /*!< 0x00080000 */
#define COMP_CSR_BLANKING_2        (0x4UL << COMP_CSR_BLANKING_Pos)            /*!< 0x00100000 */

#define COMP_CSR_BRGEN_Pos         (22U)
#define COMP_CSR_BRGEN_Msk         (0x1UL << COMP_CSR_BRGEN_Pos)               /*!< 0x00400000 */
#define COMP_CSR_BRGEN             COMP_CSR_BRGEN_Msk                          /*!< Comparator voltage scaler enable */
#define COMP_CSR_SCALEN_Pos        (23U)
#define COMP_CSR_SCALEN_Msk        (0x1UL << COMP_CSR_SCALEN_Pos)              /*!< 0x00800000 */
#define COMP_CSR_SCALEN            COMP_CSR_SCALEN_Msk                         /*!< Comparator scaler bridge enable */

#define COMP_CSR_INMESEL_Pos       (25U)
#define COMP_CSR_INMESEL_Msk       (0x3UL << COMP_CSR_INMESEL_Pos)             /*!< 0x06000000 */
#define COMP_CSR_INMESEL           COMP_CSR_INMESEL_Msk                        /*!< Comparator input minus extended selection */
#define COMP_CSR_INMESEL_0         (0x1UL << COMP_CSR_INMESEL_Pos)             /*!< 0x02000000 */
#define COMP_CSR_INMESEL_1         (0x2UL << COMP_CSR_INMESEL_Pos)             /*!< 0x04000000 */

#define COMP_CSR_VALUE_Pos         (30U)
#define COMP_CSR_VALUE_Msk         (0x1UL << COMP_CSR_VALUE_Pos)               /*!< 0x40000000 */
#define COMP_CSR_VALUE             COMP_CSR_VALUE_Msk                          /*!< Comparator output level */

#define COMP_CSR_LOCK_Pos          (31U)
#define COMP_CSR_LOCK_Msk          (0x1UL << COMP_CSR_LOCK_Pos)                /*!< 0x80000000 */
#define COMP_CSR_LOCK              COMP_CSR_LOCK_Msk                           /*!< Comparator lock */

/******************************************************************************/
/*                                                                            */
/*                         Operational Amplifier (OPAMP)                      */
/*                                                                            */
/******************************************************************************/
/*********************  Bit definition for OPAMPx_CSR register  ***************/
#define OPAMP_CSR_OPAMPxEN_Pos           (0U)
#define OPAMP_CSR_OPAMPxEN_Msk           (0x1UL << OPAMP_CSR_OPAMPxEN_Pos)     /*!< 0x00000001 */
#define OPAMP_CSR_OPAMPxEN               OPAMP_CSR_OPAMPxEN_Msk                /*!< OPAMP enable */
#define OPAMP_CSR_OPALPM_Pos             (1U)
#define OPAMP_CSR_OPALPM_Msk             (0x1UL << OPAMP_CSR_OPALPM_Pos)       /*!< 0x00000002 */
#define OPAMP_CSR_OPALPM                 OPAMP_CSR_OPALPM_Msk                  /*!< Operational amplifier Low Power Mode */

#define OPAMP_CSR_OPAMODE_Pos            (2U)
#define OPAMP_CSR_OPAMODE_Msk            (0x3UL << OPAMP_CSR_OPAMODE_Pos)      /*!< 0x0000000C */
#define OPAMP_CSR_OPAMODE                OPAMP_CSR_OPAMODE_Msk                 /*!< Operational amplifier PGA mode */
#define OPAMP_CSR_OPAMODE_0              (0x1UL << OPAMP_CSR_OPAMODE_Pos)      /*!< 0x00000004 */
#define OPAMP_CSR_OPAMODE_1              (0x2UL << OPAMP_CSR_OPAMODE_Pos)      /*!< 0x00000008 */

#define OPAMP_CSR_PGGAIN_Pos             (4U)
#define OPAMP_CSR_PGGAIN_Msk             (0x3UL << OPAMP_CSR_PGGAIN_Pos)       /*!< 0x00000030 */
#define OPAMP_CSR_PGGAIN                 OPAMP_CSR_PGGAIN_Msk                  /*!< Operational amplifier Programmable amplifier gain value */
#define OPAMP_CSR_PGGAIN_0               (0x1UL << OPAMP_CSR_PGGAIN_Pos)       /*!< 0x00000010 */
#define OPAMP_CSR_PGGAIN_1               (0x2UL << OPAMP_CSR_PGGAIN_Pos)       /*!< 0x00000020 */

#define OPAMP_CSR_VMSEL_Pos              (8U)
#define OPAMP_CSR_VMSEL_Msk              (0x3UL << OPAMP_CSR_VMSEL_Pos)        /*!< 0x00000300 */
#define OPAMP_CSR_VMSEL                  OPAMP_CSR_VMSEL_Msk                   /*!< Inverting input selection */
#define OPAMP_CSR_VMSEL_0                (0x1UL << OPAMP_CSR_VMSEL_Pos)        /*!< 0x00000100 */
#define OPAMP_CSR_VMSEL_1                (0x2UL << OPAMP_CSR_VMSEL_Pos)        /*!< 0x00000200 */

#define OPAMP_CSR_VPSEL_Pos              (10U)
#define OPAMP_CSR_VPSEL_Msk              (0x1UL << OPAMP_CSR_VPSEL_Pos)        /*!< 0x00000400 */
#define OPAMP_CSR_VPSEL                  OPAMP_CSR_VPSEL_Msk                   /*!< Non inverted input selection */
#define OPAMP_CSR_CALON_Pos              (12U)
#define OPAMP_CSR_CALON_Msk              (0x1UL << OPAMP_CSR_CALON_Pos)        /*!< 0x00001000 */
#define OPAMP_CSR_CALON                  OPAMP_CSR_CALON_Msk                   /*!< Calibration mode enable */
#define OPAMP_CSR_CALSEL_Pos             (13U)
#define OPAMP_CSR_CALSEL_Msk             (0x1UL << OPAMP_CSR_CALSEL_Pos)       /*!< 0x00002000 */
#define OPAMP_CSR_CALSEL                 OPAMP_CSR_CALSEL_Msk                  /*!< Calibration selection */
#define OPAMP_CSR_USERTRIM_Pos           (14U)
#define OPAMP_CSR_USERTRIM_Msk           (0x1UL << OPAMP_CSR_USERTRIM_Pos)     /*!< 0x00004000 */
#define OPAMP_CSR_USERTRIM               OPAMP_CSR_USERTRIM_Msk                /*!< User trimming enable */
#define OPAMP_CSR_CALOUT_Pos             (15U)
#define OPAMP_CSR_CALOUT_Msk             (0x1UL << OPAMP_CSR_CALOUT_Pos)       /*!< 0x00008000 */
#define OPAMP_CSR_CALOUT                 OPAMP_CSR_CALOUT_Msk                  /*!< Operational amplifier1 calibration output */

/*********************  Bit definition for OPAMP1_CSR register  ***************/
#define OPAMP1_CSR_OPAEN_Pos              (0U)
#define OPAMP1_CSR_OPAEN_Msk              (0x1UL << OPAMP1_CSR_OPAEN_Pos)      /*!< 0x00000001 */
#define OPAMP1_CSR_OPAEN                  OPAMP1_CSR_OPAEN_Msk                 /*!< Operational amplifier1 Enable */
#define OPAMP1_CSR_OPALPM_Pos             (1U)
#define OPAMP1_CSR_OPALPM_Msk             (0x1UL << OPAMP1_CSR_OPALPM_Pos)     /*!< 0x00000002 */
#define OPAMP1_CSR_OPALPM                 OPAMP1_CSR_OPALPM_Msk                /*!< Operational amplifier1 Low Power Mode */

#define OPAMP1_CSR_OPAMODE_Pos            (2U)
#define OPAMP1_CSR_OPAMODE_Msk            (0x3UL << OPAMP1_CSR_OPAMODE_Pos)    /*!< 0x0000000C */
#define OPAMP1_CSR_OPAMODE                OPAMP1_CSR_OPAMODE_Msk               /*!< Operational amplifier1 PGA mode */
#define OPAMP1_CSR_OPAMODE_0              (0x1UL << OPAMP1_CSR_OPAMODE_Pos)    /*!< 0x00000004 */
#define OPAMP1_CSR_OPAMODE_1              (0x2UL << OPAMP1_CSR_OPAMODE_Pos)    /*!< 0x00000008 */

#define OPAMP1_CSR_PGAGAIN_Pos            (4U)
#define OPAMP1_CSR_PGAGAIN_Msk            (0x3UL << OPAMP1_CSR_PGAGAIN_Pos)    /*!< 0x00000030 */
#define OPAMP1_CSR_PGAGAIN                OPAMP1_CSR_PGAGAIN_Msk               /*!< Operational amplifier1 Programmable amplifier gain value */
#define OPAMP1_CSR_PGAGAIN_0              (0x1UL << OPAMP1_CSR_PGAGAIN_Pos)    /*!< 0x00000010 */
#define OPAMP1_CSR_PGAGAIN_1              (0x2UL << OPAMP1_CSR_PGAGAIN_Pos)    /*!< 0x00000020 */

#define OPAMP1_CSR_VMSEL_Pos              (8U)
#define OPAMP1_CSR_VMSEL_Msk              (0x3UL << OPAMP1_CSR_VMSEL_Pos)      /*!< 0x00000300 */
#define OPAMP1_CSR_VMSEL                  OPAMP1_CSR_VMSEL_Msk                 /*!< Inverting input selection */
#define OPAMP1_CSR_VMSEL_0                (0x1UL << OPAMP1_CSR_VMSEL_Pos)      /*!< 0x00000100 */
#define OPAMP1_CSR_VMSEL_1                (0x2UL << OPAMP1_CSR_VMSEL_Pos)      /*!< 0x00000200 */

#define OPAMP1_CSR_VPSEL_Pos              (10U)
#define OPAMP1_CSR_VPSEL_Msk              (0x1UL << OPAMP1_CSR_VPSEL_Pos)      /*!< 0x00000400 */
#define OPAMP1_CSR_VPSEL                  OPAMP1_CSR_VPSEL_Msk                 /*!< Non inverted input selection */
#define OPAMP1_CSR_CALON_Pos              (12U)
#define OPAMP1_CSR_CALON_Msk              (0x1UL << OPAMP1_CSR_CALON_Pos)      /*!< 0x00001000 */
#define OPAMP1_CSR_CALON                  OPAMP1_CSR_CALON_Msk                 /*!< Calibration mode enable */
#define OPAMP1_CSR_CALSEL_Pos             (13U)
#define OPAMP1_CSR_CALSEL_Msk             (0x1UL << OPAMP1_CSR_CALSEL_Pos)     /*!< 0x00002000 */
#define OPAMP1_CSR_CALSEL                 OPAMP1_CSR_CALSEL_Msk                /*!< Calibration selection */
#define OPAMP1_CSR_USERTRIM_Pos           (14U)
#define OPAMP1_CSR_USERTRIM_Msk           (0x1UL << OPAMP1_CSR_USERTRIM_Pos)   /*!< 0x00004000 */
#define OPAMP1_CSR_USERTRIM               OPAMP1_CSR_USERTRIM_Msk              /*!< User trimming enable */
#define OPAMP1_CSR_CALOUT_Pos             (15U)
#define OPAMP1_CSR_CALOUT_Msk             (0x1UL << OPAMP1_CSR_CALOUT_Pos)     /*!< 0x00008000 */
#define OPAMP1_CSR_CALOUT                 OPAMP1_CSR_CALOUT_Msk                /*!< Operational amplifier1 calibration output */

#define OPAMP1_CSR_OPARANGE_Pos           (31U)
#define OPAMP1_CSR_OPARANGE_Msk           (0x1UL << OPAMP1_CSR_OPARANGE_Pos)   /*!< 0x80000000 */
#define OPAMP1_CSR_OPARANGE               OPAMP1_CSR_OPARANGE_Msk              /*!< Common to several OPAMP instances: Operational amplifier voltage supply range. Bit intended to be used with OPAMP common instance (OPAMP_Common_TypeDef) */

/*******************  Bit definition for OPAMP_OTR register  ******************/
#define OPAMP_OTR_TRIMOFFSETN_Pos        (0U)
#define OPAMP_OTR_TRIMOFFSETN_Msk        (0x1FUL << OPAMP_OTR_TRIMOFFSETN_Pos) /*!< 0x0000001F */
#define OPAMP_OTR_TRIMOFFSETN            OPAMP_OTR_TRIMOFFSETN_Msk             /*!< Trim for NMOS differential pairs */
#define OPAMP_OTR_TRIMOFFSETP_Pos        (8U)
#define OPAMP_OTR_TRIMOFFSETP_Msk        (0x1FUL << OPAMP_OTR_TRIMOFFSETP_Pos) /*!< 0x00001F00 */
#define OPAMP_OTR_TRIMOFFSETP            OPAMP_OTR_TRIMOFFSETP_Msk             /*!< Trim for PMOS differential pairs */

/*******************  Bit definition for OPAMP1_OTR register  ******************/
#define OPAMP1_OTR_TRIMOFFSETN_Pos        (0U)
#define OPAMP1_OTR_TRIMOFFSETN_Msk        (0x1FUL << OPAMP1_OTR_TRIMOFFSETN_Pos) /*!< 0x0000001F */
#define OPAMP1_OTR_TRIMOFFSETN            OPAMP1_OTR_TRIMOFFSETN_Msk           /*!< Trim for NMOS differential pairs */
#define OPAMP1_OTR_TRIMOFFSETP_Pos        (8U)
#define OPAMP1_OTR_TRIMOFFSETP_Msk        (0x1FUL << OPAMP1_OTR_TRIMOFFSETP_Pos) /*!< 0x00001F00 */
#define OPAMP1_OTR_TRIMOFFSETP            OPAMP1_OTR_TRIMOFFSETP_Msk           /*!< Trim for PMOS differential pairs */

/*******************  Bit definition for OPAMP_LPOTR register  ****************/
#define OPAMP_LPOTR_TRIMLPOFFSETN_Pos    (0U)
#define OPAMP_LPOTR_TRIMLPOFFSETN_Msk    (0x1FUL << OPAMP_LPOTR_TRIMLPOFFSETN_Pos) /*!< 0x0000001F */
#define OPAMP_LPOTR_TRIMLPOFFSETN        OPAMP_LPOTR_TRIMLPOFFSETN_Msk         /*!< Trim for NMOS differential pairs */
#define OPAMP_LPOTR_TRIMLPOFFSETP_Pos    (8U)
#define OPAMP_LPOTR_TRIMLPOFFSETP_Msk    (0x1FUL << OPAMP_LPOTR_TRIMLPOFFSETP_Pos) /*!< 0x00001F00 */
#define OPAMP_LPOTR_TRIMLPOFFSETP        OPAMP_LPOTR_TRIMLPOFFSETP_Msk         /*!< Trim for PMOS differential pairs */

/*******************  Bit definition for OPAMP1_LPOTR register  ****************/
#define OPAMP1_LPOTR_TRIMLPOFFSETN_Pos    (0U)
#define OPAMP1_LPOTR_TRIMLPOFFSETN_Msk    (0x1FUL << OPAMP1_LPOTR_TRIMLPOFFSETN_Pos) /*!< 0x0000001F */
#define OPAMP1_LPOTR_TRIMLPOFFSETN        OPAMP1_LPOTR_TRIMLPOFFSETN_Msk       /*!< Trim for NMOS differential pairs */
#define OPAMP1_LPOTR_TRIMLPOFFSETP_Pos    (8U)
#define OPAMP1_LPOTR_TRIMLPOFFSETP_Msk    (0x1FUL << OPAMP1_LPOTR_TRIMLPOFFSETP_Pos) /*!< 0x00001F00 */
#define OPAMP1_LPOTR_TRIMLPOFFSETP        OPAMP1_LPOTR_TRIMLPOFFSETP_Msk       /*!< Trim for PMOS differential pairs */

/******************************************************************************/
/*                                                                            */
/*                          Touch Sensing Controller (TSC)                    */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for TSC_CR register  *********************/
#define TSC_CR_TSCE_Pos          (0U)
#define TSC_CR_TSCE_Msk          (0x1UL << TSC_CR_TSCE_Pos)                    /*!< 0x00000001 */
#define TSC_CR_TSCE              TSC_CR_TSCE_Msk                               /*!<Touch sensing controller enable */
#define TSC_CR_START_Pos         (1U)
#define TSC_CR_START_Msk         (0x1UL << TSC_CR_START_Pos)                   /*!< 0x00000002 */
#define TSC_CR_START             TSC_CR_START_Msk                              /*!<Start acquisition */
#define TSC_CR_AM_Pos            (2U)
#define TSC_CR_AM_Msk            (0x1UL << TSC_CR_AM_Pos)                      /*!< 0x00000004 */
#define TSC_CR_AM                TSC_CR_AM_Msk                                 /*!<Acquisition mode */
#define TSC_CR_SYNCPOL_Pos       (3U)
#define TSC_CR_SYNCPOL_Msk       (0x1UL << TSC_CR_SYNCPOL_Pos)                 /*!< 0x00000008 */
#define TSC_CR_SYNCPOL           TSC_CR_SYNCPOL_Msk                            /*!<Synchronization pin polarity */
#define TSC_CR_IODEF_Pos         (4U)
#define TSC_CR_IODEF_Msk         (0x1UL << TSC_CR_IODEF_Pos)                   /*!< 0x00000010 */
#define TSC_CR_IODEF             TSC_CR_IODEF_Msk                              /*!<IO default mode */

#define TSC_CR_MCV_Pos           (5U)
#define TSC_CR_MCV_Msk           (0x7UL << TSC_CR_MCV_Pos)                     /*!< 0x000000E0 */
#define TSC_CR_MCV               TSC_CR_MCV_Msk                                /*!<MCV[2:0] bits (Max Count Value) */
#define TSC_CR_MCV_0             (0x1UL << TSC_CR_MCV_Pos)                     /*!< 0x00000020 */
#define TSC_CR_MCV_1             (0x2UL << TSC_CR_MCV_Pos)                     /*!< 0x00000040 */
#define TSC_CR_MCV_2             (0x4UL << TSC_CR_MCV_Pos)                     /*!< 0x00000080 */

#define TSC_CR_PGPSC_Pos         (12U)
#define TSC_CR_PGPSC_Msk         (0x7UL << TSC_CR_PGPSC_Pos)                   /*!< 0x00007000 */
#define TSC_CR_PGPSC             TSC_CR_PGPSC_Msk                              /*!<PGPSC[2:0] bits (Pulse Generator Prescaler) */
#define TSC_CR_PGPSC_0           (0x1UL << TSC_CR_PGPSC_Pos)                   /*!< 0x00001000 */
#define TSC_CR_PGPSC_1           (0x2UL << TSC_CR_PGPSC_Pos)                   /*!< 0x00002000 */
#define TSC_CR_PGPSC_2           (0x4UL << TSC_CR_PGPSC_Pos)                   /*!< 0x00004000 */

#define TSC_CR_SSPSC_Pos         (15U)
#define TSC_CR_SSPSC_Msk         (0x1UL << TSC_CR_SSPSC_Pos)                   /*!< 0x00008000 */
#define TSC_CR_SSPSC             TSC_CR_SSPSC_Msk                              /*!<Spread Spectrum Prescaler */
#define TSC_CR_SSE_Pos           (16U)
#define TSC_CR_SSE_Msk           (0x1UL << TSC_CR_SSE_Pos)                     /*!< 0x00010000 */
#define TSC_CR_SSE               TSC_CR_SSE_Msk                                /*!<Spread Spectrum Enable */

#define TSC_CR_SSD_Pos           (17U)
#define TSC_CR_SSD_Msk           (0x7FUL << TSC_CR_SSD_Pos)                    /*!< 0x00FE0000 */
#define TSC_CR_SSD               TSC_CR_SSD_Msk                                /*!<SSD[6:0] bits (Spread Spectrum Deviation) */
#define TSC_CR_SSD_0             (0x01UL << TSC_CR_SSD_Pos)                    /*!< 0x00020000 */
#define TSC_CR_SSD_1             (0x02UL << TSC_CR_SSD_Pos)                    /*!< 0x00040000 */
#define TSC_CR_SSD_2             (0x04UL << TSC_CR_SSD_Pos)                    /*!< 0x00080000 */
#define TSC_CR_SSD_3             (0x08UL << TSC_CR_SSD_Pos)                    /*!< 0x00100000 */
#define TSC_CR_SSD_4             (0x10UL << TSC_CR_SSD_Pos)                    /*!< 0x00200000 */
#define TSC_CR_SSD_5             (0x20UL << TSC_CR_SSD_Pos)                    /*!< 0x00400000 */
#define TSC_CR_SSD_6             (0x40UL << TSC_CR_SSD_Pos)                    /*!< 0x00800000 */

#define TSC_CR_CTPL_Pos          (24U)
#define TSC_CR_CTPL_Msk          (0xFUL << TSC_CR_CTPL_Pos)                    /*!< 0x0F000000 */
#define TSC_CR_CTPL              TSC_CR_CTPL_Msk                               /*!<CTPL[3:0] bits (Charge Transfer pulse low) */
#define TSC_CR_CTPL_0            (0x1UL << TSC_CR_CTPL_Pos)                    /*!< 0x01000000 */
#define TSC_CR_CTPL_1            (0x2UL << TSC_CR_CTPL_Pos)                    /*!< 0x02000000 */
#define TSC_CR_CTPL_2            (0x4UL << TSC_CR_CTPL_Pos)                    /*!< 0x04000000 */
#define TSC_CR_CTPL_3            (0x8UL << TSC_CR_CTPL_Pos)                    /*!< 0x08000000 */

#define TSC_CR_CTPH_Pos          (28U)
#define TSC_CR_CTPH_Msk          (0xFUL << TSC_CR_CTPH_Pos)                    /*!< 0xF0000000 */
#define TSC_CR_CTPH              TSC_CR_CTPH_Msk                               /*!<CTPH[3:0] bits (Charge Transfer pulse high) */
#define TSC_CR_CTPH_0            (0x1UL << TSC_CR_CTPH_Pos)                    /*!< 0x10000000 */
#define TSC_CR_CTPH_1            (0x2UL << TSC_CR_CTPH_Pos)                    /*!< 0x20000000 */
#define TSC_CR_CTPH_2            (0x4UL << TSC_CR_CTPH_Pos)                    /*!< 0x40000000 */
#define TSC_CR_CTPH_3            (0x8UL << TSC_CR_CTPH_Pos)                    /*!< 0x80000000 */

/*******************  Bit definition for TSC_IER register  ********************/
#define TSC_IER_EOAIE_Pos        (0U)
#define TSC_IER_EOAIE_Msk        (0x1UL << TSC_IER_EOAIE_Pos)                  /*!< 0x00000001 */
#define TSC_IER_EOAIE            TSC_IER_EOAIE_Msk                             /*!<End of acquisition interrupt enable */
#define TSC_IER_MCEIE_Pos        (1U)
#define TSC_IER_MCEIE_Msk        (0x1UL << TSC_IER_MCEIE_Pos)                  /*!< 0x00000002 */
#define TSC_IER_MCEIE            TSC_IER_MCEIE_Msk                             /*!<Max count error interrupt enable */

/*******************  Bit definition for TSC_ICR register  ********************/
#define TSC_ICR_EOAIC_Pos        (0U)
#define TSC_ICR_EOAIC_Msk        (0x1UL << TSC_ICR_EOAIC_Pos)                  /*!< 0x00000001 */
#define TSC_ICR_EOAIC            TSC_ICR_EOAIC_Msk                             /*!<End of acquisition interrupt clear */
#define TSC_ICR_MCEIC_Pos        (1U)
#define TSC_ICR_MCEIC_Msk        (0x1UL << TSC_ICR_MCEIC_Pos)                  /*!< 0x00000002 */
#define TSC_ICR_MCEIC            TSC_ICR_MCEIC_Msk                             /*!<Max count error interrupt clear */

/*******************  Bit definition for TSC_ISR register  ********************/
#define TSC_ISR_EOAF_Pos         (0U)
#define TSC_ISR_EOAF_Msk         (0x1UL << TSC_ISR_EOAF_Pos)                   /*!< 0x00000001 */
#define TSC_ISR_EOAF             TSC_ISR_EOAF_Msk                              /*!<End of acquisition flag */
#define TSC_ISR_MCEF_Pos         (1U)
#define TSC_ISR_MCEF_Msk         (0x1UL << TSC_ISR_MCEF_Pos)                   /*!< 0x00000002 */
#define TSC_ISR_MCEF             TSC_ISR_MCEF_Msk                              /*!<Max count error flag */

/*******************  Bit definition for TSC_IOHCR register  ******************/
#define TSC_IOHCR_G1_IO1_Pos     (0U)
#define TSC_IOHCR_G1_IO1_Msk     (0x1UL << TSC_IOHCR_G1_IO1_Pos)               /*!< 0x00000001 */
#define TSC_IOHCR_G1_IO1         TSC_IOHCR_G1_IO1_Msk                          /*!<GROUP1_IO1 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G1_IO2_Pos     (1U)
#define TSC_IOHCR_G1_IO2_Msk     (0x1UL << TSC_IOHCR_G1_IO2_Pos)               /*!< 0x00000002 */
#define TSC_IOHCR_G1_IO2         TSC_IOHCR_G1_IO2_Msk                          /*!<GROUP1_IO2 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G1_IO3_Pos     (2U)
#define TSC_IOHCR_G1_IO3_Msk     (0x1UL << TSC_IOHCR_G1_IO3_Pos)               /*!< 0x00000004 */
#define TSC_IOHCR_G1_IO3         TSC_IOHCR_G1_IO3_Msk                          /*!<GROUP1_IO3 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G1_IO4_Pos     (3U)
#define TSC_IOHCR_G1_IO4_Msk     (0x1UL << TSC_IOHCR_G1_IO4_Pos)               /*!< 0x00000008 */
#define TSC_IOHCR_G1_IO4         TSC_IOHCR_G1_IO4_Msk                          /*!<GROUP1_IO4 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G2_IO1_Pos     (4U)
#define TSC_IOHCR_G2_IO1_Msk     (0x1UL << TSC_IOHCR_G2_IO1_Pos)               /*!< 0x00000010 */
#define TSC_IOHCR_G2_IO1         TSC_IOHCR_G2_IO1_Msk                          /*!<GROUP2_IO1 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G2_IO2_Pos     (5U)
#define TSC_IOHCR_G2_IO2_Msk     (0x1UL << TSC_IOHCR_G2_IO2_Pos)               /*!< 0x00000020 */
#define TSC_IOHCR_G2_IO2         TSC_IOHCR_G2_IO2_Msk                          /*!<GROUP2_IO2 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G2_IO3_Pos     (6U)
#define TSC_IOHCR_G2_IO3_Msk     (0x1UL << TSC_IOHCR_G2_IO3_Pos)               /*!< 0x00000040 */
#define TSC_IOHCR_G2_IO3         TSC_IOHCR_G2_IO3_Msk                          /*!<GROUP2_IO3 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G2_IO4_Pos     (7U)
#define TSC_IOHCR_G2_IO4_Msk     (0x1UL << TSC_IOHCR_G2_IO4_Pos)               /*!< 0x00000080 */
#define TSC_IOHCR_G2_IO4         TSC_IOHCR_G2_IO4_Msk                          /*!<GROUP2_IO4 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G3_IO1_Pos     (8U)
#define TSC_IOHCR_G3_IO1_Msk     (0x1UL << TSC_IOHCR_G3_IO1_Pos)               /*!< 0x00000100 */
#define TSC_IOHCR_G3_IO1         TSC_IOHCR_G3_IO1_Msk                          /*!<GROUP3_IO1 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G3_IO2_Pos     (9U)
#define TSC_IOHCR_G3_IO2_Msk     (0x1UL << TSC_IOHCR_G3_IO2_Pos)               /*!< 0x00000200 */
#define TSC_IOHCR_G3_IO2         TSC_IOHCR_G3_IO2_Msk                          /*!<GROUP3_IO2 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G3_IO3_Pos     (10U)
#define TSC_IOHCR_G3_IO3_Msk     (0x1UL << TSC_IOHCR_G3_IO3_Pos)               /*!< 0x00000400 */
#define TSC_IOHCR_G3_IO3         TSC_IOHCR_G3_IO3_Msk                          /*!<GROUP3_IO3 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G3_IO4_Pos     (11U)
#define TSC_IOHCR_G3_IO4_Msk     (0x1UL << TSC_IOHCR_G3_IO4_Pos)               /*!< 0x00000800 */
#define TSC_IOHCR_G3_IO4         TSC_IOHCR_G3_IO4_Msk                          /*!<GROUP3_IO4 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G4_IO1_Pos     (12U)
#define TSC_IOHCR_G4_IO1_Msk     (0x1UL << TSC_IOHCR_G4_IO1_Pos)               /*!< 0x00001000 */
#define TSC_IOHCR_G4_IO1         TSC_IOHCR_G4_IO1_Msk                          /*!<GROUP4_IO1 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G4_IO2_Pos     (13U)
#define TSC_IOHCR_G4_IO2_Msk     (0x1UL << TSC_IOHCR_G4_IO2_Pos)               /*!< 0x00002000 */
#define TSC_IOHCR_G4_IO2         TSC_IOHCR_G4_IO2_Msk                          /*!<GROUP4_IO2 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G4_IO3_Pos     (14U)
#define TSC_IOHCR_G4_IO3_Msk     (0x1UL << TSC_IOHCR_G4_IO3_Pos)               /*!< 0x00004000 */
#define TSC_IOHCR_G4_IO3         TSC_IOHCR_G4_IO3_Msk                          /*!<GROUP4_IO3 schmitt trigger hysteresis mode */
#define TSC_IOHCR_G4_IO4_Pos     (15U)
#define TSC_IOHCR_G4_IO4_Msk     (0x1UL << TSC_IOHCR_G4_IO4_Pos)               /*!< 0x00008000 */
#define TSC_IOHCR_G4_IO4         TSC_IOHCR_G4_IO4_Msk                          /*!<GROUP4_IO4 schmitt trigger hysteresis mode */

/*******************  Bit definition for TSC_IOASCR register  *****************/
#define TSC_IOASCR_G1_IO1_Pos    (0U)
#define TSC_IOASCR_G1_IO1_Msk    (0x1UL << TSC_IOASCR_G1_IO1_Pos)              /*!< 0x00000001 */
#define TSC_IOASCR_G1_IO1        TSC_IOASCR_G1_IO1_Msk                         /*!<GROUP1_IO1 analog switch enable */
#define TSC_IOASCR_G1_IO2_Pos    (1U)
#define TSC_IOASCR_G1_IO2_Msk    (0x1UL << TSC_IOASCR_G1_IO2_Pos)              /*!< 0x00000002 */
#define TSC_IOASCR_G1_IO2        TSC_IOASCR_G1_IO2_Msk                         /*!<GROUP1_IO2 analog switch enable */
#define TSC_IOASCR_G1_IO3_Pos    (2U)
#define TSC_IOASCR_G1_IO3_Msk    (0x1UL << TSC_IOASCR_G1_IO3_Pos)              /*!< 0x00000004 */
#define TSC_IOASCR_G1_IO3        TSC_IOASCR_G1_IO3_Msk                         /*!<GROUP1_IO3 analog switch enable */
#define TSC_IOASCR_G1_IO4_Pos    (3U)
#define TSC_IOASCR_G1_IO4_Msk    (0x1UL << TSC_IOASCR_G1_IO4_Pos)              /*!< 0x00000008 */
#define TSC_IOASCR_G1_IO4        TSC_IOASCR_G1_IO4_Msk                         /*!<GROUP1_IO4 analog switch enable */
#define TSC_IOASCR_G2_IO1_Pos    (4U)
#define TSC_IOASCR_G2_IO1_Msk    (0x1UL << TSC_IOASCR_G2_IO1_Pos)              /*!< 0x00000010 */
#define TSC_IOASCR_G2_IO1        TSC_IOASCR_G2_IO1_Msk                         /*!<GROUP2_IO1 analog switch enable */
#define TSC_IOASCR_G2_IO2_Pos    (5U)
#define TSC_IOASCR_G2_IO2_Msk    (0x1UL << TSC_IOASCR_G2_IO2_Pos)              /*!< 0x00000020 */
#define TSC_IOASCR_G2_IO2        TSC_IOASCR_G2_IO2_Msk                         /*!<GROUP2_IO2 analog switch enable */
#define TSC_IOASCR_G2_IO3_Pos    (6U)
#define TSC_IOASCR_G2_IO3_Msk    (0x1UL << TSC_IOASCR_G2_IO3_Pos)              /*!< 0x00000040 */
#define TSC_IOASCR_G2_IO3        TSC_IOASCR_G2_IO3_Msk                         /*!<GROUP2_IO3 analog switch enable */
#define TSC_IOASCR_G2_IO4_Pos    (7U)
#define TSC_IOASCR_G2_IO4_Msk    (0x1UL << TSC_IOASCR_G2_IO4_Pos)              /*!< 0x00000080 */
#define TSC_IOASCR_G2_IO4        TSC_IOASCR_G2_IO4_Msk                         /*!<GROUP2_IO4 analog switch enable */
#define TSC_IOASCR_G3_IO1_Pos    (8U)
#define TSC_IOASCR_G3_IO1_Msk    (0x1UL << TSC_IOASCR_G3_IO1_Pos)              /*!< 0x00000100 */
#define TSC_IOASCR_G3_IO1        TSC_IOASCR_G3_IO1_Msk                         /*!<GROUP3_IO1 analog switch enable */
#define TSC_IOASCR_G3_IO2_Pos    (9U)
#define TSC_IOASCR_G3_IO2_Msk    (0x1UL << TSC_IOASCR_G3_IO2_Pos)              /*!< 0x00000200 */
#define TSC_IOASCR_G3_IO2        TSC_IOASCR_G3_IO2_Msk                         /*!<GROUP3_IO2 analog switch enable */
#define TSC_IOASCR_G3_IO3_Pos    (10U)
#define TSC_IOASCR_G3_IO3_Msk    (0x1UL << TSC_IOASCR_G3_IO3_Pos)              /*!< 0x00000400 */
#define TSC_IOASCR_G3_IO3        TSC_IOASCR_G3_IO3_Msk                         /*!<GROUP3_IO3 analog switch enable */
#define TSC_IOASCR_G3_IO4_Pos    (11U)
#define TSC_IOASCR_G3_IO4_Msk    (0x1UL << TSC_IOASCR_G3_IO4_Pos)              /*!< 0x00000800 */
#define TSC_IOASCR_G3_IO4        TSC_IOASCR_G3_IO4_Msk                         /*!<GROUP3_IO4 analog switch enable */
#define TSC_IOASCR_G4_IO1_Pos    (12U)
#define TSC_IOASCR_G4_IO1_Msk    (0x1UL << TSC_IOASCR_G4_IO1_Pos)              /*!< 0x00001000 */
#define TSC_IOASCR_G4_IO1        TSC_IOASCR_G4_IO1_Msk                         /*!<GROUP4_IO1 analog switch enable */
#define TSC_IOASCR_G4_IO2_Pos    (13U)
#define TSC_IOASCR_G4_IO2_Msk    (0x1UL << TSC_IOASCR_G4_IO2_Pos)              /*!< 0x00002000 */
#define TSC_IOASCR_G4_IO2        TSC_IOASCR_G4_IO2_Msk                         /*!<GROUP4_IO2 analog switch enable */
#define TSC_IOASCR_G4_IO3_Pos    (14U)
#define TSC_IOASCR_G4_IO3_Msk    (0x1UL << TSC_IOASCR_G4_IO3_Pos)              /*!< 0x00004000 */
#define TSC_IOASCR_G4_IO3        TSC_IOASCR_G4_IO3_Msk                         /*!<GROUP4_IO3 analog switch enable */
#define TSC_IOASCR_G4_IO4_Pos    (15U)
#define TSC_IOASCR_G4_IO4_Msk    (0x1UL << TSC_IOASCR_G4_IO4_Pos)              /*!< 0x00008000 */
#define TSC_IOASCR_G4_IO4        TSC_IOASCR_G4_IO4_Msk                         /*!<GROUP4_IO4 analog switch enable */

/*******************  Bit definition for TSC_IOSCR register  ******************/
#define TSC_IOSCR_G1_IO1_Pos     (0U)
#define TSC_IOSCR_G1_IO1_Msk     (0x1UL << TSC_IOSCR_G1_IO1_Pos)               /*!< 0x00000001 */
#define TSC_IOSCR_G1_IO1         TSC_IOSCR_G1_IO1_Msk                          /*!<GROUP1_IO1 sampling mode */
#define TSC_IOSCR_G1_IO2_Pos     (1U)
#define TSC_IOSCR_G1_IO2_Msk     (0x1UL << TSC_IOSCR_G1_IO2_Pos)               /*!< 0x00000002 */
#define TSC_IOSCR_G1_IO2         TSC_IOSCR_G1_IO2_Msk                          /*!<GROUP1_IO2 sampling mode */
#define TSC_IOSCR_G1_IO3_Pos     (2U)
#define TSC_IOSCR_G1_IO3_Msk     (0x1UL << TSC_IOSCR_G1_IO3_Pos)               /*!< 0x00000004 */
#define TSC_IOSCR_G1_IO3         TSC_IOSCR_G1_IO3_Msk                          /*!<GROUP1_IO3 sampling mode */
#define TSC_IOSCR_G1_IO4_Pos     (3U)
#define TSC_IOSCR_G1_IO4_Msk     (0x1UL << TSC_IOSCR_G1_IO4_Pos)               /*!< 0x00000008 */
#define TSC_IOSCR_G1_IO4         TSC_IOSCR_G1_IO4_Msk                          /*!<GROUP1_IO4 sampling mode */
#define TSC_IOSCR_G2_IO1_Pos     (4U)
#define TSC_IOSCR_G2_IO1_Msk     (0x1UL << TSC_IOSCR_G2_IO1_Pos)               /*!< 0x00000010 */
#define TSC_IOSCR_G2_IO1         TSC_IOSCR_G2_IO1_Msk                          /*!<GROUP2_IO1 sampling mode */
#define TSC_IOSCR_G2_IO2_Pos     (5U)
#define TSC_IOSCR_G2_IO2_Msk     (0x1UL << TSC_IOSCR_G2_IO2_Pos)               /*!< 0x00000020 */
#define TSC_IOSCR_G2_IO2         TSC_IOSCR_G2_IO2_Msk                          /*!<GROUP2_IO2 sampling mode */
#define TSC_IOSCR_G2_IO3_Pos     (6U)
#define TSC_IOSCR_G2_IO3_Msk     (0x1UL << TSC_IOSCR_G2_IO3_Pos)               /*!< 0x00000040 */
#define TSC_IOSCR_G2_IO3         TSC_IOSCR_G2_IO3_Msk                          /*!<GROUP2_IO3 sampling mode */
#define TSC_IOSCR_G2_IO4_Pos     (7U)
#define TSC_IOSCR_G2_IO4_Msk     (0x1UL << TSC_IOSCR_G2_IO4_Pos)               /*!< 0x00000080 */
#define TSC_IOSCR_G2_IO4         TSC_IOSCR_G2_IO4_Msk                          /*!<GROUP2_IO4 sampling mode */
#define TSC_IOSCR_G3_IO1_Pos     (8U)
#define TSC_IOSCR_G3_IO1_Msk     (0x1UL << TSC_IOSCR_G3_IO1_Pos)               /*!< 0x00000100 */
#define TSC_IOSCR_G3_IO1         TSC_IOSCR_G3_IO1_Msk                          /*!<GROUP3_IO1 sampling mode */
#define TSC_IOSCR_G3_IO2_Pos     (9U)
#define TSC_IOSCR_G3_IO2_Msk     (0x1UL << TSC_IOSCR_G3_IO2_Pos)               /*!< 0x00000200 */
#define TSC_IOSCR_G3_IO2         TSC_IOSCR_G3_IO2_Msk                          /*!<GROUP3_IO2 sampling mode */
#define TSC_IOSCR_G3_IO3_Pos     (10U)
#define TSC_IOSCR_G3_IO3_Msk     (0x1UL << TSC_IOSCR_G3_IO3_Pos)               /*!< 0x00000400 */
#define TSC_IOSCR_G3_IO3         TSC_IOSCR_G3_IO3_Msk                          /*!<GROUP3_IO3 sampling mode */
#define TSC_IOSCR_G3_IO4_Pos     (11U)
#define TSC_IOSCR_G3_IO4_Msk     (0x1UL << TSC_IOSCR_G3_IO4_Pos)               /*!< 0x00000800 */
#define TSC_IOSCR_G3_IO4         TSC_IOSCR_G3_IO4_Msk                          /*!<GROUP3_IO4 sampling mode */
#define TSC_IOSCR_G4_IO1_Pos     (12U)
#define TSC_IOSCR_G4_IO1_Msk     (0x1UL << TSC_IOSCR_G4_IO1_Pos)               /*!< 0x00001000 */
#define TSC_IOSCR_G4_IO1         TSC_IOSCR_G4_IO1_Msk                          /*!<GROUP4_IO1 sampling mode */
#define TSC_IOSCR_G4_IO2_Pos     (13U)
#define TSC_IOSCR_G4_IO2_Msk     (0x1UL << TSC_IOSCR_G4_IO2_Pos)               /*!< 0x00002000 */
#define TSC_IOSCR_G4_IO2         TSC_IOSCR_G4_IO2_Msk                          /*!<GROUP4_IO2 sampling mode */
#define TSC_IOSCR_G4_IO3_Pos     (14U)
#define TSC_IOSCR_G4_IO3_Msk     (0x1UL << TSC_IOSCR_G4_IO3_Pos)               /*!< 0x00004000 */
#define TSC_IOSCR_G4_IO3         TSC_IOSCR_G4_IO3_Msk                          /*!<GROUP4_IO3 sampling mode */
#define TSC_IOSCR_G4_IO4_Pos     (15U)
#define TSC_IOSCR_G4_IO4_Msk     (0x1UL << TSC_IOSCR_G4_IO4_Pos)               /*!< 0x00008000 */
#define TSC_IOSCR_G4_IO4         TSC_IOSCR_G4_IO4_Msk                          /*!<GROUP4_IO4 sampling mode */

/*******************  Bit definition for TSC_IOCCR register  ******************/
#define TSC_IOCCR_G1_IO1_Pos     (0U)
#define TSC_IOCCR_G1_IO1_Msk     (0x1UL << TSC_IOCCR_G1_IO1_Pos)               /*!< 0x00000001 */
#define TSC_IOCCR_G1_IO1         TSC_IOCCR_G1_IO1_Msk                          /*!<GROUP1_IO1 channel mode */
#define TSC_IOCCR_G1_IO2_Pos     (1U)
#define TSC_IOCCR_G1_IO2_Msk     (0x1UL << TSC_IOCCR_G1_IO2_Pos)               /*!< 0x00000002 */
#define TSC_IOCCR_G1_IO2         TSC_IOCCR_G1_IO2_Msk                          /*!<GROUP1_IO2 channel mode */
#define TSC_IOCCR_G1_IO3_Pos     (2U)
#define TSC_IOCCR_G1_IO3_Msk     (0x1UL << TSC_IOCCR_G1_IO3_Pos)               /*!< 0x00000004 */
#define TSC_IOCCR_G1_IO3         TSC_IOCCR_G1_IO3_Msk                          /*!<GROUP1_IO3 channel mode */
#define TSC_IOCCR_G1_IO4_Pos     (3U)
#define TSC_IOCCR_G1_IO4_Msk     (0x1UL << TSC_IOCCR_G1_IO4_Pos)               /*!< 0x00000008 */
#define TSC_IOCCR_G1_IO4         TSC_IOCCR_G1_IO4_Msk                          /*!<GROUP1_IO4 channel mode */
#define TSC_IOCCR_G2_IO1_Pos     (4U)
#define TSC_IOCCR_G2_IO1_Msk     (0x1UL << TSC_IOCCR_G2_IO1_Pos)               /*!< 0x00000010 */
#define TSC_IOCCR_G2_IO1         TSC_IOCCR_G2_IO1_Msk                          /*!<GROUP2_IO1 channel mode */
#define TSC_IOCCR_G2_IO2_Pos     (5U)
#define TSC_IOCCR_G2_IO2_Msk     (0x1UL << TSC_IOCCR_G2_IO2_Pos)               /*!< 0x00000020 */
#define TSC_IOCCR_G2_IO2         TSC_IOCCR_G2_IO2_Msk                          /*!<GROUP2_IO2 channel mode */
#define TSC_IOCCR_G2_IO3_Pos     (6U)
#define TSC_IOCCR_G2_IO3_Msk     (0x1UL << TSC_IOCCR_G2_IO3_Pos)               /*!< 0x00000040 */
#define TSC_IOCCR_G2_IO3         TSC_IOCCR_G2_IO3_Msk                          /*!<GROUP2_IO3 channel mode */
#define TSC_IOCCR_G2_IO4_Pos     (7U)
#define TSC_IOCCR_G2_IO4_Msk     (0x1UL << TSC_IOCCR_G2_IO4_Pos)               /*!< 0x00000080 */
#define TSC_IOCCR_G2_IO4         TSC_IOCCR_G2_IO4_Msk                          /*!<GROUP2_IO4 channel mode */
#define TSC_IOCCR_G3_IO1_Pos     (8U)
#define TSC_IOCCR_G3_IO1_Msk     (0x1UL << TSC_IOCCR_G3_IO1_Pos)               /*!< 0x00000100 */
#define TSC_IOCCR_G3_IO1         TSC_IOCCR_G3_IO1_Msk                          /*!<GROUP3_IO1 channel mode */
#define TSC_IOCCR_G3_IO2_Pos     (9U)
#define TSC_IOCCR_G3_IO2_Msk     (0x1UL << TSC_IOCCR_G3_IO2_Pos)               /*!< 0x00000200 */
#define TSC_IOCCR_G3_IO2         TSC_IOCCR_G3_IO2_Msk                          /*!<GROUP3_IO2 channel mode */
#define TSC_IOCCR_G3_IO3_Pos     (10U)
#define TSC_IOCCR_G3_IO3_Msk     (0x1UL << TSC_IOCCR_G3_IO3_Pos)               /*!< 0x00000400 */
#define TSC_IOCCR_G3_IO3         TSC_IOCCR_G3_IO3_Msk                          /*!<GROUP3_IO3 channel mode */
#define TSC_IOCCR_G3_IO4_Pos     (11U)
#define TSC_IOCCR_G3_IO4_Msk     (0x1UL << TSC_IOCCR_G3_IO4_Pos)               /*!< 0x00000800 */
#define TSC_IOCCR_G3_IO4         TSC_IOCCR_G3_IO4_Msk                          /*!<GROUP3_IO4 channel mode */
#define TSC_IOCCR_G4_IO1_Pos     (12U)
#define TSC_IOCCR_G4_IO1_Msk     (0x1UL << TSC_IOCCR_G4_IO1_Pos)               /*!< 0x00001000 */
#define TSC_IOCCR_G4_IO1         TSC_IOCCR_G4_IO1_Msk                          /*!<GROUP4_IO1 channel mode */
#define TSC_IOCCR_G4_IO2_Pos     (13U)
#define TSC_IOCCR_G4_IO2_Msk     (0x1UL << TSC_IOCCR_G4_IO2_Pos)               /*!< 0x00002000 */
#define TSC_IOCCR_G4_IO2         TSC_IOCCR_G4_IO2_Msk                          /*!<GROUP4_IO2 channel mode */
#define TSC_IOCCR_G4_IO3_Pos     (14U)
#define TSC_IOCCR_G4_IO3_Msk     (0x1UL << TSC_IOCCR_G4_IO3_Pos)               /*!< 0x00004000 */
#define TSC_IOCCR_G4_IO3         TSC_IOCCR_G4_IO3_Msk                          /*!<GROUP4_IO3 channel mode */
#define TSC_IOCCR_G4_IO4_Pos     (15U)
#define TSC_IOCCR_G4_IO4_Msk     (0x1UL << TSC_IOCCR_G4_IO4_Pos)               /*!< 0x00008000 */
#define TSC_IOCCR_G4_IO4         TSC_IOCCR_G4_IO4_Msk                          /*!<GROUP4_IO4 channel mode */

/*******************  Bit definition for TSC_IOGCSR register  *****************/
#define TSC_IOGCSR_G1E_Pos       (0U)
#define TSC_IOGCSR_G1E_Msk       (0x1UL << TSC_IOGCSR_G1E_Pos)                 /*!< 0x00000001 */
#define TSC_IOGCSR_G1E           TSC_IOGCSR_G1E_Msk                            /*!<Analog IO GROUP1 enable */
#define TSC_IOGCSR_G2E_Pos       (1U)
#define TSC_IOGCSR_G2E_Msk       (0x1UL << TSC_IOGCSR_G2E_Pos)                 /*!< 0x00000002 */
#define TSC_IOGCSR_G2E           TSC_IOGCSR_G2E_Msk                            /*!<Analog IO GROUP2 enable */
#define TSC_IOGCSR_G3E_Pos       (2U)
#define TSC_IOGCSR_G3E_Msk       (0x1UL << TSC_IOGCSR_G3E_Pos)                 /*!< 0x00000004 */
#define TSC_IOGCSR_G3E           TSC_IOGCSR_G3E_Msk                            /*!<Analog IO GROUP3 enable */
#define TSC_IOGCSR_G4E_Pos       (3U)
#define TSC_IOGCSR_G4E_Msk       (0x1UL << TSC_IOGCSR_G4E_Pos)                 /*!< 0x00000008 */
#define TSC_IOGCSR_G4E           TSC_IOGCSR_G4E_Msk                            /*!<Analog IO GROUP4 enable */
#define TSC_IOGCSR_G1S_Pos       (16U)
#define TSC_IOGCSR_G1S_Msk       (0x1UL << TSC_IOGCSR_G1S_Pos)                 /*!< 0x00010000 */
#define TSC_IOGCSR_G1S           TSC_IOGCSR_G1S_Msk                            /*!<Analog IO GROUP1 status */
#define TSC_IOGCSR_G2S_Pos       (17U)
#define TSC_IOGCSR_G2S_Msk       (0x1UL << TSC_IOGCSR_G2S_Pos)                 /*!< 0x00020000 */
#define TSC_IOGCSR_G2S           TSC_IOGCSR_G2S_Msk                            /*!<Analog IO GROUP2 status */
#define TSC_IOGCSR_G3S_Pos       (18U)
#define TSC_IOGCSR_G3S_Msk       (0x1UL << TSC_IOGCSR_G3S_Pos)                 /*!< 0x00040000 */
#define TSC_IOGCSR_G3S           TSC_IOGCSR_G3S_Msk                            /*!<Analog IO GROUP3 status */
#define TSC_IOGCSR_G4S_Pos       (19U)
#define TSC_IOGCSR_G4S_Msk       (0x1UL << TSC_IOGCSR_G4S_Pos)                 /*!< 0x00080000 */
#define TSC_IOGCSR_G4S           TSC_IOGCSR_G4S_Msk                            /*!<Analog IO GROUP4 status */

/*******************  Bit definition for TSC_IOGXCR register  *****************/
#define TSC_IOGXCR_CNT_Pos       (0U)
#define TSC_IOGXCR_CNT_Msk       (0x3FFFUL << TSC_IOGXCR_CNT_Pos)              /*!< 0x00003FFF */
#define TSC_IOGXCR_CNT           TSC_IOGXCR_CNT_Msk                            /*!<CNT[13:0] bits (Counter value) */

/******************************************************************************/
/*                                                                            */
/*      Universal Synchronous Asynchronous Receiver Transmitter (USART)       */
/*                                                                            */
/******************************************************************************/

/*
* @brief Specific device feature definitions (not present on all devices in the STM32L4 serie)
*/
#define USART_TCBGT_SUPPORT

/******************  Bit definition for USART_CR1 register  *******************/
#define USART_CR1_UE_Pos              (0U)
#define USART_CR1_UE_Msk              (0x1UL << USART_CR1_UE_Pos)              /*!< 0x00000001 */
#define USART_CR1_UE                  USART_CR1_UE_Msk                         /*!< USART Enable */
#define USART_CR1_UESM_Pos            (1U)
#define USART_CR1_UESM_Msk            (0x1UL << USART_CR1_UESM_Pos)            /*!< 0x00000002 */
#define USART_CR1_UESM                USART_CR1_UESM_Msk                       /*!< USART Enable in STOP Mode */
#define USART_CR1_RE_Pos              (2U)
#define USART_CR1_RE_Msk              (0x1UL << USART_CR1_RE_Pos)              /*!< 0x00000004 */
#define USART_CR1_RE                  USART_CR1_RE_Msk                         /*!< Receiver Enable */
#define USART_CR1_TE_Pos              (3U)
#define USART_CR1_TE_Msk              (0x1UL << USART_CR1_TE_Pos)              /*!< 0x00000008 */
#define USART_CR1_TE                  USART_CR1_TE_Msk                         /*!< Transmitter Enable */
#define USART_CR1_IDLEIE_Pos          (4U)
#define USART_CR1_IDLEIE_Msk          (0x1UL << USART_CR1_IDLEIE_Pos)          /*!< 0x00000010 */
#define USART_CR1_IDLEIE              USART_CR1_IDLEIE_Msk                     /*!< IDLE Interrupt Enable */
#define USART_CR1_RXNEIE_Pos          (5U)
#define USART_CR1_RXNEIE_Msk          (0x1UL << USART_CR1_RXNEIE_Pos)          /*!< 0x00000020 */
#define USART_CR1_RXNEIE              USART_CR1_RXNEIE_Msk                     /*!< RXNE Interrupt Enable */
#define USART_CR1_TCIE_Pos            (6U)
#define USART_CR1_TCIE_Msk            (0x1UL << USART_CR1_TCIE_Pos)            /*!< 0x00000040 */
#define USART_CR1_TCIE                USART_CR1_TCIE_Msk                       /*!< Transmission Complete Interrupt Enable */
#define USART_CR1_TXEIE_Pos           (7U)
#define USART_CR1_TXEIE_Msk           (0x1UL << USART_CR1_TXEIE_Pos)           /*!< 0x00000080 */
#define USART_CR1_TXEIE               USART_CR1_TXEIE_Msk                      /*!< TXE Interrupt Enable */
#define USART_CR1_PEIE_Pos            (8U)
#define USART_CR1_PEIE_Msk            (0x1UL << USART_CR1_PEIE_Pos)            /*!< 0x00000100 */
#define USART_CR1_PEIE                USART_CR1_PEIE_Msk                       /*!< PE Interrupt Enable */
#define USART_CR1_PS_Pos              (9U)
#define USART_CR1_PS_Msk              (0x1UL << USART_CR1_PS_Pos)              /*!< 0x00000200 */
#define USART_CR1_PS                  USART_CR1_PS_Msk                         /*!< Parity Selection */
#define USART_CR1_PCE_Pos             (10U)
#define USART_CR1_PCE_Msk             (0x1UL << USART_CR1_PCE_Pos)             /*!< 0x00000400 */
#define USART_CR1_PCE                 USART_CR1_PCE_Msk                        /*!< Parity Control Enable */
#define USART_CR1_WAKE_Pos            (11U)
#define USART_CR1_WAKE_Msk            (0x1UL << USART_CR1_WAKE_Pos)            /*!< 0x00000800 */
#define USART_CR1_WAKE                USART_CR1_WAKE_Msk                       /*!< Receiver Wakeup method */
#define USART_CR1_M_Pos               (12U)
#define USART_CR1_M_Msk               (0x10001UL << USART_CR1_M_Pos)           /*!< 0x10001000 */
#define USART_CR1_M                   USART_CR1_M_Msk                          /*!< Word length */
#define USART_CR1_M0_Pos              (12U)
#define USART_CR1_M0_Msk              (0x1UL << USART_CR1_M0_Pos)              /*!< 0x00001000 */
#define USART_CR1_M0                  USART_CR1_M0_Msk                         /*!< Word length - Bit 0 */
#define USART_CR1_MME_Pos             (13U)
#define USART_CR1_MME_Msk             (0x1UL << USART_CR1_MME_Pos)             /*!< 0x00002000 */
#define USART_CR1_MME                 USART_CR1_MME_Msk                        /*!< Mute Mode Enable */
#define USART_CR1_CMIE_Pos            (14U)
#define USART_CR1_CMIE_Msk            (0x1UL << USART_CR1_CMIE_Pos)            /*!< 0x00004000 */
#define USART_CR1_CMIE                USART_CR1_CMIE_Msk                       /*!< Character match interrupt enable */
#define USART_CR1_OVER8_Pos           (15U)
#define USART_CR1_OVER8_Msk           (0x1UL << USART_CR1_OVER8_Pos)           /*!< 0x00008000 */
#define USART_CR1_OVER8               USART_CR1_OVER8_Msk                      /*!< Oversampling by 8-bit or 16-bit mode */
#define USART_CR1_DEDT_Pos            (16U)
#define USART_CR1_DEDT_Msk            (0x1FUL << USART_CR1_DEDT_Pos)           /*!< 0x001F0000 */
#define USART_CR1_DEDT                USART_CR1_DEDT_Msk                       /*!< DEDT[4:0] bits (Driver Enable Deassertion Time) */
#define USART_CR1_DEDT_0              (0x01UL << USART_CR1_DEDT_Pos)           /*!< 0x00010000 */
#define USART_CR1_DEDT_1              (0x02UL << USART_CR1_DEDT_Pos)           /*!< 0x00020000 */
#define USART_CR1_DEDT_2              (0x04UL << USART_CR1_DEDT_Pos)           /*!< 0x00040000 */
#define USART_CR1_DEDT_3              (0x08UL << USART_CR1_DEDT_Pos)           /*!< 0x00080000 */
#define USART_CR1_DEDT_4              (0x10UL << USART_CR1_DEDT_Pos)           /*!< 0x00100000 */
#define USART_CR1_DEAT_Pos            (21U)
#define USART_CR1_DEAT_Msk            (0x1FUL << USART_CR1_DEAT_Pos)           /*!< 0x03E00000 */
#define USART_CR1_DEAT                USART_CR1_DEAT_Msk                       /*!< DEAT[4:0] bits (Driver Enable Assertion Time) */
#define USART_CR1_DEAT_0              (0x01UL << USART_CR1_DEAT_Pos)           /*!< 0x00200000 */
#define USART_CR1_DEAT_1              (0x02UL << USART_CR1_DEAT_Pos)           /*!< 0x00400000 */
#define USART_CR1_DEAT_2              (0x04UL << USART_CR1_DEAT_Pos)           /*!< 0x00800000 */
#define USART_CR1_DEAT_3              (0x08UL << USART_CR1_DEAT_Pos)           /*!< 0x01000000 */
#define USART_CR1_DEAT_4              (0x10UL << USART_CR1_DEAT_Pos)           /*!< 0x02000000 */
#define USART_CR1_RTOIE_Pos           (26U)
#define USART_CR1_RTOIE_Msk           (0x1UL << USART_CR1_RTOIE_Pos)           /*!< 0x04000000 */
#define USART_CR1_RTOIE               USART_CR1_RTOIE_Msk                      /*!< Receive Time Out interrupt enable */
#define USART_CR1_EOBIE_Pos           (27U)
#define USART_CR1_EOBIE_Msk           (0x1UL << USART_CR1_EOBIE_Pos)           /*!< 0x08000000 */
#define USART_CR1_EOBIE               USART_CR1_EOBIE_Msk                      /*!< End of Block interrupt enable */
#define USART_CR1_M1_Pos              (28U)
#define USART_CR1_M1_Msk              (0x1UL << USART_CR1_M1_Pos)              /*!< 0x10000000 */
#define USART_CR1_M1                  USART_CR1_M1_Msk                         /*!< Word length - Bit 1 */

/******************  Bit definition for USART_CR2 register  *******************/
#define USART_CR2_ADDM7_Pos           (4U)
#define USART_CR2_ADDM7_Msk           (0x1UL << USART_CR2_ADDM7_Pos)           /*!< 0x00000010 */
#define USART_CR2_ADDM7               USART_CR2_ADDM7_Msk                      /*!< 7-bit or 4-bit Address Detection */
#define USART_CR2_LBDL_Pos            (5U)
#define USART_CR2_LBDL_Msk            (0x1UL << USART_CR2_LBDL_Pos)            /*!< 0x00000020 */
#define USART_CR2_LBDL                USART_CR2_LBDL_Msk                       /*!< LIN Break Detection Length */
#define USART_CR2_LBDIE_Pos           (6U)
#define USART_CR2_LBDIE_Msk           (0x1UL << USART_CR2_LBDIE_Pos)           /*!< 0x00000040 */
#define USART_CR2_LBDIE               USART_CR2_LBDIE_Msk                      /*!< LIN Break Detection Interrupt Enable */
#define USART_CR2_LBCL_Pos            (8U)
#define USART_CR2_LBCL_Msk            (0x1UL << USART_CR2_LBCL_Pos)            /*!< 0x00000100 */
#define USART_CR2_LBCL                USART_CR2_LBCL_Msk                       /*!< Last Bit Clock pulse */
#define USART_CR2_CPHA_Pos            (9U)
#define USART_CR2_CPHA_Msk            (0x1UL << USART_CR2_CPHA_Pos)            /*!< 0x00000200 */
#define USART_CR2_CPHA                USART_CR2_CPHA_Msk                       /*!< Clock Phase */
#define USART_CR2_CPOL_Pos            (10U)
#define USART_CR2_CPOL_Msk            (0x1UL << USART_CR2_CPOL_Pos)            /*!< 0x00000400 */
#define USART_CR2_CPOL                USART_CR2_CPOL_Msk                       /*!< Clock Polarity */
#define USART_CR2_CLKEN_Pos           (11U)
#define USART_CR2_CLKEN_Msk           (0x1UL << USART_CR2_CLKEN_Pos)           /*!< 0x00000800 */
#define USART_CR2_CLKEN               USART_CR2_CLKEN_Msk                      /*!< Clock Enable */
#define USART_CR2_STOP_Pos            (12U)
#define USART_CR2_STOP_Msk            (0x3UL << USART_CR2_STOP_Pos)            /*!< 0x00003000 */
#define USART_CR2_STOP                USART_CR2_STOP_Msk                       /*!< STOP[1:0] bits (STOP bits) */
#define USART_CR2_STOP_0              (0x1UL << USART_CR2_STOP_Pos)            /*!< 0x00001000 */
#define USART_CR2_STOP_1              (0x2UL << USART_CR2_STOP_Pos)            /*!< 0x00002000 */
#define USART_CR2_LINEN_Pos           (14U)
#define USART_CR2_LINEN_Msk           (0x1UL << USART_CR2_LINEN_Pos)           /*!< 0x00004000 */
#define USART_CR2_LINEN               USART_CR2_LINEN_Msk                      /*!< LIN mode enable */
#define USART_CR2_SWAP_Pos            (15U)
#define USART_CR2_SWAP_Msk            (0x1UL << USART_CR2_SWAP_Pos)            /*!< 0x00008000 */
#define USART_CR2_SWAP                USART_CR2_SWAP_Msk                       /*!< SWAP TX/RX pins */
#define USART_CR2_RXINV_Pos           (16U)
#define USART_CR2_RXINV_Msk           (0x1UL << USART_CR2_RXINV_Pos)           /*!< 0x00010000 */
#define USART_CR2_RXINV               USART_CR2_RXINV_Msk                      /*!< RX pin active level inversion */
#define USART_CR2_TXINV_Pos           (17U)
#define USART_CR2_TXINV_Msk           (0x1UL << USART_CR2_TXINV_Pos)           /*!< 0x00020000 */
#define USART_CR2_TXINV               USART_CR2_TXINV_Msk                      /*!< TX pin active level inversion */
#define USART_CR2_DATAINV_Pos         (18U)
#define USART_CR2_DATAINV_Msk         (0x1UL << USART_CR2_DATAINV_Pos)         /*!< 0x00040000 */
#define USART_CR2_DATAINV             USART_CR2_DATAINV_Msk                    /*!< Binary data inversion */
#define USART_CR2_MSBFIRST_Pos        (19U)
#define USART_CR2_MSBFIRST_Msk        (0x1UL << USART_CR2_MSBFIRST_Pos)        /*!< 0x00080000 */
#define USART_CR2_MSBFIRST            USART_CR2_MSBFIRST_Msk                   /*!< Most Significant Bit First */
#define USART_CR2_ABREN_Pos           (20U)
#define USART_CR2_ABREN_Msk           (0x1UL << USART_CR2_ABREN_Pos)           /*!< 0x00100000 */
#define USART_CR2_ABREN               USART_CR2_ABREN_Msk                      /*!< Auto Baud-Rate Enable*/
#define USART_CR2_ABRMODE_Pos         (21U)
#define USART_CR2_ABRMODE_Msk         (0x3UL << USART_CR2_ABRMODE_Pos)         /*!< 0x00600000 */
#define USART_CR2_ABRMODE             USART_CR2_ABRMODE_Msk                    /*!< ABRMOD[1:0] bits (Auto Baud-Rate Mode) */
#define USART_CR2_ABRMODE_0           (0x1UL << USART_CR2_ABRMODE_Pos)         /*!< 0x00200000 */
#define USART_CR2_ABRMODE_1           (0x2UL << USART_CR2_ABRMODE_Pos)         /*!< 0x00400000 */
#define USART_CR2_RTOEN_Pos           (23U)
#define USART_CR2_RTOEN_Msk           (0x1UL << USART_CR2_RTOEN_Pos)           /*!< 0x00800000 */
#define USART_CR2_RTOEN               USART_CR2_RTOEN_Msk                      /*!< Receiver Time-Out enable */
#define USART_CR2_ADD_Pos             (24U)
#define USART_CR2_ADD_Msk             (0xFFUL << USART_CR2_ADD_Pos)            /*!< 0xFF000000 */
#define USART_CR2_ADD                 USART_CR2_ADD_Msk                        /*!< Address of the USART node */

/******************  Bit definition for USART_CR3 register  *******************/
#define USART_CR3_EIE_Pos             (0U)
#define USART_CR3_EIE_Msk             (0x1UL << USART_CR3_EIE_Pos)             /*!< 0x00000001 */
#define USART_CR3_EIE                 USART_CR3_EIE_Msk                        /*!< Error Interrupt Enable */
#define USART_CR3_IREN_Pos            (1U)
#define USART_CR3_IREN_Msk            (0x1UL << USART_CR3_IREN_Pos)            /*!< 0x00000002 */
#define USART_CR3_IREN                USART_CR3_IREN_Msk                       /*!< IrDA mode Enable */
#define USART_CR3_IRLP_Pos            (2U)
#define USART_CR3_IRLP_Msk            (0x1UL << USART_CR3_IRLP_Pos)            /*!< 0x00000004 */
#define USART_CR3_IRLP                USART_CR3_IRLP_Msk                       /*!< IrDA Low-Power */
#define USART_CR3_HDSEL_Pos           (3U)
#define USART_CR3_HDSEL_Msk           (0x1UL << USART_CR3_HDSEL_Pos)           /*!< 0x00000008 */
#define USART_CR3_HDSEL               USART_CR3_HDSEL_Msk                      /*!< Half-Duplex Selection */
#define USART_CR3_NACK_Pos            (4U)
#define USART_CR3_NACK_Msk            (0x1UL << USART_CR3_NACK_Pos)            /*!< 0x00000010 */
#define USART_CR3_NACK                USART_CR3_NACK_Msk                       /*!< SmartCard NACK enable */
#define USART_CR3_SCEN_Pos            (5U)
#define USART_CR3_SCEN_Msk            (0x1UL << USART_CR3_SCEN_Pos)            /*!< 0x00000020 */
#define USART_CR3_SCEN                USART_CR3_SCEN_Msk                       /*!< SmartCard mode enable */
#define USART_CR3_DMAR_Pos            (6U)
#define USART_CR3_DMAR_Msk            (0x1UL << USART_CR3_DMAR_Pos)            /*!< 0x00000040 */
#define USART_CR3_DMAR                USART_CR3_DMAR_Msk                       /*!< DMA Enable Receiver */
#define USART_CR3_DMAT_Pos            (7U)
#define USART_CR3_DMAT_Msk            (0x1UL << USART_CR3_DMAT_Pos)            /*!< 0x00000080 */
#define USART_CR3_DMAT                USART_CR3_DMAT_Msk                       /*!< DMA Enable Transmitter */
#define USART_CR3_RTSE_Pos            (8U)
#define USART_CR3_RTSE_Msk            (0x1UL << USART_CR3_RTSE_Pos)            /*!< 0x00000100 */
#define USART_CR3_RTSE                USART_CR3_RTSE_Msk                       /*!< RTS Enable */
#define USART_CR3_CTSE_Pos            (9U)
#define USART_CR3_CTSE_Msk            (0x1UL << USART_CR3_CTSE_Pos)            /*!< 0x00000200 */
#define USART_CR3_CTSE                USART_CR3_CTSE_Msk                       /*!< CTS Enable */
#define USART_CR3_CTSIE_Pos           (10U)
#define USART_CR3_CTSIE_Msk           (0x1UL << USART_CR3_CTSIE_Pos)           /*!< 0x00000400 */
#define USART_CR3_CTSIE               USART_CR3_CTSIE_Msk                      /*!< CTS Interrupt Enable */
#define USART_CR3_ONEBIT_Pos          (11U)
#define USART_CR3_ONEBIT_Msk          (0x1UL << USART_CR3_ONEBIT_Pos)          /*!< 0x00000800 */
#define USART_CR3_ONEBIT              USART_CR3_ONEBIT_Msk                     /*!< One sample bit method enable */
#define USART_CR3_OVRDIS_Pos          (12U)
#define USART_CR3_OVRDIS_Msk          (0x1UL << USART_CR3_OVRDIS_Pos)          /*!< 0x00001000 */
#define USART_CR3_OVRDIS              USART_CR3_OVRDIS_Msk                     /*!< Overrun Disable */
#define USART_CR3_DDRE_Pos            (13U)
#define USART_CR3_DDRE_Msk            (0x1UL << USART_CR3_DDRE_Pos)            /*!< 0x00002000 */
#define USART_CR3_DDRE                USART_CR3_DDRE_Msk                       /*!< DMA Disable on Reception Error */
#define USART_CR3_DEM_Pos             (14U)
#define USART_CR3_DEM_Msk             (0x1UL << USART_CR3_DEM_Pos)             /*!< 0x00004000 */
#define USART_CR3_DEM                 USART_CR3_DEM_Msk                        /*!< Driver Enable Mode */
#define USART_CR3_DEP_Pos             (15U)
#define USART_CR3_DEP_Msk             (0x1UL << USART_CR3_DEP_Pos)             /*!< 0x00008000 */
#define USART_CR3_DEP                 USART_CR3_DEP_Msk                        /*!< Driver Enable Polarity Selection */
#define USART_CR3_SCARCNT_Pos         (17U)
#define USART_CR3_SCARCNT_Msk         (0x7UL << USART_CR3_SCARCNT_Pos)         /*!< 0x000E0000 */
#define USART_CR3_SCARCNT             USART_CR3_SCARCNT_Msk                    /*!< SCARCNT[2:0] bits (SmartCard Auto-Retry Count) */
#define USART_CR3_SCARCNT_0           (0x1UL << USART_CR3_SCARCNT_Pos)         /*!< 0x00020000 */
#define USART_CR3_SCARCNT_1           (0x2UL << USART_CR3_SCARCNT_Pos)         /*!< 0x00040000 */
#define USART_CR3_SCARCNT_2           (0x4UL << USART_CR3_SCARCNT_Pos)         /*!< 0x00080000 */
#define USART_CR3_WUS_Pos             (20U)
#define USART_CR3_WUS_Msk             (0x3UL << USART_CR3_WUS_Pos)             /*!< 0x00300000 */
#define USART_CR3_WUS                 USART_CR3_WUS_Msk                        /*!< WUS[1:0] bits (Wake UP Interrupt Flag Selection) */
#define USART_CR3_WUS_0               (0x1UL << USART_CR3_WUS_Pos)             /*!< 0x00100000 */
#define USART_CR3_WUS_1               (0x2UL << USART_CR3_WUS_Pos)             /*!< 0x00200000 */
#define USART_CR3_WUFIE_Pos           (22U)
#define USART_CR3_WUFIE_Msk           (0x1UL << USART_CR3_WUFIE_Pos)           /*!< 0x00400000 */
#define USART_CR3_WUFIE               USART_CR3_WUFIE_Msk                      /*!< Wake Up Interrupt Enable */
#define USART_CR3_UCESM_Pos           (23U)
#define USART_CR3_UCESM_Msk           (0x1UL << USART_CR3_UCESM_Pos)           /*!< 0x02000000 */
#define USART_CR3_UCESM               USART_CR3_UCESM_Msk                      /*!< USART Clock enable in Stop mode */
#define USART_CR3_TCBGTIE_Pos         (24U)
#define USART_CR3_TCBGTIE_Msk         (0x1UL << USART_CR3_TCBGTIE_Pos)         /*!< 0x01000000 */
#define USART_CR3_TCBGTIE             USART_CR3_TCBGTIE_Msk                    /*!< Transmission Complete Before Guard Time Interrupt Enable */

/******************  Bit definition for USART_BRR register  *******************/
#define USART_BRR_DIV_FRACTION_Pos    (0U)
#define USART_BRR_DIV_FRACTION_Msk    (0xFUL << USART_BRR_DIV_FRACTION_Pos)    /*!< 0x0000000F */
#define USART_BRR_DIV_FRACTION        USART_BRR_DIV_FRACTION_Msk               /*!< Fraction of USARTDIV */
#define USART_BRR_DIV_MANTISSA_Pos    (4U)
#define USART_BRR_DIV_MANTISSA_Msk    (0xFFFUL << USART_BRR_DIV_MANTISSA_Pos)  /*!< 0x0000FFF0 */
#define USART_BRR_DIV_MANTISSA        USART_BRR_DIV_MANTISSA_Msk               /*!< Mantissa of USARTDIV */

/******************  Bit definition for USART_GTPR register  ******************/
#define USART_GTPR_PSC_Pos            (0U)
#define USART_GTPR_PSC_Msk            (0xFFUL << USART_GTPR_PSC_Pos)           /*!< 0x000000FF */
#define USART_GTPR_PSC                USART_GTPR_PSC_Msk                       /*!< PSC[7:0] bits (Prescaler value) */
#define USART_GTPR_GT_Pos             (8U)
#define USART_GTPR_GT_Msk             (0xFFUL << USART_GTPR_GT_Pos)            /*!< 0x0000FF00 */
#define USART_GTPR_GT                 USART_GTPR_GT_Msk                        /*!< GT[7:0] bits (Guard time value) */

/*******************  Bit definition for USART_RTOR register  *****************/
#define USART_RTOR_RTO_Pos            (0U)
#define USART_RTOR_RTO_Msk            (0xFFFFFFUL << USART_RTOR_RTO_Pos)       /*!< 0x00FFFFFF */
#define USART_RTOR_RTO                USART_RTOR_RTO_Msk                       /*!< Receiver Time Out Value */
#define USART_RTOR_BLEN_Pos           (24U)
#define USART_RTOR_BLEN_Msk           (0xFFUL << USART_RTOR_BLEN_Pos)          /*!< 0xFF000000 */
#define USART_RTOR_BLEN               USART_RTOR_BLEN_Msk                      /*!< Block Length */

/*******************  Bit definition for USART_RQR register  ******************/
#define USART_RQR_ABRRQ_Pos           (0U)
#define USART_RQR_ABRRQ_Msk           (0x1UL << USART_RQR_ABRRQ_Pos)           /*!< 0x00000001 */
#define USART_RQR_ABRRQ               USART_RQR_ABRRQ_Msk                      /*!< Auto-Baud Rate Request */
#define USART_RQR_SBKRQ_Pos           (1U)
#define USART_RQR_SBKRQ_Msk           (0x1UL << USART_RQR_SBKRQ_Pos)           /*!< 0x00000002 */
#define USART_RQR_SBKRQ               USART_RQR_SBKRQ_Msk                      /*!< Send Break Request */
#define USART_RQR_MMRQ_Pos            (2U)
#define USART_RQR_MMRQ_Msk            (0x1UL << USART_RQR_MMRQ_Pos)            /*!< 0x00000004 */
#define USART_RQR_MMRQ                USART_RQR_MMRQ_Msk                       /*!< Mute Mode Request */
#define USART_RQR_RXFRQ_Pos           (3U)
#define USART_RQR_RXFRQ_Msk           (0x1UL << USART_RQR_RXFRQ_Pos)           /*!< 0x00000008 */
#define USART_RQR_RXFRQ               USART_RQR_RXFRQ_Msk                      /*!< Receive Data flush Request */
#define USART_RQR_TXFRQ_Pos           (4U)
#define USART_RQR_TXFRQ_Msk           (0x1UL << USART_RQR_TXFRQ_Pos)           /*!< 0x00000010 */
#define USART_RQR_TXFRQ               USART_RQR_TXFRQ_Msk                      /*!< Transmit data flush Request */

/*******************  Bit definition for USART_ISR register  ******************/
#define USART_ISR_PE_Pos              (0U)
#define USART_ISR_PE_Msk              (0x1UL << USART_ISR_PE_Pos)              /*!< 0x00000001 */
#define USART_ISR_PE                  USART_ISR_PE_Msk                         /*!< Parity Error */
#define USART_ISR_FE_Pos              (1U)
#define USART_ISR_FE_Msk              (0x1UL << USART_ISR_FE_Pos)              /*!< 0x00000002 */
#define USART_ISR_FE                  USART_ISR_FE_Msk                         /*!< Framing Error */
#define USART_ISR_NE_Pos              (2U)
#define USART_ISR_NE_Msk              (0x1UL << USART_ISR_NE_Pos)              /*!< 0x00000004 */
#define USART_ISR_NE                  USART_ISR_NE_Msk                         /*!< Noise Error detected Flag */
#define USART_ISR_ORE_Pos             (3U)
#define USART_ISR_ORE_Msk             (0x1UL << USART_ISR_ORE_Pos)             /*!< 0x00000008 */
#define USART_ISR_ORE                 USART_ISR_ORE_Msk                        /*!< OverRun Error */
#define USART_ISR_IDLE_Pos            (4U)
#define USART_ISR_IDLE_Msk            (0x1UL << USART_ISR_IDLE_Pos)            /*!< 0x00000010 */
#define USART_ISR_IDLE                USART_ISR_IDLE_Msk                       /*!< IDLE line detected */
#define USART_ISR_RXNE_Pos            (5U)
#define USART_ISR_RXNE_Msk            (0x1UL << USART_ISR_RXNE_Pos)            /*!< 0x00000020 */
#define USART_ISR_RXNE                USART_ISR_RXNE_Msk                       /*!< Read Data Register Not Empty */
#define USART_ISR_TC_Pos              (6U)
#define USART_ISR_TC_Msk              (0x1UL << USART_ISR_TC_Pos)              /*!< 0x00000040 */
#define USART_ISR_TC                  USART_ISR_TC_Msk                         /*!< Transmission Complete */
#define USART_ISR_TXE_Pos             (7U)
#define USART_ISR_TXE_Msk             (0x1UL << USART_ISR_TXE_Pos)             /*!< 0x00000080 */
#define USART_ISR_TXE                 USART_ISR_TXE_Msk                        /*!< Transmit Data Register Empty */
#define USART_ISR_LBDF_Pos            (8U)
#define USART_ISR_LBDF_Msk            (0x1UL << USART_ISR_LBDF_Pos)            /*!< 0x00000100 */
#define USART_ISR_LBDF                USART_ISR_LBDF_Msk                       /*!< LIN Break Detection Flag */
#define USART_ISR_CTSIF_Pos           (9U)
#define USART_ISR_CTSIF_Msk           (0x1UL << USART_ISR_CTSIF_Pos)           /*!< 0x00000200 */
#define USART_ISR_CTSIF               USART_ISR_CTSIF_Msk                      /*!< CTS interrupt flag */
#define USART_ISR_CTS_Pos             (10U)
#define USART_ISR_CTS_Msk             (0x1UL << USART_ISR_CTS_Pos)             /*!< 0x00000400 */
#define USART_ISR_CTS                 USART_ISR_CTS_Msk                        /*!< CTS flag */
#define USART_ISR_RTOF_Pos            (11U)
#define USART_ISR_RTOF_Msk            (0x1UL << USART_ISR_RTOF_Pos)            /*!< 0x00000800 */
#define USART_ISR_RTOF                USART_ISR_RTOF_Msk                       /*!< Receiver Time Out */
#define USART_ISR_EOBF_Pos            (12U)
#define USART_ISR_EOBF_Msk            (0x1UL << USART_ISR_EOBF_Pos)            /*!< 0x00001000 */
#define USART_ISR_EOBF                USART_ISR_EOBF_Msk                       /*!< End Of Block Flag */
#define USART_ISR_ABRE_Pos            (14U)
#define USART_ISR_ABRE_Msk            (0x1UL << USART_ISR_ABRE_Pos)            /*!< 0x00004000 */
#define USART_ISR_ABRE                USART_ISR_ABRE_Msk                       /*!< Auto-Baud Rate Error */
#define USART_ISR_ABRF_Pos            (15U)
#define USART_ISR_ABRF_Msk            (0x1UL << USART_ISR_ABRF_Pos)            /*!< 0x00008000 */
#define USART_ISR_ABRF                USART_ISR_ABRF_Msk                       /*!< Auto-Baud Rate Flag */
#define USART_ISR_BUSY_Pos            (16U)
#define USART_ISR_BUSY_Msk            (0x1UL << USART_ISR_BUSY_Pos)            /*!< 0x00010000 */
#define USART_ISR_BUSY                USART_ISR_BUSY_Msk                       /*!< Busy Flag */
#define USART_ISR_CMF_Pos             (17U)
#define USART_ISR_CMF_Msk             (0x1UL << USART_ISR_CMF_Pos)             /*!< 0x00020000 */
#define USART_ISR_CMF                 USART_ISR_CMF_Msk                        /*!< Character Match Flag */
#define USART_ISR_SBKF_Pos            (18U)
#define USART_ISR_SBKF_Msk            (0x1UL << USART_ISR_SBKF_Pos)            /*!< 0x00040000 */
#define USART_ISR_SBKF                USART_ISR_SBKF_Msk                       /*!< Send Break Flag */
#define USART_ISR_RWU_Pos             (19U)
#define USART_ISR_RWU_Msk             (0x1UL << USART_ISR_RWU_Pos)             /*!< 0x00080000 */
#define USART_ISR_RWU                 USART_ISR_RWU_Msk                        /*!< Receive Wake Up from mute mode Flag */
#define USART_ISR_WUF_Pos             (20U)
#define USART_ISR_WUF_Msk             (0x1UL << USART_ISR_WUF_Pos)             /*!< 0x00100000 */
#define USART_ISR_WUF                 USART_ISR_WUF_Msk                        /*!< Wake Up from stop mode Flag */
#define USART_ISR_TEACK_Pos           (21U)
#define USART_ISR_TEACK_Msk           (0x1UL << USART_ISR_TEACK_Pos)           /*!< 0x00200000 */
#define USART_ISR_TEACK               USART_ISR_TEACK_Msk                      /*!< Transmit Enable Acknowledge Flag */
#define USART_ISR_REACK_Pos           (22U)
#define USART_ISR_REACK_Msk           (0x1UL << USART_ISR_REACK_Pos)           /*!< 0x00400000 */
#define USART_ISR_REACK               USART_ISR_REACK_Msk                      /*!< Receive Enable Acknowledge Flag */
#define USART_ISR_TCBGT_Pos           (25U)
#define USART_ISR_TCBGT_Msk           (0x1UL << USART_ISR_TCBGT_Pos)           /*!< 0x02000000 */
#define USART_ISR_TCBGT               USART_ISR_TCBGT_Msk                      /*!< Transmission Complete Before Guard Time Completion Flag */

/*******************  Bit definition for USART_ICR register  ******************/
#define USART_ICR_PECF_Pos            (0U)
#define USART_ICR_PECF_Msk            (0x1UL << USART_ICR_PECF_Pos)            /*!< 0x00000001 */
#define USART_ICR_PECF                USART_ICR_PECF_Msk                       /*!< Parity Error Clear Flag */
#define USART_ICR_FECF_Pos            (1U)
#define USART_ICR_FECF_Msk            (0x1UL << USART_ICR_FECF_Pos)            /*!< 0x00000002 */
#define USART_ICR_FECF                USART_ICR_FECF_Msk                       /*!< Framing Error Clear Flag */
#define USART_ICR_NECF_Pos            (2U)
#define USART_ICR_NECF_Msk            (0x1UL << USART_ICR_NECF_Pos)            /*!< 0x00000004 */
#define USART_ICR_NECF                USART_ICR_NECF_Msk                       /*!< Noise Error detected Clear Flag */
#define USART_ICR_ORECF_Pos           (3U)
#define USART_ICR_ORECF_Msk           (0x1UL << USART_ICR_ORECF_Pos)           /*!< 0x00000008 */
#define USART_ICR_ORECF               USART_ICR_ORECF_Msk                      /*!< OverRun Error Clear Flag */
#define USART_ICR_IDLECF_Pos          (4U)
#define USART_ICR_IDLECF_Msk          (0x1UL << USART_ICR_IDLECF_Pos)          /*!< 0x00000010 */
#define USART_ICR_IDLECF              USART_ICR_IDLECF_Msk                     /*!< IDLE line detected Clear Flag */
#define USART_ICR_TCCF_Pos            (6U)
#define USART_ICR_TCCF_Msk            (0x1UL << USART_ICR_TCCF_Pos)            /*!< 0x00000040 */
#define USART_ICR_TCCF                USART_ICR_TCCF_Msk                       /*!< Transmission Complete Clear Flag */
#define USART_ICR_TCBGTCF_Pos         (7U)
#define USART_ICR_TCBGTCF_Msk         (0x1UL << USART_ICR_TCBGTCF_Pos)         /*!< 0x00000080 */
#define USART_ICR_TCBGTCF             USART_ICR_TCBGTCF_Msk                    /*!< Transmission Complete Before Guard Time Clear Flag */
#define USART_ICR_LBDCF_Pos           (8U)
#define USART_ICR_LBDCF_Msk           (0x1UL << USART_ICR_LBDCF_Pos)           /*!< 0x00000100 */
#define USART_ICR_LBDCF               USART_ICR_LBDCF_Msk                      /*!< LIN Break Detection Clear Flag */
#define USART_ICR_CTSCF_Pos           (9U)
#define USART_ICR_CTSCF_Msk           (0x1UL << USART_ICR_CTSCF_Pos)           /*!< 0x00000200 */
#define USART_ICR_CTSCF               USART_ICR_CTSCF_Msk                      /*!< CTS Interrupt Clear Flag */
#define USART_ICR_RTOCF_Pos           (11U)
#define USART_ICR_RTOCF_Msk           (0x1UL << USART_ICR_RTOCF_Pos)           /*!< 0x00000800 */
#define USART_ICR_RTOCF               USART_ICR_RTOCF_Msk                      /*!< Receiver Time Out Clear Flag */
#define USART_ICR_EOBCF_Pos           (12U)
#define USART_ICR_EOBCF_Msk           (0x1UL << USART_ICR_EOBCF_Pos)           /*!< 0x00001000 */
#define USART_ICR_EOBCF               USART_ICR_EOBCF_Msk                      /*!< End Of Block Clear Flag */
#define USART_ICR_CMCF_Pos            (17U)
#define USART_ICR_CMCF_Msk            (0x1UL << USART_ICR_CMCF_Pos)            /*!< 0x00020000 */
#define USART_ICR_CMCF                USART_ICR_CMCF_Msk                       /*!< Character Match Clear Flag */
#define USART_ICR_WUCF_Pos            (20U)
#define USART_ICR_WUCF_Msk            (0x1UL << USART_ICR_WUCF_Pos)            /*!< 0x00100000 */
#define USART_ICR_WUCF                USART_ICR_WUCF_Msk                       /*!< Wake Up from stop mode Clear Flag */

/* Legacy defines */
#define USART_ICR_NCF_Pos             USART_ICR_NECF_Pos
#define USART_ICR_NCF_Msk             USART_ICR_NECF_Msk
#define USART_ICR_NCF                 USART_ICR_NECF

/*******************  Bit definition for USART_RDR register  ******************/
#define USART_RDR_RDR_Pos             (0U)
#define USART_RDR_RDR_Msk             (0x1FFUL << USART_RDR_RDR_Pos)           /*!< 0x000001FF */
#define USART_RDR_RDR                 USART_RDR_RDR_Msk                        /*!< RDR[8:0] bits (Receive Data value) */

/*******************  Bit definition for USART_TDR register  ******************/
#define USART_TDR_TDR_Pos             (0U)
#define USART_TDR_TDR_Msk             (0x1FFUL << USART_TDR_TDR_Pos)           /*!< 0x000001FF */
#define USART_TDR_TDR                 USART_TDR_TDR_Msk                        /*!< TDR[8:0] bits (Transmit Data value) */


/******************************************************************************/
/*                                                                            */
/*                            Window WATCHDOG                                 */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for WWDG_CR register  ********************/
#define WWDG_CR_T_Pos           (0U)
#define WWDG_CR_T_Msk           (0x7FUL << WWDG_CR_T_Pos)                      /*!< 0x0000007F */
#define WWDG_CR_T               WWDG_CR_T_Msk                                  /*!<T[6:0] bits (7-Bit counter (MSB to LSB)) */
#define WWDG_CR_T_0             (0x01UL << WWDG_CR_T_Pos)                      /*!< 0x00000001 */
#define WWDG_CR_T_1             (0x02UL << WWDG_CR_T_Pos)                      /*!< 0x00000002 */
#define WWDG_CR_T_2             (0x04UL << WWDG_CR_T_Pos)                      /*!< 0x00000004 */
#define WWDG_CR_T_3             (0x08UL << WWDG_CR_T_Pos)                      /*!< 0x00000008 */
#define WWDG_CR_T_4             (0x10UL << WWDG_CR_T_Pos)                      /*!< 0x00000010 */
#define WWDG_CR_T_5             (0x20UL << WWDG_CR_T_Pos)                      /*!< 0x00000020 */
#define WWDG_CR_T_6             (0x40UL << WWDG_CR_T_Pos)                      /*!< 0x00000040 */

#define WWDG_CR_WDGA_Pos        (7U)
#define WWDG_CR_WDGA_Msk        (0x1UL << WWDG_CR_WDGA_Pos)                    /*!< 0x00000080 */
#define WWDG_CR_WDGA            WWDG_CR_WDGA_Msk                               /*!<Activation bit */

/*******************  Bit definition for WWDG_CFR register  *******************/
#define WWDG_CFR_W_Pos          (0U)
#define WWDG_CFR_W_Msk          (0x7FUL << WWDG_CFR_W_Pos)                     /*!< 0x0000007F */
#define WWDG_CFR_W              WWDG_CFR_W_Msk                                 /*!<W[6:0] bits (7-bit window value) */
#define WWDG_CFR_W_0            (0x01UL << WWDG_CFR_W_Pos)                     /*!< 0x00000001 */
#define WWDG_CFR_W_1            (0x02UL << WWDG_CFR_W_Pos)                     /*!< 0x00000002 */
#define WWDG_CFR_W_2            (0x04UL << WWDG_CFR_W_Pos)                     /*!< 0x00000004 */
#define WWDG_CFR_W_3            (0x08UL << WWDG_CFR_W_Pos)                     /*!< 0x00000008 */
#define WWDG_CFR_W_4            (0x10UL << WWDG_CFR_W_Pos)                     /*!< 0x00000010 */
#define WWDG_CFR_W_5            (0x20UL << WWDG_CFR_W_Pos)                     /*!< 0x00000020 */
#define WWDG_CFR_W_6            (0x40UL << WWDG_CFR_W_Pos)                     /*!< 0x00000040 */

#define WWDG_CFR_WDGTB_Pos      (7U)
#define WWDG_CFR_WDGTB_Msk      (0x3UL << WWDG_CFR_WDGTB_Pos)                  /*!< 0x00000180 */
#define WWDG_CFR_WDGTB          WWDG_CFR_WDGTB_Msk                             /*!<WDGTB[1:0] bits (Timer Base) */
#define WWDG_CFR_WDGTB_0        (0x1UL << WWDG_CFR_WDGTB_Pos)                  /*!< 0x00000080 */
#define WWDG_CFR_WDGTB_1        (0x2UL << WWDG_CFR_WDGTB_Pos)                  /*!< 0x00000100 */

#define WWDG_CFR_EWI_Pos        (9U)
#define WWDG_CFR_EWI_Msk        (0x1UL << WWDG_CFR_EWI_Pos)                    /*!< 0x00000200 */
#define WWDG_CFR_EWI            WWDG_CFR_EWI_Msk                               /*!<Early Wakeup Interrupt */

/*******************  Bit definition for WWDG_SR register  ********************/
#define WWDG_SR_EWIF_Pos        (0U)
#define WWDG_SR_EWIF_Msk        (0x1UL << WWDG_SR_EWIF_Pos)                    /*!< 0x00000001 */
#define WWDG_SR_EWIF            WWDG_SR_EWIF_Msk                               /*!<Early Wakeup Interrupt Flag */


/******************************************************************************/
/*                                                                            */
/*                                 Debug MCU                                  */
/*                                                                            */
/******************************************************************************/
/********************  Bit definition for DBGMCU_IDCODE register  *************/
#define DBGMCU_IDCODE_DEV_ID_Pos               (0U)
#define DBGMCU_IDCODE_DEV_ID_Msk               (0xFFFUL << DBGMCU_IDCODE_DEV_ID_Pos) /*!< 0x00000FFF */
#define DBGMCU_IDCODE_DEV_ID                   DBGMCU_IDCODE_DEV_ID_Msk
#define DBGMCU_IDCODE_REV_ID_Pos               (16U)
#define DBGMCU_IDCODE_REV_ID_Msk               (0xFFFFUL << DBGMCU_IDCODE_REV_ID_Pos) /*!< 0xFFFF0000 */
#define DBGMCU_IDCODE_REV_ID                   DBGMCU_IDCODE_REV_ID_Msk

/********************  Bit definition for DBGMCU_CR register  *****************/
#define DBGMCU_CR_DBG_SLEEP_Pos                (0U)
#define DBGMCU_CR_DBG_SLEEP_Msk                (0x1UL << DBGMCU_CR_DBG_SLEEP_Pos) /*!< 0x00000001 */
#define DBGMCU_CR_DBG_SLEEP                    DBGMCU_CR_DBG_SLEEP_Msk
#define DBGMCU_CR_DBG_STOP_Pos                 (1U)
#define DBGMCU_CR_DBG_STOP_Msk                 (0x1UL << DBGMCU_CR_DBG_STOP_Pos) /*!< 0x00000002 */
#define DBGMCU_CR_DBG_STOP                     DBGMCU_CR_DBG_STOP_Msk
#define DBGMCU_CR_DBG_STANDBY_Pos              (2U)
#define DBGMCU_CR_DBG_STANDBY_Msk              (0x1UL << DBGMCU_CR_DBG_STANDBY_Pos) /*!< 0x00000004 */
#define DBGMCU_CR_DBG_STANDBY                  DBGMCU_CR_DBG_STANDBY_Msk
#define DBGMCU_CR_TRACE_IOEN_Pos               (5U)
#define DBGMCU_CR_TRACE_IOEN_Msk               (0x1UL << DBGMCU_CR_TRACE_IOEN_Pos) /*!< 0x00000020 */
#define DBGMCU_CR_TRACE_IOEN                   DBGMCU_CR_TRACE_IOEN_Msk

#define DBGMCU_CR_TRACE_MODE_Pos               (6U)
#define DBGMCU_CR_TRACE_MODE_Msk               (0x3UL << DBGMCU_CR_TRACE_MODE_Pos) /*!< 0x000000C0 */
#define DBGMCU_CR_TRACE_MODE                   DBGMCU_CR_TRACE_MODE_Msk
#define DBGMCU_CR_TRACE_MODE_0                 (0x1UL << DBGMCU_CR_TRACE_MODE_Pos) /*!< 0x00000040 */
#define DBGMCU_CR_TRACE_MODE_1                 (0x2UL << DBGMCU_CR_TRACE_MODE_Pos) /*!< 0x00000080 */

/********************  Bit definition for DBGMCU_APB1FZR1 register  ***********/
#define DBGMCU_APB1FZR1_DBG_TIM2_STOP_Pos      (0U)
#define DBGMCU_APB1FZR1_DBG_TIM2_STOP_Msk      (0x1UL << DBGMCU_APB1FZR1_DBG_TIM2_STOP_Pos) /*!< 0x00000001 */
#define DBGMCU_APB1FZR1_DBG_TIM2_STOP          DBGMCU_APB1FZR1_DBG_TIM2_STOP_Msk
#define DBGMCU_APB1FZR1_DBG_TIM6_STOP_Pos      (4U)
#define DBGMCU_APB1FZR1_DBG_TIM6_STOP_Msk      (0x1UL << DBGMCU_APB1FZR1_DBG_TIM6_STOP_Pos) /*!< 0x00000010 */
#define DBGMCU_APB1FZR1_DBG_TIM6_STOP          DBGMCU_APB1FZR1_DBG_TIM6_STOP_Msk
#define DBGMCU_APB1FZR1_DBG_RTC_STOP_Pos       (10U)
#define DBGMCU_APB1FZR1_DBG_RTC_STOP_Msk       (0x1UL << DBGMCU_APB1FZR1_DBG_RTC_STOP_Pos) /*!< 0x00000400 */
#define DBGMCU_APB1FZR1_DBG_RTC_STOP           DBGMCU_APB1FZR1_DBG_RTC_STOP_Msk
#define DBGMCU_APB1FZR1_DBG_WWDG_STOP_Pos      (11U)
#define DBGMCU_APB1FZR1_DBG_WWDG_STOP_Msk      (0x1UL << DBGMCU_APB1FZR1_DBG_WWDG_STOP_Pos) /*!< 0x00000800 */
#define DBGMCU_APB1FZR1_DBG_WWDG_STOP          DBGMCU_APB1FZR1_DBG_WWDG_STOP_Msk
#define DBGMCU_APB1FZR1_DBG_IWDG_STOP_Pos      (12U)
#define DBGMCU_APB1FZR1_DBG_IWDG_STOP_Msk      (0x1UL << DBGMCU_APB1FZR1_DBG_IWDG_STOP_Pos) /*!< 0x00001000 */
#define DBGMCU_APB1FZR1_DBG_IWDG_STOP          DBGMCU_APB1FZR1_DBG_IWDG_STOP_Msk
#define DBGMCU_APB1FZR1_DBG_I2C1_STOP_Pos      (21U)
#define DBGMCU_APB1FZR1_DBG_I2C1_STOP_Msk      (0x1UL << DBGMCU_APB1FZR1_DBG_I2C1_STOP_Pos) /*!< 0x00200000 */
#define DBGMCU_APB1FZR1_DBG_I2C1_STOP          DBGMCU_APB1FZR1_DBG_I2C1_STOP_Msk
#define DBGMCU_APB1FZR1_DBG_I2C2_STOP_Pos      (22U)
#define DBGMCU_APB1FZR1_DBG_I2C2_STOP_Msk      (0x1UL << DBGMCU_APB1FZR1_DBG_I2C2_STOP_Pos) /*!< 0x00400000 */
#define DBGMCU_APB1FZR1_DBG_I2C2_STOP          DBGMCU_APB1FZR1_DBG_I2C2_STOP_Msk
#define DBGMCU_APB1FZR1_DBG_I2C3_STOP_Pos      (23U)
#define DBGMCU_APB1FZR1_DBG_I2C3_STOP_Msk      (0x1UL << DBGMCU_APB1FZR1_DBG_I2C3_STOP_Pos) /*!< 0x00800000 */
#define DBGMCU_APB1FZR1_DBG_I2C3_STOP          DBGMCU_APB1FZR1_DBG_I2C3_STOP_Msk
#define DBGMCU_APB1FZR1_DBG_LPTIM1_STOP_Pos    (31U)
#define DBGMCU_APB1FZR1_DBG_LPTIM1_STOP_Msk    (0x1UL << DBGMCU_APB1FZR1_DBG_LPTIM1_STOP_Pos) /*!< 0x80000000 */
#define DBGMCU_APB1FZR1_DBG_LPTIM1_STOP        DBGMCU_APB1FZR1_DBG_LPTIM1_STOP_Msk

/********************  Bit definition for DBGMCU_APB1FZR2 register  **********/
#define DBGMCU_APB1FZR2_DBG_LPTIM2_STOP_Pos    (5U)
#define DBGMCU_APB1FZR2_DBG_LPTIM2_STOP_Msk    (0x1UL << DBGMCU_APB1FZR2_DBG_LPTIM2_STOP_Pos) /*!< 0x00000020 */
#define DBGMCU_APB1FZR2_DBG_LPTIM2_STOP        DBGMCU_APB1FZR2_DBG_LPTIM2_STOP_Msk

/********************  Bit definition for DBGMCU_APB2FZ register  ************/
#define DBGMCU_APB2FZ_DBG_TIM1_STOP_Pos        (11U)
#define DBGMCU_APB2FZ_DBG_TIM1_STOP_Msk        (0x1UL << DBGMCU_APB2FZ_DBG_TIM1_STOP_Pos) /*!< 0x00000800 */
#define DBGMCU_APB2FZ_DBG_TIM1_STOP            DBGMCU_APB2FZ_DBG_TIM1_STOP_Msk
#define DBGMCU_APB2FZ_DBG_TIM15_STOP_Pos       (16U)
#define DBGMCU_APB2FZ_DBG_TIM15_STOP_Msk       (0x1UL << DBGMCU_APB2FZ_DBG_TIM15_STOP_Pos) /*!< 0x00010000 */
#define DBGMCU_APB2FZ_DBG_TIM15_STOP           DBGMCU_APB2FZ_DBG_TIM15_STOP_Msk
#define DBGMCU_APB2FZ_DBG_TIM16_STOP_Pos       (17U)
#define DBGMCU_APB2FZ_DBG_TIM16_STOP_Msk       (0x1UL << DBGMCU_APB2FZ_DBG_TIM16_STOP_Pos) /*!< 0x00020000 */
#define DBGMCU_APB2FZ_DBG_TIM16_STOP           DBGMCU_APB2FZ_DBG_TIM16_STOP_Msk

/******************************************************************************/
/*                                                                            */
/*                         USB Device FS Endpoint registers                   */
/*                                                                            */
/******************************************************************************/
#define USB_EP0R                                 USB_BASE                  /*!< endpoint 0 register address */
#define USB_EP1R                                 (USB_BASE + 0x00000004UL)   /*!< endpoint 1 register address */
#define USB_EP2R                                 (USB_BASE + 0x00000008UL)   /*!< endpoint 2 register address */
#define USB_EP3R                                 (USB_BASE + 0x0000000CUL)   /*!< endpoint 3 register address */
#define USB_EP4R                                 (USB_BASE + 0x00000010UL)   /*!< endpoint 4 register address */
#define USB_EP5R                                 (USB_BASE + 0x00000014UL)   /*!< endpoint 5 register address */
#define USB_EP6R                                 (USB_BASE + 0x00000018UL)   /*!< endpoint 6 register address */
#define USB_EP7R                                 (USB_BASE + 0x0000001CUL)   /*!< endpoint 7 register address */

/* bit positions */
#define USB_EP_CTR_RX                            ((uint16_t)0x8000U)           /*!<  EndPoint Correct TRansfer RX */
#define USB_EP_DTOG_RX                           ((uint16_t)0x4000U)           /*!<  EndPoint Data TOGGLE RX */
#define USB_EPRX_STAT                            ((uint16_t)0x3000U)           /*!<  EndPoint RX STATus bit field */
#define USB_EP_SETUP                             ((uint16_t)0x0800U)           /*!<  EndPoint SETUP */
#define USB_EP_T_FIELD                           ((uint16_t)0x0600U)           /*!<  EndPoint TYPE */
#define USB_EP_KIND                              ((uint16_t)0x0100U)           /*!<  EndPoint KIND */
#define USB_EP_CTR_TX                            ((uint16_t)0x0080U)           /*!<  EndPoint Correct TRansfer TX */
#define USB_EP_DTOG_TX                           ((uint16_t)0x0040U)           /*!<  EndPoint Data TOGGLE TX */
#define USB_EPTX_STAT                            ((uint16_t)0x0030U)           /*!<  EndPoint TX STATus bit field */
#define USB_EPADDR_FIELD                         ((uint16_t)0x000FU)           /*!<  EndPoint ADDRess FIELD */

/* EndPoint REGister MASK (no toggle fields) */
#define USB_EPREG_MASK     (USB_EP_CTR_RX|USB_EP_SETUP|USB_EP_T_FIELD|USB_EP_KIND|USB_EP_CTR_TX|USB_EPADDR_FIELD)
                                                                         /*!< EP_TYPE[1:0] EndPoint TYPE */
#define USB_EP_TYPE_MASK                         ((uint16_t)0x0600U)           /*!< EndPoint TYPE Mask */
#define USB_EP_BULK                              ((uint16_t)0x0000U)           /*!< EndPoint BULK */
#define USB_EP_CONTROL                           ((uint16_t)0x0200U)           /*!< EndPoint CONTROL */
#define USB_EP_ISOCHRONOUS                       ((uint16_t)0x0400U)           /*!< EndPoint ISOCHRONOUS */
#define USB_EP_INTERRUPT                         ((uint16_t)0x0600U)           /*!< EndPoint INTERRUPT */
#define USB_EP_T_MASK                        ((uint16_t) ~USB_EP_T_FIELD & USB_EPREG_MASK)

#define USB_EPKIND_MASK                      ((uint16_t)~USB_EP_KIND & USB_EPREG_MASK) /*!< EP_KIND EndPoint KIND */
                                                                         /*!< STAT_TX[1:0] STATus for TX transfer */
#define USB_EP_TX_DIS                            ((uint16_t)0x0000U)           /*!< EndPoint TX DISabled */
#define USB_EP_TX_STALL                          ((uint16_t)0x0010U)           /*!< EndPoint TX STALLed */
#define USB_EP_TX_NAK                            ((uint16_t)0x0020U)           /*!< EndPoint TX NAKed */
#define USB_EP_TX_VALID                          ((uint16_t)0x0030U)           /*!< EndPoint TX VALID */
#define USB_EPTX_DTOG1                           ((uint16_t)0x0010U)           /*!< EndPoint TX Data TOGgle bit1 */
#define USB_EPTX_DTOG2                           ((uint16_t)0x0020U)           /*!< EndPoint TX Data TOGgle bit2 */
#define USB_EPTX_DTOGMASK  (USB_EPTX_STAT|USB_EPREG_MASK)
                                                                         /*!< STAT_RX[1:0] STATus for RX transfer */
#define USB_EP_RX_DIS                            ((uint16_t)0x0000U)           /*!< EndPoint RX DISabled */
#define USB_EP_RX_STALL                          ((uint16_t)0x1000U)           /*!< EndPoint RX STALLed */
#define USB_EP_RX_NAK                            ((uint16_t)0x2000U)           /*!< EndPoint RX NAKed */
#define USB_EP_RX_VALID                          ((uint16_t)0x3000U)           /*!< EndPoint RX VALID */
#define USB_EPRX_DTOG1                           ((uint16_t)0x1000U)           /*!< EndPoint RX Data TOGgle bit1 */
#define USB_EPRX_DTOG2                           ((uint16_t)0x2000U)           /*!< EndPoint RX Data TOGgle bit1 */
#define USB_EPRX_DTOGMASK  (USB_EPRX_STAT|USB_EPREG_MASK)

/******************************************************************************/
/*                                                                            */
/*                         USB Device FS General registers                    */
/*                                                                            */
/******************************************************************************/
#define USB_CNTR                             (USB_BASE + 0x00000040UL)     /*!< Control register */
#define USB_ISTR                             (USB_BASE + 0x00000044UL)     /*!< Interrupt status register */
#define USB_FNR                              (USB_BASE + 0x00000048UL)     /*!< Frame number register */
#define USB_DADDR                            (USB_BASE + 0x0000004CUL)     /*!< Device address register */
#define USB_BTABLE                           (USB_BASE + 0x00000050UL)     /*!< Buffer Table address register */
#define USB_LPMCSR                           (USB_BASE + 0x00000054UL)     /*!< LPM Control and Status register */
#define USB_BCDR                             (USB_BASE + 0x00000058UL)     /*!< Battery Charging detector register*/

/******************  Bits definition for USB_CNTR register  *******************/
#define USB_CNTR_CTRM                            ((uint16_t)0x8000U)           /*!< Correct TRansfer Mask */
#define USB_CNTR_PMAOVRM                         ((uint16_t)0x4000U)           /*!< DMA OVeR/underrun Mask */
#define USB_CNTR_ERRM                            ((uint16_t)0x2000U)           /*!< ERRor Mask */
#define USB_CNTR_WKUPM                           ((uint16_t)0x1000U)           /*!< WaKe UP Mask */
#define USB_CNTR_SUSPM                           ((uint16_t)0x0800U)           /*!< SUSPend Mask */
#define USB_CNTR_RESETM                          ((uint16_t)0x0400U)           /*!< RESET Mask   */
#define USB_CNTR_SOFM                            ((uint16_t)0x0200U)           /*!< Start Of Frame Mask */
#define USB_CNTR_ESOFM                           ((uint16_t)0x0100U)           /*!< Expected Start Of Frame Mask */
#define USB_CNTR_L1REQM                          ((uint16_t)0x0080U)           /*!< LPM L1 state request interrupt mask */
#define USB_CNTR_L1RESUME                        ((uint16_t)0x0020U)           /*!< LPM L1 Resume request */
#define USB_CNTR_RESUME                          ((uint16_t)0x0010U)           /*!< RESUME request */
#define USB_CNTR_FSUSP                           ((uint16_t)0x0008U)           /*!< Force SUSPend */
#define USB_CNTR_LPMODE                          ((uint16_t)0x0004U)           /*!< Low-power MODE */
#define USB_CNTR_PDWN                            ((uint16_t)0x0002U)           /*!< Power DoWN */
#define USB_CNTR_FRES                            ((uint16_t)0x0001U)           /*!< Force USB RESet */

/******************  Bits definition for USB_ISTR register  *******************/
#define USB_ISTR_EP_ID                           ((uint16_t)0x000FU)           /*!< EndPoint IDentifier (read-only bit)  */
#define USB_ISTR_DIR                             ((uint16_t)0x0010U)           /*!< DIRection of transaction (read-only bit)  */
#define USB_ISTR_L1REQ                           ((uint16_t)0x0080U)           /*!< LPM L1 state request  */
#define USB_ISTR_ESOF                            ((uint16_t)0x0100U)           /*!< Expected Start Of Frame (clear-only bit) */
#define USB_ISTR_SOF                             ((uint16_t)0x0200U)           /*!< Start Of Frame (clear-only bit) */
#define USB_ISTR_RESET                           ((uint16_t)0x0400U)           /*!< RESET (clear-only bit) */
#define USB_ISTR_SUSP                            ((uint16_t)0x0800U)           /*!< SUSPend (clear-only bit) */
#define USB_ISTR_WKUP                            ((uint16_t)0x1000U)           /*!< WaKe UP (clear-only bit) */
#define USB_ISTR_ERR                             ((uint16_t)0x2000U)           /*!< ERRor (clear-only bit) */
#define USB_ISTR_PMAOVR                          ((uint16_t)0x4000U)           /*!< DMA OVeR/underrun (clear-only bit) */
#define USB_ISTR_CTR                             ((uint16_t)0x8000U)           /*!< Correct TRansfer (clear-only bit) */

#define USB_CLR_L1REQ                        (~USB_ISTR_L1REQ)           /*!< clear LPM L1  bit */
#define USB_CLR_ESOF                         (~USB_ISTR_ESOF)            /*!< clear Expected Start Of Frame bit */
#define USB_CLR_SOF                          (~USB_ISTR_SOF)             /*!< clear Start Of Frame bit */
#define USB_CLR_RESET                        (~USB_ISTR_RESET)           /*!< clear RESET bit */
#define USB_CLR_SUSP                         (~USB_ISTR_SUSP)            /*!< clear SUSPend bit */
#define USB_CLR_WKUP                         (~USB_ISTR_WKUP)            /*!< clear WaKe UP bit */
#define USB_CLR_ERR                          (~USB_ISTR_ERR)             /*!< clear ERRor bit */
#define USB_CLR_PMAOVR                       (~USB_ISTR_PMAOVR)          /*!< clear DMA OVeR/underrun bit*/
#define USB_CLR_CTR                          (~USB_ISTR_CTR)             /*!< clear Correct TRansfer bit */

/******************  Bits definition for USB_FNR register  ********************/
#define USB_FNR_FN                               ((uint16_t)0x07FFU)           /*!< Frame Number */
#define USB_FNR_LSOF                             ((uint16_t)0x1800U)           /*!< Lost SOF */
#define USB_FNR_LCK                              ((uint16_t)0x2000U)           /*!< LoCKed */
#define USB_FNR_RXDM                             ((uint16_t)0x4000U)           /*!< status of D- data line */
#define USB_FNR_RXDP                             ((uint16_t)0x8000U)           /*!< status of D+ data line */

/******************  Bits definition for USB_DADDR register    ****************/
#define USB_DADDR_ADD                            ((uint8_t)0x7FU)              /*!< ADD[6:0] bits (Device Address) */
#define USB_DADDR_ADD0                           ((uint8_t)0x01U)              /*!< Bit 0 */
#define USB_DADDR_ADD1                           ((uint8_t)0x02U)              /*!< Bit 1 */
#define USB_DADDR_ADD2                           ((uint8_t)0x04U)              /*!< Bit 2 */
#define USB_DADDR_ADD3                           ((uint8_t)0x08U)              /*!< Bit 3 */
#define USB_DADDR_ADD4                           ((uint8_t)0x10U)              /*!< Bit 4 */
#define USB_DADDR_ADD5                           ((uint8_t)0x20U)              /*!< Bit 5 */
#define USB_DADDR_ADD6                           ((uint8_t)0x40U)              /*!< Bit 6 */

#define USB_DADDR_EF                             ((uint8_t)0x80U)              /*!< Enable Function */

/******************  Bit definition for USB_BTABLE register  ******************/
#define USB_BTABLE_BTABLE                        ((uint16_t)0xFFF8U)           /*!< Buffer Table */

/******************  Bits definition for USB_BCDR register  *******************/
#define USB_BCDR_BCDEN                           ((uint16_t)0x0001U)           /*!< Battery charging detector (BCD) enable */
#define USB_BCDR_DCDEN                           ((uint16_t)0x0002U)           /*!< Data contact detection (DCD) mode enable */
#define USB_BCDR_PDEN                            ((uint16_t)0x0004U)           /*!< Primary detection (PD) mode enable */
#define USB_BCDR_SDEN                            ((uint16_t)0x0008U)           /*!< Secondary detection (SD) mode enable */
#define USB_BCDR_DCDET                           ((uint16_t)0x0010U)           /*!< Data contact detection (DCD) status */
#define USB_BCDR_PDET                            ((uint16_t)0x0020U)           /*!< Primary detection (PD) status */
#define USB_BCDR_SDET                            ((uint16_t)0x0040U)           /*!< Secondary detection (SD) status */
#define USB_BCDR_PS2DET                          ((uint16_t)0x0080U)           /*!< PS2 port or proprietary charger detected */
#define USB_BCDR_DPPU                            ((uint16_t)0x8000U)           /*!< DP Pull-up Enable */

/*******************  Bit definition for LPMCSR register  *********************/
#define USB_LPMCSR_LMPEN                         ((uint16_t)0x0001U)           /*!< LPM support enable  */
#define USB_LPMCSR_LPMACK                        ((uint16_t)0x0002U)           /*!< LPM Token acknowledge enable*/
#define USB_LPMCSR_REMWAKE                       ((uint16_t)0x0008U)           /*!< bRemoteWake value received with last ACKed LPM Token */
#define USB_LPMCSR_BESL                          ((uint16_t)0x00F0U)           /*!< BESL value received with last ACKed LPM Token  */

/*!< Buffer descriptor table */
/*****************  Bit definition for USB_ADDR0_TX register  *****************/
#define USB_ADDR0_TX_ADDR0_TX_Pos                (1U)
#define USB_ADDR0_TX_ADDR0_TX_Msk                (0x7FFFUL << USB_ADDR0_TX_ADDR0_TX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR0_TX_ADDR0_TX                    USB_ADDR0_TX_ADDR0_TX_Msk     /*!< Transmission Buffer Address 0 */

/*****************  Bit definition for USB_ADDR1_TX register  *****************/
#define USB_ADDR1_TX_ADDR1_TX_Pos                (1U)
#define USB_ADDR1_TX_ADDR1_TX_Msk                (0x7FFFUL << USB_ADDR1_TX_ADDR1_TX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR1_TX_ADDR1_TX                    USB_ADDR1_TX_ADDR1_TX_Msk     /*!< Transmission Buffer Address 1 */

/*****************  Bit definition for USB_ADDR2_TX register  *****************/
#define USB_ADDR2_TX_ADDR2_TX_Pos                (1U)
#define USB_ADDR2_TX_ADDR2_TX_Msk                (0x7FFFUL << USB_ADDR2_TX_ADDR2_TX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR2_TX_ADDR2_TX                    USB_ADDR2_TX_ADDR2_TX_Msk     /*!< Transmission Buffer Address 2 */

/*****************  Bit definition for USB_ADDR3_TX register  *****************/
#define USB_ADDR3_TX_ADDR3_TX_Pos                (1U)
#define USB_ADDR3_TX_ADDR3_TX_Msk                (0x7FFFUL << USB_ADDR3_TX_ADDR3_TX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR3_TX_ADDR3_TX                    USB_ADDR3_TX_ADDR3_TX_Msk     /*!< Transmission Buffer Address 3 */

/*****************  Bit definition for USB_ADDR4_TX register  *****************/
#define USB_ADDR4_TX_ADDR4_TX_Pos                (1U)
#define USB_ADDR4_TX_ADDR4_TX_Msk                (0x7FFFUL << USB_ADDR4_TX_ADDR4_TX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR4_TX_ADDR4_TX                    USB_ADDR4_TX_ADDR4_TX_Msk     /*!< Transmission Buffer Address 4 */

/*****************  Bit definition for USB_ADDR5_TX register  *****************/
#define USB_ADDR5_TX_ADDR5_TX_Pos                (1U)
#define USB_ADDR5_TX_ADDR5_TX_Msk                (0x7FFFUL << USB_ADDR5_TX_ADDR5_TX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR5_TX_ADDR5_TX                    USB_ADDR5_TX_ADDR5_TX_Msk     /*!< Transmission Buffer Address 5 */

/*****************  Bit definition for USB_ADDR6_TX register  *****************/
#define USB_ADDR6_TX_ADDR6_TX_Pos                (1U)
#define USB_ADDR6_TX_ADDR6_TX_Msk                (0x7FFFUL << USB_ADDR6_TX_ADDR6_TX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR6_TX_ADDR6_TX                    USB_ADDR6_TX_ADDR6_TX_Msk     /*!< Transmission Buffer Address 6 */

/*****************  Bit definition for USB_ADDR7_TX register  *****************/
#define USB_ADDR7_TX_ADDR7_TX_Pos                (1U)
#define USB_ADDR7_TX_ADDR7_TX_Msk                (0x7FFFUL << USB_ADDR7_TX_ADDR7_TX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR7_TX_ADDR7_TX                    USB_ADDR7_TX_ADDR7_TX_Msk     /*!< Transmission Buffer Address 7 */

/*----------------------------------------------------------------------------*/

/*****************  Bit definition for USB_COUNT0_TX register  ****************/
#define USB_COUNT0_TX_COUNT0_TX_Pos              (0U)
#define USB_COUNT0_TX_COUNT0_TX_Msk              (0x3FFUL << USB_COUNT0_TX_COUNT0_TX_Pos) /*!< 0x000003FF */
#define USB_COUNT0_TX_COUNT0_TX                  USB_COUNT0_TX_COUNT0_TX_Msk   /*!< Transmission Byte Count 0 */

/*****************  Bit definition for USB_COUNT1_TX register  ****************/
#define USB_COUNT1_TX_COUNT1_TX_Pos              (0U)
#define USB_COUNT1_TX_COUNT1_TX_Msk              (0x3FFUL << USB_COUNT1_TX_COUNT1_TX_Pos) /*!< 0x000003FF */
#define USB_COUNT1_TX_COUNT1_TX                  USB_COUNT1_TX_COUNT1_TX_Msk   /*!< Transmission Byte Count 1 */

/*****************  Bit definition for USB_COUNT2_TX register  ****************/
#define USB_COUNT2_TX_COUNT2_TX_Pos              (0U)
#define USB_COUNT2_TX_COUNT2_TX_Msk              (0x3FFUL << USB_COUNT2_TX_COUNT2_TX_Pos) /*!< 0x000003FF */
#define USB_COUNT2_TX_COUNT2_TX                  USB_COUNT2_TX_COUNT2_TX_Msk   /*!< Transmission Byte Count 2 */

/*****************  Bit definition for USB_COUNT3_TX register  ****************/
#define USB_COUNT3_TX_COUNT3_TX_Pos              (0U)
#define USB_COUNT3_TX_COUNT3_TX_Msk              (0x3FFUL << USB_COUNT3_TX_COUNT3_TX_Pos) /*!< 0x000003FF */
#define USB_COUNT3_TX_COUNT3_TX                  USB_COUNT3_TX_COUNT3_TX_Msk   /*!< Transmission Byte Count 3 */

/*****************  Bit definition for USB_COUNT4_TX register  ****************/
#define USB_COUNT4_TX_COUNT4_TX_Pos              (0U)
#define USB_COUNT4_TX_COUNT4_TX_Msk              (0x3FFUL << USB_COUNT4_TX_COUNT4_TX_Pos) /*!< 0x000003FF */
#define USB_COUNT4_TX_COUNT4_TX                  USB_COUNT4_TX_COUNT4_TX_Msk   /*!< Transmission Byte Count 4 */

/*****************  Bit definition for USB_COUNT5_TX register  ****************/
#define USB_COUNT5_TX_COUNT5_TX_Pos              (0U)
#define USB_COUNT5_TX_COUNT5_TX_Msk              (0x3FFUL << USB_COUNT5_TX_COUNT5_TX_Pos) /*!< 0x000003FF */
#define USB_COUNT5_TX_COUNT5_TX                  USB_COUNT5_TX_COUNT5_TX_Msk   /*!< Transmission Byte Count 5 */

/*****************  Bit definition for USB_COUNT6_TX register  ****************/
#define USB_COUNT6_TX_COUNT6_TX_Pos              (0U)
#define USB_COUNT6_TX_COUNT6_TX_Msk              (0x3FFUL << USB_COUNT6_TX_COUNT6_TX_Pos) /*!< 0x000003FF */
#define USB_COUNT6_TX_COUNT6_TX                  USB_COUNT6_TX_COUNT6_TX_Msk   /*!< Transmission Byte Count 6 */

/*****************  Bit definition for USB_COUNT7_TX register  ****************/
#define USB_COUNT7_TX_COUNT7_TX_Pos              (0U)
#define USB_COUNT7_TX_COUNT7_TX_Msk              (0x3FFUL << USB_COUNT7_TX_COUNT7_TX_Pos) /*!< 0x000003FF */
#define USB_COUNT7_TX_COUNT7_TX                  USB_COUNT7_TX_COUNT7_TX_Msk   /*!< Transmission Byte Count 7 */

/*----------------------------------------------------------------------------*/

/****************  Bit definition for USB_COUNT0_TX_0 register  ***************/
#define USB_COUNT0_TX_0_COUNT0_TX_0         (0x000003FFUL)       /*!< Transmission Byte Count 0 (low) */

/****************  Bit definition for USB_COUNT0_TX_1 register  ***************/
#define USB_COUNT0_TX_1_COUNT0_TX_1         (0x03FF0000UL)       /*!< Transmission Byte Count 0 (high) */

/****************  Bit definition for USB_COUNT1_TX_0 register  ***************/
#define USB_COUNT1_TX_0_COUNT1_TX_0         (0x000003FFUL)       /*!< Transmission Byte Count 1 (low) */

/****************  Bit definition for USB_COUNT1_TX_1 register  ***************/
#define USB_COUNT1_TX_1_COUNT1_TX_1         (0x03FF0000UL)       /*!< Transmission Byte Count 1 (high) */

/****************  Bit definition for USB_COUNT2_TX_0 register  ***************/
#define USB_COUNT2_TX_0_COUNT2_TX_0         (0x000003FFUL)       /*!< Transmission Byte Count 2 (low) */

/****************  Bit definition for USB_COUNT2_TX_1 register  ***************/
#define USB_COUNT2_TX_1_COUNT2_TX_1         (0x03FF0000UL)       /*!< Transmission Byte Count 2 (high) */

/****************  Bit definition for USB_COUNT3_TX_0 register  ***************/
#define USB_COUNT3_TX_0_COUNT3_TX_0         (0x000003FFUL)       /*!< Transmission Byte Count 3 (low) */

/****************  Bit definition for USB_COUNT3_TX_1 register  ***************/
#define USB_COUNT3_TX_1_COUNT3_TX_1         (0x03FF0000UL)       /*!< Transmission Byte Count 3 (high) */

/****************  Bit definition for USB_COUNT4_TX_0 register  ***************/
#define USB_COUNT4_TX_0_COUNT4_TX_0         (0x000003FFUL)       /*!< Transmission Byte Count 4 (low) */

/****************  Bit definition for USB_COUNT4_TX_1 register  ***************/
#define USB_COUNT4_TX_1_COUNT4_TX_1         (0x03FF0000UL)       /*!< Transmission Byte Count 4 (high) */

/****************  Bit definition for USB_COUNT5_TX_0 register  ***************/
#define USB_COUNT5_TX_0_COUNT5_TX_0         (0x000003FFUL)       /*!< Transmission Byte Count 5 (low) */

/****************  Bit definition for USB_COUNT5_TX_1 register  ***************/
#define USB_COUNT5_TX_1_COUNT5_TX_1         (0x03FF0000UL)       /*!< Transmission Byte Count 5 (high) */

/****************  Bit definition for USB_COUNT6_TX_0 register  ***************/
#define USB_COUNT6_TX_0_COUNT6_TX_0         (0x000003FFUL)       /*!< Transmission Byte Count 6 (low) */

/****************  Bit definition for USB_COUNT6_TX_1 register  ***************/
#define USB_COUNT6_TX_1_COUNT6_TX_1         (0x03FF0000UL)       /*!< Transmission Byte Count 6 (high) */

/****************  Bit definition for USB_COUNT7_TX_0 register  ***************/
#define USB_COUNT7_TX_0_COUNT7_TX_0         (0x000003FFUL)       /*!< Transmission Byte Count 7 (low) */

/****************  Bit definition for USB_COUNT7_TX_1 register  ***************/
#define USB_COUNT7_TX_1_COUNT7_TX_1         (0x03FF0000UL)       /*!< Transmission Byte Count 7 (high) */

/*----------------------------------------------------------------------------*/

/*****************  Bit definition for USB_ADDR0_RX register  *****************/
#define USB_ADDR0_RX_ADDR0_RX_Pos                (1U)
#define USB_ADDR0_RX_ADDR0_RX_Msk                (0x7FFFUL << USB_ADDR0_RX_ADDR0_RX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR0_RX_ADDR0_RX                    USB_ADDR0_RX_ADDR0_RX_Msk     /*!< Reception Buffer Address 0 */

/*****************  Bit definition for USB_ADDR1_RX register  *****************/
#define USB_ADDR1_RX_ADDR1_RX_Pos                (1U)
#define USB_ADDR1_RX_ADDR1_RX_Msk                (0x7FFFUL << USB_ADDR1_RX_ADDR1_RX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR1_RX_ADDR1_RX                    USB_ADDR1_RX_ADDR1_RX_Msk     /*!< Reception Buffer Address 1 */

/*****************  Bit definition for USB_ADDR2_RX register  *****************/
#define USB_ADDR2_RX_ADDR2_RX_Pos                (1U)
#define USB_ADDR2_RX_ADDR2_RX_Msk                (0x7FFFUL << USB_ADDR2_RX_ADDR2_RX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR2_RX_ADDR2_RX                    USB_ADDR2_RX_ADDR2_RX_Msk     /*!< Reception Buffer Address 2 */

/*****************  Bit definition for USB_ADDR3_RX register  *****************/
#define USB_ADDR3_RX_ADDR3_RX_Pos                (1U)
#define USB_ADDR3_RX_ADDR3_RX_Msk                (0x7FFFUL << USB_ADDR3_RX_ADDR3_RX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR3_RX_ADDR3_RX                    USB_ADDR3_RX_ADDR3_RX_Msk     /*!< Reception Buffer Address 3 */

/*****************  Bit definition for USB_ADDR4_RX register  *****************/
#define USB_ADDR4_RX_ADDR4_RX_Pos                (1U)
#define USB_ADDR4_RX_ADDR4_RX_Msk                (0x7FFFUL << USB_ADDR4_RX_ADDR4_RX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR4_RX_ADDR4_RX                    USB_ADDR4_RX_ADDR4_RX_Msk     /*!< Reception Buffer Address 4 */

/*****************  Bit definition for USB_ADDR5_RX register  *****************/
#define USB_ADDR5_RX_ADDR5_RX_Pos                (1U)
#define USB_ADDR5_RX_ADDR5_RX_Msk                (0x7FFFUL << USB_ADDR5_RX_ADDR5_RX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR5_RX_ADDR5_RX                    USB_ADDR5_RX_ADDR5_RX_Msk     /*!< Reception Buffer Address 5 */

/*****************  Bit definition for USB_ADDR6_RX register  *****************/
#define USB_ADDR6_RX_ADDR6_RX_Pos                (1U)
#define USB_ADDR6_RX_ADDR6_RX_Msk                (0x7FFFUL << USB_ADDR6_RX_ADDR6_RX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR6_RX_ADDR6_RX                    USB_ADDR6_RX_ADDR6_RX_Msk     /*!< Reception Buffer Address 6 */

/*****************  Bit definition for USB_ADDR7_RX register  *****************/
#define USB_ADDR7_RX_ADDR7_RX_Pos                (1U)
#define USB_ADDR7_RX_ADDR7_RX_Msk                (0x7FFFUL << USB_ADDR7_RX_ADDR7_RX_Pos) /*!< 0x0000FFFE */
#define USB_ADDR7_RX_ADDR7_RX                    USB_ADDR7_RX_ADDR7_RX_Msk     /*!< Reception Buffer Address 7 */

/*----------------------------------------------------------------------------*/

/*****************  Bit definition for USB_COUNT0_RX register  ****************/
#define USB_COUNT0_RX_COUNT0_RX_Pos              (0U)
#define USB_COUNT0_RX_COUNT0_RX_Msk              (0x3FFUL << USB_COUNT0_RX_COUNT0_RX_Pos) /*!< 0x000003FF */
#define USB_COUNT0_RX_COUNT0_RX                  USB_COUNT0_RX_COUNT0_RX_Msk   /*!< Reception Byte Count */

#define USB_COUNT0_RX_NUM_BLOCK_Pos              (10U)
#define USB_COUNT0_RX_NUM_BLOCK_Msk              (0x1FUL << USB_COUNT0_RX_NUM_BLOCK_Pos) /*!< 0x00007C00 */
#define USB_COUNT0_RX_NUM_BLOCK                  USB_COUNT0_RX_NUM_BLOCK_Msk   /*!< NUM_BLOCK[4:0] bits (Number of blocks) */
#define USB_COUNT0_RX_NUM_BLOCK_0                (0x01UL << USB_COUNT0_RX_NUM_BLOCK_Pos) /*!< 0x00000400 */
#define USB_COUNT0_RX_NUM_BLOCK_1                (0x02UL << USB_COUNT0_RX_NUM_BLOCK_Pos) /*!< 0x00000800 */
#define USB_COUNT0_RX_NUM_BLOCK_2                (0x04UL << USB_COUNT0_RX_NUM_BLOCK_Pos) /*!< 0x00001000 */
#define USB_COUNT0_RX_NUM_BLOCK_3                (0x08UL << USB_COUNT0_RX_NUM_BLOCK_Pos) /*!< 0x00002000 */
#define USB_COUNT0_RX_NUM_BLOCK_4                (0x10UL << USB_COUNT0_RX_NUM_BLOCK_Pos) /*!< 0x00004000 */

#define USB_COUNT0_RX_BLSIZE_Pos                 (15U)
#define USB_COUNT0_RX_BLSIZE_Msk                 (0x1UL << USB_COUNT0_RX_BLSIZE_Pos) /*!< 0x00008000 */
#define USB_COUNT0_RX_BLSIZE                     USB_COUNT0_RX_BLSIZE_Msk      /*!< BLock SIZE */

/*****************  Bit definition for USB_COUNT1_RX register  ****************/
#define USB_COUNT1_RX_COUNT1_RX_Pos              (0U)
#define USB_COUNT1_RX_COUNT1_RX_Msk              (0x3FFUL << USB_COUNT1_RX_COUNT1_RX_Pos) /*!< 0x000003FF */
#define USB_COUNT1_RX_COUNT1_RX                  USB_COUNT1_RX_COUNT1_RX_Msk   /*!< Reception Byte Count */

#define USB_COUNT1_RX_NUM_BLOCK_Pos              (10U)
#define USB_COUNT1_RX_NUM_BLOCK_Msk              (0x1FUL << USB_COUNT1_RX_NUM_BLOCK_Pos) /*!< 0x00007C00 */
#define USB_COUNT1_RX_NUM_BLOCK                  USB_COUNT1_RX_NUM_BLOCK_Msk   /*!< NUM_BLOCK[4:0] bits (Number of blocks) */
#define USB_COUNT1_RX_NUM_BLOCK_0                (0x01UL << USB_COUNT1_RX_NUM_BLOCK_Pos) /*!< 0x00000400 */
#define USB_COUNT1_RX_NUM_BLOCK_1                (0x02UL << USB_COUNT1_RX_NUM_BLOCK_Pos) /*!< 0x00000800 */
#define USB_COUNT1_RX_NUM_BLOCK_2                (0x04UL << USB_COUNT1_RX_NUM_BLOCK_Pos) /*!< 0x00001000 */
#define USB_COUNT1_RX_NUM_BLOCK_3                (0x08UL << USB_COUNT1_RX_NUM_BLOCK_Pos) /*!< 0x00002000 */
#define USB_COUNT1_RX_NUM_BLOCK_4                (0x10UL << USB_COUNT1_RX_NUM_BLOCK_Pos) /*!< 0x00004000 */

#define USB_COUNT1_RX_BLSIZE_Pos                 (15U)
#define USB_COUNT1_RX_BLSIZE_Msk                 (0x1UL << USB_COUNT1_RX_BLSIZE_Pos) /*!< 0x00008000 */
#define USB_COUNT1_RX_BLSIZE                     USB_COUNT1_RX_BLSIZE_Msk      /*!< BLock SIZE */

/*****************  Bit definition for USB_COUNT2_RX register  ****************/
#define USB_COUNT2_RX_COUNT2_RX_Pos              (0U)
#define USB_COUNT2_RX_COUNT2_RX_Msk              (0x3FFUL << USB_COUNT2_RX_COUNT2_RX_Pos) /*!< 0x000003FF */
#define USB_COUNT2_RX_COUNT2_RX                  USB_COUNT2_RX_COUNT2_RX_Msk   /*!< Reception Byte Count */

#define USB_COUNT2_RX_NUM_BLOCK_Pos              (10U)
#define USB_COUNT2_RX_NUM_BLOCK_Msk              (0x1FUL << USB_COUNT2_RX_NUM_BLOCK_Pos) /*!< 0x00007C00 */
#define USB_COUNT2_RX_NUM_BLOCK                  USB_COUNT2_RX_NUM_BLOCK_Msk   /*!< NUM_BLOCK[4:0] bits (Number of blocks) */
#define USB_COUNT2_RX_NUM_BLOCK_0                (0x01UL << USB_COUNT2_RX_NUM_BLOCK_Pos) /*!< 0x00000400 */
#define USB_COUNT2_RX_NUM_BLOCK_1                (0x02UL << USB_COUNT2_RX_NUM_BLOCK_Pos) /*!< 0x00000800 */
#define USB_COUNT2_RX_NUM_BLOCK_2                (0x04UL << USB_COUNT2_RX_NUM_BLOCK_Pos) /*!< 0x00001000 */
#define USB_COUNT2_RX_NUM_BLOCK_3                (0x08UL << USB_COUNT2_RX_NUM_BLOCK_Pos) /*!< 0x00002000 */
#define USB_COUNT2_RX_NUM_BLOCK_4                (0x10UL << USB_COUNT2_RX_NUM_BLOCK_Pos) /*!< 0x00004000 */

#define USB_COUNT2_RX_BLSIZE_Pos                 (15U)
#define USB_COUNT2_RX_BLSIZE_Msk                 (0x1UL << USB_COUNT2_RX_BLSIZE_Pos) /*!< 0x00008000 */
#define USB_COUNT2_RX_BLSIZE                     USB_COUNT2_RX_BLSIZE_Msk      /*!< BLock SIZE */

/*****************  Bit definition for USB_COUNT3_RX register  ****************/
#define USB_COUNT3_RX_COUNT3_RX_Pos              (0U)
#define USB_COUNT3_RX_COUNT3_RX_Msk              (0x3FFUL << USB_COUNT3_RX_COUNT3_RX_Pos) /*!< 0x000003FF */
#define USB_COUNT3_RX_COUNT3_RX                  USB_COUNT3_RX_COUNT3_RX_Msk   /*!< Reception Byte Count */

#define USB_COUNT3_RX_NUM_BLOCK_Pos              (10U)
#define USB_COUNT3_RX_NUM_BLOCK_Msk              (0x1FUL << USB_COUNT3_RX_NUM_BLOCK_Pos) /*!< 0x00007C00 */
#define USB_COUNT3_RX_NUM_BLOCK                  USB_COUNT3_RX_NUM_BLOCK_Msk   /*!< NUM_BLOCK[4:0] bits (Number of blocks) */
#define USB_COUNT3_RX_NUM_BLOCK_0                (0x01UL << USB_COUNT3_RX_NUM_BLOCK_Pos) /*!< 0x00000400 */
#define USB_COUNT3_RX_NUM_BLOCK_1                (0x02UL << USB_COUNT3_RX_NUM_BLOCK_Pos) /*!< 0x00000800 */
#define USB_COUNT3_RX_NUM_BLOCK_2                (0x04UL << USB_COUNT3_RX_NUM_BLOCK_Pos) /*!< 0x00001000 */
#define USB_COUNT3_RX_NUM_BLOCK_3                (0x08UL << USB_COUNT3_RX_NUM_BLOCK_Pos) /*!< 0x00002000 */
#define USB_COUNT3_RX_NUM_BLOCK_4                (0x10UL << USB_COUNT3_RX_NUM_BLOCK_Pos) /*!< 0x00004000 */

#define USB_COUNT3_RX_BLSIZE_Pos                 (15U)
#define USB_COUNT3_RX_BLSIZE_Msk                 (0x1UL << USB_COUNT3_RX_BLSIZE_Pos) /*!< 0x00008000 */
#define USB_COUNT3_RX_BLSIZE                     USB_COUNT3_RX_BLSIZE_Msk      /*!< BLock SIZE */

/*****************  Bit definition for USB_COUNT4_RX register  ****************/
#define USB_COUNT4_RX_COUNT4_RX_Pos              (0U)
#define USB_COUNT4_RX_COUNT4_RX_Msk              (0x3FFUL << USB_COUNT4_RX_COUNT4_RX_Pos) /*!< 0x000003FF */
#define USB_COUNT4_RX_COUNT4_RX                  USB_COUNT4_RX_COUNT4_RX_Msk   /*!< Reception Byte Count */

#define USB_COUNT4_RX_NUM_BLOCK_Pos              (10U)
#define USB_COUNT4_RX_NUM_BLOCK_Msk              (0x1FUL << USB_COUNT4_RX_NUM_BLOCK_Pos) /*!< 0x00007C00 */
#define USB_COUNT4_RX_NUM_BLOCK                  USB_COUNT4_RX_NUM_BLOCK_Msk   /*!< NUM_BLOCK[4:0] bits (Number of blocks) */
#define USB_COUNT4_RX_NUM_BLOCK_0                (0x01UL << USB_COUNT4_RX_NUM_BLOCK_Pos) /*!< 0x00000400 */
#define USB_COUNT4_RX_NUM_BLOCK_1                (0x02UL << USB_COUNT4_RX_NUM_BLOCK_Pos) /*!< 0x00000800 */
#define USB_COUNT4_RX_NUM_BLOCK_2                (0x04UL << USB_COUNT4_RX_NUM_BLOCK_Pos) /*!< 0x00001000 */
#define USB_COUNT4_RX_NUM_BLOCK_3                (0x08UL << USB_COUNT4_RX_NUM_BLOCK_Pos) /*!< 0x00002000 */
#define USB_COUNT4_RX_NUM_BLOCK_4                (0x10UL << USB_COUNT4_RX_NUM_BLOCK_Pos) /*!< 0x00004000 */

#define USB_COUNT4_RX_BLSIZE_Pos                 (15U)
#define USB_COUNT4_RX_BLSIZE_Msk                 (0x1UL << USB_COUNT4_RX_BLSIZE_Pos) /*!< 0x00008000 */
#define USB_COUNT4_RX_BLSIZE                     USB_COUNT4_RX_BLSIZE_Msk      /*!< BLock SIZE */

/*****************  Bit definition for USB_COUNT5_RX register  ****************/
#define USB_COUNT5_RX_COUNT5_RX_Pos              (0U)
#define USB_COUNT5_RX_COUNT5_RX_Msk              (0x3FFUL << USB_COUNT5_RX_COUNT5_RX_Pos) /*!< 0x000003FF */
#define USB_COUNT5_RX_COUNT5_RX                  USB_COUNT5_RX_COUNT5_RX_Msk   /*!< Reception Byte Count */

#define USB_COUNT5_RX_NUM_BLOCK_Pos              (10U)
#define USB_COUNT5_RX_NUM_BLOCK_Msk              (0x1FUL << USB_COUNT5_RX_NUM_BLOCK_Pos) /*!< 0x00007C00 */
#define USB_COUNT5_RX_NUM_BLOCK                  USB_COUNT5_RX_NUM_BLOCK_Msk   /*!< NUM_BLOCK[4:0] bits (Number of blocks) */
#define USB_COUNT5_RX_NUM_BLOCK_0                (0x01UL << USB_COUNT5_RX_NUM_BLOCK_Pos) /*!< 0x00000400 */
#define USB_COUNT5_RX_NUM_BLOCK_1                (0x02UL << USB_COUNT5_RX_NUM_BLOCK_Pos) /*!< 0x00000800 */
#define USB_COUNT5_RX_NUM_BLOCK_2                (0x04UL << USB_COUNT5_RX_NUM_BLOCK_Pos) /*!< 0x00001000 */
#define USB_COUNT5_RX_NUM_BLOCK_3                (0x08UL << USB_COUNT5_RX_NUM_BLOCK_Pos) /*!< 0x00002000 */
#define USB_COUNT5_RX_NUM_BLOCK_4                (0x10UL << USB_COUNT5_RX_NUM_BLOCK_Pos) /*!< 0x00004000 */

#define USB_COUNT5_RX_BLSIZE_Pos                 (15U)
#define USB_COUNT5_RX_BLSIZE_Msk                 (0x1UL << USB_COUNT5_RX_BLSIZE_Pos) /*!< 0x00008000 */
#define USB_COUNT5_RX_BLSIZE                     USB_COUNT5_RX_BLSIZE_Msk      /*!< BLock SIZE */

/*****************  Bit definition for USB_COUNT6_RX register  ****************/
#define USB_COUNT6_RX_COUNT6_RX_Pos              (0U)
#define USB_COUNT6_RX_COUNT6_RX_Msk              (0x3FFUL << USB_COUNT6_RX_COUNT6_RX_Pos) /*!< 0x000003FF */
#define USB_COUNT6_RX_COUNT6_RX                  USB_COUNT6_RX_COUNT6_RX_Msk   /*!< Reception Byte Count */

#define USB_COUNT6_RX_NUM_BLOCK_Pos              (10U)
#define USB_COUNT6_RX_NUM_BLOCK_Msk              (0x1FUL << USB_COUNT6_RX_NUM_BLOCK_Pos) /*!< 0x00007C00 */
#define USB_COUNT6_RX_NUM_BLOCK                  USB_COUNT6_RX_NUM_BLOCK_Msk   /*!< NUM_BLOCK[4:0] bits (Number of blocks) */
#define USB_COUNT6_RX_NUM_BLOCK_0                (0x01UL << USB_COUNT6_RX_NUM_BLOCK_Pos) /*!< 0x00000400 */
#define USB_COUNT6_RX_NUM_BLOCK_1                (0x02UL << USB_COUNT6_RX_NUM_BLOCK_Pos) /*!< 0x00000800 */
#define USB_COUNT6_RX_NUM_BLOCK_2                (0x04UL << USB_COUNT6_RX_NUM_BLOCK_Pos) /*!< 0x00001000 */
#define USB_COUNT6_RX_NUM_BLOCK_3                (0x08UL << USB_COUNT6_RX_NUM_BLOCK_Pos) /*!< 0x00002000 */
#define USB_COUNT6_RX_NUM_BLOCK_4                (0x10UL << USB_COUNT6_RX_NUM_BLOCK_Pos) /*!< 0x00004000 */

#define USB_COUNT6_RX_BLSIZE_Pos                 (15U)
#define USB_COUNT6_RX_BLSIZE_Msk                 (0x1UL << USB_COUNT6_RX_BLSIZE_Pos) /*!< 0x00008000 */
#define USB_COUNT6_RX_BLSIZE                     USB_COUNT6_RX_BLSIZE_Msk      /*!< BLock SIZE */

/*****************  Bit definition for USB_COUNT7_RX register  ****************/
#define USB_COUNT7_RX_COUNT7_RX_Pos              (0U)
#define USB_COUNT7_RX_COUNT7_RX_Msk              (0x3FFUL << USB_COUNT7_RX_COUNT7_RX_Pos) /*!< 0x000003FF */
#define USB_COUNT7_RX_COUNT7_RX                  USB_COUNT7_RX_COUNT7_RX_Msk   /*!< Reception Byte Count */

#define USB_COUNT7_RX_NUM_BLOCK_Pos              (10U)
#define USB_COUNT7_RX_NUM_BLOCK_Msk              (0x1FUL << USB_COUNT7_RX_NUM_BLOCK_Pos) /*!< 0x00007C00 */
#define USB_COUNT7_RX_NUM_BLOCK                  USB_COUNT7_RX_NUM_BLOCK_Msk   /*!< NUM_BLOCK[4:0] bits (Number of blocks) */
#define USB_COUNT7_RX_NUM_BLOCK_0                (0x01UL << USB_COUNT7_RX_NUM_BLOCK_Pos) /*!< 0x00000400 */
#define USB_COUNT7_RX_NUM_BLOCK_1                (0x02UL << USB_COUNT7_RX_NUM_BLOCK_Pos) /*!< 0x00000800 */
#define USB_COUNT7_RX_NUM_BLOCK_2                (0x04UL << USB_COUNT7_RX_NUM_BLOCK_Pos) /*!< 0x00001000 */
#define USB_COUNT7_RX_NUM_BLOCK_3                (0x08UL << USB_COUNT7_RX_NUM_BLOCK_Pos) /*!< 0x00002000 */
#define USB_COUNT7_RX_NUM_BLOCK_4                (0x10UL << USB_COUNT7_RX_NUM_BLOCK_Pos) /*!< 0x00004000 */

#define USB_COUNT7_RX_BLSIZE_Pos                 (15U)
#define USB_COUNT7_RX_BLSIZE_Msk                 (0x1UL << USB_COUNT7_RX_BLSIZE_Pos) /*!< 0x00008000 */
#define USB_COUNT7_RX_BLSIZE                     USB_COUNT7_RX_BLSIZE_Msk      /*!< BLock SIZE */

/*----------------------------------------------------------------------------*/

/****************  Bit definition for USB_COUNT0_RX_0 register  ***************/
#define USB_COUNT0_RX_0_COUNT0_RX_0              (0x000003FFUL)          /*!< Reception Byte Count (low) */

#define USB_COUNT0_RX_0_NUM_BLOCK_0              (0x00007C00UL)          /*!< NUM_BLOCK_0[4:0] bits (Number of blocks) (low) */
#define USB_COUNT0_RX_0_NUM_BLOCK_0_0            (0x00000400UL)          /*!< Bit 0 */
#define USB_COUNT0_RX_0_NUM_BLOCK_0_1            (0x00000800UL)          /*!< Bit 1 */
#define USB_COUNT0_RX_0_NUM_BLOCK_0_2            (0x00001000UL)          /*!< Bit 2 */
#define USB_COUNT0_RX_0_NUM_BLOCK_0_3            (0x00002000UL)          /*!< Bit 3 */
#define USB_COUNT0_RX_0_NUM_BLOCK_0_4            (0x00004000UL)          /*!< Bit 4 */

#define USB_COUNT0_RX_0_BLSIZE_0                 (0x00008000UL)          /*!< BLock SIZE (low) */

/****************  Bit definition for USB_COUNT0_RX_1 register  ***************/
#define USB_COUNT0_RX_1_COUNT0_RX_1              (0x03FF0000UL)          /*!< Reception Byte Count (high) */

#define USB_COUNT0_RX_1_NUM_BLOCK_1              (0x7C000000UL)          /*!< NUM_BLOCK_1[4:0] bits (Number of blocks) (high) */
#define USB_COUNT0_RX_1_NUM_BLOCK_1_0            (0x04000000UL)          /*!< Bit 1 */
#define USB_COUNT0_RX_1_NUM_BLOCK_1_1            (0x08000000UL)          /*!< Bit 1 */
#define USB_COUNT0_RX_1_NUM_BLOCK_1_2            (0x10000000UL)          /*!< Bit 2 */
#define USB_COUNT0_RX_1_NUM_BLOCK_1_3            (0x20000000UL)          /*!< Bit 3 */
#define USB_COUNT0_RX_1_NUM_BLOCK_1_4            (0x40000000UL)          /*!< Bit 4 */

#define USB_COUNT0_RX_1_BLSIZE_1                 (0x80000000UL)          /*!< BLock SIZE (high) */

/****************  Bit definition for USB_COUNT1_RX_0 register  ***************/
#define USB_COUNT1_RX_0_COUNT1_RX_0              (0x000003FFUL)          /*!< Reception Byte Count (low) */

#define USB_COUNT1_RX_0_NUM_BLOCK_0              (0x00007C00UL)          /*!< NUM_BLOCK_0[4:0] bits (Number of blocks) (low) */
#define USB_COUNT1_RX_0_NUM_BLOCK_0_0            (0x00000400UL)          /*!< Bit 0 */
#define USB_COUNT1_RX_0_NUM_BLOCK_0_1            (0x00000800UL)          /*!< Bit 1 */
#define USB_COUNT1_RX_0_NUM_BLOCK_0_2            (0x00001000UL)          /*!< Bit 2 */
#define USB_COUNT1_RX_0_NUM_BLOCK_0_3            (0x00002000UL)          /*!< Bit 3 */
#define USB_COUNT1_RX_0_NUM_BLOCK_0_4            (0x00004000UL)          /*!< Bit 4 */

#define USB_COUNT1_RX_0_BLSIZE_0                 (0x00008000UL)          /*!< BLock SIZE (low) */

/****************  Bit definition for USB_COUNT1_RX_1 register  ***************/
#define USB_COUNT1_RX_1_COUNT1_RX_1              (0x03FF0000UL)          /*!< Reception Byte Count (high) */

#define USB_COUNT1_RX_1_NUM_BLOCK_1              (0x7C000000UL)          /*!< NUM_BLOCK_1[4:0] bits (Number of blocks) (high) */
#define USB_COUNT1_RX_1_NUM_BLOCK_1_0            (0x04000000UL)          /*!< Bit 0 */
#define USB_COUNT1_RX_1_NUM_BLOCK_1_1            (0x08000000UL)          /*!< Bit 1 */
#define USB_COUNT1_RX_1_NUM_BLOCK_1_2            (0x10000000UL)          /*!< Bit 2 */
#define USB_COUNT1_RX_1_NUM_BLOCK_1_3            (0x20000000UL)          /*!< Bit 3 */
#define USB_COUNT1_RX_1_NUM_BLOCK_1_4            (0x40000000UL)          /*!< Bit 4 */

#define USB_COUNT1_RX_1_BLSIZE_1                 (0x80000000UL)          /*!< BLock SIZE (high) */

/****************  Bit definition for USB_COUNT2_RX_0 register  ***************/
#define USB_COUNT2_RX_0_COUNT2_RX_0              (0x000003FFUL)          /*!< Reception Byte Count (low) */

#define USB_COUNT2_RX_0_NUM_BLOCK_0              (0x00007C00UL)          /*!< NUM_BLOCK_0[4:0] bits (Number of blocks) (low) */
#define USB_COUNT2_RX_0_NUM_BLOCK_0_0            (0x00000400UL)          /*!< Bit 0 */
#define USB_COUNT2_RX_0_NUM_BLOCK_0_1            (0x00000800UL)          /*!< Bit 1 */
#define USB_COUNT2_RX_0_NUM_BLOCK_0_2            (0x00001000UL)          /*!< Bit 2 */
#define USB_COUNT2_RX_0_NUM_BLOCK_0_3            (0x00002000UL)          /*!< Bit 3 */
#define USB_COUNT2_RX_0_NUM_BLOCK_0_4            (0x00004000UL)          /*!< Bit 4 */

#define USB_COUNT2_RX_0_BLSIZE_0                 (0x00008000UL)          /*!< BLock SIZE (low) */

/****************  Bit definition for USB_COUNT2_RX_1 register  ***************/
#define USB_COUNT2_RX_1_COUNT2_RX_1              (0x03FF0000UL)          /*!< Reception Byte Count (high) */

#define USB_COUNT2_RX_1_NUM_BLOCK_1              (0x7C000000UL)          /*!< NUM_BLOCK_1[4:0] bits (Number of blocks) (high) */
#define USB_COUNT2_RX_1_NUM_BLOCK_1_0            (0x04000000UL)          /*!< Bit 0 */
#define USB_COUNT2_RX_1_NUM_BLOCK_1_1            (0x08000000UL)          /*!< Bit 1 */
#define USB_COUNT2_RX_1_NUM_BLOCK_1_2            (0x10000000UL)          /*!< Bit 2 */
#define USB_COUNT2_RX_1_NUM_BLOCK_1_3            (0x20000000UL)          /*!< Bit 3 */
#define USB_COUNT2_RX_1_NUM_BLOCK_1_4            (0x40000000UL)          /*!< Bit 4 */

#define USB_COUNT2_RX_1_BLSIZE_1                 (0x80000000UL)          /*!< BLock SIZE (high) */

/****************  Bit definition for USB_COUNT3_RX_0 register  ***************/
#define USB_COUNT3_RX_0_COUNT3_RX_0              (0x000003FFUL)          /*!< Reception Byte Count (low) */

#define USB_COUNT3_RX_0_NUM_BLOCK_0              (0x00007C00UL)          /*!< NUM_BLOCK_0[4:0] bits (Number of blocks) (low) */
#define USB_COUNT3_RX_0_NUM_BLOCK_0_0            (0x00000400UL)          /*!< Bit 0 */
#define USB_COUNT3_RX_0_NUM_BLOCK_0_1            (0x00000800UL)          /*!< Bit 1 */
#define USB_COUNT3_RX_0_NUM_BLOCK_0_2            (0x00001000UL)          /*!< Bit 2 */
#define USB_COUNT3_RX_0_NUM_BLOCK_0_3            (0x00002000UL)          /*!< Bit 3 */
#define USB_COUNT3_RX_0_NUM_BLOCK_0_4            (0x00004000UL)          /*!< Bit 4 */

#define USB_COUNT3_RX_0_BLSIZE_0                 (0x00008000UL)          /*!< BLock SIZE (low) */

/****************  Bit definition for USB_COUNT3_RX_1 register  ***************/
#define USB_COUNT3_RX_1_COUNT3_RX_1              (0x03FF0000UL)          /*!< Reception Byte Count (high) */

#define USB_COUNT3_RX_1_NUM_BLOCK_1              (0x7C000000UL)          /*!< NUM_BLOCK_1[4:0] bits (Number of blocks) (high) */
#define USB_COUNT3_RX_1_NUM_BLOCK_1_0            (0x04000000UL)          /*!< Bit 0 */
#define USB_COUNT3_RX_1_NUM_BLOCK_1_1            (0x08000000UL)          /*!< Bit 1 */
#define USB_COUNT3_RX_1_NUM_BLOCK_1_2            (0x10000000UL)          /*!< Bit 2 */
#define USB_COUNT3_RX_1_NUM_BLOCK_1_3            (0x20000000UL)          /*!< Bit 3 */
#define USB_COUNT3_RX_1_NUM_BLOCK_1_4            (0x40000000UL)          /*!< Bit 4 */

#define USB_COUNT3_RX_1_BLSIZE_1                 (0x80000000UL)          /*!< BLock SIZE (high) */

/****************  Bit definition for USB_COUNT4_RX_0 register  ***************/
#define USB_COUNT4_RX_0_COUNT4_RX_0              (0x000003FFUL)          /*!< Reception Byte Count (low) */

#define USB_COUNT4_RX_0_NUM_BLOCK_0              (0x00007C00UL)          /*!< NUM_BLOCK_0[4:0] bits (Number of blocks) (low) */
#define USB_COUNT4_RX_0_NUM_BLOCK_0_0            (0x00000400UL)          /*!< Bit 0 */
#define USB_COUNT4_RX_0_NUM_BLOCK_0_1            (0x00000800UL)          /*!< Bit 1 */
#define USB_COUNT4_RX_0_NUM_BLOCK_0_2            (0x00001000UL)          /*!< Bit 2 */
#define USB_COUNT4_RX_0_NUM_BLOCK_0_3            (0x00002000UL)          /*!< Bit 3 */
#define USB_COUNT4_RX_0_NUM_BLOCK_0_4            (0x00004000UL)          /*!< Bit 4 */

#define USB_COUNT4_RX_0_BLSIZE_0                 (0x00008000UL)          /*!< BLock SIZE (low) */

/****************  Bit definition for USB_COUNT4_RX_1 register  ***************/
#define USB_COUNT4_RX_1_COUNT4_RX_1              (0x03FF0000UL)          /*!< Reception Byte Count (high) */

#define USB_COUNT4_RX_1_NUM_BLOCK_1              (0x7C000000UL)          /*!< NUM_BLOCK_1[4:0] bits (Number of blocks) (high) */
#define USB_COUNT4_RX_1_NUM_BLOCK_1_0            (0x04000000UL)          /*!< Bit 0 */
#define USB_COUNT4_RX_1_NUM_BLOCK_1_1            (0x08000000UL)          /*!< Bit 1 */
#define USB_COUNT4_RX_1_NUM_BLOCK_1_2            (0x10000000UL)          /*!< Bit 2 */
#define USB_COUNT4_RX_1_NUM_BLOCK_1_3            (0x20000000UL)          /*!< Bit 3 */
#define USB_COUNT4_RX_1_NUM_BLOCK_1_4            (0x40000000UL)          /*!< Bit 4 */

#define USB_COUNT4_RX_1_BLSIZE_1                 (0x80000000UL)          /*!< BLock SIZE (high) */

/****************  Bit definition for USB_COUNT5_RX_0 register  ***************/
#define USB_COUNT5_RX_0_COUNT5_RX_0              (0x000003FFUL)          /*!< Reception Byte Count (low) */

#define USB_COUNT5_RX_0_NUM_BLOCK_0              (0x00007C00UL)          /*!< NUM_BLOCK_0[4:0] bits (Number of blocks) (low) */
#define USB_COUNT5_RX_0_NUM_BLOCK_0_0            (0x00000400UL)          /*!< Bit 0 */
#define USB_COUNT5_RX_0_NUM_BLOCK_0_1            (0x00000800UL)          /*!< Bit 1 */
#define USB_COUNT5_RX_0_NUM_BLOCK_0_2            (0x00001000UL)          /*!< Bit 2 */
#define USB_COUNT5_RX_0_NUM_BLOCK_0_3            (0x00002000UL)          /*!< Bit 3 */
#define USB_COUNT5_RX_0_NUM_BLOCK_0_4            (0x00004000UL)          /*!< Bit 4 */

#define USB_COUNT5_RX_0_BLSIZE_0                 (0x00008000UL)          /*!< BLock SIZE (low) */

/****************  Bit definition for USB_COUNT5_RX_1 register  ***************/
#define USB_COUNT5_RX_1_COUNT5_RX_1              (0x03FF0000UL)          /*!< Reception Byte Count (high) */

#define USB_COUNT5_RX_1_NUM_BLOCK_1              (0x7C000000UL)          /*!< NUM_BLOCK_1[4:0] bits (Number of blocks) (high) */
#define USB_COUNT5_RX_1_NUM_BLOCK_1_0            (0x04000000UL)          /*!< Bit 0 */
#define USB_COUNT5_RX_1_NUM_BLOCK_1_1            (0x08000000UL)          /*!< Bit 1 */
#define USB_COUNT5_RX_1_NUM_BLOCK_1_2            (0x10000000UL)          /*!< Bit 2 */
#define USB_COUNT5_RX_1_NUM_BLOCK_1_3            (0x20000000UL)          /*!< Bit 3 */
#define USB_COUNT5_RX_1_NUM_BLOCK_1_4            (0x40000000UL)          /*!< Bit 4 */

#define USB_COUNT5_RX_1_BLSIZE_1                 (0x80000000UL)          /*!< BLock SIZE (high) */

/***************  Bit definition for USB_COUNT6_RX_0  register  ***************/
#define USB_COUNT6_RX_0_COUNT6_RX_0              (0x000003FFUL)          /*!< Reception Byte Count (low) */

#define USB_COUNT6_RX_0_NUM_BLOCK_0              (0x00007C00UL)          /*!< NUM_BLOCK_0[4:0] bits (Number of blocks) (low) */
#define USB_COUNT6_RX_0_NUM_BLOCK_0_0            (0x00000400UL)          /*!< Bit 0 */
#define USB_COUNT6_RX_0_NUM_BLOCK_0_1            (0x00000800UL)          /*!< Bit 1 */
#define USB_COUNT6_RX_0_NUM_BLOCK_0_2            (0x00001000UL)          /*!< Bit 2 */
#define USB_COUNT6_RX_0_NUM_BLOCK_0_3            (0x00002000UL)          /*!< Bit 3 */
#define USB_COUNT6_RX_0_NUM_BLOCK_0_4            (0x00004000UL)          /*!< Bit 4 */

#define USB_COUNT6_RX_0_BLSIZE_0                 (0x00008000UL)          /*!< BLock SIZE (low) */

/****************  Bit definition for USB_COUNT6_RX_1 register  ***************/
#define USB_COUNT6_RX_1_COUNT6_RX_1              (0x03FF0000UL)          /*!< Reception Byte Count (high) */

#define USB_COUNT6_RX_1_NUM_BLOCK_1              (0x7C000000UL)          /*!< NUM_BLOCK_1[4:0] bits (Number of blocks) (high) */
#define USB_COUNT6_RX_1_NUM_BLOCK_1_0            (0x04000000UL)          /*!< Bit 0 */
#define USB_COUNT6_RX_1_NUM_BLOCK_1_1            (0x08000000UL)          /*!< Bit 1 */
#define USB_COUNT6_RX_1_NUM_BLOCK_1_2            (0x10000000UL)          /*!< Bit 2 */
#define USB_COUNT6_RX_1_NUM_BLOCK_1_3            (0x20000000UL)          /*!< Bit 3 */
#define USB_COUNT6_RX_1_NUM_BLOCK_1_4            (0x40000000UL)          /*!< Bit 4 */

#define USB_COUNT6_RX_1_BLSIZE_1                 (0x80000000UL)          /*!< BLock SIZE (high) */

/***************  Bit definition for USB_COUNT7_RX_0 register  ****************/
#define USB_COUNT7_RX_0_COUNT7_RX_0              (0x000003FFUL)          /*!< Reception Byte Count (low) */

#define USB_COUNT7_RX_0_NUM_BLOCK_0              (0x00007C00UL)          /*!< NUM_BLOCK_0[4:0] bits (Number of blocks) (low) */
#define USB_COUNT7_RX_0_NUM_BLOCK_0_0            (0x00000400UL)          /*!< Bit 0 */
#define USB_COUNT7_RX_0_NUM_BLOCK_0_1            (0x00000800UL)          /*!< Bit 1 */
#define USB_COUNT7_RX_0_NUM_BLOCK_0_2            (0x00001000UL)          /*!< Bit 2 */
#define USB_COUNT7_RX_0_NUM_BLOCK_0_3            (0x00002000UL)          /*!< Bit 3 */
#define USB_COUNT7_RX_0_NUM_BLOCK_0_4            (0x00004000UL)          /*!< Bit 4 */

#define USB_COUNT7_RX_0_BLSIZE_0                 (0x00008000UL)          /*!< BLock SIZE (low) */

/***************  Bit definition for USB_COUNT7_RX_1 register  ****************/
#define USB_COUNT7_RX_1_COUNT7_RX_1              (0x03FF0000UL)          /*!< Reception Byte Count (high) */

#define USB_COUNT7_RX_1_NUM_BLOCK_1              (0x7C000000UL)          /*!< NUM_BLOCK_1[4:0] bits (Number of blocks) (high) */
#define USB_COUNT7_RX_1_NUM_BLOCK_1_0            (0x04000000UL)          /*!< Bit 0 */
#define USB_COUNT7_RX_1_NUM_BLOCK_1_1            (0x08000000UL)          /*!< Bit 1 */
#define USB_COUNT7_RX_1_NUM_BLOCK_1_2            (0x10000000UL)          /*!< Bit 2 */
#define USB_COUNT7_RX_1_NUM_BLOCK_1_3            (0x20000000UL)          /*!< Bit 3 */
#define USB_COUNT7_RX_1_NUM_BLOCK_1_4            (0x40000000UL)          /*!< Bit 4 */

#define USB_COUNT7_RX_1_BLSIZE_1                 (0x80000000UL)          /*!< BLock SIZE (high) */


/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup Exported_macros
  * @{
  */

/******************************* ADC Instances ********************************/
#define IS_ADC_ALL_INSTANCE(INSTANCE) (((INSTANCE) == ADC1) || \
                                       ((INSTANCE) == ADC2))

#define IS_ADC_MULTIMODE_MASTER_INSTANCE(INSTANCE) ((INSTANCE) == ADC1)

#define IS_ADC_COMMON_INSTANCE(INSTANCE) ((INSTANCE) == ADC12_COMMON)

/******************************* AES Instances ********************************/
#define IS_AES_ALL_INSTANCE(INSTANCE) ((INSTANCE) == AES)

/******************************** CAN Instances ******************************/

/******************************** COMP Instances ******************************/
#define IS_COMP_ALL_INSTANCE(INSTANCE) ((INSTANCE) == COMP1)

/******************************* CRC Instances ********************************/
#define IS_CRC_ALL_INSTANCE(INSTANCE) ((INSTANCE) == CRC)


/******************************** DMA Instances *******************************/
#define IS_DMA_ALL_INSTANCE(INSTANCE) (((INSTANCE) == DMA1_Channel1) || \
                                       ((INSTANCE) == DMA1_Channel2) || \
                                       ((INSTANCE) == DMA1_Channel3) || \
                                       ((INSTANCE) == DMA1_Channel4) || \
                                       ((INSTANCE) == DMA1_Channel5) || \
                                       ((INSTANCE) == DMA1_Channel6) || \
                                       ((INSTANCE) == DMA1_Channel7) || \
                                       ((INSTANCE) == DMA2_Channel1) || \
                                       ((INSTANCE) == DMA2_Channel2) || \
                                       ((INSTANCE) == DMA2_Channel3) || \
                                       ((INSTANCE) == DMA2_Channel4) || \
                                       ((INSTANCE) == DMA2_Channel5) || \
                                       ((INSTANCE) == DMA2_Channel6) || \
                                       ((INSTANCE) == DMA2_Channel7))

/******************************* GPIO Instances *******************************/
#define IS_GPIO_ALL_INSTANCE(INSTANCE) (((INSTANCE) == GPIOA) || \
                                        ((INSTANCE) == GPIOB) || \
                                        ((INSTANCE) == GPIOC) || \
                                        ((INSTANCE) == GPIOD) || \
                                        ((INSTANCE) == GPIOH))

/******************************* GPIO AF Instances ****************************/
/* On L4, all GPIO Bank support AF */
#define IS_GPIO_AF_INSTANCE(INSTANCE)   IS_GPIO_ALL_INSTANCE(INSTANCE)

/**************************** GPIO Lock Instances *****************************/
/* On L4, all GPIO Bank support the Lock mechanism */
#define IS_GPIO_LOCK_INSTANCE(INSTANCE) IS_GPIO_ALL_INSTANCE(INSTANCE)

/******************************** I2C Instances *******************************/
#define IS_I2C_ALL_INSTANCE(INSTANCE) (((INSTANCE) == I2C1) || \
                                       ((INSTANCE) == I2C2) || \
                                       ((INSTANCE) == I2C3))

/****************** I2C Instances : wakeup capability from stop modes *********/
#define IS_I2C_WAKEUP_FROMSTOP_INSTANCE(INSTANCE) IS_I2C_ALL_INSTANCE(INSTANCE)

/****************************** OPAMP Instances *******************************/
#define IS_OPAMP_ALL_INSTANCE(INSTANCE) ((INSTANCE) == OPAMP1)

#define IS_OPAMP_COMMON_INSTANCE(COMMON_INSTANCE) ((COMMON_INSTANCE) == OPAMP1_COMMON)

/******************************* QSPI Instances *******************************/
#define IS_QSPI_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == QUADSPI)

/******************************* RNG Instances ********************************/
#define IS_RNG_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == RNG)

/****************************** RTC Instances *********************************/
#define IS_RTC_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == RTC)

/******************************** SAI Instances *******************************/

/****************************** SMBUS Instances *******************************/
#define IS_SMBUS_ALL_INSTANCE(INSTANCE) (((INSTANCE) == I2C1) || \
                                         ((INSTANCE) == I2C2) || \
                                         ((INSTANCE) == I2C3))

/******************************** SPI Instances *******************************/
#define IS_SPI_ALL_INSTANCE(INSTANCE) (((INSTANCE) == SPI1) || \
                                       ((INSTANCE) == SPI2))

/****************** LPTIM Instances : All supported instances *****************/
#define IS_LPTIM_INSTANCE(INSTANCE)     (((INSTANCE) == LPTIM1) || \
                                         ((INSTANCE) == LPTIM2))

/****************** LPTIM Instances : supporting the encoder mode *************/
#define IS_LPTIM_ENCODER_INTERFACE_INSTANCE(INSTANCE) ((INSTANCE) == LPTIM1)

/****************** TIM Instances : All supported instances *******************/
#define IS_TIM_INSTANCE(INSTANCE)       (((INSTANCE) == TIM1)   || \
                                         ((INSTANCE) == TIM2)   || \
                                         ((INSTANCE) == TIM6)   || \
                                         ((INSTANCE) == TIM15)  || \
                                         ((INSTANCE) == TIM16))

/****************** TIM Instances : supporting 32 bits counter ****************/
#define IS_TIM_32B_COUNTER_INSTANCE(INSTANCE) ((INSTANCE) == TIM2)

/****************** TIM Instances : supporting the break function *************/
#define IS_TIM_BREAK_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)    || \
                                            ((INSTANCE) == TIM15)   || \
                                            ((INSTANCE) == TIM16))

/************** TIM Instances : supporting Break source selection *************/
#define IS_TIM_BREAKSOURCE_INSTANCE(INSTANCE) (((INSTANCE) == TIM1)   || \
                                               ((INSTANCE) == TIM15)  || \
                                               ((INSTANCE) == TIM16))

/****************** TIM Instances : supporting 2 break inputs *****************/
#define IS_TIM_BKIN2_INSTANCE(INSTANCE)    ((INSTANCE) == TIM1)

/************* TIM Instances : at least 1 capture/compare channel *************/
#define IS_TIM_CC1_INSTANCE(INSTANCE)   (((INSTANCE) == TIM1)   || \
                                         ((INSTANCE) == TIM2)   || \
                                         ((INSTANCE) == TIM15)  || \
                                         ((INSTANCE) == TIM16))

/************ TIM Instances : at least 2 capture/compare channels *************/
#define IS_TIM_CC2_INSTANCE(INSTANCE)   (((INSTANCE) == TIM1)   || \
                                         ((INSTANCE) == TIM2)   || \
                                         ((INSTANCE) == TIM15))

/************ TIM Instances : at least 3 capture/compare channels *************/
#define IS_TIM_CC3_INSTANCE(INSTANCE)   (((INSTANCE) == TIM1)   || \
                                         ((INSTANCE) == TIM2))

/************ TIM Instances : at least 4 capture/compare channels *************/
#define IS_TIM_CC4_INSTANCE(INSTANCE)   (((INSTANCE) == TIM1)   || \
                                         ((INSTANCE) == TIM2))

/****************** TIM Instances : at least 5 capture/compare channels *******/
#define IS_TIM_CC5_INSTANCE(INSTANCE)   ((INSTANCE) == TIM1)

/****************** TIM Instances : at least 6 capture/compare channels *******/
#define IS_TIM_CC6_INSTANCE(INSTANCE)   ((INSTANCE) == TIM1)

/************ TIM Instances : DMA requests generation (TIMx_DIER.COMDE) *******/
#define IS_TIM_CCDMA_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)   || \
                                            ((INSTANCE) == TIM15)  || \
                                            ((INSTANCE) == TIM16))

/****************** TIM Instances : DMA requests generation (TIMx_DIER.UDE) ***/
#define IS_TIM_DMA_INSTANCE(INSTANCE)      (((INSTANCE) == TIM1)   || \
                                            ((INSTANCE) == TIM2)   || \
                                            ((INSTANCE) == TIM6)   || \
                                            ((INSTANCE) == TIM15)  || \
                                            ((INSTANCE) == TIM16))

/************ TIM Instances : DMA requests generation (TIMx_DIER.CCxDE) *******/
#define IS_TIM_DMA_CC_INSTANCE(INSTANCE)   (((INSTANCE) == TIM1)   || \
                                            ((INSTANCE) == TIM2)   || \
                                            ((INSTANCE) == TIM15)  || \
                                            ((INSTANCE) == TIM16))

/******************** TIM Instances : DMA burst feature ***********************/
#define IS_TIM_DMABURST_INSTANCE(INSTANCE) (((INSTANCE) == TIM1)   || \
                                            ((INSTANCE) == TIM2)   || \
                                            ((INSTANCE) == TIM15)  || \
                                            ((INSTANCE) == TIM16))

/******************* TIM Instances : output(s) available **********************/
#define IS_TIM_CCX_INSTANCE(INSTANCE, CHANNEL) \
    ((((INSTANCE) == TIM1) &&                  \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4) ||          \
      ((CHANNEL) == TIM_CHANNEL_5) ||          \
      ((CHANNEL) == TIM_CHANNEL_6)))           \
     ||                                        \
     (((INSTANCE) == TIM2) &&                  \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
     ||                                        \
     (((INSTANCE) == TIM15) &&                 \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2)))           \
     ||                                        \
     (((INSTANCE) == TIM16) &&                 \
     (((CHANNEL) == TIM_CHANNEL_1))))

/****************** TIM Instances : supporting complementary output(s) ********/
#define IS_TIM_CCXN_INSTANCE(INSTANCE, CHANNEL) \
   ((((INSTANCE) == TIM1) &&                    \
     (((CHANNEL) == TIM_CHANNEL_1) ||           \
      ((CHANNEL) == TIM_CHANNEL_2) ||           \
      ((CHANNEL) == TIM_CHANNEL_3)))            \
    ||                                          \
    (((INSTANCE) == TIM15) &&                   \
     ((CHANNEL) == TIM_CHANNEL_1))              \
    ||                                          \
    (((INSTANCE) == TIM16) &&                   \
     ((CHANNEL) == TIM_CHANNEL_1)))

/****************** TIM Instances : supporting clock division *****************/
#define IS_TIM_CLOCK_DIVISION_INSTANCE(INSTANCE)   (((INSTANCE) == TIM1)    || \
                                                    ((INSTANCE) == TIM2)    || \
                                                    ((INSTANCE) == TIM15)   || \
                                                    ((INSTANCE) == TIM16))

/****** TIM Instances : supporting external clock mode 1 for ETRF input *******/
#define IS_TIM_CLOCKSOURCE_ETRMODE1_INSTANCE(INSTANCE) (((INSTANCE) == TIM1) || \
                                                        ((INSTANCE) == TIM2) || \
                                                        ((INSTANCE) == TIM15))

/****** TIM Instances : supporting external clock mode 2 for ETRF input *******/
#define IS_TIM_CLOCKSOURCE_ETRMODE2_INSTANCE(INSTANCE) (((INSTANCE) == TIM1) || \
                                                        ((INSTANCE) == TIM2))

/****************** TIM Instances : supporting external clock mode 1 for TIX inputs*/
#define IS_TIM_CLOCKSOURCE_TIX_INSTANCE(INSTANCE)      (((INSTANCE) == TIM1) || \
                                                        ((INSTANCE) == TIM2) || \
                                                        ((INSTANCE) == TIM15))

/****************** TIM Instances : supporting internal trigger inputs(ITRX) *******/
#define IS_TIM_CLOCKSOURCE_ITRX_INSTANCE(INSTANCE)     (((INSTANCE) == TIM1) || \
                                                        ((INSTANCE) == TIM2) || \
                                                        ((INSTANCE) == TIM15))

/****************** TIM Instances : supporting combined 3-phase PWM mode ******/
#define IS_TIM_COMBINED3PHASEPWM_INSTANCE(INSTANCE) ((INSTANCE) == TIM1)

/****************** TIM Instances : supporting commutation event generation ***/
#define IS_TIM_COMMUTATION_EVENT_INSTANCE(INSTANCE) (((INSTANCE) == TIM1)   || \
                                                     ((INSTANCE) == TIM15)  || \
                                                     ((INSTANCE) == TIM16))

/****************** TIM Instances : supporting counting mode selection ********/
#define IS_TIM_COUNTER_MODE_SELECT_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1) || \
                                                        ((INSTANCE) == TIM2))

/****************** TIM Instances : supporting encoder interface **************/
#define IS_TIM_ENCODER_INTERFACE_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1)  || \
                                                      ((INSTANCE) == TIM2))

/****************** TIM Instances : supporting Hall sensor interface **********/
#define IS_TIM_HALL_SENSOR_INTERFACE_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1) || \
                                                          ((INSTANCE) == TIM2))

/**************** TIM Instances : external trigger input available ************/
#define IS_TIM_ETR_INSTANCE(INSTANCE)      (((INSTANCE) == TIM1)  || \
                                            ((INSTANCE) == TIM2))

/************* TIM Instances : supporting ETR source selection ***************/
#define IS_TIM_ETRSEL_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)  || \
                                             ((INSTANCE) == TIM2))

/****** TIM Instances : Master mode available (TIMx_CR2.MMS available )********/
#define IS_TIM_MASTER_INSTANCE(INSTANCE)   (((INSTANCE) == TIM1)  || \
                                            ((INSTANCE) == TIM2)  || \
                                            ((INSTANCE) == TIM6)  || \
                                            ((INSTANCE) == TIM15))

/*********** TIM Instances : Slave mode available (TIMx_SMCR available )*******/
#define IS_TIM_SLAVE_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)  || \
                                            ((INSTANCE) == TIM2)  || \
                                            ((INSTANCE) == TIM15))

/****************** TIM Instances : supporting OCxREF clear *******************/
#define IS_TIM_OCXREF_CLEAR_INSTANCE(INSTANCE)        (((INSTANCE) == TIM1) || \
                                                       ((INSTANCE) == TIM2))

/****************** TIM Instances : remapping capability **********************/
#define IS_TIM_REMAP_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)  || \
                                            ((INSTANCE) == TIM2)  || \
                                            ((INSTANCE) == TIM15) || \
                                            ((INSTANCE) == TIM16))

/****************** TIM Instances : supporting repetition counter *************/
#define IS_TIM_REPETITION_COUNTER_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1)  || \
                                                       ((INSTANCE) == TIM15) || \
                                                       ((INSTANCE) == TIM16))

/****************** TIM Instances : supporting synchronization ****************/
#define IS_TIM_SYNCHRO_INSTANCE(INSTANCE)  IS_TIM_MASTER_INSTANCE(INSTANCE)

/****************** TIM Instances : supporting ADC triggering through TRGO2 ***/
#define IS_TIM_TRGO2_INSTANCE(INSTANCE)    ((INSTANCE) == TIM1)

/******************* TIM Instances : Timer input XOR function *****************/
#define IS_TIM_XOR_INSTANCE(INSTANCE)      (((INSTANCE) == TIM1)   || \
                                            ((INSTANCE) == TIM2)   || \
                                            ((INSTANCE) == TIM15))

/****************** TIM Instances : Advanced timer instances *******************/
#define IS_TIM_ADVANCED_INSTANCE(INSTANCE)    ((INSTANCE) == TIM1)

/****************************** TSC Instances *********************************/
#define IS_TSC_ALL_INSTANCE(INSTANCE) ((INSTANCE) == TSC)

/******************** USART Instances : Synchronous mode **********************/
#define IS_USART_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                     ((INSTANCE) == USART2) || \
                                     ((INSTANCE) == USART3))

/******************** UART Instances : Asynchronous mode **********************/
#define IS_UART_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                    ((INSTANCE) == USART2) || \
                                    ((INSTANCE) == USART3))

/****************** UART Instances : Auto Baud Rate detection ****************/
#define IS_USART_AUTOBAUDRATE_DETECTION_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                                            ((INSTANCE) == USART2) || \
                                                            ((INSTANCE) == USART3))

/****************** UART Instances : Driver Enable *****************/
#define IS_UART_DRIVER_ENABLE_INSTANCE(INSTANCE)     (((INSTANCE) == USART1) || \
                                                      ((INSTANCE) == USART2) || \
                                                      ((INSTANCE) == USART3) || \
                                                      ((INSTANCE) == LPUART1))

/******************** UART Instances : Half-Duplex mode **********************/
#define IS_UART_HALFDUPLEX_INSTANCE(INSTANCE)   (((INSTANCE) == USART1) || \
                                                 ((INSTANCE) == USART2) || \
                                                 ((INSTANCE) == USART3) || \
                                                 ((INSTANCE) == LPUART1))

/****************** UART Instances : Hardware Flow control ********************/
#define IS_UART_HWFLOW_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                           ((INSTANCE) == USART2) || \
                                           ((INSTANCE) == USART3) || \
                                           ((INSTANCE) == LPUART1))

/******************** UART Instances : LIN mode **********************/
#define IS_UART_LIN_INSTANCE(INSTANCE)   (((INSTANCE) == USART1) || \
                                          ((INSTANCE) == USART2) || \
                                          ((INSTANCE) == USART3))

/******************** UART Instances : Wake-up from Stop mode **********************/
#define IS_UART_WAKEUP_FROMSTOP_INSTANCE(INSTANCE)   (((INSTANCE) == USART1) || \
                                                      ((INSTANCE) == USART2) || \
                                                      ((INSTANCE) == USART3) || \
                                                      ((INSTANCE) == LPUART1))

/*********************** UART Instances : IRDA mode ***************************/
#define IS_IRDA_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                    ((INSTANCE) == USART2) || \
                                    ((INSTANCE) == USART3))

/********************* USART Instances : Smard card mode ***********************/
#define IS_SMARTCARD_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                         ((INSTANCE) == USART2) || \
                                         ((INSTANCE) == USART3))

/******************** LPUART Instance *****************************************/
#define IS_LPUART_INSTANCE(INSTANCE)    ((INSTANCE) == LPUART1)

/****************************** IWDG Instances ********************************/
#define IS_IWDG_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == IWDG)

/****************************** WWDG Instances ********************************/
#define IS_WWDG_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == WWDG)

/******************************* USB Instances *******************************/
#define IS_USB_ALL_INSTANCE(INSTANCE) ((INSTANCE) == USB)
#define IS_PCD_ALL_INSTANCE           IS_USB_ALL_INSTANCE
/**
  * @}
  */


/******************************************************************************/
/*  For a painless codes migration between the STM32L4xx device product       */
/*  lines, the aliases defined below are put in place to overcome the         */
/*  differences in the interrupt handlers and IRQn definitions.               */
/*  No need to update developed interrupt code when moving across             */
/*  product lines within the same STM32L4 Family                              */
/******************************************************************************/

/* Aliases for __IRQn */
#define ADC1_IRQn                      ADC1_2_IRQn
#define HASH_RNG_IRQn                  RNG_IRQn
#define HASH_CRS_IRQn                  CRS_IRQn
#define TIM1_TRG_COM_TIM17_IRQn        TIM1_TRG_COM_IRQn
#define TIM6_DAC_IRQn                  TIM6_IRQn

/* Aliases for __IRQHandler */
#define ADC1_IRQHandler                ADC1_2_IRQHandler
#define HASH_RNG_IRQHandler            RNG_IRQHandler
#define HASH_CRS_IRQHandler            CRS_IRQHandler
#define TIM1_TRG_COM_TIM17_IRQHandler  TIM1_TRG_COM_IRQHandler
#define TIM6_DAC_IRQHandler            TIM6_IRQHandler

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32L422xx_H */

/**
  * @}
  */

  /**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
