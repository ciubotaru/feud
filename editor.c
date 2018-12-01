#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>		/* for isdigit */
#include <math.h>		/* for log10 */
#include "world.h"

void editor_start_menu(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);
	wprintw(local_win, "  To open a saved game, press 'o'.\n");
	wprintw(local_win, "  To create a new game, press 'c'.\n");
	wprintw(local_win, "  To exit, press 'q'.");

	int ch;
	while (1) {
		ch = wgetch(local_win);
		switch (ch) {
		case 'o':
			current_screen = MAP_EDITOR;
			return;
		case 'c':
			current_screen = NEW_GAME;
			return;
		case 'q':
			current_screen = 99;
			return;
		}
	}
}

void map_editor(WINDOW *local_win)
{
	if (!cursor) cursor = world->grid->tiles[0][0];
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	uint16_t h_multiplier = cursor->height / 24;
	uint16_t w_multiplier = cursor->width / 48;
	int i, j;
	char tile_char = '.';
	int color_nr = 0;
	character_t *character = world->selected_character;
//	tile_t *cursor = cursor;
//	    world->grid->tiles[cursor->height][world->grid->
//							   cursor_width];
	region_t *region = NULL;
	if (selected_region == NULL && world->regionlist != NULL)
		selected_region = world->regionlist;
	region = selected_region;
	piece_t *piece = cursor->piece;

	for (i = 24 * (h_multiplier + 1) - 1; i >= 24 * h_multiplier; i--) {
		if (i >= world->grid->height) {
			wprintw(local_win, "\n");
			continue;
		}
		for (j = 48 * w_multiplier;
		     j < (48 * (w_multiplier + 1) <
			  world->grid->width ? 48 * (w_multiplier +
						     1) : world->grid->width);
		     j++) {
/**
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
**/
			if (world->grid->tiles[i][j]->piece != NULL) {
				color_nr =
				    world->grid->tiles[i][j]->piece->owner->id %
				    7 + 10;
				switch (world->grid->tiles[i][j]->piece->type) {
				case NOBLE:	/* noble */
					switch (world->grid->tiles[i][j]->
						piece->owner->rank) {
					case KNIGHT:
						tile_char = 'k';
						break;
					case BARON:
						tile_char = 'b';
						break;
					case COUNT:
						tile_char = 'c';
						break;
					case DUKE:
						tile_char = 'd';
						break;
					case KING:
						tile_char = 'K';
						break;
					}
					break;
				case SOLDIER:	/* soldier */
					tile_char = 's';
					break;
/* ToDo change 2 to SHIP when we have ships */
				case 2:	/* ship */
					tile_char = 'S';
					break;
				}
			} else if (world->grid->tiles[i][j]->walkable) {
				if (world->grid->tiles[i][j]->region != NULL
				    && world->grid->tiles[i][j]->region->
				    owner != NULL)
					color_nr =
					    world->grid->tiles[i][j]->region->
					    owner->id % 7 + 10;
				else
					color_nr = 1;
				tile_char = '.';
			} else {
				color_nr = 1;	/* blue */
				tile_char = '~';
			}
			if (cursor == world->grid->tiles[i][j]) {
				if (color_nr == 1)
					color_nr = 26;
				else
					color_nr += 10;
			}
			wcolor_set(local_win, color_nr, NULL);
			wprintw(local_win, "%c", tile_char);
		}
		wprintw(local_win, "\n");
	}
	wcolor_set(local_win, 1, NULL);

	/* global info */
	mvwprintw(local_win, 0, 50, "Date: %s of year %d",
		  months[world->current_time.tm_mon],
		  world->current_time.tm_year);
	mvwprintw(local_win, 1, 50, "Nr. heroes: %d", count_characters());
	mvwprintw(local_win, 2, 50, "Map size: %dx%d", world->grid->height,
		  world->grid->width);

	/* character info */
	if (character != NULL)
		mvwprintw(local_win, 4, 50, "Sel. pl.: %s (%s)", character->name,
			  rank_name[character->rank]);
	if (region != NULL)
		mvwprintw(local_win, 6, 50, "Sel. reg.: %s (%d)", region->name,
			  region->size);
	else
		mvwprintw(local_win, 6, 50, "Sel. reg.: not selected");
/**
		character_age_mon = (world->current_time.tm_year - character->birthdate.tm_year) * 12 + (world->current_time.tm_mon - character->birthdate.tm_mon);
		mvwprintw(local_win, 5, 50, "Age: %d year(s) %d month(s)", character_age_mon / 12, character_age_mon % 12);
		mvwprintw(local_win, 6, 50, "Money: %d (#%d)", character->money, character->rank_money);
		mvwprintw(local_win, 7, 50, "Army: %d (#%d)", count_pieces_by_owner(character), character->rank_army);
		mvwprintw(local_win, 8, 50, "Land: %d (#%d)", count_tiles_by_owner(character), character->rank_land);
		mvwprintw(local_win, 9, 50, "Heir: %s", (character->heir != NULL ? character->heir->name : "none"));
**/
	mvwprintw(local_win, 10, 50, "Moves left: %d", world->moves_left);

	/* place info */
	mvwprintw(local_win, 12, 50, "Tile: %d, %d", cursor->height,
		  cursor->width);
	mvwprintw(local_win, 13, 50, "Terrain: %s %s",
		  (cursor->walkable ? "walkable" : "unwalkable"), "land");
	mvwprintw(local_win, 14, 50, "Region: %s (%d)",
		  (cursor->region == NULL ? "none" : cursor->region->name),
		  (cursor->region == NULL ? 0 : cursor->region->id));
	mvwprintw(local_win, 15, 50, "Owned by: %s",
		  (cursor->region == NULL
		   || cursor->region->owner ==
		   NULL ? "none" : cursor->region->owner->name));

	/* piece info */
	mvwprintw(local_win, 17, 50, "Unit: %s",
		  (piece == NULL ? " " : piece_name[piece->type]));
	mvwprintw(local_win, 18, 50, "Owned by: %s",
		  (piece == NULL ? " " : piece->owner->name));
/**
	mvwprintw(local_win, 19, 50, "Diplomacy: %s", (piece == NULL || piece->owner->id == character->id ? " " : dipstatuslist[get_diplomacy(piece->owner, character)]));
**/

	int ch;
	ch = get_input(local_win);

	switch (ch) {
	case 27:		/* escape */
		current_screen = MAIN_SCREEN;
		return;
		break;
	case 1065:		/* up */
		if (cursor->height < world->grid->height - 1) cursor = world->grid->tiles[cursor->height + 1][cursor->width];
		break;
	case 1066:		/* down */
		if (cursor->height > 0) cursor = world->grid->tiles[cursor->height - 1][cursor->width];
		break;
	case 1067:		/* right */
		if (cursor->width < world->grid->width - 1) cursor = world->grid->tiles[cursor->height][cursor->width + 1];
		break;
	case 1068:		/* left */
		if (cursor->width > 0) cursor = world->grid->tiles[cursor->height][cursor->width - 1];
		break;
	case 'c':
		/* if tile is not part of region, include it, else remove */
		if (cursor->region == NULL)
			change_tile_region(region, cursor);
		else
			change_tile_region(NULL, cursor);
		break;
	case 'h':
		current_screen = CHARACTERS_DIALOG;
		break;
	case 'n':
		/* if tile not wlakable, just ignore */
		if (cursor->walkable == 0)
			break;
		/* if tile is occupied, remove piece */
		if (piece != NULL)
			remove_piece(piece);
		/* if tile free, add soldier */
		else
			add_piece(NOBLE, cursor->height,
				  cursor->width, character);
		break;
	case 'q':
		current_screen = 99;
		break;
	case 'r':
		current_screen = REGIONS_DIALOG;
		break;
	case 's':
		save_game();
		break;
	case 't':
		current_screen = GAME_TIME_DIALOG;
		break;
	case 'u':
		/* if tile not wlakable, just ignore */
		if (cursor->walkable == 0)
			break;
		/* if tile is occupied, remove piece */
		if (piece != NULL)
			remove_piece(piece);
		/* if tile free, add soldier */
		else
			add_piece(SOLDIER, cursor->height,
				  cursor->width, character);
		break;
	case 'v':
		current_screen = VALIDATE_DIALOG;
		break;
	case 'w':
		toggle_walkable(cursor->height, cursor->width);
		break;
	case '\t':
		if (piece != NULL) {
			piece = next_piece(piece);
			cursor = piece->tile;
		}
		break;
	case '>':
		if (region != NULL) {
			if (region->next != NULL)
				region = region->next;
			else
				region = world->regionlist;
			selected_region = region;
		} else if (world->regionlist != NULL) {
			region = world->regionlist;
			selected_region = region;
		}
		break;
	case ' ':
		character = world->selected_character;
		if (character->next != NULL)
			character = character->next;
		else
			character = world->characterlist;
		world->selected_character = character;
		piece = get_noble_by_owner(character);
		if (piece != NULL) {
			cursor = piece->tile;
		}
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
		world->moves_left = ch - '0';
		break;
	case '?':
		current_screen = HELP_DIALOG;
		break;
	}
	return;
}

