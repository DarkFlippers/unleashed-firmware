/**
 ******************************************************************************
  * File Name          : hw_timerserver.c
  * Description        : Hardware timerserver source file for STM32WPAN Middleware.
  *
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "hw_conf.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  TimerID_Free,
  TimerID_Created,
  TimerID_Running
}TimerIDStatus_t;

typedef enum
{
  SSR_Read_Requested,
  SSR_Read_Not_Requested
}RequestReadSSR_t;

typedef enum
{
  WakeupTimerValue_Overpassed,
  WakeupTimerValue_LargeEnough
}WakeupTimerLimitation_Status_t;

typedef struct
{
  HW_TS_pTimerCb_t  pTimerCallBack;
  uint32_t        CounterInit;
  uint32_t        CountLeft;
  TimerIDStatus_t     TimerIDStatus;
  HW_TS_Mode_t   TimerMode;
  uint32_t        TimerProcessID;
  uint8_t         PreviousID;
  uint8_t         NextID;
}TimerContext_t;

/* Private defines -----------------------------------------------------------*/
#define SSR_FORBIDDEN_VALUE   0xFFFFFFFF
#define TIMER_LIST_EMPTY      0xFFFF

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * START of Section TIMERSERVER_CONTEXT
 */

PLACE_IN_SECTION("TIMERSERVER_CONTEXT") static volatile TimerContext_t aTimerContext[CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER];
PLACE_IN_SECTION("TIMERSERVER_CONTEXT") static volatile uint8_t CurrentRunningTimerID;
PLACE_IN_SECTION("TIMERSERVER_CONTEXT") static volatile uint8_t PreviousRunningTimerID;
PLACE_IN_SECTION("TIMERSERVER_CONTEXT") static volatile uint32_t SSRValueOnLastSetup;
PLACE_IN_SECTION("TIMERSERVER_CONTEXT") static volatile WakeupTimerLimitation_Status_t  WakeupTimerLimitation;

/**
 * END of Section TIMERSERVER_CONTEXT
 */

static RTC_HandleTypeDef *phrtc;  /**< RTC handle */
static uint8_t  WakeupTimerDivider;
static uint8_t  AsynchPrescalerUserConfig;
static uint16_t SynchPrescalerUserConfig;
static volatile uint16_t MaxWakeupTimerSetup;

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void RestartWakeupCounter(uint16_t Value);
static uint16_t ReturnTimeElapsed(void);
static void RescheduleTimerList(void);
static void UnlinkTimer(uint8_t TimerID, RequestReadSSR_t RequestReadSSR);
static void LinkTimerBefore(uint8_t TimerID, uint8_t RefTimerID);
static void LinkTimerAfter(uint8_t TimerID, uint8_t RefTimerID);
static uint16_t linkTimer(uint8_t TimerID);
static uint32_t ReadRtcSsrValue(void);

__weak void HW_TS_RTC_CountUpdated_AppNot(void);

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief  Read the RTC_SSR value
 *         As described in the reference manual, the RTC_SSR shall be read twice to ensure
 *         reliability of the value
 * @param  None
 * @retval SSR value read
 */
static uint32_t ReadRtcSsrValue(void)
{
  uint32_t first_read;
  uint32_t second_read;

  first_read = (uint32_t)(READ_BIT(RTC->SSR, RTC_SSR_SS));

  second_read = (uint32_t)(READ_BIT(RTC->SSR, RTC_SSR_SS));

  while(first_read != second_read)
  {
    first_read = second_read;

    second_read = (uint32_t)(READ_BIT(RTC->SSR, RTC_SSR_SS));
  }

  return second_read;
}

/**
 * @brief  Insert a Timer in the list after the Timer ID specified
 * @param  TimerID:   The ID of the Timer
 * @param  RefTimerID: The ID of the Timer to be linked after
 * @retval None
 */
static void LinkTimerAfter(uint8_t TimerID, uint8_t RefTimerID)
{
  uint8_t next_id;

  next_id = aTimerContext[RefTimerID].NextID;

  if(next_id != CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER)
  {
    aTimerContext[next_id].PreviousID = TimerID;
  }
  aTimerContext[TimerID].NextID = next_id;
  aTimerContext[TimerID].PreviousID = RefTimerID ;
  aTimerContext[RefTimerID].NextID = TimerID;

  return;
}

