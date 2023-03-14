/* hmac-sha512.c -- hashed message authentication codes
   Copyright (C) 2018-2022 Free Software Foundation, Inc.

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

#include "hmac_sha512.h"

#include "sha512.h"

#define GL_HMAC_NAME 512
#define GL_HMAC_BLOCKSIZE 128
#define GL_HMAC_HASHSIZE 64
#include "hmac_common.h"