void characters_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);

	wprintw(local_win, "  To add a hero, press 'a'.\n");
	wprintw(local_win, "  To delete a hero, press 'd'.\n");
	wprintw(local_win, "  To edit a hero, press 'e'.\n");
	wprintw(local_win, "  To scroll, press up/down keys.\n");
	wprintw(local_win, "  To return to map editor, press 'q'.\n\n");

	int nr_characters = count_characters();
	int counter = 0;
	int section = 0;
	character_t *current = world->characterlist;
	int characterlist_selector;
	while (current) {
		characterlist_selector = get_character_order(world->selected_character);
		section = characterlist_selector / 10;
		if (counter >= section * 10 && counter < section * 10 + 10
		    && counter < nr_characters) {
			if (counter == characterlist_selector) {
				wattron(local_win, COLOR_PAIR(26));
				world->selected_character = current;
			} else
				wattron(local_win, COLOR_PAIR(1));
			mvwprintw(local_win, 8 + counter % 10, 2, "%3d. %s\n",
				  current->id, current->name);
			wattron(local_win, COLOR_PAIR(1));
		}
		current = current->next;
		counter++;
	}

	int user_move = get_input(local_win);
	switch (user_move) {
	case 'a':
		current_screen = ADD_CHARACTER_DIALOG;
		break;
	case 'd':
		current = world->selected_character;
		if (world->selected_character->next) world->selected_character = world->selected_character->next;
		else world->selected_character = world->selected_character->prev;
		remove_character(current);
		break;
	case 'e':
		current_screen = EDIT_CHARACTER_DIALOG;
		break;
	case 1065:
		if (world->selected_character->prev) {
			world->selected_character = world->selected_character->prev;
		}
		break;
	case 1066:
		if (world->selected_character->next) {
			world->selected_character = world->selected_character->next;
		}
		break;
	case 'q':
		characterlist_selector = 0;
		current_screen = MAP_EDITOR;
		break;
	default:
		break;
	}
	return;
}

