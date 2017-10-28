#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map.h"
#include "piece.h"
#include "diplomacy.h"
#include "window.h"
#include "world.h"

tile_t *tile_init()
{
	tile_t *instance = malloc(sizeof(tile_t));
	instance->walkable = 1;
	instance->region = NULL;
	instance->piece = NULL;
	return instance;
}

region_t *regionlist = NULL;

region_t *create_regionlist()
{
	/* single instance */
	if (world->regionlist != NULL)
		return world->regionlist;
	world->regionlist = malloc(sizeof(region_t));
	if (world->regionlist == NULL)
		return NULL;
	return world->regionlist;
}

void fill_region_details(region_t * region, const char *name)
{
	region->id = world->next_region_id;
	world->next_region_id++;
	region->size = 0;	/* start with one tile */
	strcpy(region->name, name);
	region->owner = NULL;
	region->tiles = NULL;
	region->next = NULL;
}

region_t *add_region(const char *name)
{
	if (world->regionlist == NULL) {
		world->regionlist = create_regionlist();
		fill_region_details(world->regionlist, name);
		return world->regionlist;
	}

	region_t *current = world->regionlist;

	/*fast-forward to the end of list */
	while (current->next != NULL) {
		current = current->next;
	}

	/* now we can add a new variable */
	current->next = malloc(sizeof(region_t));
	if (!current->next)
		return NULL;
	fill_region_details(current->next, name);
	return current->next;
}

void change_tile_region(region_t * new_region, tile_t * tile)
{
	/* same region */
	if (new_region != NULL && tile->region != NULL
	    && tile->region->id == new_region->id) {
		return;
	}
	/* same NO region */
	if (new_region == NULL && tile->region == NULL) {
		return;
	}
	int i, j;
	/* if tile is not assigned AND new region is real */
	if (tile->region == NULL && new_region != NULL) {
		tile->region = new_region;
		new_region->size++;
		new_region->tiles =
		    realloc(new_region->tiles,
			    (new_region->size) * sizeof(tile_t *));
		new_region->tiles[new_region->size - 1] = tile;
		return;
	}
	/* reassign tile from one region to another */
	else if (tile->region != NULL && new_region != NULL) {
		tile_t **newlist =
		    malloc(sizeof(tile_t *) * (tile->region->size - 1));
		j = 0;
		for (i = 0; i < tile->region->size; i++) {
			if ((tile->region->tiles)[i] != tile) {
				newlist[j] = (tile->region->tiles)[i];
				j++;
			}
		}
		tile->region->size--;
		new_region->tiles =
		    realloc(new_region->tiles,
			    (new_region->size + 1) * sizeof(tile_t *));
		new_region->size++;
		new_region->tiles[new_region->size - 1] = tile;
		free(tile->region->tiles);
		(tile->region->tiles) = newlist;
		tile->region = new_region;
		return;
	}
	/* remove tile from region */
	else if (new_region == NULL) {
		tile_t **newlist =
		    malloc(sizeof(tile_t *) * (tile->region->size - 1));
		j = 0;
		for (i = 0; i < tile->region->size; i++) {
			if (tile->region->tiles[i] != tile) {
				newlist[j] = (tile->region->tiles)[i];
				j++;
			}
		}
		tile->region->size--;
		if (tile->region->tiles != NULL)
			free(tile->region->tiles);
		tile->region->tiles = newlist;
		tile->region = NULL;
		return;
	}
}

void change_region_owner(player_t * new_player, region_t * region)
{
	/* same owner */
	if (region->owner != NULL && new_player != NULL
	    && region->owner->id == new_player->id) {
		return;
	}
	/* if region is not assigned AND new owner is real */
	if (region->owner == NULL && new_player != NULL) {
		region->owner = new_player;
	}
	/* reassign region from one player to another */
	else if (region->owner != NULL && new_player != NULL) {
		region->owner = new_player;
	}
	/* remove tile from region */
	else if (new_player == NULL) {
		region->owner = NULL;
	}
}

void change_region_name(char *new_name, region_t * region)
{
	if (region == NULL)
		return;
	region_t *current = world->regionlist;
	while (current != NULL) {
		if (strcmp(region->name, new_name) == 0)
			return;	/* non-unique name */
		current = current->next;
	}
	strcpy(region->name, new_name);
}