/**
 * @brief  Insert a Timer in the list before the ID specified
 * @param  TimerID:   The ID of the Timer
 * @param  RefTimerID: The ID of the Timer to be linked before
 * @retval None
 */
static void LinkTimerBefore(uint8_t TimerID, uint8_t RefTimerID)
{
  uint8_t previous_id;

  if(RefTimerID != CurrentRunningTimerID)
  {
    previous_id = aTimerContext[RefTimerID].PreviousID;

    aTimerContext[previous_id].NextID = TimerID;
    aTimerContext[TimerID].NextID = RefTimerID;
    aTimerContext[TimerID].PreviousID = previous_id ;
    aTimerContext[RefTimerID].PreviousID = TimerID;
  }
  else
  {
    aTimerContext[TimerID].NextID = RefTimerID;
    aTimerContext[RefTimerID].PreviousID = TimerID;
  }

  return;
}

/**
 * @brief  Insert a Timer in the list
 * @param  TimerID:   The ID of the Timer
 * @retval None
 */
static uint16_t linkTimer(uint8_t TimerID)
{
  uint32_t time_left;
  uint16_t time_elapsed;
  uint8_t timer_id_lookup;
  uint8_t next_id;

  if(CurrentRunningTimerID == CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER)
  {
    /**
     * No timer in the list
     */
    PreviousRunningTimerID = CurrentRunningTimerID;
    CurrentRunningTimerID = TimerID;
    aTimerContext[TimerID].NextID = CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER;

    SSRValueOnLastSetup = SSR_FORBIDDEN_VALUE;
    time_elapsed = 0;
  }
  else
  {
    time_elapsed = ReturnTimeElapsed();

    /**
     * update count of the timer to be linked
     */
    aTimerContext[TimerID].CountLeft += time_elapsed;
    time_left = aTimerContext[TimerID].CountLeft;

    /**
     * Search for index where the new timer shall be linked
     */
    if(aTimerContext[CurrentRunningTimerID].CountLeft <= time_left)
    {
      /**
       * Search for the ID after the first one
       */
      timer_id_lookup = CurrentRunningTimerID;
      next_id = aTimerContext[timer_id_lookup].NextID;
      while((next_id != CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER) && (aTimerContext[next_id].CountLeft <= time_left))
      {
        timer_id_lookup = aTimerContext[timer_id_lookup].NextID;
        next_id = aTimerContext[timer_id_lookup].NextID;
      }

      /**
       * Link after the ID
       */
      LinkTimerAfter(TimerID, timer_id_lookup);
    }
    else
    {
      /**
       * Link before the first ID
       */
      LinkTimerBefore(TimerID, CurrentRunningTimerID);
      PreviousRunningTimerID = CurrentRunningTimerID;
      CurrentRunningTimerID = TimerID;
    }
  }

  return time_elapsed;
}

/**
 * @brief  Remove a Timer from the list
 * @param  TimerID:   The ID of the Timer
 * @param  RequestReadSSR: Request to read the SSR register or not
 * @retval None
 */
static void UnlinkTimer(uint8_t TimerID, RequestReadSSR_t RequestReadSSR)
{
  uint8_t previous_id;
  uint8_t next_id;

  if(TimerID == CurrentRunningTimerID)
  {
    PreviousRunningTimerID = CurrentRunningTimerID;
    CurrentRunningTimerID = aTimerContext[TimerID].NextID;
  }
  else
  {
    previous_id = aTimerContext[TimerID].PreviousID;
    next_id = aTimerContext[TimerID].NextID;

    aTimerContext[previous_id].NextID = aTimerContext[TimerID].NextID;
    if(next_id != CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER)
    {
      aTimerContext[next_id].PreviousID = aTimerContext[TimerID].PreviousID;
    }
  }

  /**
   * Timer is out of the list
   */
  aTimerContext[TimerID].TimerIDStatus = TimerID_Created;

  if((CurrentRunningTimerID == CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER) && (RequestReadSSR == SSR_Read_Requested))
  {
    SSRValueOnLastSetup = SSR_FORBIDDEN_VALUE;
  }

  return;
}

