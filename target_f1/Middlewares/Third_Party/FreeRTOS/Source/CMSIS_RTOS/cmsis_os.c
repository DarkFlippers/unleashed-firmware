/* ----------------------------------------------------------------------
 * $Date:        5. February 2013
 * $Revision:    V1.02
 *
 * Project:      CMSIS-RTOS API
 * Title:        cmsis_os.c
 *
 * Version 0.02
 *    Initial Proposal Phase
 * Version 0.03
 *    osKernelStart added, optional feature: main started as thread
 *    osSemaphores have standard behavior
 *    osTimerCreate does not start the timer, added osTimerStart
 *    osThreadPass is renamed to osThreadYield
 * Version 1.01
 *    Support for C++ interface
 *     - const attribute removed from the osXxxxDef_t typedef's
 *     - const attribute added to the osXxxxDef macros
 *    Added: osTimerDelete, osMutexDelete, osSemaphoreDelete
 *    Added: osKernelInitialize
 * Version 1.02
 *    Control functions for short timeouts in microsecond resolution:
 *    Added: osKernelSysTick, osKernelSysTickFrequency, osKernelSysTickMicroSec
 *    Removed: osSignalGet 
 *    
 *  
 *----------------------------------------------------------------------------
 *
 * Portions Copyright © 2016 STMicroelectronics International N.V. All rights reserved.
 * Portions Copyright (c) 2013 ARM LIMITED
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - Neither the name of ARM  nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/

#include <string.h>
#include "cmsis_os.h"

/*
 * ARM Compiler 4/5
 */
#if   defined ( __CC_ARM )

  #define __ASM            __asm                                      
  #define __INLINE         __inline                                     
  #define __STATIC_INLINE  static __inline

  #include "cmsis_armcc.h"

/*
 * GNU Compiler
 */
#elif defined ( __GNUC__ )

  #define __ASM            __asm                                      /*!< asm keyword for GNU Compiler          */
  #define __INLINE         inline                                     /*!< inline keyword for GNU Compiler       */
  #define __STATIC_INLINE  static inline

  #include "cmsis_gcc.h"


/*
 * IAR Compiler
 */
#elif defined ( __ICCARM__ )

  #ifndef   __ASM
    #define __ASM                     __asm
  #endif
  #ifndef   __INLINE
    #define __INLINE                  inline
  #endif
  #ifndef   __STATIC_INLINE
    #define __STATIC_INLINE           static inline
  #endif

  #include <cmsis_iar.h>
#endif

extern void xPortSysTickHandler(void);

/* Convert from CMSIS type osPriority to FreeRTOS priority number */
static unsigned portBASE_TYPE makeFreeRtosPriority (osPriority priority)
{
  unsigned portBASE_TYPE fpriority = tskIDLE_PRIORITY;
  
  if (priority != osPriorityError) {
    fpriority += (priority - osPriorityIdle);
  }
  
  return fpriority;
}

#if (INCLUDE_uxTaskPriorityGet == 1)
/* Convert from FreeRTOS priority number to CMSIS type osPriority */
static osPriority makeCmsisPriority (unsigned portBASE_TYPE fpriority)
{
  osPriority priority = osPriorityError;
  
  if ((fpriority - tskIDLE_PRIORITY) <= (osPriorityRealtime - osPriorityIdle)) {
    priority = (osPriority)((int)osPriorityIdle + (int)(fpriority - tskIDLE_PRIORITY));
  }
  
  return priority;
}
#endif


/* Determine whether we are in thread mode or handler mode. */
static int inHandlerMode (void)
{
  return __get_IPSR() != 0;
}

/*********************** Kernel Control Functions *****************************/
/**
* @brief  Initialize the RTOS Kernel for creating objects.
* @retval status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osKernelInitialize shall be consistent in every CMSIS-RTOS.
*/
osStatus osKernelInitialize (void);

/**
* @brief  Start the RTOS Kernel with executing the specified thread.
* @param  thread_def    thread definition referenced with \ref osThread.
* @param  argument      pointer that is passed to the thread function as start argument.
* @retval status code that indicates the execution status of the function
* @note   MUST REMAIN UNCHANGED: \b osKernelStart shall be consistent in every CMSIS-RTOS.
*/
osStatus osKernelStart (void)
{
  vTaskStartScheduler();
  
  return osOK;
}

/**
* @brief  Check if the RTOS kernel is already started
* @param  None
* @retval (0) RTOS is not started
*         (1) RTOS is started
*         (-1) if this feature is disabled in FreeRTOSConfig.h 
* @note  MUST REMAIN UNCHANGED: \b osKernelRunning shall be consistent in every CMSIS-RTOS.
*/
int32_t osKernelRunning(void)
{
#if ( ( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ) )
  if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
    return 0;
  else
    return 1;
#else
	return (-1);
#endif	
}

#if (defined (osFeature_SysTick)  &&  (osFeature_SysTick != 0))     // System Timer available
/**
* @brief  Get the value of the Kernel SysTick timer
* @param  None
* @retval None
* @note   MUST REMAIN UNCHANGED: \b osKernelSysTick shall be consistent in every CMSIS-RTOS.
*/
uint32_t osKernelSysTick(void)
{
  if (inHandlerMode()) {
    return xTaskGetTickCountFromISR();
  }
  else {
    return xTaskGetTickCount();
  }
}
#endif    // System Timer available
/*********************** Thread Management *****************************/
/**
* @brief  Create a thread and add it to Active Threads and set it to state READY.
* @param  thread_def    thread definition referenced with \ref osThread.
* @param  argument      pointer that is passed to the thread function as start argument.
* @retval thread ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osThreadCreate shall be consistent in every CMSIS-RTOS.
*/
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument)
{
  TaskHandle_t handle;
  
#if( configSUPPORT_STATIC_ALLOCATION == 1 ) &&  ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
  if((thread_def->buffer != NULL) && (thread_def->controlblock != NULL)) {
    handle = xTaskCreateStatic((TaskFunction_t)thread_def->pthread,(const portCHAR *)thread_def->name,
              thread_def->stacksize, argument, makeFreeRtosPriority(thread_def->tpriority),
              thread_def->buffer, thread_def->controlblock);
  }
  else {
    if (xTaskCreate((TaskFunction_t)thread_def->pthread,(const portCHAR *)thread_def->name,
              thread_def->stacksize, argument, makeFreeRtosPriority(thread_def->tpriority),
              &handle) != pdPASS)  {
      return NULL;
    } 
  }
#elif( configSUPPORT_STATIC_ALLOCATION == 1 )

    handle = xTaskCreateStatic((TaskFunction_t)thread_def->pthread,(const portCHAR *)thread_def->name,
              thread_def->stacksize, argument, makeFreeRtosPriority(thread_def->tpriority),
              thread_def->buffer, thread_def->controlblock);
#else
  if (xTaskCreate((TaskFunction_t)thread_def->pthread,(const portCHAR *)thread_def->name,
                   thread_def->stacksize, argument, makeFreeRtosPriority(thread_def->tpriority),
                   &handle) != pdPASS)  {
    return NULL;
  }     
#endif
  
  return handle;
}