void add_character_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(TRUE);
	echo();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	int stage = 0;
	int error = 0;
	uint16_t money = 0;
	char name[17] = { 0 };
	char money_ch[MONEY_MAX_DIGITS + 1] = { 0 };
	unsigned char rank = 0;

	character_t *character = NULL;
	while (1) {
		switch (stage) {
		case 0:
			if (error == 1)
				mvwprintw(local_win, 2, 2,
					  "Name too short or not unique. Type another one:\n\n  ");
			else
				mvwprintw(local_win, 2, 2,
					  "Type a unique name for the new hero:         \n\n  ");
			wgetnstr(local_win, name, 16);
			if (strlen(name) == 0) {
				current_screen = CHARACTERS_DIALOG;
				return;
			}
			if (get_character_by_name(name) == NULL) {
				stage = 1;
				error = 0;
			} else
				error = 1;
			break;
		case 1:
			curs_set(FALSE);
			noecho();
			if (error == 1)
				mvwprintw(local_win, 6, 2,
					  "Error. Choose a valid rank (1-4):       \n\n  ");
			else
				mvwprintw(local_win, 6, 2,
					  "Choose hero ranking (1-5 for knight, baron, count, duke or king), default is king:\n\n  ");
			rank = wgetch(local_win);
			if (rank == '1' || rank == '2' || rank == '3'
			    || rank == '4' || rank == '5') {
				rank = (int)rank - '0';
				rank -= 1;	/* to start notation from 0 */
			} else if (rank == 10)
				rank = KING;
			else
				error = 1;
			if (!error) {
				mvwprintw(local_win, 8, 2, "%s",
					  rank_name[rank]);
				stage = 2;
			}
			break;
		case 2:
			curs_set(TRUE);
			echo();
			if (error == 1)
				mvwprintw(local_win, 10, 2,
					  "Error. Try another number (0-%d):       \n\n  ", MONEY_MAX);
			else
				mvwprintw(local_win, 10, 2,
					  "Type the initial amount of money (0-%d):\n\n  ", MONEY_MAX);
			money_ch[0] = '\0';
			wgetnstr(local_win, money_ch, MONEY_MAX_DIGITS);
			if (strlen(money_ch) == 0) {
				error = 1;
				break;
			}
			for (i = 0; i < strlen(money_ch); i++) {
				if (!isdigit(money_ch[i])) {
					error = 1;
					break;
				}
			}
			money = atoi(money_ch);
			stage = 3;
			break;
		case 3:
			character = add_character(name);
			set_money(character, money);
			set_character_rank(character, rank);
			world->selected_character = character;
			current_screen = CHARACTERS_DIALOG;
			curs_set(FALSE);
			noecho();
			return;
			break;
		}
	}
	return;
}

void regions_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);

	wprintw(local_win, "  To create a new region, press 'a'.\n");
	wprintw(local_win, "  To delete a region, press 'd'.\n");
	wprintw(local_win, "  To rename a region, press 'e'.\n");
	wprintw(local_win, "  To edit region owner, press 'o'.\n");
	wprintw(local_win, "  To scroll, press up/down keys.\n");
	wprintw(local_win, "  To return to map editor, press 'q'.\n\n");

	int nr_regions = count_regions();
	int counter = 0;
	int section = 0;
	region_t *current = world->regionlist;
	int regionlist_selector =
	    get_region_order(selected_region);
	while (current != NULL) {
		section = regionlist_selector / 10;
		if (counter >= section * 10 && counter < section * 10 + 10
		    && counter < nr_regions) {
			if (counter == regionlist_selector) {
				wattron(local_win, COLOR_PAIR(26));
			} else
				wattron(local_win, COLOR_PAIR(1));
			mvwprintw(local_win, 10 + counter % 10, 2, "%3d. %s (%i tiles, owned by %s)\n", current->id, current->name, current->size, (current->owner != NULL ? current->owner->name : "none"));	/* size */
			wattron(local_win, COLOR_PAIR(1));
		}
		current = current->next;
		counter++;
	}

	int user_move = get_input(local_win);
	switch (user_move) {
	case 1065:
		if (regionlist_selector > 0) {
			--regionlist_selector;
			selected_region =
			    get_region_by_order(regionlist_selector);
		}
		break;
	case 1066:
		if (regionlist_selector < nr_regions - 1) {
			regionlist_selector++;
			selected_region =
			    get_region_by_order(regionlist_selector);
		}
		break;
	case 'a':
		current_screen = ADD_REGION_DIALOG;
		return;
		break;
	case 'd':
		counter = 0;
		current = world->regionlist;
		while (counter < regionlist_selector) {
			current = current->next;
			counter++;
		}
		if (current != NULL) {
			remove_region(current);
			sort_region_list();
			if (regionlist_selector > 0) {
				regionlist_selector--;
				selected_region =
				    get_region_by_order(regionlist_selector);
			}
		}
		break;
	case 'e':
		current_screen = RENAME_REGION_DIALOG;
		return;
		break;
	case 'o':
		current_screen = REGION_CHARACTER_DIALOG;
		return;
		break;
	case 'q':
		regionlist_selector = 0;
		current_screen = MAP_EDITOR;
		return;
		break;
	}
	return;
}

void add_region_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(TRUE);
	echo();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	int stage = 0;
	int error = 0;
	while (1) {
		switch (stage) {
		case 0:
			if (error == 1)
				mvwprintw(local_win, 2, 2,
					  "Name too short or not unique. Type another one:\n\n  ");
			else
				mvwprintw(local_win, 2, 2,
					  "Type a unique name for the new region (press Enter to return):\n\n  ");
			char name[17] = { 0 };
			wgetnstr(local_win, name, 16);
			if (strlen(name) == 0) {
				current_screen = CHARACTERS_DIALOG;
				return;
			}
			if (get_region_by_name(name) == NULL) {
				stage = 1;
				error = 0;
			} else
				error = 1;
			break;
		case 1:
			add_region(name);
			sort_region_list();
			current_screen = REGIONS_DIALOG;
			return;
			break;
		}
	}
	return;
}

