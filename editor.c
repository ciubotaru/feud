#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>		/* for isdigit */

#include "world.h"

void editor_start_menu()
{
	WINDOW *local_win = newwin(25, 80, 0, 0);
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
	ch = wgetch(local_win);
	switch (ch) {
	case 'o':
		delwin(local_win);
		current_screen = MAP_EDITOR;
		break;
	case 'c':
		delwin(local_win);
		current_screen = NEW_GAME;
		break;
	case 'q':
		delwin(local_win);
		current_screen = 99;
		break;
	}

	return;
}

void map_editor()
{
	WINDOW *local_win;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);

	uint16_t h_multiplier = world->grid->cursor_height / 24;
	uint16_t w_multiplier = world->grid->cursor_width / 48;
	int i, j;
	char tile_char;
	int color_nr = 0;
	player_t *player = get_player_by_id(world->selected_player);
	tile_t *tile =
	    world->grid->tiles[world->grid->cursor_height][world->grid->
							   cursor_width];
	region_t *region = NULL;
	if (world->selected_region == 0 && world->regionlist != NULL)
		world->selected_region = world->regionlist->id;
	region = get_region_by_id(world->selected_region);
	piece_t *piece = tile->piece;

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
			if (world->grid->cursor_height == i
			    && world->grid->cursor_width == j) {
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
	mvwprintw(local_win, 1, 50, "Nr. players: %d", count_players());
	mvwprintw(local_win, 2, 50, "Map size: %dx%d", world->grid->height,
		  world->grid->width);

	/* player info */
	if (player != NULL)
		mvwprintw(local_win, 4, 50, "Sel. pl.: %s (%s)", player->name,
			  ranklist[player->rank]);
	if (region != NULL)
		mvwprintw(local_win, 6, 50, "Sel. reg.: %s (%d)", region->name,
			  region->size);
	else
		mvwprintw(local_win, 6, 50, "Sel. reg.: not selected");
/**
		player_age_mon = (world->current_time.tm_year - player->birthdate.tm_year) * 12 + (world->current_time.tm_mon - player->birthdate.tm_mon);
		mvwprintw(local_win, 5, 50, "Age: %d year(s) %d month(s)", player_age_mon / 12, player_age_mon % 12);
		mvwprintw(local_win, 6, 50, "Money: %d (#%d)", player->money, player->rank_money);
		mvwprintw(local_win, 7, 50, "Army: %d (#%d)", count_pieces_by_owner(player), player->rank_army);
		mvwprintw(local_win, 8, 50, "Land: %d (#%d)", count_tiles_by_owner(player), player->rank_land);
		mvwprintw(local_win, 9, 50, "Heir: %s", (player->heir != NULL ? player->heir->name : "none"));
**/
	mvwprintw(local_win, 10, 50, "Moves left: %d", world->moves_left);

	/* place info */
	mvwprintw(local_win, 12, 50, "Tile: %d, %d", world->grid->cursor_height,
		  world->grid->cursor_width);
	mvwprintw(local_win, 13, 50, "Terrain: %s %s",
		  (tile->walkable ? "walkable" : "unwalkable"), "land");
	mvwprintw(local_win, 14, 50, "Region: %s (%d)",
		  (tile->region == NULL ? "none" : tile->region->name),
		  (tile->region == NULL ? 0 : tile->region->id));
	mvwprintw(local_win, 15, 50, "Owned by: %s",
		  (tile->region == NULL
		   || tile->region->owner ==
		   NULL ? "none" : tile->region->owner->name));

	/* piece info */
	mvwprintw(local_win, 17, 50, "Unit: %s",
		  (piece == NULL ? " " : unit_type_list[piece->type]));
	mvwprintw(local_win, 18, 50, "Owned by: %s",
		  (piece == NULL ? " " : piece->owner->name));
/**
	mvwprintw(local_win, 19, 50, "Diplomacy: %s", (piece == NULL || piece->owner->id == player->id ? " " : dipstatuslist[get_diplomacy(piece->owner, player)]));
**/

	int ch;
	ch = get_input(local_win);

	switch (ch) {
	case 27:		/* escape */
		current_screen = MAIN_SCREEN;
		return;
		break;
	case 1065:		/* up */
		set_cursor(world->grid->cursor_height + 1,
			   world->grid->cursor_width);
		break;
	case 1066:		/* down */
		set_cursor(world->grid->cursor_height - 1,
			   world->grid->cursor_width);
		break;
	case 1067:		/* right */
		set_cursor(world->grid->cursor_height,
			   world->grid->cursor_width + 1);
		break;
	case 1068:		/* left */
		set_cursor(world->grid->cursor_height,
			   world->grid->cursor_width - 1);
		break;
	case 'c':
		/* if tile is not part of region, include it, else remove */
		if (tile->region == NULL)
			change_tile_region(region,
					   world->grid->tiles[world->grid->
							      cursor_height]
					   [world->grid->cursor_width]);
		else
			change_tile_region(NULL,
					   world->grid->tiles[world->grid->
							      cursor_height]
					   [world->grid->cursor_width]);
		break;
	case 'n':
		/* if tile not wlakable, just ignore */
		if (tile->walkable == 0)
			break;
		/* if tile is occupied, remove piece */
		if (piece != NULL)
			remove_piece(piece);
		/* if tile free, add soldier */
		else
			add_piece(NOBLE, world->grid->cursor_height,
				  world->grid->cursor_width, player);
		break;
	case 'p':
		current_screen = PLAYERS_DIALOG;
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
		if (tile->walkable == 0)
			break;
		/* if tile is occupied, remove piece */
		if (piece != NULL)
			remove_piece(piece);
		/* if tile free, add soldier */
		else
			add_piece(SOLDIER, world->grid->cursor_height,
				  world->grid->cursor_width, player);
		break;
	case 'v':
		current_screen = VALIDATE_DIALOG;
		break;
	case 'w':
		toggle_walkable(world->grid->cursor_height,
				world->grid->cursor_width);
		break;
	case '\t':
		if (piece != NULL) {
			piece = next_piece(piece);
			world->grid->cursor_height = piece->height;
			world->grid->cursor_width = piece->width;
		}
		break;
	case '>':
		if (region != NULL) {
			if (region->next != NULL)
				region = region->next;
			else
				region = world->regionlist;
			world->selected_region = region->id;
		} else if (world->regionlist != NULL) {
			region = world->regionlist;
			world->selected_region = region->id;
		}
		break;
	case ' ':
		player = get_player_by_id(world->selected_player);
		if (player->next != NULL)
			player = player->next;
		else
			player = world->playerlist;
		world->selected_player = player->id;
		piece = get_noble_by_owner(player);
		if (piece != NULL) {
			world->grid->cursor_height = piece->height;
			world->grid->cursor_width = piece->width;
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

void new_game_dialog()
{
	WINDOW *local_win;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	wprintw(local_win,
		"\n\n  You are about to create a new game. This will destroy the old game and\n  overwrite the save file. Press 'y' to confirm.");
	int confirm = wgetch(local_win);
	if (confirm != 'y' && confirm != 'Y') {
		current_screen = MAIN_SCREEN;
		return;
	}
	wmove(local_win, 2, 0);
	wclrtoeol(local_win);
	wmove(local_win, 3, 0);
	wclrtoeol(local_win);
	wmove(local_win, 2, 2);

	curs_set(TRUE);
	echo();
	wprintw(local_win, "Map height (10-100, default 24): ");
	char h_ch[4];
	int h = 24;
	wgetnstr(local_win, h_ch, 3);
	for (i = 0; i < strlen(h_ch); i++) {
		if (!isdigit(h_ch[i]))
			break;
	}
	if (strlen(h_ch) > 0)
		h = atoi(h_ch);
	if (h == 0)
		h = 24;
	else if (h < 10)
		h = 10;
	else if (h > 100)
		h = 100;
	mvwprintw(local_win, 2, 35, "%3d", h);

	wprintw(local_win, "\n\n  Map width (10-100, default 80): ");
	char w_ch[4];
	int w = 80;
	wgetnstr(local_win, w_ch, 3);
	for (i = 0; i < strlen(w_ch); i++) {
		if (!isdigit(w_ch[i]))
			break;
	}
	if (strlen(w_ch) > 0)
		w = atoi(w_ch);
	if (w == 0)
		w = 80;
	else if (w < 10)
		w = 10;
	else if (w > 100)
		w = 100;
	mvwprintw(local_win, 4, 34, "%3d", w);

	wprintw(local_win,
		"\n\n  Size of region (10-100, default 36, type 0 to skip): ");
	char r_ch[8];
	int region_size = 36;
	wgetnstr(local_win, r_ch, 3);
	for (i = 0; i < strlen(r_ch); i++) {
		if (!isdigit(r_ch[i]))
			break;
	}
	if (strlen(r_ch) > 0)
		region_size = atoi(r_ch);
	else if (region_size < 10)
		region_size = 10;
	else if (region_size > 100)
		region_size = 100;
	mvwprintw(local_win, 6, 55, "%3d", region_size);

	wprintw(local_win, "\n\n  Starting year (default 0): ");
	char y_ch[8];
	int y = 0;
	wgetnstr(local_win, y_ch, 7);
	for (i = 0; i < strlen(y_ch); i++) {
		if (!isdigit(y_ch[i]))
			break;
	}
	if (strlen(y_ch) > 0)
		y = atoi(y_ch);
	wmove(local_win, 8, 29);
	wclrtoeol(local_win);
	mvwprintw(local_win, 8, 29, "%i", y);

	wprintw(local_win, "\n\n  Starting month (1-12, default 1): ");
	char m_ch[3];
	int m = 0;
	wgetnstr(local_win, m_ch, 2);
	for (i = 0; i < strlen(m_ch); i++) {
		if (!isdigit(m_ch[i]))
			break;
	}
	if (strlen(m_ch) > 0)
		m = atoi(m_ch);
	if (m > 12)
		m = 12;
	if (m > 0)
		m--;
	wmove(local_win, 10, 36);
	wclrtoeol(local_win);
	mvwprintw(local_win, 10, 36, "%s", months[m]);

	/* remove regions */
	while (world->regionlist != NULL)
		remove_region(world->regionlist);
	/* remove pieces */
	clear_piece_list();
	/* remove players */
	clear_player_list();
	/* remove tiles */
	remove_grid();

	/* use collected data to create world */
	world->grid = create_grid(h, w);

	if (region_size > 0) {
		voronoi();
	}

	world->current_time.tm_year = y;
	world->current_time.tm_mon = m;

	save_game();

	wprintw(local_win,
		"\n\n  Map created and saved. You can edit it now. Press any key.");
	curs_set(FALSE);
	noecho();
	wgetch(local_win);
	current_screen = MAP_EDITOR;
	return;
}

void players_dialog()
{
	WINDOW *local_win;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);

	wprintw(local_win, "  To add a player, press 'a'.\n");
	wprintw(local_win, "  To delete a player, press 'd'.\n");
	wprintw(local_win, "  To edit a player, press 'e'.\n");
	wprintw(local_win, "  To scroll, press up/down keys.\n");
	wprintw(local_win, "  To return to map editor, press 'q'.\n\n");

	int nr_players = count_players();
	int counter = 0;
	int section = 0;
	player_t *current = world->playerlist;
	int playerlist_selector =
	    get_player_order(get_player_by_id(world->selected_player));
	while (current != NULL) {
		section = playerlist_selector / 10;
		if (counter >= section * 10 && counter < section * 10 + 10
		    && counter < nr_players) {
			if (counter == playerlist_selector) {
				wattron(local_win, COLOR_PAIR(26));
				world->selected_player = current->id;
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
		current_screen = ADD_PLAYER_DIALOG;
		break;
	case 'd':
		counter = 0;
		current = world->playerlist;
		while (counter < playerlist_selector) {
			current = current->next;
			counter++;
		}
		if (current != NULL) {
			remove_player(current);
			if (playerlist_selector > 0) {
				playerlist_selector--;
				world->selected_player =
				    get_player_by_order(playerlist_selector)->
				    id;
			}
		}
		break;
	case 'e':
		current_screen = EDIT_PLAYER_DIALOG;
		break;
	case 1065:
		if (playerlist_selector > 0) {
			playerlist_selector--;
			world->selected_player =
			    get_player_by_order(playerlist_selector)->id;
		}
		break;
	case 1066:
		if (playerlist_selector < nr_players - 1) {
			playerlist_selector++;
			world->selected_player =
			    get_player_by_order(playerlist_selector)->id;
		}
		break;
	case 'q':
		playerlist_selector = 0;
		current_screen = MAP_EDITOR;
		break;
	default:
		break;
	}
	return;
}

void add_player_dialog()
{
	WINDOW *local_win;

	local_win = newwin(25, 80, 0, 0);
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
	char money_ch[4] = { 0 };
	unsigned char rank = 0;

	player_t *player = NULL;
	while (1) {
		switch (stage) {
		case 0:
			if (error == 1)
				mvwprintw(local_win, 2, 2,
					  "Name too short or not unique. Type another one:\n\n  ");
			else
				mvwprintw(local_win, 2, 2,
					  "Type a unique name for the new player:         \n\n  ");
			wgetnstr(local_win, name, 16);
			if (strlen(name) == 0) {
				current_screen = PLAYERS_DIALOG;
				return;
			}
			if (get_player_by_name(name) == NULL) {
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
					  "Choose player ranking (1-5 for knight, baron, count, duke or king), default is king:\n\n  ");
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
					  ranklist[rank]);
				stage = 2;
			}
			break;
		case 2:
			curs_set(TRUE);
			echo();
			if (error == 1)
				mvwprintw(local_win, 10, 2,
					  "Error. Try another number (0-999):       \n\n  ");
			else
				mvwprintw(local_win, 10, 2,
					  "Type the initial amount of money (0-999):\n\n  ");
			money_ch[0] = '\0';
			wgetnstr(local_win, money_ch, 3);
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
			player = add_player(name);
			set_money(player, money);
			set_player_rank(player, rank);
			world->selected_player = player->id;
			current_screen = PLAYERS_DIALOG;
			curs_set(FALSE);
			noecho();
			return;
			break;
		}
	}
	return;
}

void regions_dialog()
{
	WINDOW *local_win;

	local_win = newwin(25, 80, 0, 0);
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
	    get_region_order(get_region_by_id(world->selected_region));
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
			world->selected_region =
			    get_region_by_order(regionlist_selector)->id;
		}
		break;
	case 1066:
		if (regionlist_selector < nr_regions - 1) {
			regionlist_selector++;
			world->selected_region =
			    get_region_by_order(regionlist_selector)->id;
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
			if (regionlist_selector > 0) {
				regionlist_selector--;
				world->selected_region =
				    get_region_by_order(regionlist_selector)->
				    id;
			}
		}
		break;
	case 'e':
		current_screen = RENAME_REGION_DIALOG;
		return;
		break;
	case 'o':
		current_screen = REGION_PLAYER_DIALOG;
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

void add_region_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
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
				current_screen = PLAYERS_DIALOG;
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
			current_screen = REGIONS_DIALOG;
			return;
			break;
		}
	}
	return;
}