/**
* @brief  Return the thread ID of the current running thread.
* @retval thread ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osThreadGetId shall be consistent in every CMSIS-RTOS.
*/
osThreadId osThreadGetId (void)
{
#if ( ( INCLUDE_xTaskGetCurrentTaskHandle == 1 ) || ( configUSE_MUTEXES == 1 ) )
  return xTaskGetCurrentTaskHandle();
#else
	return NULL;
#endif
}

/**
* @brief  Terminate execution of a thread and remove it from Active Threads.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osThreadTerminate shall be consistent in every CMSIS-RTOS.
*/
osStatus osThreadTerminate (osThreadId thread_id)
{
#if (INCLUDE_vTaskDelete == 1)
  vTaskDelete(thread_id);
  return osOK;
#else
  return osErrorOS;
#endif
}

/**
* @brief  Pass control to next thread that is in state \b READY.
* @retval status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osThreadYield shall be consistent in every CMSIS-RTOS.
*/
osStatus osThreadYield (void)
{
  taskYIELD();
  
  return osOK;
}

/**
* @brief   Change priority of an active thread.
* @param   thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @param   priority      new priority value for the thread function.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osThreadSetPriority shall be consistent in every CMSIS-RTOS.
*/
osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority)
{
#if (INCLUDE_vTaskPrioritySet == 1)
  vTaskPrioritySet(thread_id, makeFreeRtosPriority(priority));
  return osOK;
#else
  return osErrorOS;
#endif
}

/**
* @brief   Get current priority of an active thread.
* @param   thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  current priority value of the thread function.
* @note   MUST REMAIN UNCHANGED: \b osThreadGetPriority shall be consistent in every CMSIS-RTOS.
*/
osPriority osThreadGetPriority (osThreadId thread_id)
{
#if (INCLUDE_uxTaskPriorityGet == 1)
  if (inHandlerMode())
  {
    return makeCmsisPriority(uxTaskPriorityGetFromISR(thread_id));  
  }
  else
  {  
    return makeCmsisPriority(uxTaskPriorityGet(thread_id));
  }
#else
  return osPriorityError;
#endif
}

/*********************** Generic Wait Functions *******************************/
/**
* @brief   Wait for Timeout (Time Delay)
* @param   millisec      time delay value
* @retval  status code that indicates the execution status of the function.
*/
osStatus osDelay (uint32_t millisec)
{
#if INCLUDE_vTaskDelay
  TickType_t ticks = millisec / portTICK_PERIOD_MS;
  
  vTaskDelay(ticks ? ticks : 1);          /* Minimum delay = 1 tick */
  
  return osOK;
#else
  (void) millisec;
  
  return osErrorResource;
#endif
}

#if (defined (osFeature_Wait)  &&  (osFeature_Wait != 0)) /* Generic Wait available */
/**
* @brief  Wait for Signal, Message, Mail, or Timeout
* @param   millisec  timeout value or 0 in case of no time-out
* @retval  event that contains signal, message, or mail information or error code.
* @note   MUST REMAIN UNCHANGED: \b osWait shall be consistent in every CMSIS-RTOS.
*/
osEvent osWait (uint32_t millisec);

#endif  /* Generic Wait available */

/***********************  Timer Management Functions ***************************/
/**
* @brief  Create a timer.
* @param  timer_def     timer object referenced with \ref osTimer.
* @param  type          osTimerOnce for one-shot or osTimerPeriodic for periodic behavior.
* @param  argument      argument to the timer call back function.
* @retval  timer ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osTimerCreate shall be consistent in every CMSIS-RTOS.
*/
osTimerId osTimerCreate (const osTimerDef_t *timer_def, os_timer_type type, void *argument)
{
#if (configUSE_TIMERS == 1)

#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) ) 
  if(timer_def->controlblock != NULL) {
    return xTimerCreateStatic((const char *)"",
                      1, // period should be filled when starting the Timer using osTimerStart
                      (type == osTimerPeriodic) ? pdTRUE : pdFALSE,
                      (void *) argument,
                      (TaskFunction_t)timer_def->ptimer,
                      (StaticTimer_t *)timer_def->controlblock);
  }
  else {
    return xTimerCreate((const char *)"",
                      1, // period should be filled when starting the Timer using osTimerStart
                      (type == osTimerPeriodic) ? pdTRUE : pdFALSE,
                      (void *) argument,
                      (TaskFunction_t)timer_def->ptimer);
 }
#elif( configSUPPORT_STATIC_ALLOCATION == 1 )
  return xTimerCreateStatic((const char *)"",
                      1, // period should be filled when starting the Timer using osTimerStart
                      (type == osTimerPeriodic) ? pdTRUE : pdFALSE,
                      (void *) argument,
                      (TaskFunction_t)timer_def->ptimer,
                      (StaticTimer_t *)timer_def->controlblock);  
#else
  return xTimerCreate((const char *)"",
                      1, // period should be filled when starting the Timer using osTimerStart
                      (type == osTimerPeriodic) ? pdTRUE : pdFALSE,
                      (void *) argument,
                      (TaskFunction_t)timer_def->ptimer);
#endif

#else 
	return NULL;
#endif
}

