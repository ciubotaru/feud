#include <stdio.h>
#include <stdlib.h>
#include "time.h"
#include "player.h"
#include "world.h"

const char *months[] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

void init_gametime()
{
//      if (world->current_time != NULL) return;
//      world->current_time = malloc(sizeof(gametime_t));
	world->current_time.tm_year = 0;
	world->current_time.tm_mon = 0;
}

void set_gametime(uint16_t year, unsigned char mon)
{
//      if (world->current_time == NULL) init_gametime();
	if (mon < 1 || mon > 12)
		return;
	world->current_time.tm_year = year;
	world->current_time.tm_mon = mon;
}

void increment_gametime()
{
//      if (world->current_time == NULL) init_gametime();
	if (world->current_time.tm_mon == 11) {
		world->current_time.tm_year += 1;
		world->current_time.tm_mon = 0;
	} else
		world->current_time.tm_mon += 1;
}

void print_gametime()
{
	printf("It's %s of year %i.\n", months[world->current_time.tm_mon],
	       world->current_time.tm_year);
}
