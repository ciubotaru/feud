#include <stdlib.h>
#include <string.h>
#include "world.h"

world_t *world = NULL;

void create_world()
{
	if (world != NULL)
		return;		// world;
	world = malloc(sizeof(world_t));
	if (world == NULL)
		return;		// NULL;
	world->grid = NULL;

	/**
	 * start from 1. id = 0 means "no player",
	 * e.g. in region ownership.
	**/
	world->playerlist = NULL;
	world->next_player_id = 1;
	world->selected_player = 0;

//      world->current_time = NULL;
	init_gametime();

	world->piecelist = NULL;
	world->next_piece_id = 0;
	world->ranklist = NULL;

	/**
	 * start from 1. 0 means no region
	**/
	world->regionlist = NULL;
	world->next_region_id = 1;
	world->selected_region = 0;

	world->moves_left = 0;
	world->check_death = 0;
	init_gametime();

	world->diplomacylist = NULL;
}

void destroy_world()
{
	if (world == NULL)
		return;
	/* remove regions */
	while (world->regionlist != NULL)
		remove_region(world->regionlist);
	/* remove pieces */
	clear_piece_list();
	/* remove players */
	clear_player_list();
	/* remove tiles */
	remove_grid();
}

char message[255] = { 0 };

char user_message[255] = { 0 };

void add_user_message(char *new_text)
{
	memset(&user_message, 0, strlen(user_message));
	strcpy(user_message, new_text);
}

int validate_game_data(char **error_message)
{
	/* returns 0 if game is playable or 1 otherwise */
	char *msg;
	int error = 0;
	int i, j;
	player_t *player = NULL;
	region_t *region = NULL;
	piece_t *piece = NULL;
	if (world == NULL) {
		msg = "World not created";
		goto error;
	}
	int total_months =
	    world->current_time.tm_year * 12 + world->current_time.tm_mon;

	/* Stop if there's no map */
	if (world->grid == NULL) {
		msg = "Grid missing.";
		goto error;
	}

	/* Stop if there are no players */
	if (world->playerlist == NULL) {
		msg = "Playerlist missing.";
		goto error;
	}

	/* Stop if there are no pieces */
	if (world->piecelist == NULL) {
		msg = "Piecelist missing.";
		goto error;
	}

	/* Stop if there are no regions */
	if (world->regionlist == NULL) {
		msg = "Regionlist missing.";
		goto error;
	}

	/* Stop if there are empty regions */
	region = world->regionlist;
	while (region != NULL) {
		if (region->size == 0) {
			msg = "Empty region detected.";
			goto error;
		}
		region = region->next;
	}

	/* There should be at least two players */
	int nr_players = count_players();
	if (nr_players < 2) {
		msg = "Less than 2 players.";
		goto error;
	}

	/* Each player should control at least one region */
	player = world->playerlist;
	while (player != NULL) {
		region = world->regionlist;
		error = 1;
		while (region != NULL) {
			if (region->owner != NULL
			    && region->owner->id == player->id) {
				error = 0;
				break;
			}
			region = region->next;
		}
		if (error == 1) {
			msg = "Landless player detected.";
			goto error;
		}
		player = player->next;
	}

	/* Each player should have at least one unit */
	player = world->playerlist;
	while (player != NULL) {
		piece = world->piecelist;
		error = 1;
		while (piece != NULL) {
			if (piece->owner != NULL
			    && piece->owner->id == player->id) {
				error = 0;
				break;
			}
			piece = piece->next;
		}
		if (error == 1) {
			msg = "Pieceless player detected.";
			goto error;
		}
		player = player->next;
	}

	/* Each player must have exactly one noble */
	int nr_nobles = 0;
	piece = world->piecelist;
	while (piece != NULL) {
		if (piece->type == NOBLE)
			nr_nobles++;
		piece = piece->next;
	}
	if (nr_nobles != nr_players) {
		msg = "Nobles are more/less than players.";
		goto error;
	}

	/* All units should stand on walkable tiles */
	error = 0;
	piece = world->piecelist;
	while (piece != NULL) {
		if (world->grid->tiles[piece->height][piece->width]->walkable ==
		    0) {
			error = 1;
			break;
		}
		if (error == 1) {
			msg = "Piece on unwalkable tile detected.";
			goto error;
		}
		piece = piece->next;
	}

	/* death date should be later than current date */
	error = 0;
	player = world->playerlist;
	while (player != NULL) {
		if (player->deathdate.tm_year * 12 + player->deathdate.tm_mon <=
		    total_months) {
			error = 1;
			break;
		}
		player = player->next;
	}
	if (error != 0) {
		msg = "A dead player detected. Recheck!";
		goto error;
	}

	/* birth date should be earlier than current date */
	error = 0;
	player = world->playerlist;
	while (player != NULL) {
		if (player->birthdate.tm_year * 12 + player->birthdate.tm_mon >
		    total_months) {
			error = 1;
			break;
		}
		if (error == 1) {
			msg = "An unborn player detected.";
			goto error;
		}
		player = player->next;
	}

	/* check if pointers from grid to pieces correspond to piece coords */
	error = 0;
	tile_t *tile = NULL;
	piece = world->piecelist;
	while (piece != NULL) {
		if (piece->id !=
		    world->grid->tiles[piece->height][piece->width]->piece->
		    id) {
			error = 1;
			break;
		}
		piece = piece->next;
	}
	if (error == 1) {
		msg = "Grid-piece inconsistency broken.";
		goto error;
	}

	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
			tile = world->grid->tiles[i][j];
			if (tile->piece != NULL) {
				if (tile->piece->height != i
				    || tile->piece->width != j) {
					error = 1;
					break;
				}
			}
		}
	}
	if (error == 1) {
		msg = "Grid-piece inconsistency broken.";
		goto error;
	}

	/* check if all lords are higher ranks than their vassals */
	error = 0;
	player = world->playerlist;
	while (player != NULL) {
		if (player->lord != NULL) {
			if (player->lord->rank <= player->rank) {
				error = 1;
				break;
			}
		}
		if (error == 1) {
			msg =
			    "Inconsistency between lord and vassal rank detected.";
			goto error;
		}
		player = player->next;
	}

	/* If we are here, then all checks passed */
	return 0;

 error:
	if (error_message) {
		*error_message = malloc(strlen(msg) + 1);
		memcpy(*error_message, msg, strlen(msg) + 1);
	}
	return 1;
}
