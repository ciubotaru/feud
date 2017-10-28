#ifndef WORLD_H
#define WORLD_H
#include "file.h"
#include "player.h"
#include "map.h"
#include "dice.h"
#include "piece.h"
#include "diplomacy.h"
#include "voronoi.h"
#include "window.h"
#include "config.h"

typedef struct {
	grid_t *grid;

	player_t *playerlist;
	uint16_t next_player_id;
	uint16_t selected_player;

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
} world_t;

world_t *world;

void create_world(void);

void destroy_world(void);

char message[255];		/* ToDo replace all occurences with user_message */

char user_message[255];

char display_message_str[255];

void add_user_message(char *new_text);

void msg_to_display(char *new_text);

int validate_game_data(char **error_message);

#endif
