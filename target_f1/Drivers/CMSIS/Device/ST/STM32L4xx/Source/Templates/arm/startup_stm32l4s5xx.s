;*******************************************************************************
;* File Name          : startup_stm32l4s5xx.s
;* Author             : MCD Application Team
;* Description        : STM32L4S5xx Ultra Low Power devices vector table for MDK-ARM toolchain.
;*                      This module performs:
;*                      - Set the initial SP
;*                      - Set the initial PC == Reset_Handler
;*                      - Set the vector table entries with the exceptions ISR address
;*                      - Branches to __main in the C library (which eventually
;*                        calls main()).
;*                      After Reset the Cortex-M4 processor is in Thread mode,
;*                      priority is Privileged, and the Stack is set to Main.
;* <<< Use Configuration Wizard in Context Menu >>>
;*******************************************************************************
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
; Amount of memory (in bytes) allocated for Stack
; Tailor this value to your application needs
; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x400;

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x200;

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp               ; Top of Stack
                DCD     Reset_Handler              ; Reset Handler
                DCD     NMI_Handler                ; NMI Handler
                DCD     HardFault_Handler          ; Hard Fault Handler
                DCD     MemManage_Handler          ; MPU Fault Handler
                DCD     BusFault_Handler           ; Bus Fault Handler
                DCD     UsageFault_Handler         ; Usage Fault Handler
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     SVC_Handler                ; SVCall Handler
                DCD     DebugMon_Handler           ; Debug Monitor Handler
                DCD     0                          ; Reserved
                DCD     PendSV_Handler             ; PendSV Handler
                DCD     SysTick_Handler            ; SysTick Handler

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
                DCD     LPUART1_IRQHandler                ; LP UART1 interrupt
                DCD     OCTOSPI1_IRQHandler               ; OctoSPI1 global interrupt
                DCD     I2C3_EV_IRQHandler                ; I2C3 event
                DCD     I2C3_ER_IRQHandler                ; I2C3 error
                DCD     SAI1_IRQHandler                   ; Serial Audio Interface 1 global interrupt
                DCD     SAI2_IRQHandler                   ; Serial Audio Interface 2 global interrupt
                DCD     OCTOSPI2_IRQHandler               ; OctoSPI2 global interrupt
                DCD     TSC_IRQHandler                    ; Touch Sense Controller global interrupt
                DCD     0                                 ; Reserved
                DCD     AES_IRQHandler                    ; AES global interrupt
                DCD     RNG_IRQHandler                    ; RNG global interrupt
                DCD     FPU_IRQHandler                    ; FPU
                DCD     HASH_CRS_IRQHandler               ; HASH and CRS global interrupt
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

__Vectors_End

__Vectors_Size  EQU  __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY

