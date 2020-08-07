#include "ref.h"

q31_t ref__QADD8(q31_t x, q31_t y)
{
   q31_t sum;
   q7_t r, s, t, u;

   r = (q7_t) x;
   s = (q7_t) y;

   r = ref_sat_n((q31_t) (r + s), 8);
   s = ref_sat_n(((q31_t) (((x << 16) >> 24) + ((y << 16) >> 24))), 8);
   t = ref_sat_n(((q31_t) (((x << 8) >> 24) + ((y << 8) >> 24))), 8);
   u = ref_sat_n(((q31_t) ((x >> 24) + (y >> 24))), 8);

   sum =
      (((q31_t) u << 24) & 0xFF000000) | (((q31_t) t << 16) & 0x00FF0000) |
      (((q31_t) s << 8) & 0x0000FF00) | (r & 0x000000FF);

   return sum;

}

q31_t ref__QSUB8(q31_t x, q31_t y)
{
   q31_t sum;
   q31_t r, s, t, u;

   r = (q7_t) x;
   s = (q7_t) y;

   r = ref_sat_n((r - s), 8);
   s = ref_sat_n(((q31_t) (((x << 16) >> 24) - ((y << 16) >> 24))), 8) << 8;
   t = ref_sat_n(((q31_t) (((x << 8) >> 24) - ((y << 8) >> 24))), 8) << 16;
   u = ref_sat_n(((q31_t) ((x >> 24) - (y >> 24))), 8) << 24;

   sum = (u & 0xFF000000) | (t & 0x00FF0000) | (s & 0x0000FF00) | (r & 0x000000FF);

   return sum;
}

q31_t ref__QADD16(q31_t x, q31_t y)
{
   q31_t sum;
   q31_t r, s;

   r = (q15_t) x;
   s = (q15_t) y;

   r = ref_sat_q15(r + s);
   s = (q31_t)ref_sat_q15(((q31_t) ((x >> 16) + (y >> 16)))) << 16;

   sum = (s & 0xFFFF0000) | (r & 0x0000FFFF);

   return sum;

}

q31_t ref__SHADD16(q31_t x, q31_t y)
{
   q31_t sum;
   q31_t r, s;

   r = (q15_t) x;
   s = (q15_t) y;

   r = (r + s) >> 1;
   s = ((q31_t) (((x >> 16) + (y >> 16)) >> 1) << 16);

   sum = (s & 0xFFFF0000) | (r & 0x0000FFFF);

   return sum;

}

q31_t ref__QSUB16(q31_t x, q31_t y)
{
   q31_t sum;
   q31_t r, s;

   r = (q15_t) x;
   s = (q15_t) y;

   r = ref_sat_q15(r - s);
   s = (q31_t)ref_sat_q15(((q31_t) ((x >> 16) - (y >> 16)))) << 16;

   sum = (s & 0xFFFF0000) | (r & 0x0000FFFF);

   return sum;
}

q31_t ref__SHSUB16(q31_t x, q31_t y)
{
   q31_t diff;
   q31_t r, s;

   r = (q15_t) x;
   s = (q15_t) y;

   r = ((r >> 1) - (s >> 1));
   s = (((x >> 17) - (y >> 17)) << 16);

   diff = (s & 0xFFFF0000) | (r & 0x0000FFFF);

   return diff;
}

q31_t ref__QASX(q31_t x, q31_t y)
{
   q31_t sum = 0;
	q31_t xL, xH, yL, yH;
	
   // extract bottom halfword and sign extend
	xL = (q15_t)(x & 0xffff);
   // extract bottom halfword and sign extend
	yL = (q15_t)(y & 0xffff);
   // extract top halfword and sign extend
   xH = (q15_t)(x >> 16);
   // extract top halfword and sign extend
   yH = (q15_t)(y >> 16);
   
   sum = (((q31_t)ref_sat_q15(xH + yL )) << 16) |
         (((q31_t)ref_sat_q15(xL - yH )) & 0xffff);

   return sum;
}