unsigned char claim_region(player_t * player, region_t * region)
{
	/**
	 * return value:
	 * 0 -- not claimed
	 * 1 -- claimed from nature
	 * 2 -- conquered from enemy
	**/
	if (player == NULL || region == NULL)
		return 0;

	player_t *current_owner = region->owner;

	/* if the region is free, take it and stop here */
	if (current_owner == NULL) {
		change_region_owner(player, region);
		return 1;
	}

	/* if region is already ours, stop here */
	if (current_owner == player)
		return 0;

	/* if current owner is in the region, stop here */
	piece_t *owners_noble = get_noble_by_owner(current_owner);
	region_t *present_location =
	    world->grid->tiles[owners_noble->height][owners_noble->width]->
	    region;
	if (region == present_location)
		return 0;

	/* if we are at war with current owner, take it */
	if (get_diplomacy(current_owner, player) == WAR) {
		change_region_owner(player, region);
		/* if previous owner has no other regions, he lost */
		world->check_death = 1;
		return 2;
	}
	return 0;
}

region_t *get_region_by_id(const uint16_t id)
{
	region_t *current = world->regionlist;
	while (current != NULL) {
		if (id == current->id) {
			return current;
		}
		if (current->next == NULL)
			return NULL;
		current = current->next;
	}
	return NULL;
}

region_t *get_region_by_name(const char *name)
{
	region_t *current = world->regionlist;
	while (current != NULL) {
		if (strcmp(current->name, name) == 0) {
			return current;
		}
		if (current->next == NULL)
			return NULL;
		current = current->next;
	}
	return NULL;
}

uint16_t get_region_order(region_t * region)
{
	if (region == NULL)
		return 0;
	uint16_t counter = 0;
	region_t *current = world->regionlist;
	while (current != NULL && current->id != region->id) {
		counter++;
		current = current->next;
	}
	return counter;
}

region_t *get_region_by_order(uint16_t order)
{
	uint16_t counter = order;
	region_t *current = world->regionlist;
	while (current->next != NULL && counter > 0) {
		counter--;
		current = current->next;
	}
	return current;
}

void clear_region(region_t * region)
{
	if (region == NULL || region->size == 0)
		return;
	while (region->size > 0)
		change_tile_region(NULL, (region->tiles)[0]);
	free(region->tiles);
}

