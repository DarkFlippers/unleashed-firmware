/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cs_varint.h"

size_t cs_varint_llen(uint64_t num) {
  size_t llen = 0;

  do {
    llen++;
  } while (num >>= 7);

  return llen;
}

size_t cs_varint_encode(uint64_t num, uint8_t *buf, size_t buf_size) {
  size_t llen = 0;

  do {
    uint8_t byte = num & 0x7f;
    num >>= 7;
    if (num != 0) byte |= 0x80;
    if (llen < buf_size) *buf++ = byte;
    llen++;
  } while (num != 0);

  return llen;
}

bool cs_varint_decode(const uint8_t *buf, size_t buf_size, uint64_t *num,
                      size_t *llen) {
  size_t i = 0, shift = 0;
  uint64_t n = 0;

  do {
    if (i == buf_size || i == (8 * sizeof(*num) / 7 + 1)) return false;
    /*
     * Each byte of varint contains 7 bits, in little endian order.
     * MSB is a continuation bit: it tells whether next byte is used.
     */
    n |= ((uint64_t)(buf[i] & 0x7f)) << shift;
    /*
     * First we increment i, then check whether it is within boundary and
     * whether decoded byte had continuation bit set.
     */
    i++;
    shift += 7;
  } while (shift < sizeof(uint64_t) * 8 && (buf[i - 1] & 0x80));

  *num = n;
  *llen = i;

  return true;
}

uint64_t cs_varint_decode_unsafe(const uint8_t *buf, int *llen) {
  uint64_t v;
  size_t l;
  cs_varint_decode(buf, ~0, &v, &l);
  *llen = l;
  return v;
}
