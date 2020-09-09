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

/* Standard includes. */
#include <stdlib.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#if ( INCLUDE_xTimerPendFunctionCall == 1 ) && ( configUSE_TIMERS == 0 )
	#error configUSE_TIMERS must be set to 1 to make the xTimerPendFunctionCall() function available.
#endif

/* Lint e961 and e750 are suppressed as a MISRA exception justified because the
MPU ports require MPU_WRAPPERS_INCLUDED_FROM_API_FILE to be defined for the
header files above, but not in this file, in order to generate the correct
privileged Vs unprivileged linkage and placement. */
#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE /*lint !e961 !e750. */


/* This entire source file will be skipped if the application is not configured
to include software timer functionality.  This #if is closed at the very bottom
of this file.  If you want to include software timer functionality then ensure
configUSE_TIMERS is set to 1 in FreeRTOSConfig.h. */
#if ( configUSE_TIMERS == 1 )

/* Misc definitions. */
#define tmrNO_DELAY		( TickType_t ) 0U

/* The name assigned to the timer service task.  This can be overridden by
defining trmTIMER_SERVICE_TASK_NAME in FreeRTOSConfig.h. */
#ifndef configTIMER_SERVICE_TASK_NAME
	#define configTIMER_SERVICE_TASK_NAME "Tmr Svc"
#endif

/* The definition of the timers themselves. */
typedef struct tmrTimerControl
{
	const char				*pcTimerName;		/*<< Text name.  This is not used by the kernel, it is included simply to make debugging easier. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
	ListItem_t				xTimerListItem;		/*<< Standard linked list item as used by all kernel features for event management. */
	TickType_t				xTimerPeriodInTicks;/*<< How quickly and often the timer expires. */
	UBaseType_t				uxAutoReload;		/*<< Set to pdTRUE if the timer should be automatically restarted once expired.  Set to pdFALSE if the timer is, in effect, a one-shot timer. */
	void 					*pvTimerID;			/*<< An ID to identify the timer.  This allows the timer to be identified when the same callback is used for multiple timers. */
	TimerCallbackFunction_t	pxCallbackFunction;	/*<< The function that will be called when the timer expires. */
	#if( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t			uxTimerNumber;		/*<< An ID assigned by trace tools such as FreeRTOS+Trace */
	#endif

	#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
		uint8_t 			ucStaticallyAllocated; /*<< Set to pdTRUE if the timer was created statically so no attempt is made to free the memory again if the timer is later deleted. */
	#endif
} xTIMER;

/* The old xTIMER name is maintained above then typedefed to the new Timer_t
name below to enable the use of older kernel aware debuggers. */
typedef xTIMER Timer_t;

/* The definition of messages that can be sent and received on the timer queue.
Two types of message can be queued - messages that manipulate a software timer,
and messages that request the execution of a non-timer related callback.  The
two message types are defined in two separate structures, xTimerParametersType
and xCallbackParametersType respectively. */
typedef struct tmrTimerParameters
{
	TickType_t			xMessageValue;		/*<< An optional value used by a subset of commands, for example, when changing the period of a timer. */
	Timer_t *			pxTimer;			/*<< The timer to which the command will be applied. */
} TimerParameter_t;


typedef struct tmrCallbackParameters
{
	PendedFunction_t	pxCallbackFunction;	/* << The callback function to execute. */
	void *pvParameter1;						/* << The value that will be used as the callback functions first parameter. */
	uint32_t ulParameter2;					/* << The value that will be used as the callback functions second parameter. */
} CallbackParameters_t;

/* The structure that contains the two message types, along with an identifier
that is used to determine which message type is valid. */
typedef struct tmrTimerQueueMessage
{
	BaseType_t			xMessageID;			/*<< The command being sent to the timer service task. */
	union
	{
		TimerParameter_t xTimerParameters;

		/* Don't include xCallbackParameters if it is not going to be used as
		it makes the structure (and therefore the timer queue) larger. */
		#if ( INCLUDE_xTimerPendFunctionCall == 1 )
			CallbackParameters_t xCallbackParameters;
		#endif /* INCLUDE_xTimerPendFunctionCall */
	} u;
} DaemonTaskMessage_t;

/*lint -save -e956 A manual analysis and inspection has been used to determine
which static variables must be declared volatile. */

/* The list in which active timers are stored.  Timers are referenced in expire
time order, with the nearest expiry time at the front of the list.  Only the
timer service task is allowed to access these lists. */
PRIVILEGED_DATA static List_t xActiveTimerList1 = {0};
PRIVILEGED_DATA static List_t xActiveTimerList2 = {0};
PRIVILEGED_DATA static List_t *pxCurrentTimerList = NULL;
PRIVILEGED_DATA static List_t *pxOverflowTimerList = NULL;

/* A queue that is used to send commands to the timer service task. */
PRIVILEGED_DATA static QueueHandle_t xTimerQueue = NULL;
PRIVILEGED_DATA static TaskHandle_t xTimerTaskHandle = NULL;