void region_to_player()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	region_t *region = get_region_by_id(world->selected_region);
	player_t *selected_player = get_player_by_id(world->selected_player);
	int playerlist_selector =
	    get_player_order(get_player_by_id(world->selected_player));

	while (1) {
		curs_set(FALSE);
		noecho();
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		mvwprintw(local_win, 2, 2,
			  "To give this region to a player, press Enter.\n");
		mvwprintw(local_win, 3, 2,
			  "To unset region owner, press 'd'.\n");
		mvwprintw(local_win, 4, 2, "To scroll, press up/down keys.\n");
		mvwprintw(local_win, 5, 2, "To return, press 'q'.\n\n");

		int nr_players = count_players();
		int counter = 0;
		int section = 0;
		player_t *current = world->playerlist;
		while (current != NULL) {
			section = playerlist_selector / 10;
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_players) {
				if (counter == playerlist_selector) {
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
			if (playerlist_selector > 0) {
				playerlist_selector--;
				counter = 0;
				current = world->playerlist;
				while (current != NULL) {
					if (counter == playerlist_selector) {
						selected_player = current;
						break;
					}
					counter++;
					current = current->next;
				}
			}
			break;
		case 1066:
			if (playerlist_selector < nr_players - 1) {
				playerlist_selector++;
				counter = 0;
				current = world->playerlist;
				while (current != NULL) {
					if (counter == playerlist_selector) {
						selected_player = current;
						break;
					}
					counter++;
					current = current->next;
				}
			}
			break;
		case 10:
			change_region_owner(selected_player, region);
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

void rename_region_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);
	curs_set(TRUE);
	echo();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	region_t *region = get_region_by_id(world->selected_region);
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
	current_screen = REGIONS_DIALOG;
	return;
}

void edit_player_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	player_t *active_player = get_player_by_id(world->selected_player);
	/* we can not get into this function if there's no selected_player, but still */
	if (active_player == NULL) {
		current_screen = PLAYERS_DIALOG;
		return;
	}
	mvwprintw(local_win, 2, 2, "Name: %s", active_player->name);
	mvwprintw(local_win, 3, 2, "Rank: %s", ranklist[active_player->rank]);
	mvwprintw(local_win, 4, 2, "Money: %d", active_player->money);
	mvwprintw(local_win, 5, 2, "Birthdate: %s of year %i",
		  months[active_player->birthdate.tm_mon],
		  active_player->birthdate.tm_year);
	mvwprintw(local_win, 6, 2, "Deathdate: %s of year %i",
		  months[active_player->deathdate.tm_mon],
		  active_player->deathdate.tm_year);
	mvwprintw(local_win, 7, 2, "Heir: %s",
		  (active_player->heir ==
		   NULL ? "none" : active_player->heir->name));
	mvwprintw(local_win, 8, 2, "Lord: %s",
		  (active_player->lord ==
		   NULL ? "none" : active_player->lord->name));

	mvwprintw(local_win, 10, 2, "To rename player, press 'r'");
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
			if (active_player->rank < KING)
				active_player->rank++;
			return;
			break;
		case '-':
			if (active_player->rank > KNIGHT)
				active_player->rank--;
			return;
			break;
		case 'b':
			current_screen = PLAYER_DATES_DIALOG;
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
			current_screen = PLAYER_MONEY_DIALOG;
			return;
			break;
		case 'q':
			current_screen = PLAYERS_DIALOG;
			return;
			break;
		case 'r':
			current_screen = RENAME_PLAYER_DIALOG;
			return;
			break;
		}
	}
}