q31_t ref__SHASX(q31_t x, q31_t y)
{
   q31_t sum;
   q31_t r, s;

   r = (q15_t) x;
   s = (q15_t) y;

    r = (r - (y >> 16)) / 2;
    s = (((x >> 16) + s) << 15);

   sum = (s & 0xFFFF0000) | (r & 0x0000FFFF);

   return sum;
}

q31_t ref__QSAX(q31_t x, q31_t y)
{
   q31_t sum = 0;
	q31_t xL, xH, yL, yH;
	
   // extract bottom halfword and sign extend
	xL = (q15_t)(x & 0xffff);
   // extract bottom halfword and sign extend
	yL = (q15_t)(y & 0xffff);
   // extract top halfword and sign extend
   xH = (q15_t)(x >> 16);
   // extract top halfword and sign extend
   yH = (q15_t)(y >> 16);
   
   sum = (((q31_t)ref_sat_q15(xH - yL )) << 16) |
         (((q31_t)ref_sat_q15(xL + yH )) & 0xffff);

   return sum;
}

q31_t ref__SHSAX(q31_t x, q31_t y)
{
   q31_t sum;
   q31_t r, s;

   r = (q15_t) x;
   s = (q15_t) y;

    r = (r + (y >> 16)) / 2;
    s = (((x >> 16) - s) << 15);

   sum = (s & 0xFFFF0000) | (r & 0x0000FFFF);

   return sum;
}

q31_t ref__SMUSDX(q31_t x, q31_t y)
{
   return ((q31_t) (((q15_t) x * (q15_t) (y >> 16)) - ((q15_t) (x >> 16) * (q15_t) y)));
}

q31_t ref__SMUADX(q31_t x, q31_t y)
{
   return ((q31_t) (((q15_t) x * (q15_t) (y >> 16)) + ((q15_t) (x >> 16) * (q15_t) y)));
}

q31_t ref__QADD(q31_t x, q31_t y)
{
   return ref_sat_q31((q63_t) x + y);
}

q31_t ref__QSUB(q31_t x, q31_t y)
{
   return ref_sat_q31((q63_t) x - y);
}

q31_t ref__SMLAD(q31_t x, q31_t y, q31_t sum)
{
   return (sum + ((q15_t) (x >> 16) * (q15_t) (y >> 16)) + ((q15_t) x * (q15_t) y));
}

q31_t ref__SMLADX(q31_t x, q31_t y, q31_t sum)
{
   return (sum + ((q15_t) (x >> 16) * (q15_t) (y)) + ((q15_t) x * (q15_t) (y >> 16)));
}

q31_t ref__SMLSDX(q31_t x, q31_t y, q31_t sum)
{
   return (sum - ((q15_t) (x >> 16) * (q15_t) (y)) + ((q15_t) x * (q15_t) (y >> 16)));
}

q63_t ref__SMLALD(q31_t x, q31_t y, q63_t sum)
{
   return (sum + ((q15_t) (x >> 16) * (q15_t) (y >> 16)) + ((q15_t) x * (q15_t) y));
}

q63_t ref__SMLALDX(q31_t x, q31_t y, q63_t sum)
{
   return (sum + ((q15_t) (x >> 16) * (q15_t) y)) + ((q15_t) x * (q15_t) (y >> 16));
}

q31_t ref__SMUAD(q31_t x, q31_t y)
{
   return (((x >> 16) * (y >> 16)) + (((x << 16) >> 16) * ((y << 16) >> 16)));
}

q31_t ref__SMUSD(q31_t x, q31_t y)
{
   return (-((x >> 16) * (y >> 16)) + (((x << 16) >> 16) * ((y << 16) >> 16)));
}

q31_t ref__SXTB16(q31_t x)
{
   return ((((x << 24) >> 24) & 0x0000FFFF) | (((x << 8) >> 8) & 0xFFFF0000));
}
