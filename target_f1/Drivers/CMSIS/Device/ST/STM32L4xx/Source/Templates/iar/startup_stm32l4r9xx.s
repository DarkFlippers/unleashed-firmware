;********************************************************************************
;* File Name          : startup_stm32l4r9xx.s
;* Author             : MCD Application Team
;* Description        : STM32L4R9xx Ultra Low Power Devices vector
;*                      This module performs:
;*                      - Set the initial SP
;*                      - Set the initial PC == _iar_program_start,
;*                      - Set the vector table entries with the exceptions ISR
;*                        address.
;*                      - Branches to main in the C library (which eventually
;*                        calls main()).
;*                      After Reset the Cortex-M4 processor is in Thread mode,
;*                      priority is Privileged, and the Stack is set to Main.
;********************************************************************************
;*
;* <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
;* All rights reserved.</center></h2>
;*
;* This software component is licensed by ST under BSD 3-Clause license,
;* the "License"; You may not use this file except in compliance with the
;* License. You may obtain a copy of the License at:
;*                        opensource.org/licenses/BSD-3-Clause
;*
;*******************************************************************************
;
;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        EXTERN  SystemInit
        PUBLIC  __vector_table

        DATA
__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler             ; Reset Handler

        DCD     NMI_Handler               ; NMI Handler
        DCD     HardFault_Handler         ; Hard Fault Handler
        DCD     MemManage_Handler         ; MPU Fault Handler
        DCD     BusFault_Handler          ; Bus Fault Handler
        DCD     UsageFault_Handler        ; Usage Fault Handler
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     SVC_Handler               ; SVCall Handler
        DCD     DebugMon_Handler          ; Debug Monitor Handler
        DCD     0                         ; Reserved
        DCD     PendSV_Handler            ; PendSV Handler
        DCD     SysTick_Handler           ; SysTick Handler

         ; External Interrupts
        DCD     WWDG_IRQHandler                   ; Window WatchDog
        DCD     PVD_PVM_IRQHandler                ; PVD/PVM1/PVM2/PVM3/PVM4 through EXTI Line detection
        DCD     TAMP_STAMP_IRQHandler             ; Tamper and TimeStamps through the EXTI line
        DCD     RTC_WKUP_IRQHandler               ; RTC Wakeup through the EXTI line
        DCD     FLASH_IRQHandler                  ; FLASH
        DCD     RCC_IRQHandler                    ; RCC
        DCD     EXTI0_IRQHandler                  ; EXTI Line0
        DCD     EXTI1_IRQHandler                  ; EXTI Line1
        DCD     EXTI2_IRQHandler                  ; EXTI Line2
        DCD     EXTI3_IRQHandler                  ; EXTI Line3
        DCD     EXTI4_IRQHandler                  ; EXTI Line4
        DCD     DMA1_Channel1_IRQHandler          ; DMA1 Channel 1
        DCD     DMA1_Channel2_IRQHandler          ; DMA1 Channel 2
        DCD     DMA1_Channel3_IRQHandler          ; DMA1 Channel 3
        DCD     DMA1_Channel4_IRQHandler          ; DMA1 Channel 4
        DCD     DMA1_Channel5_IRQHandler          ; DMA1 Channel 5
        DCD     DMA1_Channel6_IRQHandler          ; DMA1 Channel 6
        DCD     DMA1_Channel7_IRQHandler          ; DMA1 Channel 7
        DCD     ADC1_IRQHandler                   ; ADC1
        DCD     CAN1_TX_IRQHandler                ; CAN1 TX
        DCD     CAN1_RX0_IRQHandler               ; CAN1 RX0
        DCD     CAN1_RX1_IRQHandler               ; CAN1 RX1
        DCD     CAN1_SCE_IRQHandler               ; CAN1 SCE
        DCD     EXTI9_5_IRQHandler                ; External Line[9:5]s
        DCD     TIM1_BRK_TIM15_IRQHandler         ; TIM1 Break and TIM15
        DCD     TIM1_UP_TIM16_IRQHandler          ; TIM1 Update and TIM16
        DCD     TIM1_TRG_COM_TIM17_IRQHandler     ; TIM1 Trigger and Commutation and TIM17
        DCD     TIM1_CC_IRQHandler                ; TIM1 Capture Compare
        DCD     TIM2_IRQHandler                   ; TIM2
        DCD     TIM3_IRQHandler                   ; TIM3
        DCD     TIM4_IRQHandler                   ; TIM4
        DCD     I2C1_EV_IRQHandler                ; I2C1 Event
        DCD     I2C1_ER_IRQHandler                ; I2C1 Error
        DCD     I2C2_EV_IRQHandler                ; I2C2 Event
        DCD     I2C2_ER_IRQHandler                ; I2C2 Error
        DCD     SPI1_IRQHandler                   ; SPI1
        DCD     SPI2_IRQHandler                   ; SPI2
        DCD     USART1_IRQHandler                 ; USART1
        DCD     USART2_IRQHandler                 ; USART2
        DCD     USART3_IRQHandler                 ; USART3
        DCD     EXTI15_10_IRQHandler              ; External Line[15:10]
        DCD     RTC_Alarm_IRQHandler              ; RTC Alarm (A and B) through EXTI Line
        DCD     DFSDM1_FLT3_IRQHandler            ; DFSDM1 Filter 3 global Interrupt
        DCD     TIM8_BRK_IRQHandler               ; TIM8 Break Interrupt
        DCD     TIM8_UP_IRQHandler                ; TIM8 Update Interrupt
        DCD     TIM8_TRG_COM_IRQHandler           ; TIM8 Trigger and Commutation Interrupt
        DCD     TIM8_CC_IRQHandler                ; TIM8 Capture Compare Interrupt
        DCD     0                                 ; Reserved
        DCD     FMC_IRQHandler                    ; FMC
        DCD     SDMMC1_IRQHandler                 ; SDMMC1
        DCD     TIM5_IRQHandler                   ; TIM5
        DCD     SPI3_IRQHandler                   ; SPI3
        DCD     UART4_IRQHandler                  ; UART4
        DCD     UART5_IRQHandler                  ; UART5
        DCD     TIM6_DAC_IRQHandler               ; TIM6 and DAC1&2 underrun errors
        DCD     TIM7_IRQHandler                   ; TIM7
        DCD     DMA2_Channel1_IRQHandler          ; DMA2 Channel 1
        DCD     DMA2_Channel2_IRQHandler          ; DMA2 Channel 2
        DCD     DMA2_Channel3_IRQHandler          ; DMA2 Channel 3
        DCD     DMA2_Channel4_IRQHandler          ; DMA2 Channel 4
        DCD     DMA2_Channel5_IRQHandler          ; DMA2 Channel 5
        DCD     DFSDM1_FLT0_IRQHandler            ; DFSDM1 Filter 0 global Interrupt
        DCD     DFSDM1_FLT1_IRQHandler            ; DFSDM1 Filter 1 global Interrupt
        DCD     DFSDM1_FLT2_IRQHandler            ; DFSDM1 Filter 2 global Interrupt
        DCD     COMP_IRQHandler                   ; COMP Interrupt
        DCD     LPTIM1_IRQHandler                 ; LP TIM1 interrupt
        DCD     LPTIM2_IRQHandler                 ; LP TIM2 interrupt
        DCD     OTG_FS_IRQHandler                 ; USB OTG FS
        DCD     DMA2_Channel6_IRQHandler          ; DMA2 Channel 6
        DCD     DMA2_Channel7_IRQHandler          ; DMA2 Channel 7
        DCD     LPUART1_IRQHandler                ; LP UART 1 interrupt
        DCD     OCTOSPI1_IRQHandler               ; OctoSPI1 global interrupt
        DCD     I2C3_EV_IRQHandler                ; I2C3 event
        DCD     I2C3_ER_IRQHandler                ; I2C3 error
        DCD     SAI1_IRQHandler                   ; Serial Audio Interface 1 global interrupt
        DCD     SAI2_IRQHandler                   ; Serial Audio Interface 2 global interrupt
        DCD     OCTOSPI2_IRQHandler               ; OctoSPI2 global interrupt
        DCD     TSC_IRQHandler                    ; Touch Sense Controller global interrupt
        DCD     DSI_IRQHandler                    ; DSI global interrupt
        DCD     0                                 ; Reserved
        DCD     RNG_IRQHandler                    ; RNG global interrupt
        DCD     FPU_IRQHandler                    ; FPU
        DCD     CRS_IRQHandler                    ; CRS interrupt
        DCD     I2C4_ER_IRQHandler                ; I2C4 error
        DCD     I2C4_EV_IRQHandler                ; I2C4 event
        DCD     DCMI_IRQHandler                   ; DCMI global interrupt
        DCD     0                                 ; Reserved
        DCD     0                                 ; Reserved
        DCD     0                                 ; Reserved
        DCD     0                                 ; Reserved
        DCD     DMA2D_IRQHandler                  ; DMA2D global interrupt       
        DCD     LTDC_IRQHandler                   ; LTDC global interrupt
        DCD     LTDC_ER_IRQHandler                ; LTDC error global interrupt
        DCD     GFXMMU_IRQHandler                 ; GFXMMU global interrupt
        DCD     DMAMUX1_OVR_IRQHandler            ; DMAMUX1 overrun global interrupt

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB
        PUBWEAK Reset_Handler
        SECTION .text:CODE:NOROOT:REORDER(2)