void rename_player_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);
	curs_set(TRUE);
	echo();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	player_t *player = get_player_by_id(world->selected_player);

	while (1) {
		wmove(local_win, 3, 0);
		wclrtoeol(local_win);
		mvwprintw(local_win, 2, 2,
			  "Current name is '%s'. Type a new name, or press Enter to return.\n  New name: ",
			  player->name);
		char name[17] = { 0 };
		wgetnstr(local_win, name, 16);
		if (strlen(name) == 0) {
			current_screen = EDIT_PLAYER_DIALOG;
			return;
		}
		if (get_player_by_name(name) == NULL) {
			strcpy(player->name, name);
			current_screen = EDIT_PLAYER_DIALOG;
			return;
		}
		mvwprintw(local_win, 23, 2,
			  "Name not unique. Try another one.");
	}
}

void change_player_money_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);
	curs_set(TRUE);
	echo();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	int error = 0;
	player_t *player = get_player_by_id(world->selected_player);
	uint16_t money = 0;
	char money_ch[4] = { 0 };

	while (1) {
		wmove(local_win, 3, 0);
		wclrtoeol(local_win);
		mvwprintw(local_win, 2, 2,
			  "Current amount is '%d'. Type new amount (0-999), or press Enter to return.\n  New amount: ",
			  get_money(player));
		wgetnstr(local_win, money_ch, 3);
		if (strlen(money_ch) == 0) {
			current_screen = EDIT_PLAYER_DIALOG;
			return;
		}
		for (i = 0; i < strlen(money_ch); i++) {
			if (!isdigit(money_ch[i])) {
				error = 1;
			}
		}
		if (!error) {
			money = atoi(money_ch);
			set_money(player, money);
			current_screen = EDIT_PLAYER_DIALOG;
			return;
		}
		mvwprintw(local_win, 23, 2, "Error. Try another number.");
	}
}

