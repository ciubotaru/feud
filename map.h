#ifndef MAP_H
#define MAP_H

#define MAP_WIDTH 80
#define MAP_HEIGHT 24

#include <stdint.h>
#include "piece.h"

struct tile;

struct region;

struct grid;

typedef struct tile {
	unsigned char walkable;
//      uint16_t owner; /* 0 is no owner */
	struct region *region;
	uint16_t height;
	uint16_t width;
	piece_t *piece;		/* should be a pointer to piece_t structure, NULL if empty */
} tile_t;

typedef struct region {
	uint16_t id;
	uint16_t size;
	char name[17];
	tile_t **tiles;		/* pointer to an array of pointers to tiles */
	character_t *owner;
	struct region *next;
} region_t;

region_t *add_region(const char *name);	//, const uint16_t height, const uint16_t width);

void change_tile_region(region_t *new_region, tile_t *tile);

void change_region_owner(character_t *new_character, region_t *region);

void change_region_name(char *new_name, region_t *region);

unsigned char claim_region(character_t *character, region_t *region);

region_t *get_region_by_id(const uint16_t id);

region_t *get_region_by_name(const char *name);

uint16_t get_region_order(region_t *region);

region_t *get_region_by_order(uint16_t order);

void update_land_ranking();

void clear_region(region_t *region);

void remove_region(region_t *region);

void sort_region_list();

void clear_region_list();

uint16_t count_tiles_by_owner(character_t *owner);

uint16_t count_regions_by_owner(character_t *owner);

int count_regions();

void update_land_ranking();

typedef struct grid {
	uint16_t height;
	uint16_t width;
	uint16_t cursor_height;
	uint16_t cursor_width;
	tile_t ***tiles;
} grid_t;

tile_t *tile_init();

grid_t *create_grid(uint16_t height, uint16_t width);

void remove_grid();

unsigned int is_legal_move(const uint16_t src_height, const uint16_t src_width,
			   const uint16_t dst_height, const uint16_t dst_width);

unsigned int move_piece(piece_t *piece, const uint16_t dst_heigh,
			const uint16_t dst_width);

void toggle_walkable(const uint16_t height, const uint16_t width);

#endif				/* MAP_H */