/**
* @brief  Start or restart a timer.
* @param  timer_id      timer ID obtained by \ref osTimerCreate.
* @param  millisec      time delay value of the timer.
* @retval  status code that indicates the execution status of the function
* @note   MUST REMAIN UNCHANGED: \b osTimerStart shall be consistent in every CMSIS-RTOS.
*/
osStatus osTimerStart (osTimerId timer_id, uint32_t millisec)
{
  osStatus result = osOK;
#if (configUSE_TIMERS == 1)  
  portBASE_TYPE taskWoken = pdFALSE;
  TickType_t ticks = millisec / portTICK_PERIOD_MS;

  if (ticks == 0)
    ticks = 1;
    
  if (inHandlerMode()) 
  {
    if (xTimerChangePeriodFromISR(timer_id, ticks, &taskWoken) != pdPASS)
    {
      result = osErrorOS;
    }
    else
    {
      portEND_SWITCHING_ISR(taskWoken);     
    }
  }
  else 
  {
    if (xTimerChangePeriod(timer_id, ticks, 0) != pdPASS)
      result = osErrorOS;
  }

#else 
  result = osErrorOS;
#endif
  return result;
}

/**
* @brief  Stop a timer.
* @param  timer_id      timer ID obtained by \ref osTimerCreate
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osTimerStop shall be consistent in every CMSIS-RTOS.
*/
osStatus osTimerStop (osTimerId timer_id)
{
  osStatus result = osOK;
#if (configUSE_TIMERS == 1)  
  portBASE_TYPE taskWoken = pdFALSE;

  if (inHandlerMode()) {
    if (xTimerStopFromISR(timer_id, &taskWoken) != pdPASS) {
      return osErrorOS;
    }
    portEND_SWITCHING_ISR(taskWoken);
  }
  else {
    if (xTimerStop(timer_id, 0) != pdPASS) {
      result = osErrorOS;
    }
  }
#else 
  result = osErrorOS;
#endif 
  return result;
}

/**
* @brief  Delete a timer.
* @param  timer_id      timer ID obtained by \ref osTimerCreate
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osTimerDelete shall be consistent in every CMSIS-RTOS.
*/
osStatus osTimerDelete (osTimerId timer_id)
{
osStatus result = osOK;

#if (configUSE_TIMERS == 1)

   if (inHandlerMode()) {
     return osErrorISR;
  }
  else { 
    if ((xTimerDelete(timer_id, osWaitForever )) != pdPASS) {
      result = osErrorOS;
    }
  } 
    
#else 
  result = osErrorOS;
#endif 
 
  return result;
}

/***************************  Signal Management ********************************/
/**
* @brief  Set the specified Signal Flags of an active thread.
* @param  thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @param  signals       specifies the signal flags of the thread that should be set.
* @retval previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters.
* @note   MUST REMAIN UNCHANGED: \b osSignalSet shall be consistent in every CMSIS-RTOS.
*/
int32_t osSignalSet (osThreadId thread_id, int32_t signal)
{
#if( configUSE_TASK_NOTIFICATIONS == 1 )	
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  uint32_t ulPreviousNotificationValue = 0;
  
  if (inHandlerMode())
  {
    if(xTaskGenericNotifyFromISR( thread_id , (uint32_t)signal, eSetBits, &ulPreviousNotificationValue, &xHigherPriorityTaskWoken ) != pdPASS )
      return 0x80000000;
    
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
  }  
  else if(xTaskGenericNotify( thread_id , (uint32_t)signal, eSetBits, &ulPreviousNotificationValue) != pdPASS )
    return 0x80000000;
  
  return ulPreviousNotificationValue;
#else
  (void) thread_id;
  (void) signal;

  return 0x80000000; /* Task Notification not supported */ 	
#endif
}

/**
* @brief  Clear the specified Signal Flags of an active thread.
* @param  thread_id  thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @param  signals    specifies the signal flags of the thread that shall be cleared.
* @retval  previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters.
* @note   MUST REMAIN UNCHANGED: \b osSignalClear shall be consistent in every CMSIS-RTOS.
*/
int32_t osSignalClear (osThreadId thread_id, int32_t signal);

/**
* @brief  Wait for one or more Signal Flags to become signaled for the current \b RUNNING thread.
* @param  signals   wait until all specified signal flags set or 0 for any single signal flag.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval  event flag information or error code.
* @note   MUST REMAIN UNCHANGED: \b osSignalWait shall be consistent in every CMSIS-RTOS.
*/
osEvent osSignalWait (int32_t signals, uint32_t millisec)
{
  osEvent ret;

#if( configUSE_TASK_NOTIFICATIONS == 1 )
	
  TickType_t ticks;

  ret.value.signals = 0;  
  ticks = 0;
  if (millisec == osWaitForever) {
    ticks = portMAX_DELAY;
  }
  else if (millisec != 0) {
    ticks = millisec / portTICK_PERIOD_MS;
    if (ticks == 0) {
      ticks = 1;
    }
  }  
  
  if (inHandlerMode())
  {
    ret.status = osErrorISR;  /*Not allowed in ISR*/
  }
  else
  {
    if(xTaskNotifyWait( 0,(uint32_t) signals, (uint32_t *)&ret.value.signals, ticks) != pdTRUE)
    {
      if(ticks == 0)  ret.status = osOK;
      else  ret.status = osEventTimeout;
    }
    else if(ret.value.signals < 0)
    {
      ret.status =  osErrorValue;     
    }
    else  ret.status =  osEventSignal;
  }
#else
  (void) signals;
  (void) millisec;
	
  ret.status =  osErrorOS;	/* Task Notification not supported */
#endif
  
  return ret;
}