; Reset handler
Reset_Handler    PROC
                 EXPORT  Reset_Handler             [WEAK]
        IMPORT  SystemInit
        IMPORT  __main

                 LDR     R0, =SystemInit
                 BLX     R0
                 LDR     R0, =__main
                 BX      R0
                 ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler                [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler          [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler          [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler           [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler         [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler                [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler           [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler             [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler            [WEAK]
                B       .
                ENDP

Default_Handler PROC

        EXPORT     WWDG_IRQHandler                   [WEAK]
        EXPORT     PVD_PVM_IRQHandler                [WEAK]
        EXPORT     TAMP_STAMP_IRQHandler             [WEAK]
        EXPORT     RTC_WKUP_IRQHandler               [WEAK]
        EXPORT     FLASH_IRQHandler                  [WEAK]
        EXPORT     RCC_IRQHandler                    [WEAK]
        EXPORT     EXTI0_IRQHandler                  [WEAK]
        EXPORT     EXTI1_IRQHandler                  [WEAK]
        EXPORT     EXTI2_IRQHandler                  [WEAK]
        EXPORT     EXTI3_IRQHandler                  [WEAK]
        EXPORT     EXTI4_IRQHandler                  [WEAK]
        EXPORT     DMA1_Channel1_IRQHandler          [WEAK]
        EXPORT     DMA1_Channel2_IRQHandler          [WEAK]
        EXPORT     DMA1_Channel3_IRQHandler          [WEAK]
        EXPORT     DMA1_Channel4_IRQHandler          [WEAK]
        EXPORT     DMA1_Channel5_IRQHandler          [WEAK]
        EXPORT     DMA1_Channel6_IRQHandler          [WEAK]
        EXPORT     DMA1_Channel7_IRQHandler          [WEAK]
        EXPORT     ADC1_IRQHandler                   [WEAK]
        EXPORT     CAN1_TX_IRQHandler                [WEAK]
        EXPORT     CAN1_RX0_IRQHandler               [WEAK]
        EXPORT     CAN1_RX1_IRQHandler               [WEAK]
        EXPORT     CAN1_SCE_IRQHandler               [WEAK]
        EXPORT     EXTI9_5_IRQHandler                [WEAK]
        EXPORT     TIM1_BRK_TIM15_IRQHandler         [WEAK]
        EXPORT     TIM1_UP_TIM16_IRQHandler          [WEAK]
        EXPORT     TIM1_TRG_COM_TIM17_IRQHandler     [WEAK]
        EXPORT     TIM1_CC_IRQHandler                [WEAK]
        EXPORT     TIM2_IRQHandler                   [WEAK]
        EXPORT     TIM3_IRQHandler                   [WEAK]
        EXPORT     TIM4_IRQHandler                   [WEAK]
        EXPORT     I2C1_EV_IRQHandler                [WEAK]
        EXPORT     I2C1_ER_IRQHandler                [WEAK]
        EXPORT     I2C2_EV_IRQHandler                [WEAK]
        EXPORT     I2C2_ER_IRQHandler                [WEAK]
        EXPORT     SPI1_IRQHandler                   [WEAK]
        EXPORT     SPI2_IRQHandler                   [WEAK]
        EXPORT     USART1_IRQHandler                 [WEAK]
        EXPORT     USART2_IRQHandler                 [WEAK]
        EXPORT     USART3_IRQHandler                 [WEAK]
        EXPORT     EXTI15_10_IRQHandler              [WEAK]
        EXPORT     RTC_Alarm_IRQHandler              [WEAK]
        EXPORT     DFSDM1_FLT3_IRQHandler            [WEAK]
        EXPORT     TIM8_BRK_IRQHandler               [WEAK]
        EXPORT     TIM8_UP_IRQHandler                [WEAK]
        EXPORT     TIM8_TRG_COM_IRQHandler           [WEAK]
        EXPORT     TIM8_CC_IRQHandler                [WEAK]
        EXPORT     FMC_IRQHandler                    [WEAK]
        EXPORT     SDMMC1_IRQHandler                 [WEAK]
        EXPORT     TIM5_IRQHandler                   [WEAK]
        EXPORT     SPI3_IRQHandler                   [WEAK]
        EXPORT     UART4_IRQHandler                  [WEAK]
        EXPORT     UART5_IRQHandler                  [WEAK]
        EXPORT     TIM6_DAC_IRQHandler               [WEAK]
        EXPORT     TIM7_IRQHandler                   [WEAK]
        EXPORT     DMA2_Channel1_IRQHandler          [WEAK]
        EXPORT     DMA2_Channel2_IRQHandler          [WEAK]
        EXPORT     DMA2_Channel3_IRQHandler          [WEAK]
        EXPORT     DMA2_Channel4_IRQHandler          [WEAK]
        EXPORT     DMA2_Channel5_IRQHandler          [WEAK]
        EXPORT     DFSDM1_FLT0_IRQHandler            [WEAK]
        EXPORT     DFSDM1_FLT1_IRQHandler            [WEAK]
        EXPORT     DFSDM1_FLT2_IRQHandler            [WEAK]
        EXPORT     COMP_IRQHandler                   [WEAK]
        EXPORT     LPTIM1_IRQHandler                 [WEAK]
        EXPORT     LPTIM2_IRQHandler                 [WEAK]
        EXPORT     OTG_FS_IRQHandler                 [WEAK]
        EXPORT     DMA2_Channel6_IRQHandler          [WEAK]
        EXPORT     DMA2_Channel7_IRQHandler          [WEAK]
        EXPORT     LPUART1_IRQHandler                [WEAK]
        EXPORT     OCTOSPI1_IRQHandler               [WEAK]
        EXPORT     I2C3_EV_IRQHandler                [WEAK]
        EXPORT     I2C3_ER_IRQHandler                [WEAK]
        EXPORT     SAI1_IRQHandler                   [WEAK]
        EXPORT     SAI2_IRQHandler                   [WEAK]
        EXPORT     OCTOSPI2_IRQHandler               [WEAK]
        EXPORT     TSC_IRQHandler                    [WEAK]
        EXPORT     AES_IRQHandler                    [WEAK]
        EXPORT     RNG_IRQHandler                    [WEAK]
        EXPORT     FPU_IRQHandler                    [WEAK]
        EXPORT     HASH_CRS_IRQHandler               [WEAK]
        EXPORT     I2C4_ER_IRQHandler                [WEAK]
        EXPORT     I2C4_EV_IRQHandler                [WEAK]
        EXPORT     DCMI_IRQHandler                   [WEAK]
        EXPORT     DMA2D_IRQHandler                  [WEAK]
        EXPORT     LTDC_IRQHandler                   [WEAK]
        EXPORT     LTDC_ER_IRQHandler                [WEAK]
        EXPORT     GFXMMU_IRQHandler                 [WEAK]
        EXPORT     DMAMUX1_OVR_IRQHandler             [WEAK]

WWDG_IRQHandler
PVD_PVM_IRQHandler
TAMP_STAMP_IRQHandler
RTC_WKUP_IRQHandler
FLASH_IRQHandler
RCC_IRQHandler
EXTI0_IRQHandler
EXTI1_IRQHandler
EXTI2_IRQHandler
EXTI3_IRQHandler
EXTI4_IRQHandler
DMA1_Channel1_IRQHandler
DMA1_Channel2_IRQHandler
DMA1_Channel3_IRQHandler
DMA1_Channel4_IRQHandler
DMA1_Channel5_IRQHandler
DMA1_Channel6_IRQHandler
DMA1_Channel7_IRQHandler
ADC1_IRQHandler
CAN1_TX_IRQHandler
CAN1_RX0_IRQHandler
CAN1_RX1_IRQHandler
CAN1_SCE_IRQHandler
EXTI9_5_IRQHandler
TIM1_BRK_TIM15_IRQHandler
TIM1_UP_TIM16_IRQHandler
TIM1_TRG_COM_TIM17_IRQHandler
TIM1_CC_IRQHandler
TIM2_IRQHandler
TIM3_IRQHandler
TIM4_IRQHandler
I2C1_EV_IRQHandler
I2C1_ER_IRQHandler
I2C2_EV_IRQHandler
I2C2_ER_IRQHandler
SPI1_IRQHandler
SPI2_IRQHandler
USART1_IRQHandler
USART2_IRQHandler
USART3_IRQHandler
EXTI15_10_IRQHandler
RTC_Alarm_IRQHandler
DFSDM1_FLT3_IRQHandler
TIM8_BRK_IRQHandler
TIM8_UP_IRQHandler
TIM8_TRG_COM_IRQHandler
TIM8_CC_IRQHandler
FMC_IRQHandler
SDMMC1_IRQHandler
TIM5_IRQHandler
SPI3_IRQHandler
UART4_IRQHandler
UART5_IRQHandler
TIM6_DAC_IRQHandler
TIM7_IRQHandler
DMA2_Channel1_IRQHandler
DMA2_Channel2_IRQHandler
DMA2_Channel3_IRQHandler
DMA2_Channel4_IRQHandler
DMA2_Channel5_IRQHandler
DFSDM1_FLT0_IRQHandler
DFSDM1_FLT1_IRQHandler
DFSDM1_FLT2_IRQHandler
COMP_IRQHandler
LPTIM1_IRQHandler
LPTIM2_IRQHandler
OTG_FS_IRQHandler
DMA2_Channel6_IRQHandler
DMA2_Channel7_IRQHandler
LPUART1_IRQHandler
OCTOSPI1_IRQHandler
I2C3_EV_IRQHandler
I2C3_ER_IRQHandler
SAI1_IRQHandler
SAI2_IRQHandler
OCTOSPI2_IRQHandler
TSC_IRQHandler
AES_IRQHandler
RNG_IRQHandler
FPU_IRQHandler
HASH_CRS_IRQHandler
I2C4_ER_IRQHandler
I2C4_EV_IRQHandler
DCMI_IRQHandler
DMA2D_IRQHandler
LTDC_IRQHandler
LTDC_ER_IRQHandler
GFXMMU_IRQHandler
DMAMUX1_OVR_IRQHandler

                B       .

                ENDP

                ALIGN

;*******************************************************************************
; User Stack and Heap initialization
;*******************************************************************************
                 IF      :DEF:__MICROLIB

                 EXPORT  __initial_sp
                 EXPORT  __heap_base
                 EXPORT  __heap_limit

                 ELSE

                 IMPORT  __use_two_region_memory
                 EXPORT  __user_initial_stackheap

__user_initial_stackheap

                 LDR     R0, =  Heap_Mem
                 LDR     R1, =(Stack_Mem + Stack_Size)
                 LDR     R2, = (Heap_Mem +  Heap_Size)
                 LDR     R3, = Stack_Mem
                 BX      LR

                 ALIGN

                 ENDIF

                 END

;************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE*****