/**
 * @brief  Return the number of ticks counted by the wakeuptimer since it has been started
 * @note  The API is reading the SSR register to get how many ticks have been counted
 *        since the time the timer has been started
 * @param  None
 * @retval Time expired in Ticks
 */
static uint16_t ReturnTimeElapsed(void)
{
  uint32_t  return_value;
  uint32_t  wrap_counter;

  if(SSRValueOnLastSetup != SSR_FORBIDDEN_VALUE)
  {
    return_value = ReadRtcSsrValue(); /**< Read SSR register first */

    if (SSRValueOnLastSetup >= return_value)
    {
      return_value = SSRValueOnLastSetup - return_value;
    }
    else
    {
      wrap_counter = SynchPrescalerUserConfig - return_value;
      return_value = SSRValueOnLastSetup + wrap_counter;
    }

    /**
     * At this stage, ReturnValue holds the number of ticks counted by SSR
     * Need to translate in number of ticks counted by the Wakeuptimer
     */
    return_value = return_value*AsynchPrescalerUserConfig;
    return_value = return_value >> WakeupTimerDivider;
  }
  else
  {
    return_value = 0;
  }

  return (uint16_t)return_value;
}

/**
 * @brief  Set the wakeup counter
 * @note  The API is writing the counter value so that the value is decreased by one to cope with the fact
 *    the interrupt is generated with 1 extra clock cycle (See RefManuel)
 *    It assumes all condition are met to be allowed to write the wakeup counter
 * @param  Value: Value to be written in the counter
 * @retval None
 */
static void RestartWakeupCounter(uint16_t Value)
{
  /**
   * The wakeuptimer has been disabled in the calling function to reduce the time to poll the WUTWF
   * FLAG when the new value will have to be written
   *  __HAL_RTC_WAKEUPTIMER_DISABLE(phrtc);
   */

  if(Value == 0)
  {
    SSRValueOnLastSetup = ReadRtcSsrValue();

    /**
     * Simulate that the Timer expired
     */
    HAL_NVIC_SetPendingIRQ(CFG_HW_TS_RTC_WAKEUP_HANDLER_ID);
  }
  else
  {
    if((Value > 1) ||(WakeupTimerDivider != 1))
    {
      Value -= 1;
    }

    while(__HAL_RTC_WAKEUPTIMER_GET_FLAG(phrtc, RTC_FLAG_WUTWF) == RESET);

    /**
     * make sure to clear the flags after checking the WUTWF.
     * It takes 2 RTCCLK between the time the WUTE bit is disabled and the
     * time the timer is disabled. The WUTWF bit somehow guarantee the system is stable
     * Otherwise, when the timer is periodic with 1 Tick, it may generate an extra interrupt in between
     * due to the autoreload feature
     */
    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(phrtc, RTC_FLAG_WUTF);   /**<  Clear flag in RTC module */
    __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG(); /**<  Clear flag in EXTI module */
    HAL_NVIC_ClearPendingIRQ(CFG_HW_TS_RTC_WAKEUP_HANDLER_ID);   /**<  Clear pending bit in NVIC */

    MODIFY_REG(RTC->WUTR, RTC_WUTR_WUT, Value);

    /**
     * Update the value here after the WUTWF polling that may take some time
     */
    SSRValueOnLastSetup = ReadRtcSsrValue();

    __HAL_RTC_WAKEUPTIMER_ENABLE(phrtc);    /**<  Enable the Wakeup Timer */

    HW_TS_RTC_CountUpdated_AppNot();
  }

  return ;
}

/**
 * @brief  Reschedule the list of timer
 * @note  1) Update the count left for each timer in the list
 *    2) Setup the wakeuptimer
 * @param  None
 * @retval None
 */
