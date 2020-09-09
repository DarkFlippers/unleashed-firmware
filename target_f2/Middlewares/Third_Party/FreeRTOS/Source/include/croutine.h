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

#ifndef CO_ROUTINE_H
#define CO_ROUTINE_H

#ifndef INC_FREERTOS_H
	#error "include FreeRTOS.h must appear in source files before include croutine.h"
#endif

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Used to hide the implementation of the co-routine control block.  The
control block structure however has to be included in the header due to
the macro implementation of the co-routine functionality. */
typedef void * CoRoutineHandle_t;

/* Defines the prototype to which co-routine functions must conform. */
typedef void (*crCOROUTINE_CODE)( CoRoutineHandle_t, UBaseType_t );

typedef struct corCoRoutineControlBlock
{
	crCOROUTINE_CODE 	pxCoRoutineFunction;
	ListItem_t			xGenericListItem;	/*< List item used to place the CRCB in ready and blocked queues. */
	ListItem_t			xEventListItem;		/*< List item used to place the CRCB in event lists. */
	UBaseType_t 		uxPriority;			/*< The priority of the co-routine in relation to other co-routines. */
	UBaseType_t 		uxIndex;			/*< Used to distinguish between co-routines when multiple co-routines use the same co-routine function. */
	uint16_t 			uxState;			/*< Used internally by the co-routine implementation. */
} CRCB_t; /* Co-routine control block.  Note must be identical in size down to uxPriority with TCB_t. */

/**
 * croutine. h
 *<pre>
 BaseType_t xCoRoutineCreate(
                                 crCOROUTINE_CODE pxCoRoutineCode,
                                 UBaseType_t uxPriority,
                                 UBaseType_t uxIndex
                               );</pre>
 *
 * Create a new co-routine and add it to the list of co-routines that are
 * ready to run.
 *
 * @param pxCoRoutineCode Pointer to the co-routine function.  Co-routine
 * functions require special syntax - see the co-routine section of the WEB
 * documentation for more information.
 *
 * @param uxPriority The priority with respect to other co-routines at which
 *  the co-routine will run.
 *
 * @param uxIndex Used to distinguish between different co-routines that
 * execute the same function.  See the example below and the co-routine section
 * of the WEB documentation for further information.
 *
 * @return pdPASS if the co-routine was successfully created and added to a ready
 * list, otherwise an error code defined with ProjDefs.h.
 *
 * Example usage:
   <pre>
 // Co-routine to be created.
 void vFlashCoRoutine( CoRoutineHandle_t xHandle, UBaseType_t uxIndex )
 {
 // Variables in co-routines must be declared static if they must maintain value across a blocking call.
 // This may not be necessary for const variables.
 static const char cLedToFlash[ 2 ] = { 5, 6 };
 static const TickType_t uxFlashRates[ 2 ] = { 200, 400 };

     // Must start every co-routine with a call to crSTART();
     crSTART( xHandle );

     for( ;; )
     {
         // This co-routine just delays for a fixed period, then toggles
         // an LED.  Two co-routines are created using this function, so
         // the uxIndex parameter is used to tell the co-routine which
         // LED to flash and how int32_t to delay.  This assumes xQueue has
         // already been created.
         vParTestToggleLED( cLedToFlash[ uxIndex ] );
         crDELAY( xHandle, uxFlashRates[ uxIndex ] );
     }

     // Must end every co-routine with a call to crEND();
     crEND();
 }

 // Function that creates two co-routines.
 void vOtherFunction( void )
 {
 uint8_t ucParameterToPass;
 TaskHandle_t xHandle;

     // Create two co-routines at priority 0.  The first is given index 0
     // so (from the code above) toggles LED 5 every 200 ticks.  The second
     // is given index 1 so toggles LED 6 every 400 ticks.
     for( uxIndex = 0; uxIndex < 2; uxIndex++ )
     {
         xCoRoutineCreate( vFlashCoRoutine, 0, uxIndex );
     }
 }
   </pre>
 * \defgroup xCoRoutineCreate xCoRoutineCreate
 * \ingroup Tasks
 */
BaseType_t xCoRoutineCreate( crCOROUTINE_CODE pxCoRoutineCode, UBaseType_t uxPriority, UBaseType_t uxIndex );


