#include "ref.h"

float32_t ref_detrm(float32_t *pSrc, float32_t *temp, uint32_t size)
{
   float32_t s = 1, det = 0;
   int i, j, m, n, c;

   if ( size == 1 )
   {
      return ( pSrc[ 0 ] );
   }
   else
   {
      det = 0;

      for ( c = 0;c < size;c++ )
      {
         m = 0;
         n = 0;

         for ( i = 0;i < size;i++ )
         {
            for ( j = 0;j < size;j++ )
            {
               temp[ i*size + j ] = 0;

               if ( i != 0 && j != c )
               {
                  temp[ m*(size-1) + n ] = pSrc[ i*size + j ];

                  if ( n < ( size - 2 ) )
                  {
                     n++;
                  }
                  else
                  {
                     n = 0;
                     m++;
                  }
               }
            }
         }

         det += s * ( pSrc[ c ] * ref_detrm( temp, temp + size*size, size - 1 ) );
         s = -s;
      }
   }
   
   return ( det );
}


void ref_cofact(float32_t *pSrc, float32_t *pDst, float32_t *temp, uint32_t size)
{
   int p, q, m, n, i, j;
	
	 if (size == 1)
	 {
		 pDst[0] = 1;
		 return;
	 }

   for ( q = 0;q < size;q++ )
   {
      for ( p = 0;p < size;p++ )
      {
         m = 0;
         n = 0;

         for ( i = 0;i < size;i++ )
         {
            for ( j = 0;j < size;j++ )
            {
               temp[ i*size + j ] = 0;

               if ( i != q && j != p )
               {
                  temp[ m*(size-1) + n ] = pSrc[ i*size + j ];

                  if ( n < ( size - 2 ) )
									{
										n++;
									}
                  else
                  {
                     n = 0;
                     m++;
                  }
               }
            }
         }

         pDst[ q*size + p ] = ref_pow( -1, q + p ) * ref_detrm( temp, temp + (size-1)*(size-1), size - 1 );
      }
   }
}



float64_t ref_detrm64(float64_t *pSrc, float64_t *temp, uint32_t size)
{
   float64_t s = 1, det = 0;
   int i, j, m, n, c;

   if ( size == 1 )
   {
      return ( pSrc[ 0 ] );
   }
   else
   {
      det = 0;

      for ( c = 0;c < size;c++ )
      {
         m = 0;
         n = 0;

         for ( i = 0;i < size;i++ )
         {
            for ( j = 0;j < size;j++ )
            {
               temp[ i*size + j ] = 0;

               if ( i != 0 && j != c )
               {
                  temp[ m*(size-1) + n ] = pSrc[ i*size + j ];

                  if ( n < ( size - 2 ) )
                  {
                     n++;
                  }
                  else
                  {
                     n = 0;
                     m++;
                  }
               }
            }
         }

         det += s * ( pSrc[ c ] * ref_detrm64( temp, temp + size*size, size - 1 ) );
         s = -s;
      }
   }
   
   return ( det );
}


void ref_cofact64(float64_t *pSrc, float64_t *pDst, float64_t *temp, uint32_t size)
{
   int p, q, m, n, i, j;
	
	 if (size == 1)
	 {
		 pDst[0] = 1;
		 return;
	 }

   for ( q = 0;q < size;q++ )
   {
      for ( p = 0;p < size;p++ )
      {
         m = 0;
         n = 0;

         for ( i = 0;i < size;i++ )
         {
            for ( j = 0;j < size;j++ )
            {
               temp[ i*size + j ] = 0;

               if ( i != q && j != p )
               {
                  temp[ m*(size-1) + n ] = pSrc[ i*size + j ];

                  if ( n < ( size - 2 ) )
									{
										n++;
									}
                  else
                  {
                     n = 0;
                     m++;
                  }
               }
            }
         }

         pDst[ q*size + p ] = ref_pow( -1, q + p ) * ref_detrm64( temp, temp + (size-1)*(size-1), size - 1 );
      }
   }
}
