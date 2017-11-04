#ifndef PLAYER_H
#define PLAYER_H
#include <stdint.h>
#include "time.h"

/* player ranks */
#define KNIGHT 0
#define BARON 1
#define COUNT 2
#define DUKE 3
#define KING 4

char *const ranklist[5];

typedef struct player {
	uint16_t id;
	char name[17];		/* 16 + endchar */
	uint16_t money;
	unsigned char rank;
	uint16_t rank_land;
	uint16_t rank_army;
	uint16_t rank_money;
	struct gametime birthdate;
	struct gametime deathdate;
	struct player *lord;
	struct player *heir;
//      uint16_t nr_vassals;
//      struct player **vassals;
	struct player *next;
} player_t;

player_t *add_player(const char *name);

void remove_player(player_t * player);

void clear_player_list();

uint16_t get_money(player_t * player);

void set_money(player_t * player, const uint16_t money);

player_t *get_player_by_name(const char *name);

player_t *get_player_by_id(const uint16_t id);

uint16_t get_player_order(player_t * player);

unsigned char get_player_rank(player_t * player);

void set_player_rank(player_t * player, unsigned char rank);

player_t *get_player_by_order(int playerlist_selector);

int transfer_money(player_t * source, player_t * destination, const int amount);

void set_expected_age(player_t * player);

void set_successor(player_t * player, player_t * successor);

unsigned int count_players();

void update_money_ranking();

int is_gameover();

void check_death();

void succession(player_t * player);

#endif				/* PLAYER_H */
