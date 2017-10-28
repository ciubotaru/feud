#include <stdlib.h>
#include <stdio.h>		/* for printf */
#include <string.h>
#include "piece.h"
#include "map.h"
#include "world.h"

char *const unit_type_list[] = {
	"noble",
	"soldier",
//      "ship",
};

//uint16_t piece_id = 0;

//piece_t *piecelist = NULL;

piece_t *create_piecelist()
{
	/* single instance */
	if (world->piecelist != NULL)
		return world->piecelist;
	world->piecelist = calloc(sizeof(piece_t), 1);
	if (world->piecelist == NULL)
		return NULL;
	return world->piecelist;
}

static void fill_piece_details(piece_t * piece, const unsigned char type,
			       const uint16_t height, const uint16_t width,
			       player_t * owner)
{
	piece->id = world->next_piece_id;
	piece->type = type;
	piece->width = width;
	piece->height = height;
	piece->owner = owner;
	world->grid->tiles[height][width]->piece = piece;
//      grid->tiles[height][width]->region->owner->id = owner->id;
	world->next_piece_id++;
}

piece_t *add_piece(const unsigned char type, const uint16_t height,
		   const uint16_t width, player_t * owner)
{
	if (owner == NULL)
		return NULL;	/* bad owner */
	if (world->grid == NULL)
		return NULL;	/* grid not created */
	if (world->grid->tiles[height][width]->walkable != 1)
		return NULL;	/* unwalkable tile */
	if (world->grid->tiles[height][width]->piece != NULL)
		return NULL;	/* tile occupied */

	if (world->piecelist == NULL) {
		world->piecelist = create_piecelist();
		fill_piece_details(world->piecelist, type, height, width,
				   owner);
		return world->piecelist;
	}
	piece_t *current = world->piecelist;

	/*fast-forward to the end of list */
	while (current->next != NULL) {
		current = current->next;
	}

	/* now we can add a new variable */
	current->next = calloc(sizeof(piece_t), 1);
	if (!current->next)
		return NULL;
	fill_piece_details(current->next, type, height, width, owner);
	return current->next;
}

piece_t *get_noble_by_owner(player_t * owner)
{
	if (owner == NULL)
		return NULL;
	piece_t *current = world->piecelist;
	while (current != NULL) {
		if (current->owner->id == owner->id && current->type == 0)
			return current;
		else
			current = current->next;
	}
	return NULL;
}

piece_t *next_piece(piece_t * start_piece)
{
	player_t *active_player = start_piece->owner;
	piece_t *current = start_piece;
	while (current != NULL) {
		if (current->next != NULL)
			current = current->next;
		else {
			current = world->piecelist;
//                      increment_gametime(); /* cycling around pieces of same player should not change time */
		}
		if (current->owner->id == active_player->id)
			return current;
	}
	return NULL;
}

void remove_piece(piece_t * piece)
{
	/* TODO if it was a noble, remove all player's soldiers, land ownership, diplomacy etc. AND the player himself */

	if (world->grid == NULL || piece == NULL || world->piecelist == NULL)
		return;

	piece_t *current = world->piecelist;
	piece_t *previous = NULL;

	while (current != NULL) {
		if (current == piece) {
			/** if it's the first element in list, i.e. no previous, AND not the last, then set playerlist to next and free current **/
			/** if it's the last element AND not the first, then free current and set previous->next to NULL **/
			/** it there is previous and next, then set previous->next to current->next; free current **/
			/** if it's the ONLY element then free playerlist **/

			/** check if first **/
			if (previous == NULL) {
				/** ... AND last **/
				if (current->next == NULL) {
					world->grid->tiles[current->
							   height][current->
								   width]->
					    piece = NULL;
					free(current);
					world->piecelist = NULL;
					return;
				}
				/** first, but not last **/
				else {
					piece_t *tmp = world->piecelist->next;
					world->grid->tiles[world->piecelist->
							   height][world->
								   piecelist->
								   width]->
					    piece = NULL;
					free(world->piecelist);
					world->piecelist = tmp;
					return;
				}
			}
			/** not first **/
			else {
				/** check if last **/
				if (current->next == NULL) {
					world->grid->tiles[current->
							   height][current->
								   width]->
					    piece = NULL;
					free(current);
					previous->next = NULL;
					return;
				}
				/** not first, not last **/
				else {
					previous->next = current->next;
					world->grid->tiles[current->
							   height][current->
								   width]->
					    piece = NULL;
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

void clear_piece_list()
{
	if (world->grid == NULL)
		return;
	while (world->piecelist != NULL)
		remove_piece(world->piecelist);
	world->next_piece_id = 0;
	world->piecelist = NULL;
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

uint16_t count_pieces_by_owner(player_t * owner)
{
	uint16_t count = 0;
	piece_t *current = world->piecelist;
	while (current != NULL) {
		if (current->owner->id == owner->id && current->type == 1)
			count++;
		current = current->next;
	}
	return count;
}

void update_army_ranking()
{
	player_t *player = NULL;
	player_t *player2 = NULL;

	/* reset ranks to 1 */
	player = world->playerlist;
	while (player != NULL) {
		player->rank_army = 1;
		player = player->next;
	}
	/* rewind to start */
	player = world->playerlist;

	while (player != NULL) {
		player2 = world->playerlist;
		while (player2->id != player->id) {
			if (count_pieces_by_owner(player) <=
			    count_pieces_by_owner(player2))
				player->rank_army++;
			else
				player2->rank_army++;
			player2 = player2->next;
		}
		player = player->next;
	}
}