Reset_Handler
        LDR     R0, =SystemInit
        BLX     R0
        LDR     R0, =__iar_program_start
        BX      R0

        PUBWEAK NMI_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
HardFault_Handler
        B HardFault_Handler

        PUBWEAK MemManage_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
MemManage_Handler
        B MemManage_Handler

        PUBWEAK BusFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
BusFault_Handler
        B BusFault_Handler

        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
UsageFault_Handler
        B UsageFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
SVC_Handler
        B SVC_Handler

        PUBWEAK DebugMon_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
DebugMon_Handler
        B DebugMon_Handler

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
PendSV_Handler
        B PendSV_Handler

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
SysTick_Handler
        B SysTick_Handler

        PUBWEAK WWDG_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
WWDG_IRQHandler
        B WWDG_IRQHandler

        PUBWEAK PVD_PVM_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
PVD_PVM_IRQHandler
        B PVD_PVM_IRQHandler

        PUBWEAK TAMP_STAMP_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TAMP_STAMP_IRQHandler
        B TAMP_STAMP_IRQHandler

        PUBWEAK RTC_WKUP_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RTC_WKUP_IRQHandler
        B RTC_WKUP_IRQHandler

        PUBWEAK FLASH_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
FLASH_IRQHandler
        B FLASH_IRQHandler

        PUBWEAK RCC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RCC_IRQHandler
        B RCC_IRQHandler

        PUBWEAK EXTI0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI0_IRQHandler
        B EXTI0_IRQHandler

        PUBWEAK EXTI1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI1_IRQHandler
        B EXTI1_IRQHandler

        PUBWEAK EXTI2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI2_IRQHandler
        B EXTI2_IRQHandler

        PUBWEAK EXTI3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI3_IRQHandler
        B EXTI3_IRQHandler

        PUBWEAK EXTI4_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI4_IRQHandler
        B EXTI4_IRQHandler

        PUBWEAK DMA1_Channel1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel1_IRQHandler
        B DMA1_Channel1_IRQHandler

        PUBWEAK DMA1_Channel2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel2_IRQHandler
        B DMA1_Channel2_IRQHandler

        PUBWEAK DMA1_Channel3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel3_IRQHandler
        B DMA1_Channel3_IRQHandler

        PUBWEAK DMA1_Channel4_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel4_IRQHandler
        B DMA1_Channel4_IRQHandler

        PUBWEAK DMA1_Channel5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel5_IRQHandler
        B DMA1_Channel5_IRQHandler

        PUBWEAK DMA1_Channel6_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel6_IRQHandler
        B DMA1_Channel6_IRQHandler

        PUBWEAK DMA1_Channel7_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel7_IRQHandler
        B DMA1_Channel7_IRQHandler

        PUBWEAK ADC1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
