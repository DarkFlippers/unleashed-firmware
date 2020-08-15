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

/*
 * Stream buffers are used to send a continuous stream of data from one task or
 * interrupt to another.  Their implementation is light weight, making them
 * particularly suited for interrupt to task and core to core communication
 * scenarios.
 *
 * ***NOTE***:  Uniquely among FreeRTOS objects, the stream buffer
 * implementation (so also the message buffer implementation, as message buffers
 * are built on top of stream buffers) assumes there is only one task or
 * interrupt that will write to the buffer (the writer), and only one task or
 * interrupt that will read from the buffer (the reader).  It is safe for the
 * writer and reader to be different tasks or interrupts, but, unlike other
 * FreeRTOS objects, it is not safe to have multiple different writers or
 * multiple different readers.  If there are to be multiple different writers
 * then the application writer must place each call to a writing API function
 * (such as xStreamBufferSend()) inside a critical section and set the send
 * block time to 0.  Likewise, if there are to be multiple different readers
 * then the application writer must place each call to a reading API function
 * (such as xStreamBufferRead()) inside a critical section section and set the
 * receive block time to 0.
 *
 */

#ifndef STREAM_BUFFER_H
#define STREAM_BUFFER_H

#if defined( __cplusplus )
extern "C" {
#endif

/**
 * Type by which stream buffers are referenced.  For example, a call to
 * xStreamBufferCreate() returns an StreamBufferHandle_t variable that can
 * then be used as a parameter to xStreamBufferSend(), xStreamBufferReceive(),
 * etc.
 */
typedef void * StreamBufferHandle_t;


/**
 * message_buffer.h
 *
<pre>
StreamBufferHandle_t xStreamBufferCreate( size_t xBufferSizeBytes, size_t xTriggerLevelBytes );
</pre>
 *
 * Creates a new stream buffer using dynamically allocated memory.  See
 * xStreamBufferCreateStatic() for a version that uses statically allocated
 * memory (memory that is allocated at compile time).
 *
 * configSUPPORT_DYNAMIC_ALLOCATION must be set to 1 or left undefined in
 * FreeRTOSConfig.h for xStreamBufferCreate() to be available.
 *
 * @param xBufferSizeBytes The total number of bytes the stream buffer will be
 * able to hold at any one time.
 *
 * @param xTriggerLevelBytes The number of bytes that must be in the stream
 * buffer before a task that is blocked on the stream buffer to wait for data is
 * moved out of the blocked state.  For example, if a task is blocked on a read
 * of an empty stream buffer that has a trigger level of 1 then the task will be
 * unblocked when a single byte is written to the buffer or the task's block
 * time expires.  As another example, if a task is blocked on a read of an empty
 * stream buffer that has a trigger level of 10 then the task will not be
 * unblocked until the stream buffer contains at least 10 bytes or the task's
 * block time expires.  If a reading task's block time expires before the
 * trigger level is reached then the task will still receive however many bytes
 * are actually available.  Setting a trigger level of 0 will result in a
 * trigger level of 1 being used.  It is not valid to specify a trigger level
 * that is greater than the buffer size.
 *
 * @return If NULL is returned, then the stream buffer cannot be created
 * because there is insufficient heap memory available for FreeRTOS to allocate
 * the stream buffer data structures and storage area.  A non-NULL value being
 * returned indicates that the stream buffer has been created successfully -
 * the returned value should be stored as the handle to the created stream
 * buffer.
 *
 * Example use:
<pre>

void vAFunction( void )
{
StreamBufferHandle_t xStreamBuffer;
const size_t xStreamBufferSizeBytes = 100, xTriggerLevel = 10;

    // Create a stream buffer that can hold 100 bytes.  The memory used to hold
    // both the stream buffer structure and the data in the stream buffer is
    // allocated dynamically.
    xStreamBuffer = xStreamBufferCreate( xStreamBufferSizeBytes, xTriggerLevel );

    if( xStreamBuffer == NULL )
    {
        // There was not enough heap memory space available to create the
        // stream buffer.
    }
    else
    {
        // The stream buffer was created successfully and can now be used.
    }
}
</pre>
 * \defgroup xStreamBufferCreate xStreamBufferCreate
 * \ingroup StreamBufferManagement
 */
#define xStreamBufferCreate( xBufferSizeBytes, xTriggerLevelBytes ) xStreamBufferGenericCreate( xBufferSizeBytes, xTriggerLevelBytes, pdFALSE )

/**
 * stream_buffer.h
 *
<pre>
StreamBufferHandle_t xStreamBufferCreateStatic( size_t xBufferSizeBytes,
                                                size_t xTriggerLevelBytes,
                                                uint8_t *pucStreamBufferStorageArea,
                                                StaticStreamBuffer_t *pxStaticStreamBuffer );
</pre>
 * Creates a new stream buffer using statically allocated memory.  See
 * xStreamBufferCreate() for a version that uses dynamically allocated memory.
 *
 * configSUPPORT_STATIC_ALLOCATION must be set to 1 in FreeRTOSConfig.h for
 * xStreamBufferCreateStatic() to be available.
 *
 * @param xBufferSizeBytes The size, in bytes, of the buffer pointed to by the
 * pucStreamBufferStorageArea parameter.
 *
 * @param xTriggerLevelBytes The number of bytes that must be in the stream
 * buffer before a task that is blocked on the stream buffer to wait for data is
 * moved out of the blocked state.  For example, if a task is blocked on a read
 * of an empty stream buffer that has a trigger level of 1 then the task will be
 * unblocked when a single byte is written to the buffer or the task's block
 * time expires.  As another example, if a task is blocked on a read of an empty
 * stream buffer that has a trigger level of 10 then the task will not be
 * unblocked until the stream buffer contains at least 10 bytes or the task's
 * block time expires.  If a reading task's block time expires before the
 * trigger level is reached then the task will still receive however many bytes
 * are actually available.  Setting a trigger level of 0 will result in a
 * trigger level of 1 being used.  It is not valid to specify a trigger level
 * that is greater than the buffer size.
 *
 * @param pucStreamBufferStorageArea Must point to a uint8_t array that is at
 * least xBufferSizeBytes + 1 big.  This is the array to which streams are
 * copied when they are written to the stream buffer.
 *
 * @param pxStaticStreamBuffer Must point to a variable of type
 * StaticStreamBuffer_t, which will be used to hold the stream buffer's data
 * structure.
 *
 * @return If the stream buffer is created successfully then a handle to the
 * created stream buffer is returned. If either pucStreamBufferStorageArea or
 * pxStaticstreamBuffer are NULL then NULL is returned.
 *
 * Example use:
<pre>

// Used to dimension the array used to hold the streams.  The available space
// will actually be one less than this, so 999.
#define STORAGE_SIZE_BYTES 1000

// Defines the memory that will actually hold the streams within the stream
// buffer.
static uint8_t ucStorageBuffer[ STORAGE_SIZE_BYTES ];

// The variable used to hold the stream buffer structure.
StaticStreamBuffer_t xStreamBufferStruct;

void MyFunction( void )
{
StreamBufferHandle_t xStreamBuffer;
const size_t xTriggerLevel = 1;

    xStreamBuffer = xStreamBufferCreateStatic( sizeof( ucBufferStorage ),
                                               xTriggerLevel,
                                               ucBufferStorage,
                                               &xStreamBufferStruct );

    // As neither the pucStreamBufferStorageArea or pxStaticStreamBuffer
    // parameters were NULL, xStreamBuffer will not be NULL, and can be used to
    // reference the created stream buffer in other stream buffer API calls.

    // Other code that uses the stream buffer can go here.
}

</pre>
 * \defgroup xStreamBufferCreateStatic xStreamBufferCreateStatic
 * \ingroup StreamBufferManagement
 */
#define xStreamBufferCreateStatic( xBufferSizeBytes, xTriggerLevelBytes, pucStreamBufferStorageArea, pxStaticStreamBuffer ) xStreamBufferGenericCreateStatic( xBufferSizeBytes, xTriggerLevelBytes, pdFALSE, pucStreamBufferStorageArea, pxStaticStreamBuffer )

/**
 * stream_buffer.h
 *
<pre>
size_t xStreamBufferSend( StreamBufferHandle_t xStreamBuffer,
                          const void *pvTxData,
                          size_t xDataLengthBytes,
                          TickType_t xTicksToWait );
<pre>
 *
 * Sends bytes to a stream buffer.  The bytes are copied into the stream buffer.
 *
 * ***NOTE***:  Uniquely among FreeRTOS objects, the stream buffer
 * implementation (so also the message buffer implementation, as message buffers
 * are built on top of stream buffers) assumes there is only one task or
 * interrupt that will write to the buffer (the writer), and only one task or
 * interrupt that will read from the buffer (the reader).  It is safe for the
 * writer and reader to be different tasks or interrupts, but, unlike other
 * FreeRTOS objects, it is not safe to have multiple different writers or
 * multiple different readers.  If there are to be multiple different writers
 * then the application writer must place each call to a writing API function
 * (such as xStreamBufferSend()) inside a critical section and set the send
 * block time to 0.  Likewise, if there are to be multiple different readers
 * then the application writer must place each call to a reading API function
 * (such as xStreamBufferRead()) inside a critical section and set the receive
 * block time to 0.
 *
 * Use xStreamBufferSend() to write to a stream buffer from a task.  Use
 * xStreamBufferSendFromISR() to write to a stream buffer from an interrupt
 * service routine (ISR).
 *
 * @param xStreamBuffer The handle of the stream buffer to which a stream is
 * being sent.
 *
 * @param pvTxData A pointer to the buffer that holds the bytes to be copied
 * into the stream buffer.
 *
 * @param xDataLengthBytes   The maximum number of bytes to copy from pvTxData
 * into the stream buffer.
 *
 * @param xTicksToWait The maximum amount of time the task should remain in the
 * Blocked state to wait for enough space to become available in the stream
 * buffer, should the stream buffer contain too little space to hold the
 * another xDataLengthBytes bytes.  The block time is specified in tick periods,
 * so the absolute time it represents is dependent on the tick frequency.  The
 * macro pdMS_TO_TICKS() can be used to convert a time specified in milliseconds
 * into a time specified in ticks.  Setting xTicksToWait to portMAX_DELAY will
 * cause the task to wait indefinitely (without timing out), provided
 * INCLUDE_vTaskSuspend is set to 1 in FreeRTOSConfig.h.  If a task times out
 * before it can write all xDataLengthBytes into the buffer it will still write
 * as many bytes as possible.  A task does not use any CPU time when it is in
 * the blocked state.
 *
 * @return The number of bytes written to the stream buffer.  If a task times
 * out before it can write all xDataLengthBytes into the buffer it will still
 * write as many bytes as possible.
 *
 * Example use:
<pre>
void vAFunction( StreamBufferHandle_t xStreamBuffer )
{
size_t xBytesSent;
uint8_t ucArrayToSend[] = { 0, 1, 2, 3 };
char *pcStringToSend = "String to send";
const TickType_t x100ms = pdMS_TO_TICKS( 100 );

    // Send an array to the stream buffer, blocking for a maximum of 100ms to
    // wait for enough space to be available in the stream buffer.
    xBytesSent = xStreamBufferSend( xStreamBuffer, ( void * ) ucArrayToSend, sizeof( ucArrayToSend ), x100ms );

    if( xBytesSent != sizeof( ucArrayToSend ) )
    {
        // The call to xStreamBufferSend() times out before there was enough
        // space in the buffer for the data to be written, but it did
        // successfully write xBytesSent bytes.
    }

    // Send the string to the stream buffer.  Return immediately if there is not
    // enough space in the buffer.
    xBytesSent = xStreamBufferSend( xStreamBuffer, ( void * ) pcStringToSend, strlen( pcStringToSend ), 0 );

    if( xBytesSent != strlen( pcStringToSend ) )
    {
        // The entire string could not be added to the stream buffer because
        // there was not enough free space in the buffer, but xBytesSent bytes
        // were sent.  Could try again to send the remaining bytes.
    }
}
</pre>
 * \defgroup xStreamBufferSend xStreamBufferSend
 * \ingroup StreamBufferManagement
 */
size_t xStreamBufferSend( StreamBufferHandle_t xStreamBuffer,
						  const void *pvTxData,
						  size_t xDataLengthBytes,
						  TickType_t xTicksToWait ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
size_t xStreamBufferSendFromISR( StreamBufferHandle_t xStreamBuffer,
                                 const void *pvTxData,
                                 size_t xDataLengthBytes,
                                 BaseType_t *pxHigherPriorityTaskWoken );
<pre>
 *
 * Interrupt safe version of the API function that sends a stream of bytes to
 * the stream buffer.
 *
 * ***NOTE***:  Uniquely among FreeRTOS objects, the stream buffer
 * implementation (so also the message buffer implementation, as message buffers
 * are built on top of stream buffers) assumes there is only one task or
 * interrupt that will write to the buffer (the writer), and only one task or
 * interrupt that will read from the buffer (the reader).  It is safe for the
 * writer and reader to be different tasks or interrupts, but, unlike other
 * FreeRTOS objects, it is not safe to have multiple different writers or
 * multiple different readers.  If there are to be multiple different writers
 * then the application writer must place each call to a writing API function
 * (such as xStreamBufferSend()) inside a critical section and set the send
 * block time to 0.  Likewise, if there are to be multiple different readers
 * then the application writer must place each call to a reading API function
 * (such as xStreamBufferRead()) inside a critical section and set the receive
 * block time to 0.
 *
 * Use xStreamBufferSend() to write to a stream buffer from a task.  Use
 * xStreamBufferSendFromISR() to write to a stream buffer from an interrupt
 * service routine (ISR).
 *
 * @param xStreamBuffer The handle of the stream buffer to which a stream is
 * being sent.
 *
 * @param pvTxData A pointer to the data that is to be copied into the stream
 * buffer.
 *
 * @param xDataLengthBytes The maximum number of bytes to copy from pvTxData
 * into the stream buffer.
 *
 * @param pxHigherPriorityTaskWoken  It is possible that a stream buffer will
 * have a task blocked on it waiting for data.  Calling
 * xStreamBufferSendFromISR() can make data available, and so cause a task that
 * was waiting for data to leave the Blocked state.  If calling
 * xStreamBufferSendFromISR() causes a task to leave the Blocked state, and the
 * unblocked task has a priority higher than the currently executing task (the
 * task that was interrupted), then, internally, xStreamBufferSendFromISR()
 * will set *pxHigherPriorityTaskWoken to pdTRUE.  If
 * xStreamBufferSendFromISR() sets this value to pdTRUE, then normally a
 * context switch should be performed before the interrupt is exited.  This will
 * ensure that the interrupt returns directly to the highest priority Ready
 * state task.  *pxHigherPriorityTaskWoken should be set to pdFALSE before it
 * is passed into the function.  See the example code below for an example.
 *
 * @return The number of bytes actually written to the stream buffer, which will
 * be less than xDataLengthBytes if the stream buffer didn't have enough free
 * space for all the bytes to be written.
 *
 * Example use:
<pre>
// A stream buffer that has already been created.
StreamBufferHandle_t xStreamBuffer;

void vAnInterruptServiceRoutine( void )
{
size_t xBytesSent;
char *pcStringToSend = "String to send";
BaseType_t xHigherPriorityTaskWoken = pdFALSE; // Initialised to pdFALSE.

    // Attempt to send the string to the stream buffer.
    xBytesSent = xStreamBufferSendFromISR( xStreamBuffer,
                                           ( void * ) pcStringToSend,
                                           strlen( pcStringToSend ),
                                           &xHigherPriorityTaskWoken );

    if( xBytesSent != strlen( pcStringToSend ) )
    {
        // There was not enough free space in the stream buffer for the entire
        // string to be written, ut xBytesSent bytes were written.
    }

    // If xHigherPriorityTaskWoken was set to pdTRUE inside
    // xStreamBufferSendFromISR() then a task that has a priority above the
    // priority of the currently executing task was unblocked and a context
    // switch should be performed to ensure the ISR returns to the unblocked
    // task.  In most FreeRTOS ports this is done by simply passing
    // xHigherPriorityTaskWoken into taskYIELD_FROM_ISR(), which will test the
    // variables value, and perform the context switch if necessary.  Check the
    // documentation for the port in use for port specific instructions.
    taskYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
</pre>
 * \defgroup xStreamBufferSendFromISR xStreamBufferSendFromISR
 * \ingroup StreamBufferManagement
 */
size_t xStreamBufferSendFromISR( StreamBufferHandle_t xStreamBuffer,
								 const void *pvTxData,
								 size_t xDataLengthBytes,
								 BaseType_t * const pxHigherPriorityTaskWoken ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
size_t xStreamBufferReceive( StreamBufferHandle_t xStreamBuffer,
                             void *pvRxData,
                             size_t xBufferLengthBytes,
                             TickType_t xTicksToWait );
</pre>
 *
 * Receives bytes from a stream buffer.
 *
 * ***NOTE***:  Uniquely among FreeRTOS objects, the stream buffer
 * implementation (so also the message buffer implementation, as message buffers
 * are built on top of stream buffers) assumes there is only one task or
 * interrupt that will write to the buffer (the writer), and only one task or
 * interrupt that will read from the buffer (the reader).  It is safe for the
 * writer and reader to be different tasks or interrupts, but, unlike other
 * FreeRTOS objects, it is not safe to have multiple different writers or
 * multiple different readers.  If there are to be multiple different writers
 * then the application writer must place each call to a writing API function
 * (such as xStreamBufferSend()) inside a critical section and set the send
 * block time to 0.  Likewise, if there are to be multiple different readers
 * then the application writer must place each call to a reading API function
 * (such as xStreamBufferRead()) inside a critical section and set the receive
 * block time to 0.
 *
 * Use xStreamBufferReceive() to read from a stream buffer from a task.  Use
 * xStreamBufferReceiveFromISR() to read from a stream buffer from an
 * interrupt service routine (ISR).
 *
 * @param xStreamBuffer The handle of the stream buffer from which bytes are to
 * be received.
 *
 * @param pvRxData A pointer to the buffer into which the received bytes will be
 * copied.
 *
 * @param xBufferLengthBytes The length of the buffer pointed to by the
 * pvRxData parameter.  This sets the maximum number of bytes to receive in one
 * call.  xStreamBufferReceive will return as many bytes as possible up to a
 * maximum set by xBufferLengthBytes.
 *
 * @param xTicksToWait The maximum amount of time the task should remain in the
 * Blocked state to wait for data to become available if the stream buffer is
 * empty.  xStreamBufferReceive() will return immediately if xTicksToWait is
 * zero.  The block time is specified in tick periods, so the absolute time it
 * represents is dependent on the tick frequency.  The macro pdMS_TO_TICKS() can
 * be used to convert a time specified in milliseconds into a time specified in
 * ticks.  Setting xTicksToWait to portMAX_DELAY will cause the task to wait
 * indefinitely (without timing out), provided INCLUDE_vTaskSuspend is set to 1
 * in FreeRTOSConfig.h.  A task does not use any CPU time when it is in the
 * Blocked state.
 *
 * @return The number of bytes actually read from the stream buffer, which will
 * be less than xBufferLengthBytes if the call to xStreamBufferReceive() timed
 * out before xBufferLengthBytes were available.
 *
 * Example use:
<pre>
void vAFunction( StreamBuffer_t xStreamBuffer )
{
uint8_t ucRxData[ 20 ];
size_t xReceivedBytes;
const TickType_t xBlockTime = pdMS_TO_TICKS( 20 );

    // Receive up to another sizeof( ucRxData ) bytes from the stream buffer.
    // Wait in the Blocked state (so not using any CPU processing time) for a
    // maximum of 100ms for the full sizeof( ucRxData ) number of bytes to be
    // available.
    xReceivedBytes = xStreamBufferReceive( xStreamBuffer,
                                           ( void * ) ucRxData,
                                           sizeof( ucRxData ),
                                           xBlockTime );

    if( xReceivedBytes > 0 )
    {
        // A ucRxData contains another xRecievedBytes bytes of data, which can
        // be processed here....
    }
}
</pre>
 * \defgroup xStreamBufferReceive xStreamBufferReceive
 * \ingroup StreamBufferManagement
 */
size_t xStreamBufferReceive( StreamBufferHandle_t xStreamBuffer,
							 void *pvRxData,
							 size_t xBufferLengthBytes,
							 TickType_t xTicksToWait ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
size_t xStreamBufferReceiveFromISR( StreamBufferHandle_t xStreamBuffer,
                                    void *pvRxData,
                                    size_t xBufferLengthBytes,
                                    BaseType_t *pxHigherPriorityTaskWoken );
</pre>
 *
 * An interrupt safe version of the API function that receives bytes from a
 * stream buffer.
 *
 * Use xStreamBufferReceive() to read bytes from a stream buffer from a task.
 * Use xStreamBufferReceiveFromISR() to read bytes from a stream buffer from an
 * interrupt service routine (ISR).
 *
 * @param xStreamBuffer The handle of the stream buffer from which a stream
 * is being received.
 *
 * @param pvRxData A pointer to the buffer into which the received bytes are
 * copied.
 *
 * @param xBufferLengthBytes The length of the buffer pointed to by the
 * pvRxData parameter.  This sets the maximum number of bytes to receive in one
 * call.  xStreamBufferReceive will return as many bytes as possible up to a
 * maximum set by xBufferLengthBytes.
 *
 * @param pxHigherPriorityTaskWoken  It is possible that a stream buffer will
 * have a task blocked on it waiting for space to become available.  Calling
 * xStreamBufferReceiveFromISR() can make space available, and so cause a task
 * that is waiting for space to leave the Blocked state.  If calling
 * xStreamBufferReceiveFromISR() causes a task to leave the Blocked state, and
 * the unblocked task has a priority higher than the currently executing task
 * (the task that was interrupted), then, internally,
 * xStreamBufferReceiveFromISR() will set *pxHigherPriorityTaskWoken to pdTRUE.
 * If xStreamBufferReceiveFromISR() sets this value to pdTRUE, then normally a
 * context switch should be performed before the interrupt is exited.  That will
 * ensure the interrupt returns directly to the highest priority Ready state
 * task.  *pxHigherPriorityTaskWoken should be set to pdFALSE before it is
 * passed into the function.  See the code example below for an example.
 *
 * @return The number of bytes read from the stream buffer, if any.
 *
 * Example use:
<pre>
// A stream buffer that has already been created.
StreamBuffer_t xStreamBuffer;

void vAnInterruptServiceRoutine( void )
{
uint8_t ucRxData[ 20 ];
size_t xReceivedBytes;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;  // Initialised to pdFALSE.

    // Receive the next stream from the stream buffer.
    xReceivedBytes = xStreamBufferReceiveFromISR( xStreamBuffer,
                                                  ( void * ) ucRxData,
                                                  sizeof( ucRxData ),
                                                  &xHigherPriorityTaskWoken );

    if( xReceivedBytes > 0 )
    {
        // ucRxData contains xReceivedBytes read from the stream buffer.
        // Process the stream here....
    }

    // If xHigherPriorityTaskWoken was set to pdTRUE inside
    // xStreamBufferReceiveFromISR() then a task that has a priority above the
    // priority of the currently executing task was unblocked and a context
    // switch should be performed to ensure the ISR returns to the unblocked
    // task.  In most FreeRTOS ports this is done by simply passing
    // xHigherPriorityTaskWoken into taskYIELD_FROM_ISR(), which will test the
    // variables value, and perform the context switch if necessary.  Check the
    // documentation for the port in use for port specific instructions.
    taskYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
</pre>
 * \defgroup xStreamBufferReceiveFromISR xStreamBufferReceiveFromISR
 * \ingroup StreamBufferManagement
 */
size_t xStreamBufferReceiveFromISR( StreamBufferHandle_t xStreamBuffer,
									void *pvRxData,
									size_t xBufferLengthBytes,
									BaseType_t * const pxHigherPriorityTaskWoken ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
void vStreamBufferDelete( StreamBufferHandle_t xStreamBuffer );
</pre>
 *
 * Deletes a stream buffer that was previously created using a call to
 * xStreamBufferCreate() or xStreamBufferCreateStatic().  If the stream
 * buffer was created using dynamic memory (that is, by xStreamBufferCreate()),
 * then the allocated memory is freed.
 *
 * A stream buffer handle must not be used after the stream buffer has been
 * deleted.
 *
 * @param xStreamBuffer The handle of the stream buffer to be deleted.
 *
 * \defgroup vStreamBufferDelete vStreamBufferDelete
 * \ingroup StreamBufferManagement
 */
void vStreamBufferDelete( StreamBufferHandle_t xStreamBuffer ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
BaseType_t xStreamBufferIsFull( StreamBufferHandle_t xStreamBuffer );
</pre>
 *
 * Queries a stream buffer to see if it is full.  A stream buffer is full if it
 * does not have any free space, and therefore cannot accept any more data.
 *
 * @param xStreamBuffer The handle of the stream buffer being queried.
 *
 * @return If the stream buffer is full then pdTRUE is returned.  Otherwise
 * pdFALSE is returned.
 *
 * \defgroup xStreamBufferIsFull xStreamBufferIsFull
 * \ingroup StreamBufferManagement
 */
BaseType_t xStreamBufferIsFull( StreamBufferHandle_t xStreamBuffer ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
BaseType_t xStreamBufferIsEmpty( StreamBufferHandle_t xStreamBuffer );
</pre>
 *
 * Queries a stream buffer to see if it is empty.  A stream buffer is empty if
 * it does not contain any data.
 *
 * @param xStreamBuffer The handle of the stream buffer being queried.
 *
 * @return If the stream buffer is empty then pdTRUE is returned.  Otherwise
 * pdFALSE is returned.
 *
 * \defgroup xStreamBufferIsEmpty xStreamBufferIsEmpty
 * \ingroup StreamBufferManagement
 */
BaseType_t xStreamBufferIsEmpty( StreamBufferHandle_t xStreamBuffer ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
BaseType_t xStreamBufferReset( StreamBufferHandle_t xStreamBuffer );
</pre>
 *
 * Resets a stream buffer to its initial, empty, state.  Any data that was in
 * the stream buffer is discarded.  A stream buffer can only be reset if there
 * are no tasks blocked waiting to either send to or receive from the stream
 * buffer.
 *
 * @param xStreamBuffer The handle of the stream buffer being reset.
 *
 * @return If the stream buffer is reset then pdPASS is returned.  If there was
 * a task blocked waiting to send to or read from the stream buffer then the
 * stream buffer is not reset and pdFAIL is returned.
 *
 * \defgroup xStreamBufferReset xStreamBufferReset
 * \ingroup StreamBufferManagement
 */
BaseType_t xStreamBufferReset( StreamBufferHandle_t xStreamBuffer ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
size_t xStreamBufferSpacesAvailable( StreamBufferHandle_t xStreamBuffer );
</pre>
 *
 * Queries a stream buffer to see how much free space it contains, which is
 * equal to the amount of data that can be sent to the stream buffer before it
 * is full.
 *
 * @param xStreamBuffer The handle of the stream buffer being queried.
 *
 * @return The number of bytes that can be written to the stream buffer before
 * the stream buffer would be full.
 *
 * \defgroup xStreamBufferSpacesAvailable xStreamBufferSpacesAvailable
 * \ingroup StreamBufferManagement
 */
size_t xStreamBufferSpacesAvailable( StreamBufferHandle_t xStreamBuffer ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
size_t xStreamBufferBytesAvailable( StreamBufferHandle_t xStreamBuffer );
</pre>
 *
 * Queries a stream buffer to see how much data it contains, which is equal to
 * the number of bytes that can be read from the stream buffer before the stream
 * buffer would be empty.
 *
 * @param xStreamBuffer The handle of the stream buffer being queried.
 *
 * @return The number of bytes that can be read from the stream buffer before
 * the stream buffer would be empty.
 *
 * \defgroup xStreamBufferBytesAvailable xStreamBufferBytesAvailable
 * \ingroup StreamBufferManagement
 */
size_t xStreamBufferBytesAvailable( StreamBufferHandle_t xStreamBuffer ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
BaseType_t xStreamBufferSetTriggerLevel( StreamBufferHandle_t xStreamBuffer, size_t xTriggerLevel );
</pre>
 *
 * A stream buffer's trigger level is the number of bytes that must be in the
 * stream buffer before a task that is blocked on the stream buffer to
 * wait for data is moved out of the blocked state.  For example, if a task is
 * blocked on a read of an empty stream buffer that has a trigger level of 1
 * then the task will be unblocked when a single byte is written to the buffer
 * or the task's block time expires.  As another example, if a task is blocked
 * on a read of an empty stream buffer that has a trigger level of 10 then the
 * task will not be unblocked until the stream buffer contains at least 10 bytes
 * or the task's block time expires.  If a reading task's block time expires
 * before the trigger level is reached then the task will still receive however
 * many bytes are actually available.  Setting a trigger level of 0 will result
 * in a trigger level of 1 being used.  It is not valid to specify a trigger
 * level that is greater than the buffer size.
 *
 * A trigger level is set when the stream buffer is created, and can be modified
 * using xStreamBufferSetTriggerLevel().
 *
 * @param xStreamBuffer The handle of the stream buffer being updated.
 *
 * @param xTriggerLevel The new trigger level for the stream buffer.
 *
 * @return If xTriggerLevel was less than or equal to the stream buffer's length
 * then the trigger level will be updated and pdTRUE is returned.  Otherwise
 * pdFALSE is returned.
 *
 * \defgroup xStreamBufferSetTriggerLevel xStreamBufferSetTriggerLevel
 * \ingroup StreamBufferManagement
 */
BaseType_t xStreamBufferSetTriggerLevel( StreamBufferHandle_t xStreamBuffer, size_t xTriggerLevel ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
BaseType_t xStreamBufferSendCompletedFromISR( StreamBufferHandle_t xStreamBuffer, BaseType_t *pxHigherPriorityTaskWoken );
</pre>
 *
 * For advanced users only.
 *
 * The sbSEND_COMPLETED() macro is called from within the FreeRTOS APIs when
 * data is sent to a message buffer or stream buffer.  If there was a task that
 * was blocked on the message or stream buffer waiting for data to arrive then
 * the sbSEND_COMPLETED() macro sends a notification to the task to remove it
 * from the Blocked state.  xStreamBufferSendCompletedFromISR() does the same
 * thing.  It is provided to enable application writers to implement their own
 * version of sbSEND_COMPLETED(), and MUST NOT BE USED AT ANY OTHER TIME.
 *
 * See the example implemented in FreeRTOS/Demo/Minimal/MessageBufferAMP.c for
 * additional information.
 *
 * @param xStreamBuffer The handle of the stream buffer to which data was
 * written.
 *
 * @param pxHigherPriorityTaskWoken *pxHigherPriorityTaskWoken should be
 * initialised to pdFALSE before it is passed into
 * xStreamBufferSendCompletedFromISR().  If calling
 * xStreamBufferSendCompletedFromISR() removes a task from the Blocked state,
 * and the task has a priority above the priority of the currently running task,
 * then *pxHigherPriorityTaskWoken will get set to pdTRUE indicating that a
 * context switch should be performed before exiting the ISR.
 *
 * @return If a task was removed from the Blocked state then pdTRUE is returned.
 * Otherwise pdFALSE is returned.
 *
 * \defgroup xStreamBufferSendCompletedFromISR xStreamBufferSendCompletedFromISR
 * \ingroup StreamBufferManagement
 */
BaseType_t xStreamBufferSendCompletedFromISR( StreamBufferHandle_t xStreamBuffer, BaseType_t *pxHigherPriorityTaskWoken ) PRIVILEGED_FUNCTION;

/**
 * stream_buffer.h
 *
<pre>
BaseType_t xStreamBufferReceiveCompletedFromISR( StreamBufferHandle_t xStreamBuffer, BaseType_t *pxHigherPriorityTaskWoken );
</pre>
 *
 * For advanced users only.
 *
 * The sbRECEIVE_COMPLETED() macro is called from within the FreeRTOS APIs when
 * data is read out of a message buffer or stream buffer.  If there was a task
 * that was blocked on the message or stream buffer waiting for data to arrive
 * then the sbRECEIVE_COMPLETED() macro sends a notification to the task to
 * remove it from the Blocked state.  xStreamBufferReceiveCompletedFromISR()
 * does the same thing.  It is provided to enable application writers to
 * implement their own version of sbRECEIVE_COMPLETED(), and MUST NOT BE USED AT
 * ANY OTHER TIME.
 *
 * See the example implemented in FreeRTOS/Demo/Minimal/MessageBufferAMP.c for
 * additional information.
 *
 * @param xStreamBuffer The handle of the stream buffer from which data was
 * read.
 *
 * @param pxHigherPriorityTaskWoken *pxHigherPriorityTaskWoken should be
 * initialised to pdFALSE before it is passed into
 * xStreamBufferReceiveCompletedFromISR().  If calling
 * xStreamBufferReceiveCompletedFromISR() removes a task from the Blocked state,
 * and the task has a priority above the priority of the currently running task,
 * then *pxHigherPriorityTaskWoken will get set to pdTRUE indicating that a
 * context switch should be performed before exiting the ISR.
 *
 * @return If a task was removed from the Blocked state then pdTRUE is returned.
 * Otherwise pdFALSE is returned.
 *
 * \defgroup xStreamBufferReceiveCompletedFromISR xStreamBufferReceiveCompletedFromISR
 * \ingroup StreamBufferManagement
 */
BaseType_t xStreamBufferReceiveCompletedFromISR( StreamBufferHandle_t xStreamBuffer, BaseType_t *pxHigherPriorityTaskWoken ) PRIVILEGED_FUNCTION;

/* Functions below here are not part of the public API. */
StreamBufferHandle_t xStreamBufferGenericCreate( size_t xBufferSizeBytes,
												 size_t xTriggerLevelBytes,
												 BaseType_t xIsMessageBuffer ) PRIVILEGED_FUNCTION;

StreamBufferHandle_t xStreamBufferGenericCreateStatic( size_t xBufferSizeBytes,
													   size_t xTriggerLevelBytes,
													   BaseType_t xIsMessageBuffer,
													   uint8_t * const pucStreamBufferStorageArea,
													   StaticStreamBuffer_t * const pxStaticStreamBuffer ) PRIVILEGED_FUNCTION;

#if( configUSE_TRACE_FACILITY == 1 )
	void vStreamBufferSetStreamBufferNumber( StreamBufferHandle_t xStreamBuffer, UBaseType_t uxStreamBufferNumber ) PRIVILEGED_FUNCTION;
	UBaseType_t uxStreamBufferGetStreamBufferNumber( StreamBufferHandle_t xStreamBuffer ) PRIVILEGED_FUNCTION;
	uint8_t ucStreamBufferGetStreamBufferType( StreamBufferHandle_t xStreamBuffer ) PRIVILEGED_FUNCTION;
#endif

#if defined( __cplusplus )
}
#endif

#endif	/* !defined( STREAM_BUFFER_H ) */
