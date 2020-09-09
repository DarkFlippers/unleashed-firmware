/*

  u8g2_hvline.c

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


  Calltree
  
    void u8g2_DrawHVLine(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
    u8g2->cb->draw_l90
    u8g2_draw_hv_line_2dir
    u8g2->ll_hvline(u8g2, x, y, len, dir);
    

*/

#include "u8g2.h"
#include <assert.h>

/*==========================================================*/
/* intersection procedure */

/*
  Description:
    clip range from pos a (included) with line len (a+len excluded) agains c (included) to d (excluded)
  Assumptions:
    len > 0
    c <= d		(this is not checked)
  will return 0 if there is no intersection and if a > b

*/

static uint8_t u8g2_clip_intersection2(u8g2_uint_t *ap, u8g2_uint_t *len, u8g2_uint_t c, u8g2_uint_t d)
{
  u8g2_uint_t a = *ap;
  u8g2_uint_t b;
  b  = a;
  b += *len;

  /*
    Description:
      clip range from a (included) to b (excluded) agains c (included) to d (excluded)
    Assumptions:
      a <= b		(violation is checked and handled correctly)
      c <= d		(this is not checked)
    will return 0 if there is no intersection and if a > b

    optimized clipping: c is set to 0 --> 27 Oct 2018: again removed the c==0 assumption
    
    replaced by uint8_t u8g2_clip_intersection2
  */

  /* handle the a>b case correctly. If code and time is critical, this could */
  /* be removed completly (be aware about memory curruption for wrong */
  /* arguments) or return 0 for a>b (will lead to skipped lines for wrong */
  /* arguments) */  
  
  /* removing the following if clause completly may lead to memory corruption of a>b */
  if ( a > b )
  {    
    /* replacing this if with a simple "return 0;" will not handle the case with negative a */    
    if ( a < d )
    {
      b = d;
      b--;
    }
    else
    {
      a = c;
    }
  }
  
  /* from now on, the asumption a <= b is ok */
  
  if ( a >= d )
    return 0;
  if ( b <= c )
    return 0;
  if ( a < c )		
    a = c;
  if ( b > d )
    b = d;
  
  *ap = a;
  b -= a;
  *len = b;
  return 1;
}



/*==========================================================*/
/* draw procedures */

/*
  x,y		Upper left position of the line within the pixel buffer 
  len		length of the line in pixel, len must not be 0
  dir		0: horizontal line (left to right)
		1: vertical line (top to bottom)
  This function first adjusts the y position to the local buffer. Then it
  will clip the line and call u8g2_draw_low_level_hv_line()

*/
void u8g2_draw_hv_line_2dir(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{

  /* clipping happens before the display rotation */

  /* transform to pixel buffer coordinates */
  y -= u8g2->pixel_curr_row;
  
  u8g2->ll_hvline(u8g2, x, y, len, dir);
}


/*
  This is the toplevel function for the hv line draw procedures.
  This function should be called by the user.
  
  "dir" may have 4 directions: 0 (left to right), 1, 2, 3 (down up)
*/
void u8g2_DrawHVLine(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{
  /* Make a call to the callback function (e.g. u8g2_draw_l90_r0). */
  /* The callback may rotate the hv line */
  /* after rotation this will call u8g2_draw_hv_line_4dir() */
  
#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT
  if ( u8g2->is_page_clip_window_intersection != 0 )
#endif /* U8G2_WITH_CLIP_WINDOW_SUPPORT */
    if ( len != 0 )
    {
    
      /* convert to two directions */    
      if ( len > 1 )
      {
	if ( dir == 2 )
	{
	  x -= len;
	  x++;
	}
	else if ( dir == 3 )
	{
	  y -= len;
	  y++;
	}
      }
      dir &= 1;  
      
      /* clip against the user window */
      if ( dir == 0 )
      {
	if ( y < u8g2->user_y0 )
	  return;
	if ( y >= u8g2->user_y1 )
	  return;
	if ( u8g2_clip_intersection2(&x, &len, u8g2->user_x0, u8g2->user_x1) == 0 )
	  return;
      }
      else
      {
	if ( x < u8g2->user_x0 )
	  return;
	if ( x >= u8g2->user_x1 )
	  return;
	if ( u8g2_clip_intersection2(&y, &len, u8g2->user_y0, u8g2->user_y1) == 0 )
	  return;
      }
      
      
      u8g2->cb->draw_l90(u8g2, x, y, len, dir);
    }
}

void u8g2_DrawHLine(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len)
{
// #ifdef U8G2_WITH_INTERSECTION
//   if ( u8g2_IsIntersection(u8g2, x, y, x+len, y+1) == 0 ) 
//     return;
// #endif /* U8G2_WITH_INTERSECTION */
  u8g2_DrawHVLine(u8g2, x, y, len, 0);
}

void u8g2_DrawVLine(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len)
{
// #ifdef U8G2_WITH_INTERSECTION
//   if ( u8g2_IsIntersection(u8g2, x, y, x+1, y+len) == 0 ) 
//     return;
// #endif /* U8G2_WITH_INTERSECTION */
  u8g2_DrawHVLine(u8g2, x, y, len, 1);
}

void u8g2_DrawPixel(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y)
{
#ifdef U8G2_WITH_INTERSECTION
  if ( y < u8g2->user_y0 )
    return;
  if ( y >= u8g2->user_y1 )
    return;
  if ( x < u8g2->user_x0 )
    return;
  if ( x >= u8g2->user_x1 )
    return;
#endif /* U8G2_WITH_INTERSECTION */
  u8g2_DrawHVLine(u8g2, x, y, 1, 0);
}

/*
  Assign the draw color for all drawing functions.
  color may be 0 or 1. The actual color is defined by the display.
  With color = 1 the drawing function will set the display memory to 1.
  For OLEDs this ususally means, that the pixel is enabled and the LED 
  at the pixel is turned on.
  On an LCD it usually means that the LCD segment of the pixel is enabled, 
  which absorbs the light.
  For eInk/ePaper it means black ink.

  7 Jan 2017: Allow color value 2 for XOR operation.
  
*/
void u8g2_SetDrawColor(u8g2_t *u8g2, uint8_t color)
{
  u8g2->draw_color = color;	/* u8g2_SetDrawColor: just assign the argument */ 
  if ( color >= 3 )
    u8g2->draw_color = 1;	/* u8g2_SetDrawColor: make color as one if arg is invalid */
}

