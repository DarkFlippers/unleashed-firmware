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

#include <stdlib.h>
#include <string.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#if ( configUSE_CO_ROUTINES == 1 )
	#include "croutine.h"
#endif

/* Lint e961 and e750 are suppressed as a MISRA exception justified because the
MPU ports require MPU_WRAPPERS_INCLUDED_FROM_API_FILE to be defined for the
header files above, but not in this file, in order to generate the correct
privileged Vs unprivileged linkage and placement. */
#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE /*lint !e961 !e750. */


/* Constants used with the cRxLock and cTxLock structure members. */
#define queueUNLOCKED					( ( int8_t ) -1 )
#define queueLOCKED_UNMODIFIED			( ( int8_t ) 0 )

/* When the Queue_t structure is used to represent a base queue its pcHead and
pcTail members are used as pointers into the queue storage area.  When the
Queue_t structure is used to represent a mutex pcHead and pcTail pointers are
not necessary, and the pcHead pointer is set to NULL to indicate that the
pcTail pointer actually points to the mutex holder (if any).  Map alternative
names to the pcHead and pcTail structure members to ensure the readability of
the code is maintained despite this dual use of two structure members.  An
alternative implementation would be to use a union, but use of a union is
against the coding standard (although an exception to the standard has been
permitted where the dual use also significantly changes the type of the
structure member). */
#define pxMutexHolder					pcTail
#define uxQueueType						pcHead
#define queueQUEUE_IS_MUTEX				NULL

/* Semaphores do not actually store or copy data, so have an item size of
zero. */
#define queueSEMAPHORE_QUEUE_ITEM_LENGTH ( ( UBaseType_t ) 0 )
#define queueMUTEX_GIVE_BLOCK_TIME		 ( ( TickType_t ) 0U )

#if( configUSE_PREEMPTION == 0 )
	/* If the cooperative scheduler is being used then a yield should not be
	performed just because a higher priority task has been woken. */
	#define queueYIELD_IF_USING_PREEMPTION()
#else
	#define queueYIELD_IF_USING_PREEMPTION() portYIELD_WITHIN_API()
#endif

/*
 * Definition of the queue used by the scheduler.
 * Items are queued by copy, not reference.  See the following link for the
 * rationale: http://www.freertos.org/Embedded-RTOS-Queues.html
 */
typedef struct QueueDefinition
{
	int8_t *pcHead;					/*< Points to the beginning of the queue storage area. */
	int8_t *pcTail;					/*< Points to the byte at the end of the queue storage area.  Once more byte is allocated than necessary to store the queue items, this is used as a marker. */
	int8_t *pcWriteTo;				/*< Points to the free next place in the storage area. */

	union							/* Use of a union is an exception to the coding standard to ensure two mutually exclusive structure members don't appear simultaneously (wasting RAM). */
	{
		int8_t *pcReadFrom;			/*< Points to the last place that a queued item was read from when the structure is used as a queue. */
		UBaseType_t uxRecursiveCallCount;/*< Maintains a count of the number of times a recursive mutex has been recursively 'taken' when the structure is used as a mutex. */
	} u;

	List_t xTasksWaitingToSend;		/*< List of tasks that are blocked waiting to post onto this queue.  Stored in priority order. */
	List_t xTasksWaitingToReceive;	/*< List of tasks that are blocked waiting to read from this queue.  Stored in priority order. */

	volatile UBaseType_t uxMessagesWaiting;/*< The number of items currently in the queue. */
	UBaseType_t uxLength;			/*< The length of the queue defined as the number of items it will hold, not the number of bytes. */
	UBaseType_t uxItemSize;			/*< The size of each items that the queue will hold. */

	volatile int8_t cRxLock;		/*< Stores the number of items received from the queue (removed from the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */
	volatile int8_t cTxLock;		/*< Stores the number of items transmitted to the queue (added to the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */

	#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
		uint8_t ucStaticallyAllocated;	/*< Set to pdTRUE if the memory used by the queue was statically allocated to ensure no attempt is made to free the memory. */
	#endif

	#if ( configUSE_QUEUE_SETS == 1 )
		struct QueueDefinition *pxQueueSetContainer;
	#endif

	#if ( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t uxQueueNumber;
		uint8_t ucQueueType;
	#endif

} xQUEUE;

/* The old xQUEUE name is maintained above then typedefed to the new Queue_t
name below to enable the use of older kernel aware debuggers. */
typedef xQUEUE Queue_t;

/*-----------------------------------------------------------*/

/*
 * The queue registry is just a means for kernel aware debuggers to locate
 * queue structures.  It has no other purpose so is an optional component.
 */
#if ( configQUEUE_REGISTRY_SIZE > 0 )

	/* The type stored within the queue registry array.  This allows a name
	to be assigned to each queue making kernel aware debugging a little
	more user friendly. */
	typedef struct QUEUE_REGISTRY_ITEM
	{
		const char *pcQueueName; /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
		QueueHandle_t xHandle;
	} xQueueRegistryItem;

	/* The old xQueueRegistryItem name is maintained above then typedefed to the
	new xQueueRegistryItem name below to enable the use of older kernel aware
	debuggers. */
	typedef xQueueRegistryItem QueueRegistryItem_t;

	/* The queue registry is simply an array of QueueRegistryItem_t structures.
	The pcQueueName member of a structure being NULL is indicative of the
	array position being vacant. */
	PRIVILEGED_DATA QueueRegistryItem_t xQueueRegistry[ configQUEUE_REGISTRY_SIZE ];

#endif /* configQUEUE_REGISTRY_SIZE */

/*
 * Unlocks a queue locked by a call to prvLockQueue.  Locking a queue does not
 * prevent an ISR from adding or removing items to the queue, but does prevent
 * an ISR from removing tasks from the queue event lists.  If an ISR finds a
 * queue is locked it will instead increment the appropriate queue lock count
 * to indicate that a task may require unblocking.  When the queue in unlocked
 * these lock counts are inspected, and the appropriate action taken.
 */
static void prvUnlockQueue( Queue_t * const pxQueue ) PRIVILEGED_FUNCTION;

/*
 * Uses a critical section to determine if there is any data in a queue.
 *
 * @return pdTRUE if the queue contains no items, otherwise pdFALSE.
 */
static BaseType_t prvIsQueueEmpty( const Queue_t *pxQueue ) PRIVILEGED_FUNCTION;

/*
 * Uses a critical section to determine if there is any space in a queue.
 *
 * @return pdTRUE if there is no space, otherwise pdFALSE;
 */
static BaseType_t prvIsQueueFull( const Queue_t *pxQueue ) PRIVILEGED_FUNCTION;

/*
 * Copies an item into the queue, either at the front of the queue or the
 * back of the queue.
 */
static BaseType_t prvCopyDataToQueue( Queue_t * const pxQueue, const void *pvItemToQueue, const BaseType_t xPosition ) PRIVILEGED_FUNCTION;

/*
 * Copies an item out of a queue.
 */
static void prvCopyDataFromQueue( Queue_t * const pxQueue, void * const pvBuffer ) PRIVILEGED_FUNCTION;

#if ( configUSE_QUEUE_SETS == 1 )
	/*
	 * Checks to see if a queue is a member of a queue set, and if so, notifies
	 * the queue set that the queue contains data.
	 */
	static BaseType_t prvNotifyQueueSetContainer( const Queue_t * const pxQueue, const BaseType_t xCopyPosition ) PRIVILEGED_FUNCTION;
#endif

/*
 * Called after a Queue_t structure has been allocated either statically or
 * dynamically to fill in the structure's members.
 */
static void prvInitialiseNewQueue( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, uint8_t *pucQueueStorage, const uint8_t ucQueueType, Queue_t *pxNewQueue ) PRIVILEGED_FUNCTION;

/*
 * Mutexes are a special type of queue.  When a mutex is created, first the
 * queue is created, then prvInitialiseMutex() is called to configure the queue
 * as a mutex.
 */
#if( configUSE_MUTEXES == 1 )
	static void prvInitialiseMutex( Queue_t *pxNewQueue ) PRIVILEGED_FUNCTION;
#endif

#if( configUSE_MUTEXES == 1 )
	/*
	 * If a task waiting for a mutex causes the mutex holder to inherit a
	 * priority, but the waiting task times out, then the holder should
	 * disinherit the priority - but only down to the highest priority of any
	 * other tasks that are waiting for the same mutex.  This function returns
	 * that priority.
	 */
	static UBaseType_t prvGetDisinheritPriorityAfterTimeout( const Queue_t * const pxQueue ) PRIVILEGED_FUNCTION;
#endif
/*-----------------------------------------------------------*/

/*
 * Macro to mark a queue as locked.  Locking a queue prevents an ISR from
 * accessing the queue event lists.
 */
#define prvLockQueue( pxQueue )								\
	taskENTER_CRITICAL();									\
	{														\
		if( ( pxQueue )->cRxLock == queueUNLOCKED )			\
		{													\
			( pxQueue )->cRxLock = queueLOCKED_UNMODIFIED;	\
		}													\
		if( ( pxQueue )->cTxLock == queueUNLOCKED )			\
		{													\
			( pxQueue )->cTxLock = queueLOCKED_UNMODIFIED;	\
		}													\
	}														\
	taskEXIT_CRITICAL()
/*-----------------------------------------------------------*/

BaseType_t xQueueGenericReset( QueueHandle_t xQueue, BaseType_t xNewQueue )
{
Queue_t * const pxQueue = ( Queue_t * ) xQueue;

	configASSERT( pxQueue );

	taskENTER_CRITICAL();
	{
		pxQueue->pcTail = pxQueue->pcHead + ( pxQueue->uxLength * pxQueue->uxItemSize );
		pxQueue->uxMessagesWaiting = ( UBaseType_t ) 0U;
		pxQueue->pcWriteTo = pxQueue->pcHead;
		pxQueue->u.pcReadFrom = pxQueue->pcHead + ( ( pxQueue->uxLength - ( UBaseType_t ) 1U ) * pxQueue->uxItemSize );
		pxQueue->cRxLock = queueUNLOCKED;
		pxQueue->cTxLock = queueUNLOCKED;

		if( xNewQueue == pdFALSE )
		{
			/* If there are tasks blocked waiting to read from the queue, then
			the tasks will remain blocked as after this function exits the queue
			will still be empty.  If there are tasks blocked waiting to write to
			the queue, then one should be unblocked as after this function exits
			it will be possible to write to it. */
			if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) == pdFALSE )
			{
				if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToSend ) ) != pdFALSE )
				{
					queueYIELD_IF_USING_PREEMPTION();
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
		}
		else
		{
			/* Ensure the event queues start in the correct state. */
			vListInitialise( &( pxQueue->xTasksWaitingToSend ) );
			vListInitialise( &( pxQueue->xTasksWaitingToReceive ) );
		}
	}
	taskEXIT_CRITICAL();

	/* A value is returned for calling semantic consistency with previous
	versions. */
	return pdPASS;
}
/*-----------------------------------------------------------*/

