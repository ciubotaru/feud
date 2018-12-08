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
	if (world->diplomacylist == NULL)
		return NULL;
	return world->diplomacylist;
}

dipstatus_t *set_diplomacy(character_t *character1, character_t *character2,
			   const unsigned int status)
{
	if (character1 == NULL || character2 == NULL)
		return NULL;

	/* only alliance between lord and vassal */
	if ((character1->lord != NULL && character1->lord->id == character2->id)
	    || (character2->lord != NULL && character2->lord->id == character1->id))
		if (status != ALLIANCE)
			return NULL;

	/* if diplomacylist does not exist, create it */
	if (world->diplomacylist == NULL) {
//printf("Diplomacy list does not exist. Creating...\n");
		world->diplomacylist = create_diplomacylist();
		world->diplomacylist->character1 = character1;
		world->diplomacylist->character2 = character2;
		world->diplomacylist->status = status;
		world->diplomacylist->pending_offer = NULL;
		world->diplomacylist->next = NULL;
		return world->diplomacylist;
	}

	/* if dipstatus between two characters exists, update it */
	dipstatus_t *current = world->diplomacylist;
	while (current != NULL) {
		if ((current->character1 == character1 && current->character2 == character2)
		    || (current->character1 == character2
			&& current->character2 == character1)) {
			/* if new status is different, remove offers if any */
			if (current->status != status && current->pending_offer) {
				free(current->pending_offer);
				current->pending_offer = NULL;
			}
			current->status = status;
			return current;
		}
		if (current->next == NULL)
			break;
		current = current->next;
	}

	/* if dipstatus doesn't exist, append it */
	current->next = malloc(sizeof(dipstatus_t));
	if (!current->next)
		return NULL;
	current->next->character1 = character1;
	current->next->character2 = character2;
	current->next->status = status;
	current->next->pending_offer = NULL;
	current->next->next = NULL;
	return current->next;
}

dipstatus_t *get_dipstatus(character_t *character1, character_t *character2)
{
	dipstatus_t *current = world->diplomacylist;
	while (current != NULL) {
		if ((current->character1->id == character1->id
		     && current->character2->id == character2->id)
		    || (current->character1->id == character2->id
			&& current->character2->id == character1->id)) {
			return current;
		}
		current = current->next;
	}
	/* if nothing was found, create dipstatus */
	if ((character1->lord != NULL && character1->lord->id == character2->id)
	    || (character2->lord != NULL && character2->lord->id == character1->id))
		current = set_diplomacy(character1, character2, ALLIANCE);
	else current = set_diplomacy(character1, character2, NEUTRAL);
	return current;
}

unsigned char get_diplomacy(character_t *character1, character_t *character2)
{
	dipstatus_t *dipstatus = get_dipstatus(character1, character2);
	return dipstatus->status;
}

void remove_diplomacy(dipstatus_t *dipstatus)
{
	if (dipstatus == NULL)
		return;

	dipstatus_t *current = world->diplomacylist;
	dipstatus_t *previous = NULL;
	while (current != NULL) {
		if (current == dipstatus) {
			/** if it's the first element in list, i.e. no previous, AND not the last, then set characterlist to next and free current **/
			/** if it's the last element AND not the first, then free current and set previous->next to NULL **/
			/** it there is previous and next, then set previous->next to current->next; free current **/
			/** if it's the ONLY element then free characterlist **/

			/** check if first **/
			if (previous == NULL) {
				/** ... AND last **/
				if (current->next == NULL) {
					free(current);
					world->diplomacylist = NULL;
					return;
				}
				/** first, but not last **/
				else {
					dipstatus_t *tmp =
					    world->diplomacylist->next;
					free(world->diplomacylist);
					world->diplomacylist = tmp;
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

void remove_redundant_diplomacy() {
	dipstatus_t *current = world->diplomacylist;
	dipstatus_t *next = current;
	while (current) {
		next = current->next;
		if (current->character1 == current->character2 ||
			(current->status == NEUTRAL && current->pending_offer == NULL) ||
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
	if (character->id == lord->id)
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
			    && current_character->lord->id == character->id
			    && get_character_rank(current_character) >= character_rank)
				current_character->lord = NULL;
			current_character = current_character->next;
		}
	}
	character->lord = lord;
	set_diplomacy(character, lord, ALLIANCE);
}

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
	character_t *vassal = add_character(name);
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
		    && current_vassal->lord->id == character->id)
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

dipoffer_t *open_offer(character_t *from, character_t *to, const unsigned int offer)
{
	dipstatus_t *dipstatus = get_dipstatus(from, to);
	/* if another offer is pending, reject it */
	unsigned char status = dipstatus->status;
	/* war->neutral or neutral->alliance */
	dipoffer_t *dipoffer = NULL;
	if ((status == WAR && offer == NEUTRAL)
	    || (status == NEUTRAL && offer == ALLIANCE)) {
		dipoffer = malloc(sizeof(dipoffer_t));
		dipoffer->from = from;
		dipoffer->to = to;
		dipoffer->offer = offer;
		dipstatus->pending_offer = dipoffer;
	}
	return dipoffer;
}

void close_offer(dipoffer_t *offer, const unsigned int result)
{
	if (offer == NULL || offer->from == NULL || offer->to == NULL)
		return;
	dipstatus_t *dipstatus = get_dipstatus(offer->from, offer->to);
	if (dipstatus == NULL) {
		free(offer);
		return;
	}
	if (result == ACCEPT) {
		switch (dipstatus->status) {
		case WAR:
			if (offer->offer == NEUTRAL)
				dipstatus->status = NEUTRAL;
			break;
		case NEUTRAL:
			if (offer->offer == ALLIANCE)
				dipstatus->status = ALLIANCE;
			break;
		}
	}
	free(offer);
	dipstatus->pending_offer = NULL;
	return;
}
