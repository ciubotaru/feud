#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "diplomacy.h"
#include "world.h"

char *const dipstatus_name[] = {
	[NEUTRAL] = "neutral",
	[ALLIANCE] = "alliance",
	[WAR] = "war"
};

dipstatus_t *create_diplomacylist()
{
	/* single instance */
	if (world->diplomacylist != NULL)
		return world->diplomacylist;
	world->diplomacylist = malloc(sizeof(dipstatus_t));
	if (world->diplomacylist == NULL) exit(EXIT_FAILURE);
	return world->diplomacylist;
}

inline static void fill_diplomacy_details(dipstatus_t *diplomacy, character_t *character1,
			   character_t *character2, const unsigned int status)
{
	if (!diplomacy || !character1 || !character2) return;
	if (character1->id < character2->id) {
		diplomacy->character1 = character1;
		diplomacy->character2 = character2;
	}
	else {
		diplomacy->character1 = character2;
		diplomacy->character2 = character1;
	}
	diplomacy->status = status;
	diplomacy->offer = 0;
	diplomacy->prev = NULL;
	diplomacy->next = NULL;
}

inline static dipstatus_t *get_dipstatus_if_exist(character_t *character1, character_t *character2) {
	if (character1 == NULL || character2 == NULL) return NULL;

	character_t *first, *second;
	if (character1->id < character2->id) {
		first = character1;
		second = character2;
	}
	else {
		first = character2;
		second = character1;
	}

	dipstatus_t *current = world->diplomacylist;
	while (current != NULL) {
		if (current->character1 == first && current->character2 == second) {
			break;
		}
		current = current->next;
	}
	return current;
}

inline static dipstatus_t *add_dipstatus(character_t *character1, character_t *character2,
			   const unsigned int status)
{
	if (character1 == NULL || character2 == NULL) return NULL;

	character_t *first, *second;
	if (character1->id < character2->id) {
		first = character1;
		second = character2;
	}
	else {
		first = character2;
		second = character1;
	}

	dipstatus_t *current = world->diplomacylist;
	dipstatus_t *prev = NULL;

	/* if diplomacylist does not exist, create it */
	if (!current) {
		current = create_diplomacylist();
		fill_diplomacy_details(current, first, second, status);
		world->diplomacylist = current;
		return current;
	}
	else {
		while (current && (current->character1->id < first->id || (current->character1 == first && current->character2->id < second->id))) {
			prev = current;
			current = current->next;
		}
		dipstatus_t *new = malloc(sizeof(dipstatus_t));
		if (!new) exit(EXIT_FAILURE);
		fill_diplomacy_details(new, character1, character2, status);
		if (prev) {
			prev->next = new;
			new->prev = prev;
		}
		else {
			world->diplomacylist = new;
		}
		if (current) {
			current->prev = new;
			new->next = current;
		}
		return new;
	}
}

dipstatus_t *set_diplomacy(character_t *character1, character_t *character2,
			   const unsigned int status)
{
	dipstatus_t *current = get_dipstatus_if_exist(character1, character2);
	if (current) {
		current->status = status;
	}
	else current = add_dipstatus(character1, character2, status);
	return current;
}

dipstatus_t *get_dipstatus(character_t *character1, character_t *character2)
{
	dipstatus_t *current = get_dipstatus_if_exist(character1, character2);
	if (current) return current;
	/* if nothing found, create neutral diplomacy and return it */
	return add_dipstatus(character1, character2, NEUTRAL);
}

unsigned char get_diplomacy(character_t *character1, character_t *character2)
{
	dipstatus_t *dipstatus = get_dipstatus(character1, character2);
	if (dipstatus) return dipstatus->status;
	if ((character1->lord && character1->lord == character2)
	    || (character2->lord && character2->lord == character1))
		return ALLIANCE;
	return NEUTRAL;
}

void remove_diplomacy(dipstatus_t *dipstatus)
{
	if (dipstatus == NULL)
		return;

	dipstatus_t *current = world->diplomacylist;
	while (current != NULL) {
		if (current == dipstatus) {
			if (current->prev) current->prev->next = current->next;
			else world->diplomacylist = current->next;
			if (current->next) current->next->prev = current->prev;
			dipstatus_t *tmp = current;
			free(tmp);
			return;
		}
		current = current->next;
	}
}

void clear_diplomacy_list() {
	if (!world) return;
	dipstatus_t *current = world->diplomacylist;
	dipstatus_t *next = current;
	while (current) {
		next = current->next;
		remove_diplomacy(current);
		current = next;
	}
}

void remove_redundant_diplomacy() {
	dipstatus_t *current = world->diplomacylist;
	dipstatus_t *next = current;
	while (current) {
		next = current->next;
		if (current->character1 == current->character2 ||
			(current->status == NEUTRAL && current->offer == 0) ||
			current->character1->lord == current->character2 ||
			current->character1 == current->character2->lord
		) remove_diplomacy(current);
		current = next;
	}
}

