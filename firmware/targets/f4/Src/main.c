#include "main.h"

#include "cmsis_os2.h"
#include "adc.h"
#include "aes.h"
#include "comp.h"
#include "crc.h"
#include "pka.h"
#include "rf.h"
#include "rng.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "fatfs/fatfs.h"

#include <furi.h>
#include <api-hal.h>
#include <flipper.h>

void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_RTC_Init();
    MX_SPI1_Init();
    MX_SPI2_Init();
    MX_USART1_UART_Init();
    MX_USB_Device_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_TIM16_Init();
    MX_COMP1_Init();
    MX_RF_Init();
    MX_PKA_Init();
    MX_RNG_Init();
    MX_AES1_Init();
    MX_AES2_Init();
    MX_CRC_Init();

    api_hal_init();
    MX_FATFS_Init();
    delay_us_init_DWT();

    furi_init();
    // CMSIS initialization
    osKernelInitialize();
    // Init flipper
    flipper_init();
    // Start kernel
    osKernelStart();

    while (1) {}
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    HAL_PWR_EnableBkUpAccess();

    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMLOW);
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE
                                                            |RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
    RCC_OscInitStruct.PLL.PLLN = 8;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                                                            |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                                            |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV2;
    RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
        Error_Handler();
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS|RCC_PERIPHCLK_RFWAKEUP
                                                            |RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                                                            |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_CLK48SEL
                                                            |RCC_PERIPHCLK_USB|RCC_PERIPHCLK_RNG
                                                            |RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.PLLSAI1.PLLN = 6;
    PeriphClkInitStruct.PLLSAI1.PLLP = RCC_PLLP_DIV2;
    PeriphClkInitStruct.PLLSAI1.PLLQ = RCC_PLLQ_DIV2;
    PeriphClkInitStruct.PLLSAI1.PLLR = RCC_PLLR_DIV2;
    PeriphClkInitStruct.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_USBCLK|RCC_PLLSAI1_ADCCLK;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLLSAI1;
    PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_CLK48;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_LSE;
    PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSE;
    PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        Error_Handler();
    }

    // Enable CSS for both clocks
    HAL_RCC_EnableCSS();
    HAL_RCCEx_EnableLSECSS();
}

void Error_Handler(void) {
    asm("bkpt 1");
    while(1) {}
}

#ifdef  USE_FULL_ASSERT
/**
    * @brief  Reports the name of the source file and the source line number
    *         where the assert_param error has occurred.
    * @param  file: pointer to the source file name
    * @param  line: assert_param error line source number
    * @retval None
    */
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
         tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
