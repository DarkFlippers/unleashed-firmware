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
#include <stdint.h>
#include <string.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#if( configUSE_TASK_NOTIFICATIONS != 1 )
	#error configUSE_TASK_NOTIFICATIONS must be set to 1 to build stream_buffer.c
#endif

/* Lint e961 and e750 are suppressed as a MISRA exception justified because the
MPU ports require MPU_WRAPPERS_INCLUDED_FROM_API_FILE to be defined for the
header files above, but not in this file, in order to generate the correct
privileged Vs unprivileged linkage and placement. */
#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE /*lint !e961 !e750. */

/* If the user has not provided application specific Rx notification macros,
or #defined the notification macros away, them provide default implementations
that uses task notifications. */
/*lint -save -e9026 Function like macros allowed and needed here so they can be overidden. */
#ifndef sbRECEIVE_COMPLETED
	#define sbRECEIVE_COMPLETED( pxStreamBuffer )										\
		vTaskSuspendAll();																\
		{																				\
			if( ( pxStreamBuffer )->xTaskWaitingToSend != NULL )						\
			{																			\
				( void ) xTaskNotify( ( pxStreamBuffer )->xTaskWaitingToSend,			\
									  ( uint32_t ) 0,									\
									  eNoAction );										\
				( pxStreamBuffer )->xTaskWaitingToSend = NULL;							\
			}																			\
		}																				\
		( void ) xTaskResumeAll();
#endif /* sbRECEIVE_COMPLETED */

#ifndef sbRECEIVE_COMPLETED_FROM_ISR
	#define sbRECEIVE_COMPLETED_FROM_ISR( pxStreamBuffer,								\
										  pxHigherPriorityTaskWoken )					\
	{																					\
	UBaseType_t uxSavedInterruptStatus;													\
																						\
		uxSavedInterruptStatus = ( UBaseType_t ) portSET_INTERRUPT_MASK_FROM_ISR();		\
		{																				\
			if( ( pxStreamBuffer )->xTaskWaitingToSend != NULL )						\
			{																			\
				( void ) xTaskNotifyFromISR( ( pxStreamBuffer )->xTaskWaitingToSend,	\
											 ( uint32_t ) 0,							\
											 eNoAction,									\
											 pxHigherPriorityTaskWoken );				\
				( pxStreamBuffer )->xTaskWaitingToSend = NULL;							\
			}																			\
		}																				\
		portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );					\
	}
#endif /* sbRECEIVE_COMPLETED_FROM_ISR */

/* If the user has not provided an application specific Tx notification macro,
or #defined the notification macro away, them provide a default implementation
that uses task notifications. */
#ifndef sbSEND_COMPLETED
	#define sbSEND_COMPLETED( pxStreamBuffer )											\
		vTaskSuspendAll();																\
		{																				\
			if( ( pxStreamBuffer )->xTaskWaitingToReceive != NULL )						\
			{																			\
				( void ) xTaskNotify( ( pxStreamBuffer )->xTaskWaitingToReceive,		\
									  ( uint32_t ) 0,									\
									  eNoAction );										\
				( pxStreamBuffer )->xTaskWaitingToReceive = NULL;						\
			}																			\
		}																				\
		( void ) xTaskResumeAll();
#endif /* sbSEND_COMPLETED */

#ifndef sbSEND_COMPLETE_FROM_ISR
	#define sbSEND_COMPLETE_FROM_ISR( pxStreamBuffer, pxHigherPriorityTaskWoken )		\
	{																					\
	UBaseType_t uxSavedInterruptStatus;													\
																						\
		uxSavedInterruptStatus = ( UBaseType_t ) portSET_INTERRUPT_MASK_FROM_ISR();		\
		{																				\
			if( ( pxStreamBuffer )->xTaskWaitingToReceive != NULL )						\
			{																			\
				( void ) xTaskNotifyFromISR( ( pxStreamBuffer )->xTaskWaitingToReceive,	\
											 ( uint32_t ) 0,							\
											 eNoAction,									\
											 pxHigherPriorityTaskWoken );				\
				( pxStreamBuffer )->xTaskWaitingToReceive = NULL;						\
			}																			\
		}																				\
		portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );					\
	}
#endif /* sbSEND_COMPLETE_FROM_ISR */
/*lint -restore (9026) */

/* The number of bytes used to hold the length of a message in the buffer. */
#define sbBYTES_TO_STORE_MESSAGE_LENGTH ( sizeof( size_t ) )

/* Bits stored in the ucFlags field of the stream buffer. */
#define sbFLAGS_IS_MESSAGE_BUFFER		( ( uint8_t ) 1 ) /* Set if the stream buffer was created as a message buffer, in which case it holds discrete messages rather than a stream. */
#define sbFLAGS_IS_STATICALLY_ALLOCATED ( ( uint8_t ) 2 ) /* Set if the stream buffer was created using statically allocated memory. */

/*-----------------------------------------------------------*/

