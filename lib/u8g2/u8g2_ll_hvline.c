/*

  u8g2_ll_hvline.c
  
  low level hvline

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


  *ptr |= or_mask
  *ptr ^= xor_mask
  
  color = 0:   or_mask = 1, xor_mask = 1
  color = 1:   or_mask = 1, xor_mask = 0
  color = 2:   or_mask = 0, xor_mask = 1

  if ( color <= 1 )
    or_mask  = mask;
  if ( color != 1 )
    xor_mask = mask;
    
*/

#include "u8g2.h"
#include <assert.h>

/*=================================================*/
/*
  u8g2_ll_hvline_vertical_top_lsb
    SSD13xx
    UC1701    
*/


#ifdef U8G2_WITH_HVLINE_SPEED_OPTIMIZATION

/*
  x,y		Upper left position of the line within the local buffer (not the display!)
  len		length of the line in pixel, len must not be 0
  dir		0: horizontal line (left to right)
		1: vertical line (top to bottom)
  asumption: 
    all clipping done
*/
void u8g2_ll_hvline_vertical_top_lsb(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{
  uint16_t offset;
  uint8_t *ptr;
  uint8_t bit_pos, mask;
  uint8_t or_mask, xor_mask;
#ifdef __unix
  uint8_t *max_ptr = u8g2->tile_buf_ptr + u8g2_GetU8x8(u8g2)->display_info->tile_width*u8g2->tile_buf_height*8;
#endif

  //assert(x >= u8g2->buf_x0);
  //assert(x < u8g2_GetU8x8(u8g2)->display_info->tile_width*8);
  //assert(y >= u8g2->buf_y0);
  //assert(y < u8g2_GetU8x8(u8g2)->display_info->tile_height*8);
  
  /* bytes are vertical, lsb on top (y=0), msb at bottom (y=7) */
  bit_pos = y;		/* overflow truncate is ok here... */
  bit_pos &= 7; 	/* ... because only the lowest 3 bits are needed */
  mask = 1;
  mask <<= bit_pos;

  or_mask = 0;
  xor_mask = 0;
  if ( u8g2->draw_color <= 1 )
    or_mask  = mask;
  if ( u8g2->draw_color != 1 )
    xor_mask = mask;


  offset = y;		/* y might be 8 or 16 bit, but we need 16 bit, so use a 16 bit variable */
  offset &= ~7;
  offset *= u8g2_GetU8x8(u8g2)->display_info->tile_width;
  ptr = u8g2->tile_buf_ptr;
  ptr += offset;
  ptr += x;
  
  if ( dir == 0 )
  {
      do
      {
#ifdef __unix
	assert(ptr < max_ptr);
#endif
	*ptr |= or_mask;
	*ptr ^= xor_mask;
	ptr++;
	len--;
      } while( len != 0 );
  }
  else
  {    
    do
    {
#ifdef __unix
      assert(ptr < max_ptr);
#endif
      *ptr |= or_mask;
      *ptr ^= xor_mask;
      
      bit_pos++;
      bit_pos &= 7;

      len--;

      if ( bit_pos == 0 )
      {
	ptr+=u8g2->pixel_buf_width;	/* 6 Jan 17: Changed u8g2->width to u8g2->pixel_buf_width, issue #148 */
		
	if ( u8g2->draw_color <= 1 )
	  or_mask  = 1;
	if ( u8g2->draw_color != 1 )
	  xor_mask = 1;
      }
      else
      {
	or_mask <<= 1;
	xor_mask <<= 1;
      }
    } while( len != 0 );
  }
}



#else /* U8G2_WITH_HVLINE_SPEED_OPTIMIZATION */

/*
  x,y position within the buffer
*/
static void u8g2_draw_pixel_vertical_top_lsb(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y)
{
  uint16_t offset;
  uint8_t *ptr;
  uint8_t bit_pos, mask;
  
  //assert(x >= u8g2->buf_x0);
  //assert(x < u8g2_GetU8x8(u8g2)->display_info->tile_width*8);
  //assert(y >= u8g2->buf_y0);
  //assert(y < u8g2_GetU8x8(u8g2)->display_info->tile_height*8);
  
  /* bytes are vertical, lsb on top (y=0), msb at bottom (y=7) */
  bit_pos = y;		/* overflow truncate is ok here... */
  bit_pos &= 7; 	/* ... because only the lowest 3 bits are needed */
  mask = 1;
  mask <<= bit_pos;

  offset = y;		/* y might be 8 or 16 bit, but we need 16 bit, so use a 16 bit variable */
  offset &= ~7;
  offset *= u8g2_GetU8x8(u8g2)->display_info->tile_width;
  ptr = u8g2->tile_buf_ptr;
  ptr += offset;
  ptr += x;


  if ( u8g2->draw_color <= 1 )
    *ptr |= mask;
  if ( u8g2->draw_color != 1 )
    *ptr ^= mask;

}

/*
  x,y		Upper left position of the line within the local buffer (not the display!)
  len		length of the line in pixel, len must not be 0
  dir		0: horizontal line (left to right)
		1: vertical line (top to bottom)
  asumption: 
    all clipping done
*/
void u8g2_ll_hvline_vertical_top_lsb(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{
  if ( dir == 0 )
  {
    do
    {
      u8g2_draw_pixel_vertical_top_lsb(u8g2, x, y);
      x++;
      len--;
    } while( len != 0 );
  }
  else
  {
    do
    {
      u8g2_draw_pixel_vertical_top_lsb(u8g2, x, y);
      y++;
      len--;
    } while( len != 0 );
  }
}


#endif /* U8G2_WITH_HVLINE_SPEED_OPTIMIZATION */

/*=================================================*/
/*
  u8g2_ll_hvline_horizontal_right_lsb
    ST7920
*/

#ifdef U8G2_WITH_HVLINE_SPEED_OPTIMIZATION

/*
  x,y		Upper left position of the line within the local buffer (not the display!)
  len		length of the line in pixel, len must not be 0
  dir		0: horizontal line (left to right)
		1: vertical line (top to bottom)
  asumption: 
    all clipping done
*/

/* SH1122, LD7032, ST7920, ST7986, LC7981, T6963, SED1330, RA8835, MAX7219, LS0 */ 
void u8g2_ll_hvline_horizontal_right_lsb(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{
  uint16_t offset;
  uint8_t *ptr;
  uint8_t bit_pos;
  uint8_t mask;
  uint8_t tile_width = u8g2_GetU8x8(u8g2)->display_info->tile_width;

  bit_pos = x;		/* overflow truncate is ok here... */
  bit_pos &= 7; 	/* ... because only the lowest 3 bits are needed */
  mask = 128;
  mask >>= bit_pos;

  offset = y;		/* y might be 8 or 16 bit, but we need 16 bit, so use a 16 bit variable */
  offset *= tile_width;
  offset += x>>3;
  ptr = u8g2->tile_buf_ptr;
  ptr += offset;
  
  if ( dir == 0 )
  {
      
    do
    {

      if ( u8g2->draw_color <= 1 )
	*ptr |= mask;
      if ( u8g2->draw_color != 1 )
	*ptr ^= mask;
      
      mask >>= 1;
      if ( mask == 0 )
      {
	mask = 128;
        ptr++;
      }
      
      //x++;
      len--;
    } while( len != 0 );
  }
  else
  {
    do
    {
      if ( u8g2->draw_color <= 1 )
	*ptr |= mask;
      if ( u8g2->draw_color != 1 )
	*ptr ^= mask;
      
      ptr += tile_width;
      //y++;
      len--;
    } while( len != 0 );
  }
}

#else /* U8G2_WITH_HVLINE_SPEED_OPTIMIZATION */


/*
  x,y position within the buffer
*/
/* SH1122, LD7032, ST7920, ST7986, LC7981, T6963, SED1330, RA8835, MAX7219, LS0 */ 
static void u8g2_draw_pixel_horizontal_right_lsb(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y)
{
  uint16_t offset;
  uint8_t *ptr;
  uint8_t bit_pos, mask;

  //assert(x >= u8g2->buf_x0);
  //assert(x < u8g2_GetU8x8(u8g2)->display_info->tile_width*8);
  //assert(y >= u8g2->buf_y0);
  //assert(y < u8g2_GetU8x8(u8g2)->display_info->tile_height*8);
  
  /* bytes are vertical, lsb on top (y=0), msb at bottom (y=7) */
  bit_pos = x;		/* overflow truncate is ok here... */
  bit_pos &= 7; 	/* ... because only the lowest 3 bits are needed */
  mask = 128;
  mask >>= bit_pos;
  x >>= 3;

  offset = y;		/* y might be 8 or 16 bit, but we need 16 bit, so use a 16 bit variable */
  offset *= u8g2_GetU8x8(u8g2)->display_info->tile_width;
  offset += x;
  ptr = u8g2->tile_buf_ptr;
  ptr += offset;
  

  if ( u8g2->draw_color <= 1 )
    *ptr |= mask;
  if ( u8g2->draw_color != 1 )
    *ptr ^= mask;
  
}

/*
  x,y		Upper left position of the line within the local buffer (not the display!)
  len		length of the line in pixel, len must not be 0
  dir		0: horizontal line (left to right)
		1: vertical line (top to bottom)
  asumption: 
    all clipping done
*/
/* SH1122, LD7032, ST7920, ST7986, LC7981, T6963, SED1330, RA8835, MAX7219, LS0 */ 
void u8g2_ll_hvline_horizontal_right_lsb(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{
  if ( dir == 0 )
  {
    do
    {
      u8g2_draw_pixel_horizontal_right_lsb(u8g2, x, y);
      x++;
      len--;
    } while( len != 0 );
  }
  else
  {
    do
    {
      u8g2_draw_pixel_horizontal_right_lsb(u8g2, x, y);
      y++;
      len--;
    } while( len != 0 );
  }
}

#endif /* U8G2_WITH_HVLINE_SPEED_OPTIMIZATION */
