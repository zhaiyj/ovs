/*
 * Copyright (c) 2008, 2009 Nicira Networks.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HASH_H
#define HASH_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "util.h"

/* This is the public domain lookup3 hash by Bob Jenkins from
 * http://burtleburtle.net/bob/c/lookup3.c, modified for style. */

#define HASH_ROT(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define HASH_MIX(a, b, c)                       \
    do {                                        \
      a -= c; a ^= HASH_ROT(c,  4); c += b;     \
      b -= a; b ^= HASH_ROT(a,  6); a += c;     \
      c -= b; c ^= HASH_ROT(b,  8); b += a;     \
      a -= c; a ^= HASH_ROT(c, 16); c += b;     \
      b -= a; b ^= HASH_ROT(a, 19); a += c;     \
      c -= b; c ^= HASH_ROT(b,  4); b += a;     \
    } while (0)

#define HASH_FINAL(a, b, c)                     \
    do {                                        \
      c ^= b; c -= HASH_ROT(b, 14);             \
      a ^= c; a -= HASH_ROT(c, 11);             \
      b ^= a; b -= HASH_ROT(a, 25);             \
      c ^= b; c -= HASH_ROT(b, 16);             \
      a ^= c; a -= HASH_ROT(c,  4);             \
      b ^= a; b -= HASH_ROT(a, 14);             \
      c ^= b; c -= HASH_ROT(b, 24);             \
    } while (0)

uint32_t hash_words(const uint32_t *, size_t n_word, uint32_t basis);
uint32_t hash_2words(const uint32_t *, uint32_t basis);
uint32_t hash_bytes(const void *, size_t n_bytes, uint32_t basis);

static inline uint32_t hash_string(const char *s, uint32_t basis)
{
    return hash_bytes(s, strlen(s), basis);
}

/* This is Bob Jenkins' integer hash from
 * http://burtleburtle.net/bob/hash/integer.html, modified for style. */
static inline uint32_t hash_int(uint32_t x, uint32_t basis)
{
    x -= x << 6;
    x ^= x >> 17;
    x -= x << 9;
    x ^= x << 4;
    x -= x << 3;
    x ^= x << 10;
    x ^= x >> 15;
    return x + basis;
}

/* An attempt at a useful 1-bit hash function.  Has not been analyzed for
 * quality. */
static inline uint32_t hash_boolean(bool x, uint32_t basis)
{
    enum { P0 = 0xc2b73583 };   /* This is hash_int(1, 0). */
    enum { P1 = 0xe90f1258 };   /* This is hash_int(2, 0). */
    return (x ? P0 : P1) ^ HASH_ROT(basis, 1);
}

static inline uint32_t hash_double(double x, uint32_t basis)
{
    BUILD_ASSERT_DECL(sizeof x == 8);
    return hash_2words((const uint32_t *) &x, basis);
}

static inline uint32_t hash_pointer(const void *p, uint32_t basis)
{
    /* Often pointers are hashed simply by casting to integer type, but that
     * has pitfalls since the lower bits of a pointer are often all 0 for
     * alignment reasons.  It's hard to guess where the entropy really is, so
     * we give up here and just use a high-quality hash function.
     *
     * The double cast suppresses a warning on 64-bit systems about casting to
     * an integer to different size.  That's OK in this case, since most of the
     * entropy in the pointer is almost certainly in the lower 32 bits. */
    return hash_int((uint32_t) (uintptr_t) p, basis);
}

#endif /* hash.h */