/* Structure that hold state information on the buffer. */
typedef struct xSTREAM_BUFFER /*lint !e9058 Style convention uses tag. */
{
	volatile size_t xTail;				/* Index to the next item to read within the buffer. */
	volatile size_t xHead;				/* Index to the next item to write within the buffer. */
	size_t xLength;						/* The length of the buffer pointed to by pucBuffer. */
	size_t xTriggerLevelBytes;			/* The number of bytes that must be in the stream buffer before a task that is waiting for data is unblocked. */
	volatile TaskHandle_t xTaskWaitingToReceive; /* Holds the handle of a task waiting for data, or NULL if no tasks are waiting. */
	volatile TaskHandle_t xTaskWaitingToSend;	/* Holds the handle of a task waiting to send data to a message buffer that is full. */
	uint8_t *pucBuffer;					/* Points to the buffer itself - that is - the RAM that stores the data passed through the buffer. */
	uint8_t ucFlags;

	#if ( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t uxStreamBufferNumber;		/* Used for tracing purposes. */
	#endif
} StreamBuffer_t;

/*
 * The number of bytes available to be read from the buffer.
 */
static size_t prvBytesInBuffer( const StreamBuffer_t * const pxStreamBuffer ) PRIVILEGED_FUNCTION;

/*
 * Add xCount bytes from pucData into the pxStreamBuffer message buffer.
 * Returns the number of bytes written, which will either equal xCount in the
 * success case, or 0 if there was not enough space in the buffer (in which case
 * no data is written into the buffer).
 */
static size_t prvWriteBytesToBuffer( StreamBuffer_t * const pxStreamBuffer, const uint8_t *pucData, size_t xCount ) PRIVILEGED_FUNCTION;

/*
 * If the stream buffer is being used as a message buffer, then reads an entire
 * message out of the buffer.  If the stream buffer is being used as a stream
 * buffer then read as many bytes as possible from the buffer.
 * prvReadBytesFromBuffer() is called to actually extract the bytes from the
 * buffer's data storage area.
 */
static size_t prvReadMessageFromBuffer( StreamBuffer_t *pxStreamBuffer,
										void *pvRxData,
										size_t xBufferLengthBytes,
										size_t xBytesAvailable,
										size_t xBytesToStoreMessageLength ) PRIVILEGED_FUNCTION;

/*
 * If the stream buffer is being used as a message buffer, then writes an entire
 * message to the buffer.  If the stream buffer is being used as a stream
 * buffer then write as many bytes as possible to the buffer.
 * prvWriteBytestoBuffer() is called to actually send the bytes to the buffer's
 * data storage area.
 */
static size_t prvWriteMessageToBuffer(  StreamBuffer_t * const pxStreamBuffer,
										const void * pvTxData,
										size_t xDataLengthBytes,
										size_t xSpace,
										size_t xRequiredSpace ) PRIVILEGED_FUNCTION;

/*
 * Read xMaxCount bytes from the pxStreamBuffer message buffer and write them
 * to pucData.
 */
static size_t prvReadBytesFromBuffer( StreamBuffer_t *pxStreamBuffer,
									  uint8_t *pucData,
									  size_t xMaxCount,
									  size_t xBytesAvailable ); PRIVILEGED_FUNCTION

/*
 * Called by both pxStreamBufferCreate() and pxStreamBufferCreateStatic() to
 * initialise the members of the newly created stream buffer structure.
 */
static void prvInitialiseNewStreamBuffer( StreamBuffer_t * const pxStreamBuffer,
										  uint8_t * const pucBuffer,
										  size_t xBufferSizeBytes,
										  size_t xTriggerLevelBytes,
										  BaseType_t xIsMessageBuffer ) PRIVILEGED_FUNCTION;

/*-----------------------------------------------------------*/

#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )

	StreamBufferHandle_t xStreamBufferGenericCreate( size_t xBufferSizeBytes, size_t xTriggerLevelBytes, BaseType_t xIsMessageBuffer )
	{
	uint8_t *pucAllocatedMemory;

		/* In case the stream buffer is going to be used as a message buffer
		(that is, it will hold discrete messages with a little meta data that
		says how big the next message is) check the buffer will be large enough
		to hold at least one message. */
		configASSERT( xBufferSizeBytes > sbBYTES_TO_STORE_MESSAGE_LENGTH );
		configASSERT( xTriggerLevelBytes <= xBufferSizeBytes );

		/* A trigger level of 0 would cause a waiting task to unblock even when
		the buffer was empty. */
		if( xTriggerLevelBytes == ( size_t ) 0 )
		{
			xTriggerLevelBytes = ( size_t ) 1; /*lint !e9044 Parameter modified to ensure it doesn't have a dangerous value. */
		}

		/* A stream buffer requires a StreamBuffer_t structure and a buffer.
		Both are allocated in a single call to pvPortMalloc().  The
		StreamBuffer_t structure is placed at the start of the allocated memory
		and the buffer follows immediately after.  The requested size is
		incremented so the free space is returned as the user would expect -
		this is a quirk of the implementation that means otherwise the free
		space would be reported as one byte smaller than would be logically
		expected. */
		xBufferSizeBytes++;
		pucAllocatedMemory = ( uint8_t * ) pvPortMalloc( xBufferSizeBytes + sizeof( StreamBuffer_t ) ); /*lint !e9079 malloc() only returns void*. */

		if( pucAllocatedMemory != NULL )
		{
			prvInitialiseNewStreamBuffer( ( StreamBuffer_t * ) pucAllocatedMemory, /* Structure at the start of the allocated memory. */ /*lint !e9087 Safe cast as allocated memory is aligned. */ /*lint !e826 Area is not too small and alignment is guaranteed provided malloc() behaves as expected and returns aligned buffer. */
										   pucAllocatedMemory + sizeof( StreamBuffer_t ),  /* Storage area follows. */ /*lint !e9016 Indexing past structure valid for uint8_t pointer, also storage area has no alignment requirement. */
										   xBufferSizeBytes,
										   xTriggerLevelBytes,
										   xIsMessageBuffer );

			traceSTREAM_BUFFER_CREATE( ( ( StreamBuffer_t * ) pucAllocatedMemory ), xIsMessageBuffer );
		}
		else
		{
			traceSTREAM_BUFFER_CREATE_FAILED( xIsMessageBuffer );
		}

		return ( StreamBufferHandle_t * ) pucAllocatedMemory; /*lint !e9087 !e826 Safe cast as allocated memory is aligned. */
	}