void region_to_character(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	region_t *region = selected_region;
	character_t *selected_character = world->selected_character;
	int characterlist_selector =
	    get_character_order(world->selected_character);

	while (1) {
		curs_set(FALSE);
		noecho();
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		mvwprintw(local_win, 2, 2,
			  "To give this region to a hero, press Enter.\n");
		mvwprintw(local_win, 3, 2,
			  "To unset region owner, press 'd'.\n");
		mvwprintw(local_win, 4, 2, "To scroll, press up/down keys.\n");
		mvwprintw(local_win, 5, 2, "To return, press 'q'.\n\n");

		int nr_characters = count_characters();
		int counter = 0;
		int section = 0;
		character_t *current = world->characterlist;
		while (current != NULL) {
			section = characterlist_selector / 10;
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_characters) {
				if (counter == characterlist_selector) {
					wattron(local_win, COLOR_PAIR(26));
				} else
					wattron(local_win, COLOR_PAIR(1));
				mvwprintw(local_win, 8 + counter % 10, 2,
					  "%3d. %s", current->id,
					  current->name);
				if (region->owner != NULL
				    && region->owner->id == current->id)
					wprintw(local_win,
						" (current owner)\n");
				wattron(local_win, COLOR_PAIR(1));
			}
			current = current->next;
			counter++;
		}

		int user_move = get_input(local_win);
		switch (user_move) {
		case 1065:
			if (characterlist_selector > 0) {
				characterlist_selector--;
				counter = 0;
				current = world->characterlist;
				while (current != NULL) {
					if (counter == characterlist_selector) {
						selected_character = current;
						break;
					}
					counter++;
					current = current->next;
				}
			}
			break;
		case 1066:
			if (characterlist_selector < nr_characters - 1) {
				characterlist_selector++;
				counter = 0;
				current = world->characterlist;
				while (current != NULL) {
					if (counter == characterlist_selector) {
						selected_character = current;
						break;
					}
					counter++;
					current = current->next;
				}
			}
			break;
		case 10:
			change_region_owner(selected_character, region);
			current_screen = REGIONS_DIALOG;
			return;
			break;
		case 'd':
			change_region_owner(NULL, region);
			current_screen = REGIONS_DIALOG;
			return;
			break;
		case 'q':
			current_screen = REGIONS_DIALOG;
			return;
			break;
		}
	}
	return;
}

void rename_region_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(TRUE);
	echo();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	region_t *region = selected_region;
	mvwprintw(local_win, 2, 2,
		  "To rename this region, type new name (press Enter to cancel):\n\n  ");
	char name[17] = { 0 };
	wgetnstr(local_win, name, 16);
	curs_set(FALSE);
	noecho();
	if (strlen(name) == 0) {
		current_screen = REGIONS_DIALOG;
		return;
	}
	change_region_name(name, region);
	sort_region_list();
	current_screen = REGIONS_DIALOG;
	return;
}

void edit_character_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	character_t *active_character = world->selected_character;
	/* we can not get into this function if there's no selected_character, but still */
	if (active_character == NULL) {
		current_screen = CHARACTERS_DIALOG;
		return;
	}
	mvwprintw(local_win, 2, 2, "Name: %s", active_character->name);
	mvwprintw(local_win, 3, 2, "Rank: %s", rank_name[active_character->rank]);
	mvwprintw(local_win, 4, 2, "Money: %d", active_character->money);
	mvwprintw(local_win, 5, 2, "Birthdate: %s of year %i",
		  months[active_character->birthdate.tm_mon],
		  active_character->birthdate.tm_year);
	mvwprintw(local_win, 6, 2, "Deathdate: %s of year %i",
		  months[active_character->deathdate.tm_mon],
		  active_character->deathdate.tm_year);
	mvwprintw(local_win, 7, 2, "Heir: %s",
		  (active_character->heir ==
		   NULL ? "none" : active_character->heir->name));
	mvwprintw(local_win, 8, 2, "Lord: %s",
		  (active_character->lord ==
		   NULL ? "none" : active_character->lord->name));

	mvwprintw(local_win, 10, 2, "To rename a hero, press 'r'");
	mvwprintw(local_win, 11, 2, "To change rank, press '+' or '-'");
	mvwprintw(local_win, 12, 2, "To change money, press 'm'");
	mvwprintw(local_win, 13, 2, "To change dates, press 'b'");
	mvwprintw(local_win, 14, 2, "To set heir, press 'h'");
	mvwprintw(local_win, 15, 2, "To set lord, press 'l'");
	mvwprintw(local_win, 16, 2, "To edit diplomacy, press 'd'");
	mvwprintw(local_win, 17, 2, "To return, press 'q'");

	while (1) {
		int user_move = get_input(local_win);
		switch (user_move) {
		case '+':
			if (active_character->rank < KING)
				active_character->rank++;
			return;
			break;
		case '-':
			if (active_character->rank > KNIGHT)
				active_character->rank--;
			return;
			break;
		case 'b':
			current_screen = CHARACTER_DATES_DIALOG;
			return;
			break;
		case 'd':
			current_screen = DIPLOMACY_DIALOG;
			return;
			break;
		case 'h':
			current_screen = HEIR_DIALOG;
			return;
			break;
		case 'l':
			current_screen = FEUDAL_DIALOG;
			return;
			break;
		case 'm':
			current_screen = CHARACTER_MONEY_DIALOG;
			return;
			break;
		case 'q':
			current_screen = CHARACTERS_DIALOG;
			return;
			break;
		case 'r':
			current_screen = RENAME_CHARACTER_DIALOG;
			return;
			break;
		}
	}
}

void rename_character_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(TRUE);
	echo();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	character_t *character = world->selected_character;

	while (1) {
		wmove(local_win, 3, 0);
		wclrtoeol(local_win);
		mvwprintw(local_win, 2, 2,
			  "Current name is '%s'. Type a new name, or press Enter to return.\n  New name: ",
			  character->name);
		char name[17] = { 0 };
		wgetnstr(local_win, name, 16);
		if (strlen(name) == 0) {
			current_screen = EDIT_CHARACTER_DIALOG;
			return;
		}
		if (get_character_by_name(name) == NULL) {
			strcpy(character->name, name);
			current_screen = EDIT_CHARACTER_DIALOG;
			return;
		}
		mvwprintw(local_win, 23, 2,
			  "Name not unique. Try another one.");
	}
}

