/******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/
/*
 *      PROJECT:   ST25R391x firmware
 *      $Revision: $
 *      LANGUAGE:  ANSI C
 */

/*! \file timer.c
 *
 *  \brief SW Timer implementation
 *
 *  \author Gustavo Patricio
 *
 *   This module makes use of a System Tick in millisconds and provides
 *   an abstraction for SW timers
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "timer.h"
#include <furi.h>

/*
******************************************************************************
* LOCAL DEFINES
******************************************************************************
*/

/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/

static uint32_t timerStopwatchTick;

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
uint32_t timerCalculateTimer(uint16_t time) {
    return (furi_get_tick() + time);
}

/*******************************************************************************/
bool timerIsExpired(uint32_t timer) {
    uint32_t uDiff;
    int32_t sDiff;

    uDiff = (timer - furi_get_tick()); /* Calculate the diff between the timers */
    sDiff = uDiff; /* Convert the diff to a signed var      */

    /* Check if the given timer has expired already */
    if(sDiff < 0) {
        return true;
    }

    return false;
}

/*******************************************************************************/
void timerDelay(uint16_t tOut) {
    uint32_t t;

    /* Calculate the timer and wait blocking until is running */
    t = timerCalculateTimer(tOut);
    while(timerIsRunning(t))
        ;
}

/*******************************************************************************/
void timerStopwatchStart(void) {
    timerStopwatchTick = furi_get_tick();
}

/*******************************************************************************/
uint32_t timerStopwatchMeasure(void) {
    return (uint32_t)(furi_get_tick() - timerStopwatchTick);
}
