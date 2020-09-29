#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#define HAL_MAX_DELAY INT_MAX

typedef uint32_t UART_HandleTypeDef;
uint16_t
HAL_UART_Transmit(UART_HandleTypeDef* handle, uint8_t* bufer, uint16_t size, uint32_t wait_ms);

typedef uint32_t TIM_HandleTypeDef;

#define LED_RED_GPIO_Port 1
#define LED_RED_Pin 1