/****************************  Mutex Management ********************************/
/**
* @brief  Create and Initialize a Mutex object
* @param  mutex_def     mutex definition referenced with \ref osMutex.
* @retval  mutex ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osMutexCreate shall be consistent in every CMSIS-RTOS.
*/
osMutexId osMutexCreate (const osMutexDef_t *mutex_def)
{
#if ( configUSE_MUTEXES == 1)

#if( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )

  if (mutex_def->controlblock != NULL) {
    return xSemaphoreCreateMutexStatic( mutex_def->controlblock );
     }
  else {
    return xSemaphoreCreateMutex(); 
  }
#elif ( configSUPPORT_STATIC_ALLOCATION == 1 )
  return xSemaphoreCreateMutexStatic( mutex_def->controlblock );
#else  
    return xSemaphoreCreateMutex(); 
#endif
#else
  return NULL;
#endif
}

/**
* @brief Wait until a Mutex becomes available
* @param mutex_id      mutex ID obtained by \ref osMutexCreate.
* @param millisec      timeout value or 0 in case of no time-out.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMutexWait shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexWait (osMutexId mutex_id, uint32_t millisec)
{
  TickType_t ticks;
  portBASE_TYPE taskWoken = pdFALSE;  
  
  
  if (mutex_id == NULL) {
    return osErrorParameter;
  }
  
  ticks = 0;
  if (millisec == osWaitForever) {
    ticks = portMAX_DELAY;
  }
  else if (millisec != 0) {
    ticks = millisec / portTICK_PERIOD_MS;
    if (ticks == 0) {
      ticks = 1;
    }
  }
  
  if (inHandlerMode()) {
    if (xSemaphoreTakeFromISR(mutex_id, &taskWoken) != pdTRUE) {
      return osErrorOS;
    }
	portEND_SWITCHING_ISR(taskWoken);
  } 
  else if (xSemaphoreTake(mutex_id, ticks) != pdTRUE) {
    return osErrorOS;
  }
  
  return osOK;
}

/**
* @brief Release a Mutex that was obtained by \ref osMutexWait
* @param mutex_id      mutex ID obtained by \ref osMutexCreate.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMutexRelease shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexRelease (osMutexId mutex_id)
{
  osStatus result = osOK;
  portBASE_TYPE taskWoken = pdFALSE;
  
  if (inHandlerMode()) {
    if (xSemaphoreGiveFromISR(mutex_id, &taskWoken) != pdTRUE) {
      return osErrorOS;
    }
    portEND_SWITCHING_ISR(taskWoken);
  }
  else if (xSemaphoreGive(mutex_id) != pdTRUE) 
  {
    result = osErrorOS;
  }
  return result;
}

/**
* @brief Delete a Mutex
* @param mutex_id  mutex ID obtained by \ref osMutexCreate.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMutexDelete shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexDelete (osMutexId mutex_id)
{
  if (inHandlerMode()) {
    return osErrorISR;
  }

  vQueueDelete(mutex_id);

  return osOK;
}

/********************  Semaphore Management Functions **************************/

#if (defined (osFeature_Semaphore)  &&  (osFeature_Semaphore != 0))

/**
* @brief Create and Initialize a Semaphore object used for managing resources
* @param semaphore_def semaphore definition referenced with \ref osSemaphore.
* @param count         number of available resources.
* @retval  semaphore ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osSemaphoreCreate shall be consistent in every CMSIS-RTOS.
*/
osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count)
{ 
#if( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )

  osSemaphoreId sema;
  
  if (semaphore_def->controlblock != NULL){
    if (count == 1) {
      return xSemaphoreCreateBinaryStatic( semaphore_def->controlblock );
    }
    else {
#if (configUSE_COUNTING_SEMAPHORES == 1 )
      return xSemaphoreCreateCountingStatic( count, count, semaphore_def->controlblock );
#else
      return NULL;
#endif
    }
  }
  else {
    if (count == 1) {
      vSemaphoreCreateBinary(sema);
      return sema;
    }
    else {
#if (configUSE_COUNTING_SEMAPHORES == 1 )	
      return xSemaphoreCreateCounting(count, count);
#else
      return NULL;
#endif    
    }
  }
#elif ( configSUPPORT_STATIC_ALLOCATION == 1 ) // configSUPPORT_DYNAMIC_ALLOCATION == 0
  if(count == 1) {
    return xSemaphoreCreateBinaryStatic( semaphore_def->controlblock );
  }
  else
  {
#if (configUSE_COUNTING_SEMAPHORES == 1 )
      return xSemaphoreCreateCountingStatic( count, count, semaphore_def->controlblock );
#else
      return NULL;
#endif    
  }
#else  // configSUPPORT_STATIC_ALLOCATION == 0  && configSUPPORT_DYNAMIC_ALLOCATION == 1
  osSemaphoreId sema;
 
  if (count == 1) {
    vSemaphoreCreateBinary(sema);
    return sema;
  }
  else {
#if (configUSE_COUNTING_SEMAPHORES == 1 )	
    return xSemaphoreCreateCounting(count, count);
#else
    return NULL;
#endif
  }
#endif
}

/**
* @brief Wait until a Semaphore token becomes available
* @param  semaphore_id  semaphore object referenced with \ref osSemaphore.
* @param  millisec      timeout value or 0 in case of no time-out.
* @retval  number of available tokens, or -1 in case of incorrect parameters.
* @note   MUST REMAIN UNCHANGED: \b osSemaphoreWait shall be consistent in every CMSIS-RTOS.
*/
int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec)
{
  TickType_t ticks;
  portBASE_TYPE taskWoken = pdFALSE;  
  
  
  if (semaphore_id == NULL) {
    return osErrorParameter;
  }
  
  ticks = 0;
  if (millisec == osWaitForever) {
    ticks = portMAX_DELAY;
  }
  else if (millisec != 0) {
    ticks = millisec / portTICK_PERIOD_MS;
    if (ticks == 0) {
      ticks = 1;
    }
  }
  
  if (inHandlerMode()) {
    if (xSemaphoreTakeFromISR(semaphore_id, &taskWoken) != pdTRUE) {
      return osErrorOS;
    }
	portEND_SWITCHING_ISR(taskWoken);
  }  
  else if (xSemaphoreTake(semaphore_id, ticks) != pdTRUE) {
    return osErrorOS;
  }
  
  return osOK;
}

