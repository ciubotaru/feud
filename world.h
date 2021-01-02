#ifndef WORLD_H
#define WORLD_H
#include "file.h"
#include "character.h"
#include "map.h"
#include "dice.h"
#include "diplomacy.h"
#include "logic.h"
#include "mapgen.h"
#include "new.h"
#include "piece.h"
#include "window.h"
#include "config.h"

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) > (Y)) ? (Y) : (X))

typedef struct {
	grid_t *grid;

	character_t *characterlist;
	uint16_t next_character_id;
	character_t *selected_character;

	gametime_t current_time;

	piece_t *piecelist;
	uint16_t next_piece_id;
	char *const *ranklist;

	region_t *regionlist;
	uint16_t next_region_id;
	uint16_t selected_region;

	unsigned char moves_left;
	unsigned char check_death;

	dipstatus_t *diplomacylist;

	char message[255];
} world_t;

extern world_t *world;

void create_world(void);

void destroy_world(void);

int validate_game_data();

#endif
