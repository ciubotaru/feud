#ifndef GAMETIME_H
#define GAMETIME_H
#define MIN_AGE 60
#define MAX_AGE 100
#include <stdint.h>

const char *months[12];

typedef struct gametime {
	uint16_t tm_year;
	unsigned char tm_mon;
} gametime_t;

//gametime_t current_time;

void init_gametime();

void set_gametime(uint16_t year, unsigned char mon);

void increment_gametime();

void print_gametime();

#endif				/* GAMETIME_H */