#if( configSUPPORT_STATIC_ALLOCATION == 1 )

	QueueHandle_t xQueueGenericCreateStatic( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, uint8_t *pucQueueStorage, StaticQueue_t *pxStaticQueue, const uint8_t ucQueueType )
	{
	Queue_t *pxNewQueue;

		configASSERT( uxQueueLength > ( UBaseType_t ) 0 );

		/* The StaticQueue_t structure and the queue storage area must be
		supplied. */
		configASSERT( pxStaticQueue != NULL );

		/* A queue storage area should be provided if the item size is not 0, and
		should not be provided if the item size is 0. */
		configASSERT( !( ( pucQueueStorage != NULL ) && ( uxItemSize == 0 ) ) );
		configASSERT( !( ( pucQueueStorage == NULL ) && ( uxItemSize != 0 ) ) );

		#if( configASSERT_DEFINED == 1 )
		{
			/* Sanity check that the size of the structure used to declare a
			variable of type StaticQueue_t or StaticSemaphore_t equals the size of
			the real queue and semaphore structures. */
			volatile size_t xSize = sizeof( StaticQueue_t );
			configASSERT( xSize == sizeof( Queue_t ) );
		}
		#endif /* configASSERT_DEFINED */

		/* The address of a statically allocated queue was passed in, use it.
		The address of a statically allocated storage area was also passed in
		but is already set. */
		pxNewQueue = ( Queue_t * ) pxStaticQueue; /*lint !e740 Unusual cast is ok as the structures are designed to have the same alignment, and the size is checked by an assert. */

		if( pxNewQueue != NULL )
		{
			#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
			{
				/* Queues can be allocated wither statically or dynamically, so
				note this queue was allocated statically in case the queue is
				later deleted. */
				pxNewQueue->ucStaticallyAllocated = pdTRUE;
			}
			#endif /* configSUPPORT_DYNAMIC_ALLOCATION */

			prvInitialiseNewQueue( uxQueueLength, uxItemSize, pucQueueStorage, ucQueueType, pxNewQueue );
		}
		else
		{
			traceQUEUE_CREATE_FAILED( ucQueueType );
		}

		return pxNewQueue;
	}

#endif /* configSUPPORT_STATIC_ALLOCATION */
/*-----------------------------------------------------------*/

#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )

	QueueHandle_t xQueueGenericCreate( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, const uint8_t ucQueueType )
	{
	Queue_t *pxNewQueue;
	size_t xQueueSizeInBytes;
	uint8_t *pucQueueStorage;

		configASSERT( uxQueueLength > ( UBaseType_t ) 0 );

		if( uxItemSize == ( UBaseType_t ) 0 )
		{
			/* There is not going to be a queue storage area. */
			xQueueSizeInBytes = ( size_t ) 0;
		}
		else
		{
			/* Allocate enough space to hold the maximum number of items that
			can be in the queue at any time. */
			xQueueSizeInBytes = ( size_t ) ( uxQueueLength * uxItemSize ); /*lint !e961 MISRA exception as the casts are only redundant for some ports. */
		}

		pxNewQueue = ( Queue_t * ) pvPortMalloc( sizeof( Queue_t ) + xQueueSizeInBytes );

		if( pxNewQueue != NULL )
		{
			/* Jump past the queue structure to find the location of the queue
			storage area. */
			pucQueueStorage = ( ( uint8_t * ) pxNewQueue ) + sizeof( Queue_t );

			#if( configSUPPORT_STATIC_ALLOCATION == 1 )
			{
				/* Queues can be created either statically or dynamically, so
				note this task was created dynamically in case it is later
				deleted. */
				pxNewQueue->ucStaticallyAllocated = pdFALSE;
			}
			#endif /* configSUPPORT_STATIC_ALLOCATION */

			prvInitialiseNewQueue( uxQueueLength, uxItemSize, pucQueueStorage, ucQueueType, pxNewQueue );
		}
		else
		{
			traceQUEUE_CREATE_FAILED( ucQueueType );
		}

		return pxNewQueue;
	}

#endif /* configSUPPORT_STATIC_ALLOCATION */
/*-----------------------------------------------------------*/

static void prvInitialiseNewQueue( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, uint8_t *pucQueueStorage, const uint8_t ucQueueType, Queue_t *pxNewQueue )
{
	/* Remove compiler warnings about unused parameters should
	configUSE_TRACE_FACILITY not be set to 1. */
	( void ) ucQueueType;

	if( uxItemSize == ( UBaseType_t ) 0 )
	{
		/* No RAM was allocated for the queue storage area, but PC head cannot
		be set to NULL because NULL is used as a key to say the queue is used as
		a mutex.  Therefore just set pcHead to point to the queue as a benign
		value that is known to be within the memory map. */
		pxNewQueue->pcHead = ( int8_t * ) pxNewQueue;
	}
	else
	{
		/* Set the head to the start of the queue storage area. */
		pxNewQueue->pcHead = ( int8_t * ) pucQueueStorage;
	}

	/* Initialise the queue members as described where the queue type is
	defined. */
	pxNewQueue->uxLength = uxQueueLength;
	pxNewQueue->uxItemSize = uxItemSize;
	( void ) xQueueGenericReset( pxNewQueue, pdTRUE );

	#if ( configUSE_TRACE_FACILITY == 1 )
	{
		pxNewQueue->ucQueueType = ucQueueType;
	}
	#endif /* configUSE_TRACE_FACILITY */

	#if( configUSE_QUEUE_SETS == 1 )
	{
		pxNewQueue->pxQueueSetContainer = NULL;
	}
	#endif /* configUSE_QUEUE_SETS */

	traceQUEUE_CREATE( pxNewQueue );
}
/*-----------------------------------------------------------*/

#if( configUSE_MUTEXES == 1 )

	static void prvInitialiseMutex( Queue_t *pxNewQueue )
	{
		if( pxNewQueue != NULL )
		{
			/* The queue create function will set all the queue structure members
			correctly for a generic queue, but this function is creating a
			mutex.  Overwrite those members that need to be set differently -
			in particular the information required for priority inheritance. */
			pxNewQueue->pxMutexHolder = NULL;
			pxNewQueue->uxQueueType = queueQUEUE_IS_MUTEX;

			/* In case this is a recursive mutex. */
			pxNewQueue->u.uxRecursiveCallCount = 0;

			traceCREATE_MUTEX( pxNewQueue );

			/* Start with the semaphore in the expected state. */
			( void ) xQueueGenericSend( pxNewQueue, NULL, ( TickType_t ) 0U, queueSEND_TO_BACK );
		}
		else
		{
			traceCREATE_MUTEX_FAILED();
		}
	}

#endif /* configUSE_MUTEXES */
/*-----------------------------------------------------------*/

#if( ( configUSE_MUTEXES == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )

	QueueHandle_t xQueueCreateMutex( const uint8_t ucQueueType )
	{
	Queue_t *pxNewQueue;
	const UBaseType_t uxMutexLength = ( UBaseType_t ) 1, uxMutexSize = ( UBaseType_t ) 0;

		pxNewQueue = ( Queue_t * ) xQueueGenericCreate( uxMutexLength, uxMutexSize, ucQueueType );
		prvInitialiseMutex( pxNewQueue );

		return pxNewQueue;
	}

#endif /* configUSE_MUTEXES */
/*-----------------------------------------------------------*/

#if( ( configUSE_MUTEXES == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 1 ) )

	QueueHandle_t xQueueCreateMutexStatic( const uint8_t ucQueueType, StaticQueue_t *pxStaticQueue )
	{
	Queue_t *pxNewQueue;
	const UBaseType_t uxMutexLength = ( UBaseType_t ) 1, uxMutexSize = ( UBaseType_t ) 0;

		/* Prevent compiler warnings about unused parameters if
		configUSE_TRACE_FACILITY does not equal 1. */
		( void ) ucQueueType;

		pxNewQueue = ( Queue_t * ) xQueueGenericCreateStatic( uxMutexLength, uxMutexSize, NULL, pxStaticQueue, ucQueueType );
		prvInitialiseMutex( pxNewQueue );

		return pxNewQueue;
	}

#endif /* configUSE_MUTEXES */
/*-----------------------------------------------------------*/

#if ( ( configUSE_MUTEXES == 1 ) && ( INCLUDE_xSemaphoreGetMutexHolder == 1 ) )

	void* xQueueGetMutexHolder( QueueHandle_t xSemaphore )
	{
	void *pxReturn;

		/* This function is called by xSemaphoreGetMutexHolder(), and should not
		be called directly.  Note:  This is a good way of determining if the
		calling task is the mutex holder, but not a good way of determining the
		identity of the mutex holder, as the holder may change between the
		following critical section exiting and the function returning. */
		taskENTER_CRITICAL();
		{
			if( ( ( Queue_t * ) xSemaphore )->uxQueueType == queueQUEUE_IS_MUTEX )
			{
				pxReturn = ( void * ) ( ( Queue_t * ) xSemaphore )->pxMutexHolder;
			}
			else
			{
				pxReturn = NULL;
			}
		}
		taskEXIT_CRITICAL();

		return pxReturn;
	} /*lint !e818 xSemaphore cannot be a pointer to const because it is a typedef. */

#endif
/*-----------------------------------------------------------*/

#if ( ( configUSE_MUTEXES == 1 ) && ( INCLUDE_xSemaphoreGetMutexHolder == 1 ) )

	void* xQueueGetMutexHolderFromISR( QueueHandle_t xSemaphore )
	{
	void *pxReturn;

		configASSERT( xSemaphore );

		/* Mutexes cannot be used in interrupt service routines, so the mutex
		holder should not change in an ISR, and therefore a critical section is
		not required here. */
		if( ( ( Queue_t * ) xSemaphore )->uxQueueType == queueQUEUE_IS_MUTEX )
		{
			pxReturn = ( void * ) ( ( Queue_t * ) xSemaphore )->pxMutexHolder;
		}
		else
		{
			pxReturn = NULL;
		}

		return pxReturn;
	} /*lint !e818 xSemaphore cannot be a pointer to const because it is a typedef. */

#endif
/*-----------------------------------------------------------*/

