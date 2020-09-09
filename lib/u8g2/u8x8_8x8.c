/*

  u8x8_8x8.c
  
  font procedures, directly interfaces display procedures
  
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

#include "u8x8.h"

#if defined(ESP8266)
uint8_t u8x8_pgm_read_esp(const uint8_t * addr) 
{
    uint32_t bytes;
    bytes = *(uint32_t*)((uint32_t)addr & ~3);
    return ((uint8_t*)&bytes)[(uint32_t)addr & 3];
}
#endif


void u8x8_SetFont(u8x8_t *u8x8, const uint8_t *font_8x8)
{
  u8x8->font = font_8x8;
}

/*
 Args:
   u8x8: ptr to u8x8 structure
   encoding: glyph for which the data is requested (must be between 0 and 255)
   buf: pointer to 8 bytes
*/
static void u8x8_get_glyph_data(u8x8_t *u8x8, uint8_t encoding, uint8_t *buf, uint8_t tile_offset) U8X8_NOINLINE;
static void u8x8_get_glyph_data(u8x8_t *u8x8, uint8_t encoding, uint8_t *buf, uint8_t tile_offset) 
{
  uint8_t first, last, tiles, i;
  uint16_t offset;
  first = u8x8_pgm_read(u8x8->font+0);
  last = u8x8_pgm_read(u8x8->font+1);
  tiles = u8x8_pgm_read(u8x8->font+2);		/* new 2019 format */
  tiles *= u8x8_pgm_read(u8x8->font+3);	/* new 2019 format */
  
  /* get the glyph bitmap from the font */
  if ( first <= encoding && encoding <= last )
  {
    offset = encoding;
    offset -= first;
    offset *= tiles;		/* new 2019 format */
    offset += tile_offset;	/* new 2019 format */
    offset *= 8;
    offset +=4;			/* changed from 2 to 4, new 2019 format */
    for( i = 0; i < 8; i++ )
    {
      buf[i] = u8x8_pgm_read(u8x8->font+offset);
      offset++;
    }
  }
  else
  {
    for( i = 0; i < 8; i++ )
    {
      buf[i] = 0;
    }
  }
  
  /* invert the bitmap if required */
  if ( u8x8->is_font_inverse_mode )
  {
    for( i = 0; i < 8; i++ )
    {
      buf[i] ^= 255;
    }
  }
  
}

void u8x8_DrawGlyph(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t encoding)
{
  uint8_t th = u8x8_pgm_read(u8x8->font+2);		/* new 2019 format */
  uint8_t tv = u8x8_pgm_read(u8x8->font+3);	/* new 2019 format */
  uint8_t xx, tile;
  uint8_t buf[8];
  th += x;
  tv += y;
  tile = 0;
  do
  {
    xx = x;
    do
    {
      u8x8_get_glyph_data(u8x8, encoding, buf, tile);
      u8x8_DrawTile(u8x8, xx, y, 1, buf);
      tile++;
      xx++;
    } while( xx < th );
    y++;
  } while( y < tv );
}


/*
  Source: http://graphics.stanford.edu/~seander/bithacks.html
	Section: Interleave bits by Binary Magic Numbers 
   Original codes is here:
		static const unsigned int B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
		static const unsigned int S[] = {1, 2, 4, 8};

		unsigned int x; // Interleave lower 16 bits of x and y, so the bits of x
		unsigned int y; // are in the even positions and bits from y in the odd;
		unsigned int z; // z gets the resulting 32-bit Morton Number.  
				// x and y must initially be less than 65536.

		x = (x | (x << S[3])) & B[3];
		x = (x | (x << S[2])) & B[2];
		x = (x | (x << S[1])) & B[1];
		x = (x | (x << S[0])) & B[0];

		y = (y | (y << S[3])) & B[3];
		y = (y | (y << S[2])) & B[2];
		y = (y | (y << S[1])) & B[1];
		y = (y | (y << S[0])) & B[0];

		z = x | (y << 1);
*/
uint16_t u8x8_upscale_byte(uint8_t x) 
{
	uint16_t y = x;
	y |= (y << 4);		// x = (x | (x << S[2])) & B[2];
	y &= 0x0f0f;
	y |= (y << 2);		// x = (x | (x << S[1])) & B[1];
	y &= 0x3333;
	y |= (y << 1);		// x = (x | (x << S[0])) & B[0];
	y &= 0x5555;
  
	y |= (y << 1);		// z = x | (y << 1);
	return y;
}

static void u8x8_upscale_buf(uint8_t *src, uint8_t *dest) U8X8_NOINLINE;
static void u8x8_upscale_buf(uint8_t *src, uint8_t *dest)
{
  uint8_t i = 4;  
  do 
  {
    *dest++ = *src;
    *dest++ = *src++;
    i--;
  } while( i > 0 );
}

