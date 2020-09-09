/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */


#ifndef TIMERS_H
#define TIMERS_H

#ifndef INC_FREERTOS_H
	#error "include FreeRTOS.h must appear in source files before include timers.h"
#endif

/*lint -save -e537 This headers are only multiply included if the application code
happens to also be including task.h. */
#include "task.h"
/*lint -restore */

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * MACROS AND DEFINITIONS
 *----------------------------------------------------------*/

/* IDs for commands that can be sent/received on the timer queue.  These are to
be used solely through the macros that make up the public software timer API,
as defined below.  The commands that are sent from interrupts must use the
highest numbers as tmrFIRST_FROM_ISR_COMMAND is used to determine if the task
or interrupt version of the queue send function should be used. */
#define tmrCOMMAND_EXECUTE_CALLBACK_FROM_ISR 	( ( BaseType_t ) -2 )
#define tmrCOMMAND_EXECUTE_CALLBACK				( ( BaseType_t ) -1 )
#define tmrCOMMAND_START_DONT_TRACE				( ( BaseType_t ) 0 )
#define tmrCOMMAND_START					    ( ( BaseType_t ) 1 )
#define tmrCOMMAND_RESET						( ( BaseType_t ) 2 )
#define tmrCOMMAND_STOP							( ( BaseType_t ) 3 )
#define tmrCOMMAND_CHANGE_PERIOD				( ( BaseType_t ) 4 )
#define tmrCOMMAND_DELETE						( ( BaseType_t ) 5 )

#define tmrFIRST_FROM_ISR_COMMAND				( ( BaseType_t ) 6 )
#define tmrCOMMAND_START_FROM_ISR				( ( BaseType_t ) 6 )
#define tmrCOMMAND_RESET_FROM_ISR				( ( BaseType_t ) 7 )
#define tmrCOMMAND_STOP_FROM_ISR				( ( BaseType_t ) 8 )
#define tmrCOMMAND_CHANGE_PERIOD_FROM_ISR		( ( BaseType_t ) 9 )


/**
 * Type by which software timers are referenced.  For example, a call to
 * xTimerCreate() returns an TimerHandle_t variable that can then be used to
 * reference the subject timer in calls to other software timer API functions
 * (for example, xTimerStart(), xTimerReset(), etc.).
 */
typedef void * TimerHandle_t;

/*
 * Defines the prototype to which timer callback functions must conform.
 */
typedef void (*TimerCallbackFunction_t)( TimerHandle_t xTimer );

/*
 * Defines the prototype to which functions used with the
 * xTimerPendFunctionCallFromISR() function must conform.
 */
typedef void (*PendedFunction_t)( void *, uint32_t );

/**
 * TimerHandle_t xTimerCreate( 	const char * const pcTimerName,
 * 								TickType_t xTimerPeriodInTicks,
 * 								UBaseType_t uxAutoReload,
 * 								void * pvTimerID,
 * 								TimerCallbackFunction_t pxCallbackFunction );
 *
 * Creates a new software timer instance, and returns a handle by which the
 * created software timer can be referenced.
 *
 * Internally, within the FreeRTOS implementation, software timers use a block
 * of memory, in which the timer data structure is stored.  If a software timer
 * is created using xTimerCreate() then the required memory is automatically
 * dynamically allocated inside the xTimerCreate() function.  (see
 * http://www.freertos.org/a00111.html).  If a software timer is created using
 * xTimerCreateStatic() then the application writer must provide the memory that
 * will get used by the software timer.  xTimerCreateStatic() therefore allows a
 * software timer to be created without using any dynamic memory allocation.
 *
 * Timers are created in the dormant state.  The xTimerStart(), xTimerReset(),
 * xTimerStartFromISR(), xTimerResetFromISR(), xTimerChangePeriod() and
 * xTimerChangePeriodFromISR() API functions can all be used to transition a
 * timer into the active state.
 *
 * @param pcTimerName A text name that is assigned to the timer.  This is done
 * purely to assist debugging.  The kernel itself only ever references a timer
 * by its handle, and never by its name.
 *
 * @param xTimerPeriodInTicks The timer period.  The time is defined in tick
 * periods so the constant portTICK_PERIOD_MS can be used to convert a time that
 * has been specified in milliseconds.  For example, if the timer must expire
 * after 100 ticks, then xTimerPeriodInTicks should be set to 100.
 * Alternatively, if the timer must expire after 500ms, then xPeriod can be set
 * to ( 500 / portTICK_PERIOD_MS ) provided configTICK_RATE_HZ is less than or
 * equal to 1000.
 *
 * @param uxAutoReload If uxAutoReload is set to pdTRUE then the timer will
 * expire repeatedly with a frequency set by the xTimerPeriodInTicks parameter.
 * If uxAutoReload is set to pdFALSE then the timer will be a one-shot timer and
 * enter the dormant state after it expires.
 *
 * @param pvTimerID An identifier that is assigned to the timer being created.
 * Typically this would be used in the timer callback function to identify which
 * timer expired when the same callback function is assigned to more than one
 * timer.
 *
 * @param pxCallbackFunction The function to call when the timer expires.
 * Callback functions must have the prototype defined by TimerCallbackFunction_t,
 * which is	"void vCallbackFunction( TimerHandle_t xTimer );".
 *
 * @return If the timer is successfully created then a handle to the newly
 * created timer is returned.  If the timer cannot be created (because either
 * there is insufficient FreeRTOS heap remaining to allocate the timer
 * structures, or the timer period was set to 0) then NULL is returned.
 *
 * Example usage:
 * @verbatim
 * #define NUM_TIMERS 5
 *
 * // An array to hold handles to the created timers.
 * TimerHandle_t xTimers[ NUM_TIMERS ];
 *
 * // An array to hold a count of the number of times each timer expires.
 * int32_t lExpireCounters[ NUM_TIMERS ] = { 0 };
 *
 * // Define a callback function that will be used by multiple timer instances.
 * // The callback function does nothing but count the number of times the
 * // associated timer expires, and stop the timer once the timer has expired
 * // 10 times.
 * void vTimerCallback( TimerHandle_t pxTimer )
 * {
 * int32_t lArrayIndex;
 * const int32_t xMaxExpiryCountBeforeStopping = 10;
 *
 * 	   // Optionally do something if the pxTimer parameter is NULL.
 * 	   configASSERT( pxTimer );
 *
 *     // Which timer expired?
 *     lArrayIndex = ( int32_t ) pvTimerGetTimerID( pxTimer );
 *
 *     // Increment the number of times that pxTimer has expired.
 *     lExpireCounters[ lArrayIndex ] += 1;
 *
 *     // If the timer has expired 10 times then stop it from running.
 *     if( lExpireCounters[ lArrayIndex ] == xMaxExpiryCountBeforeStopping )
 *     {
 *         // Do not use a block time if calling a timer API function from a
 *         // timer callback function, as doing so could cause a deadlock!
 *         xTimerStop( pxTimer, 0 );
 *     }
 * }
 *
 * void main( void )
 * {
 * int32_t x;
 *
 *     // Create then start some timers.  Starting the timers before the scheduler
 *     // has been started means the timers will start running immediately that
 *     // the scheduler starts.
 *     for( x = 0; x < NUM_TIMERS; x++ )
 *     {
 *         xTimers[ x ] = xTimerCreate(    "Timer",       // Just a text name, not used by the kernel.
 *                                         ( 100 * x ),   // The timer period in ticks.
 *                                         pdTRUE,        // The timers will auto-reload themselves when they expire.
 *                                         ( void * ) x,  // Assign each timer a unique id equal to its array index.
 *                                         vTimerCallback // Each timer calls the same callback when it expires.
 *                                     );
 *
 *         if( xTimers[ x ] == NULL )
 *         {
 *             // The timer was not created.
 *         }
 *         else
 *         {
 *             // Start the timer.  No block time is specified, and even if one was
 *             // it would be ignored because the scheduler has not yet been
 *             // started.
 *             if( xTimerStart( xTimers[ x ], 0 ) != pdPASS )
 *             {
 *                 // The timer could not be set into the Active state.
 *             }
 *         }
 *     }
 *
 *     // ...
 *     // Create tasks here.
 *     // ...
 *
 *     // Starting the scheduler will start the timers running as they have already
 *     // been set into the active state.
 *     vTaskStartScheduler();
 *
 *     // Should not reach here.
 *     for( ;; );
 * }
 * @endverbatim
 */
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
	TimerHandle_t xTimerCreate(	const char * const pcTimerName,			/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
								const TickType_t xTimerPeriodInTicks,
								const UBaseType_t uxAutoReload,
								void * const pvTimerID,
								TimerCallbackFunction_t pxCallbackFunction ) PRIVILEGED_FUNCTION;
