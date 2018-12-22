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

static void fill_diplomacy_details(dipstatus_t *diplomacy, character_t *character1,
			   character_t *character2, const unsigned int status)
{
	if (!diplomacy || !character1 || !character2) return;
	diplomacy->character1 = character1;
	diplomacy->character2 = character2;
	diplomacy->status = status;
	diplomacy->pending_offer = NULL;
	diplomacy->prev = NULL;
	diplomacy->next = NULL;
}

dipstatus_t *set_diplomacy(character_t *character1, character_t *character2,
			   const unsigned int status)
{
	if (character1 == NULL || character2 == NULL) return NULL;

	/* if diplomacylist does not exist, create it */
	if (world->diplomacylist == NULL) {
//printf("Diplomacy list does not exist. Creating...\n");
		world->diplomacylist = create_diplomacylist();
		fill_diplomacy_details(world->diplomacylist, character1, character2, status);
		return world->diplomacylist;
	}

	dipstatus_t *current = get_dipstatus(character1, character2);
	/* if dipstatus between two characters exists, update it */
	if (current) {
		current->status = status;
		current->pending_offer = NULL;
		return current;
	}
	/* if no dipstatus, prepend to diplomacylist */
	else {
		current = malloc(sizeof(dipstatus_t));
		if (!current) return NULL;
		fill_diplomacy_details(current, character1, character2, status);
		world->diplomacylist->prev = current;
		current->next = world->diplomacylist;
		world->diplomacylist = current;
		return current;
	}
}

dipstatus_t *get_dipstatus(character_t *character1, character_t *character2)
{
	dipstatus_t *current = world->diplomacylist;
	while (current != NULL) {
		if ((current->character1 == character1
		     && current->character2 == character2)
		    || (current->character1 == character2
			&& current->character2 == character1)) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}

unsigned char get_diplomacy(character_t *character1, character_t *character2)
{
	dipstatus_t *dipstatus = get_dipstatus(character1, character2);
	if (dipstatus) return dipstatus->status;
	else return NEUTRAL;
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
	/* if dipstatus does not exist, set it to default */
	if (!dipstatus) {
		dipstatus = set_diplomacy(from, to, NEUTRAL);
		if (!dipstatus) return NULL;
	}
	else {
		/* if another offer is pending, reject it */
		if (dipstatus->pending_offer)
			close_offer(dipstatus->pending_offer, REJECT);
	}
	unsigned char status = dipstatus->status;
	/* war->neutral or neutral->alliance */
	dipoffer_t *dipoffer = NULL;
	if ((status == WAR && offer == NEUTRAL)
	    || (status == NEUTRAL && offer == ALLIANCE)) {
		dipoffer = malloc(sizeof(dipoffer_t));
		if (!dipoffer) return NULL;
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
	if (dipstatus->status == NEUTRAL) remove_diplomacy(dipstatus);
	return;
}