void change_player_dates_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);
	curs_set(TRUE);
	echo();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	int error = 0;
	int stage = 1;
	player_t *player = get_player_by_id(world->selected_player);
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
					  months[player->birthdate.tm_mon],
					  player->birthdate.tm_year);
			wgetnstr(local_win, birthyear_ch, 3);
			if (strlen(birthyear_ch) == 0) {
				mvwprintw(local_win, 4, 8, "%d",
					  player->birthdate.tm_year);
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
				player->birthdate.tm_year = birthyear;
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
					  months[player->birthdate.tm_mon]);
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
			player->birthdate.tm_mon = birthmon - 1;
			mvwprintw(local_win, 4, 47, "%s",
				  months[player->birthdate.tm_mon]);
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
					  months[player->deathdate.tm_mon],
					  player->deathdate.tm_year);
			uint16_t deathyear = 0;
			char deathyear_ch[3] = { 0 };
			wgetnstr(local_win, deathyear_ch, 3);
			if (strlen(deathyear_ch) == 0) {
				mvwprintw(local_win, 8, 8, "%d",
					  player->deathdate.tm_year);
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
				player->deathdate.tm_year = deathyear;
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
					  months[player->deathdate.tm_mon]);
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
			player->deathdate.tm_mon = deathmon - 1;
			mvwprintw(local_win, 8, 47, "%s",
				  months[player->deathdate.tm_mon]);
			error = 0;
			stage = 5;
			break;
		case 5:
			curs_set(FALSE);
			noecho();
			mvwprintw(local_win, 23, 2,
				  "Changes saved. Press any key to continue.");
			wgetch(local_win);
			current_screen = EDIT_PLAYER_DIALOG;
			return;
		}
	}
}