#endif

/**
 * TimerHandle_t xTimerCreateStatic(const char * const pcTimerName,
 * 									TickType_t xTimerPeriodInTicks,
 * 									UBaseType_t uxAutoReload,
 * 									void * pvTimerID,
 * 									TimerCallbackFunction_t pxCallbackFunction,
 *									StaticTimer_t *pxTimerBuffer );
 *
 * Creates a new software timer instance, and returns a handle by which the
 * created software timer can be referenced.
 *
 * Internally, within the FreeRTOS implementation, software timers use a block
 * of memory, in which the timer data structure is stored.  If a software timer
 * is created using xTimerCreate() then the required memory is automatically
 * dynamically allocated inside the xTimerCreate() function.  (see
 * http://www.freertos.org/a00111.html).  If a software timer is created using
 * xTimerCreateStatic() then the application writer must provide the memory that
 * will get used by the software timer.  xTimerCreateStatic() therefore allows a
 * software timer to be created without using any dynamic memory allocation.
 *
 * Timers are created in the dormant state.  The xTimerStart(), xTimerReset(),
 * xTimerStartFromISR(), xTimerResetFromISR(), xTimerChangePeriod() and
 * xTimerChangePeriodFromISR() API functions can all be used to transition a
 * timer into the active state.
 *
 * @param pcTimerName A text name that is assigned to the timer.  This is done
 * purely to assist debugging.  The kernel itself only ever references a timer
 * by its handle, and never by its name.
 *
 * @param xTimerPeriodInTicks The timer period.  The time is defined in tick
 * periods so the constant portTICK_PERIOD_MS can be used to convert a time that
 * has been specified in milliseconds.  For example, if the timer must expire
 * after 100 ticks, then xTimerPeriodInTicks should be set to 100.
 * Alternatively, if the timer must expire after 500ms, then xPeriod can be set
 * to ( 500 / portTICK_PERIOD_MS ) provided configTICK_RATE_HZ is less than or
 * equal to 1000.
 *
 * @param uxAutoReload If uxAutoReload is set to pdTRUE then the timer will
 * expire repeatedly with a frequency set by the xTimerPeriodInTicks parameter.
 * If uxAutoReload is set to pdFALSE then the timer will be a one-shot timer and
 * enter the dormant state after it expires.
 *
 * @param pvTimerID An identifier that is assigned to the timer being created.
 * Typically this would be used in the timer callback function to identify which
 * timer expired when the same callback function is assigned to more than one
 * timer.
 *
 * @param pxCallbackFunction The function to call when the timer expires.
 * Callback functions must have the prototype defined by TimerCallbackFunction_t,
 * which is "void vCallbackFunction( TimerHandle_t xTimer );".
 *
 * @param pxTimerBuffer Must point to a variable of type StaticTimer_t, which
 * will be then be used to hold the software timer's data structures, removing
 * the need for the memory to be allocated dynamically.
 *
 * @return If the timer is created then a handle to the created timer is
 * returned.  If pxTimerBuffer was NULL then NULL is returned.
 *
 * Example usage:
 * @verbatim
 *
 * // The buffer used to hold the software timer's data structure.
 * static StaticTimer_t xTimerBuffer;
 *
 * // A variable that will be incremented by the software timer's callback
 * // function.
 * UBaseType_t uxVariableToIncrement = 0;
 *
 * // A software timer callback function that increments a variable passed to
 * // it when the software timer was created.  After the 5th increment the
 * // callback function stops the software timer.
 * static void prvTimerCallback( TimerHandle_t xExpiredTimer )
 * {
 * UBaseType_t *puxVariableToIncrement;
 * BaseType_t xReturned;
 *
 *     // Obtain the address of the variable to increment from the timer ID.
 *     puxVariableToIncrement = ( UBaseType_t * ) pvTimerGetTimerID( xExpiredTimer );
 *
 *     // Increment the variable to show the timer callback has executed.
 *     ( *puxVariableToIncrement )++;
 *
 *     // If this callback has executed the required number of times, stop the
 *     // timer.
 *     if( *puxVariableToIncrement == 5 )
 *     {
 *         // This is called from a timer callback so must not block.
 *         xTimerStop( xExpiredTimer, staticDONT_BLOCK );
 *     }
 * }
 *
 *
 * void main( void )
 * {
 *     // Create the software time.  xTimerCreateStatic() has an extra parameter
 *     // than the normal xTimerCreate() API function.  The parameter is a pointer
 *     // to the StaticTimer_t structure that will hold the software timer
 *     // structure.  If the parameter is passed as NULL then the structure will be
 *     // allocated dynamically, just as if xTimerCreate() had been called.
 *     xTimer = xTimerCreateStatic( "T1",             // Text name for the task.  Helps debugging only.  Not used by FreeRTOS.
 *                                  xTimerPeriod,     // The period of the timer in ticks.
 *                                  pdTRUE,           // This is an auto-reload timer.
 *                                  ( void * ) &uxVariableToIncrement,    // A variable incremented by the software timer's callback function
 *                                  prvTimerCallback, // The function to execute when the timer expires.
 *                                  &xTimerBuffer );  // The buffer that will hold the software timer structure.
 *
 *     // The scheduler has not started yet so a block time is not used.
 *     xReturned = xTimerStart( xTimer, 0 );
 *
 *     // ...
 *     // Create tasks here.
 *     // ...
 *
 *     // Starting the scheduler will start the timers running as they have already
 *     // been set into the active state.
 *     vTaskStartScheduler();
 *
 *     // Should not reach here.
 *     for( ;; );
 * }
 * @endverbatim
 */