ADC1_IRQHandler
        B ADC1_IRQHandler

        PUBWEAK CAN1_TX_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
CAN1_TX_IRQHandler
        B CAN1_TX_IRQHandler

        PUBWEAK CAN1_RX0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
CAN1_RX0_IRQHandler
        B CAN1_RX0_IRQHandler

        PUBWEAK CAN1_RX1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
CAN1_RX1_IRQHandler
        B CAN1_RX1_IRQHandler

        PUBWEAK CAN1_SCE_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
CAN1_SCE_IRQHandler
        B CAN1_SCE_IRQHandler

        PUBWEAK EXTI9_5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI9_5_IRQHandler
        B EXTI9_5_IRQHandler

        PUBWEAK TIM1_BRK_TIM15_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM1_BRK_TIM15_IRQHandler
        B TIM1_BRK_TIM15_IRQHandler

        PUBWEAK TIM1_UP_TIM16_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM1_UP_TIM16_IRQHandler
        B TIM1_UP_TIM16_IRQHandler

        PUBWEAK TIM1_TRG_COM_TIM17_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM1_TRG_COM_TIM17_IRQHandler
        B TIM1_TRG_COM_TIM17_IRQHandler

        PUBWEAK TIM1_CC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM1_CC_IRQHandler
        B TIM1_CC_IRQHandler

        PUBWEAK TIM2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM2_IRQHandler
        B TIM2_IRQHandler

        PUBWEAK TIM3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM3_IRQHandler
        B TIM3_IRQHandler

        PUBWEAK TIM4_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM4_IRQHandler
        B TIM4_IRQHandler

        PUBWEAK I2C1_EV_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C1_EV_IRQHandler
        B I2C1_EV_IRQHandler

        PUBWEAK I2C1_ER_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C1_ER_IRQHandler
        B I2C1_ER_IRQHandler

        PUBWEAK I2C2_EV_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C2_EV_IRQHandler
        B I2C2_EV_IRQHandler

        PUBWEAK I2C2_ER_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C2_ER_IRQHandler
        B I2C2_ER_IRQHandler

        PUBWEAK SPI1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI1_IRQHandler
        B SPI1_IRQHandler

        PUBWEAK SPI2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI2_IRQHandler
        B SPI2_IRQHandler

        PUBWEAK USART1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
