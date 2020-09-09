/*

  u8g2_intersection.c 
  
  Intersection calculation, code taken from u8g_clip.c

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

#ifdef __GNUC__
#define U8G2_ALWAYS_INLINE __inline__ __attribute__((always_inline))
#else
#define U8G2_ALWAYS_INLINE
#endif 


#if defined(U8G2_WITH_INTERSECTION) || defined(U8G2_WITH_CLIP_WINDOW_SUPPORT)

#ifdef OLD_VERSION_WITH_SYMETRIC_BOUNDARIES

/*
  intersection assumptions:
    a1 <= a2 is always true    
    
    minimized version
    ---1----0 1             b1 <= a2 && b1 > b2
    -----1--0 1             b2 >= a1 && b1 > b2
    ---1-1--- 1             b1 <= a2 && b2 >= a1
  */


/*
  calculate the intersection between a0/a1 and v0/v1
  The intersection check returns one if the range of a0/a1 has an intersection with v0/v1.
  The intersection check includes the boundary values v1 and a1.

  The following asserts will succeed:
    assert( u8g2_is_intersection_decision_tree(4, 6, 7, 9) == 0 );
    assert( u8g2_is_intersection_decision_tree(4, 6, 6, 9) != 0 );
    assert( u8g2_is_intersection_decision_tree(6, 9, 4, 6) != 0 );
    assert( u8g2_is_intersection_decision_tree(7, 9, 4, 6) == 0 );  
*/

//static uint8_t U8G2_ALWAYS_INLINE u8g2_is_intersection_decision_tree(u8g_uint_t a0, u8g_uint_t a1, u8g_uint_t v0, u8g_uint_t v1) 
static uint8_t u8g2_is_intersection_decision_tree(u8g2_uint_t a0, u8g2_uint_t a1, u8g2_uint_t v0, u8g2_uint_t v1) 
{
  if ( v0 <= a1 )
  {
    if ( v1 >= a0 )
    {
      return 1;
    }
    else
    {
      if ( v0 > v1 )
      {
	return 1;
      }
      else
      {
	return 0;
      }
    }
  }
  else
  {
    if ( v1 >= a0 )
    {
      if ( v0 > v1 )
      {
	return 1;
      }
      else
      {
	return 0;
      }
    }
    else
    {
      return 0;
    }
  }
}

#endif	/* OLD_VERSION_WITH_SYMETRIC_BOUNDARIES */


/*
  version with asymetric boundaries.
  a1 and v1 are excluded
  v0 == v1 is not support end return 1
*/
uint8_t u8g2_is_intersection_decision_tree(u8g2_uint_t a0, u8g2_uint_t a1, u8g2_uint_t v0, u8g2_uint_t v1)
{
  if ( v0 < a1 )		// v0 <= a1
  {
    if ( v1 > a0 )	// v1 >= a0
    {
      return 1;
    }
    else
    {
      if ( v0 > v1 )	// v0 > v1
      {
	return 1;
      }
      else
      {
	return 0;
      }
    }
  }
  else
  {
    if ( v1 > a0 )	// v1 >= a0
    {
      if ( v0 > v1 )	// v0 > v1
      {
	return 1;
      }
      else
      {
	return 0;
      }
    }
    else
    {
      return 0;
    }
  }
}



/* upper limits are not included (asymetric boundaries) */
uint8_t u8g2_IsIntersection(u8g2_t *u8g2, u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t x1, u8g2_uint_t y1)
{
  if ( u8g2_is_intersection_decision_tree(u8g2->user_y0, u8g2->user_y1, y0, y1) == 0 )
    return 0; 
  
  return u8g2_is_intersection_decision_tree(u8g2->user_x0, u8g2->user_x1, x0, x1);
}


#endif /* U8G2_WITH_INTERSECTION */

