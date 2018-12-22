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
	if (!instance) return NULL;
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
	if (!region) return;
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
		if (!newlist) return;
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
		if (!newlist) return;
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

void change_region_owner(character_t * new_character, region_t * region)
{
	/* same owner */
	if (region->owner != NULL && new_character != NULL
	    && region->owner->id == new_character->id) {
		return;
	}
	/* if region is not assigned AND new owner is real */
	if (region->owner == NULL && new_character != NULL) {
		region->owner = new_character;
	}
	/* reassign region from one character to another */
	else if (region->owner != NULL && new_character != NULL) {
		region->owner = new_character;
	}
	/* remove tile from region */
	else if (new_character == NULL) {
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

unsigned char claim_region(character_t * character, region_t * region)
{
	/**
	 * return value:
	 * 0 -- not claimed
	 * 1 -- claimed from nature
	 * 2 -- conquered from enemy
	**/
	if (character == NULL || region == NULL)
		return 0;

	character_t *current_owner = region->owner;

	/* if the region is free, take it and stop here */
	if (current_owner == NULL) {
		change_region_owner(character, region);
		return 1;
	}

	/* if region is already ours, stop here */
	if (current_owner == character)
		return 0;

	/* if current owner is in the region, stop here */
	piece_t *owners_noble = get_noble_by_owner(current_owner);
	region_t *present_location = owners_noble->tile->region;
	if (region == present_location)
		return 0;

	/* if we are at war with current owner, take it */
	if (get_diplomacy(current_owner, character) == WAR) {
		change_region_owner(character, region);
		/* if previous owner has no other regions, he lost */
		world->check_death = 1;
		return 2;
	}

	/* if we are neutral with current owner, take it and trigger war */
	if (get_diplomacy(current_owner, character) == NEUTRAL) {
		change_region_owner(character, region);
		set_diplomacy(current_owner, character, WAR);
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

	clear_region(region);
	region_t *prev = NULL;
	region_t *current = world->regionlist;
	while (current != NULL) {
		if (current == region) {
			if (prev) prev->next = current->next;
			else world->regionlist = current->next;
			region_t *tmp = current;
			free(tmp);
			return;
		}
		prev = current;
		current = current->next;
	}
}

void sort_region_list() {
	if (world->regionlist == NULL || world->regionlist->next == NULL) return;
	int permutations;
	region_t *previous, *current, *next;
	do {
		current = world->regionlist;
		previous = NULL;
		next = current->next;
		permutations = 0;
		while (next != NULL) {
			if (strcmp(current->name, next->name) > 0) {
				permutations++;
				if (previous != NULL) previous->next = next;
				else world->regionlist = next;
				current->next = next->next;
				next->next = current;
			}
			previous = current;
			current = next;
			next = next->next;
		}
	} while (permutations > 0);
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

uint16_t count_tiles_by_owner(character_t * owner)
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

uint16_t count_regions_by_owner(character_t * owner)
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

uint16_t count_regions()
{
	uint16_t counter = 0;
	region_t *current = world->regionlist;
	while (current != NULL) {
		counter++;
		current = current->next;
	}
	return counter;
}

void update_land_ranking()
{
	character_t *character = NULL;
	character_t *character2 = NULL;

	/* reset ranks to 1 */
	character = world->characterlist;
	while (character != NULL) {
		character->rank_land = 1;
		character = character->next;
	}
	/* rewind to start */
	character = world->characterlist;

	while (character != NULL) {
		character2 = world->characterlist;
		while (character2->id != character->id) {
			if (count_tiles_by_owner(character) <=
			    count_tiles_by_owner(character2))
				character->rank_land++;
			else
				character2->rank_land++;
			character2 = character2->next;
		}
		character = character->next;
	}
}

grid_t *create_grid(const uint16_t height, const uint16_t width)
{
	if (world->grid != NULL)
		return world->grid;
	int i, j;
	grid_t *grid = malloc(sizeof(grid_t));
	if (!grid) return NULL;
	grid->height = height;
	grid->width = width;
	grid->tiles = malloc(height * sizeof(void *));
	if (!grid->tiles) {
		free(grid);
		return NULL;
	}
	for (i = 0; i < height; i++) {
		grid->tiles[i] = malloc(width * sizeof(void *));
		for (j = 0; j < width; j++) {
			grid->tiles[i][j] = tile_init();
			grid->tiles[i][j]->height = i;
			grid->tiles[i][j]->width = j;
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
	/* there is a piece on source tile and piece belongs to current character */
	character_t *current_character = world->selected_character;
	piece_t *src_piece = world->grid->tiles[src_height][src_width]->piece;
	if (src_piece == NULL || src_piece->owner->id != current_character->id)
		return 0;
	/* no piece at dest or neutral/enemy piece */
	piece_t *dst_piece = world->grid->tiles[dst_height][dst_width]->piece;
	if (dst_piece != NULL) {
		/* no own piece on destination tile */
		if (dst_piece->owner->id == current_character->id)
			return 0;
		/* no allied piece on destination */
		if (get_diplomacy(current_character, dst_piece->owner) == ALLIANCE)
			return 0;
	}
	return 1;
}

unsigned int move_piece(piece_t * piece, const uint16_t dst_height,
			const uint16_t dst_width)
{
	character_t *src_character = piece->owner;
	uint16_t src_height = piece->tile->height;
	uint16_t src_width = piece->tile->width;
	unsigned int movable =
	    is_legal_move(src_height, src_width, dst_height, dst_width);

	if (movable == 0)
		return 1;

	/* if there's a piece at destination, remove it */
	piece_t *dst_piece = world->grid->tiles[dst_height][dst_width]->piece;
	character_t *dst_character = NULL;
	if (dst_piece != NULL) {
		dst_character = dst_piece->owner;
		/* a noble was killed, means a character lost */
		if (dst_piece->type == NOBLE) {
			succession(dst_character);
			remove_character(dst_character);
		} else {
			remove_piece(dst_piece);
			/* if src and dest are neutral, trigger war */
			if (get_diplomacy(src_character, dst_character) != WAR) set_diplomacy(src_character, dst_character, WAR);
		}
		update_army_ranking();
	}
	/* update grid */
	world->grid->tiles[src_height][src_width]->piece = NULL;
	piece->tile = world->grid->tiles[dst_height][dst_width];
	world->grid->tiles[dst_height][dst_width]->piece = piece;
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

tile_t *region_center(region_t *region) {
	if (!region || region->size == 0) return NULL;
	int16_t sum_h = 0;
	int16_t sum_w = 0;
	uint16_t i;
	for (i = 0; i < region->size; i++) {
		sum_h += region->tiles[i]->height;
		sum_w += region->tiles[i]->width;
	}
	int16_t center_h = sum_h / region->size;
	int16_t center_w = sum_w / region->size;
	tile_t *tile = region->tiles[0];

	for (i = 1; i < region->size; i++) {
		if (abs(region->tiles[i]->height - center_h) + abs(region->tiles[i]->width - center_w) < abs(tile->height - center_h) + abs(tile->width - center_w)) {
			tile = region->tiles[i];
		}
	}
	return tile;
}

tile_t *get_empty_tile_in_region(region_t *region) {
	if (!region || region->size == 0) return NULL;
	/* compute region center */
	tile_t *center = region_center(region);
	if (!center) return NULL;
	if (!center->piece) return center;
	uint16_t distance_min = world->grid->height + world->grid->width;
	uint16_t distance = 0;
	tile_t *tile_min = NULL;
	tile_t *tile = NULL;
	uint16_t i;
	for (i = 0; i < region->size; i++) {
		tile = region->tiles[i];
		if (!tile->piece) {
			distance = abs(tile->height - center->height) + abs(tile->width - center->width);
			if (distance < distance_min) {
				distance_min = distance;
				tile_min = tile;
			}
		}
	}
	return tile_min;
}
