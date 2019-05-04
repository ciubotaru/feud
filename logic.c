#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "world.h"

typedef struct {
	tile_t **moves;
	uint16_t nr_moves;
	uint16_t allocated_memory;
} available_moves_t;

available_moves_t *available_moves = NULL;

int16_t matrix[4][2] = {
	{1, 0},
	{0, 1},
	{-1, 0},
	{0, -1}
};

typedef struct {
	character_t *ai_sovereign;
	character_t *next_to_check;
} ai_diplomacy_t;

ai_diplomacy_t *ai_diplomacy = NULL;

int think_diplomacy(char *buffer) {
	if (!ai_diplomacy) {
		ai_diplomacy = malloc(sizeof(ai_diplomacy_t));
		if (!ai_diplomacy) return 2;
		ai_diplomacy->ai_sovereign = get_sovereign(world->selected_character);
		ai_diplomacy->next_to_check = world->characterlist;
	}
	dipstatus_t *current_dipstatus = NULL;
	dipstatus_t *sovereign_dipstatus = NULL;
	dipoffer_t *current_dipoffer = NULL;
	while (ai_diplomacy->next_to_check) {
		if (ai_diplomacy->next_to_check == world->selected_character) {
			ai_diplomacy->next_to_check = ai_diplomacy->next_to_check->next;
			continue;
		}
		current_dipstatus = get_dipstatus(ai_diplomacy->next_to_check, world->selected_character);
		/* check if we are in the same clan */
		if (get_sovereign(ai_diplomacy->next_to_check) == ai_diplomacy->ai_sovereign) {
			if (!current_dipstatus || current_dipstatus->status != ALLIANCE) {
				if (current_dipstatus) current_dipoffer = current_dipstatus->pending_offer;
				else current_dipoffer = NULL;
				/* if they sent us an alliance offer, accept it */
				if (current_dipoffer) {
					if (current_dipoffer->from == ai_diplomacy->next_to_check && current_dipoffer->offer == ALLIANCE) {
						close_offer(current_dipoffer, ACCEPT);
						sprintf(buffer, "accept alliance offer from %i\n", ai_diplomacy->next_to_check->id);
						ai_diplomacy->next_to_check = ai_diplomacy->next_to_check->next;
						return 0;
					}
					else {
						close_offer(current_dipoffer, REJECT);
						sprintf(buffer, "reject offer from %i\n", ai_diplomacy->next_to_check->id);
						return 0;
					}
				}
				/* offer alliance */
				else {
					current_dipoffer = open_offer(world->selected_character, ai_diplomacy->next_to_check, ALLIANCE);
					sprintf(buffer, "offer alliance to %i\n", ai_diplomacy->next_to_check->id);
					ai_diplomacy->next_to_check = ai_diplomacy->next_to_check->next;
					return 0;
				}
			}
		}
		else {
			/* MOST IMPORTANT -- diplomacy towards outsiders */
			if (world->selected_character == ai_diplomacy->ai_sovereign) {
				/* we are the overlord -- declare total war */
				if (current_dipstatus->status != WAR) {
					set_diplomacy(ai_diplomacy->next_to_check, world->selected_character, WAR);
					sprintf(buffer, "declare war to %i\n", ai_diplomacy->next_to_check->id);
					ai_diplomacy->next_to_check = ai_diplomacy->next_to_check->next;
					return 0;
				}
			}
			else {
				/* we are a vassal; copy overlord's diplomacy */
				sovereign_dipstatus = get_dipstatus(ai_diplomacy->next_to_check, ai_diplomacy->ai_sovereign);
				dipoffer_t *sovereign_dipoffer = sovereign_dipstatus->pending_offer;
				if (sovereign_dipstatus->status != current_dipstatus->status) {
					/* if our sovereign is at war and has not sent a peace offer, declare war */
					if (sovereign_dipstatus->status == WAR && (sovereign_dipoffer == NULL || sovereign_dipoffer->from != ai_diplomacy->ai_sovereign)) {
						sprintf(buffer, "declare war to %i\n", ai_diplomacy->next_to_check->id);
						set_diplomacy(ai_diplomacy->next_to_check, world->selected_character, WAR);
						ai_diplomacy->next_to_check = ai_diplomacy->next_to_check->next;
						return 0;
					}
					/* what if overlord negotiates peace, but opponent keeps attacking vassals? */
				}
			}
		}
		ai_diplomacy->next_to_check = ai_diplomacy->next_to_check->next;
	}
	return 1;
}