#if( configSUPPORT_STATIC_ALLOCATION == 1 )
	TimerHandle_t xTimerCreateStatic(	const char * const pcTimerName,			/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
										const TickType_t xTimerPeriodInTicks,
										const UBaseType_t uxAutoReload,
										void * const pvTimerID,
										TimerCallbackFunction_t pxCallbackFunction,
										StaticTimer_t *pxTimerBuffer ) PRIVILEGED_FUNCTION;
#endif /* configSUPPORT_STATIC_ALLOCATION */

/**
 * void *pvTimerGetTimerID( TimerHandle_t xTimer );
 *
 * Returns the ID assigned to the timer.
 *
 * IDs are assigned to timers using the pvTimerID parameter of the call to
 * xTimerCreated() that was used to create the timer, and by calling the
 * vTimerSetTimerID() API function.
 *
 * If the same callback function is assigned to multiple timers then the timer
 * ID can be used as time specific (timer local) storage.
 *
 * @param xTimer The timer being queried.
 *
 * @return The ID assigned to the timer being queried.
 *
 * Example usage:
 *
 * See the xTimerCreate() API function example usage scenario.
 */
void *pvTimerGetTimerID( const TimerHandle_t xTimer ) PRIVILEGED_FUNCTION;

/**
 * void vTimerSetTimerID( TimerHandle_t xTimer, void *pvNewID );
 *
 * Sets the ID assigned to the timer.
 *
 * IDs are assigned to timers using the pvTimerID parameter of the call to
 * xTimerCreated() that was used to create the timer.
 *
 * If the same callback function is assigned to multiple timers then the timer
 * ID can be used as time specific (timer local) storage.
 *
 * @param xTimer The timer being updated.
 *
 * @param pvNewID The ID to assign to the timer.
 *
 * Example usage:
 *
 * See the xTimerCreate() API function example usage scenario.
 */
void vTimerSetTimerID( TimerHandle_t xTimer, void *pvNewID ) PRIVILEGED_FUNCTION;

/**
 * BaseType_t xTimerIsTimerActive( TimerHandle_t xTimer );
 *
 * Queries a timer to see if it is active or dormant.
 *
 * A timer will be dormant if:
 *     1) It has been created but not started, or
 *     2) It is an expired one-shot timer that has not been restarted.
 *
 * Timers are created in the dormant state.  The xTimerStart(), xTimerReset(),
 * xTimerStartFromISR(), xTimerResetFromISR(), xTimerChangePeriod() and
 * xTimerChangePeriodFromISR() API functions can all be used to transition a timer into the
 * active state.
 *
 * @param xTimer The timer being queried.
 *
 * @return pdFALSE will be returned if the timer is dormant.  A value other than
 * pdFALSE will be returned if the timer is active.
 *
 * Example usage:
 * @verbatim
 * // This function assumes xTimer has already been created.
 * void vAFunction( TimerHandle_t xTimer )
 * {
 *     if( xTimerIsTimerActive( xTimer ) != pdFALSE ) // or more simply and equivalently "if( xTimerIsTimerActive( xTimer ) )"
 *     {
 *         // xTimer is active, do something.
 *     }
 *     else
 *     {
 *         // xTimer is not active, do something else.
 *     }
 * }
 * @endverbatim
 */
BaseType_t xTimerIsTimerActive( TimerHandle_t xTimer ) PRIVILEGED_FUNCTION;

/**
 * TaskHandle_t xTimerGetTimerDaemonTaskHandle( void );
 *
 * Simply returns the handle of the timer service/daemon task.  It it not valid
 * to call xTimerGetTimerDaemonTaskHandle() before the scheduler has been started.
 */
TaskHandle_t xTimerGetTimerDaemonTaskHandle( void ) PRIVILEGED_FUNCTION;

/**
 * BaseType_t xTimerStart( TimerHandle_t xTimer, TickType_t xTicksToWait );
 *
 * Timer functionality is provided by a timer service/daemon task.  Many of the
 * public FreeRTOS timer API functions send commands to the timer service task
 * through a queue called the timer command queue.  The timer command queue is
 * private to the kernel itself and is not directly accessible to application
 * code.  The length of the timer command queue is set by the
 * configTIMER_QUEUE_LENGTH configuration constant.
 *
 * xTimerStart() starts a timer that was previously created using the
 * xTimerCreate() API function.  If the timer had already been started and was
 * already in the active state, then xTimerStart() has equivalent functionality
 * to the xTimerReset() API function.
 *
 * Starting a timer ensures the timer is in the active state.  If the timer
 * is not stopped, deleted, or reset in the mean time, the callback function
 * associated with the timer will get called 'n' ticks after xTimerStart() was
 * called, where 'n' is the timers defined period.
 *
 * It is valid to call xTimerStart() before the scheduler has been started, but
 * when this is done the timer will not actually start until the scheduler is
 * started, and the timers expiry time will be relative to when the scheduler is
 * started, not relative to when xTimerStart() was called.
 *
 * The configUSE_TIMERS configuration constant must be set to 1 for xTimerStart()
 * to be available.
 *
 * @param xTimer The handle of the timer being started/restarted.
 *
 * @param xTicksToWait Specifies the time, in ticks, that the calling task should
 * be held in the Blocked state to wait for the start command to be successfully
 * sent to the timer command queue, should the queue already be full when
 * xTimerStart() was called.  xTicksToWait is ignored if xTimerStart() is called
 * before the scheduler is started.
 *
 * @return pdFAIL will be returned if the start command could not be sent to
 * the timer command queue even after xTicksToWait ticks had passed.  pdPASS will
 * be returned if the command was successfully sent to the timer command queue.
 * When the command is actually processed will depend on the priority of the
 * timer service/daemon task relative to other tasks in the system, although the
 * timers expiry time is relative to when xTimerStart() is actually called.  The
 * timer service/daemon task priority is set by the configTIMER_TASK_PRIORITY
 * configuration constant.
 *
 * Example usage:
 *
 * See the xTimerCreate() API function example usage scenario.
 *
 */
#define xTimerStart( xTimer, xTicksToWait ) xTimerGenericCommand( ( xTimer ), tmrCOMMAND_START, ( xTaskGetTickCount() ), NULL, ( xTicksToWait ) )

