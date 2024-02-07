#pragma once
#include "stdbool.h"
#include <stm32wbxx_ll_gpio.h>
#include <stm32wbxx_ll_system.h>
#include <stm32wbxx_ll_exti.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Number of gpio on one port
 */
#define GPIO_NUMBER (16U)

/**
 * Interrupt callback prototype
 */
typedef void (*GpioExtiCallback)(void* ctx);

/**
 * Gpio interrupt type
 */
typedef struct {
    GpioExtiCallback callback;
    void* context;
} GpioInterrupt;

/**
 * Gpio modes
 */
typedef enum {
    GpioModeInput,
    GpioModeOutputPushPull,
    GpioModeOutputOpenDrain,
    GpioModeAltFunctionPushPull,
    GpioModeAltFunctionOpenDrain,
    GpioModeAnalog,
    GpioModeInterruptRise,
    GpioModeInterruptFall,
    GpioModeInterruptRiseFall,
    GpioModeEventRise,
    GpioModeEventFall,
    GpioModeEventRiseFall,
} GpioMode;

/**
 * Gpio pull modes
 */
typedef enum {
    GpioPullNo,
    GpioPullUp,
    GpioPullDown,
} GpioPull;

/**
 * Gpio speed modes
 */
typedef enum {
    GpioSpeedLow,
    GpioSpeedMedium,
    GpioSpeedHigh,
    GpioSpeedVeryHigh,
} GpioSpeed;

/**
 * Gpio alternate functions
 */