void validate_dialog()
{
	WINDOW *local_win;

	char *msg;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);

	int i, j;
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

void edit_time_dialog()
{
	WINDOW *local_win;

	local_win = newwin(25, 80, 0, 0);
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

void successor_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);

	int i;
	player_t *active_player = get_player_by_id(world->selected_player);
	player_t *heir = active_player->heir;
	int playerlist_selector =
	    get_player_order(get_player_by_id(world->selected_player));

	while (1) {
		curs_set(FALSE);
		noecho();
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		mvwprintw(local_win, 2, 2, "To pick a player, press Enter.\n");
		mvwprintw(local_win, 3, 2, "To scroll, press up/down keys.\n");
		mvwprintw(local_win, 4, 2, "To remove heir, press 'd'.\n");
		mvwprintw(local_win, 5, 2, "To return, press 'q'.\n\n");

		int nr_players = count_players();
		int counter = 0;
		int section = 0;
		player_t *current = world->playerlist;
		while (current != NULL) {
			section = playerlist_selector / 10;
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_players) {
				if (counter == playerlist_selector) {
					wattron(local_win, COLOR_PAIR(26));
				} else
					wattron(local_win, COLOR_PAIR(1));
				mvwprintw(local_win, 8 + counter % 10, 2,
					  "%3d. %s", current->id,
					  current->name);
				if (current->id == world->selected_player)
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
			if (playerlist_selector > 0)
				playerlist_selector--;
			break;
		case 1066:
			if (playerlist_selector < nr_players - 1)
				playerlist_selector++;
			break;
		case 10:
			heir = get_player_by_order(playerlist_selector);
			if (heir->id != active_player->id)
				set_successor(active_player, heir);
			else
				heir = active_player->heir;
			break;
		case 'd':
			heir = NULL;
			set_successor(active_player, heir);
			break;
		case 'q':
			current_screen = EDIT_PLAYER_DIALOG;
			return;
			break;
		}
	}
}

