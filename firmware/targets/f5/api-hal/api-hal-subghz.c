#include "api-hal-subghz.h"
#include <stm32wbxx_ll_gpio.h>
#include "main.h"

void api_hal_rf_band_set(RfBand band) {
    if (band == RfBand1) {
        LL_GPIO_ResetOutputPin(PA_SW_0_GPIO_Port, PA_SW_0_Pin);
        LL_GPIO_SetOutputPin(PA_SW_1_GPIO_Port, PA_SW_1_Pin);
    } else if (band == RfBand2) {
        LL_GPIO_SetOutputPin(PA_SW_0_GPIO_Port, PA_SW_0_Pin);
        LL_GPIO_ResetOutputPin(PA_SW_1_GPIO_Port, PA_SW_1_Pin);
    } else if (band == RfBand3) {
        LL_GPIO_SetOutputPin(PA_SW_0_GPIO_Port, PA_SW_0_Pin);
        LL_GPIO_SetOutputPin(PA_SW_1_GPIO_Port, PA_SW_1_Pin);
    } else if (band == RfBandIsolation) {
        LL_GPIO_ResetOutputPin(PA_SW_0_GPIO_Port, PA_SW_0_Pin);
        LL_GPIO_ResetOutputPin(PA_SW_1_GPIO_Port, PA_SW_1_Pin);
    }
}
