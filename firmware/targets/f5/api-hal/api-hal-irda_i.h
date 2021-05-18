#pragma once
#include "api-hal-tim_i.h"

/**
 * Function to handle IRDA timer ISR.
 *
 * @param   source - reason for interrupt request.
 */
void api_hal_irda_tim_isr(TimerIRQSource source);