void remove_region(region_t * region)
{
	if (world->regionlist == NULL)
		return;
	region_t *current = world->regionlist;
	region_t *previous = NULL;

	while (current != NULL) {
		if (current == region) {
			/** if it's the first element in list, i.e. no previous, AND not the last, then set playerlist to next and free current **/
			/** if it's the last element AND not the first, then free current and set previous->next to NULL **/
			/** it there is previous and next, then set previous->next to current->next; free current **/
			/** if it's the ONLY element then free playerlist **/

			/** check if first **/
			if (previous == NULL) {
				/** ... AND last **/
				if (current->next == NULL) {
					clear_region(current);
					free(current);
					world->regionlist = NULL;
					return;
				}
				/** first, but not last **/
				else {
					region_t *tmp = current->next;
					clear_region(current);
					free(current);
					world->regionlist = tmp;
					return;
				}
			}
			/** not first **/
			else {
				/** check if last **/
				if (current->next == NULL) {
					clear_region(current);
					free(current);
					previous->next = NULL;
					return;
				}
				/** not first, not last **/
				else {
					previous->next = current->next;
					clear_region(current);
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

void clear_region_list()
{
	if (world->regionlist == NULL)
		return;
	while (world->regionlist != NULL)
		remove_region(world->regionlist);
	world->next_region_id = 1;
	world->regionlist = NULL;
}

uint16_t count_tiles_by_owner(player_t * owner)
{
	uint16_t count = 0;
	region_t *current = world->regionlist;
	while (current != NULL) {
		if (current->owner != NULL && current->owner->id == owner->id)
			count += current->size;
		current = current->next;
	}
	return count;
}

uint16_t count_regions_by_owner(player_t * owner)
{
	uint16_t count = 0;
	region_t *current = world->regionlist;
	while (current != NULL) {
		if (current->owner != NULL && current->owner->id == owner->id)
			count++;
		current = current->next;
	}
	return count;
}

int count_regions()
{
	int counter = 0;
	region_t *current = world->regionlist;
	while (current != NULL) {
		counter++;
		current = current->next;
	}
	return counter;
}

void update_land_ranking()
{
	player_t *player = NULL;
	player_t *player2 = NULL;

	/* reset ranks to 1 */
	player = world->playerlist;
	while (player != NULL) {
		player->rank_land = 1;
		player = player->next;
	}
	/* rewind to start */
	player = world->playerlist;

	while (player != NULL) {
		player2 = world->playerlist;
		while (player2->id != player->id) {
			if (count_tiles_by_owner(player) <=
			    count_tiles_by_owner(player2))
				player->rank_land++;
			else
				player2->rank_land++;
			player2 = player2->next;
		}
		player = player->next;
	}
}

grid_t *create_grid(const uint16_t height, const uint16_t width)
{
	if (world->grid != NULL)
		return world->grid;
	int i, j;
	grid_t *grid = malloc(sizeof(grid_t));
	grid->height = height;
	grid->width = width;
	grid->cursor_height = 0;
	grid->cursor_width = 0;
	grid->tiles = malloc(height * sizeof(void *));
	for (i = 0; i < height; i++) {
		grid->tiles[i] = malloc(width * sizeof(void *));
		for (j = 0; j < width; j++) {
			grid->tiles[i][j] = tile_init();
		}
	}
	world->grid = grid;
	return world->grid;
}

void remove_grid()
{
	if (world->grid == NULL)
		return;
	int i, j;
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
			if (world->grid->tiles[i][j]->piece != NULL)
				remove_piece(world->grid->tiles[i][j]->piece);
			free(world->grid->tiles[i][j]);
		}
		free(world->grid->tiles[i]);
	}
	free(world->grid->tiles);
	free(world->grid);
	world->grid = NULL;
}

unsigned int is_legal_move(const uint16_t src_height, const uint16_t src_width,
			   const uint16_t dst_height, const uint16_t dst_width)
{
	/* at least one move left */
	if (world->moves_left == 0)
		return 0;
	/* destination is within map limits */
	if (dst_height >= world->grid->height
	    || dst_width >= world->grid->width)
		return 0;
	/* source tile and destination tile are one move away */
	int16_t diff_height = (int16_t) src_height - (int16_t) dst_height;
	int16_t diff_width = (int16_t) src_width - (int16_t) dst_width;
	if (diff_height * diff_height + diff_width * diff_width != 1)
		return 0;
	/* destination is walkable */
	if (world->grid->tiles[dst_height][dst_width]->walkable == 0)
		return 0;
	/* there is a piece on source tile and piece belongs to current player */
	player_t *current_player = get_player_by_id(world->selected_player);
	piece_t *src_piece = world->grid->tiles[src_height][src_width]->piece;
	if (src_piece == NULL || src_piece->owner->id != current_player->id)
		return 0;
	piece_t *dst_piece = world->grid->tiles[dst_height][dst_width]->piece;
	if (dst_piece != NULL) {
		/* no own piece on destination tile */
		if (dst_piece->owner->id == current_player->id)
			return 0;
		/* no allied piece on destination */
		if (get_diplomacy(current_player, dst_piece->owner) == 1)
			return 0;
	}
	return 1;
}

unsigned int move_piece(piece_t * piece, const uint16_t dst_height,
			const uint16_t dst_width)
{
	uint16_t src_height = piece->height;
	uint16_t src_width = piece->width;
	unsigned int movable =
	    is_legal_move(src_height, src_width, dst_height, dst_width);

	if (movable == 0)
		return 1;

	/* if there's a piece at destination, remove it */
	piece_t *dst_piece = world->grid->tiles[dst_height][dst_width]->piece;
	player_t *dst_player = NULL;
	if (dst_piece != NULL) {
		dst_player = dst_piece->owner;
		/* a noble was killed, means a player lost */
		if (dst_piece->type == NOBLE) {
			succession(dst_player);
			remove_player(dst_player);
		} else
			remove_piece(dst_piece);
		update_army_ranking();
	}
	/* update grid */
	world->grid->tiles[piece->height][piece->width]->piece = NULL;
	piece->height = dst_height;
	piece->width = dst_width;
	world->grid->tiles[dst_height][dst_width]->piece = piece;
	set_cursor(dst_height, dst_width);
	world->moves_left--;
	return 0;
}

void toggle_walkable(const uint16_t height, const uint16_t width)
{
	/* return if grid not created or tile is occupied */
	if (world->grid == NULL
	    || world->grid->tiles[height][width]->piece != NULL)
		return;
	world->grid->tiles[height][width]->walkable =
	    (world->grid->tiles[height][width]->walkable + 1) % 2;
}