/**
* @brief Release a Semaphore token
* @param  semaphore_id  semaphore object referenced with \ref osSemaphore.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osSemaphoreRelease shall be consistent in every CMSIS-RTOS.
*/
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id)
{
  osStatus result = osOK;
  portBASE_TYPE taskWoken = pdFALSE;
  
  
  if (inHandlerMode()) {
    if (xSemaphoreGiveFromISR(semaphore_id, &taskWoken) != pdTRUE) {
      return osErrorOS;
    }
    portEND_SWITCHING_ISR(taskWoken);
  }
  else {
    if (xSemaphoreGive(semaphore_id) != pdTRUE) {
      result = osErrorOS;
    }
  }
  
  return result;
}

/**
* @brief Delete a Semaphore
* @param  semaphore_id  semaphore object referenced with \ref osSemaphore.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osSemaphoreDelete shall be consistent in every CMSIS-RTOS.
*/
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id)
{
  if (inHandlerMode()) {
    return osErrorISR;
  }

  vSemaphoreDelete(semaphore_id);

  return osOK; 
}

#endif    /* Use Semaphores */

/*******************   Memory Pool Management Functions  ***********************/

#if (defined (osFeature_Pool)  &&  (osFeature_Pool != 0)) 

//TODO
//This is a primitive and inefficient wrapper around the existing FreeRTOS memory management.
//A better implementation will have to modify heap_x.c!


typedef struct os_pool_cb {
  void *pool;
  uint8_t *markers;
  uint32_t pool_sz;
  uint32_t item_sz;
  uint32_t currentIndex;
} os_pool_cb_t;


/**
* @brief Create and Initialize a memory pool
* @param  pool_def      memory pool definition referenced with \ref osPool.
* @retval  memory pool ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osPoolCreate shall be consistent in every CMSIS-RTOS.
*/
osPoolId osPoolCreate (const osPoolDef_t *pool_def)
{
#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
  osPoolId thePool;
  int itemSize = 4 * ((pool_def->item_sz + 3) / 4);
  uint32_t i;
  
  /* First have to allocate memory for the pool control block. */
 thePool = pvPortMalloc(sizeof(os_pool_cb_t));

  
  if (thePool) {
    thePool->pool_sz = pool_def->pool_sz;
    thePool->item_sz = itemSize;
    thePool->currentIndex = 0;
    
    /* Memory for markers */
    thePool->markers = pvPortMalloc(pool_def->pool_sz);
   
    if (thePool->markers) {
      /* Now allocate the pool itself. */
     thePool->pool = pvPortMalloc(pool_def->pool_sz * itemSize);
      
      if (thePool->pool) {
        for (i = 0; i < pool_def->pool_sz; i++) {
          thePool->markers[i] = 0;
        }
      }
      else {
        vPortFree(thePool->markers);
        vPortFree(thePool);
        thePool = NULL;
      }
    }
    else {
      vPortFree(thePool);
      thePool = NULL;
    }
  }

  return thePool;
 
#else
  return NULL;
#endif
}

/**
* @brief Allocate a memory block from a memory pool
* @param pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
* @retval  address of the allocated memory block or NULL in case of no memory available.
* @note   MUST REMAIN UNCHANGED: \b osPoolAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osPoolAlloc (osPoolId pool_id)
{
  int dummy = 0;
  void *p = NULL;
  uint32_t i;
  uint32_t index;
  
  if (inHandlerMode()) {
    dummy = portSET_INTERRUPT_MASK_FROM_ISR();
  }
  else {
    vPortEnterCritical();
  }
  
  for (i = 0; i < pool_id->pool_sz; i++) {
    index = (pool_id->currentIndex + i) % pool_id->pool_sz;
    
    if (pool_id->markers[index] == 0) {
      pool_id->markers[index] = 1;
      p = (void *)((uint32_t)(pool_id->pool) + (index * pool_id->item_sz));
      pool_id->currentIndex = index;
      break;
    }
  }
  
  if (inHandlerMode()) {
    portCLEAR_INTERRUPT_MASK_FROM_ISR(dummy);
  }
  else {
    vPortExitCritical();
  }
  
  return p;
}

/**
* @brief Allocate a memory block from a memory pool and set memory block to zero
* @param  pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
* @retval  address of the allocated memory block or NULL in case of no memory available.
* @note   MUST REMAIN UNCHANGED: \b osPoolCAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osPoolCAlloc (osPoolId pool_id)
{
  void *p = osPoolAlloc(pool_id);
  
  if (p != NULL)
  {
    memset(p, 0, sizeof(pool_id->pool_sz));
  }
  
  return p;
}

/**
* @brief Return an allocated memory block back to a specific memory pool
* @param  pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
* @param  block         address of the allocated memory block that is returned to the memory pool.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osPoolFree shall be consistent in every CMSIS-RTOS.
*/
osStatus osPoolFree (osPoolId pool_id, void *block)
{
  uint32_t index;
  
  if (pool_id == NULL) {
    return osErrorParameter;
  }
  
  if (block == NULL) {
    return osErrorParameter;
  }
  
  if (block < pool_id->pool) {
    return osErrorParameter;
  }
  
  index = (uint32_t)block - (uint32_t)(pool_id->pool);
  if (index % pool_id->item_sz) {
    return osErrorParameter;
  }
  index = index / pool_id->item_sz;
  if (index >= pool_id->pool_sz) {
    return osErrorParameter;
  }
  
  pool_id->markers[index] = 0;
  
  return osOK;
}


#endif   /* Use Memory Pool Management */

/*******************   Message Queue Management Functions  *********************/

#if (defined (osFeature_MessageQ)  &&  (osFeature_MessageQ != 0)) /* Use Message Queues */

