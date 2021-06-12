#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map.h"
#include "piece.h"
#include "diplomacy.h"
#include "window.h"
#include "world.h"

inline static tile_t *tile_init();

inline static tile_t *tile_init()
{
	tile_t *instance = malloc(sizeof(tile_t));
	if (!instance) exit(EXIT_FAILURE);
	instance->walkable = 1;
	instance->region = NULL;
	instance->piece = NULL;
	return instance;
}

region_t *add_region(const char *name)
{
	region_t *new = malloc(sizeof(region_t));
	if (!new) exit(EXIT_FAILURE);
	new->id = world->next_region_id;
	world->next_region_id++;
	new->size = 0;	/* start with no tiles */
	strcpy(new->name, name);
	new->owner = NULL;
	new->tiles = NULL;
	new->prev = NULL;
	new->next = NULL;

	if (world->regionlist == NULL) {
		world->regionlist = new;
	}
	else {
		region_t *prev = world->regionlist;
		/*fast-forward to the end of list */
		while (prev->next != NULL) {
			prev = prev->next;
		}
		new->prev = prev;
		prev->next = new;
	}
	return new;
}

void change_tile_region(region_t * new_region, tile_t * tile)
{
	/* same region */
	if (new_region != NULL && tile->region != NULL
	    && tile->region == new_region) {
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
		tile_t **tmp = realloc(new_region->tiles,
		    (new_region->size) * sizeof(tile_t *));
		if (!tmp) exit(EXIT_FAILURE);
		new_region->tiles = tmp;
		new_region->tiles[new_region->size - 1] = tile;
		return;
	}
	/* reassign tile from one region to another */
	else if (tile->region != NULL && new_region != NULL) {
		tile_t **newlist =
		    malloc(sizeof(tile_t *) * (tile->region->size - 1));
		if (!newlist) exit(EXIT_FAILURE);
		j = 0;
		for (i = 0; i < tile->region->size; i++) {
			if ((tile->region->tiles)[i] != tile) {
				newlist[j] = (tile->region->tiles)[i];
				j++;
			}
		}
		tile->region->size--;
		tile_t **tmp = realloc(new_region->tiles,
		    (new_region->size + 1) * sizeof(tile_t *));
		if (!tmp) exit(EXIT_FAILURE);
		new_region->tiles = tmp;
		new_region->size++;
		new_region->tiles[new_region->size - 1] = tile;
		free(tile->region->tiles);
		(tile->region->tiles) = newlist;
		tile->region = new_region;
		return;
	}
	/* remove tile from region */
	else if (tile->region != NULL && new_region == NULL) {
		tile_t **newlist =
		    malloc(sizeof(tile_t *) * (tile->region->size - 1));
		if (!newlist) exit(EXIT_FAILURE);
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
	    && region->owner == new_character) {
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
	region_t *prev = region->prev;
	region_t *next = region->next;
	if (prev) prev->next = next;
	if (next) next->prev = prev;
	if (world->regionlist == region) world->regionlist = next;
}

void sort_region_list() {
	if (world->regionlist == NULL || world->regionlist->next == NULL) return;
	int permutations;
	region_t *current, *next;
	do {
		current = world->regionlist;
		next = current->next;
		permutations = 0;
		while (next != NULL) {
			if (strcmp(current->name, next->name) > 0) {
				permutations++;
				if (current->prev != NULL) current->prev->next = next;
				else world->regionlist = next;
				if (next->next) next->next->prev = current;
				next->prev = current->prev;
				current->next = next->next;
				current->prev = next;
				next->next = current;
			}
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
}

uint16_t count_tiles_by_owner(character_t * owner)
{
	uint16_t count = 0;
	region_t *current = world->regionlist;
	while (current != NULL) {
		if (current->owner != NULL && current->owner == owner)
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
		if (current->owner != NULL && current->owner == owner)
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

grid_t *create_grid(const uint16_t height, const uint16_t width)
{
	if (world->grid != NULL)
		return world->grid;
	int i, j;
	grid_t *grid = malloc(sizeof(grid_t));
	if (!grid) exit(EXIT_FAILURE);
	grid->height = height;
	grid->width = width;
	grid->tiles = malloc(height * sizeof(void *));
	if (!grid->tiles)  exit(EXIT_FAILURE);
	for (i = 0; i < height; i++) {
		grid->tiles[i] = malloc(width * sizeof(void *));
		if (!grid->tiles[i])  exit(EXIT_FAILURE);
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
		if (dst_piece->owner == current_character)
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