/**
 * BaseType_t xTimerStop( TimerHandle_t xTimer, TickType_t xTicksToWait );
 *
 * Timer functionality is provided by a timer service/daemon task.  Many of the
 * public FreeRTOS timer API functions send commands to the timer service task
 * through a queue called the timer command queue.  The timer command queue is
 * private to the kernel itself and is not directly accessible to application
 * code.  The length of the timer command queue is set by the
 * configTIMER_QUEUE_LENGTH configuration constant.
 *
 * xTimerStop() stops a timer that was previously started using either of the
 * The xTimerStart(), xTimerReset(), xTimerStartFromISR(), xTimerResetFromISR(),
 * xTimerChangePeriod() or xTimerChangePeriodFromISR() API functions.
 *
 * Stopping a timer ensures the timer is not in the active state.
 *
 * The configUSE_TIMERS configuration constant must be set to 1 for xTimerStop()
 * to be available.
 *
 * @param xTimer The handle of the timer being stopped.
 *
 * @param xTicksToWait Specifies the time, in ticks, that the calling task should
 * be held in the Blocked state to wait for the stop command to be successfully
 * sent to the timer command queue, should the queue already be full when
 * xTimerStop() was called.  xTicksToWait is ignored if xTimerStop() is called
 * before the scheduler is started.
 *
 * @return pdFAIL will be returned if the stop command could not be sent to
 * the timer command queue even after xTicksToWait ticks had passed.  pdPASS will
 * be returned if the command was successfully sent to the timer command queue.
 * When the command is actually processed will depend on the priority of the
 * timer service/daemon task relative to other tasks in the system.  The timer
 * service/daemon task priority is set by the configTIMER_TASK_PRIORITY
 * configuration constant.
 *
 * Example usage:
 *
 * See the xTimerCreate() API function example usage scenario.
 *
 */
#define xTimerStop( xTimer, xTicksToWait ) xTimerGenericCommand( ( xTimer ), tmrCOMMAND_STOP, 0U, NULL, ( xTicksToWait ) )

/**
 * BaseType_t xTimerChangePeriod( 	TimerHandle_t xTimer,
 *										TickType_t xNewPeriod,
 *										TickType_t xTicksToWait );
 *
 * Timer functionality is provided by a timer service/daemon task.  Many of the
 * public FreeRTOS timer API functions send commands to the timer service task
 * through a queue called the timer command queue.  The timer command queue is
 * private to the kernel itself and is not directly accessible to application
 * code.  The length of the timer command queue is set by the
 * configTIMER_QUEUE_LENGTH configuration constant.
 *
 * xTimerChangePeriod() changes the period of a timer that was previously
 * created using the xTimerCreate() API function.
 *
 * xTimerChangePeriod() can be called to change the period of an active or
 * dormant state timer.
 *
 * The configUSE_TIMERS configuration constant must be set to 1 for
 * xTimerChangePeriod() to be available.
 *
 * @param xTimer The handle of the timer that is having its period changed.
 *
 * @param xNewPeriod The new period for xTimer. Timer periods are specified in
 * tick periods, so the constant portTICK_PERIOD_MS can be used to convert a time
 * that has been specified in milliseconds.  For example, if the timer must
 * expire after 100 ticks, then xNewPeriod should be set to 100.  Alternatively,
 * if the timer must expire after 500ms, then xNewPeriod can be set to
 * ( 500 / portTICK_PERIOD_MS ) provided configTICK_RATE_HZ is less than
 * or equal to 1000.
 *
 * @param xTicksToWait Specifies the time, in ticks, that the calling task should
 * be held in the Blocked state to wait for the change period command to be
 * successfully sent to the timer command queue, should the queue already be
 * full when xTimerChangePeriod() was called.  xTicksToWait is ignored if
 * xTimerChangePeriod() is called before the scheduler is started.
 *
 * @return pdFAIL will be returned if the change period command could not be
 * sent to the timer command queue even after xTicksToWait ticks had passed.
 * pdPASS will be returned if the command was successfully sent to the timer
 * command queue.  When the command is actually processed will depend on the
 * priority of the timer service/daemon task relative to other tasks in the
 * system.  The timer service/daemon task priority is set by the
 * configTIMER_TASK_PRIORITY configuration constant.
 *
 * Example usage:
 * @verbatim
 * // This function assumes xTimer has already been created.  If the timer
 * // referenced by xTimer is already active when it is called, then the timer
 * // is deleted.  If the timer referenced by xTimer is not active when it is
 * // called, then the period of the timer is set to 500ms and the timer is
 * // started.
 * void vAFunction( TimerHandle_t xTimer )
 * {
 *     if( xTimerIsTimerActive( xTimer ) != pdFALSE ) // or more simply and equivalently "if( xTimerIsTimerActive( xTimer ) )"
 *     {
 *         // xTimer is already active - delete it.
 *         xTimerDelete( xTimer );
 *     }
 *     else
 *     {
 *         // xTimer is not active, change its period to 500ms.  This will also
 *         // cause the timer to start.  Block for a maximum of 100 ticks if the
 *         // change period command cannot immediately be sent to the timer
 *         // command queue.
 *         if( xTimerChangePeriod( xTimer, 500 / portTICK_PERIOD_MS, 100 ) == pdPASS )
 *         {
 *             // The command was successfully sent.
 *         }
 *         else
 *         {
 *             // The command could not be sent, even after waiting for 100 ticks
 *             // to pass.  Take appropriate action here.
 *         }
 *     }
 * }
 * @endverbatim
 */
 #define xTimerChangePeriod( xTimer, xNewPeriod, xTicksToWait ) xTimerGenericCommand( ( xTimer ), tmrCOMMAND_CHANGE_PERIOD, ( xNewPeriod ), NULL, ( xTicksToWait ) )

/**
 * BaseType_t xTimerDelete( TimerHandle_t xTimer, TickType_t xTicksToWait );
 *
 * Timer functionality is provided by a timer service/daemon task.  Many of the
 * public FreeRTOS timer API functions send commands to the timer service task
 * through a queue called the timer command queue.  The timer command queue is
 * private to the kernel itself and is not directly accessible to application
 * code.  The length of the timer command queue is set by the
 * configTIMER_QUEUE_LENGTH configuration constant.
 *
 * xTimerDelete() deletes a timer that was previously created using the
 * xTimerCreate() API function.
 *
 * The configUSE_TIMERS configuration constant must be set to 1 for
 * xTimerDelete() to be available.
 *
 * @param xTimer The handle of the timer being deleted.
 *
 * @param xTicksToWait Specifies the time, in ticks, that the calling task should
 * be held in the Blocked state to wait for the delete command to be
 * successfully sent to the timer command queue, should the queue already be
 * full when xTimerDelete() was called.  xTicksToWait is ignored if xTimerDelete()
 * is called before the scheduler is started.
 *
 * @return pdFAIL will be returned if the delete command could not be sent to
 * the timer command queue even after xTicksToWait ticks had passed.  pdPASS will
 * be returned if the command was successfully sent to the timer command queue.
 * When the command is actually processed will depend on the priority of the
 * timer service/daemon task relative to other tasks in the system.  The timer
 * service/daemon task priority is set by the configTIMER_TASK_PRIORITY
 * configuration constant.
 *
 * Example usage:
 *
 * See the xTimerChangePeriod() API function example usage scenario.
 */
#define xTimerDelete( xTimer, xTicksToWait ) xTimerGenericCommand( ( xTimer ), tmrCOMMAND_DELETE, 0U, NULL, ( xTicksToWait ) )