void homage_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);

	int i;
	player_t *active_player = get_player_by_id(world->selected_player);
	player_t *selected_player = get_player_by_id(world->selected_player);
	int playerlist_selector =
	    get_player_order(get_player_by_id(world->selected_player));

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

		if (selected_player->id == active_player->id)
			mvwprintw(local_win, 2, 2,
				  "[You can't pay homage to yourself]\n");
		else if (selected_player->lord != NULL
			 && selected_player->lord->id == active_player->id)
			mvwprintw(local_win, 2, 2,
				  "[You can't pay homage to your own vassal]\n");
		else if (selected_player->rank <= KNIGHT)
			mvwprintw(local_win, 2, 2,
				  "[You can't pay homage to a knight]");
		else if (active_player->lord != NULL
			 && active_player->lord == selected_player) {
			mvwprintw(local_win, 2, 2, "To unset lord, press 'd'");
			unset_lord_ok = 1;
		} else {
			mvwprintw(local_win, 2, 2,
				  "To set lord, press Enter.\n");
			set_lord_ok = 1;
		}
		mvwprintw(local_win, 3, 2, "To scroll, press up/down keys.\n");
		mvwprintw(local_win, 4, 2, "To return, press 'q'.\n\n");

		int nr_players = count_players();
		int counter = 0;
		int section = 0;
		player_t *current = world->playerlist;
		while (current != NULL) {
			section = playerlist_selector / 10;
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_players) {
				if (counter == playerlist_selector) {
					wattron(local_win, COLOR_PAIR(26));
				} else
					wattron(local_win, COLOR_PAIR(1));
				mvwprintw(local_win, 8 + counter % 10, 2,
					  "%3d. %s", current->id,
					  current->name);
				if (current->id == active_player->id)
					wprintw(local_win, " (you)\n");
				else
					wprintw(local_win, " (%s)\n",
						ranklist[current->rank]);
				wattron(local_win, COLOR_PAIR(1));
			}
			current = current->next;
			counter++;
		}

		int user_move = get_input(local_win);
		switch (user_move) {
		case 1065:
			if (playerlist_selector > 0) {
				playerlist_selector--;
				counter = 0;
				current = world->playerlist;
				while (current != NULL) {
					if (counter == playerlist_selector) {
						selected_player = current;
						break;
					}
					counter++;
					current = current->next;
				}
			}
			break;
		case 1066:
			if (playerlist_selector < nr_players - 1) {
				playerlist_selector++;
				counter = 0;
				current = world->playerlist;
				while (current != NULL) {
					if (counter == playerlist_selector) {
						selected_player = current;
						break;
					}
					counter++;
					current = current->next;
				}
			}
			break;
		case 10:
			if (set_lord_ok) {
				active_player->lord = selected_player;
				current_screen = EDIT_PLAYER_DIALOG;
				return;
			}
			break;
		case 'd':
			if (unset_lord_ok) {
				active_player->lord = NULL;
				current_screen = EDIT_PLAYER_DIALOG;
				return;
			}
			break;
		case 'q':
			current_screen = EDIT_PLAYER_DIALOG;
			return;
			break;
		}
	}
}

