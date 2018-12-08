#ifndef CHARACTER_H
#define CHARACTER_H
#include <stdint.h>
#include "time.h"

/* noble ranks */
enum ranklist {KNIGHT, BARON, COUNT, DUKE, KING};

#define MONEY_MAX 999
#define MONEY_MAX_DIGITS 3

extern char *const rank_name[];

typedef struct character {
	uint16_t id;
	char name[17];		/* 16 + endchar */
	uint16_t money;
	enum ranklist rank;
	uint16_t rank_land;
	uint16_t rank_army;
	uint16_t rank_money;
	struct gametime birthdate;
	struct gametime deathdate;
	struct character *lord;
	struct character *heir;
//      uint16_t nr_vassals;
//      struct character **vassals;
	struct character *prev;
	struct character *next;
} character_t;

character_t *add_character(const char *name);

character_t *add_character_before(character_t *parent, const char *name);

void remove_character(character_t *character);

void clear_character_list();

uint16_t get_money(character_t *character);

void set_money(character_t *character, const uint16_t money);

character_t *get_character_by_name(const char *name);

character_t *get_character_by_id(const uint16_t id);

uint16_t get_character_order(character_t *character);

unsigned char get_character_rank(character_t *character);

void set_character_rank(character_t *character, unsigned char rank);

character_t *get_character_by_order(int characterlist_selector);

int transfer_money(character_t *source, character_t *destination, const int amount);

void set_expected_age(character_t *character);

void set_successor(character_t *character, character_t *successor);

unsigned int count_characters();

void update_money_ranking();

int is_gameover();

void check_death();

void succession(character_t *character);

#endif				/* CHARACTER_H */