/**
 * BaseType_t xTimerReset( TimerHandle_t xTimer, TickType_t xTicksToWait );
 *
 * Timer functionality is provided by a timer service/daemon task.  Many of the
 * public FreeRTOS timer API functions send commands to the timer service task
 * through a queue called the timer command queue.  The timer command queue is
 * private to the kernel itself and is not directly accessible to application
 * code.  The length of the timer command queue is set by the
 * configTIMER_QUEUE_LENGTH configuration constant.
 *
 * xTimerReset() re-starts a timer that was previously created using the
 * xTimerCreate() API function.  If the timer had already been started and was
 * already in the active state, then xTimerReset() will cause the timer to
 * re-evaluate its expiry time so that it is relative to when xTimerReset() was
 * called.  If the timer was in the dormant state then xTimerReset() has
 * equivalent functionality to the xTimerStart() API function.
 *
 * Resetting a timer ensures the timer is in the active state.  If the timer
 * is not stopped, deleted, or reset in the mean time, the callback function
 * associated with the timer will get called 'n' ticks after xTimerReset() was
 * called, where 'n' is the timers defined period.
 *
 * It is valid to call xTimerReset() before the scheduler has been started, but
 * when this is done the timer will not actually start until the scheduler is
 * started, and the timers expiry time will be relative to when the scheduler is
 * started, not relative to when xTimerReset() was called.
 *
 * The configUSE_TIMERS configuration constant must be set to 1 for xTimerReset()
 * to be available.
 *
 * @param xTimer The handle of the timer being reset/started/restarted.
 *
 * @param xTicksToWait Specifies the time, in ticks, that the calling task should
 * be held in the Blocked state to wait for the reset command to be successfully
 * sent to the timer command queue, should the queue already be full when
 * xTimerReset() was called.  xTicksToWait is ignored if xTimerReset() is called
 * before the scheduler is started.
 *
 * @return pdFAIL will be returned if the reset command could not be sent to
 * the timer command queue even after xTicksToWait ticks had passed.  pdPASS will
 * be returned if the command was successfully sent to the timer command queue.
 * When the command is actually processed will depend on the priority of the
 * timer service/daemon task relative to other tasks in the system, although the
 * timers expiry time is relative to when xTimerStart() is actually called.  The
 * timer service/daemon task priority is set by the configTIMER_TASK_PRIORITY
 * configuration constant.
 *
 * Example usage:
 * @verbatim
 * // When a key is pressed, an LCD back-light is switched on.  If 5 seconds pass
 * // without a key being pressed, then the LCD back-light is switched off.  In
 * // this case, the timer is a one-shot timer.
 *
 * TimerHandle_t xBacklightTimer = NULL;
 *
 * // The callback function assigned to the one-shot timer.  In this case the
 * // parameter is not used.
 * void vBacklightTimerCallback( TimerHandle_t pxTimer )
 * {
 *     // The timer expired, therefore 5 seconds must have passed since a key
 *     // was pressed.  Switch off the LCD back-light.
 *     vSetBacklightState( BACKLIGHT_OFF );
 * }
 *
 * // The key press event handler.
 * void vKeyPressEventHandler( char cKey )
 * {
 *     // Ensure the LCD back-light is on, then reset the timer that is
 *     // responsible for turning the back-light off after 5 seconds of
 *     // key inactivity.  Wait 10 ticks for the command to be successfully sent
 *     // if it cannot be sent immediately.
 *     vSetBacklightState( BACKLIGHT_ON );
 *     if( xTimerReset( xBacklightTimer, 100 ) != pdPASS )
 *     {
 *         // The reset command was not executed successfully.  Take appropriate
 *         // action here.
 *     }
 *
 *     // Perform the rest of the key processing here.
 * }
 *
 * void main( void )
 * {
 * int32_t x;
 *
 *     // Create then start the one-shot timer that is responsible for turning
 *     // the back-light off if no keys are pressed within a 5 second period.
 *     xBacklightTimer = xTimerCreate( "BacklightTimer",           // Just a text name, not used by the kernel.
 *                                     ( 5000 / portTICK_PERIOD_MS), // The timer period in ticks.
 *                                     pdFALSE,                    // The timer is a one-shot timer.
 *                                     0,                          // The id is not used by the callback so can take any value.
 *                                     vBacklightTimerCallback     // The callback function that switches the LCD back-light off.
 *                                   );
 *
 *     if( xBacklightTimer == NULL )
 *     {
 *         // The timer was not created.
 *     }
 *     else
 *     {
 *         // Start the timer.  No block time is specified, and even if one was
 *         // it would be ignored because the scheduler has not yet been
 *         // started.
 *         if( xTimerStart( xBacklightTimer, 0 ) != pdPASS )
 *         {
 *             // The timer could not be set into the Active state.
 *         }
 *     }
 *
 *     // ...
 *     // Create tasks here.
 *     // ...
 *
 *     // Starting the scheduler will start the timer running as it has already
 *     // been set into the active state.
 *     vTaskStartScheduler();
 *
 *     // Should not reach here.
 *     for( ;; );
 * }
 * @endverbatim
 */
#define xTimerReset( xTimer, xTicksToWait ) xTimerGenericCommand( ( xTimer ), tmrCOMMAND_RESET, ( xTaskGetTickCount() ), NULL, ( xTicksToWait ) )

/**
 * BaseType_t xTimerStartFromISR( 	TimerHandle_t xTimer,
 *									BaseType_t *pxHigherPriorityTaskWoken );
 *
 * A version of xTimerStart() that can be called from an interrupt service
 * routine.
 *
 * @param xTimer The handle of the timer being started/restarted.
 *
 * @param pxHigherPriorityTaskWoken The timer service/daemon task spends most
 * of its time in the Blocked state, waiting for messages to arrive on the timer
 * command queue.  Calling xTimerStartFromISR() writes a message to the timer
 * command queue, so has the potential to transition the timer service/daemon
 * task out of the Blocked state.  If calling xTimerStartFromISR() causes the
 * timer service/daemon task to leave the Blocked state, and the timer service/
 * daemon task has a priority equal to or greater than the currently executing
 * task (the task that was interrupted), then *pxHigherPriorityTaskWoken will
 * get set to pdTRUE internally within the xTimerStartFromISR() function.  If
 * xTimerStartFromISR() sets this value to pdTRUE then a context switch should
 * be performed before the interrupt exits.
 *
 * @return pdFAIL will be returned if the start command could not be sent to
 * the timer command queue.  pdPASS will be returned if the command was
 * successfully sent to the timer command queue.  When the command is actually
 * processed will depend on the priority of the timer service/daemon task
 * relative to other tasks in the system, although the timers expiry time is
 * relative to when xTimerStartFromISR() is actually called.  The timer
 * service/daemon task priority is set by the configTIMER_TASK_PRIORITY
 * configuration constant.
 *
 * Example usage:
 * @verbatim
 * // This scenario assumes xBacklightTimer has already been created.  When a
 * // key is pressed, an LCD back-light is switched on.  If 5 seconds pass
 * // without a key being pressed, then the LCD back-light is switched off.  In
 * // this case, the timer is a one-shot timer, and unlike the example given for
 * // the xTimerReset() function, the key press event handler is an interrupt
 * // service routine.
 *
 * // The callback function assigned to the one-shot timer.  In this case the
 * // parameter is not used.
 * void vBacklightTimerCallback( TimerHandle_t pxTimer )
 * {
 *     // The timer expired, therefore 5 seconds must have passed since a key
 *     // was pressed.  Switch off the LCD back-light.
 *     vSetBacklightState( BACKLIGHT_OFF );
 * }
 *
 * // The key press interrupt service routine.
 * void vKeyPressEventInterruptHandler( void )
 * {
 * BaseType_t xHigherPriorityTaskWoken = pdFALSE;
 *
 *     // Ensure the LCD back-light is on, then restart the timer that is
 *     // responsible for turning the back-light off after 5 seconds of
 *     // key inactivity.  This is an interrupt service routine so can only
 *     // call FreeRTOS API functions that end in "FromISR".
 *     vSetBacklightState( BACKLIGHT_ON );
 *
 *     // xTimerStartFromISR() or xTimerResetFromISR() could be called here
 *     // as both cause the timer to re-calculate its expiry time.
 *     // xHigherPriorityTaskWoken was initialised to pdFALSE when it was
 *     // declared (in this function).
 *     if( xTimerStartFromISR( xBacklightTimer, &xHigherPriorityTaskWoken ) != pdPASS )
 *     {
 *         // The start command was not executed successfully.  Take appropriate
 *         // action here.
 *     }
 *
 *     // Perform the rest of the key processing here.
 *
 *     // If xHigherPriorityTaskWoken equals pdTRUE, then a context switch
 *     // should be performed.  The syntax required to perform a context switch
 *     // from inside an ISR varies from port to port, and from compiler to
 *     // compiler.  Inspect the demos for the port you are using to find the
 *     // actual syntax required.
 *     if( xHigherPriorityTaskWoken != pdFALSE )
 *     {
 *         // Call the interrupt safe yield function here (actual function
 *         // depends on the FreeRTOS port being used).
 *     }
 * }
 * @endverbatim
 */
