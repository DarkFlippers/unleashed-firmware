#pragma once

#include <stdint.h>

#define TIM2 ((void*)0)

typedef enum { LL_TIM_COUNTERMODE_UP } LLCounterMode;

#define LL_TIM_CLOCKSOURCE_INTERNAL  0
#define LL_TIM_UPDATESOURCE_COUNTER  0
#define LL_TIM_ETR_POLARITY_INVERTED 0
#define LL_TIM_ETR_PRESCALER_DIV1    0
#define LL_TIM_ETR_FILTER_FDIV1      0
#define LL_TIM_TS_ETRF               0
#define LL_TIM_SLAVEMODE_RESET       0

#define LL_TIM_SetPrescaler(t, v)     ((void)(v))
#define LL_TIM_SetCounterMode(t, v)   ((void)(v))
#define LL_TIM_SetAutoReload(t, v)    ((void)(v))
#define LL_TIM_SetClockSource(t, v)   ((void)(v))
#define LL_TIM_SetUpdateSource(t, v)  ((void)(v))
#define LL_TIM_ConfigETR(t, a, b, c)  ((void)0)
#define LL_TIM_SetTriggerInput(t, v)  ((void)(v))
#define LL_TIM_SetSlaveMode(t, v)     ((void)(v))
#define LL_TIM_ClearFlag_UPDATE(t)    ((void)0)
#define LL_TIM_ClearFlag_TRIG(t)      ((void)0)
#define LL_TIM_EnableIT_UPDATE(t)     ((void)0)
#define LL_TIM_EnableIT_TRIG(t)       ((void)0)
#define LL_TIM_DisableIT_UPDATE(t)    ((void)0)
#define LL_TIM_DisableIT_TRIG(t)      ((void)0)
#define LL_TIM_EnableCounter(t)       ((void)0)
#define LL_TIM_DisableCounter(t)      ((void)0)
#define LL_TIM_IsActiveFlag_UPDATE(t) (0)
#define LL_TIM_IsEnabledIT_UPDATE(t)  (0)
#define LL_TIM_IsActiveFlag_TRIG(t)   (0)
#define LL_TIM_IsEnabledIT_TRIG(t)    (0)

static struct {
    uint32_t CYCCNT;
} dwt_stub;
#define DWT (&dwt_stub)
