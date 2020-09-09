/*

  u8g2_setup.c

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
#include <string.h>
#include <assert.h>


/*============================================*/


#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT

void u8g2_SetMaxClipWindow(u8g2_t *u8g2)
{
  u8g2->clip_x0 = 0;
  u8g2->clip_y0 = 0;
  u8g2->clip_x1 = (u8g2_uint_t)~(u8g2_uint_t)0;
  u8g2->clip_y1 = (u8g2_uint_t)~(u8g2_uint_t)0;
  
  u8g2->cb->update_page_win(u8g2);
}

void u8g2_SetClipWindow(u8g2_t *u8g2, u8g2_uint_t clip_x0, u8g2_uint_t clip_y0, u8g2_uint_t clip_x1, u8g2_uint_t clip_y1 )
{
  u8g2->clip_x0 = clip_x0;
  u8g2->clip_y0 = clip_y0;
  u8g2->clip_x1 = clip_x1;
  u8g2->clip_y1 = clip_y1;
  u8g2->cb->update_page_win(u8g2);
}
#endif

/*============================================*/
/*
  This procedure is called after setting up the display (u8x8 structure).
  --> This is the central init procedure for u8g2 object
*/
void u8g2_SetupBuffer(u8g2_t *u8g2, uint8_t *buf, uint8_t tile_buf_height, u8g2_draw_ll_hvline_cb ll_hvline_cb, const u8g2_cb_t *u8g2_cb)
{
  u8g2->font = NULL;
  //u8g2->kerning = NULL;
  //u8g2->get_kerning_cb = u8g2_GetNullKerning;
  
  //u8g2->ll_hvline = u8g2_ll_hvline_vertical_top_lsb;
  u8g2->ll_hvline = ll_hvline_cb;
  
  u8g2->tile_buf_ptr = buf;
  u8g2->tile_buf_height = tile_buf_height;
  
  u8g2->tile_curr_row = 0;
  
  u8g2->font_decode.is_transparent = 0; /* issue 443 */
  u8g2->bitmap_transparency = 0;
  
  u8g2->draw_color = 1;
  u8g2->is_auto_page_clear = 1;
  
  u8g2->cb = u8g2_cb;
  u8g2->cb->update_dimension(u8g2);
#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT
  u8g2_SetMaxClipWindow(u8g2);		/* assign a clip window and call the update() procedure */
#else
  u8g2->cb->update_page_win(u8g2);
#endif

  u8g2_SetFontPosBaseline(u8g2);  /* issue 195 */
  
#ifdef U8G2_WITH_FONT_ROTATION  
  u8g2->font_decode.dir = 0;
#endif
}

/*
  Usually the display rotation is set initially, but it could be done later also
  u8g2_cb can be U8G2_R0..U8G2_R3
*/
void u8g2_SetDisplayRotation(u8g2_t *u8g2, const u8g2_cb_t *u8g2_cb)
{
  u8g2->cb = u8g2_cb;
  u8g2->cb->update_dimension(u8g2);
  u8g2->cb->update_page_win(u8g2);
}

/*============================================*/

void u8g2_SendF(u8g2_t * u8g2, const char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  u8x8_cad_vsendf(u8g2_GetU8x8(u8g2), fmt, va);
  va_end(va);
}


/*============================================*/
/* 
  update dimension: 
  calculate the following variables:
    u8g2_uint_t buf_x0;	left corner of the buffer
    u8g2_uint_t buf_x1;	right corner of the buffer (excluded)
    u8g2_uint_t buf_y0;
    u8g2_uint_t buf_y1; 	
*/

static void u8g2_update_dimension_common(u8g2_t *u8g2)
{
  const u8x8_display_info_t *display_info = u8g2_GetU8x8(u8g2)->display_info;
  u8g2_uint_t t;
  
  t = u8g2->tile_buf_height;
  t *= 8;
  u8g2->pixel_buf_height = t;
  
  t = display_info->tile_width;
#ifndef U8G2_16BIT
  if ( t >= 32 )
    t = 31;
#endif
  t *= 8;
  u8g2->pixel_buf_width = t;
  
  t = u8g2->tile_curr_row;
  t *= 8;
  u8g2->pixel_curr_row = t;
  
  t = u8g2->tile_buf_height;
  /* handle the case, where the buffer is larger than the (remaining) part of the display */
  if ( t + u8g2->tile_curr_row > display_info->tile_height )
    t = display_info->tile_height - u8g2->tile_curr_row;
  t *= 8;
  
  u8g2->buf_y0 = u8g2->pixel_curr_row;   
  u8g2->buf_y1 = u8g2->buf_y0;
  u8g2->buf_y1 += t;

  
#ifdef U8G2_16BIT
  u8g2->width = display_info->pixel_width;
  u8g2->height = display_info->pixel_height;
#else
  u8g2->width = 240;
  if ( display_info->pixel_width <= 240 )
    u8g2->width = display_info->pixel_width;
  u8g2->height = display_info->pixel_height;
#endif

}

