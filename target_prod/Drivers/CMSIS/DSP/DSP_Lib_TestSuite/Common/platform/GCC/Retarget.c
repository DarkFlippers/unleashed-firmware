/*----------------------------------------------------------------------------
 * Name:    Retarget.c
 * Purpose: 'Retarget' layer for target-dependent low level functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2012 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <sys/stat.h>
#include <string.h>
#include <errno.h>

int SER_PutChar (int c) {

  return (c);
}

int SER_GetChar (void) {

  return (-1);
}

/*-- GCC - Newlib runtime support --------------------------------------------*/

extern int  __HeapBase;
extern int  __HeapLimit;

int _open (const char * path, int flags, ...) 
{
  return (-1);
}

int _close (int fd) 
{
  return (-1);
}

int _lseek (int fd, int ptr, int dir) 
{
  return (0);
}

int __attribute__((weak)) _fstat (int fd, struct stat * st) 
{
  memset (st, 0, sizeof (* st));
  st->st_mode = S_IFCHR;
  return (0);
}

int _isatty (int fd) 
{
  return (1);
}

int _read (int fd, char * ptr, int len) 
{
  char c;
  int  i;

  for (i = 0; i < len; i++) 
  {
    c = SER_GetChar();
    if (c == 0x0D) break;
    *ptr++ = c;
    SER_PutChar(c);
  }
  return (len - i);
}

int _write (int fd, char * ptr, int len) 
{
  int i;

  for (i = 0; i < len; i++) SER_PutChar (*ptr++);
  return (i);
}

caddr_t _sbrk (int incr) 
{
  static char * heap;
         char * prev_heap;

  if (heap == NULL) 
  {
    heap = (char *)&__HeapBase;
  }
  
  prev_heap = heap;

  if ((heap + incr) > (char *)&__HeapLimit) 
  {
    errno = ENOMEM;
    return (caddr_t) -1;
  }
  
  heap += incr;

  return (caddr_t) prev_heap;
}