int think_move(char *buffer) {
	/* if rolled 6, always choose money. Stupid, but... */
	if (world->moves_left == 6) {
		world->moves_left = 0;
		world->selected_character->money++;
		strcpy(buffer, "take\n");
		return 0;
	}

	int i;
	uint16_t dst_h = 0;
	uint16_t dst_w = 0;

	/* move randomly */
	if (world->moves_left > 0) {
		if (!available_moves) {
			available_moves = malloc(sizeof(available_moves_t));
			if (!available_moves) exit(EXIT_FAILURE);
			available_moves->allocated_memory = 64;
			available_moves->moves = malloc(sizeof(tile_t *) * available_moves->allocated_memory * 2);
			if (!available_moves->moves) {
				free(available_moves);
				exit(EXIT_FAILURE);
			}
		}
		available_moves->nr_moves = 0;
		piece_t *current_piece = world->piecelist;
		while (current_piece) {
			if (current_piece->owner == world->selected_character) {
				if (available_moves->nr_moves + 4 > available_moves->allocated_memory) {
					available_moves->allocated_memory += 64;
					tile_t **tmp = realloc(available_moves->moves, sizeof(tile_t *) * available_moves->allocated_memory);
					if (!tmp) {
						if (available_moves->moves) free(available_moves->moves);
						exit(EXIT_FAILURE);
					}
					available_moves->moves = tmp;
				}
				for (i = 0; i < 4; i++) {
					if (current_piece->tile->height == 0 && matrix[i][0] < 0) continue;
					if (current_piece->tile->width == 0 && matrix[i][1] < 0) continue;
					dst_h = current_piece->tile->height + matrix[i][0];
					dst_w = current_piece->tile->width + matrix[i][1];
					if (is_legal_move(current_piece->tile->height, current_piece->tile->width, dst_h, dst_w)) {
						available_moves->moves[available_moves->nr_moves * 2] = current_piece->tile;
						available_moves->moves[available_moves->nr_moves * 2 + 1] = world->grid->tiles[dst_h][dst_w];
						available_moves->nr_moves++;
					}
				}
			}
			current_piece = current_piece->next;
		}
		if (available_moves->nr_moves > 0) {
			int random_nr = rand() % available_moves->nr_moves;
			sprintf(buffer, "piece move %i %i %i %i\n", available_moves->moves[random_nr * 2]->height, available_moves->moves[random_nr * 2]->width, available_moves->moves[random_nr * 2 + 1]->height, available_moves->moves[random_nr * 2 + 1]->width);
			move_piece(available_moves->moves[random_nr * 2]->piece, available_moves->moves[random_nr * 2 + 1]->height, available_moves->moves[random_nr * 2 + 1]->width);
			available_moves->nr_moves = 0;
			return 0;
		}
		else world->moves_left = 0;
	}
	return 1;
}

int think(char *buffer) {
	/**
	 * This function makes one move at a time,
	 * It returns 0 if further moves are possible, or 1 otherwise.
	**/
	if (!world || !world->selected_character) {
		return 2;
	}

	if (think_diplomacy(buffer) == 0) return 0;

	/* if have enough money and free tile, buy a soldier */
	if (get_money(world->selected_character) >= COST_SOLDIER) {
		/* find free tile */
		tile_t *empty_tile = get_empty_tile_in_region(get_noble_by_owner(world->selected_character)->tile->region);
		if (empty_tile) {
			add_piece(1, empty_tile->height, empty_tile->width, world->selected_character);
			set_money(world->selected_character, get_money(world->selected_character) - COST_SOLDIER);
			sprintf(buffer, "piece add %i 1 %i,%i\n", world->selected_character->id, empty_tile->height, empty_tile->width);
			return 0;
		}
	}
	
	if (think_move(buffer) == 0) return 0;

	free(ai_diplomacy);
	ai_diplomacy = NULL;
	strcpy(buffer, "done\n");
	return 1;
}