#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
/*-----------------------------------------------------------*/

#if( configSUPPORT_STATIC_ALLOCATION == 1 )

	StreamBufferHandle_t xStreamBufferGenericCreateStatic( size_t xBufferSizeBytes,
														   size_t xTriggerLevelBytes,
														   BaseType_t xIsMessageBuffer,
														   uint8_t * const pucStreamBufferStorageArea,
														   StaticStreamBuffer_t * const pxStaticStreamBuffer )
	{
	StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) pxStaticStreamBuffer; /*lint !e740 !e9087 Safe cast as StaticStreamBuffer_t is opaque Streambuffer_t. */
	StreamBufferHandle_t xReturn;

		configASSERT( pucStreamBufferStorageArea );
		configASSERT( pxStaticStreamBuffer );
		configASSERT( xTriggerLevelBytes <= xBufferSizeBytes );

		/* A trigger level of 0 would cause a waiting task to unblock even when
		the buffer was empty. */
		if( xTriggerLevelBytes == ( size_t ) 0 )
		{
			xTriggerLevelBytes = ( size_t ) 1; /*lint !e9044 Function parameter deliberately modified to ensure it is in range. */
		}

		/* In case the stream buffer is going to be used as a message buffer
		(that is, it will hold discrete messages with a little meta data that
		says how big the next message is) check the buffer will be large enough
		to hold at least one message. */
		configASSERT( xBufferSizeBytes > sbBYTES_TO_STORE_MESSAGE_LENGTH );

		#if( configASSERT_DEFINED == 1 )
		{
			/* Sanity check that the size of the structure used to declare a
			variable of type StaticStreamBuffer_t equals the size of the real
			message buffer structure. */
			volatile size_t xSize = sizeof( StaticStreamBuffer_t );
			configASSERT( xSize == sizeof( StreamBuffer_t ) );
		}
		#endif /* configASSERT_DEFINED */

		if( ( pucStreamBufferStorageArea != NULL ) && ( pxStaticStreamBuffer != NULL ) )
		{
			prvInitialiseNewStreamBuffer( pxStreamBuffer,
										  pucStreamBufferStorageArea,
										  xBufferSizeBytes,
										  xTriggerLevelBytes,
										  xIsMessageBuffer );

			/* Remember this was statically allocated in case it is ever deleted
			again. */
			pxStreamBuffer->ucFlags |= sbFLAGS_IS_STATICALLY_ALLOCATED;

			traceSTREAM_BUFFER_CREATE( pxStreamBuffer, xIsMessageBuffer );

			xReturn = ( StreamBufferHandle_t ) pxStaticStreamBuffer; /*lint !e9087 Data hiding requires cast to opaque type. */
		}
		else
		{
			xReturn = NULL;
			traceSTREAM_BUFFER_CREATE_STATIC_FAILED( xReturn, xIsMessageBuffer );
		}

		return xReturn;
	}

#endif /* ( configSUPPORT_STATIC_ALLOCATION == 1 ) */
/*-----------------------------------------------------------*/

void vStreamBufferDelete( StreamBufferHandle_t xStreamBuffer )
{
StreamBuffer_t * pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */

	configASSERT( pxStreamBuffer );

	traceSTREAM_BUFFER_DELETE( xStreamBuffer );

	if( ( pxStreamBuffer->ucFlags & sbFLAGS_IS_STATICALLY_ALLOCATED ) == ( uint8_t ) pdFALSE )
	{
		#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
		{
			/* Both the structure and the buffer were allocated using a single call
			to pvPortMalloc(), hence only one call to vPortFree() is required. */
			vPortFree( ( void * ) pxStreamBuffer ); /*lint !e9087 Standard free() semantics require void *, plus pxStreamBuffer was allocated by pvPortMalloc(). */
		}
		#else
		{
			/* Should not be possible to get here, ucFlags must be corrupt.
			Force an assert. */
			configASSERT( xStreamBuffer == ( StreamBufferHandle_t ) ~0 );
		}
		#endif
	}
	else
	{
		/* The structure and buffer were not allocated dynamically and cannot be
		freed - just scrub the structure so future use will assert. */
		memset( pxStreamBuffer, 0x00, sizeof( StreamBuffer_t ) );
	}
}
/*-----------------------------------------------------------*/

