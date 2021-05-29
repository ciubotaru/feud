#ifndef DIPLOMACY_H
#define DIPLOMACY_H

#include "character.h"
#include "piece.h"
#include "map.h"

enum dipstatuslist {NEUTRAL, ALLIANCE, WAR, NR_DIPSTATUSES};

#define REJECT 0
#define ACCEPT 1

enum offerlist {OFFER_SENT, OFFER_RECEIVED};

#define OFFER_SENT_BIT (1 << OFFER_SENT)
#define OFFER_RECEIVED_BIT (1 << OFFER_RECEIVED)

extern char *const dipstatus_name[];

typedef struct dipstatus {
	character_t *character1;
	character_t *character2;
	unsigned char status;
	unsigned char offer;
	struct dipstatus *prev;
	struct dipstatus *next;
} dipstatus_t;

dipstatus_t *set_diplomacy(character_t * name1, character_t * name2,
			   const unsigned int status);

unsigned char get_diplomacy(character_t * name1, character_t * name2);

void remove_diplomacy(dipstatus_t * dipstatus);

void clear_diplomacy_list();

void remove_redundant_diplomacy();

void homage(character_t * character, character_t * lord);

void unhomage(character_t *character);

void promote_soldier(character_t * character, piece_t * piece, region_t * region,
		     char *name);

void promote_vassal(character_t * lord, character_t * vassal);

uint16_t count_vassals(character_t * character);

character_t *get_sovereign(character_t * character);

void set_offer(character_t *from, character_t *to, const unsigned char offer_status);

unsigned char get_offer(character_t *from, character_t *to);

void open_offer(character_t *from, character_t *to);

void close_offer(character_t *from, character_t *to, const unsigned char result);

void sort_diplomacy_list();

#endif