void change_character_money_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(TRUE);
	echo();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	int error = 0;
	character_t *character = world->selected_character;
	uint16_t money = 0;
	char money_ch[MONEY_MAX_DIGITS + 1] = { 0 };

	while (1) {
		wmove(local_win, 3, 0);
		wclrtoeol(local_win);
		mvwprintw(local_win, 2, 2,
			  "Current amount is '%d'. Type new amount (0-%d), or press Enter to return.\n  New amount: ",
			  get_money(character), MONEY_MAX);
		wgetnstr(local_win, money_ch, MONEY_MAX_DIGITS);
		if (strlen(money_ch) == 0) {
			current_screen = EDIT_CHARACTER_DIALOG;
			return;
		}
		for (i = 0; i < strlen(money_ch); i++) {
			if (!isdigit(money_ch[i])) {
				error = 1;
			}
		}
		if (!error) {
			money = atoi(money_ch);
			set_money(character, money);
			current_screen = EDIT_CHARACTER_DIALOG;
			return;
		}
		mvwprintw(local_win, 23, 2, "Error. Try another number.");
	}
}

void change_character_dates_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(TRUE);
	echo();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	int error = 0;
	int stage = 1;
	character_t *character = world->selected_character;
	uint16_t birthyear = 0;
	char birthyear_ch[4] = { 0 };
	unsigned char birthmon = 0;
	char birthmon_ch[3] = { 0 };

	while (1) {
		switch (stage) {
		case 1:
			if (error == 1) {
				wmove(local_win, 2, 2);
				wclrtoeol(local_win);
				wmove(local_win, 3, 2);
				wclrtoeol(local_win);
				mvwprintw(local_win, 2, 2,
					  "Error. Try another number (0-999):\n\n  Year: ");
			} else
				mvwprintw(local_win, 2, 2,
					  "Current birthdate is '%s of %d'.\n  Type new year (0-999) and month (1-12), or press Enter to skip.\n  Year: ",
					  months[character->birthdate.tm_mon],
					  character->birthdate.tm_year);
			wgetnstr(local_win, birthyear_ch, 3);
			if (strlen(birthyear_ch) == 0) {
				mvwprintw(local_win, 4, 8, "%d",
					  character->birthdate.tm_year);
				error = 0;
				stage = 2;
				break;
			}
			for (i = 0; i < strlen(birthyear_ch); i++) {
				if (!isdigit(birthyear_ch[i])) {
					error = 1;
					break;
				}
			}
			if (!error) {
				birthyear = atoi(birthyear_ch);
				character->birthdate.tm_year = birthyear;
				error = 0;
				stage = 2;
			}
			break;
		case 2:
			if (error == 1) {
				wmove(local_win, 8, 2);
				wclrtoeol(local_win);
				wmove(local_win, 9, 2);
				wclrtoeol(local_win);
				mvwprintw(local_win, 2, 2,
					  "Error. Try another number (0-12):");
				mvwprintw(local_win, 4, 40, "Month: ");
			} else
				mvwprintw(local_win, 4, 40, "Month: ");
			wgetnstr(local_win, birthmon_ch, 2);
			if (strlen(birthmon_ch) == 0) {
				mvwprintw(local_win, 4, 47, "%s",
					  months[character->birthdate.tm_mon]);
				error = 0;
				stage = 3;
				break;
			}
			for (i = 0; i < strlen(birthmon_ch); i++) {
				if (!isdigit(birthmon_ch[i])) {
					error = 1;
					break;
				}
			}
			birthmon = atoi(birthmon_ch);
			if (birthmon < 1 || birthmon > 12) {
				error = 1;
				break;
			}
			character->birthdate.tm_mon = birthmon - 1;
			mvwprintw(local_win, 4, 47, "%s",
				  months[character->birthdate.tm_mon]);
			error = 0;
			stage = 3;
			break;
		case 3:
			if (error == 1) {
				wmove(local_win, 6, 2);
				wclrtoeol(local_win);
				wmove(local_win, 7, 2);
				wclrtoeol(local_win);
				mvwprintw(local_win, 8, 2,
					  "Error. Try another number (0-999):\n\n  Year: ");
			} else
				mvwprintw(local_win, 6, 2,
					  "Current deathdate is '%s of %d'.\n  Type new year (0-999) and month (1-12), or press Enter to skip.\n  Year: ",
					  months[character->deathdate.tm_mon],
					  character->deathdate.tm_year);
			uint16_t deathyear = 0;
			char deathyear_ch[3] = { 0 };
			wgetnstr(local_win, deathyear_ch, 3);
			if (strlen(deathyear_ch) == 0) {
				mvwprintw(local_win, 8, 8, "%d",
					  character->deathdate.tm_year);
				error = 0;
				stage = 4;
				break;
			}
			for (i = 0; i < strlen(deathyear_ch); i++) {
				if (!isdigit(deathyear_ch[i])) {
					error = 1;
					break;
				}
			}
			if (!error) {
				deathyear = atoi(deathyear_ch);
				character->deathdate.tm_year = deathyear;
				error = 0;
				stage = 4;
			}
			break;
		case 4:
			if (error == 1) {
				wmove(local_win, 6, 2);
				wclrtoeol(local_win);
				wmove(local_win, 7, 2);
				wclrtoeol(local_win);
				mvwprintw(local_win, 6, 2,
					  "Error. Try another number (0-12):");
				mvwprintw(local_win, 8, 40, "Month: ");
			} else
				mvwprintw(local_win, 8, 40, "Month: ");
			unsigned char deathmon = 0;
			char deathmon_ch[2] = { 0 };
			wgetnstr(local_win, deathmon_ch, 2);
			if (strlen(deathmon_ch) == 0) {
				mvwprintw(local_win, 8, 47, "%s",
					  months[character->deathdate.tm_mon]);
				error = 0;
				stage = 5;
				break;
			}
			for (i = 0; i < strlen(deathmon_ch); i++) {
				if (!isdigit(deathmon_ch[i])) {
					error = 1;
					break;
				}
			}
			deathmon = atoi(deathmon_ch);
			if (deathmon < 1 || deathmon > 12) {
				error = 1;
				break;
			}
			character->deathdate.tm_mon = deathmon - 1;
			mvwprintw(local_win, 8, 47, "%s",
				  months[character->deathdate.tm_mon]);
			error = 0;
			stage = 5;
			break;
		case 5:
			curs_set(FALSE);
			noecho();
			mvwprintw(local_win, 23, 2,
				  "Changes saved. Press any key to continue.");
			wgetch(local_win);
			current_screen = EDIT_CHARACTER_DIALOG;
			return;
		}
	}
}