void diplomacy_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);

	int i;
	player_t *active_player = get_player_by_id(world->selected_player);
	player_t *selected_player = get_player_by_id(world->selected_player);
	int playerlist_selector =
	    get_player_order(get_player_by_id(world->selected_player));

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

		if (active_player == selected_player) {
			dipstatus = NULL;
			status = 3;	/* self */
		} else {
			dipstatus =
			    get_dipstatus(active_player, selected_player);
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

		int nr_players = count_players();
		int counter = 0;
		int section = 0;
		player_t *current = world->playerlist;
		dipstatus_t *current_dipstatus = NULL;
		unsigned char current_status = 0;
		while (current != NULL) {
			if (active_player != current) {
				current_dipstatus =
				    get_dipstatus(active_player, current);
				current_status = current_dipstatus->status;
			}
			section = playerlist_selector / 10;
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_players) {
				if (counter == playerlist_selector) {
					wattron(local_win, COLOR_PAIR(26));
				} else
					wattron(local_win, COLOR_PAIR(1));
				mvwprintw(local_win, 8 + counter % 10, 2,
					  "%3d. %s (", current->id,
					  current->name);
				if (current->id == active_player->id)
					wprintw(local_win, "you");
				else {
					wprintw(local_win, "%s",
						dipstatuslist[current_status]);
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
			if (playerlist_selector > 0) {
				playerlist_selector--;
				counter = 0;
				current = world->playerlist;
				while (current != NULL) {
					if (counter == playerlist_selector) {
						selected_player = current;
						break;
					}
					counter++;
					current = current->next;
				}
			}
			break;
		case 1066:
			if (playerlist_selector < nr_players - 1) {
				playerlist_selector++;
				counter = 0;
				current = world->playerlist;
				while (current != NULL) {
					if (counter == playerlist_selector) {
						selected_player = current;
						break;
					}
					counter++;
					current = current->next;
				}
			}
			break;
		case 'a':	/* alliance */
			if (alliance_ok)
				set_diplomacy(active_player, selected_player,
					      ALLIANCE);
			break;
		case 'n':	/* neutral */
			if (neutral_ok)
				set_diplomacy(active_player, selected_player,
					      NEUTRAL);
			break;
		case 'q':	/* return */
			current_screen = EDIT_PLAYER_DIALOG;
			return;
			break;
		case 'w':	/* declare war */
			if (war_ok)
				set_diplomacy(active_player, selected_player,
					      WAR);
			break;
		}
	}
}