#if ( configUSE_RECURSIVE_MUTEXES == 1 )

	BaseType_t xQueueGiveMutexRecursive( QueueHandle_t xMutex )
	{
	BaseType_t xReturn;
	Queue_t * const pxMutex = ( Queue_t * ) xMutex;

		configASSERT( pxMutex );

		/* If this is the task that holds the mutex then pxMutexHolder will not
		change outside of this task.  If this task does not hold the mutex then
		pxMutexHolder can never coincidentally equal the tasks handle, and as
		this is the only condition we are interested in it does not matter if
		pxMutexHolder is accessed simultaneously by another task.  Therefore no
		mutual exclusion is required to test the pxMutexHolder variable. */
		if( pxMutex->pxMutexHolder == ( void * ) xTaskGetCurrentTaskHandle() ) /*lint !e961 Not a redundant cast as TaskHandle_t is a typedef. */
		{
			traceGIVE_MUTEX_RECURSIVE( pxMutex );

			/* uxRecursiveCallCount cannot be zero if pxMutexHolder is equal to
			the task handle, therefore no underflow check is required.  Also,
			uxRecursiveCallCount is only modified by the mutex holder, and as
			there can only be one, no mutual exclusion is required to modify the
			uxRecursiveCallCount member. */
			( pxMutex->u.uxRecursiveCallCount )--;

			/* Has the recursive call count unwound to 0? */
			if( pxMutex->u.uxRecursiveCallCount == ( UBaseType_t ) 0 )
			{
				/* Return the mutex.  This will automatically unblock any other
				task that might be waiting to access the mutex. */
				( void ) xQueueGenericSend( pxMutex, NULL, queueMUTEX_GIVE_BLOCK_TIME, queueSEND_TO_BACK );
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			xReturn = pdPASS;
		}
		else
		{
			/* The mutex cannot be given because the calling task is not the
			holder. */
			xReturn = pdFAIL;

			traceGIVE_MUTEX_RECURSIVE_FAILED( pxMutex );
		}

		return xReturn;
	}

#endif /* configUSE_RECURSIVE_MUTEXES */
/*-----------------------------------------------------------*/

#if ( configUSE_RECURSIVE_MUTEXES == 1 )

	BaseType_t xQueueTakeMutexRecursive( QueueHandle_t xMutex, TickType_t xTicksToWait )
	{
	BaseType_t xReturn;
	Queue_t * const pxMutex = ( Queue_t * ) xMutex;

		configASSERT( pxMutex );

		/* Comments regarding mutual exclusion as per those within
		xQueueGiveMutexRecursive(). */

		traceTAKE_MUTEX_RECURSIVE( pxMutex );

		if( pxMutex->pxMutexHolder == ( void * ) xTaskGetCurrentTaskHandle() ) /*lint !e961 Cast is not redundant as TaskHandle_t is a typedef. */
		{
			( pxMutex->u.uxRecursiveCallCount )++;
			xReturn = pdPASS;
		}
		else
		{
			xReturn = xQueueSemaphoreTake( pxMutex, xTicksToWait );

			/* pdPASS will only be returned if the mutex was successfully
			obtained.  The calling task may have entered the Blocked state
			before reaching here. */
			if( xReturn != pdFAIL )
			{
				( pxMutex->u.uxRecursiveCallCount )++;
			}
			else
			{
				traceTAKE_MUTEX_RECURSIVE_FAILED( pxMutex );
			}
		}

		return xReturn;
	}

#endif /* configUSE_RECURSIVE_MUTEXES */
/*-----------------------------------------------------------*/

#if( ( configUSE_COUNTING_SEMAPHORES == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 1 ) )

	QueueHandle_t xQueueCreateCountingSemaphoreStatic( const UBaseType_t uxMaxCount, const UBaseType_t uxInitialCount, StaticQueue_t *pxStaticQueue )
	{
	QueueHandle_t xHandle;

		configASSERT( uxMaxCount != 0 );
		configASSERT( uxInitialCount <= uxMaxCount );

		xHandle = xQueueGenericCreateStatic( uxMaxCount, queueSEMAPHORE_QUEUE_ITEM_LENGTH, NULL, pxStaticQueue, queueQUEUE_TYPE_COUNTING_SEMAPHORE );

		if( xHandle != NULL )
		{
			( ( Queue_t * ) xHandle )->uxMessagesWaiting = uxInitialCount;

			traceCREATE_COUNTING_SEMAPHORE();
		}
		else
		{
			traceCREATE_COUNTING_SEMAPHORE_FAILED();
		}

		return xHandle;
	}

#endif /* ( ( configUSE_COUNTING_SEMAPHORES == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) ) */
/*-----------------------------------------------------------*/

#if( ( configUSE_COUNTING_SEMAPHORES == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )

	QueueHandle_t xQueueCreateCountingSemaphore( const UBaseType_t uxMaxCount, const UBaseType_t uxInitialCount )
	{
	QueueHandle_t xHandle;

		configASSERT( uxMaxCount != 0 );
		configASSERT( uxInitialCount <= uxMaxCount );

		xHandle = xQueueGenericCreate( uxMaxCount, queueSEMAPHORE_QUEUE_ITEM_LENGTH, queueQUEUE_TYPE_COUNTING_SEMAPHORE );

		if( xHandle != NULL )
		{
			( ( Queue_t * ) xHandle )->uxMessagesWaiting = uxInitialCount;

			traceCREATE_COUNTING_SEMAPHORE();
		}
		else
		{
			traceCREATE_COUNTING_SEMAPHORE_FAILED();
		}

		return xHandle;
	}

#endif /* ( ( configUSE_COUNTING_SEMAPHORES == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) ) */
/*-----------------------------------------------------------*/

BaseType_t xQueueGenericSend( QueueHandle_t xQueue, const void * const pvItemToQueue, TickType_t xTicksToWait, const BaseType_t xCopyPosition )
{
BaseType_t xEntryTimeSet = pdFALSE, xYieldRequired;
TimeOut_t xTimeOut;
Queue_t * const pxQueue = ( Queue_t * ) xQueue;

	configASSERT( pxQueue );
	configASSERT( !( ( pvItemToQueue == NULL ) && ( pxQueue->uxItemSize != ( UBaseType_t ) 0U ) ) );
	configASSERT( !( ( xCopyPosition == queueOVERWRITE ) && ( pxQueue->uxLength != 1 ) ) );
	#if ( ( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ) )
	{
		configASSERT( !( ( xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED ) && ( xTicksToWait != 0 ) ) );
	}
	#endif


	/* This function relaxes the coding standard somewhat to allow return
	statements within the function itself.  This is done in the interest
	of execution time efficiency. */
	for( ;; )
	{
		taskENTER_CRITICAL();
		{
			/* Is there room on the queue now?  The running task must be the
			highest priority task wanting to access the queue.  If the head item
			in the queue is to be overwritten then it does not matter if the
			queue is full. */
			if( ( pxQueue->uxMessagesWaiting < pxQueue->uxLength ) || ( xCopyPosition == queueOVERWRITE ) )
			{
				traceQUEUE_SEND( pxQueue );
				xYieldRequired = prvCopyDataToQueue( pxQueue, pvItemToQueue, xCopyPosition );

				#if ( configUSE_QUEUE_SETS == 1 )
				{
					if( pxQueue->pxQueueSetContainer != NULL )
					{
						if( prvNotifyQueueSetContainer( pxQueue, xCopyPosition ) != pdFALSE )
						{
							/* The queue is a member of a queue set, and posting
							to the queue set caused a higher priority task to
							unblock. A context switch is required. */
							queueYIELD_IF_USING_PREEMPTION();
						}
						else
						{
							mtCOVERAGE_TEST_MARKER();
						}
					}
					else
					{
						/* If there was a task waiting for data to arrive on the
						queue then unblock it now. */
						if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
						{
							if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
							{
								/* The unblocked task has a priority higher than
								our own so yield immediately.  Yes it is ok to
								do this from within the critical section - the
								kernel takes care of that. */
								queueYIELD_IF_USING_PREEMPTION();
							}
							else
							{
								mtCOVERAGE_TEST_MARKER();
							}
						}
						else if( xYieldRequired != pdFALSE )
						{
							/* This path is a special case that will only get
							executed if the task was holding multiple mutexes
							and the mutexes were given back in an order that is
							different to that in which they were taken. */
							queueYIELD_IF_USING_PREEMPTION();
						}
						else
						{
							mtCOVERAGE_TEST_MARKER();
						}
					}
				}
				#else /* configUSE_QUEUE_SETS */
				{
					/* If there was a task waiting for data to arrive on the
					queue then unblock it now. */
					if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
					{
						if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
						{
							/* The unblocked task has a priority higher than
							our own so yield immediately.  Yes it is ok to do
							this from within the critical section - the kernel
							takes care of that. */
							queueYIELD_IF_USING_PREEMPTION();
						}
						else
						{
							mtCOVERAGE_TEST_MARKER();
						}
					}
					else if( xYieldRequired != pdFALSE )
					{
						/* This path is a special case that will only get
						executed if the task was holding multiple mutexes and
						the mutexes were given back in an order that is
						different to that in which they were taken. */
						queueYIELD_IF_USING_PREEMPTION();
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}
				}
				#endif /* configUSE_QUEUE_SETS */

				taskEXIT_CRITICAL();
				return pdPASS;
			}
			else
			{
				if( xTicksToWait == ( TickType_t ) 0 )
				{
					/* The queue was full and no block time is specified (or
					the block time has expired) so leave now. */
					taskEXIT_CRITICAL();

					/* Return to the original privilege level before exiting
					the function. */
					traceQUEUE_SEND_FAILED( pxQueue );
					return errQUEUE_FULL;
				}
				else if( xEntryTimeSet == pdFALSE )
				{
					/* The queue was full and a block time was specified so
					configure the timeout structure. */
					vTaskInternalSetTimeOutState( &xTimeOut );
					xEntryTimeSet = pdTRUE;
				}
				else
				{
					/* Entry time was already set. */
					mtCOVERAGE_TEST_MARKER();
				}
			}
		}
		taskEXIT_CRITICAL();

		/* Interrupts and other tasks can send to and receive from the queue
		now the critical section has been exited. */

		vTaskSuspendAll();
		prvLockQueue( pxQueue );

		/* Update the timeout state to see if it has expired yet. */
		if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) == pdFALSE )
		{
			if( prvIsQueueFull( pxQueue ) != pdFALSE )
			{
				traceBLOCKING_ON_QUEUE_SEND( pxQueue );
				vTaskPlaceOnEventList( &( pxQueue->xTasksWaitingToSend ), xTicksToWait );

				/* Unlocking the queue means queue events can effect the
				event list.  It is possible that interrupts occurring now
				remove this task from the event list again - but as the
				scheduler is suspended the task will go onto the pending
				ready last instead of the actual ready list. */
				prvUnlockQueue( pxQueue );

				/* Resuming the scheduler will move tasks from the pending
				ready list into the ready list - so it is feasible that this
				task is already in a ready list before it yields - in which
				case the yield will not cause a context switch unless there
				is also a higher priority task in the pending ready list. */
				if( xTaskResumeAll() == pdFALSE )
				{
					portYIELD_WITHIN_API();
				}
			}
			else
			{
				/* Try again. */
				prvUnlockQueue( pxQueue );
				( void ) xTaskResumeAll();
			}
		}
		else
		{
			/* The timeout has expired. */
			prvUnlockQueue( pxQueue );
			( void ) xTaskResumeAll();

			traceQUEUE_SEND_FAILED( pxQueue );
			return errQUEUE_FULL;
		}
	}
}
/*-----------------------------------------------------------*/

