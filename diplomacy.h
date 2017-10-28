#ifndef DIPLOMACY_H
#define DIPLOMACY_H

#include "player.h"
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
	player_t *from;
	player_t *to;
	unsigned char offer;
} dipoffer_t;

typedef struct dipstatus {
	player_t *player1;
	player_t *player2;
	unsigned char status;
	dipoffer_t *pending_offer;
	struct dipstatus *next;
} dipstatus_t;

dipstatus_t *set_diplomacy(player_t * name1, player_t * name2,
			   const unsigned int status);

dipstatus_t *get_dipstatus(player_t * name1, player_t * name2);

unsigned char get_diplomacy(player_t * name1, player_t * name2);

void remove_diplomacy(dipstatus_t * dipstatus);

void print_diplomacy_list();

char *diplomacy_message(const int dipstatus);

void homage(player_t * player, player_t * lord);

void promote_soldier(player_t * player, piece_t * piece, region_t * region,
		     char *name);

void promote_vassal(player_t * lord, player_t * vassal);

uint16_t count_vassals(player_t * player);

player_t *get_sovereign(player_t * player);

dipoffer_t *open_offer(player_t * from, player_t * to,
		       const unsigned int offer);

void close_offer(dipoffer_t * offer, const unsigned int result);

#endif