static void u8x8_draw_2x2_subglyph(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t encoding, uint8_t tile)
{
  uint8_t i;
  uint16_t t;
  uint8_t buf[8];
  uint8_t buf1[8];
  uint8_t buf2[8];
  u8x8_get_glyph_data(u8x8, encoding, buf, tile);
  for( i = 0; i < 8; i ++ )
  {
      t = u8x8_upscale_byte(buf[i]);
      buf1[i] = t >> 8;
      buf2[i] = t & 255;
  }
  u8x8_upscale_buf(buf2, buf);
  u8x8_DrawTile(u8x8, x, y, 1, buf);
  
  u8x8_upscale_buf(buf2+4, buf);
  u8x8_DrawTile(u8x8, x+1, y, 1, buf);
  
  u8x8_upscale_buf(buf1, buf);
  u8x8_DrawTile(u8x8, x, y+1, 1, buf);
  
  u8x8_upscale_buf(buf1+4, buf);
  u8x8_DrawTile(u8x8, x+1, y+1, 1, buf);  
}


void u8x8_Draw2x2Glyph(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t encoding)
{
  uint8_t th = u8x8_pgm_read(u8x8->font+2);		/* new 2019 format */
  uint8_t tv = u8x8_pgm_read(u8x8->font+3);	/* new 2019 format */
  uint8_t xx, tile;
  th *= 2;
  th += x;
  tv *= 2;
  tv += y;
  tile = 0;
  do
  {
    xx = x;
    do
    {
      u8x8_draw_2x2_subglyph(u8x8, xx, y, encoding, tile);
      tile++;
      xx+=2;
    } while( xx < th );
    y+=2;
  } while( y < tv );  
}

/* https://github.com/olikraus/u8g2/issues/474 */
static void u8x8_draw_1x2_subglyph(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t encoding, uint8_t tile)
{
  uint8_t i;
  uint16_t t;
  uint8_t buf[8];
  uint8_t buf1[8];
  uint8_t buf2[8];
  u8x8_get_glyph_data(u8x8, encoding, buf, tile);
  for( i = 0; i < 8; i ++ )
  {
      t = u8x8_upscale_byte(buf[i]);
      buf1[i] = t >> 8;
      buf2[i] = t & 255;
  }
  u8x8_DrawTile(u8x8, x,   y, 1, buf2);
  u8x8_DrawTile(u8x8, x, y+1, 1, buf1);
}

void u8x8_Draw1x2Glyph(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t encoding)
{
  uint8_t th = u8x8_pgm_read(u8x8->font+2);		/* new 2019 format */
  uint8_t tv = u8x8_pgm_read(u8x8->font+3);	/* new 2019 format */
  uint8_t xx, tile;
  th += x;
  tv *= 2;
  tv += y;
  tile = 0;
  do
  {
    xx = x;
    do
    {
      u8x8_draw_1x2_subglyph(u8x8, xx, y, encoding, tile);
      tile++;
      xx++;
    } while( xx < th );
    y+=2;
  } while( y < tv );  
}

/*
source: https://en.wikipedia.org/wiki/UTF-8
Bits	from 		to			bytes	Byte 1 		Byte 2 		Byte 3 		Byte 4 		Byte 5 		Byte 6
  7 	U+0000 		U+007F 		1 		0xxxxxxx
11 	U+0080 		U+07FF 		2 		110xxxxx 	10xxxxxx
16 	U+0800 		U+FFFF 		3 		1110xxxx 	10xxxxxx 	10xxxxxx
21 	U+10000 	U+1FFFFF 	4 		11110xxx 	10xxxxxx 	10xxxxxx 	10xxxxxx
26 	U+200000 	U+3FFFFFF 	5 		111110xx 	10xxxxxx 	10xxxxxx 	10xxxxxx 	10xxxxxx
31 	U+4000000 	U+7FFFFFFF 	6 		1111110x 	10xxxxxx 	10xxxxxx 	10xxxxxx 	10xxxxxx 	10xxxxxx  


*/

/* reset the internal state machine */
void u8x8_utf8_init(u8x8_t *u8x8)
{
  u8x8->utf8_state = 0;	/* also reset during u8x8_SetupDefaults() */
}

uint16_t u8x8_ascii_next(U8X8_UNUSED u8x8_t *u8x8, uint8_t b)
{
  if ( b == 0 || b == '\n' ) /* '\n' terminates the string to support the string list procedures */
    return 0x0ffff;	/* end of string detected*/
  return b;
}