/*lint -restore */

/*-----------------------------------------------------------*/

#if( configSUPPORT_STATIC_ALLOCATION == 1 )

	/* If static allocation is supported then the application must provide the
	following callback function - which enables the application to optionally
	provide the memory that will be used by the timer task as the task's stack
	and TCB. */
	extern void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

#endif

/*
 * Initialise the infrastructure used by the timer service task if it has not
 * been initialised already.
 */
static void prvCheckForValidListAndQueue( void ) PRIVILEGED_FUNCTION;

/*
 * The timer service task (daemon).  Timer functionality is controlled by this
 * task.  Other tasks communicate with the timer service task using the
 * xTimerQueue queue.
 */
static void prvTimerTask( void *pvParameters ) PRIVILEGED_FUNCTION;

/*
 * Called by the timer service task to interpret and process a command it
 * received on the timer queue.
 */
static void prvProcessReceivedCommands( void ) PRIVILEGED_FUNCTION;

/*
 * Insert the timer into either xActiveTimerList1, or xActiveTimerList2,
 * depending on if the expire time causes a timer counter overflow.
 */
static BaseType_t prvInsertTimerInActiveList( Timer_t * const pxTimer, const TickType_t xNextExpiryTime, const TickType_t xTimeNow, const TickType_t xCommandTime ) PRIVILEGED_FUNCTION;

/*
 * An active timer has reached its expire time.  Reload the timer if it is an
 * auto reload timer, then call its callback.
 */
static void prvProcessExpiredTimer( const TickType_t xNextExpireTime, const TickType_t xTimeNow ) PRIVILEGED_FUNCTION;

/*
 * The tick count has overflowed.  Switch the timer lists after ensuring the
 * current timer list does not still reference some timers.
 */
static void prvSwitchTimerLists( void ) PRIVILEGED_FUNCTION;

/*
 * Obtain the current tick count, setting *pxTimerListsWereSwitched to pdTRUE
 * if a tick count overflow occurred since prvSampleTimeNow() was last called.
 */
static TickType_t prvSampleTimeNow( BaseType_t * const pxTimerListsWereSwitched ) PRIVILEGED_FUNCTION;

/*
 * If the timer list contains any active timers then return the expire time of
 * the timer that will expire first and set *pxListWasEmpty to false.  If the
 * timer list does not contain any timers then return 0 and set *pxListWasEmpty
 * to pdTRUE.
 */
static TickType_t prvGetNextExpireTime( BaseType_t * const pxListWasEmpty ) PRIVILEGED_FUNCTION;

/*
 * If a timer has expired, process it.  Otherwise, block the timer service task
 * until either a timer does expire or a command is received.
 */
static void prvProcessTimerOrBlockTask( const TickType_t xNextExpireTime, BaseType_t xListWasEmpty ) PRIVILEGED_FUNCTION;

/*
 * Called after a Timer_t structure has been allocated either statically or
 * dynamically to fill in the structure's members.
 */
static void prvInitialiseNewTimer(	const char * const pcTimerName,			/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
									const TickType_t xTimerPeriodInTicks,
									const UBaseType_t uxAutoReload,
									void * const pvTimerID,
									TimerCallbackFunction_t pxCallbackFunction,
									Timer_t *pxNewTimer ) PRIVILEGED_FUNCTION;
/*-----------------------------------------------------------*/