static void RescheduleTimerList(void)
{
  uint8_t   localTimerID;
  uint32_t  timecountleft;
  uint16_t  wakeup_timer_value;
  uint16_t  time_elapsed;

  /**
   * The wakeuptimer is disabled now to reduce the time to poll the WUTWF
   * FLAG when the new value will have to be written
   */
  if((READ_BIT(RTC->CR, RTC_CR_WUTE) == (RTC_CR_WUTE)) == SET)
  {
    /**
     * Wait for the flag to be back to 0 when the wakeup timer is enabled
     */
    while(__HAL_RTC_WAKEUPTIMER_GET_FLAG(phrtc, RTC_FLAG_WUTWF) == SET);
  }
  __HAL_RTC_WAKEUPTIMER_DISABLE(phrtc);   /**<  Disable the Wakeup Timer */

  localTimerID = CurrentRunningTimerID;

  /**
   * Calculate what will be the value to write in the wakeuptimer
   */
  timecountleft = aTimerContext[localTimerID].CountLeft;

  /**
   * Read how much has been counted
   */
  time_elapsed = ReturnTimeElapsed();

  if(timecountleft < time_elapsed )
  {
    /**
     * There is no tick left to count
     */
    wakeup_timer_value = 0;
    WakeupTimerLimitation = WakeupTimerValue_LargeEnough;
  }
  else
  {
    if(timecountleft > (time_elapsed + MaxWakeupTimerSetup))
    {
      /**
       * The number of tick left is greater than the Wakeuptimer maximum value
       */
      wakeup_timer_value = MaxWakeupTimerSetup;

      WakeupTimerLimitation = WakeupTimerValue_Overpassed;
    }
    else
    {
      wakeup_timer_value = timecountleft - time_elapsed;
      WakeupTimerLimitation = WakeupTimerValue_LargeEnough;
    }

  }

  /**
   * update ticks left to be counted for each timer
   */
  while(localTimerID != CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER)
  {
    if (aTimerContext[localTimerID].CountLeft < time_elapsed)
    {
      aTimerContext[localTimerID].CountLeft = 0;
    }
    else
    {
      aTimerContext[localTimerID].CountLeft -= time_elapsed;
    }
    localTimerID = aTimerContext[localTimerID].NextID;
  }

  /**
   * Write next count
   */
  RestartWakeupCounter(wakeup_timer_value);

  return ;
}

/* Public functions ----------------------------------------------------------*/

/**
 * For all public interface except that may need write access to the RTC, the RTC
 * shall be unlock at the beginning and locked at the output
 * In order to ease maintainability, the unlock is done at the top and the lock at then end
 * in case some new implementation is coming in the future
 */