/**
* @brief Create and Initialize a Message Queue
* @param queue_def     queue definition referenced with \ref osMessageQ.
* @param  thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
* @retval  message queue ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osMessageCreate shall be consistent in every CMSIS-RTOS.
*/
osMessageQId osMessageCreate (const osMessageQDef_t *queue_def, osThreadId thread_id)
{
  (void) thread_id;
  
#if( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )

  if ((queue_def->buffer != NULL) && (queue_def->controlblock != NULL)) {
    return xQueueCreateStatic(queue_def->queue_sz, queue_def->item_sz, queue_def->buffer, queue_def->controlblock);
  }
  else {
    return xQueueCreate(queue_def->queue_sz, queue_def->item_sz);
  }
#elif ( configSUPPORT_STATIC_ALLOCATION == 1 )
  return xQueueCreateStatic(queue_def->queue_sz, queue_def->item_sz, queue_def->buffer, queue_def->controlblock);
#else  
  return xQueueCreate(queue_def->queue_sz, queue_def->item_sz);
#endif
}

/**
* @brief Put a Message to a Queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @param  info      message information.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMessagePut shall be consistent in every CMSIS-RTOS.
*/
osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec)
{
  portBASE_TYPE taskWoken = pdFALSE;
  TickType_t ticks;
  
  ticks = millisec / portTICK_PERIOD_MS;
  if (ticks == 0) {
    ticks = 1;
  }
  
  if (inHandlerMode()) {
    if (xQueueSendFromISR(queue_id, &info, &taskWoken) != pdTRUE) {
      return osErrorOS;
    }
    portEND_SWITCHING_ISR(taskWoken);
  }
  else {
    if (xQueueSend(queue_id, &info, ticks) != pdTRUE) {
      return osErrorOS;
    }
  }
  
  return osOK;
}

/**
* @brief Get a Message or Wait for a Message from a Queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval event information that includes status code.
* @note   MUST REMAIN UNCHANGED: \b osMessageGet shall be consistent in every CMSIS-RTOS.
*/
osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec)
{
  portBASE_TYPE taskWoken;
  TickType_t ticks;
  osEvent event;
  
  event.def.message_id = queue_id;
  event.value.v = 0;
  
  if (queue_id == NULL) {
    event.status = osErrorParameter;
    return event;
  }
  
  taskWoken = pdFALSE;
  
  ticks = 0;
  if (millisec == osWaitForever) {
    ticks = portMAX_DELAY;
  }
  else if (millisec != 0) {
    ticks = millisec / portTICK_PERIOD_MS;
    if (ticks == 0) {
      ticks = 1;
    }
  }
  
  if (inHandlerMode()) {
    if (xQueueReceiveFromISR(queue_id, &event.value.v, &taskWoken) == pdTRUE) {
      /* We have mail */
      event.status = osEventMessage;
    }
    else {
      event.status = osOK;
    }
    portEND_SWITCHING_ISR(taskWoken);
  }
  else {
    if (xQueueReceive(queue_id, &event.value.v, ticks) == pdTRUE) {
      /* We have mail */
      event.status = osEventMessage;
    }
    else {
      event.status = (ticks == 0) ? osOK : osEventTimeout;
    }
  }
  
  return event;
}

#endif     /* Use Message Queues */

/********************   Mail Queue Management Functions  ***********************/
#if (defined (osFeature_MailQ)  &&  (osFeature_MailQ != 0))  /* Use Mail Queues */


typedef struct os_mailQ_cb {
  const osMailQDef_t *queue_def;
  QueueHandle_t handle;
  osPoolId pool;
} os_mailQ_cb_t;

/**
* @brief Create and Initialize mail queue
* @param  queue_def     reference to the mail queue definition obtain with \ref osMailQ
* @param   thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
* @retval mail queue ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osMailCreate shall be consistent in every CMSIS-RTOS.
*/
osMailQId osMailCreate (const osMailQDef_t *queue_def, osThreadId thread_id)
{
#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
  (void) thread_id;
  
  osPoolDef_t pool_def = {queue_def->queue_sz, queue_def->item_sz, NULL};
  
  /* Create a mail queue control block */

  *(queue_def->cb) = pvPortMalloc(sizeof(struct os_mailQ_cb));

  if (*(queue_def->cb) == NULL) {
    return NULL;
  }
  (*(queue_def->cb))->queue_def = queue_def;
  
  /* Create a queue in FreeRTOS */
  (*(queue_def->cb))->handle = xQueueCreate(queue_def->queue_sz, sizeof(void *));


  if ((*(queue_def->cb))->handle == NULL) {
    vPortFree(*(queue_def->cb));
    return NULL;
  }
  
  /* Create a mail pool */
  (*(queue_def->cb))->pool = osPoolCreate(&pool_def);
  if ((*(queue_def->cb))->pool == NULL) {
    //TODO: Delete queue. How to do it in FreeRTOS?
    vPortFree(*(queue_def->cb));
    return NULL;
  }
  
  return *(queue_def->cb);
#else
  return NULL;
#endif
}