void homage(character_t *character, character_t *lord)
{
	if (character == NULL || lord == NULL)
		return;
	/* can not switch lord */
	if (character->lord != NULL)
		return;
	/* can not be a lord of yourself */
	if (character == lord)
		return;
	/* can not pay homage to a baron */
	unsigned char character_rank = get_character_rank(character);
	unsigned char lord_rank = get_character_rank(lord);
//      piece_t *lord_noble = get_noble_by_owner(lord);
	if (lord_rank <= 1)
		return;
	/**
	 * if lord's rank is lower or same as vassal,
	 * then vassal's rank decreases,
	 * and all his high-rank vassals become free
	**/
	if (character_rank >= lord_rank) {
		/* demote the character */
		set_character_rank(character, lord_rank - 1);
		character_rank = lord_rank - 1;
		character_t *current_character = world->characterlist;
		while (current_character != NULL) {
			/* free vassals */
			if (current_character->lord != NULL
			    && current_character->lord == character
			    && get_character_rank(current_character) >= character_rank)
				current_character->lord = NULL;
			current_character = current_character->next;
		}
	}
	character->lord = lord;
	set_diplomacy(character, lord, ALLIANCE);
}

void unhomage(character_t *character) {
	if (!character) return;
	character->lord = NULL;
}
/* not used */
void promote_soldier(character_t *character, piece_t *piece, region_t *region,
		     char *name)
{
	if (character == NULL || piece == NULL || region == NULL)
		return;
	/* promotion costs money */
	if (character->money < COST_SOLDIER)
		return;
	/* can't promote somebody else's piece */
	if (piece->owner != character)
		return;
	/* can't give somebody else's region */
	if (region->owner != character)
		return;
	/* character must have at least two regions (one for new character) */
	if (count_regions_by_owner(character) < 2)
		return;
	character_t *vassal = add_character_before(character, name);
//      piece->rank = 1;
	set_character_rank(vassal, 1);
	piece->owner = vassal;
	change_region_owner(vassal, region);
	set_diplomacy(character, vassal, ALLIANCE);
	character->money -= COST_SOLDIER;
}

void promote_vassal(character_t *lord, character_t *vassal)
{
	if (lord == NULL || vassal == NULL)
		return;
	/* vassal's new rank should be lower than lord's */
	if (vassal->rank + 1 <= lord->rank)
		return;
	/* promotion costs money */
	if (lord->money < COST_SOLDIER)
		return;
	vassal->rank++;
	lord->money -= COST_SOLDIER;
}

uint16_t count_vassals(character_t *character)
{
	if (character == NULL)
		return 0;
	uint16_t counter = 0;
	character_t *current_vassal = world->characterlist;
	while (current_vassal != NULL) {
		if (current_vassal->lord != NULL
		    && current_vassal->lord == character)
			counter++;
		current_vassal = current_vassal->next;
	}
	return counter;
}

character_t *get_sovereign(character_t *character)
{
	character_t *sovereign = character;
	while (sovereign->lord != NULL)
		sovereign = sovereign->lord;
	return sovereign;
}

static void reverse_offer(unsigned char *status) {
	unsigned char new = 0;
	if (*status & OFFER_SENT_BIT) new |= OFFER_RECEIVED_BIT;
	else if (*status & OFFER_RECEIVED_BIT) new |= OFFER_SENT_BIT;
	*status = new;
}

void set_offer(character_t *from, character_t *to, const unsigned char offer_status) {
	if (!from || !to || from == to) return;
	dipstatus_t *dipstatus = get_dipstatus(from, to);
	if (!dipstatus) return; //why would this happen
	dipstatus->offer = offer_status;
	if (from != dipstatus->character1) reverse_offer(&(dipstatus->offer));
}

unsigned char get_offer(character_t *from, character_t *to) {
	if (!from || !to || from == to) return 0;
	dipstatus_t *dipstatus = get_dipstatus(from, to);
	unsigned char retval = dipstatus->offer;
	if (from != dipstatus->character1) reverse_offer(&retval);
	return retval;
}

void open_offer(character_t *from, character_t *to) {
	if (!from || !to || from == to) return;
	dipstatus_t *dipstatus = get_dipstatus(from, to);
	if (from == dipstatus->character1)
		dipstatus->offer |= OFFER_SENT_BIT;
	else
		dipstatus->offer |= OFFER_RECEIVED_BIT;
}

void close_offer(character_t *from, character_t *to, const unsigned char result) {
	if (!from || !to || from == to) return;
	dipstatus_t *dipstatus = get_dipstatus(from, to);
	dipstatus->offer = 0;
	if (result == ACCEPT) {
		if (dipstatus->status == WAR) dipstatus->status = NEUTRAL;
		else dipstatus->status = ALLIANCE;
	}
}

void sort_diplomacy_list() {
	if (world->diplomacylist == NULL || world->diplomacylist->next == NULL) return;
	int permutations;
	dipstatus_t *current, *next;
	do {
		current = world->diplomacylist;
		next = current->next;
		permutations = 0;
		while (next != NULL) {
			if (current->character1->id > next->character1->id || (current->character1 == next->character1 && current->character2->id > next->character2->id) ) {
				permutations++;
				if (current->prev != NULL) current->prev->next = next;
				else world->diplomacylist = next;
				if (next->next) next->next->prev = current;
				next->prev = current->prev;
				current->next = next->next;
				current->prev = next;
				next->next = current;
				next = current->next;
			}
			else {
				current = next;
				next = next->next;
			}
		}
	} while (permutations > 0);
}