void validate_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	char *msg;
	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);

	char *error_message = NULL;
	int validation_result = validate_game_data(&error_message);

	if (error_message == NULL) {
		msg = "All checks passed. Game data are valid.";
		error_message = malloc(strlen(msg) + 1);
		memcpy(error_message, msg, strlen(msg) + 1);
	}
	wmove(local_win, 10, 0);
	for (i = 0; i < (80 - strlen(error_message)) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, error_message);
	free(error_message);

	msg = "Press any key to continue.";
	wmove(local_win, 12, 0);
	for (i = 0; i < (80 - strlen(msg)) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", msg);

	wgetch(local_win);
	current_screen = MAP_EDITOR;
	return;
}

void edit_time_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(TRUE);
	echo();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);

	int stage = 0;
	int error = 0;
	while (1) {
		switch (stage) {
		case 0:
			if (error == 1) {
				wmove(local_win, 2, 2);
				wclrtoeol(local_win);
				wmove(local_win, 3, 2);
				wclrtoeol(local_win);
				mvwprintw(local_win, 2, 2,
					  "Error. Try another number (0-999):\n\n  Year: ");
			} else
				mvwprintw(local_win, 2, 2,
					  "Current date is '%s of %d'.\n  Type new year (0-999) and month (1-12), or press Enter to skip.\n  Year: ",
					  months[world->current_time.tm_mon],
					  world->current_time.tm_year);
			uint16_t year = 0;
			char year_ch[3] = { 0 };
			wgetnstr(local_win, year_ch, 3);
			if (strlen(year_ch) == 0) {
				mvwprintw(local_win, 4, 8, "%d",
					  world->current_time.tm_year);
				error = 0;
				stage = 1;
				break;
			}
			for (i = 0; i < strlen(year_ch); i++) {
				if (!isdigit(year_ch[i])) {
					error = 1;
					break;
				}
			}
			if (!error) {
				year = atoi(year_ch);
				world->current_time.tm_year = year;
				error = 0;
				stage = 1;
			}
			break;
		case 1:
			if (error == 1) {
				wmove(local_win, 2, 2);
				wclrtoeol(local_win);
				wmove(local_win, 3, 2);
				wclrtoeol(local_win);
				mvwprintw(local_win, 2, 2,
					  "Error. Try another number (0-12):");
				mvwprintw(local_win, 4, 40, "Month: ");
			} else
				mvwprintw(local_win, 4, 40, "Month: ");
			unsigned char mon = 0;
			char mon_ch[2] = { 0 };
			wgetnstr(local_win, mon_ch, 2);
			if (strlen(mon_ch) == 0) {
				mvwprintw(local_win, 4, 47, "%s",
					  months[world->current_time.tm_mon]);
				error = 0;
				stage = 2;
				break;
			}
			for (i = 0; i < strlen(mon_ch); i++) {
				if (!isdigit(mon_ch[i])) {
					error = 1;
					break;
				}
			}
			mon = atoi(mon_ch);
			if (mon < 1 || mon > 12) {
				error = 1;
				break;
			}
			world->current_time.tm_mon = mon - 1;
			mvwprintw(local_win, 4, 47, "%s",
				  months[world->current_time.tm_mon]);
			error = 0;
			stage = 2;
			break;
		case 2:
			mvwprintw(local_win, 22, 2,
				  "Game time updated. Don't forget to validate game data. Press any key.");
			curs_set(FALSE);
			noecho();
			wgetch(local_win);
			current_screen = MAP_EDITOR;
			return;
			break;
		}
	}
	return;
}

void successor_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *active_character = world->selected_character;
	character_t *heir = active_character->heir;
	int characterlist_selector =
	    get_character_order(world->selected_character);

	while (1) {
		curs_set(FALSE);
		noecho();
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		mvwprintw(local_win, 2, 2, "To pick a hero, press Enter.\n");
		mvwprintw(local_win, 3, 2, "To scroll, press up/down keys.\n");
		mvwprintw(local_win, 4, 2, "To remove heir, press 'd'.\n");
		mvwprintw(local_win, 5, 2, "To return, press 'q'.\n\n");

		int nr_characters = count_characters();
		int counter = 0;
		int section = 0;
		character_t *current = world->characterlist;
		while (current != NULL) {
			section = characterlist_selector / 10;
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_characters) {
				if (counter == characterlist_selector) {
					wattron(local_win, COLOR_PAIR(26));
				} else
					wattron(local_win, COLOR_PAIR(1));
				mvwprintw(local_win, 8 + counter % 10, 2,
					  "%3d. %s", current->id,
					  current->name);
				if (current == world->selected_character)
					wprintw(local_win, " (you)\n");
				else if (heir != NULL
					 && current->id == heir->id)
					wprintw(local_win, " (current heir)\n");
				else
					wprintw(local_win, "\n");
				wattron(local_win, COLOR_PAIR(1));
			}
			current = current->next;
			counter++;
		}

		int user_move = get_input(local_win);
		switch (user_move) {
		case 1065:
			if (characterlist_selector > 0)
				characterlist_selector--;
			break;
		case 1066:
			if (characterlist_selector < nr_characters - 1)
				characterlist_selector++;
			break;
		case 10:
			heir = get_character_by_order(characterlist_selector);
			if (heir->id != active_character->id)
				set_successor(active_character, heir);
			else
				heir = active_character->heir;
			break;
		case 'd':
			heir = NULL;
			set_successor(active_character, heir);
			break;
		case 'q':
			current_screen = EDIT_CHARACTER_DIALOG;
			return;
			break;
		}
	}
}