#define xTimerStartFromISR( xTimer, pxHigherPriorityTaskWoken ) xTimerGenericCommand( ( xTimer ), tmrCOMMAND_START_FROM_ISR, ( xTaskGetTickCountFromISR() ), ( pxHigherPriorityTaskWoken ), 0U )

/**
 * BaseType_t xTimerStopFromISR( 	TimerHandle_t xTimer,
 *									BaseType_t *pxHigherPriorityTaskWoken );
 *
 * A version of xTimerStop() that can be called from an interrupt service
 * routine.
 *
 * @param xTimer The handle of the timer being stopped.
 *
 * @param pxHigherPriorityTaskWoken The timer service/daemon task spends most
 * of its time in the Blocked state, waiting for messages to arrive on the timer
 * command queue.  Calling xTimerStopFromISR() writes a message to the timer
 * command queue, so has the potential to transition the timer service/daemon
 * task out of the Blocked state.  If calling xTimerStopFromISR() causes the
 * timer service/daemon task to leave the Blocked state, and the timer service/
 * daemon task has a priority equal to or greater than the currently executing
 * task (the task that was interrupted), then *pxHigherPriorityTaskWoken will
 * get set to pdTRUE internally within the xTimerStopFromISR() function.  If
 * xTimerStopFromISR() sets this value to pdTRUE then a context switch should
 * be performed before the interrupt exits.
 *
 * @return pdFAIL will be returned if the stop command could not be sent to
 * the timer command queue.  pdPASS will be returned if the command was
 * successfully sent to the timer command queue.  When the command is actually
 * processed will depend on the priority of the timer service/daemon task
 * relative to other tasks in the system.  The timer service/daemon task
 * priority is set by the configTIMER_TASK_PRIORITY configuration constant.
 *
 * Example usage:
 * @verbatim
 * // This scenario assumes xTimer has already been created and started.  When
 * // an interrupt occurs, the timer should be simply stopped.
 *
 * // The interrupt service routine that stops the timer.
 * void vAnExampleInterruptServiceRoutine( void )
 * {
 * BaseType_t xHigherPriorityTaskWoken = pdFALSE;
 *
 *     // The interrupt has occurred - simply stop the timer.
 *     // xHigherPriorityTaskWoken was set to pdFALSE where it was defined
 *     // (within this function).  As this is an interrupt service routine, only
 *     // FreeRTOS API functions that end in "FromISR" can be used.
 *     if( xTimerStopFromISR( xTimer, &xHigherPriorityTaskWoken ) != pdPASS )
 *     {
 *         // The stop command was not executed successfully.  Take appropriate
 *         // action here.
 *     }
 *
 *     // If xHigherPriorityTaskWoken equals pdTRUE, then a context switch
 *     // should be performed.  The syntax required to perform a context switch
 *     // from inside an ISR varies from port to port, and from compiler to
 *     // compiler.  Inspect the demos for the port you are using to find the
 *     // actual syntax required.
 *     if( xHigherPriorityTaskWoken != pdFALSE )
 *     {
 *         // Call the interrupt safe yield function here (actual function
 *         // depends on the FreeRTOS port being used).
 *     }
 * }
 * @endverbatim
 */
#define xTimerStopFromISR( xTimer, pxHigherPriorityTaskWoken ) xTimerGenericCommand( ( xTimer ), tmrCOMMAND_STOP_FROM_ISR, 0, ( pxHigherPriorityTaskWoken ), 0U )

/**
 * BaseType_t xTimerChangePeriodFromISR( TimerHandle_t xTimer,
 *										 TickType_t xNewPeriod,
 *										 BaseType_t *pxHigherPriorityTaskWoken );
 *
 * A version of xTimerChangePeriod() that can be called from an interrupt
 * service routine.
 *
 * @param xTimer The handle of the timer that is having its period changed.
 *
 * @param xNewPeriod The new period for xTimer. Timer periods are specified in
 * tick periods, so the constant portTICK_PERIOD_MS can be used to convert a time
 * that has been specified in milliseconds.  For example, if the timer must
 * expire after 100 ticks, then xNewPeriod should be set to 100.  Alternatively,
 * if the timer must expire after 500ms, then xNewPeriod can be set to
 * ( 500 / portTICK_PERIOD_MS ) provided configTICK_RATE_HZ is less than
 * or equal to 1000.
 *
 * @param pxHigherPriorityTaskWoken The timer service/daemon task spends most
 * of its time in the Blocked state, waiting for messages to arrive on the timer
 * command queue.  Calling xTimerChangePeriodFromISR() writes a message to the
 * timer command queue, so has the potential to transition the timer service/
 * daemon task out of the Blocked state.  If calling xTimerChangePeriodFromISR()
 * causes the timer service/daemon task to leave the Blocked state, and the
 * timer service/daemon task has a priority equal to or greater than the
 * currently executing task (the task that was interrupted), then
 * *pxHigherPriorityTaskWoken will get set to pdTRUE internally within the
 * xTimerChangePeriodFromISR() function.  If xTimerChangePeriodFromISR() sets
 * this value to pdTRUE then a context switch should be performed before the
 * interrupt exits.
 *
 * @return pdFAIL will be returned if the command to change the timers period
 * could not be sent to the timer command queue.  pdPASS will be returned if the
 * command was successfully sent to the timer command queue.  When the command
 * is actually processed will depend on the priority of the timer service/daemon
 * task relative to other tasks in the system.  The timer service/daemon task
 * priority is set by the configTIMER_TASK_PRIORITY configuration constant.
 *
 * Example usage:
 * @verbatim
 * // This scenario assumes xTimer has already been created and started.  When
 * // an interrupt occurs, the period of xTimer should be changed to 500ms.
 *
 * // The interrupt service routine that changes the period of xTimer.
 * void vAnExampleInterruptServiceRoutine( void )
 * {
 * BaseType_t xHigherPriorityTaskWoken = pdFALSE;
 *
 *     // The interrupt has occurred - change the period of xTimer to 500ms.
 *     // xHigherPriorityTaskWoken was set to pdFALSE where it was defined
 *     // (within this function).  As this is an interrupt service routine, only
 *     // FreeRTOS API functions that end in "FromISR" can be used.
 *     if( xTimerChangePeriodFromISR( xTimer, &xHigherPriorityTaskWoken ) != pdPASS )
 *     {
 *         // The command to change the timers period was not executed
 *         // successfully.  Take appropriate action here.
 *     }
 *
 *     // If xHigherPriorityTaskWoken equals pdTRUE, then a context switch
 *     // should be performed.  The syntax required to perform a context switch
 *     // from inside an ISR varies from port to port, and from compiler to
 *     // compiler.  Inspect the demos for the port you are using to find the
 *     // actual syntax required.
 *     if( xHigherPriorityTaskWoken != pdFALSE )
 *     {
 *         // Call the interrupt safe yield function here (actual function
 *         // depends on the FreeRTOS port being used).
 *     }
 * }
 * @endverbatim
 */