USART1_IRQHandler
        B USART1_IRQHandler

        PUBWEAK USART2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
USART2_IRQHandler
        B USART2_IRQHandler

        PUBWEAK USART3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
USART3_IRQHandler
        B USART3_IRQHandler

        PUBWEAK EXTI15_10_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI15_10_IRQHandler
        B EXTI15_10_IRQHandler

        PUBWEAK RTC_Alarm_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RTC_Alarm_IRQHandler
        B RTC_Alarm_IRQHandler

        PUBWEAK DFSDM1_FLT3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DFSDM1_FLT3_IRQHandler
        B DFSDM1_FLT3_IRQHandler

        PUBWEAK TIM8_BRK_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM8_BRK_IRQHandler
        B TIM8_BRK_IRQHandler

        PUBWEAK TIM8_UP_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM8_UP_IRQHandler
        B TIM8_UP_IRQHandler

        PUBWEAK TIM8_TRG_COM_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM8_TRG_COM_IRQHandler
        B TIM8_TRG_COM_IRQHandler

        PUBWEAK TIM8_CC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM8_CC_IRQHandler
        B TIM8_CC_IRQHandler

        PUBWEAK FMC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
FMC_IRQHandler
        B FMC_IRQHandler

        PUBWEAK SDMMC1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SDMMC1_IRQHandler
        B SDMMC1_IRQHandler

        PUBWEAK TIM5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM5_IRQHandler
        B TIM5_IRQHandler

        PUBWEAK SPI3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI3_IRQHandler
        B SPI3_IRQHandler

        PUBWEAK UART4_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART4_IRQHandler
        B UART4_IRQHandler

        PUBWEAK UART5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART5_IRQHandler
        B UART5_IRQHandler

        PUBWEAK TIM6_DAC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM6_DAC_IRQHandler
        B TIM6_DAC_IRQHandler

        PUBWEAK TIM7_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM7_IRQHandler
        B TIM7_IRQHandler

        PUBWEAK DMA2_Channel1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel1_IRQHandler
        B DMA2_Channel1_IRQHandler

        PUBWEAK DMA2_Channel2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel2_IRQHandler
        B DMA2_Channel2_IRQHandler

        PUBWEAK DMA2_Channel3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel3_IRQHandler
        B DMA2_Channel3_IRQHandler

        PUBWEAK DMA2_Channel4_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel4_IRQHandler
        B DMA2_Channel4_IRQHandler

        PUBWEAK DMA2_Channel5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel5_IRQHandler
        B DMA2_Channel5_IRQHandler

        PUBWEAK DFSDM1_FLT0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DFSDM1_FLT0_IRQHandler
        B DFSDM1_FLT0_IRQHandler

        PUBWEAK DFSDM1_FLT1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DFSDM1_FLT1_IRQHandler
        B DFSDM1_FLT1_IRQHandler

        PUBWEAK DFSDM1_FLT2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DFSDM1_FLT2_IRQHandler
        B DFSDM1_FLT2_IRQHandler

        PUBWEAK COMP_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
