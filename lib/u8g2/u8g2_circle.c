/*

  u8g2_circle.c

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  

*/

#include "u8g2.h"

/*==============================================*/
/* Circle */

static void u8g2_draw_circle_section(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t x0, u8g2_uint_t y0, uint8_t option) U8G2_NOINLINE;

static void u8g2_draw_circle_section(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t x0, u8g2_uint_t y0, uint8_t option)
{
    /* upper right */
    if ( option & U8G2_DRAW_UPPER_RIGHT )
    {
      u8g2_DrawPixel(u8g2, x0 + x, y0 - y);
      u8g2_DrawPixel(u8g2, x0 + y, y0 - x);
    }
    
    /* upper left */
    if ( option & U8G2_DRAW_UPPER_LEFT )
    {
      u8g2_DrawPixel(u8g2, x0 - x, y0 - y);
      u8g2_DrawPixel(u8g2, x0 - y, y0 - x);
    }
    
    /* lower right */
    if ( option & U8G2_DRAW_LOWER_RIGHT )
    {
      u8g2_DrawPixel(u8g2, x0 + x, y0 + y);
      u8g2_DrawPixel(u8g2, x0 + y, y0 + x);
    }
    
    /* lower left */
    if ( option & U8G2_DRAW_LOWER_LEFT )
    {
      u8g2_DrawPixel(u8g2, x0 - x, y0 + y);
      u8g2_DrawPixel(u8g2, x0 - y, y0 + x);
    }
}