/**
* @brief Allocate a memory block from a mail
* @param  queue_id      mail queue ID obtained with \ref osMailCreate.
* @param  millisec      timeout value or 0 in case of no time-out.
* @retval pointer to memory block that can be filled with mail or NULL in case error.
* @note   MUST REMAIN UNCHANGED: \b osMailAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osMailAlloc (osMailQId queue_id, uint32_t millisec)
{
  (void) millisec;
  void *p;
  
  
  if (queue_id == NULL) {
    return NULL;
  }
  
  p = osPoolAlloc(queue_id->pool);
  
  return p;
}

/**
* @brief Allocate a memory block from a mail and set memory block to zero
* @param  queue_id      mail queue ID obtained with \ref osMailCreate.
* @param  millisec      timeout value or 0 in case of no time-out.
* @retval pointer to memory block that can be filled with mail or NULL in case error.
* @note   MUST REMAIN UNCHANGED: \b osMailCAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osMailCAlloc (osMailQId queue_id, uint32_t millisec)
{
  uint32_t i;
  void *p = osMailAlloc(queue_id, millisec);
  
  if (p) {
    for (i = 0; i < queue_id->queue_def->item_sz; i++) {
      ((uint8_t *)p)[i] = 0;
    }
  }
  
  return p;
}

/**
* @brief Put a mail to a queue
* @param  queue_id      mail queue ID obtained with \ref osMailCreate.
* @param  mail          memory block previously allocated with \ref osMailAlloc or \ref osMailCAlloc.
* @retval status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMailPut shall be consistent in every CMSIS-RTOS.
*/
osStatus osMailPut (osMailQId queue_id, void *mail)
{
  portBASE_TYPE taskWoken;
  
  
  if (queue_id == NULL) {
    return osErrorParameter;
  }
  
  taskWoken = pdFALSE;
  
  if (inHandlerMode()) {
    if (xQueueSendFromISR(queue_id->handle, &mail, &taskWoken) != pdTRUE) {
      return osErrorOS;
    }
    portEND_SWITCHING_ISR(taskWoken);
  }
  else {
    if (xQueueSend(queue_id->handle, &mail, 0) != pdTRUE) { 
      return osErrorOS;
    }
  }
  
  return osOK;
}

/**
* @brief Get a mail from a queue
* @param  queue_id   mail queue ID obtained with \ref osMailCreate.
* @param millisec    timeout value or 0 in case of no time-out
* @retval event that contains mail information or error code.
* @note   MUST REMAIN UNCHANGED: \b osMailGet shall be consistent in every CMSIS-RTOS.
*/
osEvent osMailGet (osMailQId queue_id, uint32_t millisec)
{
  portBASE_TYPE taskWoken;
  TickType_t ticks;
  osEvent event;
  
  event.def.mail_id = queue_id;
  
  if (queue_id == NULL) {
    event.status = osErrorParameter;
    return event;
  }
  
  taskWoken = pdFALSE;
  
  ticks = 0;
  if (millisec == osWaitForever) {
    ticks = portMAX_DELAY;
  }
  else if (millisec != 0) {
    ticks = millisec / portTICK_PERIOD_MS;
    if (ticks == 0) {
      ticks = 1;
    }
  }
  
  if (inHandlerMode()) {
    if (xQueueReceiveFromISR(queue_id->handle, &event.value.p, &taskWoken) == pdTRUE) {
      /* We have mail */
      event.status = osEventMail;
    }
    else {
      event.status = osOK;
    }
    portEND_SWITCHING_ISR(taskWoken);
  }
  else {
    if (xQueueReceive(queue_id->handle, &event.value.p, ticks) == pdTRUE) {
      /* We have mail */
      event.status = osEventMail;
    }
    else {
      event.status = (ticks == 0) ? osOK : osEventTimeout;
    }
  }
  
  return event;
}

/**
* @brief Free a memory block from a mail
* @param  queue_id mail queue ID obtained with \ref osMailCreate.
* @param  mail     pointer to the memory block that was obtained with \ref osMailGet.
* @retval status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMailFree shall be consistent in every CMSIS-RTOS.
*/
osStatus osMailFree (osMailQId queue_id, void *mail)
{
  if (queue_id == NULL) {
    return osErrorParameter;
  }
  
  return osPoolFree(queue_id->pool, mail);
}
#endif  /* Use Mail Queues */

/*************************** Additional specific APIs to Free RTOS ************/
/**
* @brief  Handles the tick increment
* @param  none.
* @retval none.
*/
void osSystickHandler(void)
{

#if (INCLUDE_xTaskGetSchedulerState  == 1 )
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
#endif  /* INCLUDE_xTaskGetSchedulerState */  
    xPortSysTickHandler();
#if (INCLUDE_xTaskGetSchedulerState  == 1 )
  }
#endif  /* INCLUDE_xTaskGetSchedulerState */  
}

#if ( INCLUDE_eTaskGetState == 1 )
/**
* @brief  Obtain the state of any thread.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  the stae of the thread, states are encoded by the osThreadState enumerated type.
*/
osThreadState osThreadGetState(osThreadId thread_id)
{
  eTaskState ThreadState;
  osThreadState result;
  
  ThreadState = eTaskGetState(thread_id);
  
  switch (ThreadState)
  {
  case eRunning :
    result = osThreadRunning;
    break;
  case eReady :
    result = osThreadReady;
    break;
  case eBlocked :
    result = osThreadBlocked;
    break;
  case eSuspended :
    result = osThreadSuspended;
    break;
  case eDeleted :
    result = osThreadDeleted;
    break;
  default:
    result = osThreadError;
  } 
  
  return result;
}
#endif /* INCLUDE_eTaskGetState */

#if (INCLUDE_eTaskGetState == 1)
/**
* @brief Check if a thread is already suspended or not.
* @param thread_id thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval status code that indicates the execution status of the function.
*/
osStatus osThreadIsSuspended(osThreadId thread_id)
{
  if (eTaskGetState(thread_id) == eSuspended)
    return osOK;
  else
    return osErrorOS;
}
#endif /* INCLUDE_eTaskGetState */
/**
* @brief  Suspend execution of a thread.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadSuspend (osThreadId thread_id)
{
#if (INCLUDE_vTaskSuspend == 1)
    vTaskSuspend(thread_id);
  
  return osOK;
#else
  return osErrorResource;
#endif
}

/**
* @brief  Resume execution of a suspended thread.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadResume (osThreadId thread_id)
{
#if (INCLUDE_vTaskSuspend == 1)  
  if(inHandlerMode())
  {
    if (xTaskResumeFromISR(thread_id) == pdTRUE)
    {
      portYIELD_FROM_ISR(pdTRUE);
    }
  }
  else
  {
    vTaskResume(thread_id);
  }
  return osOK;
#else
  return osErrorResource;
#endif
}

/**
* @brief  Suspend execution of a all active threads.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadSuspendAll (void)
{
  vTaskSuspendAll();
  
  return osOK;
}

/**
* @brief  Resume execution of a all suspended threads.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadResumeAll (void)
{
  if (xTaskResumeAll() == pdTRUE)
    return osOK;
  else
    return osErrorOS;
  
}

/**
* @brief  Delay a task until a specified time
* @param   PreviousWakeTime   Pointer to a variable that holds the time at which the 
*          task was last unblocked. PreviousWakeTime must be initialised with the current time
*          prior to its first use (PreviousWakeTime = osKernelSysTick() )
* @param   millisec    time delay value
* @retval  status code that indicates the execution status of the function.
*/
osStatus osDelayUntil (uint32_t *PreviousWakeTime, uint32_t millisec)
{
#if INCLUDE_vTaskDelayUntil
  TickType_t ticks = (millisec / portTICK_PERIOD_MS);
  vTaskDelayUntil((TickType_t *) PreviousWakeTime, ticks ? ticks : 1);
  
  return osOK;
#else
  (void) millisec;
  (void) PreviousWakeTime;
  
  return osErrorResource;
#endif
}