typedef enum {
    GpioAltFn0MCO = 0, /*!< MCO Alternate Function mapping */
    GpioAltFn0LSCO = 0, /*!< LSCO Alternate Function mapping */
    GpioAltFn0JTMS_SWDIO = 0, /*!< JTMS-SWDIO Alternate Function mapping */
    GpioAltFn0JTCK_SWCLK = 0, /*!< JTCK-SWCLK Alternate Function mapping */
    GpioAltFn0JTDI = 0, /*!< JTDI Alternate Function mapping */
    GpioAltFn0RTC_OUT = 0, /*!< RCT_OUT Alternate Function mapping */
    GpioAltFn0JTD_TRACE = 0, /*!< JTDO-TRACESWO Alternate Function mapping */
    GpioAltFn0NJTRST = 0, /*!< NJTRST Alternate Function mapping */
    GpioAltFn0RTC_REFIN = 0, /*!< RTC_REFIN Alternate Function mapping */
    GpioAltFn0TRACED0 = 0, /*!< TRACED0 Alternate Function mapping */
    GpioAltFn0TRACED1 = 0, /*!< TRACED1 Alternate Function mapping */
    GpioAltFn0TRACED2 = 0, /*!< TRACED2 Alternate Function mapping */
    GpioAltFn0TRACED3 = 0, /*!< TRACED3 Alternate Function mapping */
    GpioAltFn0TRIG_INOUT = 0, /*!< TRIG_INOUT Alternate Function mapping */
    GpioAltFn0TRACECK = 0, /*!< TRACECK Alternate Function mapping */
    GpioAltFn0SYS = 0, /*!< System Function mapping */

    GpioAltFn1TIM1 = 1, /*!< TIM1 Alternate Function mapping */
    GpioAltFn1TIM2 = 1, /*!< TIM2 Alternate Function mapping */
    GpioAltFn1LPTIM1 = 1, /*!< LPTIM1 Alternate Function mapping */

    GpioAltFn2TIM2 = 2, /*!< TIM2 Alternate Function mapping */
    GpioAltFn2TIM1 = 2, /*!< TIM1 Alternate Function mapping */

    GpioAltFn3SAI1 = 3, /*!< SAI1_CK1 Alternate Function mapping */
    GpioAltFn3SPI2 = 3, /*!< SPI2 Alternate Function mapping */
    GpioAltFn3TIM1 = 3, /*!< TIM1 Alternate Function mapping */

    GpioAltFn4I2C1 = 4, /*!< I2C1 Alternate Function mapping */
    GpioAltFn4I2C3 = 4, /*!< I2C3 Alternate Function mapping */

    GpioAltFn5SPI1 = 5, /*!< SPI1 Alternate Function mapping */
    GpioAltFn5SPI2 = 5, /*!< SPI2 Alternate Function mapping */

    GpioAltFn6MCO = 6, /*!< MCO Alternate Function mapping */
    GpioAltFn6LSCO = 6, /*!< LSCO Alternate Function mapping */
    GpioAltFn6RF_DTB0 = 6, /*!< RF_DTB0 Alternate Function mapping */
    GpioAltFn6RF_DTB1 = 6, /*!< RF_DTB1 Alternate Function mapping */
    GpioAltFn6RF_DTB2 = 6, /*!< RF_DTB2 Alternate Function mapping */
    GpioAltFn6RF_DTB3 = 6, /*!< RF_DTB3 Alternate Function mapping */
    GpioAltFn6RF_DTB4 = 6, /*!< RF_DTB4 Alternate Function mapping */
    GpioAltFn6RF_DTB5 = 6, /*!< RF_DTB5 Alternate Function mapping */
    GpioAltFn6RF_DTB6 = 6, /*!< RF_DTB6 Alternate Function mapping */
    GpioAltFn6RF_DTB7 = 6, /*!< RF_DTB7 Alternate Function mapping */
    GpioAltFn6RF_DTB8 = 6, /*!< RF_DTB8 Alternate Function mapping */
    GpioAltFn6RF_DTB9 = 6, /*!< RF_DTB9 Alternate Function mapping */
    GpioAltFn6RF_DTB10 = 6, /*!< RF_DTB10 Alternate Function mapping */
    GpioAltFn6RF_DTB11 = 6, /*!< RF_DTB11 Alternate Function mapping */
    GpioAltFn6RF_DTB12 = 6, /*!< RF_DTB12 Alternate Function mapping */
    GpioAltFn6RF_DTB13 = 6, /*!< RF_DTB13 Alternate Function mapping */
    GpioAltFn6RF_DTB14 = 6, /*!< RF_DTB14 Alternate Function mapping */
    GpioAltFn6RF_DTB15 = 6, /*!< RF_DTB15 Alternate Function mapping */
    GpioAltFn6RF_DTB16 = 6, /*!< RF_DTB16 Alternate Function mapping */
    GpioAltFn6RF_DTB17 = 6, /*!< RF_DTB17 Alternate Function mapping */
    GpioAltFn6RF_DTB18 = 6, /*!< RF_DTB18 Alternate Function mapping */
    GpioAltFn6RF_MISO = 6, /*!< RF_MISO Alternate Function mapping */
    GpioAltFn6RF_MOSI = 6, /*!< RF_MOSI Alternate Function mapping */
    GpioAltFn6RF_SCK = 6, /*!< RF_SCK Alternate Function mapping */
    GpioAltFn6RF_NSS = 6, /*!< RF_NSS Alternate Function mapping */

    GpioAltFn7USART1 = 7, /*!< USART1 Alternate Function mapping */

    GpioAltFn8LPUART1 = 8, /*!< LPUART1 Alternate Function mapping */
    GpioAltFn8IR = 8, /*!< IR Alternate Function mapping */

    GpioAltFn9TSC = 9, /*!< TSC Alternate Function mapping */

    GpioAltFn10QUADSPI = 10, /*!< QUADSPI Alternate Function mapping */
    GpioAltFn10USB = 10, /*!< USB Alternate Function mapping */

    GpioAltFn11LCD = 11, /*!< LCD Alternate Function mapping */

    GpioAltFn12COMP1 = 12, /*!< COMP1 Alternate Function mapping */
    GpioAltFn12COMP2 = 12, /*!< COMP2 Alternate Function mapping */
    GpioAltFn12TIM1 = 12, /*!< TIM1 Alternate Function mapping */

    GpioAltFn13SAI1 = 13, /*!< SAI1 Alternate Function mapping */

    GpioAltFn14TIM2 = 14, /*!< TIM2 Alternate Function mapping */
    GpioAltFn14TIM16 = 14, /*!< TIM16 Alternate Function mapping */
    GpioAltFn14TIM17 = 14, /*!< TIM17 Alternate Function mapping */
    GpioAltFn14LPTIM2 = 14, /*!< LPTIM2 Alternate Function mapping */

    GpioAltFn15EVENTOUT = 15, /*!< EVENTOUT Alternate Function mapping */

    GpioAltFnUnused = 16, /*!< just dummy value */
} GpioAltFn;