#define xTimerChangePeriodFromISR( xTimer, xNewPeriod, pxHigherPriorityTaskWoken ) xTimerGenericCommand( ( xTimer ), tmrCOMMAND_CHANGE_PERIOD_FROM_ISR, ( xNewPeriod ), ( pxHigherPriorityTaskWoken ), 0U )

/**
 * BaseType_t xTimerResetFromISR( 	TimerHandle_t xTimer,
 *									BaseType_t *pxHigherPriorityTaskWoken );
 *
 * A version of xTimerReset() that can be called from an interrupt service
 * routine.
 *
 * @param xTimer The handle of the timer that is to be started, reset, or
 * restarted.
 *
 * @param pxHigherPriorityTaskWoken The timer service/daemon task spends most
 * of its time in the Blocked state, waiting for messages to arrive on the timer
 * command queue.  Calling xTimerResetFromISR() writes a message to the timer
 * command queue, so has the potential to transition the timer service/daemon
 * task out of the Blocked state.  If calling xTimerResetFromISR() causes the
 * timer service/daemon task to leave the Blocked state, and the timer service/
 * daemon task has a priority equal to or greater than the currently executing
 * task (the task that was interrupted), then *pxHigherPriorityTaskWoken will
 * get set to pdTRUE internally within the xTimerResetFromISR() function.  If
 * xTimerResetFromISR() sets this value to pdTRUE then a context switch should
 * be performed before the interrupt exits.
 *
 * @return pdFAIL will be returned if the reset command could not be sent to
 * the timer command queue.  pdPASS will be returned if the command was
 * successfully sent to the timer command queue.  When the command is actually
 * processed will depend on the priority of the timer service/daemon task
 * relative to other tasks in the system, although the timers expiry time is
 * relative to when xTimerResetFromISR() is actually called.  The timer service/daemon
 * task priority is set by the configTIMER_TASK_PRIORITY configuration constant.
 *
 * Example usage:
 * @verbatim
 * // This scenario assumes xBacklightTimer has already been created.  When a
 * // key is pressed, an LCD back-light is switched on.  If 5 seconds pass
 * // without a key being pressed, then the LCD back-light is switched off.  In
 * // this case, the timer is a one-shot timer, and unlike the example given for
 * // the xTimerReset() function, the key press event handler is an interrupt
 * // service routine.
 *
 * // The callback function assigned to the one-shot timer.  In this case the
 * // parameter is not used.
 * void vBacklightTimerCallback( TimerHandle_t pxTimer )
 * {
 *     // The timer expired, therefore 5 seconds must have passed since a key
 *     // was pressed.  Switch off the LCD back-light.
 *     vSetBacklightState( BACKLIGHT_OFF );
 * }
 *
 * // The key press interrupt service routine.
 * void vKeyPressEventInterruptHandler( void )
 * {
 * BaseType_t xHigherPriorityTaskWoken = pdFALSE;
 *
 *     // Ensure the LCD back-light is on, then reset the timer that is
 *     // responsible for turning the back-light off after 5 seconds of
 *     // key inactivity.  This is an interrupt service routine so can only
 *     // call FreeRTOS API functions that end in "FromISR".
 *     vSetBacklightState( BACKLIGHT_ON );
 *
 *     // xTimerStartFromISR() or xTimerResetFromISR() could be called here
 *     // as both cause the timer to re-calculate its expiry time.
 *     // xHigherPriorityTaskWoken was initialised to pdFALSE when it was
 *     // declared (in this function).
 *     if( xTimerResetFromISR( xBacklightTimer, &xHigherPriorityTaskWoken ) != pdPASS )
 *     {
 *         // The reset command was not executed successfully.  Take appropriate
 *         // action here.
 *     }
 *
 *     // Perform the rest of the key processing here.
 *
 *     // If xHigherPriorityTaskWoken equals pdTRUE, then a context switch
 *     // should be performed.  The syntax required to perform a context switch
 *     // from inside an ISR varies from port to port, and from compiler to
 *     // compiler.  Inspect the demos for the port you are using to find the
 *     // actual syntax required.
 *     if( xHigherPriorityTaskWoken != pdFALSE )
 *     {
 *         // Call the interrupt safe yield function here (actual function
 *         // depends on the FreeRTOS port being used).
 *     }
 * }
 * @endverbatim
 */
#define xTimerResetFromISR( xTimer, pxHigherPriorityTaskWoken ) xTimerGenericCommand( ( xTimer ), tmrCOMMAND_RESET_FROM_ISR, ( xTaskGetTickCountFromISR() ), ( pxHigherPriorityTaskWoken ), 0U )