void homage_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *active_character = world->selected_character;
	character_t *lord = world->selected_character;
	int characterlist_selector;

	int set_lord_ok = 0;
	int unset_lord_ok = 0;

	while (1) {
		set_lord_ok = 0;
		unset_lord_ok = 0;

		curs_set(FALSE);
		noecho();
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		if (lord->id == active_character->id)
			mvwprintw(local_win, 2, 2,
				  "[You can't pay homage to yourself]\n");
		else if (lord->lord != NULL
			 && lord->lord->id == active_character->id)
			mvwprintw(local_win, 2, 2,
				  "[You can't pay homage to your own vassal]\n");
		else if (lord->rank <= KNIGHT)
			mvwprintw(local_win, 2, 2,
				  "[You can't pay homage to a knight]");
		else if (active_character->lord != NULL
			 && active_character->lord == lord) {
			mvwprintw(local_win, 2, 2, "To unset lord, press 'd'");
			unset_lord_ok = 1;
		} else {
			mvwprintw(local_win, 2, 2,
				  "To set lord, press Enter.\n");
			set_lord_ok = 1;
		}
		mvwprintw(local_win, 3, 2, "To scroll, press up/down keys.\n");
		mvwprintw(local_win, 4, 2, "To return, press 'q'.\n\n");

		int nr_characters = count_characters();
		int counter = 0;
		int section = 0;
		character_t *current = world->characterlist;
		characterlist_selector = get_character_order(lord);
		while (current != NULL) {
			section = characterlist_selector / 10;
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_characters) {
				if (counter == characterlist_selector) {
					wattron(local_win, COLOR_PAIR(26));
				} else
					wattron(local_win, COLOR_PAIR(1));
				mvwprintw(local_win, 8 + counter % 10, 2,
					  "%3d. %s", current->id,
					  current->name);
				if (current->id == active_character->id)
					wprintw(local_win, " (you)\n");
				else
					wprintw(local_win, " (%s)\n",
						rank_name[current->rank]);
				wattron(local_win, COLOR_PAIR(1));
			}
			current = current->next;
			counter++;
		}

		int user_move = get_input(local_win);
		switch (user_move) {
		case 1065:
			if (lord->prev) lord = lord->prev;
			break;
		case 1066:
			if (lord->next) lord = lord->next;
			break;
		case 10:
			if (set_lord_ok) {
				active_character->lord = lord;
				current_screen = EDIT_CHARACTER_DIALOG;
				return;
			}
			break;
		case 'd':
			if (unset_lord_ok) {
				active_character->lord = NULL;
				current_screen = EDIT_CHARACTER_DIALOG;
				return;
			}
			break;
		case 'q':
			current_screen = EDIT_CHARACTER_DIALOG;
			return;
			break;
		}
	}
}

void diplomacy_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *active_character = world->selected_character;
	character_t *selected_character = world->selected_character;
	int characterlist_selector =
	    get_character_order(world->selected_character);

	unsigned char alliance_ok, neutral_ok, war_ok;

	while (1) {
		dipstatus_t *dipstatus = NULL;
		unsigned char status = 0;
		alliance_ok = 0;	/* a */
		neutral_ok = 0;	/* n */
		war_ok = 0;	/* w */

		curs_set(FALSE);
		noecho();
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		if (active_character == selected_character) {
			dipstatus = NULL;
			status = 3;	/* self */
		} else {
			dipstatus =
			    get_dipstatus(active_character, selected_character);
			status = dipstatus->status;
		}
		switch (status) {
		case NEUTRAL:
			mvwprintw(local_win, 2, 2,
				  "To change to alliance, press 'a'");
			alliance_ok = 1;
			mvwprintw(local_win, 3, 2,
				  "To change to war, press 'w'");
			war_ok = 1;
			break;
		case ALLIANCE:
			mvwprintw(local_win, 2, 2,
				  "To change to neutral, press 'n'");
			neutral_ok = 1;
			mvwprintw(local_win, 3, 2,
				  "To change to war, press 'w'");
			war_ok = 1;
			break;
		case WAR:
			mvwprintw(local_win, 2, 2,
				  "To change to alliance, press 'a'");
			alliance_ok = 1;
			mvwprintw(local_win, 3, 2,
				  "To change to neutral, press 'n'");
			neutral_ok = 1;
			break;
		case 3:
			break;
		}

		mvwprintw(local_win, 4, 2, "To scroll, press up/down keys.");
		mvwprintw(local_win, 5, 2, "To return, press 'q'.");

		int nr_characters = count_characters();
		int counter = 0;
		int section = 0;
		character_t *current = world->characterlist;
		dipstatus_t *current_dipstatus = NULL;
		unsigned char current_status = 0;
		while (current != NULL) {
			if (active_character != current) {
				current_dipstatus =
				    get_dipstatus(active_character, current);
				current_status = current_dipstatus->status;
			}
			section = characterlist_selector / 10;
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_characters) {
				if (counter == characterlist_selector) {
					wattron(local_win, COLOR_PAIR(26));
				} else
					wattron(local_win, COLOR_PAIR(1));
				mvwprintw(local_win, 8 + counter % 10, 2,
					  "%3d. %s (", current->id,
					  current->name);
				if (current->id == active_character->id)
					wprintw(local_win, "you");
				else {
					wprintw(local_win, "%s",
						dipststus_name[current_status]);
				}
				wprintw(local_win, ")");
				wattron(local_win, COLOR_PAIR(1));
			}
			current = current->next;
			counter++;
		}

		int user_move = get_input(local_win);
		switch (user_move) {
		case 1065:
			if (characterlist_selector > 0) {
				characterlist_selector--;
				counter = 0;
				current = world->characterlist;
				while (current != NULL) {
					if (counter == characterlist_selector) {
						selected_character = current;
						break;
					}
					counter++;
					current = current->next;
				}
			}
			break;
		case 1066:
			if (characterlist_selector < nr_characters - 1) {
				characterlist_selector++;
				counter = 0;
				current = world->characterlist;
				while (current != NULL) {
					if (counter == characterlist_selector) {
						selected_character = current;
						break;
					}
					counter++;
					current = current->next;
				}
			}
			break;
		case 'a':	/* alliance */
			if (alliance_ok)
				set_diplomacy(active_character, selected_character,
					      ALLIANCE);
			break;
		case 'n':	/* neutral */
			if (neutral_ok)
				set_diplomacy(active_character, selected_character,
					      NEUTRAL);
			break;
		case 'q':	/* return */
			current_screen = EDIT_CHARACTER_DIALOG;
			return;
			break;
		case 'w':	/* declare war */
			if (war_ok)
				set_diplomacy(active_character, selected_character,
					      WAR);
			break;
		}
	}
}