/**
* @brief   Abort the delay for a specific thread
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId   
* @retval  status code that indicates the execution status of the function.
*/
osStatus osAbortDelay(osThreadId thread_id)
{
#if INCLUDE_xTaskAbortDelay
  
  xTaskAbortDelay(thread_id);
  
  return osOK;
#else
  (void) thread_id;
  
  return osErrorResource;
#endif
}

/**
* @brief   Lists all the current threads, along with their current state 
*          and stack usage high water mark.
* @param   buffer   A buffer into which the above mentioned details
*          will be written
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadList (uint8_t *buffer)
{
#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS == 1 ) )
  vTaskList((char *)buffer);
#endif
  return osOK;
}

/**
* @brief  Receive an item from a queue without removing the item from the queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval event information that includes status code.
*/
osEvent osMessagePeek (osMessageQId queue_id, uint32_t millisec)
{
  TickType_t ticks;
  osEvent event;
  
  event.def.message_id = queue_id;
  
  if (queue_id == NULL) {
    event.status = osErrorParameter;
    return event;
  }
  
  ticks = 0;
  if (millisec == osWaitForever) {
    ticks = portMAX_DELAY;
  }
  else if (millisec != 0) {
    ticks = millisec / portTICK_PERIOD_MS;
    if (ticks == 0) {
      ticks = 1;
    }
  }
  
  if (xQueuePeek(queue_id, &event.value.v, ticks) == pdTRUE) 
  {
    /* We have mail */
    event.status = osEventMessage;
  }
  else 
  {
    event.status = (ticks == 0) ? osOK : osEventTimeout;
  }
  
  return event;
}

/**
* @brief  Get the number of messaged stored in a queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @retval number of messages stored in a queue.
*/
uint32_t osMessageWaiting(osMessageQId queue_id)
{
  if (inHandlerMode()) {
    return uxQueueMessagesWaitingFromISR(queue_id);
  }
  else
  {
    return uxQueueMessagesWaiting(queue_id);
  }
}

/**
* @brief  Get the available space in a message queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @retval available space in a message queue.
*/
uint32_t osMessageAvailableSpace(osMessageQId queue_id)  
{
  return uxQueueSpacesAvailable(queue_id);
}

/**
* @brief Delete a Message Queue
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osMessageDelete (osMessageQId queue_id)
{
  if (inHandlerMode()) {
    return osErrorISR;
  }

  vQueueDelete(queue_id);

  return osOK; 
}

/**
* @brief  Create and Initialize a Recursive Mutex
* @param  mutex_def     mutex definition referenced with \ref osMutex.
* @retval  mutex ID for reference by other functions or NULL in case of error..
*/
osMutexId osRecursiveMutexCreate (const osMutexDef_t *mutex_def)
{
#if (configUSE_RECURSIVE_MUTEXES == 1)
#if( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )

  if (mutex_def->controlblock != NULL){
    return xSemaphoreCreateRecursiveMutexStatic( mutex_def->controlblock );
  }
  else {
    return xSemaphoreCreateRecursiveMutex();
  }
#elif ( configSUPPORT_STATIC_ALLOCATION == 1 )
  return xSemaphoreCreateRecursiveMutexStatic( mutex_def->controlblock );
#else 
  return xSemaphoreCreateRecursiveMutex();
#endif
#else
  return NULL;
#endif	
}

/**
* @brief  Release a Recursive Mutex
* @param   mutex_id      mutex ID obtained by \ref osRecursiveMutexCreate.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osRecursiveMutexRelease (osMutexId mutex_id)
{
#if (configUSE_RECURSIVE_MUTEXES == 1)
  osStatus result = osOK;
 
  if (xSemaphoreGiveRecursive(mutex_id) != pdTRUE) 
  {
    result = osErrorOS;
  }
  return result;
#else
	return osErrorResource;
#endif
}

/**
* @brief  Release a Recursive Mutex
* @param   mutex_id    mutex ID obtained by \ref osRecursiveMutexCreate.
* @param millisec      timeout value or 0 in case of no time-out.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osRecursiveMutexWait (osMutexId mutex_id, uint32_t millisec)
{
#if (configUSE_RECURSIVE_MUTEXES == 1)
  TickType_t ticks;
  
  if (mutex_id == NULL)
  {
    return osErrorParameter;
  }
  
  ticks = 0;
  if (millisec == osWaitForever) 
  {
    ticks = portMAX_DELAY;
  }
  else if (millisec != 0) 
  {
    ticks = millisec / portTICK_PERIOD_MS;
    if (ticks == 0) 
    {
      ticks = 1;
    }
  }
  
  if (xSemaphoreTakeRecursive(mutex_id, ticks) != pdTRUE) 
  {
    return osErrorOS;
  }
  return osOK;
#else
	return osErrorResource;
#endif
}

/**
* @brief  Returns the current count value of a counting semaphore
* @param  semaphore_id  semaphore_id ID obtained by \ref osSemaphoreCreate.
* @retval  count value
*/
uint32_t osSemaphoreGetCount(osSemaphoreId semaphore_id)
{
  return uxSemaphoreGetCount(semaphore_id);
}