/*
  pass a byte from an utf8 encoded string to the utf8 decoder state machine
  returns 
    0x0fffe: no glyph, just continue
    0x0ffff: end of string
    anything else: The decoded encoding
*/
uint16_t u8x8_utf8_next(u8x8_t *u8x8, uint8_t b)
{
  if ( b == 0 || b == '\n' )	/* '\n' terminates the string to support the string list procedures */
    return 0x0ffff;	/* end of string detected, pending UTF8 is discarded */
  if ( u8x8->utf8_state == 0 )
  {
    if ( b >= 0xfc )	/* 6 byte sequence */
    {
      u8x8->utf8_state = 5;
      b &= 1;
    }
    else if ( b >= 0xf8 )
    {
      u8x8->utf8_state = 4;
      b &= 3;
    }
    else if ( b >= 0xf0 )
    {
      u8x8->utf8_state = 3;
      b &= 7;      
    }
    else if ( b >= 0xe0 )
    {
      u8x8->utf8_state = 2;
      b &= 15;
    }
    else if ( b >= 0xc0 )
    {
      u8x8->utf8_state = 1;
      b &= 0x01f;
    }
    else
    {
      /* do nothing, just use the value as encoding */
      return b;
    }
    u8x8->encoding = b;
    return 0x0fffe;
  }
  else
  {
    u8x8->utf8_state--;
    /* The case b < 0x080 (an illegal UTF8 encoding) is not checked here. */
    u8x8->encoding<<=6;
    b &= 0x03f;
    u8x8->encoding |= b;
    if ( u8x8->utf8_state != 0 )
      return 0x0fffe;	/* nothing to do yet */
  }
  return u8x8->encoding;
}



static uint8_t u8x8_draw_string(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s) U8X8_NOINLINE;
static uint8_t u8x8_draw_string(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s)
{
  uint16_t e;
  uint8_t cnt = 0;
  uint8_t th = u8x8_pgm_read(u8x8->font+2);		/* new 2019 format */

  u8x8_utf8_init(u8x8);
  for(;;)
  {
    e = u8x8->next_cb(u8x8, (uint8_t)*s);
    if ( e == 0x0ffff )
      break;
    s++;
    if ( e != 0x0fffe )
    {
      u8x8_DrawGlyph(u8x8, x, y, e);
      x+=th;
      cnt++;
    }
  }
  return cnt;
}


uint8_t u8x8_DrawString(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s)
{
  u8x8->next_cb = u8x8_ascii_next;
  return u8x8_draw_string(u8x8, x, y, s);
}

uint8_t u8x8_DrawUTF8(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s)
{
  u8x8->next_cb = u8x8_utf8_next;
  return u8x8_draw_string(u8x8, x, y, s);
}



static uint8_t u8x8_draw_2x2_string(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s) U8X8_NOINLINE;
static uint8_t u8x8_draw_2x2_string(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s)
{
  uint16_t e;
  uint8_t cnt = 0;
  uint8_t th = u8x8_pgm_read(u8x8->font+2);	/* new 2019 format */
  
  th <<= 1;
  
  u8x8_utf8_init(u8x8);
  for(;;)
  {
    e = u8x8->next_cb(u8x8, (uint8_t)*s);
    if ( e == 0x0ffff )
      break;
    s++;
    if ( e != 0x0fffe )
    {
      u8x8_Draw2x2Glyph(u8x8, x, y, e);
      x+=th;
      cnt++;
    }
  }
  return cnt;
}


uint8_t u8x8_Draw2x2String(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s)
{
  u8x8->next_cb = u8x8_ascii_next;
  return u8x8_draw_2x2_string(u8x8, x, y, s);
}

uint8_t u8x8_Draw2x2UTF8(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s)
{
  u8x8->next_cb = u8x8_utf8_next;
  return u8x8_draw_2x2_string(u8x8, x, y, s);
}



static uint8_t u8x8_draw_1x2_string(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s) U8X8_NOINLINE;
static uint8_t u8x8_draw_1x2_string(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s)
{  
  uint16_t e;
  uint8_t cnt = 0;
  uint8_t th = u8x8_pgm_read(u8x8->font+2);	/* new 2019 format */
  u8x8_utf8_init(u8x8);
  for(;;)
  {
    e = u8x8->next_cb(u8x8, (uint8_t)*s);
    if ( e == 0x0ffff )
      break;
    s++;
    if ( e != 0x0fffe )
    {
      u8x8_Draw1x2Glyph(u8x8, x, y, e);
      x+=th;
      cnt++;
    }
  }
  return cnt;
}


uint8_t u8x8_Draw1x2String(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s)
{
  u8x8->next_cb = u8x8_ascii_next;
  return u8x8_draw_1x2_string(u8x8, x, y, s);
}

uint8_t u8x8_Draw1x2UTF8(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s)
{
  u8x8->next_cb = u8x8_utf8_next;
  return u8x8_draw_1x2_string(u8x8, x, y, s);
}



uint8_t u8x8_GetUTF8Len(u8x8_t *u8x8, const char *s)
{
  uint16_t e;
  uint8_t cnt = 0;
  u8x8_utf8_init(u8x8);
  for(;;)
  {
    e = u8x8_utf8_next(u8x8, *s);
    if ( e == 0x0ffff )
      break;
    s++;
    if ( e != 0x0fffe )
      cnt++;
  }
  return cnt;
}


