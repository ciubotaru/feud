#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map.h"
#include "player.h"
#include "world.h"

char *const ranklist[] = {
	"knight",
	"baron",
	"count",
	"duke",
	"king"
};

player_t *create_playerlist() {
	/* single instance */
	if (world->playerlist != NULL) return world->playerlist;
	world->playerlist = malloc(sizeof(player_t));
	if (world->playerlist == NULL) return NULL;
	return world->playerlist;
}

static void fill_player_details(player_t *player, const char *name) {
	player->id = world->next_player_id;
	world->next_player_id++;
	strcpy(player->name, name);
	player->money = 0;
	player->rank = 0;
	player->rank_land = 0;
	player->rank_army = 0;
	player->rank_money = 0;
	player->next = NULL;
	player->birthdate.tm_year = world->current_time.tm_year;
	player->birthdate.tm_mon = world->current_time.tm_mon;
	player->lord = NULL;
	player->heir = NULL;
	set_expected_age(player);
}

player_t *add_player(const char *name) {
	if (world->playerlist == NULL) {
		world->playerlist = create_playerlist();
		fill_player_details(world->playerlist, name);
		return world->playerlist;
	}

    player_t *current = world->playerlist;

	/*fast-forward to the end of list */
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(player_t));
	if (!current->next) return NULL;
	fill_player_details(current->next, name);
	return current->next;
}

void print_player_list() {
	if (world->playerlist == NULL) return;
    player_t *current = world->playerlist;
    while (current != NULL) {
		uint16_t age = (world->current_time.tm_year * 12 + world->current_time.tm_mon - current->birthdate.tm_year * 12 - current->birthdate.tm_mon) / 12;
		char *age_ch;
		if (age < 20) age_ch = "child";
		else if (age < 40) age_ch = "young";
		else if (age < 60) age_ch = "middle-aged";
		else if (age < 80) age_ch = "old";
		else age_ch = "very old";
        printf("ID: %i. Name: %s. Money: %i. Age: %s.\n", current->id, current->name, current->money, age_ch);
        current = current->next;
    }
}

void remove_player(player_t *player) {
	if (player == NULL) return;

	/* remove all pieces that belong to this player */
	piece_t *current_piece = world->piecelist;
	piece_t *previous_piece = NULL;
	while (current_piece != NULL) {
		previous_piece = current_piece;
		current_piece = current_piece->next;
		if (previous_piece->owner->id == player->id) {
			remove_piece(previous_piece);
		}
	}

	/* free all regions that belong to this player */
	region_t *current_region = world->regionlist;
	while (current_region != NULL) {
		if (current_region->owner != NULL && current_region->owner->id == player->id) {
			current_region->owner = NULL;
		}
		current_region = current_region->next;
	}
	/* remove all diplomacy that included this player */
	dipstatus_t *dipstatus = world->diplomacylist;
	while (dipstatus != NULL) {
		if (dipstatus->player1 == player || dipstatus->player2 == player) {
			dipstatus_t *tmp = dipstatus->next;
			remove_diplomacy(dipstatus);
			dipstatus = tmp;
		}
		else dipstatus = dipstatus->next;
	}

	/* if this player was set as heir or lord to somebody else, remove him */
    player_t *current = world->playerlist;
	while (current != NULL) {
		if (current->heir != NULL && current->heir->id == player->id) current->heir = NULL;
		if (current->lord != NULL && current->lord->id == player->id) current->lord = NULL;
		current = current->next;
	}

	current = world->playerlist;
	player_t *previous = NULL;
    while (current != NULL) {
		if (current == player) {
			/** if it's the first element in list, i.e. no previous, AND not the last, then set playerlist to next and free current **/
			/** if it's the last element AND not the first, then free current and set previous->next to NULL **/
			/** it there is previous and next, then set previous->next to current->next; free current **/
			/** if it's the ONLY element then free playerlist **/

			/** check if first **/
	        if (previous == NULL) {
				/** ... AND last **/
				if (current->next == NULL) {
					free(current);
					world->playerlist = NULL;
					return;
				}
				/** first, but not last **/
				else {
					player_t *tmp = world->playerlist->next;
					free(world->playerlist);
					world->playerlist = tmp;
					return;
				}
	        }
			/** not first **/
			else {
				/** check if last **/
				if (current->next == NULL) {
					free(current);
					previous->next = NULL;
					return;
				}
				/** not first, not last **/
				else {
					previous->next = current->next;
					free(current);
					return;
				}
			}
		}
		if (current->next == NULL) {
			/** not found **/
			return;
		}
		previous = current;
        current = current->next;
	}
}