void help_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	curs_set(FALSE);
	noecho();

	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);

	mvwprintw(local_win, 2, 2, "arrows -- move cursor or piece on the map");
	mvwprintw(local_win, 3, 2, "escape -- return to main menu");
	mvwprintw(local_win, 4, 2, "tab -- select next piece");
	mvwprintw(local_win, 5, 2, "space -- select next hero");
	mvwprintw(local_win, 6, 2, "'0-6' -- set moves left (roll dice)");
	mvwprintw(local_win, 7, 2, "'>' -- select next region");
	mvwprintw(local_win, 8, 2,
		  "'c' -- add tile to selected region or remove from its current region");
	mvwprintw(local_win, 9, 2, "'h' -- show heroes dialog");
	mvwprintw(local_win, 10, 2, "'n' -- place a noble on current tile");
	mvwprintw(local_win, 11, 2, "'q' -- quit editor");
	mvwprintw(local_win, 12, 2, "'r' -- show regions dialog");
	mvwprintw(local_win, 13, 2, "'s' -- save game to file");
	mvwprintw(local_win, 14, 2, "'t' -- set game date");
	mvwprintw(local_win, 15, 2, "'u' -- place a soldier on current tile");
	mvwprintw(local_win, 16, 2, "'v' -- validate game data");
	mvwprintw(local_win, 17, 2,
		  "'w' -- toggle walkability of current tile");
	mvwprintw(local_win, 18, 2, "'?' -- show this help");

	mvwprintw(local_win, 23, 2, "To return, press any key");
	get_input(local_win);
	current_screen = MAP_EDITOR;
}

int main()
{
	if (check_termsize()) {
		printf("Your terminal must be at least 24x80. Exiting...\n");
		return 0;
	}

	create_world();
//	cursor = world->grid->tiles[0][0];
	srand((unsigned)time(NULL));

	initscr();
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(10, COLOR_RED, COLOR_BLACK);
	init_pair(11, COLOR_GREEN, COLOR_BLACK);
	init_pair(12, COLOR_YELLOW, COLOR_BLACK);
	init_pair(13, COLOR_BLUE, COLOR_BLACK);
	init_pair(14, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(15, COLOR_CYAN, COLOR_BLACK);
	init_pair(16, COLOR_WHITE, COLOR_BLACK);
	init_pair(20, COLOR_BLACK, COLOR_RED);
	init_pair(21, COLOR_BLACK, COLOR_GREEN);
	init_pair(22, COLOR_BLACK, COLOR_YELLOW);
	init_pair(23, COLOR_BLACK, COLOR_BLUE);
	init_pair(24, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(25, COLOR_BLACK, COLOR_CYAN);
	init_pair(26, COLOR_BLACK, COLOR_WHITE);
	bkgd(COLOR_PAIR(1));

	noecho();
	curs_set(FALSE);
	WINDOW *local_win = newwin(25, 80, 0, 0);

	current_screen = 0;
	while (1) {
		switch (current_screen) {
		case MAIN_SCREEN:
			editor_start_menu(local_win);
			break;
		case NEW_GAME:
			new_game_dialog(local_win);
			break;
		case MAP_EDITOR:
			if (world->grid == NULL) {
				load_game();
				piece_t *active_piece = get_noble_by_owner(world->selected_character);
				cursor = active_piece->tile;
			}
			map_editor(local_win);
			break;
		case CHARACTERS_DIALOG:
			characters_dialog(local_win);
			break;
		case ADD_CHARACTER_DIALOG:
			add_character_dialog(local_win);
			break;
		case REGIONS_DIALOG:
			regions_dialog(local_win);
			break;
		case ADD_REGION_DIALOG:
			add_region_dialog(local_win);
			break;
		case REGION_CHARACTER_DIALOG:
			region_to_character(local_win);
			break;
		case GAME_TIME_DIALOG:
			edit_time_dialog(local_win);
			break;
		case RENAME_REGION_DIALOG:
			rename_region_dialog(local_win);
			break;
		case EDIT_CHARACTER_DIALOG:
			edit_character_dialog(local_win);
			break;
		case RENAME_CHARACTER_DIALOG:
			rename_character_dialog(local_win);
			break;
		case CHARACTER_MONEY_DIALOG:
			change_character_money_dialog(local_win);
			break;
		case CHARACTER_DATES_DIALOG:
			change_character_dates_dialog(local_win);
			break;
		case HEIR_DIALOG:
			successor_dialog(local_win);
			break;
		case FEUDAL_DIALOG:
			homage_dialog(local_win);
			break;
		case DIPLOMACY_DIALOG:
			diplomacy_dialog(local_win);
			break;
		case VALIDATE_DIALOG:
			validate_dialog(local_win);
			break;
		case HELP_DIALOG:
			help_dialog(local_win);
			break;
		case 99:
			destroy_world();
			use_default_colors();
			endwin();
			return 0;
			break;
		}
	}
}