/**
 * BaseType_t xTimerPendFunctionCallFromISR( PendedFunction_t xFunctionToPend,
 *                                          void *pvParameter1,
 *                                          uint32_t ulParameter2,
 *                                          BaseType_t *pxHigherPriorityTaskWoken );
 *
 *
 * Used from application interrupt service routines to defer the execution of a
 * function to the RTOS daemon task (the timer service task, hence this function
 * is implemented in timers.c and is prefixed with 'Timer').
 *
 * Ideally an interrupt service routine (ISR) is kept as short as possible, but
 * sometimes an ISR either has a lot of processing to do, or needs to perform
 * processing that is not deterministic.  In these cases
 * xTimerPendFunctionCallFromISR() can be used to defer processing of a function
 * to the RTOS daemon task.
 *
 * A mechanism is provided that allows the interrupt to return directly to the
 * task that will subsequently execute the pended callback function.  This
 * allows the callback function to execute contiguously in time with the
 * interrupt - just as if the callback had executed in the interrupt itself.
 *
 * @param xFunctionToPend The function to execute from the timer service/
 * daemon task.  The function must conform to the PendedFunction_t
 * prototype.
 *
 * @param pvParameter1 The value of the callback function's first parameter.
 * The parameter has a void * type to allow it to be used to pass any type.
 * For example, unsigned longs can be cast to a void *, or the void * can be
 * used to point to a structure.
 *
 * @param ulParameter2 The value of the callback function's second parameter.
 *
 * @param pxHigherPriorityTaskWoken As mentioned above, calling this function
 * will result in a message being sent to the timer daemon task.  If the
 * priority of the timer daemon task (which is set using
 * configTIMER_TASK_PRIORITY in FreeRTOSConfig.h) is higher than the priority of
 * the currently running task (the task the interrupt interrupted) then
 * *pxHigherPriorityTaskWoken will be set to pdTRUE within
 * xTimerPendFunctionCallFromISR(), indicating that a context switch should be
 * requested before the interrupt exits.  For that reason
 * *pxHigherPriorityTaskWoken must be initialised to pdFALSE.  See the
 * example code below.
 *
 * @return pdPASS is returned if the message was successfully sent to the
 * timer daemon task, otherwise pdFALSE is returned.
 *
 * Example usage:
 * @verbatim
 *
 *	// The callback function that will execute in the context of the daemon task.
 *  // Note callback functions must all use this same prototype.
 *  void vProcessInterface( void *pvParameter1, uint32_t ulParameter2 )
 *	{
 *		BaseType_t xInterfaceToService;
 *
 *		// The interface that requires servicing is passed in the second
 *      // parameter.  The first parameter is not used in this case.
 *		xInterfaceToService = ( BaseType_t ) ulParameter2;
 *
 *		// ...Perform the processing here...
 *	}
 *
 *	// An ISR that receives data packets from multiple interfaces
 *  void vAnISR( void )
 *	{
 *		BaseType_t xInterfaceToService, xHigherPriorityTaskWoken;
 *
 *		// Query the hardware to determine which interface needs processing.
 *		xInterfaceToService = prvCheckInterfaces();
 *
 *      // The actual processing is to be deferred to a task.  Request the
 *      // vProcessInterface() callback function is executed, passing in the
 *		// number of the interface that needs processing.  The interface to
 *		// service is passed in the second parameter.  The first parameter is
 *		// not used in this case.
 *		xHigherPriorityTaskWoken = pdFALSE;
 *		xTimerPendFunctionCallFromISR( vProcessInterface, NULL, ( uint32_t ) xInterfaceToService, &xHigherPriorityTaskWoken );
 *
 *		// If xHigherPriorityTaskWoken is now set to pdTRUE then a context
 *		// switch should be requested.  The macro used is port specific and will
 *		// be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
 *		// the documentation page for the port being used.
 *		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
 *
 *	}
 * @endverbatim
 */
BaseType_t xTimerPendFunctionCallFromISR( PendedFunction_t xFunctionToPend, void *pvParameter1, uint32_t ulParameter2, BaseType_t *pxHigherPriorityTaskWoken ) PRIVILEGED_FUNCTION;

 /**
  * BaseType_t xTimerPendFunctionCall( PendedFunction_t xFunctionToPend,
  *                                    void *pvParameter1,
  *                                    uint32_t ulParameter2,
  *                                    TickType_t xTicksToWait );
  *
  *
  * Used to defer the execution of a function to the RTOS daemon task (the timer
  * service task, hence this function is implemented in timers.c and is prefixed
  * with 'Timer').
  *
  * @param xFunctionToPend The function to execute from the timer service/
  * daemon task.  The function must conform to the PendedFunction_t
  * prototype.
  *
  * @param pvParameter1 The value of the callback function's first parameter.
  * The parameter has a void * type to allow it to be used to pass any type.
  * For example, unsigned longs can be cast to a void *, or the void * can be
  * used to point to a structure.
  *
  * @param ulParameter2 The value of the callback function's second parameter.
  *
  * @param xTicksToWait Calling this function will result in a message being
  * sent to the timer daemon task on a queue.  xTicksToWait is the amount of
  * time the calling task should remain in the Blocked state (so not using any
  * processing time) for space to become available on the timer queue if the
  * queue is found to be full.
  *
  * @return pdPASS is returned if the message was successfully sent to the
  * timer daemon task, otherwise pdFALSE is returned.
  *
  */
BaseType_t xTimerPendFunctionCall( PendedFunction_t xFunctionToPend, void *pvParameter1, uint32_t ulParameter2, TickType_t xTicksToWait ) PRIVILEGED_FUNCTION;

/**
 * const char * const pcTimerGetName( TimerHandle_t xTimer );
 *
 * Returns the name that was assigned to a timer when the timer was created.
 *
 * @param xTimer The handle of the timer being queried.
 *
 * @return The name assigned to the timer specified by the xTimer parameter.
 */
const char * pcTimerGetName( TimerHandle_t xTimer ) PRIVILEGED_FUNCTION; /*lint !e971 Unqualified char types are allowed for strings and single characters only. */

/**
 * TickType_t xTimerGetPeriod( TimerHandle_t xTimer );
 *
 * Returns the period of a timer.
 *
 * @param xTimer The handle of the timer being queried.
 *
 * @return The period of the timer in ticks.
 */
TickType_t xTimerGetPeriod( TimerHandle_t xTimer ) PRIVILEGED_FUNCTION;

/**
* TickType_t xTimerGetExpiryTime( TimerHandle_t xTimer );
*
* Returns the time in ticks at which the timer will expire.  If this is less
* than the current tick count then the expiry time has overflowed from the
* current time.
*
* @param xTimer The handle of the timer being queried.
*
* @return If the timer is running then the time in ticks at which the timer
* will next expire is returned.  If the timer is not running then the return
* value is undefined.
*/
TickType_t xTimerGetExpiryTime( TimerHandle_t xTimer ) PRIVILEGED_FUNCTION;

/*
 * Functions beyond this part are not part of the public API and are intended
 * for use by the kernel only.
 */
BaseType_t xTimerCreateTimerTask( void ) PRIVILEGED_FUNCTION;
BaseType_t xTimerGenericCommand( TimerHandle_t xTimer, const BaseType_t xCommandID, const TickType_t xOptionalValue, BaseType_t * const pxHigherPriorityTaskWoken, const TickType_t xTicksToWait ) PRIVILEGED_FUNCTION;

#if( configUSE_TRACE_FACILITY == 1 )
	void vTimerSetTimerNumber( TimerHandle_t xTimer, UBaseType_t uxTimerNumber ) PRIVILEGED_FUNCTION;
	UBaseType_t uxTimerGetTimerNumber( TimerHandle_t xTimer ) PRIVILEGED_FUNCTION;
#endif

#ifdef __cplusplus
}
#endif
#endif /* TIMERS_H */