BaseType_t xQueueGenericSendFromISR( QueueHandle_t xQueue, const void * const pvItemToQueue, BaseType_t * const pxHigherPriorityTaskWoken, const BaseType_t xCopyPosition )
{
BaseType_t xReturn;
UBaseType_t uxSavedInterruptStatus;
Queue_t * const pxQueue = ( Queue_t * ) xQueue;

	configASSERT( pxQueue );
	configASSERT( !( ( pvItemToQueue == NULL ) && ( pxQueue->uxItemSize != ( UBaseType_t ) 0U ) ) );
	configASSERT( !( ( xCopyPosition == queueOVERWRITE ) && ( pxQueue->uxLength != 1 ) ) );

	/* RTOS ports that support interrupt nesting have the concept of a maximum
	system call (or maximum API call) interrupt priority.  Interrupts that are
	above the maximum system call priority are kept permanently enabled, even
	when the RTOS kernel is in a critical section, but cannot make any calls to
	FreeRTOS API functions.  If configASSERT() is defined in FreeRTOSConfig.h
	then portASSERT_IF_INTERRUPT_PRIORITY_INVALID() will result in an assertion
	failure if a FreeRTOS API function is called from an interrupt that has been
	assigned a priority above the configured maximum system call priority.
	Only FreeRTOS functions that end in FromISR can be called from interrupts
	that have been assigned a priority at or (logically) below the maximum
	system call	interrupt priority.  FreeRTOS maintains a separate interrupt
	safe API to ensure interrupt entry is as fast and as simple as possible.
	More information (albeit Cortex-M specific) is provided on the following
	link: http://www.freertos.org/RTOS-Cortex-M3-M4.html */
	portASSERT_IF_INTERRUPT_PRIORITY_INVALID();

	/* Similar to xQueueGenericSend, except without blocking if there is no room
	in the queue.  Also don't directly wake a task that was blocked on a queue
	read, instead return a flag to say whether a context switch is required or
	not (i.e. has a task with a higher priority than us been woken by this
	post). */
	uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
	{
		if( ( pxQueue->uxMessagesWaiting < pxQueue->uxLength ) || ( xCopyPosition == queueOVERWRITE ) )
		{
			const int8_t cTxLock = pxQueue->cTxLock;

			traceQUEUE_SEND_FROM_ISR( pxQueue );

			/* Semaphores use xQueueGiveFromISR(), so pxQueue will not be a
			semaphore or mutex.  That means prvCopyDataToQueue() cannot result
			in a task disinheriting a priority and prvCopyDataToQueue() can be
			called here even though the disinherit function does not check if
			the scheduler is suspended before accessing the ready lists. */
			( void ) prvCopyDataToQueue( pxQueue, pvItemToQueue, xCopyPosition );

			/* The event list is not altered if the queue is locked.  This will
			be done when the queue is unlocked later. */
			if( cTxLock == queueUNLOCKED )
			{
				#if ( configUSE_QUEUE_SETS == 1 )
				{
					if( pxQueue->pxQueueSetContainer != NULL )
					{
						if( prvNotifyQueueSetContainer( pxQueue, xCopyPosition ) != pdFALSE )
						{
							/* The queue is a member of a queue set, and posting
							to the queue set caused a higher priority task to
							unblock.  A context switch is required. */
							if( pxHigherPriorityTaskWoken != NULL )
							{
								*pxHigherPriorityTaskWoken = pdTRUE;
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
					}
					else
					{
						if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
						{
							if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
							{
								/* The task waiting has a higher priority so
								record that a context switch is required. */
								if( pxHigherPriorityTaskWoken != NULL )
								{
									*pxHigherPriorityTaskWoken = pdTRUE;
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
						}
						else
						{
							mtCOVERAGE_TEST_MARKER();
						}
					}
				}
				#else /* configUSE_QUEUE_SETS */
				{
					if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
					{
						if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
						{
							/* The task waiting has a higher priority so record that a
							context	switch is required. */
							if( pxHigherPriorityTaskWoken != NULL )
							{
								*pxHigherPriorityTaskWoken = pdTRUE;
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
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}
				}
				#endif /* configUSE_QUEUE_SETS */
			}
			else
			{
				/* Increment the lock count so the task that unlocks the queue
				knows that data was posted while it was locked. */
				pxQueue->cTxLock = ( int8_t ) ( cTxLock + 1 );
			}

			xReturn = pdPASS;
		}
		else
		{
			traceQUEUE_SEND_FROM_ISR_FAILED( pxQueue );
			xReturn = errQUEUE_FULL;
		}
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

	return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xQueueGiveFromISR( QueueHandle_t xQueue, BaseType_t * const pxHigherPriorityTaskWoken )
{
BaseType_t xReturn;
UBaseType_t uxSavedInterruptStatus;
Queue_t * const pxQueue = ( Queue_t * ) xQueue;

	/* Similar to xQueueGenericSendFromISR() but used with semaphores where the
	item size is 0.  Don't directly wake a task that was blocked on a queue
	read, instead return a flag to say whether a context switch is required or
	not (i.e. has a task with a higher priority than us been woken by this
	post). */

	configASSERT( pxQueue );

	/* xQueueGenericSendFromISR() should be used instead of xQueueGiveFromISR()
	if the item size is not 0. */
	configASSERT( pxQueue->uxItemSize == 0 );

	/* Normally a mutex would not be given from an interrupt, especially if
	there is a mutex holder, as priority inheritance makes no sense for an
	interrupts, only tasks. */
	configASSERT( !( ( pxQueue->uxQueueType == queueQUEUE_IS_MUTEX ) && ( pxQueue->pxMutexHolder != NULL ) ) );

	/* RTOS ports that support interrupt nesting have the concept of a maximum
	system call (or maximum API call) interrupt priority.  Interrupts that are
	above the maximum system call priority are kept permanently enabled, even
	when the RTOS kernel is in a critical section, but cannot make any calls to
	FreeRTOS API functions.  If configASSERT() is defined in FreeRTOSConfig.h
	then portASSERT_IF_INTERRUPT_PRIORITY_INVALID() will result in an assertion
	failure if a FreeRTOS API function is called from an interrupt that has been
	assigned a priority above the configured maximum system call priority.
	Only FreeRTOS functions that end in FromISR can be called from interrupts
	that have been assigned a priority at or (logically) below the maximum
	system call	interrupt priority.  FreeRTOS maintains a separate interrupt
	safe API to ensure interrupt entry is as fast and as simple as possible.
	More information (albeit Cortex-M specific) is provided on the following
	link: http://www.freertos.org/RTOS-Cortex-M3-M4.html */
	portASSERT_IF_INTERRUPT_PRIORITY_INVALID();

	uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
	{
		const UBaseType_t uxMessagesWaiting = pxQueue->uxMessagesWaiting;

		/* When the queue is used to implement a semaphore no data is ever
		moved through the queue but it is still valid to see if the queue 'has
		space'. */
		if( uxMessagesWaiting < pxQueue->uxLength )
		{
			const int8_t cTxLock = pxQueue->cTxLock;

			traceQUEUE_SEND_FROM_ISR( pxQueue );

			/* A task can only have an inherited priority if it is a mutex
			holder - and if there is a mutex holder then the mutex cannot be
			given from an ISR.  As this is the ISR version of the function it
			can be assumed there is no mutex holder and no need to determine if
			priority disinheritance is needed.  Simply increase the count of
			messages (semaphores) available. */
			pxQueue->uxMessagesWaiting = uxMessagesWaiting + ( UBaseType_t ) 1;

			/* The event list is not altered if the queue is locked.  This will
			be done when the queue is unlocked later. */
			if( cTxLock == queueUNLOCKED )
			{
				#if ( configUSE_QUEUE_SETS == 1 )
				{
					if( pxQueue->pxQueueSetContainer != NULL )
					{
						if( prvNotifyQueueSetContainer( pxQueue, queueSEND_TO_BACK ) != pdFALSE )
						{
							/* The semaphore is a member of a queue set, and
							posting	to the queue set caused a higher priority
							task to	unblock.  A context switch is required. */
							if( pxHigherPriorityTaskWoken != NULL )
							{
								*pxHigherPriorityTaskWoken = pdTRUE;
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
					}
					else
					{
						if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
						{
							if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
							{
								/* The task waiting has a higher priority so
								record that a context switch is required. */
								if( pxHigherPriorityTaskWoken != NULL )
								{
									*pxHigherPriorityTaskWoken = pdTRUE;
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
						}
						else
						{
							mtCOVERAGE_TEST_MARKER();
						}
					}
				}
				#else /* configUSE_QUEUE_SETS */
				{
					if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
					{
						if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
						{
							/* The task waiting has a higher priority so record that a
							context	switch is required. */
							if( pxHigherPriorityTaskWoken != NULL )
							{
								*pxHigherPriorityTaskWoken = pdTRUE;
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
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}
				}
				#endif /* configUSE_QUEUE_SETS */
			}
			else
			{
				/* Increment the lock count so the task that unlocks the queue
				knows that data was posted while it was locked. */
				pxQueue->cTxLock = ( int8_t ) ( cTxLock + 1 );
			}

			xReturn = pdPASS;
		}
		else
		{
			traceQUEUE_SEND_FROM_ISR_FAILED( pxQueue );
			xReturn = errQUEUE_FULL;
		}
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

	return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xQueueReceive( QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait )
{
BaseType_t xEntryTimeSet = pdFALSE;
TimeOut_t xTimeOut;
Queue_t * const pxQueue = ( Queue_t * ) xQueue;

	/* Check the pointer is not NULL. */
	configASSERT( ( pxQueue ) );

	/* The buffer into which data is received can only be NULL if the data size
	is zero (so no data is copied into the buffer. */
	configASSERT( !( ( ( pvBuffer ) == NULL ) && ( ( pxQueue )->uxItemSize != ( UBaseType_t ) 0U ) ) );

	/* Cannot block if the scheduler is suspended. */
	#if ( ( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ) )
	{
		configASSERT( !( ( xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED ) && ( xTicksToWait != 0 ) ) );
	}
	#endif

	/* This function relaxes the coding standard somewhat to allow return
	statements within the function itself.  This is done in the interest
	of execution time efficiency. */

	for( ;; )
	{
		taskENTER_CRITICAL();
		{
			const UBaseType_t uxMessagesWaiting = pxQueue->uxMessagesWaiting;

			/* Is there data in the queue now?  To be running the calling task
			must be the highest priority task wanting to access the queue. */
			if( uxMessagesWaiting > ( UBaseType_t ) 0 )
			{
				/* Data available, remove one item. */
				prvCopyDataFromQueue( pxQueue, pvBuffer );
				traceQUEUE_RECEIVE( pxQueue );
				pxQueue->uxMessagesWaiting = uxMessagesWaiting - ( UBaseType_t ) 1;

				/* There is now space in the queue, were any tasks waiting to
				post to the queue?  If so, unblock the highest priority waiting
				task. */
				if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) == pdFALSE )
				{
					if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToSend ) ) != pdFALSE )
					{
						queueYIELD_IF_USING_PREEMPTION();
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

				taskEXIT_CRITICAL();
				return pdPASS;
			}
			else
			{
				if( xTicksToWait == ( TickType_t ) 0 )
				{
					/* The queue was empty and no block time is specified (or
					the block time has expired) so leave now. */
					taskEXIT_CRITICAL();
					traceQUEUE_RECEIVE_FAILED( pxQueue );
					return errQUEUE_EMPTY;
				}
				else if( xEntryTimeSet == pdFALSE )
				{
					/* The queue was empty and a block time was specified so
					configure the timeout structure. */
					vTaskInternalSetTimeOutState( &xTimeOut );
					xEntryTimeSet = pdTRUE;
				}
				else
				{
					/* Entry time was already set. */
					mtCOVERAGE_TEST_MARKER();
				}
			}
		}
		taskEXIT_CRITICAL();

		/* Interrupts and other tasks can send to and receive from the queue
		now the critical section has been exited. */

		vTaskSuspendAll();
		prvLockQueue( pxQueue );

		/* Update the timeout state to see if it has expired yet. */
		if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) == pdFALSE )
		{
			/* The timeout has not expired.  If the queue is still empty place
			the task on the list of tasks waiting to receive from the queue. */
			if( prvIsQueueEmpty( pxQueue ) != pdFALSE )
			{
				traceBLOCKING_ON_QUEUE_RECEIVE( pxQueue );
				vTaskPlaceOnEventList( &( pxQueue->xTasksWaitingToReceive ), xTicksToWait );
				prvUnlockQueue( pxQueue );
				if( xTaskResumeAll() == pdFALSE )
				{
					portYIELD_WITHIN_API();
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				/* The queue contains data again.  Loop back to try and read the
				data. */
				prvUnlockQueue( pxQueue );
				( void ) xTaskResumeAll();
			}
		}
		else
		{
			/* Timed out.  If there is no data in the queue exit, otherwise loop
			back and attempt to read the data. */
			prvUnlockQueue( pxQueue );
			( void ) xTaskResumeAll();

			if( prvIsQueueEmpty( pxQueue ) != pdFALSE )
			{
				traceQUEUE_RECEIVE_FAILED( pxQueue );
				return errQUEUE_EMPTY;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
	}
}
/*-----------------------------------------------------------*/

BaseType_t xQueueSemaphoreTake( QueueHandle_t xQueue, TickType_t xTicksToWait )
{
BaseType_t xEntryTimeSet = pdFALSE;
TimeOut_t xTimeOut;
Queue_t * const pxQueue = ( Queue_t * ) xQueue;

#if( configUSE_MUTEXES == 1 )
	BaseType_t xInheritanceOccurred = pdFALSE;
#endif

	/* Check the queue pointer is not NULL. */
	configASSERT( ( pxQueue ) );

	/* Check this really is a semaphore, in which case the item size will be
	0. */
	configASSERT( pxQueue->uxItemSize == 0 );

	/* Cannot block if the scheduler is suspended. */
	#if ( ( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ) )
	{
		configASSERT( !( ( xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED ) && ( xTicksToWait != 0 ) ) );
	}
	#endif


	/* This function relaxes the coding standard somewhat to allow return
	statements within the function itself.  This is done in the interest
	of execution time efficiency. */

	for( ;; )
	{
		taskENTER_CRITICAL();
		{
			/* Semaphores are queues with an item size of 0, and where the
			number of messages in the queue is the semaphore's count value. */
			const UBaseType_t uxSemaphoreCount = pxQueue->uxMessagesWaiting;

			/* Is there data in the queue now?  To be running the calling task
			must be the highest priority task wanting to access the queue. */
			if( uxSemaphoreCount > ( UBaseType_t ) 0 )
			{
				traceQUEUE_RECEIVE( pxQueue );

				/* Semaphores are queues with a data size of zero and where the
				messages waiting is the semaphore's count.  Reduce the count. */
				pxQueue->uxMessagesWaiting = uxSemaphoreCount - ( UBaseType_t ) 1;

				#if ( configUSE_MUTEXES == 1 )
				{
					if( pxQueue->uxQueueType == queueQUEUE_IS_MUTEX )
					{
						/* Record the information required to implement
						priority inheritance should it become necessary. */
						pxQueue->pxMutexHolder = ( int8_t * ) pvTaskIncrementMutexHeldCount(); /*lint !e961 Cast is not redundant as TaskHandle_t is a typedef. */
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}
				}
				#endif /* configUSE_MUTEXES */

				/* Check to see if other tasks are blocked waiting to give the
				semaphore, and if so, unblock the highest priority such task. */
				if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) == pdFALSE )
				{
					if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToSend ) ) != pdFALSE )
					{
						queueYIELD_IF_USING_PREEMPTION();
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

				taskEXIT_CRITICAL();
				return pdPASS;
			}
			else
			{
				if( xTicksToWait == ( TickType_t ) 0 )
				{
					/* For inheritance to have occurred there must have been an
					initial timeout, and an adjusted timeout cannot become 0, as
					if it were 0 the function would have exited. */
					#if( configUSE_MUTEXES == 1 )
					{
						configASSERT( xInheritanceOccurred == pdFALSE );
					}
					#endif /* configUSE_MUTEXES */

					/* The semaphore count was 0 and no block time is specified
					(or the block time has expired) so exit now. */
					taskEXIT_CRITICAL();
					traceQUEUE_RECEIVE_FAILED( pxQueue );
					return errQUEUE_EMPTY;
				}
				else if( xEntryTimeSet == pdFALSE )
				{
					/* The semaphore count was 0 and a block time was specified
					so configure the timeout structure ready to block. */
					vTaskInternalSetTimeOutState( &xTimeOut );
					xEntryTimeSet = pdTRUE;
				}
				else
				{
					/* Entry time was already set. */
					mtCOVERAGE_TEST_MARKER();
				}
			}
		}
		taskEXIT_CRITICAL();

		/* Interrupts and other tasks can give to and take from the semaphore
		now the critical section has been exited. */

		vTaskSuspendAll();
		prvLockQueue( pxQueue );

		/* Update the timeout state to see if it has expired yet. */
		if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) == pdFALSE )
		{
			/* A block time is specified and not expired.  If the semaphore
			count is 0 then enter the Blocked state to wait for a semaphore to
			become available.  As semaphores are implemented with queues the
			queue being empty is equivalent to the semaphore count being 0. */
			if( prvIsQueueEmpty( pxQueue ) != pdFALSE )
			{
				traceBLOCKING_ON_QUEUE_RECEIVE( pxQueue );

				#if ( configUSE_MUTEXES == 1 )
				{
					if( pxQueue->uxQueueType == queueQUEUE_IS_MUTEX )
					{
						taskENTER_CRITICAL();
						{
							xInheritanceOccurred = xTaskPriorityInherit( ( void * ) pxQueue->pxMutexHolder );
						}
						taskEXIT_CRITICAL();
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}
				}
				#endif

				vTaskPlaceOnEventList( &( pxQueue->xTasksWaitingToReceive ), xTicksToWait );
				prvUnlockQueue( pxQueue );
				if( xTaskResumeAll() == pdFALSE )
				{
					portYIELD_WITHIN_API();
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				/* There was no timeout and the semaphore count was not 0, so
				attempt to take the semaphore again. */
				prvUnlockQueue( pxQueue );
				( void ) xTaskResumeAll();
			}
		}
		else
		{
			/* Timed out. */
			prvUnlockQueue( pxQueue );
			( void ) xTaskResumeAll();

			/* If the semaphore count is 0 exit now as the timeout has
			expired.  Otherwise return to attempt to take the semaphore that is
			known to be available.  As semaphores are implemented by queues the
			queue being empty is equivalent to the semaphore count being 0. */
			if( prvIsQueueEmpty( pxQueue ) != pdFALSE )
			{
				#if ( configUSE_MUTEXES == 1 )
				{
					/* xInheritanceOccurred could only have be set if
					pxQueue->uxQueueType == queueQUEUE_IS_MUTEX so no need to
					test the mutex type again to check it is actually a mutex. */
					if( xInheritanceOccurred != pdFALSE )
					{
						taskENTER_CRITICAL();
						{
							UBaseType_t uxHighestWaitingPriority;

							/* This task blocking on the mutex caused another
							task to inherit this task's priority.  Now this task
							has timed out the priority should be disinherited
							again, but only as low as the next highest priority
							task that is waiting for the same mutex. */
							uxHighestWaitingPriority = prvGetDisinheritPriorityAfterTimeout( pxQueue );
							vTaskPriorityDisinheritAfterTimeout( ( void * ) pxQueue->pxMutexHolder, uxHighestWaitingPriority );
						}
						taskEXIT_CRITICAL();
					}
				}
				#endif /* configUSE_MUTEXES */

				traceQUEUE_RECEIVE_FAILED( pxQueue );
				return errQUEUE_EMPTY;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
	}
}
/*-----------------------------------------------------------*/

BaseType_t xQueuePeek( QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait )
{
BaseType_t xEntryTimeSet = pdFALSE;
TimeOut_t xTimeOut;
int8_t *pcOriginalReadPosition;
Queue_t * const pxQueue = ( Queue_t * ) xQueue;

	/* Check the pointer is not NULL. */
	configASSERT( ( pxQueue ) );

	/* The buffer into which data is received can only be NULL if the data size
	is zero (so no data is copied into the buffer. */
	configASSERT( !( ( ( pvBuffer ) == NULL ) && ( ( pxQueue )->uxItemSize != ( UBaseType_t ) 0U ) ) );

	/* Cannot block if the scheduler is suspended. */
	#if ( ( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ) )
	{
		configASSERT( !( ( xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED ) && ( xTicksToWait != 0 ) ) );
	}
	#endif


	/* This function relaxes the coding standard somewhat to allow return
	statements within the function itself.  This is done in the interest
	of execution time efficiency. */

	for( ;; )
	{
		taskENTER_CRITICAL();
		{
			const UBaseType_t uxMessagesWaiting = pxQueue->uxMessagesWaiting;

			/* Is there data in the queue now?  To be running the calling task
			must be the highest priority task wanting to access the queue. */
			if( uxMessagesWaiting > ( UBaseType_t ) 0 )
			{
				/* Remember the read position so it can be reset after the data
				is read from the queue as this function is only peeking the
				data, not removing it. */
				pcOriginalReadPosition = pxQueue->u.pcReadFrom;

				prvCopyDataFromQueue( pxQueue, pvBuffer );
				traceQUEUE_PEEK( pxQueue );

				/* The data is not being removed, so reset the read pointer. */
				pxQueue->u.pcReadFrom = pcOriginalReadPosition;

				/* The data is being left in the queue, so see if there are
				any other tasks waiting for the data. */
				if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
				{
					if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
					{
						/* The task waiting has a higher priority than this task. */
						queueYIELD_IF_USING_PREEMPTION();
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

				taskEXIT_CRITICAL();
				return pdPASS;
			}
			else
			{
				if( xTicksToWait == ( TickType_t ) 0 )
				{
					/* The queue was empty and no block time is specified (or
					the block time has expired) so leave now. */
					taskEXIT_CRITICAL();
					traceQUEUE_PEEK_FAILED( pxQueue );
					return errQUEUE_EMPTY;
				}
				else if( xEntryTimeSet == pdFALSE )
				{
					/* The queue was empty and a block time was specified so
					configure the timeout structure ready to enter the blocked
					state. */
					vTaskInternalSetTimeOutState( &xTimeOut );
					xEntryTimeSet = pdTRUE;
				}
				else
				{
					/* Entry time was already set. */
					mtCOVERAGE_TEST_MARKER();
				}
			}
		}
		taskEXIT_CRITICAL();

		/* Interrupts and other tasks can send to and receive from the queue
		now the critical section has been exited. */

		vTaskSuspendAll();
		prvLockQueue( pxQueue );

		/* Update the timeout state to see if it has expired yet. */
		if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) == pdFALSE )
		{
			/* Timeout has not expired yet, check to see if there is data in the
			queue now, and if not enter the Blocked state to wait for data. */
			if( prvIsQueueEmpty( pxQueue ) != pdFALSE )
			{
				traceBLOCKING_ON_QUEUE_PEEK( pxQueue );
				vTaskPlaceOnEventList( &( pxQueue->xTasksWaitingToReceive ), xTicksToWait );
				prvUnlockQueue( pxQueue );
				if( xTaskResumeAll() == pdFALSE )
				{
					portYIELD_WITHIN_API();
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				/* There is data in the queue now, so don't enter the blocked
				state, instead return to try and obtain the data. */
				prvUnlockQueue( pxQueue );
				( void ) xTaskResumeAll();
			}
		}
		else
		{
			/* The timeout has expired.  If there is still no data in the queue
			exit, otherwise go back and try to read the data again. */
			prvUnlockQueue( pxQueue );
			( void ) xTaskResumeAll();

			if( prvIsQueueEmpty( pxQueue ) != pdFALSE )
			{
				traceQUEUE_PEEK_FAILED( pxQueue );
				return errQUEUE_EMPTY;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
	}
}
/*-----------------------------------------------------------*/

BaseType_t xQueueReceiveFromISR( QueueHandle_t xQueue, void * const pvBuffer, BaseType_t * const pxHigherPriorityTaskWoken )
{
BaseType_t xReturn;
UBaseType_t uxSavedInterruptStatus;
Queue_t * const pxQueue = ( Queue_t * ) xQueue;

	configASSERT( pxQueue );
	configASSERT( !( ( pvBuffer == NULL ) && ( pxQueue->uxItemSize != ( UBaseType_t ) 0U ) ) );

	/* RTOS ports that support interrupt nesting have the concept of a maximum
	system call (or maximum API call) interrupt priority.  Interrupts that are
	above the maximum system call priority are kept permanently enabled, even
	when the RTOS kernel is in a critical section, but cannot make any calls to
	FreeRTOS API functions.  If configASSERT() is defined in FreeRTOSConfig.h
	then portASSERT_IF_INTERRUPT_PRIORITY_INVALID() will result in an assertion
	failure if a FreeRTOS API function is called from an interrupt that has been
	assigned a priority above the configured maximum system call priority.
	Only FreeRTOS functions that end in FromISR can be called from interrupts
	that have been assigned a priority at or (logically) below the maximum
	system call	interrupt priority.  FreeRTOS maintains a separate interrupt
	safe API to ensure interrupt entry is as fast and as simple as possible.
	More information (albeit Cortex-M specific) is provided on the following
	link: http://www.freertos.org/RTOS-Cortex-M3-M4.html */
	portASSERT_IF_INTERRUPT_PRIORITY_INVALID();

	uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
	{
		const UBaseType_t uxMessagesWaiting = pxQueue->uxMessagesWaiting;

		/* Cannot block in an ISR, so check there is data available. */
		if( uxMessagesWaiting > ( UBaseType_t ) 0 )
		{
			const int8_t cRxLock = pxQueue->cRxLock;

			traceQUEUE_RECEIVE_FROM_ISR( pxQueue );

			prvCopyDataFromQueue( pxQueue, pvBuffer );
			pxQueue->uxMessagesWaiting = uxMessagesWaiting - ( UBaseType_t ) 1;

			/* If the queue is locked the event list will not be modified.
			Instead update the lock count so the task that unlocks the queue
			will know that an ISR has removed data while the queue was
			locked. */
			if( cRxLock == queueUNLOCKED )
			{
				if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) == pdFALSE )
				{
					if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToSend ) ) != pdFALSE )
					{
						/* The task waiting has a higher priority than us so
						force a context switch. */
						if( pxHigherPriorityTaskWoken != NULL )
						{
							*pxHigherPriorityTaskWoken = pdTRUE;
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
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				/* Increment the lock count so the task that unlocks the queue
				knows that data was removed while it was locked. */
				pxQueue->cRxLock = ( int8_t ) ( cRxLock + 1 );
			}

			xReturn = pdPASS;
		}
		else
		{
			xReturn = pdFAIL;
			traceQUEUE_RECEIVE_FROM_ISR_FAILED( pxQueue );
		}
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

	return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xQueuePeekFromISR( QueueHandle_t xQueue,  void * const pvBuffer )
{
BaseType_t xReturn;
UBaseType_t uxSavedInterruptStatus;
int8_t *pcOriginalReadPosition;
Queue_t * const pxQueue = ( Queue_t * ) xQueue;

	configASSERT( pxQueue );
	configASSERT( !( ( pvBuffer == NULL ) && ( pxQueue->uxItemSize != ( UBaseType_t ) 0U ) ) );
	configASSERT( pxQueue->uxItemSize != 0 ); /* Can't peek a semaphore. */

	/* RTOS ports that support interrupt nesting have the concept of a maximum
	system call (or maximum API call) interrupt priority.  Interrupts that are
	above the maximum system call priority are kept permanently enabled, even
	when the RTOS kernel is in a critical section, but cannot make any calls to
	FreeRTOS API functions.  If configASSERT() is defined in FreeRTOSConfig.h
	then portASSERT_IF_INTERRUPT_PRIORITY_INVALID() will result in an assertion
	failure if a FreeRTOS API function is called from an interrupt that has been
	assigned a priority above the configured maximum system call priority.
	Only FreeRTOS functions that end in FromISR can be called from interrupts
	that have been assigned a priority at or (logically) below the maximum
	system call	interrupt priority.  FreeRTOS maintains a separate interrupt
	safe API to ensure interrupt entry is as fast and as simple as possible.
	More information (albeit Cortex-M specific) is provided on the following
	link: http://www.freertos.org/RTOS-Cortex-M3-M4.html */
	portASSERT_IF_INTERRUPT_PRIORITY_INVALID();

	uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
	{
		/* Cannot block in an ISR, so check there is data available. */
		if( pxQueue->uxMessagesWaiting > ( UBaseType_t ) 0 )
		{
			traceQUEUE_PEEK_FROM_ISR( pxQueue );

			/* Remember the read position so it can be reset as nothing is
			actually being removed from the queue. */
			pcOriginalReadPosition = pxQueue->u.pcReadFrom;
			prvCopyDataFromQueue( pxQueue, pvBuffer );
			pxQueue->u.pcReadFrom = pcOriginalReadPosition;

			xReturn = pdPASS;
		}
		else
		{
			xReturn = pdFAIL;
			traceQUEUE_PEEK_FROM_ISR_FAILED( pxQueue );
		}
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

	return xReturn;
}
/*-----------------------------------------------------------*/

UBaseType_t uxQueueMessagesWaiting( const QueueHandle_t xQueue )
{
UBaseType_t uxReturn;

	configASSERT( xQueue );

	taskENTER_CRITICAL();
	{
		uxReturn = ( ( Queue_t * ) xQueue )->uxMessagesWaiting;
	}
	taskEXIT_CRITICAL();

	return uxReturn;
} /*lint !e818 Pointer cannot be declared const as xQueue is a typedef not pointer. */
/*-----------------------------------------------------------*/

UBaseType_t uxQueueSpacesAvailable( const QueueHandle_t xQueue )
{
UBaseType_t uxReturn;
Queue_t *pxQueue;

	pxQueue = ( Queue_t * ) xQueue;
	configASSERT( pxQueue );

	taskENTER_CRITICAL();
	{
		uxReturn = pxQueue->uxLength - pxQueue->uxMessagesWaiting;
	}
	taskEXIT_CRITICAL();

	return uxReturn;
} /*lint !e818 Pointer cannot be declared const as xQueue is a typedef not pointer. */
/*-----------------------------------------------------------*/

UBaseType_t uxQueueMessagesWaitingFromISR( const QueueHandle_t xQueue )
{
UBaseType_t uxReturn;

	configASSERT( xQueue );

	uxReturn = ( ( Queue_t * ) xQueue )->uxMessagesWaiting;

	return uxReturn;
} /*lint !e818 Pointer cannot be declared const as xQueue is a typedef not pointer. */
/*-----------------------------------------------------------*/

void vQueueDelete( QueueHandle_t xQueue )
{
Queue_t * const pxQueue = ( Queue_t * ) xQueue;

	configASSERT( pxQueue );
	traceQUEUE_DELETE( pxQueue );

	#if ( configQUEUE_REGISTRY_SIZE > 0 )
	{
		vQueueUnregisterQueue( pxQueue );
	}
	#endif

	#if( ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 0 ) )
	{
		/* The queue can only have been allocated dynamically - free it
		again. */
		vPortFree( pxQueue );
	}
	#elif( ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 1 ) )
	{
		/* The queue could have been allocated statically or dynamically, so
		check before attempting to free the memory. */
		if( pxQueue->ucStaticallyAllocated == ( uint8_t ) pdFALSE )
		{
			vPortFree( pxQueue );
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	#else
	{
		/* The queue must have been statically allocated, so is not going to be
		deleted.  Avoid compiler warnings about the unused parameter. */
		( void ) pxQueue;
	}
	#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
}
/*-----------------------------------------------------------*/

#if ( configUSE_TRACE_FACILITY == 1 )

	UBaseType_t uxQueueGetQueueNumber( QueueHandle_t xQueue )
	{
		return ( ( Queue_t * ) xQueue )->uxQueueNumber;
	}

#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if ( configUSE_TRACE_FACILITY == 1 )

	void vQueueSetQueueNumber( QueueHandle_t xQueue, UBaseType_t uxQueueNumber )
	{
		( ( Queue_t * ) xQueue )->uxQueueNumber = uxQueueNumber;
	}

#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if ( configUSE_TRACE_FACILITY == 1 )

	uint8_t ucQueueGetQueueType( QueueHandle_t xQueue )
	{
		return ( ( Queue_t * ) xQueue )->ucQueueType;
	}

#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if( configUSE_MUTEXES == 1 )

	static UBaseType_t prvGetDisinheritPriorityAfterTimeout( const Queue_t * const pxQueue )
	{
	UBaseType_t uxHighestPriorityOfWaitingTasks;

		/* If a task waiting for a mutex causes the mutex holder to inherit a
		priority, but the waiting task times out, then the holder should
		disinherit the priority - but only down to the highest priority of any
		other tasks that are waiting for the same mutex.  For this purpose,
		return the priority of the highest priority task that is waiting for the
		mutex. */
		if( listCURRENT_LIST_LENGTH( &( pxQueue->xTasksWaitingToReceive ) ) > 0 )
		{
			uxHighestPriorityOfWaitingTasks = configMAX_PRIORITIES - listGET_ITEM_VALUE_OF_HEAD_ENTRY( &( pxQueue->xTasksWaitingToReceive ) );
		}
		else
		{
			uxHighestPriorityOfWaitingTasks = tskIDLE_PRIORITY;
		}

		return uxHighestPriorityOfWaitingTasks;
	}

#endif /* configUSE_MUTEXES */
/*-----------------------------------------------------------*/

static BaseType_t prvCopyDataToQueue( Queue_t * const pxQueue, const void *pvItemToQueue, const BaseType_t xPosition )
{
BaseType_t xReturn = pdFALSE;
UBaseType_t uxMessagesWaiting;

	/* This function is called from a critical section. */

	uxMessagesWaiting = pxQueue->uxMessagesWaiting;

	if( pxQueue->uxItemSize == ( UBaseType_t ) 0 )
	{
		#if ( configUSE_MUTEXES == 1 )
		{
			if( pxQueue->uxQueueType == queueQUEUE_IS_MUTEX )
			{
				/* The mutex is no longer being held. */
				xReturn = xTaskPriorityDisinherit( ( void * ) pxQueue->pxMutexHolder );
				pxQueue->pxMutexHolder = NULL;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		#endif /* configUSE_MUTEXES */
	}
	else if( xPosition == queueSEND_TO_BACK )
	{
		( void ) memcpy( ( void * ) pxQueue->pcWriteTo, pvItemToQueue, ( size_t ) pxQueue->uxItemSize ); /*lint !e961 !e418 MISRA exception as the casts are only redundant for some ports, plus previous logic ensures a null pointer can only be passed to memcpy() if the copy size is 0. */
		pxQueue->pcWriteTo += pxQueue->uxItemSize;
		if( pxQueue->pcWriteTo >= pxQueue->pcTail ) /*lint !e946 MISRA exception justified as comparison of pointers is the cleanest solution. */
		{
			pxQueue->pcWriteTo = pxQueue->pcHead;
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	else
	{
		( void ) memcpy( ( void * ) pxQueue->u.pcReadFrom, pvItemToQueue, ( size_t ) pxQueue->uxItemSize ); /*lint !e961 MISRA exception as the casts are only redundant for some ports. */
		pxQueue->u.pcReadFrom -= pxQueue->uxItemSize;
		if( pxQueue->u.pcReadFrom < pxQueue->pcHead ) /*lint !e946 MISRA exception justified as comparison of pointers is the cleanest solution. */
		{
			pxQueue->u.pcReadFrom = ( pxQueue->pcTail - pxQueue->uxItemSize );
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		if( xPosition == queueOVERWRITE )
		{
			if( uxMessagesWaiting > ( UBaseType_t ) 0 )
			{
				/* An item is not being added but overwritten, so subtract
				one from the recorded number of items in the queue so when
				one is added again below the number of recorded items remains
				correct. */
				--uxMessagesWaiting;
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
	}

	pxQueue->uxMessagesWaiting = uxMessagesWaiting + ( UBaseType_t ) 1;

	return xReturn;
}
/*-----------------------------------------------------------*/

static void prvCopyDataFromQueue( Queue_t * const pxQueue, void * const pvBuffer )
{
	if( pxQueue->uxItemSize != ( UBaseType_t ) 0 )
	{
		pxQueue->u.pcReadFrom += pxQueue->uxItemSize;
		if( pxQueue->u.pcReadFrom >= pxQueue->pcTail ) /*lint !e946 MISRA exception justified as use of the relational operator is the cleanest solutions. */
		{
			pxQueue->u.pcReadFrom = pxQueue->pcHead;
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
		( void ) memcpy( ( void * ) pvBuffer, ( void * ) pxQueue->u.pcReadFrom, ( size_t ) pxQueue->uxItemSize ); /*lint !e961 !e418 MISRA exception as the casts are only redundant for some ports.  Also previous logic ensures a null pointer can only be passed to memcpy() when the count is 0. */
	}
}
/*-----------------------------------------------------------*/

static void prvUnlockQueue( Queue_t * const pxQueue )
{
	/* THIS FUNCTION MUST BE CALLED WITH THE SCHEDULER SUSPENDED. */

	/* The lock counts contains the number of extra data items placed or
	removed from the queue while the queue was locked.  When a queue is
	locked items can be added or removed, but the event lists cannot be
	updated. */
	taskENTER_CRITICAL();
	{
		int8_t cTxLock = pxQueue->cTxLock;

		/* See if data was added to the queue while it was locked. */
		while( cTxLock > queueLOCKED_UNMODIFIED )
		{
			/* Data was posted while the queue was locked.  Are any tasks
			blocked waiting for data to become available? */
			#if ( configUSE_QUEUE_SETS == 1 )
			{
				if( pxQueue->pxQueueSetContainer != NULL )
				{
					if( prvNotifyQueueSetContainer( pxQueue, queueSEND_TO_BACK ) != pdFALSE )
					{
						/* The queue is a member of a queue set, and posting to
						the queue set caused a higher priority task to unblock.
						A context switch is required. */
						vTaskMissedYield();
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}
				}
				else
				{
					/* Tasks that are removed from the event list will get
					added to the pending ready list as the scheduler is still
					suspended. */
					if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
					{
						if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
						{
							/* The task waiting has a higher priority so record that a
							context	switch is required. */
							vTaskMissedYield();
						}
						else
						{
							mtCOVERAGE_TEST_MARKER();
						}
					}
					else
					{
						break;
					}
				}
			}
			#else /* configUSE_QUEUE_SETS */
			{
				/* Tasks that are removed from the event list will get added to
				the pending ready list as the scheduler is still suspended. */
				if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
				{
					if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
					{
						/* The task waiting has a higher priority so record that
						a context switch is required. */
						vTaskMissedYield();
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}
				}
				else
				{
					break;
				}
			}
			#endif /* configUSE_QUEUE_SETS */

			--cTxLock;
		}

		pxQueue->cTxLock = queueUNLOCKED;
	}
	taskEXIT_CRITICAL();

	/* Do the same for the Rx lock. */
	taskENTER_CRITICAL();
	{
		int8_t cRxLock = pxQueue->cRxLock;

		while( cRxLock > queueLOCKED_UNMODIFIED )
		{
			if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) == pdFALSE )
			{
				if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToSend ) ) != pdFALSE )
				{
					vTaskMissedYield();
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}

				--cRxLock;
			}
			else
			{
				break;
			}
		}

		pxQueue->cRxLock = queueUNLOCKED;
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

static BaseType_t prvIsQueueEmpty( const Queue_t *pxQueue )
{
BaseType_t xReturn;

	taskENTER_CRITICAL();
	{
		if( pxQueue->uxMessagesWaiting == ( UBaseType_t )  0 )
		{
			xReturn = pdTRUE;
		}
		else
		{
			xReturn = pdFALSE;
		}
	}
	taskEXIT_CRITICAL();

	return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xQueueIsQueueEmptyFromISR( const QueueHandle_t xQueue )
{
BaseType_t xReturn;

	configASSERT( xQueue );
	if( ( ( Queue_t * ) xQueue )->uxMessagesWaiting == ( UBaseType_t ) 0 )
	{
		xReturn = pdTRUE;
	}
	else
	{
		xReturn = pdFALSE;
	}

	return xReturn;
} /*lint !e818 xQueue could not be pointer to const because it is a typedef. */
/*-----------------------------------------------------------*/

static BaseType_t prvIsQueueFull( const Queue_t *pxQueue )
{
BaseType_t xReturn;

	taskENTER_CRITICAL();
	{
		if( pxQueue->uxMessagesWaiting == pxQueue->uxLength )
		{
			xReturn = pdTRUE;
		}
		else
		{
			xReturn = pdFALSE;
		}
	}
	taskEXIT_CRITICAL();

	return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xQueueIsQueueFullFromISR( const QueueHandle_t xQueue )
{
BaseType_t xReturn;

	configASSERT( xQueue );
	if( ( ( Queue_t * ) xQueue )->uxMessagesWaiting == ( ( Queue_t * ) xQueue )->uxLength )
	{
		xReturn = pdTRUE;
	}
	else
	{
		xReturn = pdFALSE;
	}

	return xReturn;
} /*lint !e818 xQueue could not be pointer to const because it is a typedef. */
/*-----------------------------------------------------------*/

#if ( configUSE_CO_ROUTINES == 1 )

	BaseType_t xQueueCRSend( QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait )
	{
	BaseType_t xReturn;
	Queue_t * const pxQueue = ( Queue_t * ) xQueue;

		/* If the queue is already full we may have to block.  A critical section
		is required to prevent an interrupt removing something from the queue
		between the check to see if the queue is full and blocking on the queue. */
		portDISABLE_INTERRUPTS();
		{
			if( prvIsQueueFull( pxQueue ) != pdFALSE )
			{
				/* The queue is full - do we want to block or just leave without
				posting? */
				if( xTicksToWait > ( TickType_t ) 0 )
				{
					/* As this is called from a coroutine we cannot block directly, but
					return indicating that we need to block. */
					vCoRoutineAddToDelayedList( xTicksToWait, &( pxQueue->xTasksWaitingToSend ) );
					portENABLE_INTERRUPTS();
					return errQUEUE_BLOCKED;
				}
				else
				{
					portENABLE_INTERRUPTS();
					return errQUEUE_FULL;
				}
			}
		}
		portENABLE_INTERRUPTS();

		portDISABLE_INTERRUPTS();
		{
			if( pxQueue->uxMessagesWaiting < pxQueue->uxLength )
			{
				/* There is room in the queue, copy the data into the queue. */
				prvCopyDataToQueue( pxQueue, pvItemToQueue, queueSEND_TO_BACK );
				xReturn = pdPASS;

				/* Were any co-routines waiting for data to become available? */
				if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
				{
					/* In this instance the co-routine could be placed directly
					into the ready list as we are within a critical section.
					Instead the same pending ready list mechanism is used as if
					the event were caused from within an interrupt. */
					if( xCoRoutineRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
					{
						/* The co-routine waiting has a higher priority so record
						that a yield might be appropriate. */
						xReturn = errQUEUE_YIELD;
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
			}
			else
			{
				xReturn = errQUEUE_FULL;
			}
		}
		portENABLE_INTERRUPTS();

		return xReturn;
	}

#endif /* configUSE_CO_ROUTINES */
/*-----------------------------------------------------------*/

#if ( configUSE_CO_ROUTINES == 1 )

	BaseType_t xQueueCRReceive( QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait )
	{
	BaseType_t xReturn;
	Queue_t * const pxQueue = ( Queue_t * ) xQueue;

		/* If the queue is already empty we may have to block.  A critical section
		is required to prevent an interrupt adding something to the queue
		between the check to see if the queue is empty and blocking on the queue. */
		portDISABLE_INTERRUPTS();
		{
			if( pxQueue->uxMessagesWaiting == ( UBaseType_t ) 0 )
			{
				/* There are no messages in the queue, do we want to block or just
				leave with nothing? */
				if( xTicksToWait > ( TickType_t ) 0 )
				{
					/* As this is a co-routine we cannot block directly, but return
					indicating that we need to block. */
					vCoRoutineAddToDelayedList( xTicksToWait, &( pxQueue->xTasksWaitingToReceive ) );
					portENABLE_INTERRUPTS();
					return errQUEUE_BLOCKED;
				}
				else
				{
					portENABLE_INTERRUPTS();
					return errQUEUE_FULL;
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		portENABLE_INTERRUPTS();

		portDISABLE_INTERRUPTS();
		{
			if( pxQueue->uxMessagesWaiting > ( UBaseType_t ) 0 )
			{
				/* Data is available from the queue. */
				pxQueue->u.pcReadFrom += pxQueue->uxItemSize;
				if( pxQueue->u.pcReadFrom >= pxQueue->pcTail )
				{
					pxQueue->u.pcReadFrom = pxQueue->pcHead;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
				--( pxQueue->uxMessagesWaiting );
				( void ) memcpy( ( void * ) pvBuffer, ( void * ) pxQueue->u.pcReadFrom, ( unsigned ) pxQueue->uxItemSize );

				xReturn = pdPASS;

				/* Were any co-routines waiting for space to become available? */
				if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) == pdFALSE )
				{
					/* In this instance the co-routine could be placed directly
					into the ready list as we are within a critical section.
					Instead the same pending ready list mechanism is used as if
					the event were caused from within an interrupt. */
					if( xCoRoutineRemoveFromEventList( &( pxQueue->xTasksWaitingToSend ) ) != pdFALSE )
					{
						xReturn = errQUEUE_YIELD;
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
			}
			else
			{
				xReturn = pdFAIL;
			}
		}
		portENABLE_INTERRUPTS();

		return xReturn;
	}

#endif /* configUSE_CO_ROUTINES */
/*-----------------------------------------------------------*/

#if ( configUSE_CO_ROUTINES == 1 )

	BaseType_t xQueueCRSendFromISR( QueueHandle_t xQueue, const void *pvItemToQueue, BaseType_t xCoRoutinePreviouslyWoken )
	{
	Queue_t * const pxQueue = ( Queue_t * ) xQueue;

		/* Cannot block within an ISR so if there is no space on the queue then
		exit without doing anything. */
		if( pxQueue->uxMessagesWaiting < pxQueue->uxLength )
		{
			prvCopyDataToQueue( pxQueue, pvItemToQueue, queueSEND_TO_BACK );

			/* We only want to wake one co-routine per ISR, so check that a
			co-routine has not already been woken. */
			if( xCoRoutinePreviouslyWoken == pdFALSE )
			{
				if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
				{
					if( xCoRoutineRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
					{
						return pdTRUE;
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

		return xCoRoutinePreviouslyWoken;
	}

#endif /* configUSE_CO_ROUTINES */
/*-----------------------------------------------------------*/

#if ( configUSE_CO_ROUTINES == 1 )

	BaseType_t xQueueCRReceiveFromISR( QueueHandle_t xQueue, void *pvBuffer, BaseType_t *pxCoRoutineWoken )
	{
	BaseType_t xReturn;
	Queue_t * const pxQueue = ( Queue_t * ) xQueue;

		/* We cannot block from an ISR, so check there is data available. If
		not then just leave without doing anything. */
		if( pxQueue->uxMessagesWaiting > ( UBaseType_t ) 0 )
		{
			/* Copy the data from the queue. */
			pxQueue->u.pcReadFrom += pxQueue->uxItemSize;
			if( pxQueue->u.pcReadFrom >= pxQueue->pcTail )
			{
				pxQueue->u.pcReadFrom = pxQueue->pcHead;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
			--( pxQueue->uxMessagesWaiting );
			( void ) memcpy( ( void * ) pvBuffer, ( void * ) pxQueue->u.pcReadFrom, ( unsigned ) pxQueue->uxItemSize );

			if( ( *pxCoRoutineWoken ) == pdFALSE )
			{
				if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) == pdFALSE )
				{
					if( xCoRoutineRemoveFromEventList( &( pxQueue->xTasksWaitingToSend ) ) != pdFALSE )
					{
						*pxCoRoutineWoken = pdTRUE;
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
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			xReturn = pdPASS;
		}
		else
		{
			xReturn = pdFAIL;
		}

		return xReturn;
	}

#endif /* configUSE_CO_ROUTINES */
/*-----------------------------------------------------------*/

#if ( configQUEUE_REGISTRY_SIZE > 0 )

	void vQueueAddToRegistry( QueueHandle_t xQueue, const char *pcQueueName ) /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
	{
	UBaseType_t ux;

		/* See if there is an empty space in the registry.  A NULL name denotes
		a free slot. */
		for( ux = ( UBaseType_t ) 0U; ux < ( UBaseType_t ) configQUEUE_REGISTRY_SIZE; ux++ )
		{
			if( xQueueRegistry[ ux ].pcQueueName == NULL )
			{
				/* Store the information on this queue. */
				xQueueRegistry[ ux ].pcQueueName = pcQueueName;
				xQueueRegistry[ ux ].xHandle = xQueue;

				traceQUEUE_REGISTRY_ADD( xQueue, pcQueueName );
				break;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
	}

#endif /* configQUEUE_REGISTRY_SIZE */
/*-----------------------------------------------------------*/

#if ( configQUEUE_REGISTRY_SIZE > 0 )

	const char *pcQueueGetName( QueueHandle_t xQueue ) /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
	{
	UBaseType_t ux;
	const char *pcReturn = NULL; /*lint !e971 Unqualified char types are allowed for strings and single characters only. */

		/* Note there is nothing here to protect against another task adding or
		removing entries from the registry while it is being searched. */
		for( ux = ( UBaseType_t ) 0U; ux < ( UBaseType_t ) configQUEUE_REGISTRY_SIZE; ux++ )
		{
			if( xQueueRegistry[ ux ].xHandle == xQueue )
			{
				pcReturn = xQueueRegistry[ ux ].pcQueueName;
				break;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}

		return pcReturn;
	} /*lint !e818 xQueue cannot be a pointer to const because it is a typedef. */

#endif /* configQUEUE_REGISTRY_SIZE */
/*-----------------------------------------------------------*/

#if ( configQUEUE_REGISTRY_SIZE > 0 )

	void vQueueUnregisterQueue( QueueHandle_t xQueue )
	{
	UBaseType_t ux;

		/* See if the handle of the queue being unregistered in actually in the
		registry. */
		for( ux = ( UBaseType_t ) 0U; ux < ( UBaseType_t ) configQUEUE_REGISTRY_SIZE; ux++ )
		{
			if( xQueueRegistry[ ux ].xHandle == xQueue )
			{
				/* Set the name to NULL to show that this slot if free again. */
				xQueueRegistry[ ux ].pcQueueName = NULL;

				/* Set the handle to NULL to ensure the same queue handle cannot
				appear in the registry twice if it is added, removed, then
				added again. */
				xQueueRegistry[ ux ].xHandle = ( QueueHandle_t ) 0;
				break;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}

	} /*lint !e818 xQueue could not be pointer to const because it is a typedef. */

#endif /* configQUEUE_REGISTRY_SIZE */
/*-----------------------------------------------------------*/

#if ( configUSE_TIMERS == 1 )

	void vQueueWaitForMessageRestricted( QueueHandle_t xQueue, TickType_t xTicksToWait, const BaseType_t xWaitIndefinitely )
	{
	Queue_t * const pxQueue = ( Queue_t * ) xQueue;

		/* This function should not be called by application code hence the
		'Restricted' in its name.  It is not part of the public API.  It is
		designed for use by kernel code, and has special calling requirements.
		It can result in vListInsert() being called on a list that can only
		possibly ever have one item in it, so the list will be fast, but even
		so it should be called with the scheduler locked and not from a critical
		section. */

		/* Only do anything if there are no messages in the queue.  This function
		will not actually cause the task to block, just place it on a blocked
		list.  It will not block until the scheduler is unlocked - at which
		time a yield will be performed.  If an item is added to the queue while
		the queue is locked, and the calling task blocks on the queue, then the
		calling task will be immediately unblocked when the queue is unlocked. */
		prvLockQueue( pxQueue );
		if( pxQueue->uxMessagesWaiting == ( UBaseType_t ) 0U )
		{
			/* There is nothing in the queue, block for the specified period. */
			vTaskPlaceOnEventListRestricted( &( pxQueue->xTasksWaitingToReceive ), xTicksToWait, xWaitIndefinitely );
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
		prvUnlockQueue( pxQueue );
	}

#endif /* configUSE_TIMERS */
/*-----------------------------------------------------------*/

#if( ( configUSE_QUEUE_SETS == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )

	QueueSetHandle_t xQueueCreateSet( const UBaseType_t uxEventQueueLength )
	{
	QueueSetHandle_t pxQueue;

		pxQueue = xQueueGenericCreate( uxEventQueueLength, ( UBaseType_t ) sizeof( Queue_t * ), queueQUEUE_TYPE_SET );

		return pxQueue;
	}

#endif /* configUSE_QUEUE_SETS */
/*-----------------------------------------------------------*/

#if ( configUSE_QUEUE_SETS == 1 )

	BaseType_t xQueueAddToSet( QueueSetMemberHandle_t xQueueOrSemaphore, QueueSetHandle_t xQueueSet )
	{
	BaseType_t xReturn;

		taskENTER_CRITICAL();
		{
			if( ( ( Queue_t * ) xQueueOrSemaphore )->pxQueueSetContainer != NULL )
			{
				/* Cannot add a queue/semaphore to more than one queue set. */
				xReturn = pdFAIL;
			}
			else if( ( ( Queue_t * ) xQueueOrSemaphore )->uxMessagesWaiting != ( UBaseType_t ) 0 )
			{
				/* Cannot add a queue/semaphore to a queue set if there are already
				items in the queue/semaphore. */
				xReturn = pdFAIL;
			}
			else
			{
				( ( Queue_t * ) xQueueOrSemaphore )->pxQueueSetContainer = xQueueSet;
				xReturn = pdPASS;
			}
		}
		taskEXIT_CRITICAL();

		return xReturn;
	}

#endif /* configUSE_QUEUE_SETS */
/*-----------------------------------------------------------*/

#if ( configUSE_QUEUE_SETS == 1 )

	BaseType_t xQueueRemoveFromSet( QueueSetMemberHandle_t xQueueOrSemaphore, QueueSetHandle_t xQueueSet )
	{
	BaseType_t xReturn;
	Queue_t * const pxQueueOrSemaphore = ( Queue_t * ) xQueueOrSemaphore;

		if( pxQueueOrSemaphore->pxQueueSetContainer != xQueueSet )
		{
			/* The queue was not a member of the set. */
			xReturn = pdFAIL;
		}
		else if( pxQueueOrSemaphore->uxMessagesWaiting != ( UBaseType_t ) 0 )
		{
			/* It is dangerous to remove a queue from a set when the queue is
			not empty because the queue set will still hold pending events for
			the queue. */
			xReturn = pdFAIL;
		}
		else
		{
			taskENTER_CRITICAL();
			{
				/* The queue is no longer contained in the set. */
				pxQueueOrSemaphore->pxQueueSetContainer = NULL;
			}
			taskEXIT_CRITICAL();
			xReturn = pdPASS;
		}

		return xReturn;
	} /*lint !e818 xQueueSet could not be declared as pointing to const as it is a typedef. */

#endif /* configUSE_QUEUE_SETS */
/*-----------------------------------------------------------*/

#if ( configUSE_QUEUE_SETS == 1 )

	QueueSetMemberHandle_t xQueueSelectFromSet( QueueSetHandle_t xQueueSet, TickType_t const xTicksToWait )
	{
	QueueSetMemberHandle_t xReturn = NULL;

		( void ) xQueueReceive( ( QueueHandle_t ) xQueueSet, &xReturn, xTicksToWait ); /*lint !e961 Casting from one typedef to another is not redundant. */
		return xReturn;
	}

#endif /* configUSE_QUEUE_SETS */
/*-----------------------------------------------------------*/

#if ( configUSE_QUEUE_SETS == 1 )

	QueueSetMemberHandle_t xQueueSelectFromSetFromISR( QueueSetHandle_t xQueueSet )
	{
	QueueSetMemberHandle_t xReturn = NULL;

		( void ) xQueueReceiveFromISR( ( QueueHandle_t ) xQueueSet, &xReturn, NULL ); /*lint !e961 Casting from one typedef to another is not redundant. */
		return xReturn;
	}

#endif /* configUSE_QUEUE_SETS */
/*-----------------------------------------------------------*/

#if ( configUSE_QUEUE_SETS == 1 )

	static BaseType_t prvNotifyQueueSetContainer( const Queue_t * const pxQueue, const BaseType_t xCopyPosition )
	{
	Queue_t *pxQueueSetContainer = pxQueue->pxQueueSetContainer;
	BaseType_t xReturn = pdFALSE;

		/* This function must be called form a critical section. */

		configASSERT( pxQueueSetContainer );
		configASSERT( pxQueueSetContainer->uxMessagesWaiting < pxQueueSetContainer->uxLength );

		if( pxQueueSetContainer->uxMessagesWaiting < pxQueueSetContainer->uxLength )
		{
			const int8_t cTxLock = pxQueueSetContainer->cTxLock;

			traceQUEUE_SEND( pxQueueSetContainer );

			/* The data copied is the handle of the queue that contains data. */
			xReturn = prvCopyDataToQueue( pxQueueSetContainer, &pxQueue, xCopyPosition );

			if( cTxLock == queueUNLOCKED )
			{
				if( listLIST_IS_EMPTY( &( pxQueueSetContainer->xTasksWaitingToReceive ) ) == pdFALSE )
				{
					if( xTaskRemoveFromEventList( &( pxQueueSetContainer->xTasksWaitingToReceive ) ) != pdFALSE )
					{
						/* The task waiting has a higher priority. */
						xReturn = pdTRUE;
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
			}
			else
			{
				pxQueueSetContainer->cTxLock = ( int8_t ) ( cTxLock + 1 );
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		return xReturn;
	}

#endif /* configUSE_QUEUE_SETS */












