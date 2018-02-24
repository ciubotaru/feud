#ifndef DIPLOMACY_H
#define DIPLOMACY_H

#include "character.h"
#include "piece.h"
#include "map.h"

#define NEUTRAL 0
#define ALLIANCE 1
#define WAR 2
#define REJECT 0
#define ACCEPT 1

char *const dipofferlist[2];	/* peace offer and alliance offer */

char *const dipstatuslist[3];

typedef struct dipoffer {
	character_t *from;
	character_t *to;
	unsigned char offer;
} dipoffer_t;

typedef struct dipstatus {
	character_t *character1;
	character_t *character2;
	unsigned char status;
	dipoffer_t *pending_offer;
	struct dipstatus *next;
} dipstatus_t;

dipstatus_t *set_diplomacy(character_t * name1, character_t * name2,
			   const unsigned int status);

dipstatus_t *get_dipstatus(character_t * name1, character_t * name2);

unsigned char get_diplomacy(character_t * name1, character_t * name2);

void remove_diplomacy(dipstatus_t * dipstatus);

void homage(character_t * character, character_t * lord);

void promote_soldier(character_t * character, piece_t * piece, region_t * region,
		     char *name);

void promote_vassal(character_t * lord, character_t * vassal);

uint16_t count_vassals(character_t * character);

character_t *get_sovereign(character_t * character);

dipoffer_t *open_offer(character_t * from, character_t * to,
		       const unsigned int offer);

void close_offer(dipoffer_t * offer, const unsigned int result);

#endif
