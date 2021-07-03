#ifndef DIPLOMACY_H
#define DIPLOMACY_H

#include "character.h"
#include "piece.h"
#include "map.h"

enum dipstatuslist {
	ALLIANCE_BIT,
	WAR_BIT,
	OFFER_SENT_BIT,
	OFFER_RECEIVED_BIT,
	HEIR_BIT, //character1 sets character2 as their heir
	GRANTOR_BIT,
	NR_DIPSTATUSES
};

#define REJECT 0
#define ACCEPT 1

#define NEUTRAL 0
#define ALLIANCE (1 << ALLIANCE_BIT)
#define WAR (1 << WAR_BIT)
#define OFFER_SENT (1 << OFFER_SENT_BIT)
#define OFFER_RECEIVED (1 << OFFER_RECEIVED_BIT)
#define HEIR (1 << HEIR_BIT)
#define GRANTOR (1 << GRANTOR_BIT)

extern char *const dipstatus_name[];

#define DIPLOMACY_MASK (unsigned char) (ALLIANCE | WAR)
#define OFFER_MASK (unsigned char) (OFFER_SENT | OFFER_RECEIVED)
#define SUCCESSION_MASK (unsigned char) (HEIR | GRANTOR)

typedef struct dipstatus {
	character_t *character1;
	character_t *character2;
	unsigned char status;
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

void promote_soldier(character_t * character, piece_t * piece, region_t * region,
		     char *name);

void promote_vassal(character_t * lord, character_t * vassal);

uint16_t count_vassals(character_t * character);

character_t *get_sovereign(character_t * character);

void set_offer(character_t *from, character_t *to, const unsigned char offer_status);

unsigned char get_offer(character_t *from, character_t *to);

void open_offer(character_t *from, character_t *to);

void close_offer(character_t *from, character_t *to, const unsigned char result);

void set_successor(character_t *character, character_t *heir);

character_t *get_successor(character_t *character);

character_t *get_liege(character_t *character);

void set_liege(character_t *character, character_t *liege);

#endif
