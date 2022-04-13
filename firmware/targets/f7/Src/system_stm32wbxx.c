#include "stm32wbxx.h"

/*!< Uncomment the following line if you need to relocate your vector Table in Internal SRAM. */
/* #define VECT_TAB_SRAM */

#ifndef VECT_TAB_OFFSET
#define VECT_TAB_OFFSET \
    0x0 /*!< Vector Table base offset field. This value must be a multiple of 0x200. */
#endif

#define VECT_TAB_BASE_ADDRESS \
    SRAM1_BASE /*!< Vector Table base offset field. This value must be a multiple of 0x200. */

/* The SystemCoreClock variable is updated in three ways:
      1) by calling CMSIS function SystemCoreClockUpdate()
      2) by calling HAL API function HAL_RCC_GetHCLKFreq()
      3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
         Note: If you use this function to configure the system clock; then there
               is no need to call the 2 first functions listed above, since SystemCoreClock
               variable is updated automatically.
  */
uint32_t SystemCoreClock = 4000000UL; /*CPU1: M4 on MSI clock after startup (4MHz)*/

const uint32_t AHBPrescTable[16UL] =
    {1UL, 3UL, 5UL, 1UL, 1UL, 6UL, 10UL, 32UL, 2UL, 4UL, 8UL, 16UL, 64UL, 128UL, 256UL, 512UL};

const uint32_t APBPrescTable[8UL] = {0UL, 0UL, 0UL, 0UL, 1UL, 2UL, 3UL, 4UL};

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

/**
  * @brief  Setup the microcontroller system.
  * @param  None
  * @retval None
  */
void SystemInit(void) {
    /* Configure the Vector Table location add offset address ------------------*/
#if defined(VECT_TAB_SRAM) && defined(VECT_TAB_BASE_ADDRESS)
    /* program in SRAMx */
    SCB->VTOR = VECT_TAB_BASE_ADDRESS |
                VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAMx for CPU1 */
#else /* program in FLASH */
    SCB->VTOR = VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif

/* FPU settings ------------------------------------------------------------*/
#if(__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |=
        ((3UL << (10UL * 2UL)) | (3UL << (11UL * 2UL))); /* set CP10 and CP11 Full Access */
#endif

    /* Reset the RCC clock configuration to the default reset state ------------*/
    /* Set MSION bit */
    RCC->CR |= RCC_CR_MSION;

    /* Reset CFGR register */
    RCC->CFGR = 0x00070000U;

    /* Reset PLLSAI1ON, PLLON, HSECSSON, HSEON, HSION, and MSIPLLON bits */
    RCC->CR &= (uint32_t)0xFAF6FEFBU;

    /*!< Reset LSI1 and LSI2 bits */
    RCC->CSR &= (uint32_t)0xFFFFFFFAU;

    /*!< Reset HSI48ON  bit */
    RCC->CRRCR &= (uint32_t)0xFFFFFFFEU;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x22041000U;

#if defined(STM32WB55xx) || defined(STM32WB5Mxx)
    /* Reset PLLSAI1CFGR register */
    RCC->PLLSAI1CFGR = 0x22041000U;
#endif

    /* Reset HSEBYP bit */
    RCC->CR &= 0xFFFBFFFFU;

    /* Disable all interrupts */
    RCC->CIER = 0x00000000;
}