void HW_TS_RTC_Wakeup_Handler(void)
{
  HW_TS_pTimerCb_t ptimer_callback;
  uint32_t timer_process_id;
  uint8_t local_current_running_timer_id;
#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
  uint32_t primask_bit;
#endif

#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
  primask_bit = __get_PRIMASK();  /**< backup PRIMASK bit */
  __disable_irq();          /**< Disable all interrupts by setting PRIMASK bit on Cortex*/
#endif

/* Disable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_DISABLE( phrtc );

  /**
   * Disable the Wakeup Timer
   * This may speed up a bit the processing to wait the timer to be disabled
   * The timer is still counting 2 RTCCLK
   */
  __HAL_RTC_WAKEUPTIMER_DISABLE(phrtc);

  local_current_running_timer_id = CurrentRunningTimerID;

  if(aTimerContext[local_current_running_timer_id].TimerIDStatus == TimerID_Running)
  {
    ptimer_callback = aTimerContext[local_current_running_timer_id].pTimerCallBack;
    timer_process_id = aTimerContext[local_current_running_timer_id].TimerProcessID;

    /**
     * It should be good to check whether the TimeElapsed is greater or not than the tick left to be counted
     * However, due to the inaccuracy of the reading of the time elapsed, it may return there is 1 tick
     * to be left whereas the count is over
     * A more secure implementation has been done with a flag to state whereas the full count has been written
     * in the wakeuptimer or not
     */
    if(WakeupTimerLimitation != WakeupTimerValue_Overpassed)
    {
      if(aTimerContext[local_current_running_timer_id].TimerMode == hw_ts_Repeated)
      {
        UnlinkTimer(local_current_running_timer_id, SSR_Read_Not_Requested);
#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
        __set_PRIMASK(primask_bit); /**< Restore PRIMASK bit*/
#endif
        HW_TS_Start(local_current_running_timer_id, aTimerContext[local_current_running_timer_id].CounterInit);

        /* Disable the write protection for RTC registers */
        __HAL_RTC_WRITEPROTECTION_DISABLE( phrtc );
        }
      else
      {
#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
        __set_PRIMASK(primask_bit); /**< Restore PRIMASK bit*/
#endif
        HW_TS_Stop(local_current_running_timer_id);

        /* Disable the write protection for RTC registers */
        __HAL_RTC_WRITEPROTECTION_DISABLE( phrtc );
        }

      HW_TS_RTC_Int_AppNot(timer_process_id, local_current_running_timer_id, ptimer_callback);
    }
    else
    {
      RescheduleTimerList();
#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
      __set_PRIMASK(primask_bit); /**< Restore PRIMASK bit*/
#endif
    }
  }
  else
  {
    /**
     * We should never end up in this case
     * However, if due to any bug in the timer server this is the case, the mistake may not impact the user.
     * We could just clean the interrupt flag and get out from this unexpected interrupt
     */
    while(__HAL_RTC_WAKEUPTIMER_GET_FLAG(phrtc, RTC_FLAG_WUTWF) == RESET);

    /**
     * make sure to clear the flags after checking the WUTWF.
     * It takes 2 RTCCLK between the time the WUTE bit is disabled and the
     * time the timer is disabled. The WUTWF bit somehow guarantee the system is stable
     * Otherwise, when the timer is periodic with 1 Tick, it may generate an extra interrupt in between
     * due to the autoreload feature
     */
    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(phrtc, RTC_FLAG_WUTF);   /**<  Clear flag in RTC module */
    __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG(); /**<  Clear flag in EXTI module */

#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
    __set_PRIMASK(primask_bit); /**< Restore PRIMASK bit*/
#endif
  }

  /* Enable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_ENABLE( phrtc );

  return;
}

void HW_TS_Init(HW_TS_InitMode_t TimerInitMode, RTC_HandleTypeDef *hrtc)
{
  uint8_t loop;
  uint32_t localmaxwakeuptimersetup;

  /**
   * Get RTC handler
   */
  phrtc = hrtc;

 /* Disable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_DISABLE( phrtc );

  SET_BIT(RTC->CR, RTC_CR_BYPSHAD);

  /**
   * Readout the user config
   */
  WakeupTimerDivider = (4 - ((uint32_t)(READ_BIT(RTC->CR, RTC_CR_WUCKSEL))));

  AsynchPrescalerUserConfig = (uint8_t)(READ_BIT(RTC->PRER, RTC_PRER_PREDIV_A) >> (uint32_t)POSITION_VAL(RTC_PRER_PREDIV_A)) + 1;

  SynchPrescalerUserConfig = (uint16_t)(READ_BIT(RTC->PRER, RTC_PRER_PREDIV_S)) + 1;

  /**
   *  Margin is taken to avoid wrong calculation when the wrap around is there and some
   *  application interrupts may have delayed the reading
   */
  localmaxwakeuptimersetup = ((((SynchPrescalerUserConfig - 1)*AsynchPrescalerUserConfig) - CFG_HW_TS_RTC_HANDLER_MAX_DELAY) >> WakeupTimerDivider);

  if(localmaxwakeuptimersetup >= 0xFFFF)
  {
    MaxWakeupTimerSetup = 0xFFFF;
  }
  else
  {
    MaxWakeupTimerSetup = (uint16_t)localmaxwakeuptimersetup;
  }

  /**
   * Configure EXTI module
   */
  LL_EXTI_EnableRisingTrig_0_31(RTC_EXTI_LINE_WAKEUPTIMER_EVENT);
  LL_EXTI_EnableIT_0_31(RTC_EXTI_LINE_WAKEUPTIMER_EVENT);

  if(TimerInitMode == hw_ts_InitMode_Full)
  {
    WakeupTimerLimitation = WakeupTimerValue_LargeEnough;
    SSRValueOnLastSetup = SSR_FORBIDDEN_VALUE;

    /**
     * Initialize the timer server
     */
    for(loop = 0; loop < CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER; loop++)
    {
      aTimerContext[loop].TimerIDStatus = TimerID_Free;
    }

    CurrentRunningTimerID = CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER;   /**<  Set ID to non valid value */

    __HAL_RTC_WAKEUPTIMER_DISABLE(phrtc);                       /**<  Disable the Wakeup Timer */
    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(phrtc, RTC_FLAG_WUTF);     /**<  Clear flag in RTC module */
    __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG(); /**<  Clear flag in EXTI module  */
    HAL_NVIC_ClearPendingIRQ(CFG_HW_TS_RTC_WAKEUP_HANDLER_ID);       /**<  Clear pending bit in NVIC  */
    __HAL_RTC_WAKEUPTIMER_ENABLE_IT(phrtc, RTC_IT_WUT);         /**<  Enable interrupt in RTC module  */
  }
  else
  {
    if(__HAL_RTC_WAKEUPTIMER_GET_FLAG(phrtc, RTC_FLAG_WUTF) != RESET)
    {
      /**
       * Simulate that the Timer expired
       */
      HAL_NVIC_SetPendingIRQ(CFG_HW_TS_RTC_WAKEUP_HANDLER_ID);
    }
  }

  /* Enable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_ENABLE( phrtc );

  HAL_NVIC_SetPriority(CFG_HW_TS_RTC_WAKEUP_HANDLER_ID, CFG_HW_TS_NVIC_RTC_WAKEUP_IT_PREEMPTPRIO, CFG_HW_TS_NVIC_RTC_WAKEUP_IT_SUBPRIO);   /**<  Set NVIC priority */
  HAL_NVIC_EnableIRQ(CFG_HW_TS_RTC_WAKEUP_HANDLER_ID); /**<  Enable NVIC */

  return;
}