COMP_IRQHandler
        B COMP_IRQHandler

        PUBWEAK LPTIM1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
LPTIM1_IRQHandler
        B LPTIM1_IRQHandler

        PUBWEAK LPTIM2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
LPTIM2_IRQHandler
        B LPTIM2_IRQHandler

        PUBWEAK OTG_FS_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
OTG_FS_IRQHandler
        B OTG_FS_IRQHandler

        PUBWEAK DMA2_Channel6_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel6_IRQHandler
        B DMA2_Channel6_IRQHandler

        PUBWEAK DMA2_Channel7_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel7_IRQHandler
        B DMA2_Channel7_IRQHandler

        PUBWEAK LPUART1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
LPUART1_IRQHandler
        B LPUART1_IRQHandler

        PUBWEAK OCTOSPI1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
OCTOSPI1_IRQHandler
        B OCTOSPI1_IRQHandler

        PUBWEAK I2C3_EV_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C3_EV_IRQHandler
        B I2C3_EV_IRQHandler

        PUBWEAK I2C3_ER_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C3_ER_IRQHandler
        B I2C3_ER_IRQHandler

        PUBWEAK SAI1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SAI1_IRQHandler
        B SAI1_IRQHandler

       PUBWEAK SAI2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SAI2_IRQHandler
        B SAI2_IRQHandler

        PUBWEAK OCTOSPI2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
OCTOSPI2_IRQHandler
        B OCTOSPI2_IRQHandler

       PUBWEAK TSC_IRQHandler
       SECTION .text:CODE:NOROOT:REORDER(1)
TSC_IRQHandler
       B TSC_IRQHandler

       PUBWEAK DSI_IRQHandler
       SECTION .text:CODE:NOROOT:REORDER(1)
DSI_IRQHandler
       B DSI_IRQHandler

        PUBWEAK RNG_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RNG_IRQHandler
        B RNG_IRQHandler

        PUBWEAK FPU_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
FPU_IRQHandler
        B FPU_IRQHandler
        
        PUBWEAK CRS_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
CRS_IRQHandler
        B CRS_IRQHandler
        
        PUBWEAK I2C4_ER_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C4_ER_IRQHandler
        B I2C4_ER_IRQHandler
        
        PUBWEAK I2C4_EV_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C4_EV_IRQHandler
        B I2C4_EV_IRQHandler

        PUBWEAK DMA2D_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2D_IRQHandler
        B DMA2D_IRQHandler

        PUBWEAK DCMI_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DCMI_IRQHandler
        B DCMI_IRQHandler
        
        PUBWEAK LTDC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
LTDC_IRQHandler
        B LTDC_IRQHandler

        PUBWEAK LTDC_ER_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
LTDC_ER_IRQHandler
        B LTDC_ER_IRQHandler

        PUBWEAK GFXMMU_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
GFXMMU_IRQHandler
        B GFXMMU_IRQHandler

        PUBWEAK DMAMUX1_OVR_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMAMUX1_OVR_IRQHandler
        B DMAMUX1_OVR_IRQHandler
        
        END
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