/**
 * croutine. h
 *<pre>
 void vCoRoutineSchedule( void );</pre>
 *
 * Run a co-routine.
 *
 * vCoRoutineSchedule() executes the highest priority co-routine that is able
 * to run.  The co-routine will execute until it either blocks, yields or is
 * preempted by a task.  Co-routines execute cooperatively so one
 * co-routine cannot be preempted by another, but can be preempted by a task.
 *
 * If an application comprises of both tasks and co-routines then
 * vCoRoutineSchedule should be called from the idle task (in an idle task
 * hook).
 *
 * Example usage:
   <pre>
 // This idle task hook will schedule a co-routine each time it is called.
 // The rest of the idle task will execute between co-routine calls.
 void vApplicationIdleHook( void )
 {
	vCoRoutineSchedule();
 }

 // Alternatively, if you do not require any other part of the idle task to
 // execute, the idle task hook can call vCoRoutineScheduler() within an
 // infinite loop.
 void vApplicationIdleHook( void )
 {
    for( ;; )
    {
        vCoRoutineSchedule();
    }
 }
 </pre>
 * \defgroup vCoRoutineSchedule vCoRoutineSchedule
 * \ingroup Tasks
 */
void vCoRoutineSchedule( void );

/**
 * croutine. h
 * <pre>
 crSTART( CoRoutineHandle_t xHandle );</pre>
 *
 * This macro MUST always be called at the start of a co-routine function.
 *
 * Example usage:
   <pre>
 // Co-routine to be created.
 void vACoRoutine( CoRoutineHandle_t xHandle, UBaseType_t uxIndex )
 {
 // Variables in co-routines must be declared static if they must maintain value across a blocking call.
 static int32_t ulAVariable;

     // Must start every co-routine with a call to crSTART();
     crSTART( xHandle );

     for( ;; )
     {
          // Co-routine functionality goes here.
     }

     // Must end every co-routine with a call to crEND();
     crEND();
 }</pre>
 * \defgroup crSTART crSTART
 * \ingroup Tasks
 */
#define crSTART( pxCRCB ) switch( ( ( CRCB_t * )( pxCRCB ) )->uxState ) { case 0:

/**
 * croutine. h
 * <pre>
 crEND();</pre>
 *
 * This macro MUST always be called at the end of a co-routine function.
 *
 * Example usage:
   <pre>
 // Co-routine to be created.
 void vACoRoutine( CoRoutineHandle_t xHandle, UBaseType_t uxIndex )
 {
 // Variables in co-routines must be declared static if they must maintain value across a blocking call.
 static int32_t ulAVariable;

     // Must start every co-routine with a call to crSTART();
     crSTART( xHandle );

     for( ;; )
     {
          // Co-routine functionality goes here.
     }

     // Must end every co-routine with a call to crEND();
     crEND();
 }</pre>
 * \defgroup crSTART crSTART
 * \ingroup Tasks
 */
#define crEND() }

/*
 * These macros are intended for internal use by the co-routine implementation
 * only.  The macros should not be used directly by application writers.
 */
#define crSET_STATE0( xHandle ) ( ( CRCB_t * )( xHandle ) )->uxState = (__LINE__ * 2); return; case (__LINE__ * 2):
#define crSET_STATE1( xHandle ) ( ( CRCB_t * )( xHandle ) )->uxState = ((__LINE__ * 2)+1); return; case ((__LINE__ * 2)+1):

/**
 * croutine. h
 *<pre>
 crDELAY( CoRoutineHandle_t xHandle, TickType_t xTicksToDelay );</pre>
 *
 * Delay a co-routine for a fixed period of time.
 *
 * crDELAY can only be called from the co-routine function itself - not
 * from within a function called by the co-routine function.  This is because
 * co-routines do not maintain their own stack.
 *
 * @param xHandle The handle of the co-routine to delay.  This is the xHandle
 * parameter of the co-routine function.
 *
 * @param xTickToDelay The number of ticks that the co-routine should delay
 * for.  The actual amount of time this equates to is defined by
 * configTICK_RATE_HZ (set in FreeRTOSConfig.h).  The constant portTICK_PERIOD_MS
 * can be used to convert ticks to milliseconds.
 *
 * Example usage:
   <pre>
 // Co-routine to be created.
 void vACoRoutine( CoRoutineHandle_t xHandle, UBaseType_t uxIndex )
 {
 // Variables in co-routines must be declared static if they must maintain value across a blocking call.
 // This may not be necessary for const variables.
 // We are to delay for 200ms.
 static const xTickType xDelayTime = 200 / portTICK_PERIOD_MS;

     // Must start every co-routine with a call to crSTART();
     crSTART( xHandle );

     for( ;; )
     {
        // Delay for 200ms.
        crDELAY( xHandle, xDelayTime );

        // Do something here.
     }

     // Must end every co-routine with a call to crEND();
     crEND();
 }</pre>
 * \defgroup crDELAY crDELAY
 * \ingroup Tasks
 */