void clear_player_list() {
	while (world->playerlist != NULL) remove_player(world->playerlist);
	world->next_player_id = 1;
	world->playerlist = NULL;
}

uint16_t get_money(player_t *player) {
	/* can be accessed without a function */
	return player->money;
}

void set_money(player_t *player, const uint16_t money) {
	if (player == NULL) return;
	player->money = money;
	return;
}

player_t *get_player_by_name(const char *name) {
    player_t *current = world->playerlist;
    while (current != NULL) {
		if (strcmp(current->name, name) == 0) {
			return current;
		}
		if (current->next == NULL) return NULL;
        current = current->next;
	}
	return NULL;	
}

player_t *get_player_by_id(const uint16_t id) {
    player_t *current = world->playerlist;
    while (current != NULL) {
		if (id == current->id) {
			return current;
		}
		if (current->next == NULL) return NULL;
        current = current->next;
	}
	return NULL;	
}

uint16_t get_player_order(player_t *player) {
	if (player == NULL) return 0;
	uint16_t counter = 0;
	player_t *current = world->playerlist;
	while (current != NULL && current->id != player->id) {
		counter++;
		current = current->next;
	}
	return counter;
}

unsigned char get_player_rank(player_t *player) {
	if (player == NULL) return 0;
/**
	piece_t *noble = get_noble_by_owner(player);
	return noble->rank;
**/
	return player->rank;
}

void set_player_rank(player_t *player, unsigned char rank) {
	if (player == NULL) return;
	player->rank = rank;
}

player_t *get_player_by_order(int playerlist_selector) {
	int counter = playerlist_selector;
	player_t *current = world->playerlist;
	while (current->next != NULL && counter > 0) {
		counter--;
		current = current->next;
	}
	return current;
}

int transfer_money(player_t *source, player_t *destination, const int amount) {
	/* check if source and destination exist. NEEDED? */
	if (source == NULL || destination == NULL || source->id == destination->id) return 1;
	/* check if source has enough */
	if (source->money < amount) return 1;
	source->money -= amount;
	destination->money += amount;
	return 0;
}

void set_expected_age(player_t *player) {
/**
	unsigned int years = 0;
	double average = (MAX_AGE - MIN_AGE) / 2;
	double limit = exp(-average), p = 1;
printf("\n%.12f\n", limit);
	do {
	    ++years;
		p *= rand() / (double) RAND_MAX;
	} while (p > limit);
	player->deathdate.tm_year = player->birthdate.tm_year + MIN_AGE + years;
	player->deathdate.tm_mon = rand() % 12;
**/

	unsigned int months = (player->birthdate.tm_year + MIN_AGE) * 12 + player->birthdate.tm_mon;
	months += rand() % ((MAX_AGE - MIN_AGE) *12);
	player->deathdate.tm_year = months / 12;
	player->deathdate.tm_mon = months % 12;
}

void set_successor(player_t *player, player_t *successor) {
	if (player == NULL || (successor != NULL && player->id == successor->id)) return; /* can not name yourself a heir */
	player->heir = successor;
	return;
}

unsigned int count_players() {
	unsigned int count = 0;
	player_t *current = world->playerlist;
	while (current != NULL) {
		count++;
        current = current->next;
	}
	return count;	
}

void update_money_ranking() {
	player_t *player = NULL;
	player_t *player2 = NULL;

	/* reset ranks to 1 */
	player = world->playerlist;
	while (player != NULL) {
		player->rank_money = 1;
		player = player->next;
	}
	/* rewind to start */
	player = world->playerlist;

	while (player != NULL) {
		player2 = world->playerlist;
		while (player2->id != player->id) {
			if (player->money <= player2->money) player->rank_money++;
			else player2->rank_money++;
			player2 = player2->next;
		}
		player = player->next; 
	}
}

int is_gameover() {
	/**
	 * The game is over if one player, or one sovereign with his vassals,
	 * is left
	**/
	player_t *player = NULL;
	int nr_sovereigns = 0;
	player_t *current = world->playerlist;
	while (current != NULL) {
		if (current->lord == NULL) nr_sovereigns++;
		current = current->next;
	}
	if (nr_sovereigns == 1) {
		player = get_sovereign(world->playerlist);
		add_to_cronicle("%s is the only sovereign left. The game is over!", player->name);
		return 1;
	}
	return 0;
}

