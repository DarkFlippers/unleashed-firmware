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

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "event_groups.h"

/* Lint e961 and e750 are suppressed as a MISRA exception justified because the
MPU ports require MPU_WRAPPERS_INCLUDED_FROM_API_FILE to be defined for the
header files above, but not in this file, in order to generate the correct
privileged Vs unprivileged linkage and placement. */
#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE /*lint !e961 !e750. */

/* The following bit fields convey control information in a task's event list
item value.  It is important they don't clash with the
taskEVENT_LIST_ITEM_VALUE_IN_USE definition. */
#if configUSE_16_BIT_TICKS == 1
	#define eventCLEAR_EVENTS_ON_EXIT_BIT	0x0100U
	#define eventUNBLOCKED_DUE_TO_BIT_SET	0x0200U
	#define eventWAIT_FOR_ALL_BITS			0x0400U
	#define eventEVENT_BITS_CONTROL_BYTES	0xff00U
#else
	#define eventCLEAR_EVENTS_ON_EXIT_BIT	0x01000000UL
	#define eventUNBLOCKED_DUE_TO_BIT_SET	0x02000000UL
	#define eventWAIT_FOR_ALL_BITS			0x04000000UL
	#define eventEVENT_BITS_CONTROL_BYTES	0xff000000UL
#endif

typedef struct xEventGroupDefinition
{
	EventBits_t uxEventBits;
	List_t xTasksWaitingForBits;		/*< List of tasks waiting for a bit to be set. */

	#if( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t uxEventGroupNumber;
	#endif

	#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
		uint8_t ucStaticallyAllocated; /*< Set to pdTRUE if the event group is statically allocated to ensure no attempt is made to free the memory. */
	#endif
} EventGroup_t;

/*-----------------------------------------------------------*/

/*
 * Test the bits set in uxCurrentEventBits to see if the wait condition is met.
 * The wait condition is defined by xWaitForAllBits.  If xWaitForAllBits is
 * pdTRUE then the wait condition is met if all the bits set in uxBitsToWaitFor
 * are also set in uxCurrentEventBits.  If xWaitForAllBits is pdFALSE then the
 * wait condition is met if any of the bits set in uxBitsToWait for are also set
 * in uxCurrentEventBits.
 */
static BaseType_t prvTestWaitCondition( const EventBits_t uxCurrentEventBits, const EventBits_t uxBitsToWaitFor, const BaseType_t xWaitForAllBits ) PRIVILEGED_FUNCTION;

/*-----------------------------------------------------------*/

#if( configSUPPORT_STATIC_ALLOCATION == 1 )

	EventGroupHandle_t xEventGroupCreateStatic( StaticEventGroup_t *pxEventGroupBuffer )
	{
	EventGroup_t *pxEventBits;

		/* A StaticEventGroup_t object must be provided. */
		configASSERT( pxEventGroupBuffer );

		#if( configASSERT_DEFINED == 1 )
		{
			/* Sanity check that the size of the structure used to declare a
			variable of type StaticEventGroup_t equals the size of the real
			event group structure. */
			volatile size_t xSize = sizeof( StaticEventGroup_t );
			configASSERT( xSize == sizeof( EventGroup_t ) );
		}
		#endif /* configASSERT_DEFINED */

		/* The user has provided a statically allocated event group - use it. */
		pxEventBits = ( EventGroup_t * ) pxEventGroupBuffer; /*lint !e740 EventGroup_t and StaticEventGroup_t are guaranteed to have the same size and alignment requirement - checked by configASSERT(). */

		if( pxEventBits != NULL )
		{
			pxEventBits->uxEventBits = 0;
			vListInitialise( &( pxEventBits->xTasksWaitingForBits ) );

			#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
			{
				/* Both static and dynamic allocation can be used, so note that
				this event group was created statically in case the event group
				is later deleted. */
				pxEventBits->ucStaticallyAllocated = pdTRUE;
			}
			#endif /* configSUPPORT_DYNAMIC_ALLOCATION */

			traceEVENT_GROUP_CREATE( pxEventBits );
		}
		else
		{
			traceEVENT_GROUP_CREATE_FAILED();
		}

		return ( EventGroupHandle_t ) pxEventBits;
	}

