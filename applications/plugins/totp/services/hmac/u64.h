/* uint64_t-like operations that work even on hosts lacking uint64_t

   Copyright (C) 2006, 2009-2022 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Paul Eggert.  */

#pragma once

#include <stdint.h>

#ifndef _GL_U64_INLINE
#define _GL_U64_INLINE _GL_INLINE
#endif

/* Return X rotated left by N bits, where 0 < N < 64.  */
#define u64rol(x, n) u64or(u64shl(x, n), u64shr(x, 64 - (n)))

/* Native implementations are trivial.  See below for comments on what
   these operations do.  */
typedef uint64_t u64;
#define u64hilo(hi, lo) ((u64)(((u64)(hi) << 32) + (lo)))
#define u64init(hi, lo) u64hilo(hi, lo)
#define u64lo(x) ((u64)(x))
#define u64size(x) u64lo(x)
#define u64lt(x, y) ((x) < (y))
#define u64and(x, y) ((x) & (y))
#define u64or(x, y) ((x) | (y))
#define u64xor(x, y) ((x) ^ (y))
#define u64plus(x, y) ((x) + (y))
#define u64shl(x, n) ((x) << (n))
#define u64shr(x, n) ((x) >> (n))