HW_TS_ReturnStatus_t HW_TS_Create(uint32_t TimerProcessID, uint8_t *pTimerId, HW_TS_Mode_t TimerMode, HW_TS_pTimerCb_t pftimeout_handler)
{
  HW_TS_ReturnStatus_t localreturnstatus;
  uint8_t loop = 0;
#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
  uint32_t primask_bit;
#endif

#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
  primask_bit = __get_PRIMASK();  /**< backup PRIMASK bit */
  __disable_irq();          /**< Disable all interrupts by setting PRIMASK bit on Cortex*/
#endif

  while((loop < CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER) && (aTimerContext[loop].TimerIDStatus != TimerID_Free))
  {
    loop++;
  }

  if(loop != CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER)
  {
    aTimerContext[loop].TimerIDStatus = TimerID_Created;

#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
    __set_PRIMASK(primask_bit); /**< Restore PRIMASK bit*/
#endif

    aTimerContext[loop].TimerProcessID = TimerProcessID;
    aTimerContext[loop].TimerMode = TimerMode;
    aTimerContext[loop].pTimerCallBack = pftimeout_handler;
    *pTimerId = loop;

    localreturnstatus = hw_ts_Successful;
  }
  else
  {
#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
    __set_PRIMASK(primask_bit); /**< Restore PRIMASK bit*/
#endif

    localreturnstatus = hw_ts_Failed;
  }

  return(localreturnstatus);
}

void HW_TS_Delete(uint8_t timer_id)
{
  HW_TS_Stop(timer_id);

  aTimerContext[timer_id].TimerIDStatus = TimerID_Free; /**<  release ID */

  return;
}

void HW_TS_Stop(uint8_t timer_id)
{
  uint8_t localcurrentrunningtimerid;

#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
  uint32_t primask_bit;
#endif

#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
  primask_bit = __get_PRIMASK();  /**< backup PRIMASK bit */
  __disable_irq();          /**< Disable all interrupts by setting PRIMASK bit on Cortex*/
#endif

  HAL_NVIC_DisableIRQ(CFG_HW_TS_RTC_WAKEUP_HANDLER_ID);    /**<  Disable NVIC */

  /* Disable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_DISABLE( phrtc );

  if(aTimerContext[timer_id].TimerIDStatus == TimerID_Running)
  {
    UnlinkTimer(timer_id, SSR_Read_Requested);
    localcurrentrunningtimerid = CurrentRunningTimerID;

    if(localcurrentrunningtimerid == CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER)
    {
      /**
       * List is empty
       */

      /**
       * Disable the timer
       */
      if((READ_BIT(RTC->CR, RTC_CR_WUTE) == (RTC_CR_WUTE)) == SET)
      {
        /**
         * Wait for the flag to be back to 0 when the wakeup timer is enabled
         */
        while(__HAL_RTC_WAKEUPTIMER_GET_FLAG(phrtc, RTC_FLAG_WUTWF) == SET);
      }
      __HAL_RTC_WAKEUPTIMER_DISABLE(phrtc);   /**<  Disable the Wakeup Timer */

      while(__HAL_RTC_WAKEUPTIMER_GET_FLAG(phrtc, RTC_FLAG_WUTWF) == RESET);

      /**
       * make sure to clear the flags after checking the WUTWF.
       * It takes 2 RTCCLK between the time the WUTE bit is disabled and the
       * time the timer is disabled. The WUTWF bit somehow guarantee the system is stable
       * Otherwise, when the timer is periodic with 1 Tick, it may generate an extra interrupt in between
       * due to the autoreload feature
       */
      __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(phrtc, RTC_FLAG_WUTF);   /**<  Clear flag in RTC module */
      __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG(); /**<  Clear flag in EXTI module */
      HAL_NVIC_ClearPendingIRQ(CFG_HW_TS_RTC_WAKEUP_HANDLER_ID);   /**<  Clear pending bit in NVIC */
    }
    else if(PreviousRunningTimerID != localcurrentrunningtimerid)
    {
      RescheduleTimerList();
    }
  }

  /* Enable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_ENABLE( phrtc );

  HAL_NVIC_EnableIRQ(CFG_HW_TS_RTC_WAKEUP_HANDLER_ID); /**<  Enable NVIC */