#endif /* configSUPPORT_STATIC_ALLOCATION */
/*-----------------------------------------------------------*/

#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )

	EventGroupHandle_t xEventGroupCreate( void )
	{
	EventGroup_t *pxEventBits;

		/* Allocate the event group. */
		pxEventBits = ( EventGroup_t * ) pvPortMalloc( sizeof( EventGroup_t ) );

		if( pxEventBits != NULL )
		{
			pxEventBits->uxEventBits = 0;
			vListInitialise( &( pxEventBits->xTasksWaitingForBits ) );

			#if( configSUPPORT_STATIC_ALLOCATION == 1 )
			{
				/* Both static and dynamic allocation can be used, so note this
				event group was allocated statically in case the event group is
				later deleted. */
				pxEventBits->ucStaticallyAllocated = pdFALSE;
			}
			#endif /* configSUPPORT_STATIC_ALLOCATION */

			traceEVENT_GROUP_CREATE( pxEventBits );
		}
		else
		{
			traceEVENT_GROUP_CREATE_FAILED();
		}

		return ( EventGroupHandle_t ) pxEventBits;
	}

#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
/*-----------------------------------------------------------*/

EventBits_t xEventGroupSync( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet, const EventBits_t uxBitsToWaitFor, TickType_t xTicksToWait )
{
EventBits_t uxOriginalBitValue, uxReturn;
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
BaseType_t xAlreadyYielded;
BaseType_t xTimeoutOccurred = pdFALSE;

	configASSERT( ( uxBitsToWaitFor & eventEVENT_BITS_CONTROL_BYTES ) == 0 );
	configASSERT( uxBitsToWaitFor != 0 );
	#if ( ( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ) )
	{
		configASSERT( !( ( xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED ) && ( xTicksToWait != 0 ) ) );
	}
	#endif

	vTaskSuspendAll();
	{
		uxOriginalBitValue = pxEventBits->uxEventBits;

		( void ) xEventGroupSetBits( xEventGroup, uxBitsToSet );

		if( ( ( uxOriginalBitValue | uxBitsToSet ) & uxBitsToWaitFor ) == uxBitsToWaitFor )
		{
			/* All the rendezvous bits are now set - no need to block. */
			uxReturn = ( uxOriginalBitValue | uxBitsToSet );

			/* Rendezvous always clear the bits.  They will have been cleared
			already unless this is the only task in the rendezvous. */
			pxEventBits->uxEventBits &= ~uxBitsToWaitFor;

			xTicksToWait = 0;
		}
		else
		{
			if( xTicksToWait != ( TickType_t ) 0 )
			{
				traceEVENT_GROUP_SYNC_BLOCK( xEventGroup, uxBitsToSet, uxBitsToWaitFor );

				/* Store the bits that the calling task is waiting for in the
				task's event list item so the kernel knows when a match is
				found.  Then enter the blocked state. */
				vTaskPlaceOnUnorderedEventList( &( pxEventBits->xTasksWaitingForBits ), ( uxBitsToWaitFor | eventCLEAR_EVENTS_ON_EXIT_BIT | eventWAIT_FOR_ALL_BITS ), xTicksToWait );

				/* This assignment is obsolete as uxReturn will get set after
				the task unblocks, but some compilers mistakenly generate a
				warning about uxReturn being returned without being set if the
				assignment is omitted. */
				uxReturn = 0;
			}
			else
			{
				/* The rendezvous bits were not set, but no block time was
				specified - just return the current event bit value. */
				uxReturn = pxEventBits->uxEventBits;
				xTimeoutOccurred = pdTRUE;
			}
		}
	}
	xAlreadyYielded = xTaskResumeAll();

	if( xTicksToWait != ( TickType_t ) 0 )
	{
		if( xAlreadyYielded == pdFALSE )
		{
			portYIELD_WITHIN_API();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		/* The task blocked to wait for its required bits to be set - at this
		point either the required bits were set or the block time expired.  If
		the required bits were set they will have been stored in the task's
		event list item, and they should now be retrieved then cleared. */
		uxReturn = uxTaskResetEventItemValue();

		if( ( uxReturn & eventUNBLOCKED_DUE_TO_BIT_SET ) == ( EventBits_t ) 0 )
		{
			/* The task timed out, just return the current event bit value. */
			taskENTER_CRITICAL();
			{
				uxReturn = pxEventBits->uxEventBits;

				/* Although the task got here because it timed out before the
				bits it was waiting for were set, it is possible that since it
				unblocked another task has set the bits.  If this is the case
				then it needs to clear the bits before exiting. */
				if( ( uxReturn & uxBitsToWaitFor ) == uxBitsToWaitFor )
				{
					pxEventBits->uxEventBits &= ~uxBitsToWaitFor;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			taskEXIT_CRITICAL();

			xTimeoutOccurred = pdTRUE;
		}
		else
		{
			/* The task unblocked because the bits were set. */
		}

		/* Control bits might be set as the task had blocked should not be
		returned. */
		uxReturn &= ~eventEVENT_BITS_CONTROL_BYTES;
	}

	traceEVENT_GROUP_SYNC_END( xEventGroup, uxBitsToSet, uxBitsToWaitFor, xTimeoutOccurred );

	/* Prevent compiler warnings when trace macros are not used. */
	( void ) xTimeoutOccurred;

	return uxReturn;
}
/*-----------------------------------------------------------*/

EventBits_t xEventGroupWaitBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToWaitFor, const BaseType_t xClearOnExit, const BaseType_t xWaitForAllBits, TickType_t xTicksToWait )
{
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
EventBits_t uxReturn, uxControlBits = 0;
BaseType_t xWaitConditionMet, xAlreadyYielded;
BaseType_t xTimeoutOccurred = pdFALSE;

	/* Check the user is not attempting to wait on the bits used by the kernel
	itself, and that at least one bit is being requested. */
	configASSERT( xEventGroup );
	configASSERT( ( uxBitsToWaitFor & eventEVENT_BITS_CONTROL_BYTES ) == 0 );
	configASSERT( uxBitsToWaitFor != 0 );
	#if ( ( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ) )
	{
		configASSERT( !( ( xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED ) && ( xTicksToWait != 0 ) ) );
	}
	#endif

	vTaskSuspendAll();
	{
		const EventBits_t uxCurrentEventBits = pxEventBits->uxEventBits;

		/* Check to see if the wait condition is already met or not. */
		xWaitConditionMet = prvTestWaitCondition( uxCurrentEventBits, uxBitsToWaitFor, xWaitForAllBits );

		if( xWaitConditionMet != pdFALSE )
		{
			/* The wait condition has already been met so there is no need to
			block. */
			uxReturn = uxCurrentEventBits;
			xTicksToWait = ( TickType_t ) 0;

			/* Clear the wait bits if requested to do so. */
			if( xClearOnExit != pdFALSE )
			{
				pxEventBits->uxEventBits &= ~uxBitsToWaitFor;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else if( xTicksToWait == ( TickType_t ) 0 )
		{
			/* The wait condition has not been met, but no block time was
			specified, so just return the current value. */
			uxReturn = uxCurrentEventBits;
			xTimeoutOccurred = pdTRUE;
		}
		else
		{
			/* The task is going to block to wait for its required bits to be
			set.  uxControlBits are used to remember the specified behaviour of
			this call to xEventGroupWaitBits() - for use when the event bits
			unblock the task. */
			if( xClearOnExit != pdFALSE )
			{
				uxControlBits |= eventCLEAR_EVENTS_ON_EXIT_BIT;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			if( xWaitForAllBits != pdFALSE )
			{
				uxControlBits |= eventWAIT_FOR_ALL_BITS;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			/* Store the bits that the calling task is waiting for in the
			task's event list item so the kernel knows when a match is
			found.  Then enter the blocked state. */
			vTaskPlaceOnUnorderedEventList( &( pxEventBits->xTasksWaitingForBits ), ( uxBitsToWaitFor | uxControlBits ), xTicksToWait );

			/* This is obsolete as it will get set after the task unblocks, but
			some compilers mistakenly generate a warning about the variable
			being returned without being set if it is not done. */
			uxReturn = 0;

			traceEVENT_GROUP_WAIT_BITS_BLOCK( xEventGroup, uxBitsToWaitFor );
		}
	}
	xAlreadyYielded = xTaskResumeAll();

	if( xTicksToWait != ( TickType_t ) 0 )
	{
		if( xAlreadyYielded == pdFALSE )
		{
			portYIELD_WITHIN_API();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		/* The task blocked to wait for its required bits to be set - at this
		point either the required bits were set or the block time expired.  If
		the required bits were set they will have been stored in the task's
		event list item, and they should now be retrieved then cleared. */
		uxReturn = uxTaskResetEventItemValue();

		if( ( uxReturn & eventUNBLOCKED_DUE_TO_BIT_SET ) == ( EventBits_t ) 0 )
		{
			taskENTER_CRITICAL();
			{
				/* The task timed out, just return the current event bit value. */
				uxReturn = pxEventBits->uxEventBits;

				/* It is possible that the event bits were updated between this
				task leaving the Blocked state and running again. */
				if( prvTestWaitCondition( uxReturn, uxBitsToWaitFor, xWaitForAllBits ) != pdFALSE )
				{
					if( xClearOnExit != pdFALSE )
					{
						pxEventBits->uxEventBits &= ~uxBitsToWaitFor;
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
				xTimeoutOccurred = pdTRUE;
			}
			taskEXIT_CRITICAL();
		}
		else
		{
			/* The task unblocked because the bits were set. */
		}

		/* The task blocked so control bits may have been set. */
		uxReturn &= ~eventEVENT_BITS_CONTROL_BYTES;
	}
	traceEVENT_GROUP_WAIT_BITS_END( xEventGroup, uxBitsToWaitFor, xTimeoutOccurred );

	/* Prevent compiler warnings when trace macros are not used. */
	( void ) xTimeoutOccurred;

	return uxReturn;
}
/*-----------------------------------------------------------*/

EventBits_t xEventGroupClearBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear )
{
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
EventBits_t uxReturn;

	/* Check the user is not attempting to clear the bits used by the kernel
	itself. */
	configASSERT( xEventGroup );
	configASSERT( ( uxBitsToClear & eventEVENT_BITS_CONTROL_BYTES ) == 0 );

	taskENTER_CRITICAL();
	{
		traceEVENT_GROUP_CLEAR_BITS( xEventGroup, uxBitsToClear );

		/* The value returned is the event group value prior to the bits being
		cleared. */
		uxReturn = pxEventBits->uxEventBits;

		/* Clear the bits. */
		pxEventBits->uxEventBits &= ~uxBitsToClear;
	}
	taskEXIT_CRITICAL();

	return uxReturn;
}
/*-----------------------------------------------------------*/

#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( INCLUDE_xTimerPendFunctionCall == 1 ) && ( configUSE_TIMERS == 1 ) )

	BaseType_t xEventGroupClearBitsFromISR( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear )
	{
		BaseType_t xReturn;

		traceEVENT_GROUP_CLEAR_BITS_FROM_ISR( xEventGroup, uxBitsToClear );
		xReturn = xTimerPendFunctionCallFromISR( vEventGroupClearBitsCallback, ( void * ) xEventGroup, ( uint32_t ) uxBitsToClear, NULL );

		return xReturn;
	}

#endif
/*-----------------------------------------------------------*/

EventBits_t xEventGroupGetBitsFromISR( EventGroupHandle_t xEventGroup )
{
UBaseType_t uxSavedInterruptStatus;
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
EventBits_t uxReturn;

	uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
	{
		uxReturn = pxEventBits->uxEventBits;
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

	return uxReturn;
}
/*-----------------------------------------------------------*/

EventBits_t xEventGroupSetBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet )
{
ListItem_t *pxListItem, *pxNext;
ListItem_t const *pxListEnd;
List_t *pxList;
EventBits_t uxBitsToClear = 0, uxBitsWaitedFor, uxControlBits;
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
BaseType_t xMatchFound = pdFALSE;

	/* Check the user is not attempting to set the bits used by the kernel
	itself. */
	configASSERT( xEventGroup );
	configASSERT( ( uxBitsToSet & eventEVENT_BITS_CONTROL_BYTES ) == 0 );

	pxList = &( pxEventBits->xTasksWaitingForBits );
	pxListEnd = listGET_END_MARKER( pxList ); /*lint !e826 !e740 The mini list structure is used as the list end to save RAM.  This is checked and valid. */
	vTaskSuspendAll();
	{
		traceEVENT_GROUP_SET_BITS( xEventGroup, uxBitsToSet );

		pxListItem = listGET_HEAD_ENTRY( pxList );

		/* Set the bits. */
		pxEventBits->uxEventBits |= uxBitsToSet;

		/* See if the new bit value should unblock any tasks. */
		while( pxListItem != pxListEnd )
		{
			pxNext = listGET_NEXT( pxListItem );
			uxBitsWaitedFor = listGET_LIST_ITEM_VALUE( pxListItem );
			xMatchFound = pdFALSE;

			/* Split the bits waited for from the control bits. */
			uxControlBits = uxBitsWaitedFor & eventEVENT_BITS_CONTROL_BYTES;
			uxBitsWaitedFor &= ~eventEVENT_BITS_CONTROL_BYTES;

			if( ( uxControlBits & eventWAIT_FOR_ALL_BITS ) == ( EventBits_t ) 0 )
			{
				/* Just looking for single bit being set. */
				if( ( uxBitsWaitedFor & pxEventBits->uxEventBits ) != ( EventBits_t ) 0 )
				{
					xMatchFound = pdTRUE;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else if( ( uxBitsWaitedFor & pxEventBits->uxEventBits ) == uxBitsWaitedFor )
			{
				/* All bits are set. */
				xMatchFound = pdTRUE;
			}
			else
			{
				/* Need all bits to be set, but not all the bits were set. */
			}

			if( xMatchFound != pdFALSE )
			{
				/* The bits match.  Should the bits be cleared on exit? */
				if( ( uxControlBits & eventCLEAR_EVENTS_ON_EXIT_BIT ) != ( EventBits_t ) 0 )
				{
					uxBitsToClear |= uxBitsWaitedFor;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}

				/* Store the actual event flag value in the task's event list
				item before removing the task from the event list.  The
				eventUNBLOCKED_DUE_TO_BIT_SET bit is set so the task knows
				that is was unblocked due to its required bits matching, rather
				than because it timed out. */
				vTaskRemoveFromUnorderedEventList( pxListItem, pxEventBits->uxEventBits | eventUNBLOCKED_DUE_TO_BIT_SET );
			}

			/* Move onto the next list item.  Note pxListItem->pxNext is not
			used here as the list item may have been removed from the event list
			and inserted into the ready/pending reading list. */
			pxListItem = pxNext;
		}

		/* Clear any bits that matched when the eventCLEAR_EVENTS_ON_EXIT_BIT
		bit was set in the control word. */
		pxEventBits->uxEventBits &= ~uxBitsToClear;
	}
	( void ) xTaskResumeAll();

	return pxEventBits->uxEventBits;
}
/*-----------------------------------------------------------*/

void vEventGroupDelete( EventGroupHandle_t xEventGroup )
{
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
const List_t *pxTasksWaitingForBits = &( pxEventBits->xTasksWaitingForBits );

	vTaskSuspendAll();
	{
		traceEVENT_GROUP_DELETE( xEventGroup );

		while( listCURRENT_LIST_LENGTH( pxTasksWaitingForBits ) > ( UBaseType_t ) 0 )
		{
			/* Unblock the task, returning 0 as the event list is being deleted
			and cannot therefore have any bits set. */
			configASSERT( pxTasksWaitingForBits->xListEnd.pxNext != ( const ListItem_t * ) &( pxTasksWaitingForBits->xListEnd ) );
			vTaskRemoveFromUnorderedEventList( pxTasksWaitingForBits->xListEnd.pxNext, eventUNBLOCKED_DUE_TO_BIT_SET );
		}

		#if( ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 0 ) )
		{
			/* The event group can only have been allocated dynamically - free
			it again. */
			vPortFree( pxEventBits );
		}
		#elif( ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 1 ) )
		{
			/* The event group could have been allocated statically or
			dynamically, so check before attempting to free the memory. */
			if( pxEventBits->ucStaticallyAllocated == ( uint8_t ) pdFALSE )
			{
				vPortFree( pxEventBits );
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
	}
	( void ) xTaskResumeAll();
}
/*-----------------------------------------------------------*/

/* For internal use only - execute a 'set bits' command that was pended from
an interrupt. */
void vEventGroupSetBitsCallback( void *pvEventGroup, const uint32_t ulBitsToSet )
{
	( void ) xEventGroupSetBits( pvEventGroup, ( EventBits_t ) ulBitsToSet );
}
/*-----------------------------------------------------------*/

/* For internal use only - execute a 'clear bits' command that was pended from
an interrupt. */
void vEventGroupClearBitsCallback( void *pvEventGroup, const uint32_t ulBitsToClear )
{
	( void ) xEventGroupClearBits( pvEventGroup, ( EventBits_t ) ulBitsToClear );
}
/*-----------------------------------------------------------*/

static BaseType_t prvTestWaitCondition( const EventBits_t uxCurrentEventBits, const EventBits_t uxBitsToWaitFor, const BaseType_t xWaitForAllBits )
{
BaseType_t xWaitConditionMet = pdFALSE;

	if( xWaitForAllBits == pdFALSE )
	{
		/* Task only has to wait for one bit within uxBitsToWaitFor to be
		set.  Is one already set? */
		if( ( uxCurrentEventBits & uxBitsToWaitFor ) != ( EventBits_t ) 0 )
		{
			xWaitConditionMet = pdTRUE;
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	else
	{
		/* Task has to wait for all the bits in uxBitsToWaitFor to be set.
		Are they set already? */
		if( ( uxCurrentEventBits & uxBitsToWaitFor ) == uxBitsToWaitFor )
		{
			xWaitConditionMet = pdTRUE;
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}

	return xWaitConditionMet;
}
/*-----------------------------------------------------------*/

#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( INCLUDE_xTimerPendFunctionCall == 1 ) && ( configUSE_TIMERS == 1 ) )

	BaseType_t xEventGroupSetBitsFromISR( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet, BaseType_t *pxHigherPriorityTaskWoken )
	{
	BaseType_t xReturn;

		traceEVENT_GROUP_SET_BITS_FROM_ISR( xEventGroup, uxBitsToSet );
		xReturn = xTimerPendFunctionCallFromISR( vEventGroupSetBitsCallback, ( void * ) xEventGroup, ( uint32_t ) uxBitsToSet, pxHigherPriorityTaskWoken );

		return xReturn;
	}

#endif
/*-----------------------------------------------------------*/

#if (configUSE_TRACE_FACILITY == 1)

	UBaseType_t uxEventGroupGetNumber( void* xEventGroup )
	{
	UBaseType_t xReturn;
	EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;

		if( xEventGroup == NULL )
		{
			xReturn = 0;
		}
		else
		{
			xReturn = pxEventBits->uxEventGroupNumber;
		}

		return xReturn;
	}

#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if ( configUSE_TRACE_FACILITY == 1 )

	void vEventGroupSetNumber( void * xEventGroup, UBaseType_t uxEventGroupNumber )
	{
		( ( EventGroup_t * ) xEventGroup )->uxEventGroupNumber = uxEventGroupNumber;
	}

#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/