#define crDELAY( xHandle, xTicksToDelay )												\
	if( ( xTicksToDelay ) > 0 )															\
	{																					\
		vCoRoutineAddToDelayedList( ( xTicksToDelay ), NULL );							\
	}																					\
	crSET_STATE0( ( xHandle ) );

/**
 * <pre>
 crQUEUE_SEND(
                  CoRoutineHandle_t xHandle,
                  QueueHandle_t pxQueue,
                  void *pvItemToQueue,
                  TickType_t xTicksToWait,
                  BaseType_t *pxResult
             )</pre>
 *
 * The macro's crQUEUE_SEND() and crQUEUE_RECEIVE() are the co-routine
 * equivalent to the xQueueSend() and xQueueReceive() functions used by tasks.
 *
 * crQUEUE_SEND and crQUEUE_RECEIVE can only be used from a co-routine whereas
 * xQueueSend() and xQueueReceive() can only be used from tasks.
 *
 * crQUEUE_SEND can only be called from the co-routine function itself - not
 * from within a function called by the co-routine function.  This is because
 * co-routines do not maintain their own stack.
 *
 * See the co-routine section of the WEB documentation for information on
 * passing data between tasks and co-routines and between ISR's and
 * co-routines.
 *
 * @param xHandle The handle of the calling co-routine.  This is the xHandle
 * parameter of the co-routine function.
 *
 * @param pxQueue The handle of the queue on which the data will be posted.
 * The handle is obtained as the return value when the queue is created using
 * the xQueueCreate() API function.
 *
 * @param pvItemToQueue A pointer to the data being posted onto the queue.
 * The number of bytes of each queued item is specified when the queue is
 * created.  This number of bytes is copied from pvItemToQueue into the queue
 * itself.
 *
 * @param xTickToDelay The number of ticks that the co-routine should block
 * to wait for space to become available on the queue, should space not be
 * available immediately. The actual amount of time this equates to is defined
 * by configTICK_RATE_HZ (set in FreeRTOSConfig.h).  The constant
 * portTICK_PERIOD_MS can be used to convert ticks to milliseconds (see example
 * below).
 *
 * @param pxResult The variable pointed to by pxResult will be set to pdPASS if
 * data was successfully posted onto the queue, otherwise it will be set to an
 * error defined within ProjDefs.h.
 *
 * Example usage:
   <pre>
 // Co-routine function that blocks for a fixed period then posts a number onto
 // a queue.
 static void prvCoRoutineFlashTask( CoRoutineHandle_t xHandle, UBaseType_t uxIndex )
 {
 // Variables in co-routines must be declared static if they must maintain value across a blocking call.
 static BaseType_t xNumberToPost = 0;
 static BaseType_t xResult;

    // Co-routines must begin with a call to crSTART().
    crSTART( xHandle );

    for( ;; )
    {
        // This assumes the queue has already been created.
        crQUEUE_SEND( xHandle, xCoRoutineQueue, &xNumberToPost, NO_DELAY, &xResult );

        if( xResult != pdPASS )
        {
            // The message was not posted!
        }

        // Increment the number to be posted onto the queue.
        xNumberToPost++;

        // Delay for 100 ticks.
        crDELAY( xHandle, 100 );
    }

    // Co-routines must end with a call to crEND().
    crEND();
 }</pre>
 * \defgroup crQUEUE_SEND crQUEUE_SEND
 * \ingroup Tasks
 */
#define crQUEUE_SEND( xHandle, pxQueue, pvItemToQueue, xTicksToWait, pxResult )			\
{																						\
	*( pxResult ) = xQueueCRSend( ( pxQueue) , ( pvItemToQueue) , ( xTicksToWait ) );	\
	if( *( pxResult ) == errQUEUE_BLOCKED )												\
	{																					\
		crSET_STATE0( ( xHandle ) );													\
		*pxResult = xQueueCRSend( ( pxQueue ), ( pvItemToQueue ), 0 );					\
	}																					\
	if( *pxResult == errQUEUE_YIELD )													\
	{																					\
		crSET_STATE1( ( xHandle ) );													\
		*pxResult = pdPASS;																\
	}																					\
}

