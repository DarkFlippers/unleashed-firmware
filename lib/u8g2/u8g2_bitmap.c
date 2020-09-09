/*

  u8g2_bitmap.c

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


void u8g2_SetBitmapMode(u8g2_t *u8g2, uint8_t is_transparent) {
  u8g2->bitmap_transparency = is_transparent;
}

/*
  x,y 	Position on the display
  len		Length of bitmap line in pixel. Note: This differs from u8glib which had a bytecount here.
  b		Pointer to the bitmap line.
  Only draw pixels which are set.
*/

void u8g2_DrawHorizontalBitmap(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, const uint8_t *b)
{
  uint8_t mask;
  uint8_t color = u8g2->draw_color;
  uint8_t ncolor = (color == 0 ? 1 : 0);

#ifdef U8G2_WITH_INTERSECTION
  if ( u8g2_IsIntersection(u8g2, x, y, x+len, y+1) == 0 ) 
    return;
#endif /* U8G2_WITH_INTERSECTION */
  
  mask = 128;
  while(len > 0)
  {
    if ( *b & mask ) {
      u8g2->draw_color = color;
      u8g2_DrawHVLine(u8g2, x, y, 1, 0);
    } else if ( u8g2->bitmap_transparency == 0 ) {
      u8g2->draw_color = ncolor;
      u8g2_DrawHVLine(u8g2, x, y, 1, 0);
    }

    x++;
    mask >>= 1;
    if ( mask == 0 )
    {
      mask = 128;
      b++;
    }
    len--;
  }
  u8g2->draw_color = color;
}


/* u8glib compatible bitmap draw function */
void u8g2_DrawBitmap(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t cnt, u8g2_uint_t h, const uint8_t *bitmap)
{
  u8g2_uint_t w;
  w = cnt;
  w *= 8;
#ifdef U8G2_WITH_INTERSECTION
  if ( u8g2_IsIntersection(u8g2, x, y, x+w, y+h) == 0 ) 
    return;
#endif /* U8G2_WITH_INTERSECTION */
  
  while( h > 0 )
  {
    u8g2_DrawHorizontalBitmap(u8g2, x, y, w, bitmap);
    bitmap += cnt;
    y++;
    h--;
  }
}



void u8g2_DrawHXBM(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, const uint8_t *b)
{
  uint8_t mask;
  uint8_t color = u8g2->draw_color;
  uint8_t ncolor = (color == 0 ? 1 : 0);
#ifdef U8G2_WITH_INTERSECTION
  if ( u8g2_IsIntersection(u8g2, x, y, x+len, y+1) == 0 ) 
    return;
#endif /* U8G2_WITH_INTERSECTION */
  
  mask = 1;
  while(len > 0) {
    if ( *b & mask ) {
      u8g2->draw_color = color;
      u8g2_DrawHVLine(u8g2, x, y, 1, 0);
    } else if ( u8g2->bitmap_transparency == 0 ) {
      u8g2->draw_color = ncolor;
      u8g2_DrawHVLine(u8g2, x, y, 1, 0);
    }
    x++;
    mask <<= 1;
    if ( mask == 0 )
    {
      mask = 1;
      b++;
    }
    len--;
  }
  u8g2->draw_color = color;
}


void u8g2_DrawXBM(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, const uint8_t *bitmap)
{
  u8g2_uint_t blen;
  blen = w;
  blen += 7;
  blen >>= 3;
#ifdef U8G2_WITH_INTERSECTION
  if ( u8g2_IsIntersection(u8g2, x, y, x+w, y+h) == 0 ) 
    return;
#endif /* U8G2_WITH_INTERSECTION */
  
  while( h > 0 )
  {
    u8g2_DrawHXBM(u8g2, x, y, w, bitmap);
    bitmap += blen;
    y++;
    h--;
  }
}






void u8g2_DrawHXBMP(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, const uint8_t *b)
{
  uint8_t mask;
  uint8_t color = u8g2->draw_color;
  uint8_t ncolor = (color == 0 ? 1 : 0);
#ifdef U8G2_WITH_INTERSECTION
  if ( u8g2_IsIntersection(u8g2, x, y, x+len, y+1) == 0 ) 
    return;
#endif /* U8G2_WITH_INTERSECTION */
  
  mask = 1;
  while(len > 0)
  {
    if( u8x8_pgm_read(b) & mask ) {
      u8g2->draw_color = color;
      u8g2_DrawHVLine(u8g2, x, y, 1, 0);
    } else if( u8g2->bitmap_transparency == 0 ) {
      u8g2->draw_color = ncolor;
      u8g2_DrawHVLine(u8g2, x, y, 1, 0);
    }
   
    x++;
    mask <<= 1;
    if ( mask == 0 )
    {
      mask = 1;
      b++;
    }
    len--;
  }
  u8g2->draw_color = color;
}


void u8g2_DrawXBMP(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, const uint8_t *bitmap)
{
  u8g2_uint_t blen;
  blen = w;
  blen += 7;
  blen >>= 3;
#ifdef U8G2_WITH_INTERSECTION
  if ( u8g2_IsIntersection(u8g2, x, y, x+w, y+h) == 0 ) 
    return;
#endif /* U8G2_WITH_INTERSECTION */
  
  while( h > 0 )
  {
    u8g2_DrawHXBMP(u8g2, x, y, w, bitmap);
    bitmap += blen;
    y++;
    h--;
  }
}