/*==========================================================*/
/* apply clip window */

#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT
static void u8g2_apply_clip_window(u8g2_t *u8g2)
{
  /* check aganst the current user_??? window */
  if ( u8g2_IsIntersection(u8g2, u8g2->clip_x0, u8g2->clip_y0, u8g2->clip_x1, u8g2->clip_y1) == 0 ) 
  {
    u8g2->is_page_clip_window_intersection = 0;
  }
  else
  {
    u8g2->is_page_clip_window_intersection = 1;

    if ( u8g2->user_x0 < u8g2->clip_x0 )
      u8g2->user_x0 = u8g2->clip_x0;
    if ( u8g2->user_x1 > u8g2->clip_x1 )
      u8g2->user_x1 = u8g2->clip_x1;
    if ( u8g2->user_y0 < u8g2->clip_y0 )
      u8g2->user_y0 = u8g2->clip_y0;
    if ( u8g2->user_y1 > u8g2->clip_y1 )
      u8g2->user_y1 = u8g2->clip_y1;
  }
}
#endif /* U8G2_WITH_CLIP_WINDOW_SUPPORT */

/*==========================================================*/


void u8g2_update_dimension_r0(u8g2_t *u8g2)
{
  u8g2_update_dimension_common(u8g2);  
}

void u8g2_update_page_win_r0(u8g2_t *u8g2)
{
  u8g2->user_x0 = 0;
  u8g2->user_x1 = u8g2->width;			/* pixel_buf_width replaced with width */
  
  u8g2->user_y0 = u8g2->buf_y0;
  u8g2->user_y1 = u8g2->buf_y1;
  
#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT
  u8g2_apply_clip_window(u8g2);
#endif /* U8G2_WITH_CLIP_WINDOW_SUPPORT */
}


void u8g2_update_dimension_r1(u8g2_t *u8g2)
{
  u8g2_update_dimension_common(u8g2);
  
  u8g2->height = u8g2_GetU8x8(u8g2)->display_info->pixel_width;
  u8g2->width = u8g2_GetU8x8(u8g2)->display_info->pixel_height;
  
}

void u8g2_update_page_win_r1(u8g2_t *u8g2)
{
  u8g2->user_x0 = u8g2->buf_y0;
  u8g2->user_x1 = u8g2->buf_y1;
  
  u8g2->user_y0 = 0;
  u8g2->user_y1 = u8g2->height;	/* pixel_buf_width replaced with height (which is the real pixel width) */
  
#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT
  u8g2_apply_clip_window(u8g2);
#endif /* U8G2_WITH_CLIP_WINDOW_SUPPORT */
}

void u8g2_update_dimension_r2(u8g2_t *u8g2)
{
  u8g2_update_dimension_common(u8g2);
}

void u8g2_update_page_win_r2(u8g2_t *u8g2)
{
  u8g2->user_x0 = 0;
  u8g2->user_x1 = u8g2->width;	/* pixel_buf_width replaced with width */
  
  /* there are ases where the height is not a multiple of 8. */
  /* in such a case u8g2->buf_y1 might be heigher than u8g2->height */
  u8g2->user_y0 = 0;
  if ( u8g2->height >= u8g2->buf_y1 )
    u8g2->user_y0 = u8g2->height - u8g2->buf_y1;
  u8g2->user_y1 = u8g2->height - u8g2->buf_y0;

#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT
  u8g2_apply_clip_window(u8g2);
#endif /* U8G2_WITH_CLIP_WINDOW_SUPPORT */
}


void u8g2_update_dimension_r3(u8g2_t *u8g2)
{
  u8g2_update_dimension_common(u8g2);
  
  u8g2->height = u8g2_GetU8x8(u8g2)->display_info->pixel_width;
  u8g2->width = u8g2_GetU8x8(u8g2)->display_info->pixel_height;

}

