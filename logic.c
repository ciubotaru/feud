#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "world.h"

int think(char *buffer) {
	/**
	 * This function makes one move at a time,
	 * It returns 0 if further moves are possible, or 1 otherwise.
	**/
	if (!world || !world->selected_character) {
		sprintf(buffer, "done\n");
		return 0;
	}
	/* if have enough money and free tile, buy a soldier */
	if (get_money(world->selected_character) >= COST_SOLDIER) {
		/* find free tile */
		int i, j;
		for (i = 0; i < world->grid->height; i++) {
			for (j = 0; j < world->grid->width; j++) {
				if (world->grid->tiles[i][j]->region != NULL && world->grid->tiles[i][j]->region->owner == world->selected_character && world->grid->tiles[i][j]->piece == NULL) {
					add_piece(1, i, j, world->selected_character);
					set_money(world->selected_character, get_money(world->selected_character) - COST_SOLDIER);
					sprintf(buffer, "piece add %i 1 %i,%i\n", world->selected_character->id, i, j);
					return 0;
				}
			}
		}
	}

	/* if rolled 6, always choose money. Stupid, but... */
	if (world->moves_left == 6) {
//		dprintf(STDOUT_FILENO, "done\n");
		world->moves_left = 0;
		world->selected_character->money++;
		strcpy(buffer, "take\n");
		return 0;
	}
	/* move randomly */
	if (world->moves_left > 0) {
/**
		float score = evaluate();
		printf("Evaluation: %f\n", score);
**/
		uint16_t nr_ai_pieces = count_pieces_by_owner(world->selected_character) + 1;
		piece_t *current_piece = world->piecelist;
		int random_nr;
		if (nr_ai_pieces > 1) random_nr = rand() % nr_ai_pieces + 1;
		else random_nr = 1;
		while (current_piece->next != NULL) {
			if (current_piece->owner == world->selected_character) {
				random_nr--;
				if (random_nr == 0) break;
			}
			current_piece = current_piece->next;
		}
		uint16_t directions_mask = 0;
		if (is_legal_move(current_piece->tile->height, current_piece->tile->width, current_piece->tile->height + 1, current_piece->tile->width)) directions_mask |= 1;
		if (is_legal_move(current_piece->tile->height, current_piece->tile->width, current_piece->tile->height, current_piece->tile->width + 1)) directions_mask |= (1 << 1);
		if (is_legal_move(current_piece->tile->height, current_piece->tile->width, current_piece->tile->height - 1, current_piece->tile->width)) directions_mask |= (1 << 2);
		if (is_legal_move(current_piece->tile->height, current_piece->tile->width, current_piece->tile->height, current_piece->tile->width - 1)) directions_mask |= (1 << 3);
		uint16_t nr_directions = __builtin_popcount (directions_mask);
		/* if piece is blocked, skip and choose another one (WHAT IF ALL PIECES are blocked???) */
//		if (nr_directions == 0) continue;
		random_nr = rand() % nr_directions + 1;
		int bit = 0;
		while (random_nr > 0) {
			if ((directions_mask >> bit) & 1) {
				random_nr--;
			}
			bit++;
		}
		switch (bit) {
			case 1:
				sprintf(buffer, "piece move %i %i %i %i\n", current_piece->tile->height, current_piece->tile->width, current_piece->tile->height + 1, current_piece->tile->width);
				move_piece(current_piece, current_piece->tile->height + 1, current_piece->tile->width);
				break;
			case 2:
				sprintf(buffer, "piece move %i %i %i %i\n", current_piece->tile->height, current_piece->tile->width, current_piece->tile->height, current_piece->tile->width + 1);
				move_piece(current_piece, current_piece->tile->height, current_piece->tile->width + 1);
				break;
			case 3:
				sprintf(buffer, "piece move %i %i %i %i\n", current_piece->tile->height, current_piece->tile->width, current_piece->tile->height - 1, current_piece->tile->width);
				move_piece(current_piece, current_piece->tile->height - 1, current_piece->tile->width);
				break;
			case 4:
				sprintf(buffer, "piece move %i %i %i %i\n", current_piece->tile->height, current_piece->tile->width, current_piece->tile->height, current_piece->tile->width - 1);
				move_piece(current_piece, current_piece->tile->height, current_piece->tile->width - 1);
				break;
		}
		return 0;
	}
	strcpy(buffer, "done\n");
	return 1;
}
