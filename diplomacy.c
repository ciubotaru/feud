#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "diplomacy.h"
#include "world.h"

char *const dipstatuslist[] = {
	"neutral",
	"alliance",
	"war"
};

dipstatus_t *create_diplomacylist() {
	/* single instance */
	if (world->diplomacylist != NULL) return world->diplomacylist;
	world->diplomacylist = malloc(sizeof(dipstatus_t));
	if (world->diplomacylist == NULL) return NULL;
	return world->diplomacylist;
}

dipstatus_t *set_diplomacy(player_t *player1, player_t *player2, const unsigned int status) {
	if (player1 == NULL || player2 == NULL) return NULL;

	/* only alliance between lord and vassal */
	if ((player1->lord != NULL && player1->lord->id == player2->id) || (player2->lord != NULL && player2->lord->id == player1->id))
		if (status != ALLIANCE) return NULL;

	/* if diplomacylist does not exist, create it */
	if (world->diplomacylist == NULL) {
//printf("Diplomacy list does not exist. Creating...\n");
		world->diplomacylist = create_diplomacylist();
		world->diplomacylist->player1 = player1;
		world->diplomacylist->player2 = player2;
		world->diplomacylist->status = status;
		world->diplomacylist->pending_offer = NULL;
		world->diplomacylist->next = NULL;
		return world->diplomacylist;
	}

	/* if dipstatus between two players exists, update it */
    dipstatus_t *current = world->diplomacylist;
    while (current != NULL) {
		if ((current->player1 == player1 && current->player2 == player2) || (current->player1 == player2 && current->player2 == player1)) {
			/* if new status is different, remove offers if any */
			if (current->status != status && current->pending_offer) {
				free(current->pending_offer);
				current->pending_offer = NULL;
			}
			current->status = status;
			return current;
		}
		if (current->next == NULL) break;
        current = current->next;
    }

	/* if dipstatus doesn't exist, append it */
    current->next = malloc(sizeof(dipstatus_t));
	if (!current->next) return NULL;
	current->next->player1 = player1;
	current->next->player2 = player2;
	current->next->status = status;
	current->next->pending_offer = NULL;
	current->next->next = NULL;
	return current;
}

dipstatus_t *get_dipstatus(player_t *player1, player_t *player2) {
	dipstatus_t *current = world->diplomacylist;
    while (current != NULL) {
		if ((current->player1->id == player1->id && current->player2->id == player2->id) || (current->player1->id == player2->id && current->player2->id == player1->id)) {
			return current;
		}
		if (current->next == NULL) break;
        current = current->next;
	}
	current = set_diplomacy(player1, player2, NEUTRAL);
	return current;
}

unsigned char get_diplomacy(player_t *player1, player_t *player2) {
/**
	dipstatus_t *current = world->diplomacylist;
    while (current != NULL) {
		if ((current->player1->id == player1->id && current->player2->id == player2->id) || (current->player1->id == player2->id && current->player2->id == player1->id)) {
			return current->status;
		}
		if (current->next == NULL) return 0;
        current = current->next;
	}
	return 0;
**/
	dipstatus_t *dipstatus = get_dipstatus(player1, player2);
	return dipstatus->status;
}