BaseType_t xStreamBufferReset( StreamBufferHandle_t xStreamBuffer )
{
StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */
BaseType_t xReturn = pdFAIL, xIsMessageBuffer;

#if( configUSE_TRACE_FACILITY == 1 )
	UBaseType_t uxStreamBufferNumber;
#endif

	configASSERT( pxStreamBuffer );

	#if( configUSE_TRACE_FACILITY == 1 )
	{
		/* Store the stream buffer number so it can be restored after the
		reset. */
		uxStreamBufferNumber = pxStreamBuffer->uxStreamBufferNumber;
	}
	#endif

	/* Can only reset a message buffer if there are no tasks blocked on it. */
	if( pxStreamBuffer->xTaskWaitingToReceive == NULL )
	{
		if( pxStreamBuffer->xTaskWaitingToSend == NULL )
		{
			if( ( pxStreamBuffer->ucFlags & sbFLAGS_IS_MESSAGE_BUFFER ) != ( uint8_t ) 0 )
			{
				xIsMessageBuffer = pdTRUE;
			}
			else
			{
				xIsMessageBuffer = pdFALSE;
			}

			prvInitialiseNewStreamBuffer( pxStreamBuffer,
										  pxStreamBuffer->pucBuffer,
										  pxStreamBuffer->xLength,
										  pxStreamBuffer->xTriggerLevelBytes,
										  xIsMessageBuffer );
			xReturn = pdPASS;

			#if( configUSE_TRACE_FACILITY == 1 )
			{
				pxStreamBuffer->uxStreamBufferNumber = uxStreamBufferNumber;
			}
			#endif

			traceSTREAM_BUFFER_RESET( xStreamBuffer );
		}
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xStreamBufferSetTriggerLevel( StreamBufferHandle_t xStreamBuffer, size_t xTriggerLevel )
{
StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */
BaseType_t xReturn;

	configASSERT( pxStreamBuffer );

	/* It is not valid for the trigger level to be 0. */
	if( xTriggerLevel == ( size_t ) 0 )
	{
		xTriggerLevel = ( size_t ) 1; /*lint !e9044 Parameter modified to ensure it doesn't have a dangerous value. */
	}

	/* The trigger level is the number of bytes that must be in the stream
	buffer before a task that is waiting for data is unblocked. */
	if( xTriggerLevel <= pxStreamBuffer->xLength )
	{
		pxStreamBuffer->xTriggerLevelBytes = xTriggerLevel;
		xReturn = pdPASS;
	}
	else
	{
		xReturn = pdFALSE;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

size_t xStreamBufferSpacesAvailable( StreamBufferHandle_t xStreamBuffer )
{
const StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */
size_t xSpace;

	configASSERT( pxStreamBuffer );

	xSpace = pxStreamBuffer->xLength + pxStreamBuffer->xTail;
	xSpace -= pxStreamBuffer->xHead;
	xSpace -= ( size_t ) 1;

	if( xSpace >= pxStreamBuffer->xLength )
	{
		xSpace -= pxStreamBuffer->xLength;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	return xSpace;
}
/*-----------------------------------------------------------*/

size_t xStreamBufferBytesAvailable( StreamBufferHandle_t xStreamBuffer )
{
const StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */
size_t xReturn;

	configASSERT( pxStreamBuffer );

	xReturn = prvBytesInBuffer( pxStreamBuffer );
	return xReturn;
}
/*-----------------------------------------------------------*/

size_t xStreamBufferSend( StreamBufferHandle_t xStreamBuffer,
						  const void *pvTxData,
						  size_t xDataLengthBytes,
						  TickType_t xTicksToWait )
{
StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */
size_t xReturn, xSpace = 0;
size_t xRequiredSpace = xDataLengthBytes;
TimeOut_t xTimeOut;

	configASSERT( pvTxData );
	configASSERT( pxStreamBuffer );

	/* This send function is used to write to both message buffers and stream
	buffers.  If this is a message buffer then the space needed must be
	increased by the amount of bytes needed to store the length of the
	message. */
	if( ( pxStreamBuffer->ucFlags & sbFLAGS_IS_MESSAGE_BUFFER ) != ( uint8_t ) 0 )
	{
		xRequiredSpace += sbBYTES_TO_STORE_MESSAGE_LENGTH;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	if( xTicksToWait != ( TickType_t ) 0 )
	{
		vTaskSetTimeOutState( &xTimeOut );

		do
		{
			/* Wait until the required number of bytes are free in the message
			buffer. */
			taskENTER_CRITICAL();
			{
				xSpace = xStreamBufferSpacesAvailable( pxStreamBuffer );

				if( xSpace < xRequiredSpace )
				{
					/* Clear notification state as going to wait for space. */
					( void ) xTaskNotifyStateClear( NULL );

					/* Should only be one writer. */
					configASSERT( pxStreamBuffer->xTaskWaitingToSend == NULL );
					pxStreamBuffer->xTaskWaitingToSend = xTaskGetCurrentTaskHandle();
				}
				else
				{
					taskEXIT_CRITICAL();
					break;
				}
			}
			taskEXIT_CRITICAL();

			traceBLOCKING_ON_STREAM_BUFFER_SEND( xStreamBuffer );
			( void ) xTaskNotifyWait( ( uint32_t ) 0, UINT32_MAX, NULL, xTicksToWait );
			pxStreamBuffer->xTaskWaitingToSend = NULL;

		} while( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) == pdFALSE );
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	if( xSpace == ( size_t ) 0 )
	{
		xSpace = xStreamBufferSpacesAvailable( pxStreamBuffer );
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	xReturn = prvWriteMessageToBuffer( pxStreamBuffer, pvTxData, xDataLengthBytes, xSpace, xRequiredSpace );

	if( xReturn > ( size_t ) 0 )
	{
		traceSTREAM_BUFFER_SEND( xStreamBuffer, xReturn );

		/* Was a task waiting for the data? */
		if( prvBytesInBuffer( pxStreamBuffer ) >= pxStreamBuffer->xTriggerLevelBytes )
		{
			sbSEND_COMPLETED( pxStreamBuffer );
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
		traceSTREAM_BUFFER_SEND_FAILED( xStreamBuffer );
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

size_t xStreamBufferSendFromISR( StreamBufferHandle_t xStreamBuffer,
								 const void *pvTxData,
								 size_t xDataLengthBytes,
								 BaseType_t * const pxHigherPriorityTaskWoken )
{
StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */
size_t xReturn, xSpace;
size_t xRequiredSpace = xDataLengthBytes;

	configASSERT( pvTxData );
	configASSERT( pxStreamBuffer );

	/* This send function is used to write to both message buffers and stream
	buffers.  If this is a message buffer then the space needed must be
	increased by the amount of bytes needed to store the length of the
	message. */
	if( ( pxStreamBuffer->ucFlags & sbFLAGS_IS_MESSAGE_BUFFER ) != ( uint8_t ) 0 )
	{
		xRequiredSpace += sbBYTES_TO_STORE_MESSAGE_LENGTH;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	xSpace = xStreamBufferSpacesAvailable( pxStreamBuffer );
	xReturn = prvWriteMessageToBuffer( pxStreamBuffer, pvTxData, xDataLengthBytes, xSpace, xRequiredSpace );

	if( xReturn > ( size_t ) 0 )
	{
		/* Was a task waiting for the data? */
		if( prvBytesInBuffer( pxStreamBuffer ) >= pxStreamBuffer->xTriggerLevelBytes )
		{
			sbSEND_COMPLETE_FROM_ISR( pxStreamBuffer, pxHigherPriorityTaskWoken );
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

	traceSTREAM_BUFFER_SEND_FROM_ISR( xStreamBuffer, xReturn );

	return xReturn;
}
/*-----------------------------------------------------------*/

static size_t prvWriteMessageToBuffer( StreamBuffer_t * const pxStreamBuffer,
									   const void * pvTxData,
									   size_t xDataLengthBytes,
									   size_t xSpace,
									   size_t xRequiredSpace )
{
	BaseType_t xShouldWrite;
	size_t xReturn;

	if( xSpace == ( size_t ) 0 )
	{
		/* Doesn't matter if this is a stream buffer or a message buffer, there
		is no space to write. */
		xShouldWrite = pdFALSE;
	}
	else if( ( pxStreamBuffer->ucFlags & sbFLAGS_IS_MESSAGE_BUFFER ) == ( uint8_t ) 0 )
	{
		/* This is a stream buffer, as opposed to a message buffer, so writing a
		stream of bytes rather than discrete messages.  Write as many bytes as
		possible. */
		xShouldWrite = pdTRUE;
		xDataLengthBytes = configMIN( xDataLengthBytes, xSpace ); /*lint !e9044 Function parameter modified to ensure it is capped to available space. */
	}
	else if( xSpace >= xRequiredSpace )
	{
		/* This is a message buffer, as opposed to a stream buffer, and there
		is enough space to write both the message length and the message itself
		into the buffer.  Start by writing the length of the data, the data
		itself will be written later in this function. */
		xShouldWrite = pdTRUE;
		( void ) prvWriteBytesToBuffer( pxStreamBuffer, ( const uint8_t * ) &( xDataLengthBytes ), sbBYTES_TO_STORE_MESSAGE_LENGTH );
	}
	else
	{
		/* There is space available, but not enough space. */
		xShouldWrite = pdFALSE;
	}

	if( xShouldWrite != pdFALSE )
	{
		/* Writes the data itself. */
		xReturn = prvWriteBytesToBuffer( pxStreamBuffer, ( const uint8_t * ) pvTxData, xDataLengthBytes ); /*lint !e9079 Storage buffer is implemented as uint8_t for ease of sizing, alighment and access. */
	}
	else
	{
		xReturn = 0;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

size_t xStreamBufferReceive( StreamBufferHandle_t xStreamBuffer,
							 void *pvRxData,
							 size_t xBufferLengthBytes,
							 TickType_t xTicksToWait )
{
StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */
size_t xReceivedLength = 0, xBytesAvailable, xBytesToStoreMessageLength;

	configASSERT( pvRxData );
	configASSERT( pxStreamBuffer );

	/* This receive function is used by both message buffers, which store
	discrete messages, and stream buffers, which store a continuous stream of
	bytes.  Discrete messages include an additional
	sbBYTES_TO_STORE_MESSAGE_LENGTH bytes that hold the length of the
	message. */
	if( ( pxStreamBuffer->ucFlags & sbFLAGS_IS_MESSAGE_BUFFER ) != ( uint8_t ) 0 )
	{
		xBytesToStoreMessageLength = sbBYTES_TO_STORE_MESSAGE_LENGTH;
	}
	else
	{
		xBytesToStoreMessageLength = 0;
	}

	if( xTicksToWait != ( TickType_t ) 0 )
	{
		/* Checking if there is data and clearing the notification state must be
		performed atomically. */
		taskENTER_CRITICAL();
		{
			xBytesAvailable = prvBytesInBuffer( pxStreamBuffer );

			/* If this function was invoked by a message buffer read then
			xBytesToStoreMessageLength holds the number of bytes used to hold
			the length of the next discrete message.  If this function was
			invoked by a stream buffer read then xBytesToStoreMessageLength will
			be 0. */
			if( xBytesAvailable <= xBytesToStoreMessageLength )
			{
				/* Clear notification state as going to wait for data. */
				( void ) xTaskNotifyStateClear( NULL );

				/* Should only be one reader. */
				configASSERT( pxStreamBuffer->xTaskWaitingToReceive == NULL );
				pxStreamBuffer->xTaskWaitingToReceive = xTaskGetCurrentTaskHandle();
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		taskEXIT_CRITICAL();

		if( xBytesAvailable <= xBytesToStoreMessageLength )
		{
			/* Wait for data to be available. */
			traceBLOCKING_ON_STREAM_BUFFER_RECEIVE( xStreamBuffer );
			( void ) xTaskNotifyWait( ( uint32_t ) 0, UINT32_MAX, NULL, xTicksToWait );
			pxStreamBuffer->xTaskWaitingToReceive = NULL;

			/* Recheck the data available after blocking. */
			xBytesAvailable = prvBytesInBuffer( pxStreamBuffer );
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	else
	{
		xBytesAvailable = prvBytesInBuffer( pxStreamBuffer );
	}

	/* Whether receiving a discrete message (where xBytesToStoreMessageLength
	holds the number of bytes used to store the message length) or a stream of
	bytes (where xBytesToStoreMessageLength is zero), the number of bytes
	available must be greater than xBytesToStoreMessageLength to be able to
	read bytes from the buffer. */
	if( xBytesAvailable > xBytesToStoreMessageLength )
	{
		xReceivedLength = prvReadMessageFromBuffer( pxStreamBuffer, pvRxData, xBufferLengthBytes, xBytesAvailable, xBytesToStoreMessageLength );

		/* Was a task waiting for space in the buffer? */
		if( xReceivedLength != ( size_t ) 0 )
		{
			traceSTREAM_BUFFER_RECEIVE( xStreamBuffer, xReceivedLength );
			sbRECEIVE_COMPLETED( pxStreamBuffer );
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	else
	{
		traceSTREAM_BUFFER_RECEIVE_FAILED( xStreamBuffer );
		mtCOVERAGE_TEST_MARKER();
	}

	return xReceivedLength;
}
/*-----------------------------------------------------------*/

size_t xStreamBufferReceiveFromISR( StreamBufferHandle_t xStreamBuffer,
									void *pvRxData,
									size_t xBufferLengthBytes,
									BaseType_t * const pxHigherPriorityTaskWoken )
{
StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */
size_t xReceivedLength = 0, xBytesAvailable, xBytesToStoreMessageLength;

	configASSERT( pvRxData );
	configASSERT( pxStreamBuffer );

	/* This receive function is used by both message buffers, which store
	discrete messages, and stream buffers, which store a continuous stream of
	bytes.  Discrete messages include an additional
	sbBYTES_TO_STORE_MESSAGE_LENGTH bytes that hold the length of the
	message. */
	if( ( pxStreamBuffer->ucFlags & sbFLAGS_IS_MESSAGE_BUFFER ) != ( uint8_t ) 0 )
	{
		xBytesToStoreMessageLength = sbBYTES_TO_STORE_MESSAGE_LENGTH;
	}
	else
	{
		xBytesToStoreMessageLength = 0;
	}

	xBytesAvailable = prvBytesInBuffer( pxStreamBuffer );

	/* Whether receiving a discrete message (where xBytesToStoreMessageLength
	holds the number of bytes used to store the message length) or a stream of
	bytes (where xBytesToStoreMessageLength is zero), the number of bytes
	available must be greater than xBytesToStoreMessageLength to be able to
	read bytes from the buffer. */
	if( xBytesAvailable > xBytesToStoreMessageLength )
	{
		xReceivedLength = prvReadMessageFromBuffer( pxStreamBuffer, pvRxData, xBufferLengthBytes, xBytesAvailable, xBytesToStoreMessageLength );

		/* Was a task waiting for space in the buffer? */
		if( xReceivedLength != ( size_t ) 0 )
		{
			sbRECEIVE_COMPLETED_FROM_ISR( pxStreamBuffer, pxHigherPriorityTaskWoken );
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

	traceSTREAM_BUFFER_RECEIVE_FROM_ISR( xStreamBuffer, xReceivedLength );

	return xReceivedLength;
}
/*-----------------------------------------------------------*/

static size_t prvReadMessageFromBuffer( StreamBuffer_t *pxStreamBuffer,
										void *pvRxData,
										size_t xBufferLengthBytes,
										size_t xBytesAvailable,
										size_t xBytesToStoreMessageLength )
{
size_t xOriginalTail, xReceivedLength, xNextMessageLength;

	if( xBytesToStoreMessageLength != ( size_t ) 0 )
	{
		/* A discrete message is being received.  First receive the length
		of the message.  A copy of the tail is stored so the buffer can be
		returned to its prior state if the length of the message is too
		large for the provided buffer. */
		xOriginalTail = pxStreamBuffer->xTail;
		( void ) prvReadBytesFromBuffer( pxStreamBuffer, ( uint8_t * ) &xNextMessageLength, xBytesToStoreMessageLength, xBytesAvailable );

		/* Reduce the number of bytes available by the number of bytes just
		read out. */
		xBytesAvailable -= xBytesToStoreMessageLength;

		/* Check there is enough space in the buffer provided by the
		user. */
		if( xNextMessageLength > xBufferLengthBytes )
		{
			/* The user has provided insufficient space to read the message
			so return the buffer to its previous state (so the length of
			the message is in the buffer again). */
			pxStreamBuffer->xTail = xOriginalTail;
			xNextMessageLength = 0;
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	else
	{
		/* A stream of bytes is being received (as opposed to a discrete
		message), so read as many bytes as possible. */
		xNextMessageLength = xBufferLengthBytes;
	}

	/* Read the actual data. */
	xReceivedLength = prvReadBytesFromBuffer( pxStreamBuffer, ( uint8_t * ) pvRxData, xNextMessageLength, xBytesAvailable ); /*lint !e9079 Data storage area is implemented as uint8_t array for ease of sizing, indexing and alignment. */

	return xReceivedLength;
}
/*-----------------------------------------------------------*/

BaseType_t xStreamBufferIsEmpty( StreamBufferHandle_t xStreamBuffer )
{
const StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */
BaseType_t xReturn;
size_t xTail;

	configASSERT( pxStreamBuffer );

	/* True if no bytes are available. */
	xTail = pxStreamBuffer->xTail;
	if( pxStreamBuffer->xHead == xTail )
	{
		xReturn = pdTRUE;
	}
	else
	{
		xReturn = pdFALSE;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xStreamBufferIsFull( StreamBufferHandle_t xStreamBuffer )
{
BaseType_t xReturn;
size_t xBytesToStoreMessageLength;
const StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */

	configASSERT( pxStreamBuffer );

	/* This generic version of the receive function is used by both message
	buffers, which store discrete messages, and stream buffers, which store a
	continuous stream of bytes.  Discrete messages include an additional
	sbBYTES_TO_STORE_MESSAGE_LENGTH bytes that hold the length of the message. */
	if( ( pxStreamBuffer->ucFlags & sbFLAGS_IS_MESSAGE_BUFFER ) != ( uint8_t ) 0 )
	{
		xBytesToStoreMessageLength = sbBYTES_TO_STORE_MESSAGE_LENGTH;
	}
	else
	{
		xBytesToStoreMessageLength = 0;
	}

	/* True if the available space equals zero. */
	if( xStreamBufferSpacesAvailable( xStreamBuffer ) <= xBytesToStoreMessageLength )
	{
		xReturn = pdTRUE;
	}
	else
	{
		xReturn = pdFALSE;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xStreamBufferSendCompletedFromISR( StreamBufferHandle_t xStreamBuffer, BaseType_t *pxHigherPriorityTaskWoken )
{
StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */
BaseType_t xReturn;
UBaseType_t uxSavedInterruptStatus;

	configASSERT( pxStreamBuffer );

	uxSavedInterruptStatus = ( UBaseType_t ) portSET_INTERRUPT_MASK_FROM_ISR();
	{
		if( ( pxStreamBuffer )->xTaskWaitingToReceive != NULL )
		{
			( void ) xTaskNotifyFromISR( ( pxStreamBuffer )->xTaskWaitingToReceive,
										 ( uint32_t ) 0,
										 eNoAction,
										 pxHigherPriorityTaskWoken );
			( pxStreamBuffer )->xTaskWaitingToReceive = NULL;
			xReturn = pdTRUE;
		}
		else
		{
			xReturn = pdFALSE;
		}
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

	return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xStreamBufferReceiveCompletedFromISR( StreamBufferHandle_t xStreamBuffer, BaseType_t *pxHigherPriorityTaskWoken )
{
StreamBuffer_t * const pxStreamBuffer = ( StreamBuffer_t * ) xStreamBuffer; /*lint !e9087 !e9079 Safe cast as StreamBufferHandle_t is opaque Streambuffer_t. */
BaseType_t xReturn;
UBaseType_t uxSavedInterruptStatus;

	configASSERT( pxStreamBuffer );

	uxSavedInterruptStatus = ( UBaseType_t ) portSET_INTERRUPT_MASK_FROM_ISR();
	{
		if( ( pxStreamBuffer )->xTaskWaitingToSend != NULL )
		{
			( void ) xTaskNotifyFromISR( ( pxStreamBuffer )->xTaskWaitingToSend,
										 ( uint32_t ) 0,
										 eNoAction,
										 pxHigherPriorityTaskWoken );
			( pxStreamBuffer )->xTaskWaitingToSend = NULL;
			xReturn = pdTRUE;
		}
		else
		{
			xReturn = pdFALSE;
		}
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

	return xReturn;
}
/*-----------------------------------------------------------*/

static size_t prvWriteBytesToBuffer( StreamBuffer_t * const pxStreamBuffer, const uint8_t *pucData, size_t xCount )
{
size_t xNextHead, xFirstLength;

	configASSERT( xCount > ( size_t ) 0 );

	xNextHead = pxStreamBuffer->xHead;

	/* Calculate the number of bytes that can be added in the first write -
	which may be less than the total number of bytes that need to be added if
	the buffer will wrap back to the beginning. */
	xFirstLength = configMIN( pxStreamBuffer->xLength - xNextHead, xCount );

	/* Write as many bytes as can be written in the first write. */
	configASSERT( ( xNextHead + xFirstLength ) <= pxStreamBuffer->xLength );
	memcpy( ( void* ) ( &( pxStreamBuffer->pucBuffer[ xNextHead ] ) ), ( const void * ) pucData, xFirstLength ); /*lint !e9087 memcpy() requires void *. */

	/* If the number of bytes written was less than the number that could be
	written in the first write... */
	if( xCount > xFirstLength )
	{
		/* ...then write the remaining bytes to the start of the buffer. */
		configASSERT( ( xCount - xFirstLength ) <= pxStreamBuffer->xLength );
		memcpy( ( void * ) pxStreamBuffer->pucBuffer, ( const void * ) &( pucData[ xFirstLength ] ), xCount - xFirstLength ); /*lint !e9087 memcpy() requires void *. */
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	xNextHead += xCount;
	if( xNextHead >= pxStreamBuffer->xLength )
	{
		xNextHead -= pxStreamBuffer->xLength;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	pxStreamBuffer->xHead = xNextHead;

	return xCount;
}
/*-----------------------------------------------------------*/

static size_t prvReadBytesFromBuffer( StreamBuffer_t *pxStreamBuffer, uint8_t *pucData, size_t xMaxCount, size_t xBytesAvailable )
{
size_t xCount, xFirstLength, xNextTail;

	/* Use the minimum of the wanted bytes and the available bytes. */
	xCount = configMIN( xBytesAvailable, xMaxCount );

	if( xCount > ( size_t ) 0 )
	{
		xNextTail = pxStreamBuffer->xTail;

		/* Calculate the number of bytes that can be read - which may be
		less than the number wanted if the data wraps around to the start of
		the buffer. */
		xFirstLength = configMIN( pxStreamBuffer->xLength - xNextTail, xCount );

		/* Obtain the number of bytes it is possible to obtain in the first
		read.  Asserts check bounds of read and write. */
		configASSERT( xFirstLength <= xMaxCount );
		configASSERT( ( xNextTail + xFirstLength ) <= pxStreamBuffer->xLength );
		memcpy( ( void * ) pucData, ( const void * ) &( pxStreamBuffer->pucBuffer[ xNextTail ] ), xFirstLength ); /*lint !e9087 memcpy() requires void *. */

		/* If the total number of wanted bytes is greater than the number
		that could be read in the first read... */
		if( xCount > xFirstLength )
		{
			/*...then read the remaining bytes from the start of the buffer. */
			configASSERT( xCount <= xMaxCount );
			memcpy( ( void * ) &( pucData[ xFirstLength ] ), ( void * ) ( pxStreamBuffer->pucBuffer ), xCount - xFirstLength ); /*lint !e9087 memcpy() requires void *. */
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		/* Move the tail pointer to effectively remove the data read from
		the buffer. */
		xNextTail += xCount;

		if( xNextTail >= pxStreamBuffer->xLength )
		{
			xNextTail -= pxStreamBuffer->xLength;
		}

		pxStreamBuffer->xTail = xNextTail;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	return xCount;
}
/*-----------------------------------------------------------*/

static size_t prvBytesInBuffer( const StreamBuffer_t * const pxStreamBuffer )
{
/* Returns the distance between xTail and xHead. */
size_t xCount;

	xCount = pxStreamBuffer->xLength + pxStreamBuffer->xHead;
	xCount -= pxStreamBuffer->xTail;
	if ( xCount >= pxStreamBuffer->xLength )
	{
		xCount -= pxStreamBuffer->xLength;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	return xCount;
}
/*-----------------------------------------------------------*/

static void prvInitialiseNewStreamBuffer( StreamBuffer_t * const pxStreamBuffer,
										  uint8_t * const pucBuffer,
										  size_t xBufferSizeBytes,
										  size_t xTriggerLevelBytes,
										  BaseType_t xIsMessageBuffer )
{
	/* Assert here is deliberately writing to the entire buffer to ensure it can
	be written to without generating exceptions, and is setting the buffer to a
	known value to assist in development/debugging. */
	#if( configASSERT_DEFINED == 1 )
	{
		/* The value written just has to be identifiable when looking at the
		memory.  Don't use 0xA5 as that is the stack fill value and could
		result in confusion as to what is actually being observed. */
		const BaseType_t xWriteValue = 0x55;
		configASSERT( memset( pucBuffer, ( int ) xWriteValue, xBufferSizeBytes ) == pucBuffer );
	}
	#endif

	memset( ( void * ) pxStreamBuffer, 0x00, sizeof( StreamBuffer_t ) ); /*lint !e9087 memset() requires void *. */
	pxStreamBuffer->pucBuffer = pucBuffer;
	pxStreamBuffer->xLength = xBufferSizeBytes;
	pxStreamBuffer->xTriggerLevelBytes = xTriggerLevelBytes;

	if( xIsMessageBuffer != pdFALSE )
	{
		pxStreamBuffer->ucFlags |= sbFLAGS_IS_MESSAGE_BUFFER;
	}
}

#if ( configUSE_TRACE_FACILITY == 1 )

	UBaseType_t uxStreamBufferGetStreamBufferNumber( StreamBufferHandle_t xStreamBuffer )
	{
		return ( ( StreamBuffer_t * ) xStreamBuffer )->uxStreamBufferNumber;
	}

#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if ( configUSE_TRACE_FACILITY == 1 )

	void vStreamBufferSetStreamBufferNumber( StreamBufferHandle_t xStreamBuffer, UBaseType_t uxStreamBufferNumber )
	{
		( ( StreamBuffer_t * ) xStreamBuffer )->uxStreamBufferNumber = uxStreamBufferNumber;
	}

#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if ( configUSE_TRACE_FACILITY == 1 )

	uint8_t ucStreamBufferGetStreamBufferType( StreamBufferHandle_t xStreamBuffer )
	{
		return ( ( StreamBuffer_t * )xStreamBuffer )->ucFlags | sbFLAGS_IS_MESSAGE_BUFFER;
	}

#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/
