#include <api-hal-rfid.h>
#include <api-hal-ibutton.h>
#include <api-hal-resources.h>

void api_hal_rfid_pins_reset() {
    // ibutton bus disable
    api_hal_ibutton_stop();

    // pulldown rfid antenna
    hal_gpio_init(&gpio_rfid_carrier_out, GpioModeOutputPushPull, GpioSpeedLow, GpioPullNo);
    hal_gpio_write(&gpio_rfid_carrier_out, true);

    // from both sides
    hal_gpio_init(&gpio_rfid_pull, GpioModeOutputPushPull, GpioSpeedLow, GpioPullNo);
    hal_gpio_write(&gpio_rfid_pull, true);
}

void api_hal_rfid_pins_emulate() {
    // ibutton low
    api_hal_ibutton_start();
    api_hal_ibutton_pin_low();

    // pull pin to timer out
    hal_gpio_init_ex(
        &gpio_rfid_pull, GpioModeAltFunctionPushPull, GpioSpeedLow, GpioPullNo, GpioAltFn1TIM1);

    // pull rfid antenna from carrier side
    hal_gpio_init(&gpio_rfid_carrier_out, GpioModeOutputPushPull, GpioSpeedLow, GpioPullNo);
    hal_gpio_write(&gpio_rfid_carrier_out, true);
}

void api_hal_rfid_pins_read() {
    // ibutton low
    api_hal_ibutton_start();
    api_hal_ibutton_pin_low();

    // dont pull rfid antenna
    hal_gpio_init(&gpio_rfid_pull, GpioModeOutputPushPull, GpioSpeedLow, GpioPullNo);
    hal_gpio_write(&gpio_rfid_pull, false);

    // carrier pin to timer out
    hal_gpio_init_ex(
        &gpio_rfid_carrier_out,
        GpioModeAltFunctionPushPull,
        GpioSpeedLow,
        GpioPullNo,
        GpioAltFn1TIM1);

    // comparator in
    hal_gpio_init(&gpio_rfid_data_in, GpioModeAnalog, GpioSpeedLow, GpioPullNo);
}

void api_hal_rfid_tim_read(float freq, float duty_cycle) {
    // TODO LL init
    uint32_t period = (uint32_t)((SystemCoreClock) / freq) - 1;

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    // basic PWM setup with needed freq and internal clock
    LFRFID_TIM.Instance = TIM1;
    LFRFID_TIM.Init.Prescaler = 0;
    LFRFID_TIM.Init.CounterMode = TIM_COUNTERMODE_UP;
    LFRFID_TIM.Init.Period = period;
    LFRFID_TIM.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    LFRFID_TIM.Init.RepetitionCounter = 0;
    LFRFID_TIM.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if(HAL_TIM_Base_Init(&LFRFID_TIM) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if(HAL_TIM_ConfigClockSource(&LFRFID_TIM, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if(HAL_TIM_PWM_Init(&LFRFID_TIM) != HAL_OK) {
        Error_Handler();
    }

    // no master-slave mode
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if(HAL_TIMEx_MasterConfigSynchronization(&LFRFID_TIM, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }

    // pwm config
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = (uint32_t)(LFRFID_TIM.Init.Period * duty_cycle);
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if(HAL_TIM_OC_ConfigChannel(&LFRFID_TIM, &sConfigOC, LFRFID_CH) != HAL_OK) {
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
    if(HAL_TIMEx_ConfigBreakDeadTime(&LFRFID_TIM, &sBreakDeadTimeConfig) != HAL_OK) {
        Error_Handler();
    }
}

void api_hal_rfid_tim_read_start() {
    HAL_TIMEx_PWMN_Start(&LFRFID_TIM, LFRFID_CH);
}

void api_hal_rfid_tim_read_stop() {
    HAL_TIMEx_PWMN_Stop(&LFRFID_TIM, LFRFID_CH);
}

void api_hal_rfid_tim_emulate(float freq) {
    // TODO LL init
    uint32_t prescaler = (uint32_t)((SystemCoreClock) / freq) - 1;

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    // basic PWM setup with needed freq and internal clock
    LFRFID_TIM.Instance = TIM1;
    LFRFID_TIM.Init.Prescaler = prescaler;
    LFRFID_TIM.Init.CounterMode = TIM_COUNTERMODE_UP;
    LFRFID_TIM.Init.Period = 1;
    LFRFID_TIM.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    LFRFID_TIM.Init.RepetitionCounter = 0;
    LFRFID_TIM.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if(HAL_TIM_Base_Init(&LFRFID_TIM) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if(HAL_TIM_ConfigClockSource(&LFRFID_TIM, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if(HAL_TIM_PWM_Init(&LFRFID_TIM) != HAL_OK) {
        Error_Handler();
    }

    // no master-slave mode
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if(HAL_TIMEx_MasterConfigSynchronization(&LFRFID_TIM, &sMasterConfig) != HAL_OK) {
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
    if(HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, LFRFID_CH) != HAL_OK) {
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
    if(HAL_TIMEx_ConfigBreakDeadTime(&LFRFID_TIM, &sBreakDeadTimeConfig) != HAL_OK) {
        Error_Handler();
    }
}

void api_hal_rfid_tim_emulate_start() {
    HAL_TIM_PWM_Start_IT(&LFRFID_TIM, LFRFID_CH);
    HAL_TIM_Base_Start_IT(&LFRFID_TIM);
}

void api_hal_rfid_tim_emulate_stop() {
    HAL_TIM_Base_Stop(&LFRFID_TIM);
    HAL_TIM_PWM_Stop(&LFRFID_TIM, LFRFID_CH);
}

void api_hal_rfid_tim_reset() {
}