/**
 * croutine. h
 * <pre>
  crQUEUE_RECEIVE(
                     CoRoutineHandle_t xHandle,
                     QueueHandle_t pxQueue,
                     void *pvBuffer,
                     TickType_t xTicksToWait,
                     BaseType_t *pxResult
                 )</pre>
 *
 * The macro's crQUEUE_SEND() and crQUEUE_RECEIVE() are the co-routine
 * equivalent to the xQueueSend() and xQueueReceive() functions used by tasks.
 *
 * crQUEUE_SEND and crQUEUE_RECEIVE can only be used from a co-routine whereas
 * xQueueSend() and xQueueReceive() can only be used from tasks.
 *
 * crQUEUE_RECEIVE can only be called from the co-routine function itself - not
 * from within a function called by the co-routine function.  This is because
 * co-routines do not maintain their own stack.
 *
 * See the co-routine section of the WEB documentation for information on
 * passing data between tasks and co-routines and between ISR's and
 * co-routines.
 *
 * @param xHandle The handle of the calling co-routine.  This is the xHandle
 * parameter of the co-routine function.
 *
 * @param pxQueue The handle of the queue from which the data will be received.
 * The handle is obtained as the return value when the queue is created using
 * the xQueueCreate() API function.
 *
 * @param pvBuffer The buffer into which the received item is to be copied.
 * The number of bytes of each queued item is specified when the queue is
 * created.  This number of bytes is copied into pvBuffer.
 *
 * @param xTickToDelay The number of ticks that the co-routine should block
 * to wait for data to become available from the queue, should data not be
 * available immediately. The actual amount of time this equates to is defined
 * by configTICK_RATE_HZ (set in FreeRTOSConfig.h).  The constant
 * portTICK_PERIOD_MS can be used to convert ticks to milliseconds (see the
 * crQUEUE_SEND example).
 *
 * @param pxResult The variable pointed to by pxResult will be set to pdPASS if
 * data was successfully retrieved from the queue, otherwise it will be set to
 * an error code as defined within ProjDefs.h.
 *
 * Example usage:
 <pre>
 // A co-routine receives the number of an LED to flash from a queue.  It
 // blocks on the queue until the number is received.
 static void prvCoRoutineFlashWorkTask( CoRoutineHandle_t xHandle, UBaseType_t uxIndex )
 {
 // Variables in co-routines must be declared static if they must maintain value across a blocking call.
 static BaseType_t xResult;
 static UBaseType_t uxLEDToFlash;

    // All co-routines must start with a call to crSTART().
    crSTART( xHandle );

    for( ;; )
    {
        // Wait for data to become available on the queue.
        crQUEUE_RECEIVE( xHandle, xCoRoutineQueue, &uxLEDToFlash, portMAX_DELAY, &xResult );

        if( xResult == pdPASS )
        {
            // We received the LED to flash - flash it!
            vParTestToggleLED( uxLEDToFlash );
        }
    }

    crEND();
 }</pre>
 * \defgroup crQUEUE_RECEIVE crQUEUE_RECEIVE
 * \ingroup Tasks
 */
#define crQUEUE_RECEIVE( xHandle, pxQueue, pvBuffer, xTicksToWait, pxResult )			\
{																						\
	*( pxResult ) = xQueueCRReceive( ( pxQueue) , ( pvBuffer ), ( xTicksToWait ) );		\
	if( *( pxResult ) == errQUEUE_BLOCKED ) 											\
	{																					\
		crSET_STATE0( ( xHandle ) );													\
		*( pxResult ) = xQueueCRReceive( ( pxQueue) , ( pvBuffer ), 0 );				\
	}																					\
	if( *( pxResult ) == errQUEUE_YIELD )												\
	{																					\
		crSET_STATE1( ( xHandle ) );													\
		*( pxResult ) = pdPASS;															\
	}																					\
}

