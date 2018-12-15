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
	 * start from 1. id = 0 means "no character",
	 * e.g. in region ownership.
	**/
	world->characterlist = NULL;
	world->next_character_id = 1;
	world->selected_character = NULL;

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
	clear_region_list();
	/* remove pieces */
	clear_piece_list();
	/* remove characters */
	clear_character_list();
	/* remove tiles */
	remove_grid();
}

char message[255] = { 0 };

char user_message[255] = { 0 };

/**
void add_user_message(char *new_text)
{
	memset(&user_message, 0, strlen(user_message));
	strcpy(user_message, new_text);
}
**/

int validate_game_data()
{
	/* returns 0 if game is playable or 1 otherwise */
	if (!world) create_world();
	char *msg;
	int error = 0;
	int i, j;
	character_t *character = NULL;
	region_t *region = NULL;
	piece_t *piece = NULL;
	int total_months =
	    world->current_time.tm_year * 12 + world->current_time.tm_mon;

	/* Stop if there's no map */
	if (world->grid == NULL) {
		msg = "Grid missing.";
		goto error;
	}

	/* Stop if there are no characters */
	if (world->characterlist == NULL) {
		msg = "Hero list missing.";
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

	/* There should be at least two characters */
	int nr_characters = count_characters();
	if (nr_characters < 2) {
		msg = "Less than 2 heroes.";
		goto error;
	}

	/* Each character should control at least one region */
	character = world->characterlist;
	while (character != NULL) {
		region = world->regionlist;
		error = 1;
		while (region != NULL) {
			if (region->owner != NULL
			    && region->owner->id == character->id) {
				error = 0;
				break;
			}
			region = region->next;
		}
		if (error == 1) {
			msg = "Landless hero detected.";
			goto error;
		}
		character = character->next;
	}

	/* Each character should have at least one unit */
	character = world->characterlist;
	while (character != NULL) {
		piece = world->piecelist;
		error = 1;
		while (piece != NULL) {
			if (piece->owner != NULL
			    && piece->owner->id == character->id) {
				error = 0;
				break;
			}
			piece = piece->next;
		}
		if (error == 1) {
			msg = "Pieceless hero detected.";
			goto error;
		}
		character = character->next;
	}

	/* Each character must have exactly one noble */
	int nr_nobles = 0;
	piece = world->piecelist;
	while (piece != NULL) {
		if (piece->type == NOBLE)
			nr_nobles++;
		piece = piece->next;
	}
	if (nr_nobles != nr_characters) {
		msg = "Nobles are more/less than heroes.";
		goto error;
	}

	/* All units should stand on walkable tiles */
	piece = world->piecelist;
	while (piece != NULL) {
		if (piece->tile->walkable == 0) {
			msg = "Piece on unwalkable tile detected.";
			goto error;
		}
		piece = piece->next;
	}

	/* death date should be later than current date */
	character = world->characterlist;
	while (character != NULL) {
		if (character->deathdate.tm_year * 12 + character->deathdate.tm_mon <=
		    total_months) {
			msg = "A dead hero detectd. Recheck!";
			goto error;
		}
		character = character->next;
	}

	/* birth date should be earlier than current date */
	character = world->characterlist;
	while (character != NULL) {
		if (character->birthdate.tm_year * 12 + character->birthdate.tm_mon >
		    total_months) {
			msg = "An unborn hero detected.";
			goto error;
		}
		character = character->next;
	}

	/* check if pointers from grid to pieces correspond to piece coords */
	tile_t *tile = NULL;
	piece = world->piecelist;
	while (piece != NULL) {
		if (piece->id != piece->tile->piece->id) {
			msg = "Grid-piece consistency broken.";
			goto error;
		}
		piece = piece->next;
	}

	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
			tile = world->grid->tiles[i][j];
			if (tile->piece != NULL) {
				if (tile->height != i
				    || tile->width != j) {
					msg = "Grid-piece consistency brpken.";
					goto error;
				}
			}
		}
	}

	/* check if all lords are higher ranks than their vassals */
	character = world->characterlist;
	while (character != NULL) {
		if (character->lord != NULL) {
			if (character->lord->rank <= character->rank) {
				msg =
				    "Inconsistency between lord and vassal rank detected.";
				goto error;
			}
		}
		character = character->next;
	}

	/* check if world->selected_character points to an existing character */
	if (world->selected_character == NULL) {
		msg = "Selected character does not exist.";
		goto error;
	}

	/* If we are here, then all checks passed */
	return 0;

 error:
	memcpy(world->message, msg, strlen(msg) + 1);
	return 1;
}