#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
  __set_PRIMASK(primask_bit); /**< Restore PRIMASK bit*/
#endif

  return;
}

void HW_TS_Start(uint8_t timer_id, uint32_t timeout_ticks)
{
  uint16_t time_elapsed;
  uint8_t localcurrentrunningtimerid;

#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
  uint32_t primask_bit;
#endif

  if(aTimerContext[timer_id].TimerIDStatus == TimerID_Running)
  {
    HW_TS_Stop( timer_id );
  }

#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
  primask_bit = __get_PRIMASK();  /**< backup PRIMASK bit */
  __disable_irq();          /**< Disable all interrupts by setting PRIMASK bit on Cortex*/
#endif

  HAL_NVIC_DisableIRQ(CFG_HW_TS_RTC_WAKEUP_HANDLER_ID);    /**<  Disable NVIC */

  /* Disable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_DISABLE( phrtc );

  aTimerContext[timer_id].TimerIDStatus = TimerID_Running;

  aTimerContext[timer_id].CountLeft = timeout_ticks;
  aTimerContext[timer_id].CounterInit = timeout_ticks;

  time_elapsed =  linkTimer(timer_id);

  localcurrentrunningtimerid = CurrentRunningTimerID;

  if(PreviousRunningTimerID != localcurrentrunningtimerid)
  {
    RescheduleTimerList();
  }
  else
  {
    aTimerContext[timer_id].CountLeft -= time_elapsed;
  }

  /* Enable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_ENABLE( phrtc );

  HAL_NVIC_EnableIRQ(CFG_HW_TS_RTC_WAKEUP_HANDLER_ID); /**<  Enable NVIC */

#if (CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION == 1)
  __set_PRIMASK(primask_bit); /**< Restore PRIMASK bit*/
#endif

  return;
}

uint16_t HW_TS_RTC_ReadLeftTicksToCount(void)
{
  uint32_t primask_bit;
  uint16_t return_value, auro_reload_value, elapsed_time_value;

  primask_bit = __get_PRIMASK();  /**< backup PRIMASK bit */
  __disable_irq();                /**< Disable all interrupts by setting PRIMASK bit on Cortex*/

  if((READ_BIT(RTC->CR, RTC_CR_WUTE) == (RTC_CR_WUTE)) == SET)
  {
    auro_reload_value = (uint32_t)(READ_BIT(RTC->WUTR, RTC_WUTR_WUT));

    elapsed_time_value = ReturnTimeElapsed();

    if(auro_reload_value > elapsed_time_value)
    {
      return_value = auro_reload_value - elapsed_time_value;
    }
    else
    {
      return_value = 0;
    }
  }
  else
  {
    return_value = TIMER_LIST_EMPTY;
  }

  __set_PRIMASK(primask_bit);     /**< Restore PRIMASK bit*/

  return (return_value);
}

__weak void HW_TS_RTC_Int_AppNot(uint32_t TimerProcessID, uint8_t TimerID, HW_TS_pTimerCb_t pTimerCallBack)
{
  pTimerCallBack();

  return;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
