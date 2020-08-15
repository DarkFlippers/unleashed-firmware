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

#ifndef PROJDEFS_H
#define PROJDEFS_H

/*
 * Defines the prototype to which task functions must conform.  Defined in this
 * file to ensure the type is known before portable.h is included.
 */
typedef void (*TaskFunction_t)( void * );

/* Converts a time in milliseconds to a time in ticks.  This macro can be
overridden by a macro of the same name defined in FreeRTOSConfig.h in case the
definition here is not suitable for your application. */
#ifndef pdMS_TO_TICKS
	#define pdMS_TO_TICKS( xTimeInMs ) ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeInMs ) * ( TickType_t ) configTICK_RATE_HZ ) / ( TickType_t ) 1000 ) )
#endif

#define pdFALSE			( ( BaseType_t ) 0 )
#define pdTRUE			( ( BaseType_t ) 1 )

#define pdPASS			( pdTRUE )
#define pdFAIL			( pdFALSE )
#define errQUEUE_EMPTY	( ( BaseType_t ) 0 )
#define errQUEUE_FULL	( ( BaseType_t ) 0 )

/* FreeRTOS error definitions. */
#define errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY	( -1 )
#define errQUEUE_BLOCKED						( -4 )
#define errQUEUE_YIELD							( -5 )

/* Macros used for basic data corruption checks. */
#ifndef configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES
	#define configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES 0
#endif

#if( configUSE_16_BIT_TICKS == 1 )
	#define pdINTEGRITY_CHECK_VALUE 0x5a5a
#else
	#define pdINTEGRITY_CHECK_VALUE 0x5a5a5a5aUL
#endif

/* The following errno values are used by FreeRTOS+ components, not FreeRTOS
itself. */
#define pdFREERTOS_ERRNO_NONE			0	/* No errors */
#define	pdFREERTOS_ERRNO_ENOENT			2	/* No such file or directory */
#define	pdFREERTOS_ERRNO_EINTR			4	/* Interrupted system call */
#define	pdFREERTOS_ERRNO_EIO			5	/* I/O error */
#define	pdFREERTOS_ERRNO_ENXIO			6	/* No such device or address */
#define	pdFREERTOS_ERRNO_EBADF			9	/* Bad file number */
#define	pdFREERTOS_ERRNO_EAGAIN			11	/* No more processes */
#define	pdFREERTOS_ERRNO_EWOULDBLOCK	11	/* Operation would block */
#define	pdFREERTOS_ERRNO_ENOMEM			12	/* Not enough memory */
#define	pdFREERTOS_ERRNO_EACCES			13	/* Permission denied */
#define	pdFREERTOS_ERRNO_EFAULT			14	/* Bad address */
#define	pdFREERTOS_ERRNO_EBUSY			16	/* Mount device busy */
#define	pdFREERTOS_ERRNO_EEXIST			17	/* File exists */
#define	pdFREERTOS_ERRNO_EXDEV			18	/* Cross-device link */
#define	pdFREERTOS_ERRNO_ENODEV			19	/* No such device */
#define	pdFREERTOS_ERRNO_ENOTDIR		20	/* Not a directory */
#define	pdFREERTOS_ERRNO_EISDIR			21	/* Is a directory */
#define	pdFREERTOS_ERRNO_EINVAL			22	/* Invalid argument */
#define	pdFREERTOS_ERRNO_ENOSPC			28	/* No space left on device */
#define	pdFREERTOS_ERRNO_ESPIPE			29	/* Illegal seek */
#define	pdFREERTOS_ERRNO_EROFS			30	/* Read only file system */
#define	pdFREERTOS_ERRNO_EUNATCH		42	/* Protocol driver not attached */
#define	pdFREERTOS_ERRNO_EBADE			50	/* Invalid exchange */
#define	pdFREERTOS_ERRNO_EFTYPE			79	/* Inappropriate file type or format */
#define	pdFREERTOS_ERRNO_ENMFILE		89	/* No more files */
#define	pdFREERTOS_ERRNO_ENOTEMPTY		90	/* Directory not empty */
#define	pdFREERTOS_ERRNO_ENAMETOOLONG 	91	/* File or path name too long */
#define	pdFREERTOS_ERRNO_EOPNOTSUPP		95	/* Operation not supported on transport endpoint */
#define	pdFREERTOS_ERRNO_ENOBUFS		105	/* No buffer space available */
#define	pdFREERTOS_ERRNO_ENOPROTOOPT	109	/* Protocol not available */
#define	pdFREERTOS_ERRNO_EADDRINUSE		112	/* Address already in use */
#define	pdFREERTOS_ERRNO_ETIMEDOUT		116	/* Connection timed out */
#define	pdFREERTOS_ERRNO_EINPROGRESS	119	/* Connection already in progress */
#define	pdFREERTOS_ERRNO_EALREADY		120	/* Socket already connected */
#define	pdFREERTOS_ERRNO_EADDRNOTAVAIL 	125	/* Address not available */
#define	pdFREERTOS_ERRNO_EISCONN		127	/* Socket is already connected */
#define	pdFREERTOS_ERRNO_ENOTCONN		128	/* Socket is not connected */
#define	pdFREERTOS_ERRNO_ENOMEDIUM		135	/* No medium inserted */
#define	pdFREERTOS_ERRNO_EILSEQ			138	/* An invalid UTF-16 sequence was encountered. */
#define	pdFREERTOS_ERRNO_ECANCELED		140	/* Operation canceled. */

/* The following endian values are used by FreeRTOS+ components, not FreeRTOS
itself. */
#define pdFREERTOS_LITTLE_ENDIAN		0
#define pdFREERTOS_BIG_ENDIAN			1

/* Re-defining endian values for generic naming. */
#define pdLITTLE_ENDIAN					pdFREERTOS_LITTLE_ENDIAN
#define pdBIG_ENDIAN					pdFREERTOS_BIG_ENDIAN


#endif /* PROJDEFS_H */