BaseType_t xTimerCreateTimerTask( void )
{
BaseType_t xReturn = pdFAIL;

	/* This function is called when the scheduler is started if
	configUSE_TIMERS is set to 1.  Check that the infrastructure used by the
	timer service task has been created/initialised.  If timers have already
	been created then the initialisation will already have been performed. */
	prvCheckForValidListAndQueue();

	if( xTimerQueue != NULL )
	{
		#if( configSUPPORT_STATIC_ALLOCATION == 1 )
		{
			StaticTask_t *pxTimerTaskTCBBuffer = NULL;
			StackType_t *pxTimerTaskStackBuffer = NULL;
			uint32_t ulTimerTaskStackSize;

			vApplicationGetTimerTaskMemory( &pxTimerTaskTCBBuffer, &pxTimerTaskStackBuffer, &ulTimerTaskStackSize );
			xTimerTaskHandle = xTaskCreateStatic(	prvTimerTask,
													configTIMER_SERVICE_TASK_NAME,
													ulTimerTaskStackSize,
													NULL,
													( ( UBaseType_t ) configTIMER_TASK_PRIORITY ) | portPRIVILEGE_BIT,
													pxTimerTaskStackBuffer,
													pxTimerTaskTCBBuffer );

			if( xTimerTaskHandle != NULL )
			{
				xReturn = pdPASS;
			}
		}
		#else
		{
			xReturn = xTaskCreate(	prvTimerTask,
									configTIMER_SERVICE_TASK_NAME,
									configTIMER_TASK_STACK_DEPTH,
									NULL,
									( ( UBaseType_t ) configTIMER_TASK_PRIORITY ) | portPRIVILEGE_BIT,
									&xTimerTaskHandle );
		}
		#endif /* configSUPPORT_STATIC_ALLOCATION */
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	configASSERT( xReturn );
	return xReturn;
}
/*-----------------------------------------------------------*/

#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )

	TimerHandle_t xTimerCreate(	const char * const pcTimerName,			/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
								const TickType_t xTimerPeriodInTicks,
								const UBaseType_t uxAutoReload,
								void * const pvTimerID,
								TimerCallbackFunction_t pxCallbackFunction )
	{
	Timer_t *pxNewTimer;

		pxNewTimer = ( Timer_t * ) pvPortMalloc( sizeof( Timer_t ) );

		if( pxNewTimer != NULL )
		{
			prvInitialiseNewTimer( pcTimerName, xTimerPeriodInTicks, uxAutoReload, pvTimerID, pxCallbackFunction, pxNewTimer );

			#if( configSUPPORT_STATIC_ALLOCATION == 1 )
			{
				/* Timers can be created statically or dynamically, so note this
				timer was created dynamically in case the timer is later
				deleted. */
				pxNewTimer->ucStaticallyAllocated = pdFALSE;
			}
			#endif /* configSUPPORT_STATIC_ALLOCATION */
		}

		return pxNewTimer;
	}

#endif /* configSUPPORT_STATIC_ALLOCATION */
/*-----------------------------------------------------------*/

#if( configSUPPORT_STATIC_ALLOCATION == 1 )

	TimerHandle_t xTimerCreateStatic(	const char * const pcTimerName,		/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
										const TickType_t xTimerPeriodInTicks,
										const UBaseType_t uxAutoReload,
										void * const pvTimerID,
										TimerCallbackFunction_t pxCallbackFunction,
										StaticTimer_t *pxTimerBuffer )
	{
	Timer_t *pxNewTimer;

		#if( configASSERT_DEFINED == 1 )
		{
			/* Sanity check that the size of the structure used to declare a
			variable of type StaticTimer_t equals the size of the real timer
			structure. */
			volatile size_t xSize = sizeof( StaticTimer_t );
			configASSERT( xSize == sizeof( Timer_t ) );
		}
		#endif /* configASSERT_DEFINED */

		/* A pointer to a StaticTimer_t structure MUST be provided, use it. */
		configASSERT( pxTimerBuffer );
		pxNewTimer = ( Timer_t * ) pxTimerBuffer; /*lint !e740 Unusual cast is ok as the structures are designed to have the same alignment, and the size is checked by an assert. */

		if( pxNewTimer != NULL )
		{
			prvInitialiseNewTimer( pcTimerName, xTimerPeriodInTicks, uxAutoReload, pvTimerID, pxCallbackFunction, pxNewTimer );

			#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
			{
				/* Timers can be created statically or dynamically so note this
				timer was created statically in case it is later deleted. */
				pxNewTimer->ucStaticallyAllocated = pdTRUE;
			}
			#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
		}

		return pxNewTimer;
	}

#endif /* configSUPPORT_STATIC_ALLOCATION */
/*-----------------------------------------------------------*/

static void prvInitialiseNewTimer(	const char * const pcTimerName,			/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
									const TickType_t xTimerPeriodInTicks,
									const UBaseType_t uxAutoReload,
									void * const pvTimerID,
									TimerCallbackFunction_t pxCallbackFunction,
									Timer_t *pxNewTimer )
{
	/* 0 is not a valid value for xTimerPeriodInTicks. */
	configASSERT( ( xTimerPeriodInTicks > 0 ) );

	if( pxNewTimer != NULL )
	{
		/* Ensure the infrastructure used by the timer service task has been
		created/initialised. */
		prvCheckForValidListAndQueue();

		/* Initialise the timer structure members using the function
		parameters. */
		pxNewTimer->pcTimerName = pcTimerName;
		pxNewTimer->xTimerPeriodInTicks = xTimerPeriodInTicks;
		pxNewTimer->uxAutoReload = uxAutoReload;
		pxNewTimer->pvTimerID = pvTimerID;
		pxNewTimer->pxCallbackFunction = pxCallbackFunction;
		vListInitialiseItem( &( pxNewTimer->xTimerListItem ) );
		traceTIMER_CREATE( pxNewTimer );
	}
}
/*-----------------------------------------------------------*/

BaseType_t xTimerGenericCommand( TimerHandle_t xTimer, const BaseType_t xCommandID, const TickType_t xOptionalValue, BaseType_t * const pxHigherPriorityTaskWoken, const TickType_t xTicksToWait )
{
BaseType_t xReturn = pdFAIL;
DaemonTaskMessage_t xMessage;

	configASSERT( xTimer );

	/* Send a message to the timer service task to perform a particular action
	on a particular timer definition. */
	if( xTimerQueue != NULL )
	{
		/* Send a command to the timer service task to start the xTimer timer. */
		xMessage.xMessageID = xCommandID;
		xMessage.u.xTimerParameters.xMessageValue = xOptionalValue;
		xMessage.u.xTimerParameters.pxTimer = ( Timer_t * ) xTimer;

		if( xCommandID < tmrFIRST_FROM_ISR_COMMAND )
		{
			if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
			{
				xReturn = xQueueSendToBack( xTimerQueue, &xMessage, xTicksToWait );
			}
			else
			{
				xReturn = xQueueSendToBack( xTimerQueue, &xMessage, tmrNO_DELAY );
			}
		}
		else
		{
			xReturn = xQueueSendToBackFromISR( xTimerQueue, &xMessage, pxHigherPriorityTaskWoken );
		}

		traceTIMER_COMMAND_SEND( xTimer, xCommandID, xOptionalValue, xReturn );
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

TaskHandle_t xTimerGetTimerDaemonTaskHandle( void )
{
	/* If xTimerGetTimerDaemonTaskHandle() is called before the scheduler has been
	started, then xTimerTaskHandle will be NULL. */
	configASSERT( ( xTimerTaskHandle != NULL ) );
	return xTimerTaskHandle;
}
/*-----------------------------------------------------------*/

TickType_t xTimerGetPeriod( TimerHandle_t xTimer )
{
Timer_t *pxTimer = ( Timer_t * ) xTimer;

	configASSERT( xTimer );
	return pxTimer->xTimerPeriodInTicks;
}
/*-----------------------------------------------------------*/

TickType_t xTimerGetExpiryTime( TimerHandle_t xTimer )
{
Timer_t * pxTimer = ( Timer_t * ) xTimer;
TickType_t xReturn;

	configASSERT( xTimer );
	xReturn = listGET_LIST_ITEM_VALUE( &( pxTimer->xTimerListItem ) );
	return xReturn;
}
/*-----------------------------------------------------------*/

const char * pcTimerGetName( TimerHandle_t xTimer ) /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
{
Timer_t *pxTimer = ( Timer_t * ) xTimer;

	configASSERT( xTimer );
	return pxTimer->pcTimerName;
}
/*-----------------------------------------------------------*/

static void prvProcessExpiredTimer( const TickType_t xNextExpireTime, const TickType_t xTimeNow )
{
BaseType_t xResult;
Timer_t * const pxTimer = ( Timer_t * ) listGET_OWNER_OF_HEAD_ENTRY( pxCurrentTimerList );

	/* Remove the timer from the list of active timers.  A check has already
	been performed to ensure the list is not empty. */
	( void ) uxListRemove( &( pxTimer->xTimerListItem ) );
	traceTIMER_EXPIRED( pxTimer );

	/* If the timer is an auto reload timer then calculate the next
	expiry time and re-insert the timer in the list of active timers. */
	if( pxTimer->uxAutoReload == ( UBaseType_t ) pdTRUE )
	{
		/* The timer is inserted into a list using a time relative to anything
		other than the current time.  It will therefore be inserted into the
		correct list relative to the time this task thinks it is now. */
		if( prvInsertTimerInActiveList( pxTimer, ( xNextExpireTime + pxTimer->xTimerPeriodInTicks ), xTimeNow, xNextExpireTime ) != pdFALSE )
		{
			/* The timer expired before it was added to the active timer
			list.  Reload it now.  */
			xResult = xTimerGenericCommand( pxTimer, tmrCOMMAND_START_DONT_TRACE, xNextExpireTime, NULL, tmrNO_DELAY );
			configASSERT( xResult );
			( void ) xResult;
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	/* Call the timer callback. */
	pxTimer->pxCallbackFunction( ( TimerHandle_t ) pxTimer );
}
/*-----------------------------------------------------------*/

static void prvTimerTask( void *pvParameters )
{
TickType_t xNextExpireTime;
BaseType_t xListWasEmpty;

	/* Just to avoid compiler warnings. */
	( void ) pvParameters;

	#if( configUSE_DAEMON_TASK_STARTUP_HOOK == 1 )
	{
		extern void vApplicationDaemonTaskStartupHook( void );

		/* Allow the application writer to execute some code in the context of
		this task at the point the task starts executing.  This is useful if the
		application includes initialisation code that would benefit from
		executing after the scheduler has been started. */
		vApplicationDaemonTaskStartupHook();
	}
	#endif /* configUSE_DAEMON_TASK_STARTUP_HOOK */

	for( ;; )
	{
		/* Query the timers list to see if it contains any timers, and if so,
		obtain the time at which the next timer will expire. */
		xNextExpireTime = prvGetNextExpireTime( &xListWasEmpty );

		/* If a timer has expired, process it.  Otherwise, block this task
		until either a timer does expire, or a command is received. */
		prvProcessTimerOrBlockTask( xNextExpireTime, xListWasEmpty );

		/* Empty the command queue. */
		prvProcessReceivedCommands();
	}
}
/*-----------------------------------------------------------*/

static void prvProcessTimerOrBlockTask( const TickType_t xNextExpireTime, BaseType_t xListWasEmpty )
{
TickType_t xTimeNow;
BaseType_t xTimerListsWereSwitched;

	vTaskSuspendAll();
	{
		/* Obtain the time now to make an assessment as to whether the timer
		has expired or not.  If obtaining the time causes the lists to switch
		then don't process this timer as any timers that remained in the list
		when the lists were switched will have been processed within the
		prvSampleTimeNow() function. */
		xTimeNow = prvSampleTimeNow( &xTimerListsWereSwitched );
		if( xTimerListsWereSwitched == pdFALSE )
		{
			/* The tick count has not overflowed, has the timer expired? */
			if( ( xListWasEmpty == pdFALSE ) && ( xNextExpireTime <= xTimeNow ) )
			{
				( void ) xTaskResumeAll();
				prvProcessExpiredTimer( xNextExpireTime, xTimeNow );
			}
			else
			{
				/* The tick count has not overflowed, and the next expire
				time has not been reached yet.  This task should therefore
				block to wait for the next expire time or a command to be
				received - whichever comes first.  The following line cannot
				be reached unless xNextExpireTime > xTimeNow, except in the
				case when the current timer list is empty. */
				if( xListWasEmpty != pdFALSE )
				{
					/* The current timer list is empty - is the overflow list
					also empty? */
					xListWasEmpty = listLIST_IS_EMPTY( pxOverflowTimerList );
				}

				vQueueWaitForMessageRestricted( xTimerQueue, ( xNextExpireTime - xTimeNow ), xListWasEmpty );

				if( xTaskResumeAll() == pdFALSE )
				{
					/* Yield to wait for either a command to arrive, or the
					block time to expire.  If a command arrived between the
					critical section being exited and this yield then the yield
					will not cause the task to block. */
					portYIELD_WITHIN_API();
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
		}
		else
		{
			( void ) xTaskResumeAll();
		}
	}
}
/*-----------------------------------------------------------*/

static TickType_t prvGetNextExpireTime( BaseType_t * const pxListWasEmpty )
{
TickType_t xNextExpireTime;

	/* Timers are listed in expiry time order, with the head of the list
	referencing the task that will expire first.  Obtain the time at which
	the timer with the nearest expiry time will expire.  If there are no
	active timers then just set the next expire time to 0.  That will cause
	this task to unblock when the tick count overflows, at which point the
	timer lists will be switched and the next expiry time can be
	re-assessed.  */
	*pxListWasEmpty = listLIST_IS_EMPTY( pxCurrentTimerList );
	if( *pxListWasEmpty == pdFALSE )
	{
		xNextExpireTime = listGET_ITEM_VALUE_OF_HEAD_ENTRY( pxCurrentTimerList );
	}
	else
	{
		/* Ensure the task unblocks when the tick count rolls over. */
		xNextExpireTime = ( TickType_t ) 0U;
	}

	return xNextExpireTime;
}
/*-----------------------------------------------------------*/

static TickType_t prvSampleTimeNow( BaseType_t * const pxTimerListsWereSwitched )
{
TickType_t xTimeNow;
PRIVILEGED_DATA static TickType_t xLastTime = ( TickType_t ) 0U; /*lint !e956 Variable is only accessible to one task. */

	xTimeNow = xTaskGetTickCount();

	if( xTimeNow < xLastTime )
	{
		prvSwitchTimerLists();
		*pxTimerListsWereSwitched = pdTRUE;
	}
	else
	{
		*pxTimerListsWereSwitched = pdFALSE;
	}

	xLastTime = xTimeNow;

	return xTimeNow;
}
/*-----------------------------------------------------------*/

static BaseType_t prvInsertTimerInActiveList( Timer_t * const pxTimer, const TickType_t xNextExpiryTime, const TickType_t xTimeNow, const TickType_t xCommandTime )
{
BaseType_t xProcessTimerNow = pdFALSE;

	listSET_LIST_ITEM_VALUE( &( pxTimer->xTimerListItem ), xNextExpiryTime );
	listSET_LIST_ITEM_OWNER( &( pxTimer->xTimerListItem ), pxTimer );

	if( xNextExpiryTime <= xTimeNow )
	{
		/* Has the expiry time elapsed between the command to start/reset a
		timer was issued, and the time the command was processed? */
		if( ( ( TickType_t ) ( xTimeNow - xCommandTime ) ) >= pxTimer->xTimerPeriodInTicks ) /*lint !e961 MISRA exception as the casts are only redundant for some ports. */
		{
			/* The time between a command being issued and the command being
			processed actually exceeds the timers period.  */
			xProcessTimerNow = pdTRUE;
		}
		else
		{
			vListInsert( pxOverflowTimerList, &( pxTimer->xTimerListItem ) );
		}
	}
	else
	{
		if( ( xTimeNow < xCommandTime ) && ( xNextExpiryTime >= xCommandTime ) )
		{
			/* If, since the command was issued, the tick count has overflowed
			but the expiry time has not, then the timer must have already passed
			its expiry time and should be processed immediately. */
			xProcessTimerNow = pdTRUE;
		}
		else
		{
			vListInsert( pxCurrentTimerList, &( pxTimer->xTimerListItem ) );
		}
	}

	return xProcessTimerNow;
}
/*-----------------------------------------------------------*/

static void	prvProcessReceivedCommands( void )
{
DaemonTaskMessage_t xMessage;
Timer_t *pxTimer;
BaseType_t xTimerListsWereSwitched, xResult;
TickType_t xTimeNow;

	while( xQueueReceive( xTimerQueue, &xMessage, tmrNO_DELAY ) != pdFAIL ) /*lint !e603 xMessage does not have to be initialised as it is passed out, not in, and it is not used unless xQueueReceive() returns pdTRUE. */
	{
		#if ( INCLUDE_xTimerPendFunctionCall == 1 )
		{
			/* Negative commands are pended function calls rather than timer
			commands. */
			if( xMessage.xMessageID < ( BaseType_t ) 0 )
			{
				const CallbackParameters_t * const pxCallback = &( xMessage.u.xCallbackParameters );

				/* The timer uses the xCallbackParameters member to request a
				callback be executed.  Check the callback is not NULL. */
				configASSERT( pxCallback );

				/* Call the function. */
				pxCallback->pxCallbackFunction( pxCallback->pvParameter1, pxCallback->ulParameter2 );
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		#endif /* INCLUDE_xTimerPendFunctionCall */

		/* Commands that are positive are timer commands rather than pended
		function calls. */
		if( xMessage.xMessageID >= ( BaseType_t ) 0 )
		{
			/* The messages uses the xTimerParameters member to work on a
			software timer. */
			pxTimer = xMessage.u.xTimerParameters.pxTimer;

			if( listIS_CONTAINED_WITHIN( NULL, &( pxTimer->xTimerListItem ) ) == pdFALSE ) /*lint !e961. The cast is only redundant when NULL is passed into the macro. */
			{
				/* The timer is in a list, remove it. */
				( void ) uxListRemove( &( pxTimer->xTimerListItem ) );
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			traceTIMER_COMMAND_RECEIVED( pxTimer, xMessage.xMessageID, xMessage.u.xTimerParameters.xMessageValue );

			/* In this case the xTimerListsWereSwitched parameter is not used, but
			it must be present in the function call.  prvSampleTimeNow() must be
			called after the message is received from xTimerQueue so there is no
			possibility of a higher priority task adding a message to the message
			queue with a time that is ahead of the timer daemon task (because it
			pre-empted the timer daemon task after the xTimeNow value was set). */
			xTimeNow = prvSampleTimeNow( &xTimerListsWereSwitched );

			switch( xMessage.xMessageID )
			{
				case tmrCOMMAND_START :
			    case tmrCOMMAND_START_FROM_ISR :
			    case tmrCOMMAND_RESET :
			    case tmrCOMMAND_RESET_FROM_ISR :
				case tmrCOMMAND_START_DONT_TRACE :
					/* Start or restart a timer. */
					if( prvInsertTimerInActiveList( pxTimer,  xMessage.u.xTimerParameters.xMessageValue + pxTimer->xTimerPeriodInTicks, xTimeNow, xMessage.u.xTimerParameters.xMessageValue ) != pdFALSE )
					{
						/* The timer expired before it was added to the active
						timer list.  Process it now. */
						pxTimer->pxCallbackFunction( ( TimerHandle_t ) pxTimer );
						traceTIMER_EXPIRED( pxTimer );

						if( pxTimer->uxAutoReload == ( UBaseType_t ) pdTRUE )
						{
							xResult = xTimerGenericCommand( pxTimer, tmrCOMMAND_START_DONT_TRACE, xMessage.u.xTimerParameters.xMessageValue + pxTimer->xTimerPeriodInTicks, NULL, tmrNO_DELAY );
							configASSERT( xResult );
							( void ) xResult;
						}
						else
						{
							mtCOVERAGE_TEST_MARKER();
						}
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}
					break;

				case tmrCOMMAND_STOP :
				case tmrCOMMAND_STOP_FROM_ISR :
					/* The timer has already been removed from the active list.
					There is nothing to do here. */
					break;

				case tmrCOMMAND_CHANGE_PERIOD :
				case tmrCOMMAND_CHANGE_PERIOD_FROM_ISR :
					pxTimer->xTimerPeriodInTicks = xMessage.u.xTimerParameters.xMessageValue;
					configASSERT( ( pxTimer->xTimerPeriodInTicks > 0 ) );

					/* The new period does not really have a reference, and can
					be longer or shorter than the old one.  The command time is
					therefore set to the current time, and as the period cannot
					be zero the next expiry time can only be in the future,
					meaning (unlike for the xTimerStart() case above) there is
					no fail case that needs to be handled here. */
					( void ) prvInsertTimerInActiveList( pxTimer, ( xTimeNow + pxTimer->xTimerPeriodInTicks ), xTimeNow, xTimeNow );
					break;

				case tmrCOMMAND_DELETE :
					/* The timer has already been removed from the active list,
					just free up the memory if the memory was dynamically
					allocated. */
					#if( ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 0 ) )
					{
						/* The timer can only have been allocated dynamically -
						free it again. */
						vPortFree( pxTimer );
					}
					#elif( ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 1 ) )
					{
						/* The timer could have been allocated statically or
						dynamically, so check before attempting to free the
						memory. */
						if( pxTimer->ucStaticallyAllocated == ( uint8_t ) pdFALSE )
						{
							vPortFree( pxTimer );
						}
						else
						{
							mtCOVERAGE_TEST_MARKER();
						}
					}
					#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
					break;

				default	:
					/* Don't expect to get here. */
					break;
			}
		}
	}
}
/*-----------------------------------------------------------*/

static void prvSwitchTimerLists( void )
{
TickType_t xNextExpireTime, xReloadTime;
List_t *pxTemp;
Timer_t *pxTimer;
BaseType_t xResult;

	/* The tick count has overflowed.  The timer lists must be switched.
	If there are any timers still referenced from the current timer list
	then they must have expired and should be processed before the lists
	are switched. */
	while( listLIST_IS_EMPTY( pxCurrentTimerList ) == pdFALSE )
	{
		xNextExpireTime = listGET_ITEM_VALUE_OF_HEAD_ENTRY( pxCurrentTimerList );

		/* Remove the timer from the list. */
		pxTimer = ( Timer_t * ) listGET_OWNER_OF_HEAD_ENTRY( pxCurrentTimerList );
		( void ) uxListRemove( &( pxTimer->xTimerListItem ) );
		traceTIMER_EXPIRED( pxTimer );

		/* Execute its callback, then send a command to restart the timer if
		it is an auto-reload timer.  It cannot be restarted here as the lists
		have not yet been switched. */
		pxTimer->pxCallbackFunction( ( TimerHandle_t ) pxTimer );

		if( pxTimer->uxAutoReload == ( UBaseType_t ) pdTRUE )
		{
			/* Calculate the reload value, and if the reload value results in
			the timer going into the same timer list then it has already expired
			and the timer should be re-inserted into the current list so it is
			processed again within this loop.  Otherwise a command should be sent
			to restart the timer to ensure it is only inserted into a list after
			the lists have been swapped. */
			xReloadTime = ( xNextExpireTime + pxTimer->xTimerPeriodInTicks );
			if( xReloadTime > xNextExpireTime )
			{
				listSET_LIST_ITEM_VALUE( &( pxTimer->xTimerListItem ), xReloadTime );
				listSET_LIST_ITEM_OWNER( &( pxTimer->xTimerListItem ), pxTimer );
				vListInsert( pxCurrentTimerList, &( pxTimer->xTimerListItem ) );
			}
			else
			{
				xResult = xTimerGenericCommand( pxTimer, tmrCOMMAND_START_DONT_TRACE, xNextExpireTime, NULL, tmrNO_DELAY );
				configASSERT( xResult );
				( void ) xResult;
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}

	pxTemp = pxCurrentTimerList;
	pxCurrentTimerList = pxOverflowTimerList;
	pxOverflowTimerList = pxTemp;
}
/*-----------------------------------------------------------*/

static void prvCheckForValidListAndQueue( void )
{
	/* Check that the list from which active timers are referenced, and the
	queue used to communicate with the timer service, have been
	initialised. */
	taskENTER_CRITICAL();
	{
		if( xTimerQueue == NULL )
		{
			vListInitialise( &xActiveTimerList1 );
			vListInitialise( &xActiveTimerList2 );
			pxCurrentTimerList = &xActiveTimerList1;
			pxOverflowTimerList = &xActiveTimerList2;

			#if( configSUPPORT_STATIC_ALLOCATION == 1 )
			{
				/* The timer queue is allocated statically in case
				configSUPPORT_DYNAMIC_ALLOCATION is 0. */
				static StaticQueue_t xStaticTimerQueue; /*lint !e956 Ok to declare in this manner to prevent additional conditional compilation guards in other locations. */
				static uint8_t ucStaticTimerQueueStorage[ ( size_t ) configTIMER_QUEUE_LENGTH * sizeof( DaemonTaskMessage_t ) ]; /*lint !e956 Ok to declare in this manner to prevent additional conditional compilation guards in other locations. */

				xTimerQueue = xQueueCreateStatic( ( UBaseType_t ) configTIMER_QUEUE_LENGTH, ( UBaseType_t ) sizeof( DaemonTaskMessage_t ), &( ucStaticTimerQueueStorage[ 0 ] ), &xStaticTimerQueue );
			}
			#else
			{
				xTimerQueue = xQueueCreate( ( UBaseType_t ) configTIMER_QUEUE_LENGTH, sizeof( DaemonTaskMessage_t ) );
			}
			#endif

			#if ( configQUEUE_REGISTRY_SIZE > 0 )
			{
				if( xTimerQueue != NULL )
				{
					vQueueAddToRegistry( xTimerQueue, "TmrQ" );
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			#endif /* configQUEUE_REGISTRY_SIZE */
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

BaseType_t xTimerIsTimerActive( TimerHandle_t xTimer )
{
BaseType_t xTimerIsInActiveList;
Timer_t *pxTimer = ( Timer_t * ) xTimer;

	configASSERT( xTimer );

	/* Is the timer in the list of active timers? */
	taskENTER_CRITICAL();
	{
		/* Checking to see if it is in the NULL list in effect checks to see if
		it is referenced from either the current or the overflow timer lists in
		one go, but the logic has to be reversed, hence the '!'. */
		xTimerIsInActiveList = ( BaseType_t ) !( listIS_CONTAINED_WITHIN( NULL, &( pxTimer->xTimerListItem ) ) ); /*lint !e961. Cast is only redundant when NULL is passed into the macro. */
	}
	taskEXIT_CRITICAL();

	return xTimerIsInActiveList;
} /*lint !e818 Can't be pointer to const due to the typedef. */
/*-----------------------------------------------------------*/

void *pvTimerGetTimerID( const TimerHandle_t xTimer )
{
Timer_t * const pxTimer = ( Timer_t * ) xTimer;
void *pvReturn;

	configASSERT( xTimer );

	taskENTER_CRITICAL();
	{
		pvReturn = pxTimer->pvTimerID;
	}
	taskEXIT_CRITICAL();

	return pvReturn;
}
/*-----------------------------------------------------------*/

void vTimerSetTimerID( TimerHandle_t xTimer, void *pvNewID )
{
Timer_t * const pxTimer = ( Timer_t * ) xTimer;

	configASSERT( xTimer );

	taskENTER_CRITICAL();
	{
		pxTimer->pvTimerID = pvNewID;
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

#if( INCLUDE_xTimerPendFunctionCall == 1 )

	BaseType_t xTimerPendFunctionCallFromISR( PendedFunction_t xFunctionToPend, void *pvParameter1, uint32_t ulParameter2, BaseType_t *pxHigherPriorityTaskWoken )
	{
	DaemonTaskMessage_t xMessage;
	BaseType_t xReturn;

		/* Complete the message with the function parameters and post it to the
		daemon task. */
		xMessage.xMessageID = tmrCOMMAND_EXECUTE_CALLBACK_FROM_ISR;
		xMessage.u.xCallbackParameters.pxCallbackFunction = xFunctionToPend;
		xMessage.u.xCallbackParameters.pvParameter1 = pvParameter1;
		xMessage.u.xCallbackParameters.ulParameter2 = ulParameter2;

		xReturn = xQueueSendFromISR( xTimerQueue, &xMessage, pxHigherPriorityTaskWoken );

		tracePEND_FUNC_CALL_FROM_ISR( xFunctionToPend, pvParameter1, ulParameter2, xReturn );

		return xReturn;
	}

#endif /* INCLUDE_xTimerPendFunctionCall */
/*-----------------------------------------------------------*/

#if( INCLUDE_xTimerPendFunctionCall == 1 )

	BaseType_t xTimerPendFunctionCall( PendedFunction_t xFunctionToPend, void *pvParameter1, uint32_t ulParameter2, TickType_t xTicksToWait )
	{
	DaemonTaskMessage_t xMessage;
	BaseType_t xReturn;

		/* This function can only be called after a timer has been created or
		after the scheduler has been started because, until then, the timer
		queue does not exist. */
		configASSERT( xTimerQueue );

		/* Complete the message with the function parameters and post it to the
		daemon task. */
		xMessage.xMessageID = tmrCOMMAND_EXECUTE_CALLBACK;
		xMessage.u.xCallbackParameters.pxCallbackFunction = xFunctionToPend;
		xMessage.u.xCallbackParameters.pvParameter1 = pvParameter1;
		xMessage.u.xCallbackParameters.ulParameter2 = ulParameter2;

		xReturn = xQueueSendToBack( xTimerQueue, &xMessage, xTicksToWait );

		tracePEND_FUNC_CALL( xFunctionToPend, pvParameter1, ulParameter2, xReturn );

		return xReturn;
	}

#endif /* INCLUDE_xTimerPendFunctionCall */
/*-----------------------------------------------------------*/

#if ( configUSE_TRACE_FACILITY == 1 )

	UBaseType_t uxTimerGetTimerNumber( TimerHandle_t xTimer )
	{
		return ( ( Timer_t * ) xTimer )->uxTimerNumber;
	}

#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if ( configUSE_TRACE_FACILITY == 1 )

	void vTimerSetTimerNumber( TimerHandle_t xTimer, UBaseType_t uxTimerNumber )
	{
		( ( Timer_t * ) xTimer )->uxTimerNumber = uxTimerNumber;
	}

#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

/* This entire source file will be skipped if the application is not configured
to include software timer functionality.  If you want to include software timer
functionality then ensure configUSE_TIMERS is set to 1 in FreeRTOSConfig.h. */
#endif /* configUSE_TIMERS == 1 */



