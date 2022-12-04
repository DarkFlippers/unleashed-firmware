/*
 * TamaLIB - A hardware agnostic tama P1 emulation library
 *
 * Copyright (C) 2021 Jean-Christophe Rona <jc@rona.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef _HAL_TYPES_H_
#define _HAL_TYPES_H_

#include <furi.h>

typedef bool bool_t;
typedef uint8_t u4_t;
typedef uint8_t u5_t;
typedef uint8_t u8_t;
typedef uint16_t u12_t;
typedef uint16_t u13_t;
typedef uint32_t u32_t;
typedef uint32_t
    timestamp_t; // WARNING: Must be an unsigned type to properly handle wrapping (u32 wraps in around 1h11m when expressed in us)

#endif /* _HAL_TYPES_H_ */