static void u8g2_draw_circle(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad, uint8_t option)
{
    u8g2_int_t f;
    u8g2_int_t ddF_x;
    u8g2_int_t ddF_y;
    u8g2_uint_t x;
    u8g2_uint_t y;

    f = 1;
    f -= rad;
    ddF_x = 1;
    ddF_y = 0;
    ddF_y -= rad;
    ddF_y *= 2;
    x = 0;
    y = rad;

    u8g2_draw_circle_section(u8g2, x, y, x0, y0, option);
    
    while ( x < y )
    {
      if (f >= 0) 
      {
        y--;
        ddF_y += 2;
        f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;

      u8g2_draw_circle_section(u8g2, x, y, x0, y0, option);    
    }
}

void u8g2_DrawCircle(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad, uint8_t option)
{
  /* check for bounding box */
#ifdef U8G2_WITH_INTERSECTION
  {
    if ( u8g2_IsIntersection(u8g2, x0-rad, y0-rad, x0+rad+1, y0+rad+1) == 0 ) 
      return;
  }
#endif /* U8G2_WITH_INTERSECTION */
  
  
  /* draw circle */
  u8g2_draw_circle(u8g2, x0, y0, rad, option);
}

/*==============================================*/
/* Disk */

static void u8g2_draw_disc_section(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t x0, u8g2_uint_t y0, uint8_t option) U8G2_NOINLINE;

static void u8g2_draw_disc_section(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t x0, u8g2_uint_t y0, uint8_t option)
{
    /* upper right */
    if ( option & U8G2_DRAW_UPPER_RIGHT )
    {
      u8g2_DrawVLine(u8g2, x0+x, y0-y, y+1);
      u8g2_DrawVLine(u8g2, x0+y, y0-x, x+1);
    }
    
    /* upper left */
    if ( option & U8G2_DRAW_UPPER_LEFT )
    {
      u8g2_DrawVLine(u8g2, x0-x, y0-y, y+1);
      u8g2_DrawVLine(u8g2, x0-y, y0-x, x+1);
    }
    
    /* lower right */
    if ( option & U8G2_DRAW_LOWER_RIGHT )
    {
      u8g2_DrawVLine(u8g2, x0+x, y0, y+1);
      u8g2_DrawVLine(u8g2, x0+y, y0, x+1);
    }
    
    /* lower left */
    if ( option & U8G2_DRAW_LOWER_LEFT )
    {
      u8g2_DrawVLine(u8g2, x0-x, y0, y+1);
      u8g2_DrawVLine(u8g2, x0-y, y0, x+1);
    }
}

static void u8g2_draw_disc(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad, uint8_t option)
{
  u8g2_int_t f;
  u8g2_int_t ddF_x;
  u8g2_int_t ddF_y;
  u8g2_uint_t x;
  u8g2_uint_t y;

  f = 1;
  f -= rad;
  ddF_x = 1;
  ddF_y = 0;
  ddF_y -= rad;
  ddF_y *= 2;
  x = 0;
  y = rad;

  u8g2_draw_disc_section(u8g2, x, y, x0, y0, option);
  
  while ( x < y )
  {
    if (f >= 0) 
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    u8g2_draw_disc_section(u8g2, x, y, x0, y0, option);    
  }
}

void u8g2_DrawDisc(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad, uint8_t option)
{
  /* check for bounding box */
#ifdef U8G2_WITH_INTERSECTION
  {
    if ( u8g2_IsIntersection(u8g2, x0-rad, y0-rad, x0+rad+1, y0+rad+1) == 0 ) 
      return;
  }
#endif /* U8G2_WITH_INTERSECTION */
  
  /* draw disc */
  u8g2_draw_disc(u8g2, x0, y0, rad, option);
}

/*==============================================*/
/* Ellipse */

/*
  Source: 
    Foley, Computer Graphics, p 90
*/
static void u8g2_draw_ellipse_section(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t x0, u8g2_uint_t y0, uint8_t option) U8G2_NOINLINE;
static void u8g2_draw_ellipse_section(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t x0, u8g2_uint_t y0, uint8_t option)
{
    /* upper right */
    if ( option & U8G2_DRAW_UPPER_RIGHT )
    {
      u8g2_DrawPixel(u8g2, x0 + x, y0 - y);
    }
    
    /* upper left */
    if ( option & U8G2_DRAW_UPPER_LEFT )
    {
      u8g2_DrawPixel(u8g2, x0 - x, y0 - y);
    }
    
    /* lower right */
    if ( option & U8G2_DRAW_LOWER_RIGHT )
    {
      u8g2_DrawPixel(u8g2, x0 + x, y0 + y);
    }
    
    /* lower left */
    if ( option & U8G2_DRAW_LOWER_LEFT )
    {
      u8g2_DrawPixel(u8g2, x0 - x, y0 + y);
    }
}

static void u8g2_draw_ellipse(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rx, u8g2_uint_t ry, uint8_t option)
{
  u8g2_uint_t x, y;
  u8g2_long_t xchg, ychg;
  u8g2_long_t err;
  u8g2_long_t rxrx2;
  u8g2_long_t ryry2;
  u8g2_long_t stopx, stopy;
  
  rxrx2 = rx;
  rxrx2 *= rx;
  rxrx2 *= 2;
  
  ryry2 = ry;
  ryry2 *= ry;
  ryry2 *= 2;
  
  x = rx;
  y = 0;
  
  xchg = 1;
  xchg -= rx;
  xchg -= rx;
  xchg *= ry;
  xchg *= ry;
  
  ychg = rx;
  ychg *= rx;
  
  err = 0;
  
  stopx = ryry2;
  stopx *= rx;
  stopy = 0;
  
  while( stopx >= stopy )
  {
    u8g2_draw_ellipse_section(u8g2, x, y, x0, y0, option);
    y++;
    stopy += rxrx2;
    err += ychg;
    ychg += rxrx2;
    if ( 2*err+xchg > 0 )
    {
      x--;
      stopx -= ryry2;
      err += xchg;
      xchg += ryry2;      
    }
  }

  x = 0;
  y = ry;
  
  xchg = ry;
  xchg *= ry;
  
  ychg = 1;
  ychg -= ry;
  ychg -= ry;
  ychg *= rx;
  ychg *= rx;
  
  err = 0;
  
  stopx = 0;

  stopy = rxrx2;
  stopy *= ry;
  

  while( stopx <= stopy )
  {
    u8g2_draw_ellipse_section(u8g2, x, y, x0, y0, option);
    x++;
    stopx += ryry2;
    err += xchg;
    xchg += ryry2;
    if ( 2*err+ychg > 0 )
    {
      y--;
      stopy -= rxrx2;
      err += ychg;
      ychg += rxrx2;
    }
  }
  
}

void u8g2_DrawEllipse(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rx, u8g2_uint_t ry, uint8_t option)
{
  /* check for bounding box */
#ifdef U8G2_WITH_INTERSECTION
  {
    if ( u8g2_IsIntersection(u8g2, x0-rx, y0-ry, x0+rx+1, y0+ry+1) == 0 ) 
      return;
  }
#endif /* U8G2_WITH_INTERSECTION */
  
  u8g2_draw_ellipse(u8g2, x0, y0, rx, ry, option);
}

/*==============================================*/
/* Filled Ellipse */

static void u8g2_draw_filled_ellipse_section(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t x0, u8g2_uint_t y0, uint8_t option) U8G2_NOINLINE;
static void u8g2_draw_filled_ellipse_section(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t x0, u8g2_uint_t y0, uint8_t option)
{
    /* upper right */
    if ( option & U8G2_DRAW_UPPER_RIGHT )
    {
      u8g2_DrawVLine(u8g2, x0+x, y0-y, y+1);
    }
    
    /* upper left */
    if ( option & U8G2_DRAW_UPPER_LEFT )
    {
      u8g2_DrawVLine(u8g2, x0-x, y0-y, y+1);
    }
    
    /* lower right */
    if ( option & U8G2_DRAW_LOWER_RIGHT )
    {
      u8g2_DrawVLine(u8g2, x0+x, y0, y+1);
    }
    
    /* lower left */
    if ( option & U8G2_DRAW_LOWER_LEFT )
    {
      u8g2_DrawVLine(u8g2, x0-x, y0, y+1);
    }
}

static void u8g2_draw_filled_ellipse(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rx, u8g2_uint_t ry, uint8_t option)
{
  u8g2_uint_t x, y;
  u8g2_long_t xchg, ychg;
  u8g2_long_t err;
  u8g2_long_t rxrx2;
  u8g2_long_t ryry2;
  u8g2_long_t stopx, stopy;
  
  rxrx2 = rx;
  rxrx2 *= rx;
  rxrx2 *= 2;
  
  ryry2 = ry;
  ryry2 *= ry;
  ryry2 *= 2;
  
  x = rx;
  y = 0;
  
  xchg = 1;
  xchg -= rx;
  xchg -= rx;
  xchg *= ry;
  xchg *= ry;
  
  ychg = rx;
  ychg *= rx;
  
  err = 0;
  
  stopx = ryry2;
  stopx *= rx;
  stopy = 0;
  
  while( stopx >= stopy )
  {
    u8g2_draw_filled_ellipse_section(u8g2, x, y, x0, y0, option);
    y++;
    stopy += rxrx2;
    err += ychg;
    ychg += rxrx2;
    if ( 2*err+xchg > 0 )
    {
      x--;
      stopx -= ryry2;
      err += xchg;
      xchg += ryry2;      
    }
  }

  x = 0;
  y = ry;
  
  xchg = ry;
  xchg *= ry;
  
  ychg = 1;
  ychg -= ry;
  ychg -= ry;
  ychg *= rx;
  ychg *= rx;
  
  err = 0;
  
  stopx = 0;

  stopy = rxrx2;
  stopy *= ry;
  

  while( stopx <= stopy )
  {
    u8g2_draw_filled_ellipse_section(u8g2, x, y, x0, y0, option);
    x++;
    stopx += ryry2;
    err += xchg;
    xchg += ryry2;
    if ( 2*err+ychg > 0 )
    {
      y--;
      stopy -= rxrx2;
      err += ychg;
      ychg += rxrx2;
    }
  }
  
}

void u8g2_DrawFilledEllipse(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rx, u8g2_uint_t ry, uint8_t option)
{
  /* check for bounding box */
#ifdef U8G2_WITH_INTERSECTION
  {
    if ( u8g2_IsIntersection(u8g2, x0-rx, y0-ry, x0+rx+1, y0+ry+1) == 0 ) 
      return;
  }
#endif /* U8G2_WITH_INTERSECTION */
  
  u8g2_draw_filled_ellipse(u8g2, x0, y0, rx, ry, option);
}


