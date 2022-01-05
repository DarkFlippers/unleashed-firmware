#include <furi_hal_rfid.h>
#include <furi_hal_ibutton.h>
#include <furi_hal_resources.h>

#include <stm32wbxx_ll_tim.h>

#define LFRFID_READ_TIM htim1
#define LFRFID_READ_CHANNEL TIM_CHANNEL_1
#define LFRFID_EMULATE_TIM htim2
#define LFRFID_EMULATE_CHANNEL TIM_CHANNEL_3

void furi_hal_rfid_init() {
    furi_hal_rfid_pins_reset();
}

void furi_hal_rfid_pins_reset() {
    // ibutton bus disable
    furi_hal_ibutton_stop();

    // pulldown rfid antenna
    hal_gpio_init(&gpio_rfid_carrier_out, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_rfid_carrier_out, false);

    // from both sides
    hal_gpio_init(&gpio_rfid_pull, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_rfid_pull, true);

    hal_gpio_init_simple(&gpio_rfid_carrier, GpioModeAnalog);
}

void furi_hal_rfid_pins_emulate() {
    // ibutton low
    furi_hal_ibutton_start();
    furi_hal_ibutton_pin_low();

    // pull pin to timer out
    hal_gpio_init_ex(
        &gpio_rfid_pull, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn1TIM2);

    // pull rfid antenna from carrier side
    hal_gpio_init(&gpio_rfid_carrier_out, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_rfid_carrier_out, false);

    hal_gpio_init_ex(
        &gpio_rfid_carrier, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn2TIM2);
}