void help_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
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
	mvwprintw(local_win, 5, 2, "space -- select next player");
	mvwprintw(local_win, 6, 2, "'0-6' -- set moves left (roll dice)");
	mvwprintw(local_win, 7, 2, "'>' -- select next region");
	mvwprintw(local_win, 8, 2,
		  "'c' -- add tile to selected region or remove from its current region");
	mvwprintw(local_win, 9, 2, "'n' -- place a noble on current tile");
	mvwprintw(local_win, 10, 2, "'p' -- show players dialog");
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

	current_screen = 0;
	while (1) {
		switch (current_screen) {
		case MAIN_SCREEN:
			editor_start_menu();
			break;
		case NEW_GAME:
			clear();
			new_game_dialog();
			break;
		case MAP_EDITOR:
			if (world->grid == NULL)
				load_game();
			map_editor();
			break;
		case PLAYERS_DIALOG:
			clear();
			players_dialog();
			break;
		case ADD_PLAYER_DIALOG:
			clear();
			add_player_dialog();
			break;
		case REGIONS_DIALOG:
			regions_dialog();
			break;
		case ADD_REGION_DIALOG:
			add_region_dialog();
			break;
		case REGION_PLAYER_DIALOG:
			region_to_player();
			break;
		case GAME_TIME_DIALOG:
			edit_time_dialog();
			break;
		case RENAME_REGION_DIALOG:
			rename_region_dialog();
			break;
		case EDIT_PLAYER_DIALOG:
			edit_player_dialog();
			break;
		case RENAME_PLAYER_DIALOG:
			rename_player_dialog();
			break;
		case PLAYER_MONEY_DIALOG:
			change_player_money_dialog();
			break;
		case PLAYER_DATES_DIALOG:
			change_player_dates_dialog();
			break;
		case HEIR_DIALOG:
			successor_dialog();
			break;
		case FEUDAL_DIALOG:
			homage_dialog();
			break;
		case DIPLOMACY_DIALOG:
			diplomacy_dialog();
			break;
		case VALIDATE_DIALOG:
			validate_dialog();
			break;
		case HELP_DIALOG:
			help_dialog();
			break;
		case 99:
			destroy_world();
			endwin();
			return 0;
			break;
		}
	}
}