/**
 * croutine. h
 * <pre>
  crQUEUE_SEND_FROM_ISR(
                            QueueHandle_t pxQueue,
                            void *pvItemToQueue,
                            BaseType_t xCoRoutinePreviouslyWoken
                       )</pre>
 *
 * The macro's crQUEUE_SEND_FROM_ISR() and crQUEUE_RECEIVE_FROM_ISR() are the
 * co-routine equivalent to the xQueueSendFromISR() and xQueueReceiveFromISR()
 * functions used by tasks.
 *
 * crQUEUE_SEND_FROM_ISR() and crQUEUE_RECEIVE_FROM_ISR() can only be used to
 * pass data between a co-routine and and ISR, whereas xQueueSendFromISR() and
 * xQueueReceiveFromISR() can only be used to pass data between a task and and
 * ISR.
 *
 * crQUEUE_SEND_FROM_ISR can only be called from an ISR to send data to a queue
 * that is being used from within a co-routine.
 *
 * See the co-routine section of the WEB documentation for information on
 * passing data between tasks and co-routines and between ISR's and
 * co-routines.
 *
 * @param xQueue The handle to the queue on which the item is to be posted.
 *
 * @param pvItemToQueue A pointer to the item that is to be placed on the
 * queue.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from pvItemToQueue
 * into the queue storage area.
 *
 * @param xCoRoutinePreviouslyWoken This is included so an ISR can post onto
 * the same queue multiple times from a single interrupt.  The first call
 * should always pass in pdFALSE.  Subsequent calls should pass in
 * the value returned from the previous call.
 *
 * @return pdTRUE if a co-routine was woken by posting onto the queue.  This is
 * used by the ISR to determine if a context switch may be required following
 * the ISR.
 *
 * Example usage:
 <pre>
 // A co-routine that blocks on a queue waiting for characters to be received.
 static void vReceivingCoRoutine( CoRoutineHandle_t xHandle, UBaseType_t uxIndex )
 {
 char cRxedChar;
 BaseType_t xResult;

     // All co-routines must start with a call to crSTART().
     crSTART( xHandle );

     for( ;; )
     {
         // Wait for data to become available on the queue.  This assumes the
         // queue xCommsRxQueue has already been created!
         crQUEUE_RECEIVE( xHandle, xCommsRxQueue, &uxLEDToFlash, portMAX_DELAY, &xResult );

         // Was a character received?
         if( xResult == pdPASS )
         {
             // Process the character here.
         }
     }

     // All co-routines must end with a call to crEND().
     crEND();
 }

 // An ISR that uses a queue to send characters received on a serial port to
 // a co-routine.
 void vUART_ISR( void )
 {
 char cRxedChar;
 BaseType_t xCRWokenByPost = pdFALSE;

     // We loop around reading characters until there are none left in the UART.
     while( UART_RX_REG_NOT_EMPTY() )
     {
         // Obtain the character from the UART.
         cRxedChar = UART_RX_REG;

         // Post the character onto a queue.  xCRWokenByPost will be pdFALSE
         // the first time around the loop.  If the post causes a co-routine
         // to be woken (unblocked) then xCRWokenByPost will be set to pdTRUE.
         // In this manner we can ensure that if more than one co-routine is
         // blocked on the queue only one is woken by this ISR no matter how
         // many characters are posted to the queue.
         xCRWokenByPost = crQUEUE_SEND_FROM_ISR( xCommsRxQueue, &cRxedChar, xCRWokenByPost );
     }
 }</pre>
 * \defgroup crQUEUE_SEND_FROM_ISR crQUEUE_SEND_FROM_ISR
 * \ingroup Tasks
 */
#define crQUEUE_SEND_FROM_ISR( pxQueue, pvItemToQueue, xCoRoutinePreviouslyWoken ) xQueueCRSendFromISR( ( pxQueue ), ( pvItemToQueue ), ( xCoRoutinePreviouslyWoken ) )


