/*

  u8g2_box.c

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

/*
  draw a filled box
  restriction: does not work for w = 0 or h = 0
*/
void u8g2_DrawBox(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
{
#ifdef U8G2_WITH_INTERSECTION
  if ( u8g2_IsIntersection(u8g2, x, y, x+w, y+h) == 0 ) 
    return;
#endif /* U8G2_WITH_INTERSECTION */
  while( h != 0 )
  { 
    u8g2_DrawHVLine(u8g2, x, y, w, 0);
    y++;    
    h--;
  }
}


/*
  draw a frame (empty box)
  restriction: does not work for w = 0 or h = 0
  ToDo:
    pixel in the corners are drawn twice. This could be optimized.
*/
void u8g2_DrawFrame(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
{
  u8g2_uint_t xtmp = x;
  
#ifdef U8G2_WITH_INTERSECTION
  if ( u8g2_IsIntersection(u8g2, x, y, x+w, y+h) == 0 ) 
    return;
#endif /* U8G2_WITH_INTERSECTION */
  
  u8g2_DrawHVLine(u8g2, x, y, w, 0);
  u8g2_DrawHVLine(u8g2, x, y, h, 1);
  x+=w;
  x--;
  u8g2_DrawHVLine(u8g2, x, y, h, 1);
  y+=h;
  y--;
  u8g2_DrawHVLine(u8g2, xtmp, y, w, 0);
}




void u8g2_DrawRBox(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r)
{
  u8g2_uint_t xl, yu;
  u8g2_uint_t yl, xr;

#ifdef U8G2_WITH_INTERSECTION
  if ( u8g2_IsIntersection(u8g2, x, y, x+w, y+h) == 0 ) 
    return;
#endif /* U8G2_WITH_INTERSECTION */

  xl = x;
  xl += r;
  yu = y;
  yu += r;
 
  xr = x;
  xr += w;
  xr -= r;
  xr -= 1;
  
  yl = y;
  yl += h;
  yl -= r; 
  yl -= 1;

  u8g2_DrawDisc(u8g2, xl, yu, r, U8G2_DRAW_UPPER_LEFT);
  u8g2_DrawDisc(u8g2, xr, yu, r, U8G2_DRAW_UPPER_RIGHT);
  u8g2_DrawDisc(u8g2, xl, yl, r, U8G2_DRAW_LOWER_LEFT);
  u8g2_DrawDisc(u8g2, xr, yl, r, U8G2_DRAW_LOWER_RIGHT);

  {
    u8g2_uint_t ww, hh;

    ww = w;
    ww -= r;
    ww -= r;
    xl++;
    yu++;
    
    if ( ww >= 3 )
    {
      ww -= 2;
      u8g2_DrawBox(u8g2, xl, y, ww, r+1);
      u8g2_DrawBox(u8g2, xl, yl, ww, r+1);
    }
    
    hh = h;
    hh -= r;
    hh -= r;
    //h--;
    if ( hh >= 3 )
    {
      hh -= 2;
      u8g2_DrawBox(u8g2, x, yu, w, hh);
    }
  }
}


void u8g2_DrawRFrame(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r)
{
  u8g2_uint_t xl, yu;

#ifdef U8G2_WITH_INTERSECTION
  if ( u8g2_IsIntersection(u8g2, x, y, x+w, y+h) == 0 ) 
    return;
#endif /* U8G2_WITH_INTERSECTION */

  xl = x;
  xl += r;
  yu = y;
  yu += r;
 
  {
    u8g2_uint_t yl, xr;
      
    xr = x;
    xr += w;
    xr -= r;
    xr -= 1;
    
    yl = y;
    yl += h;
    yl -= r; 
    yl -= 1;

    u8g2_DrawCircle(u8g2, xl, yu, r, U8G2_DRAW_UPPER_LEFT);
    u8g2_DrawCircle(u8g2, xr, yu, r, U8G2_DRAW_UPPER_RIGHT);
    u8g2_DrawCircle(u8g2, xl, yl, r, U8G2_DRAW_LOWER_LEFT);
    u8g2_DrawCircle(u8g2, xr, yl, r, U8G2_DRAW_LOWER_RIGHT);
  }

  {
    u8g2_uint_t ww, hh;

    ww = w;
    ww -= r;
    ww -= r;
    hh = h;
    hh -= r;
    hh -= r;
    
    xl++;
    yu++;
    
    if ( ww >= 3 )
    {
      ww -= 2;
      h--;
      u8g2_DrawHLine(u8g2, xl, y, ww);
      u8g2_DrawHLine(u8g2, xl, y+h, ww);
    }
    
    if ( hh >= 3 )
    {
      hh -= 2;
      w--;
      u8g2_DrawVLine(u8g2, x, yu, hh);
      u8g2_DrawVLine(u8g2, x+w, yu, hh);
    }
  }
}

