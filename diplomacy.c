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

inline static dipstatus_t *find_dipstatus(character_t *character1, character_t *character2) {
	if (!character1 || !character2) return NULL;

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
	dipstatus_t *new = malloc(sizeof(dipstatus_t));
	if (!new) exit(EXIT_FAILURE);

	character_t *first, *second;
	if (character1->id < character2->id) {
		first = character1;
		second = character2;
	}
	else {
		first = character2;
		second = character1;
	}
	new->character1 = first;
	new->character2 = second;
	new->status = status;

	dipstatus_t *current = world->diplomacylist;
	dipstatus_t *prev = NULL;
	while (current) {
		if (current->character1->id > new->character1->id) break;
		if (current->character1 == new->character1 && current->character2->id > new->character2->id) break;
		prev = current;
		current = current->next;
	}
	if (!prev) {
		world->diplomacylist = new;
	}
	else {
		prev->next = new;
	}
	new->next = current;
	return new;
}

dipstatus_t *set_diplomacy(character_t *character1, character_t *character2,
			   const unsigned int status)
{
	dipstatus_t *current = find_dipstatus(character1, character2);
	if (current) {
		current->status &= ~DIPLOMACY_MASK;
		current->status |= status;
	}
	else current = add_dipstatus(character1, character2, status);
	return current;
}

dipstatus_t *get_dipstatus(character_t *character1, character_t *character2)
{
	dipstatus_t *current = find_dipstatus(character1, character2);
	if (current) return current;
	/* if nothing found, create neutral diplomacy and return it */
	return add_dipstatus(character1, character2, NEUTRAL);
}

unsigned char get_diplomacy(character_t *character1, character_t *character2)
{
	dipstatus_t *dipstatus = get_dipstatus(character1, character2);
	return dipstatus->status & DIPLOMACY_MASK;
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
			(current->status == 0) ||
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
	if (*status & OFFER_SENT) new = OFFER_RECEIVED;
	else if (*status & OFFER_RECEIVED) new = OFFER_SENT;
	*status &= ~OFFER_MASK;
	*status |= new;
}

void set_offer(character_t *from, character_t *to, const unsigned char offer_status) {
	if (!from || !to || from == to) return;
	dipstatus_t *dipstatus = get_dipstatus(from, to);
	dipstatus->status &= ~OFFER_MASK;
	dipstatus->status |= offer_status << 2;
	if (from != dipstatus->character1) reverse_offer(&(dipstatus->status));
}

unsigned char get_offer(character_t *from, character_t *to) {
	if (!from || !to || from == to) return 0;
	dipstatus_t *dipstatus = get_dipstatus(from, to);
	unsigned char retval = dipstatus->status & OFFER_MASK;
	if (from != dipstatus->character1) reverse_offer(&retval);
	return retval;
}

void open_offer(character_t *from, character_t *to) {
	if (!from || !to || from == to) return;
	dipstatus_t *dipstatus = get_dipstatus(from, to);
	if (from == dipstatus->character1)
		dipstatus->status |= OFFER_SENT;
	else
		dipstatus->status |= OFFER_RECEIVED;
}

void close_offer(character_t *from, character_t *to, const unsigned char result) {
	if (!from || !to || from == to) return;
	dipstatus_t *dipstatus = get_dipstatus(from, to);
	dipstatus->status &= ~OFFER_MASK;
	if (result == ACCEPT) {
		switch (dipstatus->status & DIPLOMACY_MASK) {
			case WAR:
				dipstatus->status &= ~DIPLOMACY_MASK;
				dipstatus->status |= NEUTRAL;
				break;
			case NEUTRAL:
				dipstatus->status &= ~DIPLOMACY_MASK;
				dipstatus->status |= ALLIANCE;
				break;
			case ALLIANCE:
				//this should not happen
				break;
		}
	}
}

character_t *get_successor(character_t *character) {
	if (!character) return NULL;
	dipstatus_t *current = world->diplomacylist;
	character_t *successor = NULL;
	while (current) {
		if ((current->character1 == character) && (current->status & HEIR)) {
			successor = current->character2;
			break;
		}
		if ((current->character2 == character) && (current->status & GRANTOR)) {
			successor = current->character1;
			break;
		}
		current = current->next;
	}
	return successor;
}

void set_successor(character_t *grantor, character_t *heir) {
	if (!grantor) return;
	if (grantor == heir) return;
	/* clear old successor */
	dipstatus_t *dipstatus = world->diplomacylist;
	while (dipstatus) {
		if (dipstatus->character1 == grantor && dipstatus->status & HEIR) {
			dipstatus->status &= ~HEIR;
			break;
		}
		if (dipstatus->character2 == grantor && dipstatus->status & GRANTOR) {
			dipstatus->status &= ~ GRANTOR;
			break;
		}
		dipstatus = dipstatus->next;
	}
	dipstatus = get_dipstatus(grantor, heir);
	if (dipstatus->character1 == grantor)
		dipstatus->status |= HEIR;
	else
		dipstatus->status |= GRANTOR;
}
