#include <stdlib.h>
#include <stdio.h>		/* for printf */
#include <string.h>
#include "piece.h"
#include "map.h"
#include "world.h"

char *const piece_name[] = {
	[NOBLE] = "noble",
	[SOLDIER] = "soldier",
	[SHIP] = "ship"
};

//uint16_t piece_id = 0;

//piece_t *piecelist = NULL;

piece_t *create_piecelist()
{
	/* single instance */
	if (world->piecelist != NULL)
		return world->piecelist;
	world->piecelist = calloc(sizeof(piece_t), 1);
	if (world->piecelist == NULL) exit(EXIT_FAILURE);
	return world->piecelist;
}

static void fill_piece_details(piece_t * piece, const enum piece_type type,
			       const uint16_t height, const uint16_t width,
			       character_t * owner)
{
	if (!piece) return;
	piece->id = world->next_piece_id;
	piece->type = type;
	piece->tile = world->grid->tiles[height][width];
	piece->owner = owner;
	world->grid->tiles[height][width]->piece = piece;
	world->next_piece_id++;
}

piece_t *add_piece(const enum piece_type type, const uint16_t height,
		   const uint16_t width, character_t * owner)
{
	if (owner == NULL)
		return NULL;	/* bad owner */
	if (world->grid == NULL)
		return NULL;	/* grid not created */
	if (world->grid->tiles[height][width]->walkable != 1)
		return NULL;	/* unwalkable tile */
	if (world->grid->tiles[height][width]->piece != NULL)
		return NULL;	/* tile occupied */

	/* only one noble allowed */
	if ((type == 0) && (get_noble_by_owner(owner) != NULL)) return NULL;

	piece_t *current = world->piecelist;
	if (!current) {
		world->piecelist = calloc(sizeof(piece_t), 1);
		if (!world->piecelist) exit(EXIT_FAILURE);
		fill_piece_details(world->piecelist, type, height, width, owner);
		return world->piecelist;
	}

	/*fast-forward to the end of list */
	while (current->next != NULL) {
		if (current->owner == owner && current->next->owner != owner) break;
		current = current->next;
	}

	/* now we can add a new variable */
	piece_t *next = current->next;
	current->next = calloc(sizeof(piece_t), 1);
	if (!current->next) exit(EXIT_FAILURE);
	fill_piece_details(current->next, type, height, width, owner);
	current->next->next = next;
	return current->next;
}

piece_t *get_noble_by_owner(character_t * owner)
{
	if (owner == NULL)
		return NULL;
	piece_t *current = world->piecelist;
	while (current != NULL) {
		if (current->owner->id == owner->id && current->type == NOBLE)
			return current;
		else
			current = current->next;
	}
	return NULL;
}

piece_t *next_piece(piece_t * start_piece)
{
	character_t *active_character = start_piece->owner;
	piece_t *current = start_piece;
	while (current != NULL) {
		if (current->next != NULL)
			current = current->next;
		else {
			current = world->piecelist;
//                      increment_gametime(); /* cycling around pieces of same character should not change time */
		}
		if (current->owner->id == active_character->id)
			return current;
	}
	return NULL;
}

void remove_piece(piece_t * piece)
{
	/* TODO if it was a noble, remove all character's soldiers, land ownership, diplomacy etc. AND the character himself */

	if (world->grid == NULL || piece == NULL || world->piecelist == NULL)
		return;

	piece_t *prev = NULL;
	piece_t *current = world->piecelist;
	while (current != NULL) {
		if (current == piece) {
			if (prev) prev->next = current->next;
			else world->piecelist = current->next;
			piece_t *tmp = current;
			tmp->tile->piece = NULL;
			free(tmp);
			return;
		}
		prev = current;
		current = current->next;
	}
}

void clear_piece_list()
{
	if (world->grid == NULL)
		return;
	while (world->piecelist != NULL)
		remove_piece(world->piecelist);
	world->next_piece_id = 0;
}

uint16_t count_pieces()
{
	uint16_t count = 0;
	piece_t *current = world->piecelist;
	while (current != NULL) {
		count++;
		current = current->next;
	}
	return count;
}

uint16_t count_pieces_by_owner(character_t * owner)
{
	uint16_t count = 0;
	piece_t *current = world->piecelist;
	while (current != NULL) {
		if (current->owner->id == owner->id && current->type == SOLDIER)
			count++;
		current = current->next;
	}
	return count;
}