/**
 * croutine. h
 * <pre>
  crQUEUE_SEND_FROM_ISR(
                            QueueHandle_t pxQueue,
                            void *pvBuffer,
                            BaseType_t * pxCoRoutineWoken
                       )</pre>
 *
 * The macro's crQUEUE_SEND_FROM_ISR() and crQUEUE_RECEIVE_FROM_ISR() are the
 * co-routine equivalent to the xQueueSendFromISR() and xQueueReceiveFromISR()
 * functions used by tasks.
 *
 * crQUEUE_SEND_FROM_ISR() and crQUEUE_RECEIVE_FROM_ISR() can only be used to
 * pass data between a co-routine and and ISR, whereas xQueueSendFromISR() and
 * xQueueReceiveFromISR() can only be used to pass data between a task and and
 * ISR.
 *
 * crQUEUE_RECEIVE_FROM_ISR can only be called from an ISR to receive data
 * from a queue that is being used from within a co-routine (a co-routine
 * posted to the queue).
 *
 * See the co-routine section of the WEB documentation for information on
 * passing data between tasks and co-routines and between ISR's and
 * co-routines.
 *
 * @param xQueue The handle to the queue on which the item is to be posted.
 *
 * @param pvBuffer A pointer to a buffer into which the received item will be
 * placed.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from the queue into
 * pvBuffer.
 *
 * @param pxCoRoutineWoken A co-routine may be blocked waiting for space to become
 * available on the queue.  If crQUEUE_RECEIVE_FROM_ISR causes such a
 * co-routine to unblock *pxCoRoutineWoken will get set to pdTRUE, otherwise
 * *pxCoRoutineWoken will remain unchanged.
 *
 * @return pdTRUE an item was successfully received from the queue, otherwise
 * pdFALSE.
 *
 * Example usage:
 <pre>
 // A co-routine that posts a character to a queue then blocks for a fixed
 // period.  The character is incremented each time.
 static void vSendingCoRoutine( CoRoutineHandle_t xHandle, UBaseType_t uxIndex )
 {
 // cChar holds its value while this co-routine is blocked and must therefore
 // be declared static.
 static char cCharToTx = 'a';
 BaseType_t xResult;

     // All co-routines must start with a call to crSTART().
     crSTART( xHandle );

     for( ;; )
     {
         // Send the next character to the queue.
         crQUEUE_SEND( xHandle, xCoRoutineQueue, &cCharToTx, NO_DELAY, &xResult );

         if( xResult == pdPASS )
         {
             // The character was successfully posted to the queue.
         }
		 else
		 {
			// Could not post the character to the queue.
		 }

         // Enable the UART Tx interrupt to cause an interrupt in this
		 // hypothetical UART.  The interrupt will obtain the character
		 // from the queue and send it.
		 ENABLE_RX_INTERRUPT();

		 // Increment to the next character then block for a fixed period.
		 // cCharToTx will maintain its value across the delay as it is
		 // declared static.
		 cCharToTx++;
		 if( cCharToTx > 'x' )
		 {
			cCharToTx = 'a';
		 }
		 crDELAY( 100 );
     }

     // All co-routines must end with a call to crEND().
     crEND();
 }

 // An ISR that uses a queue to receive characters to send on a UART.
 void vUART_ISR( void )
 {
 char cCharToTx;
 BaseType_t xCRWokenByPost = pdFALSE;

     while( UART_TX_REG_EMPTY() )
     {
         // Are there any characters in the queue waiting to be sent?
		 // xCRWokenByPost will automatically be set to pdTRUE if a co-routine
		 // is woken by the post - ensuring that only a single co-routine is
		 // woken no matter how many times we go around this loop.
         if( crQUEUE_RECEIVE_FROM_ISR( pxQueue, &cCharToTx, &xCRWokenByPost ) )
		 {
			 SEND_CHARACTER( cCharToTx );
		 }
     }
 }</pre>
 * \defgroup crQUEUE_RECEIVE_FROM_ISR crQUEUE_RECEIVE_FROM_ISR
 * \ingroup Tasks
 */
#define crQUEUE_RECEIVE_FROM_ISR( pxQueue, pvBuffer, pxCoRoutineWoken ) xQueueCRReceiveFromISR( ( pxQueue ), ( pvBuffer ), ( pxCoRoutineWoken ) )

/*
 * This function is intended for internal use by the co-routine macros only.
 * The macro nature of the co-routine implementation requires that the
 * prototype appears here.  The function should not be used by application
 * writers.
 *
 * Removes the current co-routine from its ready list and places it in the
 * appropriate delayed list.
 */
void vCoRoutineAddToDelayedList( TickType_t xTicksToDelay, List_t *pxEventList );

/*
 * This function is intended for internal use by the queue implementation only.
 * The function should not be used by application writers.
 *
 * Removes the highest priority co-routine from the event list and places it in
 * the pending ready list.
 */
BaseType_t xCoRoutineRemoveFromEventList( const List_t *pxEventList );

#ifdef __cplusplus
}
#endif

#endif /* CO_ROUTINE_H */