/**
 * Gpio structure
 */
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} GpioPin;

/**
 * GPIO initialization function, simple version
 * @param gpio  GpioPin
 * @param mode  GpioMode
 */
void furi_hal_gpio_init_simple(const GpioPin* gpio, const GpioMode mode);

/**
 * GPIO initialization function, normal version
 * @param gpio  GpioPin
 * @param mode  GpioMode
 * @param pull  GpioPull
 * @param speed GpioSpeed
 */
void furi_hal_gpio_init(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed);

/**
 * GPIO initialization function, extended version
 * @param gpio  GpioPin
 * @param mode  GpioMode
 * @param pull  GpioPull
 * @param speed GpioSpeed
 * @param alt_fn GpioAltFn
 */
void furi_hal_gpio_init_ex(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed,
    const GpioAltFn alt_fn);

/**
 * Add and enable interrupt
 * @param gpio GpioPin
 * @param cb   GpioExtiCallback
 * @param ctx  context for callback
 */
void furi_hal_gpio_add_int_callback(const GpioPin* gpio, GpioExtiCallback cb, void* ctx);

/**
 * Enable interrupt
 * @param gpio GpioPin
 */
void furi_hal_gpio_enable_int_callback(const GpioPin* gpio);

/**
 * Disable interrupt
 * @param gpio GpioPin
 */
void furi_hal_gpio_disable_int_callback(const GpioPin* gpio);

/**
 * Remove interrupt
 * @param gpio GpioPin
 */
void furi_hal_gpio_remove_int_callback(const GpioPin* gpio);

/**
 * GPIO write pin
 * @param gpio  GpioPin
 * @param state true / false
 */
static inline void furi_hal_gpio_write(const GpioPin* gpio, const bool state) {
    // writing to BSSR is an atomic operation
    if(state == true) {
        gpio->port->BSRR = gpio->pin;
    } else {
        gpio->port->BSRR = (uint32_t)gpio->pin << GPIO_NUMBER;
    }
}

/**
 * GPIO read pin
 * @param port GPIO port
 * @param pin pin mask
 * @return true / false
 */
static inline void
    furi_hal_gpio_write_port_pin(GPIO_TypeDef* port, uint16_t pin, const bool state) {
    // writing to BSSR is an atomic operation
    if(state == true) {
        port->BSRR = pin;
    } else {
        port->BSRR = pin << GPIO_NUMBER;
    }
}

/**
 * GPIO read pin
 * @param gpio GpioPin
 * @return true / false
 */
static inline bool furi_hal_gpio_read(const GpioPin* gpio) {
    if((gpio->port->IDR & gpio->pin) != 0x00U) {
        return true;
    } else {
        return false;
    }
}

/**
 * GPIO read pin
 * @param port GPIO port
 * @param pin pin mask
 * @return true / false
 */
static inline bool furi_hal_gpio_read_port_pin(GPIO_TypeDef* port, uint16_t pin) {
    if((port->IDR & pin) != 0x00U) {
        return true;
    } else {
        return false;
    }
}

#ifdef __cplusplus
}
#endif