void remove_diplomacy(dipstatus_t *dipstatus) {
	if (dipstatus == NULL) return;

	dipstatus_t *current = world->diplomacylist;
	dipstatus_t *previous = NULL;
    while (current != NULL) {
		if (current == dipstatus) {
			/** if it's the first element in list, i.e. no previous, AND not the last, then set playerlist to next and free current **/
			/** if it's the last element AND not the first, then free current and set previous->next to NULL **/
			/** it there is previous and next, then set previous->next to current->next; free current **/
			/** if it's the ONLY element then free playerlist **/

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
					dipstatus_t *tmp = world->diplomacylist->next;
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

char *diplomacy_message(const int dipstatus) {
	char *message;
	switch (dipstatus) {
		case NEUTRAL:
			message = "neutral";
			break;
		case ALLIANCE:
			message = "alliance";
			break;
		case WAR:
			message = "war";
			break;
	}
	return message;
}

void print_diplomacy_list() {
    dipstatus_t * current = world->diplomacylist;
    while (current != NULL) {
        printf("Name1: %s. Name2: %s. Status: %s.\n", current->player1->name, current->player2->name, diplomacy_message(current->status));
        current = current->next;
    }
}

void homage(player_t *player, player_t *lord) {
	if (player == NULL || lord == NULL) return;
	/* can not switch lord */
	if (player->lord != NULL) return;
	/* can not be a lord of yourself */
	if (player->id == lord->id) return;
	/* can not pay homage to a baron */
	unsigned char player_rank = get_player_rank(player);
	unsigned char lord_rank = get_player_rank(lord);
//	piece_t *lord_noble = get_noble_by_owner(lord);
	if (lord_rank <= 1) return;
	/**
	 * if lord's rank is lower or same as vassal,
	 * then vassal's rank decreases,
	 * and all his high-rank vassals become free
	**/
	if (player_rank >= lord_rank) {
		/* demote the player */
		set_player_rank(player, lord_rank - 1);
		player_rank = lord_rank - 1;
		player_t *current_player = world->playerlist;
		while (current_player != NULL) {
			/* free vassals */
			if (current_player->lord != NULL && current_player->lord->id == player->id && get_player_rank(current_player) >= player_rank) current_player->lord = NULL;
			current_player = current_player->next;
		}
	}
	player->lord = lord;
	set_diplomacy(player, lord, ALLIANCE);
}

void promote_soldier(player_t *player, piece_t *piece, region_t *region, char *name) {
	if (player == NULL || piece == NULL || region == NULL) return;
	/* promotion costs money */
	if (player->money < COST_SOLDIER) return;
	/* can't promote somebody else's piece */
	if (piece->owner != player) return;
	/* can't give somebody else's region */
	if (region->owner != player) return;
	/* player must have at least two regions (one for new player) */
	if (count_regions_by_owner(player) < 2) return;
	player_t *vassal = add_player(name);
//	piece->rank = 1;
	set_player_rank(vassal, 1);
	piece->owner = vassal;
	change_region_owner(vassal, region);
	set_diplomacy(player, vassal, ALLIANCE);
	player->money -= COST_SOLDIER;
}

void promote_vassal(player_t *lord, player_t *vassal) {
	if (lord == NULL || vassal == NULL) return;
	/* vassal's new rank should be lower than lord's */
	if (vassal->rank + 1 <= lord->rank) return;
	/* promotion costs money */
	if (lord->money < COST_SOLDIER) return;
	vassal->rank++;
	lord->money -= COST_SOLDIER;
}

uint16_t count_vassals(player_t *player) {
	if (player == NULL) return 0;
	uint16_t counter = 0;
	player_t *current_vassal = world->playerlist;
	while (current_vassal != NULL) {
		if (current_vassal->lord != NULL && current_vassal->lord->id == player->id) counter++;
		current_vassal = current_vassal->next;
	}
	return counter;
}

player_t *get_sovereign(player_t *player) {
	player_t *sovereign = player;
	while (sovereign->lord != NULL) sovereign = sovereign->lord;
	return sovereign;
}

dipoffer_t *open_offer(player_t *from, player_t *to, const unsigned int offer) {
	dipstatus_t *dipstatus = get_dipstatus(from, to);
	/* if another offer is pending, reject it */
	unsigned char status = dipstatus->status;
	/* war->neutral or neutral->alliance */
	dipoffer_t *dipoffer = NULL;
	if ((status == WAR && offer == NEUTRAL) || (status == NEUTRAL && offer == ALLIANCE)) {
		dipoffer = malloc(sizeof(dipoffer));
		dipoffer->from = from;
		dipoffer->to = to;
		dipoffer->offer = offer;
		dipstatus->pending_offer = dipoffer;
	}
	return dipoffer;
}

void close_offer(dipoffer_t *offer, const unsigned int result) {
	if (offer == NULL || offer->from == NULL || offer->to == NULL) return;
	dipstatus_t *dipstatus = get_dipstatus(offer->from, offer->to);
	if (dipstatus == NULL) return;
	if (result == ACCEPT) {
		switch (dipstatus->status) {
			case WAR:
				if (offer->offer == NEUTRAL) dipstatus->status = NEUTRAL;
				break;
			case NEUTRAL:
				if (offer->offer == ALLIANCE) dipstatus->status = ALLIANCE;
				break;
		}
	}
	free(offer);
	dipstatus->pending_offer = NULL;
	return;
}