void furi_hal_rfid_pins_read() {
    // ibutton low
    furi_hal_ibutton_start();
    furi_hal_ibutton_pin_low();

    // dont pull rfid antenna
    hal_gpio_init(&gpio_rfid_pull, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_rfid_pull, false);

    // carrier pin to timer out
    hal_gpio_init_ex(
        &gpio_rfid_carrier_out,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        GpioSpeedLow,
        GpioAltFn1TIM1);

    // comparator in
    hal_gpio_init(&gpio_rfid_data_in, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void furi_hal_rfid_pin_pull_release() {
    hal_gpio_write(&gpio_rfid_pull, true);
}

void furi_hal_rfid_pin_pull_pulldown() {
    hal_gpio_write(&gpio_rfid_pull, false);
}

void furi_hal_rfid_tim_read(float freq, float duty_cycle) {
    // TODO LL init
    uint32_t period = (uint32_t)((SystemCoreClock) / freq) - 1;

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    // basic PWM setup with needed freq and internal clock
    LFRFID_READ_TIM.Init.Prescaler = 0;
    LFRFID_READ_TIM.Init.CounterMode = TIM_COUNTERMODE_UP;
    LFRFID_READ_TIM.Init.Period = period;
    LFRFID_READ_TIM.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    LFRFID_READ_TIM.Init.RepetitionCounter = 0;
    LFRFID_READ_TIM.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if(HAL_TIM_Base_Init(&LFRFID_READ_TIM) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if(HAL_TIM_ConfigClockSource(&LFRFID_READ_TIM, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if(HAL_TIM_PWM_Init(&LFRFID_READ_TIM) != HAL_OK) {
        Error_Handler();
    }

    // no master-slave mode
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if(HAL_TIMEx_MasterConfigSynchronization(&LFRFID_READ_TIM, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }

    // pwm config
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = (uint32_t)(LFRFID_READ_TIM.Init.Period * duty_cycle);
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if(HAL_TIM_PWM_ConfigChannel(&LFRFID_READ_TIM, &sConfigOC, LFRFID_READ_CHANNEL) != HAL_OK) {
        Error_Handler();
    }

    // no deadtime
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.BreakFilter = 0;
    sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
    sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
    sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
    sBreakDeadTimeConfig.Break2Filter = 0;
    sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    if(HAL_TIMEx_ConfigBreakDeadTime(&LFRFID_READ_TIM, &sBreakDeadTimeConfig) != HAL_OK) {
        Error_Handler();
    }
}

void furi_hal_rfid_tim_read_start() {
    HAL_TIMEx_PWMN_Start(&LFRFID_READ_TIM, LFRFID_READ_CHANNEL);
}

void furi_hal_rfid_tim_read_stop() {
    HAL_TIMEx_PWMN_Stop(&LFRFID_READ_TIM, LFRFID_READ_CHANNEL);
}

void furi_hal_rfid_tim_emulate(float freq) {
    // TODO LL init
    // uint32_t prescaler = (uint32_t)((SystemCoreClock) / freq) - 1;

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    // basic PWM setup with needed freq and internal clock
    LFRFID_EMULATE_TIM.Init.Prescaler = 0;
    LFRFID_EMULATE_TIM.Init.CounterMode = TIM_COUNTERMODE_UP;
    LFRFID_EMULATE_TIM.Init.Period = 1;
    LFRFID_EMULATE_TIM.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    LFRFID_EMULATE_TIM.Init.RepetitionCounter = 0;
    LFRFID_EMULATE_TIM.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if(HAL_TIM_Base_Init(&LFRFID_EMULATE_TIM) != HAL_OK) {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
    sClockSourceConfig.ClockPolarity = TIM_ETRPOLARITY_INVERTED;
    sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
    sClockSourceConfig.ClockFilter = 0;
    if(HAL_TIM_ConfigClockSource(&LFRFID_EMULATE_TIM, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if(HAL_TIM_PWM_Init(&LFRFID_EMULATE_TIM) != HAL_OK) {
        Error_Handler();
    }

    // no master-slave mode
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if(HAL_TIMEx_MasterConfigSynchronization(&LFRFID_EMULATE_TIM, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }

    // pwm config
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 1;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if(HAL_TIM_PWM_ConfigChannel(&LFRFID_EMULATE_TIM, &sConfigOC, LFRFID_EMULATE_CHANNEL) !=
       HAL_OK) {
        Error_Handler();
    }
}

void furi_hal_rfid_tim_emulate_start() {
    // TODO make api for interrupts priority
    for(size_t i = WWDG_IRQn; i <= DMAMUX1_OVR_IRQn; i++) {
        HAL_NVIC_SetPriority(i, 15, 0);
    }

    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    HAL_TIM_PWM_Start_IT(&LFRFID_EMULATE_TIM, LFRFID_EMULATE_CHANNEL);
    HAL_TIM_Base_Start_IT(&LFRFID_EMULATE_TIM);
}

void furi_hal_rfid_tim_emulate_stop() {
    HAL_TIM_Base_Stop(&LFRFID_EMULATE_TIM);
    HAL_TIM_PWM_Stop(&LFRFID_EMULATE_TIM, LFRFID_EMULATE_CHANNEL);
}

void furi_hal_rfid_tim_reset() {
    HAL_TIM_Base_DeInit(&LFRFID_READ_TIM);
    LL_TIM_DeInit(TIM1);
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM1);
    HAL_TIM_Base_DeInit(&LFRFID_EMULATE_TIM);
    LL_TIM_DeInit(TIM2);
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM2);
}

bool furi_hal_rfid_is_tim_emulate(TIM_HandleTypeDef* hw) {
    return (hw == &LFRFID_EMULATE_TIM);
}

void furi_hal_rfid_set_emulate_period(uint32_t period) {
    LFRFID_EMULATE_TIM.Instance->ARR = period;
}

void furi_hal_rfid_set_emulate_pulse(uint32_t pulse) {
    switch(LFRFID_EMULATE_CHANNEL) {
    case TIM_CHANNEL_1:
        LFRFID_EMULATE_TIM.Instance->CCR1 = pulse;
        break;
    case TIM_CHANNEL_2:
        LFRFID_EMULATE_TIM.Instance->CCR2 = pulse;
        break;
    case TIM_CHANNEL_3:
        LFRFID_EMULATE_TIM.Instance->CCR3 = pulse;
        break;
    case TIM_CHANNEL_4:
        LFRFID_EMULATE_TIM.Instance->CCR4 = pulse;
        break;
    default:
        furi_crash(NULL);
        break;
    }
}

void furi_hal_rfid_set_read_period(uint32_t period) {
    LFRFID_TIM.Instance->ARR = period;
}

void furi_hal_rfid_set_read_pulse(uint32_t pulse) {
    switch(LFRFID_READ_CHANNEL) {
    case TIM_CHANNEL_1:
        LFRFID_TIM.Instance->CCR1 = pulse;
        break;
    case TIM_CHANNEL_2:
        LFRFID_TIM.Instance->CCR2 = pulse;
        break;
    case TIM_CHANNEL_3:
        LFRFID_TIM.Instance->CCR3 = pulse;
        break;
    case TIM_CHANNEL_4:
        LFRFID_TIM.Instance->CCR4 = pulse;
        break;
    default:
        furi_crash(NULL);
        break;
    }
}

void furi_hal_rfid_change_read_config(float freq, float duty_cycle) {
    uint32_t period = (uint32_t)((SystemCoreClock) / freq) - 1;
    furi_hal_rfid_set_read_period(period);
    furi_hal_rfid_set_read_pulse(period * duty_cycle);
}