void check_death() {
	char message[255] = {0};
	if (world->check_death == 0) return;
	/**
	 * loop through playerlist, check if every player has a noble and
	 * at least one region
	**/
	player_t *current_player = world->playerlist;
	player_t *next = NULL;
	while (current_player != NULL) {
		next = current_player->next;
		/* if a player loses all his land, he loses the game */
		if (count_tiles_by_owner(current_player) == 0) {
			add_to_cronicle("%s %s lost all his land.\n", ranklist[current_player->rank], current_player->name);
			add_user_message(message);
			succession(current_player);
			remove_player(current_player);
		}
		/* if a player loses his noble, he loses the game */
		else if (get_noble_by_owner(current_player) == NULL) {
			add_to_cronicle("%s %s was killed.\n", ranklist[current_player->rank], current_player->name);
			add_user_message(message);
			succession(current_player);
			remove_player(current_player);
		}
		/* if a player reaches old age, he loses the game */
		else if (world->current_time.tm_year + 12 * world->current_time.tm_mon >= current_player->deathdate.tm_year + 12 * current_player->deathdate.tm_mon) {
			add_to_cronicle("%s %s died of old age.\n", ranklist[current_player->rank], current_player->name);
			add_user_message(message);
			succession(current_player);
			remove_player(current_player);
		}
		current_player = next;
	}
	world->check_death = 0;
}

void succession(player_t *player) {
	/**
	 * First, find the successor.
	 * If player named a heir, the heir will succeed.
	 * If the player did not name a heir, but has a lord, the lord will succeed.
	 * If the player did not name a heir and has no lord, the property is lost.
	 **/
	player_t *heir = NULL;
	if (player->heir != NULL) {
		heir = player->heir;
		add_to_cronicle("%s %s inherited the property of %s %s.\n", ranklist[heir->rank], heir->name, ranklist[player->rank], player->name);
	}
	else if (player->lord != NULL) {
		heir = player->lord;
		add_to_cronicle("%s %s had not named a heir, and their lord, %s %s, inherited everything.\n", ranklist[player->rank], player->name, ranklist[heir->rank], heir->name);
	}
	else {
		add_to_cronicle("%s %s had no one to inherit their property.\n", ranklist[player->rank], player->name);
		return;
	}

	/* the heir always inherits money */
	heir->money += player->money;

	/* the heir always inherits land */
	region_t *current_region = world->regionlist;
	while (current_region != NULL) {
		if (current_region->owner != NULL && current_region->owner->id == player->id) current_region->owner = heir;
		current_region = current_region->next;
	}

	/* the heir always inherits soldiers */
	piece_t *current_piece = world->piecelist;
	while (current_piece != NULL) {
		if (current_piece->owner->id == player->id && current_piece->type == 1) current_piece->owner = heir;
		current_piece = current_piece->next;
	}

	/* the heir inherits grantor's title, if it's higher than his own */
	if (player->rank > heir->rank) {
		add_to_cronicle("%s inherited the %s title.\n", heir->name, ranklist[player->rank]);
		heir->rank = player->rank;
	}

	/* the heir always inherits grantor's vassals */
	player_t *vassal = world->playerlist;
	while (vassal != NULL) {
		if (vassal->lord != NULL && vassal->lord->id == player->id) {
			vassal->lord = heir;
			set_diplomacy(vassal, player, ALLIANCE);
		}
		vassal = vassal->next;
	}

	/* if the heir has no lord, he does not inherit grantor's lord allegiance */
	if (heir->lord == NULL) return;

	/* if possible, the heir keeps his lord allegiance */
//	piece_t *heir_lord_noble = get_noble_by_owner(heir->lord);
	if (player->rank < heir->lord->rank) return;
	/* only if player's new title is incompatible with current lord allegiance, he has to change allegiance */
	else {
		/* if grantor had no lord, the heir becomes free */
		if (player->lord == NULL) heir->lord = NULL;
		else {
			add_to_cronicle("%s %s paid homage to %s %s for their title.\n", ranklist[heir->rank], heir->name, ranklist[player->lord->rank], player->lord->name);
			homage(heir, player->lord);
		}
	}
}
