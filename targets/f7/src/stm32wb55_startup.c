#include <stm32wb55_startup.h>
#include <stm32wb55_linker.h>
#include <stm32wbxx.h>

/** System Core Clock speed
 * 
 * CPU1: M4 on MSI clock after startup (4MHz).
 * Modified by RCC LL HAL.
 */
uint32_t SystemCoreClock = 4000000UL;

/** AHB Prescaler Table. Used by RCC LL HAL */
const uint32_t AHBPrescTable[16UL] =
    {1UL, 3UL, 5UL, 1UL, 1UL, 6UL, 10UL, 32UL, 2UL, 4UL, 8UL, 16UL, 64UL, 128UL, 256UL, 512UL};
/** APB Prescaler Table. Used by RCC LL HAL */
const uint32_t APBPrescTable[8UL] = {0UL, 0UL, 0UL, 0UL, 1UL, 2UL, 3UL, 4UL};
/** MSI Range Table. Used by RCC LL HAL */
const uint32_t MSIRangeTable[16UL] = {
    100000UL,
    200000UL,
    400000UL,
    800000UL,
    1000000UL,
    2000000UL,
    4000000UL,
    8000000UL,
    16000000UL,
    24000000UL,
    32000000UL,
    48000000UL,
    0UL,
    0UL,
    0UL,
    0UL}; /* 0UL values are incorrect cases */

/** MCU Initialization Routine. Part of ST HAL convention, so we keep it.*/
void SystemInit(void) {
    // Set ISR Vector location
#if defined(VECT_TAB_SRAM)
    // Point ISR Vector to SRAM
    SCB->VTOR = SRAM1_BASE;
#else
    // Point ISR Vector to 0x0, which is mapped to 0x08000000(Flash)
    SCB->VTOR = 0x0;
#endif

#if(__FPU_PRESENT == 1) && (__FPU_USED == 1)
    // Enable access to FPU
    SCB->CPACR |=
        ((3UL << (10UL * 2UL)) | (3UL << (11UL * 2UL))); /* set CP10 and CP11 Full Access */
#endif

    // Reset the RCC clock configuration to the default reset state
    // Set MSION bit
    RCC->CR |= RCC_CR_MSION;
    // Reset CFGR register
    RCC->CFGR = 0x00070000U;
    // Reset PLLSAI1ON, PLLON, HSECSSON, HSEON, HSION, and MSIPLLON bits
    RCC->CR &= (uint32_t)0xFAF6FEFBU;
    // Reset LSI1 and LSI2 bits
    RCC->CSR &= (uint32_t)0xFFFFFFFAU;
    // Reset HSI48ON bit
    RCC->CRRCR &= (uint32_t)0xFFFFFFFEU;
    // Reset PLLCFGR register
    RCC->PLLCFGR = 0x22041000U;
#if defined(STM32WB55xx) || defined(STM32WB5Mxx)
    // Reset PLLSAI1CFGR register
    RCC->PLLSAI1CFGR = 0x22041000U;
#endif
    // Reset HSEBYP bit
    RCC->CR &= 0xFFFBFFFFU;
    // Disable all RCC related interrupts
    RCC->CIER = 0x00000000;
}

void Default_Handler(void) {
    furi_crash("NotImplemented");
}

/** Start your journey here */
FURI_NAKED void Reset_Handler(void) {
    // Funny thing: SP and MSP are set to _stack_end if we came here after MCU reset
    // Now, what if we came from boot loader? Lets set SP to _stack_end again.
    // By the way Furi stage loader doing it too, but we don't know who called us.
    asm volatile("ldr r0, =_stack_end");
    asm volatile("mov sp, r0");

    // ST chip initialization routine
    SystemInit();

    // Copy data section from flash
    memcpy((void*)&_sdata, &_sidata, &_edata - &_sdata);

    // Wipe BSS
    memset((void*)&_sbss, 0x00, &_ebss - &_sbss);

    // Core2 related quirks: wipe MB_MEM2 section
    memset((void*)&_sMB_MEM2, 0x00, &_eMB_MEM2 - &_sMB_MEM2);

    // libc init array
    __libc_init_array();

    // Our main
    main();

    // You should never exit from main, but we'll catch you if you do
    furi_crash("WhyExit?");
}

/** ISR type */
typedef void (*element_t)(void);

/** System initialization vector. Contains: main stack end address, 15 pointers
 * to unmask-able ISR and 63 to mask-able ISR. */
PLACE_IN_SECTION(".isr_vector")
const element_t reset_vector[] = {
    /* Main stack top */
    (element_t)&_stack_end,
    /* 15 Unmaskable ISR */
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    NULL,
    NULL,
    NULL,
    NULL,
    SVC_Handler,
    DebugMon_Handler,
    NULL,
    PendSV_Handler,
    SysTick_Handler,
    /* 63 Maskable ISR */
    WWDG_IRQHandler,
    PVD_PVM_IRQHandler,
    TAMP_STAMP_LSECSS_IRQHandler,
    RTC_WKUP_IRQHandler,
    FLASH_IRQHandler,
    RCC_IRQHandler,
    EXTI0_IRQHandler,
    EXTI1_IRQHandler,
    EXTI2_IRQHandler,
    EXTI3_IRQHandler,
    EXTI4_IRQHandler,
    DMA1_Channel1_IRQHandler,
    DMA1_Channel2_IRQHandler,
    DMA1_Channel3_IRQHandler,
    DMA1_Channel4_IRQHandler,
    DMA1_Channel5_IRQHandler,
    DMA1_Channel6_IRQHandler,
    DMA1_Channel7_IRQHandler,
    ADC1_IRQHandler,
    USB_HP_IRQHandler,
    USB_LP_IRQHandler,
    C2SEV_PWR_C2H_IRQHandler,
    COMP_IRQHandler,
    EXTI9_5_IRQHandler,
    TIM1_BRK_IRQHandler,
    TIM1_UP_TIM16_IRQHandler,
    TIM1_TRG_COM_TIM17_IRQHandler,
    TIM1_CC_IRQHandler,
    TIM2_IRQHandler,
    PKA_IRQHandler,
    I2C1_EV_IRQHandler,
    I2C1_ER_IRQHandler,
    I2C3_EV_IRQHandler,
    I2C3_ER_IRQHandler,
    SPI1_IRQHandler,
    SPI2_IRQHandler,
    USART1_IRQHandler,
    LPUART1_IRQHandler,
    SAI1_IRQHandler,
    TSC_IRQHandler,
    EXTI15_10_IRQHandler,
    RTC_Alarm_IRQHandler,
    CRS_IRQHandler,
    PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQHandler,
    IPCC_C1_RX_IRQHandler,
    IPCC_C1_TX_IRQHandler,
    HSEM_IRQHandler,
    LPTIM1_IRQHandler,
    LPTIM2_IRQHandler,
    LCD_IRQHandler,
    QUADSPI_IRQHandler,
    AES1_IRQHandler,
    AES2_IRQHandler,
    RNG_IRQHandler,
    FPU_IRQHandler,
    DMA2_Channel1_IRQHandler,
    DMA2_Channel2_IRQHandler,
    DMA2_Channel3_IRQHandler,
    DMA2_Channel4_IRQHandler,
    DMA2_Channel5_IRQHandler,
    DMA2_Channel6_IRQHandler,
    DMA2_Channel7_IRQHandler,
    DMAMUX1_OVR_IRQHandler,
};
