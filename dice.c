#include <stdlib.h>
#include <stdio.h>
#include "dice.h"
#include "world.h"

#ifdef CUSTOM_RNG

#ifndef CHAR_BIT
#include <limits.h>
#endif
#ifndef BITS_PER_LONG
#define BITS_PER_LONG (unsigned int) (sizeof(unsigned long) * CHAR_BIT)
#endif
#define POOL_SIZE_BITS 257
#define POOL_SIZE_BYTES ((POOL_SIZE_BITS - 1) / sizeof(unsigned long) + 1)
#define POOL_SIZE_LONGS ((POOL_SIZE_BITS - 1) / sizeof(unsigned long) / CHAR_BIT + 1)
#define ENTRY_BIT (POOL_SIZE_BITS / 2)
#define ENTRY_LONG ((ENTRY_BIT - 1) / (sizeof(unsigned long) * 8) + 1)

static unsigned long pool[POOL_SIZE_LONGS] = {0};
static unsigned long r[POOL_SIZE_LONGS] = {0};
static unsigned long l[POOL_SIZE_LONGS] = {0};
static unsigned long o[POOL_SIZE_LONGS] = {0};

inline static unsigned char pool_valid() {
	int i;
	for (i = 0; i < POOL_SIZE_LONGS; i++) if (pool[i]) return 1;
	return 0;
}

inline static void right() {
	int i;
	for (i = 0; i < POOL_SIZE_LONGS; i++) r[i] = pool[i] >> 1;
	for (i = 1; i < POOL_SIZE_LONGS; i++) r[i-1] |= (pool[i] & 1UL) << (BITS_PER_LONG - 1);
	r[POOL_SIZE_LONGS - 1] |= (pool[0] & 1UL) << ((POOL_SIZE_BITS - 1) % BITS_PER_LONG);
}

inline static void left() {
	int i;
	for (i = 0; i < POOL_SIZE_LONGS; i++) l[i] = (pool[i] << 1);
	for (i = 1; i < POOL_SIZE_LONGS; i++) l[i] |= (pool[i - 1] >> (BITS_PER_LONG - 1)) & 1UL;
	l[0] |= (pool[POOL_SIZE_LONGS-1] >> ((POOL_SIZE_BITS - 1) % BITS_PER_LONG)) & 1UL;
}

inline static void rule30() {
	int i;
	right();
	left();
	for (i = 0; i < POOL_SIZE_LONGS; i++) {
		o[i] = pool[i] | r[i];
		pool[i] = l[i] ^ o[i];
	}
}

inline static void init_pool() {
	unsigned char bit;
	int i;
	for (i = 0; i < POOL_SIZE_BITS; i++) {
		bit = rand() % 2;
		if (bit) pool[ENTRY_LONG] ^= 1UL;
		rule30();
	}
}

unsigned char get_dice() {
	unsigned char ret = 0;
	int i;
	while (pool_valid() != 1) {
		init_pool();
	}
	for (i = 0; i < 8; i++) {
		if (pool[0] & 1UL) ret |= (pool[0] & 1UL) << (i % 8);
		rule30();
	}
	return ret % 6 + 1;
}

#else /* CUSTOM_RNG not defined */

unsigned char get_dice() {
	return (rand() % 6) + 1;
}

#endif