void u8g2_update_page_win_r3(u8g2_t *u8g2)
{
  /* there are ases where the height is not a multiple of 8. */
  /* in such a case u8g2->buf_y1 might be heigher than u8g2->width */
  u8g2->user_x0 = 0;
  if ( u8g2->width >= u8g2->buf_y1 )
    u8g2->user_x0 = u8g2->width - u8g2->buf_y1;
  u8g2->user_x1 = u8g2->width - u8g2->buf_y0;
  
  u8g2->user_y0 = 0;
  u8g2->user_y1 = u8g2->height;	/* pixel_buf_width replaced with height (pixel_width) */

#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT
  u8g2_apply_clip_window(u8g2);
#endif /* U8G2_WITH_CLIP_WINDOW_SUPPORT */
}


/*============================================*/
extern void u8g2_draw_hv_line_2dir(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir);


void u8g2_draw_l90_r0(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{
#ifdef __unix
  assert( dir <= 1 );
#endif
  u8g2_draw_hv_line_2dir(u8g2, x, y, len, dir);
}

void u8g2_draw_l90_mirrorr_r0(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{
  u8g2_uint_t xx;
  xx = u8g2->width;
  xx -= x;
  if ( (dir & 1) == 0 )
  {
    xx -= len;
  }
  else
  {
    xx--;
  }
  u8g2_draw_hv_line_2dir(u8g2, xx, y, len, dir);
}

/* dir = 0 or 1 */
void u8g2_draw_l90_r1(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{
  u8g2_uint_t xx, yy;

#ifdef __unix
  assert( dir <= 1 );
#endif
  
  yy = x;
  
  xx = u8g2->height;
  xx -= y;
  xx--;
  
  dir ++;
  if ( dir == 2 )
  {
    xx -= len;
    xx++;
    dir = 0;
  }
  
  u8g2_draw_hv_line_2dir(u8g2, xx, yy, len, dir);
}

void u8g2_draw_l90_r2(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{
  u8g2_uint_t xx, yy;

  /*
  yy = u8g2->height;
  yy -= y;
  yy--;
  
  xx = u8g2->width;
  xx -= x;
  xx--;
  
  if ( dir == 0 )
  {
    xx -= len;
    xx++;
  }
  else if ( dir == 1 )
  {
    yy -= len;
    yy++;
  }
  */

  yy = u8g2->height;
  yy -= y;
  
  xx = u8g2->width;
  xx -= x;
  
  if ( dir == 0 )
  {
    yy--;
    xx -= len;
  }
  else if ( dir == 1 )
  {
    xx--;
    yy -= len;
  }

  u8g2_draw_hv_line_2dir(u8g2, xx, yy, len, dir);
}

void u8g2_draw_l90_r3(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{
  u8g2_uint_t xx, yy;

  xx = y;
  
  yy = u8g2->width;
  yy -= x;
  
  if ( dir == 0 )
  {
    yy--;
    yy -= len;
    yy++;
    dir = 1;
  }
  else
  {
    yy--;
    dir = 0;
  }
  
  
  u8g2_draw_hv_line_2dir(u8g2, xx, yy, len, dir);
}



/*============================================*/
const u8g2_cb_t u8g2_cb_r0 = { u8g2_update_dimension_r0, u8g2_update_page_win_r0, u8g2_draw_l90_r0 };
const u8g2_cb_t u8g2_cb_r1 = { u8g2_update_dimension_r1, u8g2_update_page_win_r1, u8g2_draw_l90_r1 };
const u8g2_cb_t u8g2_cb_r2 = { u8g2_update_dimension_r2, u8g2_update_page_win_r2, u8g2_draw_l90_r2 };
const u8g2_cb_t u8g2_cb_r3 = { u8g2_update_dimension_r3, u8g2_update_page_win_r3, u8g2_draw_l90_r3 };
  
const u8g2_cb_t u8g2_cb_mirror = { u8g2_update_dimension_r0, u8g2_update_page_win_r0, u8g2_draw_l90_mirrorr_r0 };
  
/*============================================*/
/* setup for the null device */

/* setup for the null (empty) device */
void u8g2_Setup_null(u8g2_t *u8g2, const u8g2_cb_t *rotation, u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb)
{
  static uint8_t buf[8];
  u8g2_SetupDisplay(u8g2, u8x8_d_null_cb, u8x8_cad_empty, byte_cb, gpio_and_delay_cb);
  u8g2_SetupBuffer(u8g2, buf, 1, u8g2_ll_hvline_vertical_top_lsb, rotation);
}


  
  