#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>		//for isdigit

#include "world.h"

#define MOVE 0
#define VIEW 1

char *modes[] = {
	[MOVE] = "MOVE",
	[VIEW] = "VIEW",
};

int current_mode = VIEW;

int gameover = 0;

void draw_map()
{
	WINDOW *local_win;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);

	int i, j;
/**
	uint16_t h_multiplier = world->grid->cursor_height / 24;
	uint16_t w_multiplier = world->grid->cursor_width / 48;
**/
	int16_t h_offset = world->grid->cursor_height - 12;
	int16_t w_offset = world->grid->cursor_width - 24;

	player_t *player = get_player_by_id(world->selected_player);
	tile_t *tile =
	    world->grid->tiles[world->grid->cursor_height][world->grid->
							   cursor_width];
	piece_t *piece = tile->piece;

	char tile_char;
	int color_nr = 0;
	int player_age_mon = 0;
	for (i = 24 + h_offset; i > h_offset; i--) {
		for (j = w_offset; j < 48 + w_offset; j++) {
/**
	for (i = 24 * (h_multiplier + 1) - 1; i >= 24 * h_multiplier; i--) {
		if (i >= world->grid->height) {
			wprintw(local_win, "\n");
			continue;
		}
		for (j = 48 * w_multiplier; j < (48 * (w_multiplier + 1) < world->grid->width ? 48 * (w_multiplier + 1) : world->grid->width); j++) {
**/
/**
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
**/
			if (i < 0 || i >= world->grid->height || j < 0
			    || j >= world->grid->width) {
				color_nr = 1;
				tile_char = ' ';
			} else if (world->grid->tiles[i][j]->piece != NULL) {
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
				color_nr = 1;	//blue
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
	mvwprintw(local_win, 4, 50, "Player: %s (%s)", player->name,
		  ranklist[player->rank]);
	player_age_mon =
	    (world->current_time.tm_year - player->birthdate.tm_year) * 12 +
	    (world->current_time.tm_mon - player->birthdate.tm_mon);
	mvwprintw(local_win, 5, 50, "Age: %d year(s) %d month(s)",
		  player_age_mon / 12, player_age_mon % 12);
	mvwprintw(local_win, 6, 50, "Money: %d (#%d)", player->money,
		  player->rank_money);
	mvwprintw(local_win, 7, 50, "Army: %d (#%d)",
		  count_pieces_by_owner(player), player->rank_army);
	mvwprintw(local_win, 8, 50, "Land: %d (#%d)",
		  count_tiles_by_owner(player), player->rank_land);
	mvwprintw(local_win, 9, 50, "Heir: %s",
		  (player->heir != NULL ? player->heir->name : "none"));
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
	mvwprintw(local_win, 19, 50, "Diplomacy: ");
	if (piece != NULL && piece->owner->id != player->id) {
		dipstatus_t *diplomacy = get_dipstatus(piece->owner, player);
		switch (diplomacy->status) {
		case NEUTRAL:
			wcolor_set(local_win, 12, NULL);
			break;
		case ALLIANCE:
			wcolor_set(local_win, 11, NULL);
			break;
		case WAR:
			wcolor_set(local_win, 10, NULL);
			break;
		}
		wprintw(local_win, "%s", dipstatuslist[diplomacy->status]);
		if (diplomacy->pending_offer != NULL)
			wprintw(local_win, " *");
		wcolor_set(local_win, 1, NULL);
	}

	int message_len = strlen(message);
	if (message_len > 0) {
		for (i = 0; i < (message_len - 1) / 29 + 1; i++) {
			mvwprintw(local_win, 20 + i, 50, "%.*s", 29,
				  &message[29 * i]);
		}
	}

	mvwprintw(local_win, 23, 50, "Mode: %s\n", modes[current_mode]);
	mvwprintw(local_win, 23, 65, "Help: '?'");

	int ch;
	ch = get_input(local_win);
	unsigned char result;
	switch (ch) {
	case 1065:		//up
		if (current_mode == VIEW) {
			set_cursor(world->grid->cursor_height + 1,
				   world->grid->cursor_width);
		} else {
			move_piece(piece, world->grid->cursor_height + 1,
				   world->grid->cursor_width);
			check_death();
		}
		break;
	case 1066:		//down
		if (current_mode == VIEW) {
			set_cursor(world->grid->cursor_height - 1,
				   world->grid->cursor_width);
		} else {
			move_piece(piece, world->grid->cursor_height - 1,
				   world->grid->cursor_width);
			check_death();
		}
		break;
	case 1067:		//right
		if (current_mode == VIEW) {
			set_cursor(world->grid->cursor_height,
				   world->grid->cursor_width + 1);
		} else {
			move_piece(piece, world->grid->cursor_height,
				   world->grid->cursor_width + 1);
			check_death();
		}
		break;
	case 1068:		//left
		if (current_mode == VIEW) {
			set_cursor(world->grid->cursor_height,
				   world->grid->cursor_width - 1);
		} else {
			move_piece(piece, world->grid->cursor_height,
				   world->grid->cursor_width - 1);
			check_death();
		}
		break;
	case ' ':		//SPACE end turn
		gameover = is_gameover();
		if (gameover) {
			current_screen = GAME_OVER;
			break;
		}
/**
			else {
				current_screen = 99;
				break;
			}
**/
		world->moves_left = get_dice();
		if (player->next != NULL)
			player = player->next;
		else {
			/* start the next round and change game date */
			player = world->playerlist;
			increment_gametime();
		}
		world->selected_player = player->id;
		piece = get_noble_by_owner(player);
		world->grid->cursor_height = piece->height;
		world->grid->cursor_width = piece->width;
		current_mode = VIEW;
		break;
	case '\t':		// loop through own pieces
		if (piece != NULL) {
			piece_t *active_piece = next_piece(piece);
			world->grid->cursor_height = active_piece->height;
			world->grid->cursor_width = active_piece->width;
		}
		break;
	case 'c':		// claim a region
		/* first, switch to noble */
		piece = get_noble_by_owner(player);
		/* set cursor to noble */
		set_cursor(piece->height, piece->width);
		result = claim_region(player, tile->region);
		switch (result) {
		case 1:	/* claimed from nature */
			add_to_cronicle("%s %s claimed %s.\n",
					ranklist[player->rank], player->name,
					tile->region->name);
			update_land_ranking();
			break;
		case 2:	/* conquered from enemy */
			add_to_cronicle("%s %s conquered %s.\n",
					ranklist[player->rank], player->name,
					tile->region->name);
			check_death();
			update_land_ranking();
			break;
		default:	/* nothing changed */
			break;
		}
		break;
	case 'd':		// diplomacy list
		current_screen = DIPLOMACY_DIALOG;
		break;
	case 'f':		// feudal stuff
		current_screen = FEUDAL_DIALOG;
		break;
	case 'i':		// game information
		current_screen = INFORMATION;
		break;
	case 'h':		// name a successor
		current_screen = HEIR_DIALOG;
		break;
	case 'm':		// give money
		/* check if we have money */
		if (player->money > 0)
			current_screen = GIVE_MONEY_DIALOG;
		break;
	case 'q':		// quit
		curs_set(TRUE);
		echo();
		current_screen = 99;
		break;
	case 'r':		// give region
		current_screen = REGIONS_DIALOG;
		break;
	case 's':		// save game
		save_game();
		break;
	case 't':		// take money (when 6)
		if (world->moves_left == 6) {
			player->money++;
			world->moves_left = 0;
			update_money_ranking();
		}
		break;
	case 'u':		//place a soldier -- dialog
			/**
			 * check if in view mode
			 * check if region is our (not free, first of all)
			 * check if enough money
			 * check if walkable
			 * check if empty
			**/
		if (current_mode == VIEW && tile->region != NULL
		    && tile->region->owner != NULL
		    && tile->region->owner->id == world->selected_player
		    && player->money >= COST_SOLDIER && tile->walkable
		    && piece == NULL) {
			add_piece(1, world->grid->cursor_height,
				  world->grid->cursor_width, player);
			set_money(player, get_money(player) - COST_SOLDIER);
			update_money_ranking();
			update_army_ranking();
		}
		break;
	case 'v':		// toggle between 'move piece' and 'explore map'
		current_mode = (current_mode + 1) % 2;	/* 0->1, 1->0 */
		if (current_mode == 0) {
			piece =
			    get_noble_by_owner(get_player_by_id
					       (world->selected_player));
			world->grid->cursor_height = piece->height;
			world->grid->cursor_width = piece->width;
		}
		break;
	case '?':
		current_screen = HELP_DIALOG;
		break;
	}
	return;
}

void regions_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);

	region_t *current_region = world->regionlist;
	player_t *active_player = get_player_by_id(world->selected_player);

	int nr_regions = count_regions_by_owner(active_player);

	if (nr_regions > 1)
		wprintw(local_win,
			"  To give a region to another player, press 'g'.\n");
	else
		wprintw(local_win,
			" [You can't give away your last region.]\n");
	wprintw(local_win, "  To rename a region, press 'e'.\n");
	wprintw(local_win, "  To scroll, press up/down keys.\n");
	wprintw(local_win, "  To return to map, press 'q'.\n\n");

	/**
	 * if selected region is not owned by selected player,
	 * set to first region owned by active player
	**/
	if (get_region_by_id(world->selected_region) == NULL
	    || get_region_by_id(world->selected_region)->owner == NULL
	    || get_region_by_id(world->selected_region)->owner->id !=
	    world->selected_player) {
		current_region = world->regionlist;
		while (current_region != NULL) {
			if (current_region->owner != NULL
			    && current_region->owner->id ==
			    world->selected_player) {
				world->selected_region = current_region->id;
				break;
			}
			current_region = current_region->next;
		}
	}

	int counter = 0;
	int section = 0;
	current_region = world->regionlist;
	int regionlist_selector = 0;
	while (current_region != NULL) {
		if (current_region->owner != NULL
		    && current_region->owner->id == world->selected_player) {
			if (current_region->id == world->selected_region)
				break;
			else
				regionlist_selector++;
		}
		current_region = current_region->next;
	}

	current_region = world->regionlist;
	section = regionlist_selector / 10;
	while (current_region != NULL) {
		if (current_region->owner == NULL
		    || current_region->owner->id != world->selected_player) {
			current_region = current_region->next;
			;
			continue;
		}
		if (counter >= section * 10 && counter < section * 10 + 10
		    && counter < nr_regions) {
			if (counter == regionlist_selector) {
				wattron(local_win, COLOR_PAIR(26));
//                              world->selected_region = current_region->id;
			} else
				wattron(local_win, COLOR_PAIR(1));
			mvwprintw(local_win, 8 + counter % 10, 2,
				  "%3d. %s (%i tiles)", current_region->id,
				  current_region->name, current_region->size);
			wattron(local_win, COLOR_PAIR(1));
		}
		counter++;
		current_region = current_region->next;
	}

	int user_move = get_input(local_win);
	switch (user_move) {
	case 1065:
		if (regionlist_selector > 0) {
			regionlist_selector--;
			counter = 0;
			current_region = world->regionlist;
			while (current_region != NULL) {
				if (current_region->owner != NULL
				    && current_region->owner->id ==
				    world->selected_player) {
					if (counter == regionlist_selector) {
						world->selected_region =
						    current_region->id;
						break;
					}
					counter++;
				}
				current_region = current_region->next;
			}
		}
		break;
	case 1066:
		if (regionlist_selector < nr_regions - 1) {
			regionlist_selector++;
			counter = 0;
			current_region = world->regionlist;
			while (current_region != NULL) {
				if (current_region->owner != NULL
				    && current_region->owner->id ==
				    world->selected_player) {
					if (counter == regionlist_selector) {
						world->selected_region =
						    current_region->id;
						break;
					}
					counter++;
				}
				current_region = current_region->next;
			}
		}
		break;
	case 'e':
		current_screen = EDIT_REGION_DIALOG;
		break;
	case 'g':		/* give to another player */
		current_screen = GIVE_REGION_DIALOG;
		break;
	case 'q':		/* return to map */
		current_screen = MAIN_SCREEN;
		break;
	default:
		break;
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

	int stage = 0;
	int error = 0;
	region_t *region = get_region_by_id(world->selected_region);
	char name[17] = { 0 };
	while (1) {
		switch (stage) {
		case 0:
			if (error == 1)
				mvwprintw(local_win, 2, 2,
					  "New name is not unique. Type another one:\n\n  ");
			else
				mvwprintw(local_win, 2, 2,
					  "Type a new name for the region '%s', or press Enter to return:\n\n  ",
					  region->name);
			wgetnstr(local_win, name, 16);
			if (strlen(name) == 0) {
				current_screen = REGIONS_DIALOG;
				return;
			}
			if (get_region_by_name(name) == NULL) {
				stage = 1;
				error = 0;
			} else
				error = 1;
			break;
		case 1:
			strcpy(region->name, name);
			current_screen = REGIONS_DIALOG;
			return;
			break;
		}
	}
	return;
}

void give_region_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	player_t *active_player = get_player_by_id(world->selected_player);
	player_t *selected_player = get_player_by_id(world->selected_player);
	int playerlist_selector = get_player_order(selected_player);
	region_t *region = get_region_by_id(world->selected_region);
	int give_region_ok = 0;

	while (1) {
		give_region_ok = 0;
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s", screens[current_screen]);

		if (selected_player == active_player) {
			mvwprintw(local_win, 2, 2,
				  "[You can not give region to yourself]");
			give_region_ok = 0;
		} else {
			mvwprintw(local_win, 2, 2,
				  "To give %s to selected player, press Enter.",
				  region->name);
			give_region_ok = 1;
		}
		mvwprintw(local_win, 3, 2, "To scroll, press up/down keys.");
		mvwprintw(local_win, 4, 2, "To return, press 'q'.");

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
				if (current == active_player)
					wprintw(local_win, " (you)");
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
				selected_player =
				    get_player_by_order(playerlist_selector);
			}
			break;
		case 1066:
			if (playerlist_selector < nr_players - 1) {
				playerlist_selector++;
				selected_player =
				    get_player_by_order(playerlist_selector);
			}
			break;
		case 10:
			if (give_region_ok) {
				change_region_owner(selected_player, region);
				add_to_cronicle("%s %s granted %s to %s %s.\n",
						ranklist[active_player->rank],
						active_player->name,
						region->name,
						ranklist[selected_player->rank],
						selected_player->name);
				update_land_ranking();
				current_screen = REGIONS_DIALOG;
				return;
			}
			break;
		case 'q':
			current_screen = REGIONS_DIALOG;
			return;
			break;
		}
	}
	return;
}

void give_money_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);

	int i;
	int stage = 0;
	int error = 0;
	player_t *active_player = get_player_by_id(world->selected_player);
	player_t *receiving_player = NULL;
	int playerlist_selector;
	uint16_t money = 0;
	char money_ch[4] = { 0 };

	while (1) {
		switch (stage) {
		case 0:
			curs_set(TRUE);
			echo();
			wclear(local_win);
			for (i = 0;
			     i < (80 - strlen(screens[current_screen])) / 2;
			     i++)
				wprintw(local_win, " ");
			wprintw(local_win, "%s\n\n", screens[current_screen]);

			if (error == 1)
				wprintw(local_win,
					"  Error. Try another number (1-%d):\n\n  ",
					active_player->money);
			else
				wprintw(local_win,
					"  Type the amount to be given (1-%d), or press Enter to dismiss:\n\n  ",
					active_player->money);
			wgetnstr(local_win, money_ch, 3);
			if (strlen(money_ch) == 0) {
				current_screen = MAIN_SCREEN;
				return;
				break;
			}
			for (i = 0; i < strlen(money_ch); i++) {
				if (!isdigit(money_ch[i])) {
					error = 1;
					break;
				}
			}
			money = atoi(money_ch);
			if (money < 1 || money > active_player->money) {
				error = 1;
				break;
			}
			stage = 1;
			error = 0;
			break;
		case 1:
			playerlist_selector =
			    get_player_order(get_player_by_id
					     (world->selected_player));
			while (stage == 1) {
				curs_set(FALSE);
				noecho();
				wclear(local_win);
				for (i = 0;
				     i <
				     (80 - strlen(screens[current_screen])) / 2;
				     i++)
					wprintw(local_win, " ");
				wprintw(local_win, "%s\n\n",
					screens[current_screen]);

				wprintw(local_win,
					"  To pick a player, press Enter.\n");
				wprintw(local_win,
					"  To scroll, press up/down keys.\n");
				wprintw(local_win,
					"  To return, press 'esc'.\n\n");

				int nr_players = count_players();
				int counter = 0;
				int section = 0;
				player_t *current = world->playerlist;
				while (current != NULL) {
					section = playerlist_selector / 10;
					if (counter >= section * 10
					    && counter < section * 10 + 10
					    && counter < nr_players) {
						if (counter ==
						    playerlist_selector) {
							wattron(local_win,
								COLOR_PAIR(26));
						} else
							wattron(local_win,
								COLOR_PAIR(1));
						mvwprintw(local_win,
							  8 + counter % 10, 2,
							  "%3d. %s",
							  current->id,
							  current->name);
						if (current->id ==
						    world->selected_player)
							wprintw(local_win,
								" (you)\n");
						else
							wprintw(local_win,
								"\n");
						wattron(local_win,
							COLOR_PAIR(1));
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
					if (playerlist_selector <
					    nr_players - 1)
						playerlist_selector++;
					break;
				case 10:
					receiving_player =
					    get_player_by_order
					    (playerlist_selector);
					error = 0;
					stage = 2;
					break;
				case 27:
					current_screen = MAIN_SCREEN;
					return;
					break;
				}
			}
			break;
		case 2:
			set_money(active_player,
				  get_money(active_player) - money);
			set_money(receiving_player,
				  get_money(receiving_player) + money);
			update_money_ranking();
			current_screen = MAIN_SCREEN;
			return;
			break;
		}
	}
}

void info_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	int section = 0;
	int nr_players = count_players();

	while (1) {
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);
		wprintw(local_win, "  To scroll, press up/down keys.\n");
		wprintw(local_win, "  To return, press 'q'.\n\n");

		mvwprintw(local_win, 6, 2, " Nr.");
		mvwprintw(local_win, 6, 7, "Player Name");
		mvwprintw(local_win, 6, 44, "Money");
		mvwprintw(local_win, 6, 54, "Army");
		mvwprintw(local_win, 6, 64, "Land");

		int counter = 0;
		player_t *current = world->playerlist;
		while (current != NULL) {
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_players) {
				mvwprintw(local_win, 8 + counter % 10, 2,
					  "%3d. %s", current->id,
					  current->name);
				mvwprintw(local_win, 8 + counter % 10, 42,
					  "%3d (#%d)", current->money,
					  current->rank_money);
				mvwprintw(local_win, 8 + counter % 10, 52,
					  "%3d (#%d)",
					  count_pieces_by_owner(current),
					  current->rank_army);
				mvwprintw(local_win, 8 + counter % 10, 62,
					  "%3d (#%d)",
					  count_tiles_by_owner(current),
					  current->rank_land);
			}
			current = current->next;
			counter++;
		}

		int user_move = get_input(local_win);
		switch (user_move) {
		case 1065:
			if (section > 0)
				section--;
			break;
		case 1066:
			if (section < (nr_players - 1) / 10)
				section++;
			break;
		case 'q':
			current_screen = MAIN_SCREEN;
			return;
			break;
		}
	}
	current_screen = MAIN_SCREEN;
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
			current_screen = MAIN_SCREEN;
			return;
			break;
		}
	}
}

void feudal_dialog()
{
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);

	int i;
	player_t *active_player = get_player_by_id(world->selected_player);
//      piece_t *active_player_noble = get_noble_by_owner(active_player);
	player_t *lord = active_player->lord;
	tile_t *current_tile =
	    world->grid->tiles[world->grid->cursor_height][world->grid->
							   cursor_width];
	piece_t *current_piece = current_tile->piece;
	player_t *selected_player = NULL;
	int playerlist_selector = 0;
	int create_vassal_ok = 0;
	int promote_vassal_ok = 0;

	while (1) {
		create_vassal_ok = 0;
		promote_vassal_ok = 0;
		curs_set(FALSE);
		noecho();
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		int nr_vassals = count_vassals(active_player);

		if (lord != NULL)
			mvwprintw(local_win, 2, 2, "Your lord is %s.",
				  lord->name);
		else
			mvwprintw(local_win, 2, 2, "To pay homage, press 'h'.");
		mvwprintw(local_win, 3, 2, "To scroll, press up/down keys.");
		if (nr_vassals == 0)
			mvwprintw(local_win, 4, 2,
				  "[You have no vassals to free]");
		else
			mvwprintw(local_win, 4, 2,
				  "To free a vassal, press 'f'.");
		if (nr_vassals == 0)
			mvwprintw(local_win, 5, 2,
				  "[You have no vassals to promote]");
		else if (get_money(active_player) < COST_SOLDIER)
			mvwprintw(local_win, 5, 2,
				  "[Can't promote a vassal. Not enough money.]");
		else if (selected_player != NULL
			 && active_player->rank - selected_player->rank <= 1)
			mvwprintw(local_win, 5, 2,
				  "[Can't promote a vassal. He is one rank below you.]");
		else {
			mvwprintw(local_win, 5, 2,
				  "To promote a vassal, press 'p'.");
			promote_vassal_ok = 1;
		}
		if (active_player->rank <= KNIGHT)
			mvwprintw(local_win, 6, 2,
				  "[Can't create vassal. Your rank is too low.]");
		else if (count_regions_by_owner(active_player) < 2)
			mvwprintw(local_win, 6, 2,
				  "[Can't create vassal. Too few regions.]");
		else if (get_money(active_player) < COST_SOLDIER)
			mvwprintw(local_win, 6, 2,
				  "[Can't create vassal. Not enough money.]");
		else if (current_piece == NULL)
			mvwprintw(local_win, 6, 2,
				  "[Can't create vassal. No piece selected.]");
		else if (current_piece->owner->id != active_player->id)
			mvwprintw(local_win, 6, 2,
				  "[Can't create vassal. Selected piece not yours.]");
		else if (current_piece->type != 1)
			mvwprintw(local_win, 6, 2,
				  "[Can't create vassal. Selected piece not soldier.]");
		else {
			mvwprintw(local_win, 6, 2,
				  "To create a new vassal, press 'v'");
			create_vassal_ok = 1;
		}
		mvwprintw(local_win, 7, 2, "To return, press 'q'.");

		int counter = 0;
		int section = 0;
		player_t *current = world->playerlist;
		while (current != NULL) {
			section = playerlist_selector / 10;
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_vassals) {
				if (counter == playerlist_selector) {
					wattron(local_win, COLOR_PAIR(26));
					selected_player = current;
				} else
					wattron(local_win, COLOR_PAIR(1));
				if (current->lord != NULL
				    && current->lord->id == active_player->id) {
					mvwprintw(local_win, 9 + counter % 10,
						  2, "%3d. %s (%s)",
						  current->id, current->name,
						  ranklist[current->rank]);
					counter++;
				}
				wattron(local_win, COLOR_PAIR(1));
			}
			current = current->next;
		}

		int user_move = get_input(local_win);
		switch (user_move) {
		case 1065:
			if (playerlist_selector > 0)
				playerlist_selector--;
			break;
		case 1066:
			if (playerlist_selector < nr_vassals - 1)
				playerlist_selector++;
			break;
		case 'h':
			if (lord == NULL)
				current_screen = HOMAGE_DIALOG;
			return;
			break;
		case 'f':
			if (selected_player != NULL) {
				selected_player->lord = NULL;
				add_to_cronicle
				    ("%s %s became a sovereign of his lands.\n",
				     ranklist[selected_player->rank],
				     selected_player->name);
				playerlist_selector = 0;
				selected_player = NULL;
			}
			break;
		case 'p':
			if (promote_vassal_ok) {
				set_money(active_player,
					  get_money(active_player) -
					  COST_SOLDIER);
				set_player_rank(selected_player,
						get_player_rank(selected_player)
						+ 1);
				add_to_cronicle
				    ("%s %s bestowed the %s title upon their vassal %s.\n",
				     ranklist[active_player->rank],
				     active_player->name,
				     ranklist[selected_player->rank],
				     selected_player->name);
			}
			break;
		case 'q':
			current_screen = MAIN_SCREEN;
			return;
			break;
		case 'v':
			if (create_vassal_ok) {
				current_screen = PROMOTE_SOLDIER_DIALOG;
				return;
			}
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

	int pay_homage_ok = 0;

	while (1) {
		pay_homage_ok = 0;

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
				  "[You can't pay homage to a knight]\n");
		else {
			mvwprintw(local_win, 2, 2,
				  "To pay homage, press Enter.\n");
			pay_homage_ok = 1;
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
			if (pay_homage_ok) {
				homage(active_player, selected_player);
				current_screen = FEUDAL_DIALOG;
				return;
			}
			break;
		case 'q':
			current_screen = MAIN_SCREEN;
			return;
			break;
		}
	}
}

void promote_soldier_dialog()
{
	/* replace currently selected piece with a new player */
	WINDOW *local_win = NULL;

	local_win = newwin(25, 80, 0, 0);
	wattrset(local_win, A_BOLD);

	int i;
	player_t *active_player = get_player_by_id(world->selected_player);
	player_t *new_vassal = NULL;
	tile_t *current_tile =
	    world->grid->tiles[world->grid->cursor_height][world->grid->
							   cursor_width];
	piece_t *current_piece = current_tile->piece;	/* assumed to be our soldier */

	/* set world->selected_region to our region */
	region_t *current_region = world->regionlist;
	while (current_region != NULL) {
		if (current_region->owner != NULL
		    && current_region->owner->id == world->selected_player) {
			world->selected_region = current_region->id;
			break;
		}
		current_region = current_region->next;
	}

	int stage = 0;
	int error = 0;
	char name[17] = { 0 };
	int nr_regions = count_regions_by_owner(active_player);
	while (1) {
		switch (stage) {
		case 0:
			curs_set(true);
			echo();
			if (error == 1)
				mvwprintw(local_win, 2, 2,
					  "Name is not unique. Type another one:\n\n  ");
			else
				mvwprintw(local_win, 2, 2,
					  "Type a name for new vassal, or press Enter to return:\n\n  ");
			wgetnstr(local_win, name, 16);
			if (strlen(name) == 0) {
				current_screen = FEUDAL_DIALOG;
				return;
			}
			if (get_player_by_name(name) == NULL) {
				stage = 1;
				error = 0;
			} else
				error = 1;
			break;
		case 1:
			curs_set(false);
			noecho();
			wclear(local_win);
			for (i = 0;
			     i < (80 - strlen(screens[current_screen])) / 2;
			     i++)
				wprintw(local_win, " ");
			wprintw(local_win, "%s\n\n", screens[current_screen]);

			mvwprintw(local_win, 2, 2,
				  "To pick a region for new vassal, press Enter.");
			mvwprintw(local_win, 3, 2,
				  "To scroll, press up/down keys.");
			mvwprintw(local_win, 4, 2, "To return, press 'q'.");

			int counter = 0;
			int section = 0;
			current_region = world->regionlist;
			int regionlist_selector = 0;
			while (current_region != NULL) {
				if (current_region->owner != NULL
				    && current_region->owner->id ==
				    world->selected_player) {
					if (current_region->id ==
					    world->selected_region)
						break;
					else
						regionlist_selector++;
				}
				current_region = current_region->next;
			}

			current_region = world->regionlist;
			section = regionlist_selector / 10;

			while (current_region != NULL) {
				if (current_region->owner == NULL
				    || current_region->owner->id !=
				    world->selected_player) {
					current_region = current_region->next;
					;
					continue;
				}
				if (counter >= section * 10
				    && counter < section * 10 + 10
				    && counter < nr_regions) {
					if (counter == regionlist_selector) {
						wattron(local_win,
							COLOR_PAIR(26));
						//                              world->selected_region = current_region->id;
					} else
						wattron(local_win,
							COLOR_PAIR(1));
					mvwprintw(local_win, 8 + counter % 10,
						  2, "%3d. %s (%i tiles)",
						  current_region->id,
						  current_region->name,
						  current_region->size);
					wattron(local_win, COLOR_PAIR(1));
				}
				counter++;
				current_region = current_region->next;
			}

			int user_move = get_input(local_win);
			switch (user_move) {
			case 1065:
				if (regionlist_selector > 0) {
					regionlist_selector--;
					counter = 0;
					current_region = world->regionlist;
					while (current_region != NULL) {
						if (current_region->owner !=
						    NULL
						    && current_region->owner->
						    id ==
						    world->selected_player) {
							if (counter ==
							    regionlist_selector)
							{
								world->
								    selected_region
								    =
								    current_region->
								    id;
								break;
							}
							counter++;
						}
						current_region =
						    current_region->next;
					}
				}
				break;
			case 1066:
				if (regionlist_selector < nr_regions - 1) {
					regionlist_selector++;
					counter = 0;
					current_region = world->regionlist;
					while (current_region != NULL) {
						if (current_region->owner !=
						    NULL
						    && current_region->owner->
						    id ==
						    world->selected_player) {
							if (counter ==
							    regionlist_selector)
							{
								world->
								    selected_region
								    =
								    current_region->
								    id;
								break;
							}
							counter++;
						}
						current_region =
						    current_region->next;
					}
				}
				break;
			case 'q':	/* return to map */
				current_screen = FEUDAL_DIALOG;
				return;
				break;
			case 10:
				new_vassal = add_player(name);
				current_piece->type = NOBLE;
				set_player_rank(new_vassal, KNIGHT);
				current_piece->owner = new_vassal;
				homage(new_vassal, active_player);
				current_region =
				    get_region_by_id(world->selected_region);
				change_region_owner(new_vassal, current_region);
				add_to_cronicle
				    ("%s %s granted %s to their new vassal, %s %s.\n",
				     ranklist[active_player->rank],
				     active_player->name, current_region->name,
				     ranklist[new_vassal->rank],
				     new_vassal->name);
				current_screen = FEUDAL_DIALOG;
				return;
				break;
			default:
				break;
			}
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

	unsigned char offer_alliance_ok, quit_alliance_ok, offer_peace_ok,
	    declare_war_ok, accept_offer_ok, reject_offer_ok, retract_offer_ok;

	while (1) {
		dipstatus_t *dipstatus = NULL;
		unsigned char status = 0;
		dipoffer_t *dipoffer = NULL;
		offer_alliance_ok = 0;	/* a */
		quit_alliance_ok = 0;	/* x */
		offer_peace_ok = 0;	/* p */
		declare_war_ok = 0;	/* w */
		accept_offer_ok = 0;	/* y */
		reject_offer_ok = 0;	/* n */
		retract_offer_ok = 0;	/* r */

		curs_set(FALSE);
		noecho();
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		if (active_player == selected_player) {
			dipstatus = NULL;
			status = 3;	/* self */
			dipoffer = NULL;
		} else {
			dipstatus =
			    get_dipstatus(active_player, selected_player);
			status = dipstatus->status;
			dipoffer = dipstatus->pending_offer;
		}
		switch (status) {
		case NEUTRAL:
			if (dipoffer != NULL) {
				if (dipoffer->from == selected_player) {
					/* accept or reject */
					mvwprintw(local_win, 2, 2,
						  "To accept alliance offer, press 'y'");
					accept_offer_ok = 1;
					mvwprintw(local_win, 3, 2,
						  "To reject alliance offer, press 'n'");
					reject_offer_ok = 1;
				} else {
					/* retract own offer */
					mvwprintw(local_win, 2, 2,
						  "To retract your alliance offer, press 'r'");
					retract_offer_ok = 1;
					mvwprintw(local_win, 3, 2,
						  "[You can not declare war with a pending alliance offer]");
				}
			} else {
				mvwprintw(local_win, 2, 2,
					  "To offer alliance, press 'a'");
				offer_alliance_ok = 1;
				mvwprintw(local_win, 3, 2,
					  "To declare war, press 'w'");
				declare_war_ok = 1;
			}
			break;
		case ALLIANCE:
			if ((active_player->lord != NULL
			     && active_player->lord == selected_player)
			    || (selected_player->lord != NULL
				&& selected_player->lord == active_player))
				mvwprintw(local_win, 2, 2,
					  "[You can not break alliance with your lord or vassal]");
			else {
				mvwprintw(local_win, 2, 2,
					  "To quit alliance, press 'x'");
				quit_alliance_ok = 1;
			}
			mvwprintw(local_win, 3, 2,
				  "[You can not declare war, first quit alliance]");
			break;
		case WAR:
			mvwprintw(local_win, 2, 2,
				  "[You can not make alliance, first negotiate peace]");
			if (dipoffer != NULL) {
				if (dipoffer->from == selected_player) {
					/* accept or reject */
					mvwprintw(local_win, 3, 2,
						  "To accept peace offer, press 'y'");
					accept_offer_ok = 1;
				} else {
					/* retract own offer */
					mvwprintw(local_win, 3, 2,
						  "To retract your peace offer, press 'r'");
					retract_offer_ok = 1;
				}
			} else {
				mvwprintw(local_win, 3, 2,
					  "To offer peace, press 'p'");
				offer_peace_ok = 1;
			}
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
		dipoffer_t *current_dipoffer = NULL;
		while (current != NULL) {
			if (active_player != current) {
				current_dipstatus =
				    get_dipstatus(active_player, current);
				current_status = current_dipstatus->status;
				current_dipoffer =
				    current_dipstatus->pending_offer;
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
					if (current_dipoffer != NULL)
						wprintw(local_win,
							", %s offer %s",
							dipstatuslist
							[current_dipoffer->
							 offer],
							(current_dipoffer->
							 from ==
							 active_player ? "sent"
							 : "received"));
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
		case 'a':	/* offer alliance */
			if (offer_alliance_ok) {
				open_offer(active_player, selected_player,
					   ALLIANCE);
				dipstatus =
				    get_dipstatus(active_player,
						  selected_player);
			}
			break;
		case 'n':	/* reject offer */
			if (reject_offer_ok)
				close_offer(dipstatus->pending_offer, REJECT);
			break;
		case 'p':	/* offer peace */
			if (offer_peace_ok) {
				open_offer(active_player, selected_player,
					   NEUTRAL);
				dipstatus =
				    get_dipstatus(active_player,
						  selected_player);
			}
			break;
		case 'q':	/* return */
			current_screen = MAIN_SCREEN;
			return;
			break;
		case 'r':	/* retract offer */
			if (retract_offer_ok)
				close_offer(dipstatus->pending_offer, REJECT);
			break;
		case 'w':	/* declare war */
			if (declare_war_ok) {
				set_diplomacy(active_player, selected_player,
					      WAR);
				add_to_cronicle
				    ("%s %s declared war on %s %s.\n",
				     ranklist[active_player->rank],
				     active_player->name,
				     ranklist[selected_player->rank],
				     selected_player->name);
			}
			break;
		case 'x':	/* quit alliance */
			if (quit_alliance_ok) {
				set_diplomacy(active_player, selected_player,
					      NEUTRAL);
				add_to_cronicle
				    ("%s %s broke their alliance with %s %s.\n",
				     ranklist[active_player->rank],
				     active_player->name,
				     ranklist[selected_player->rank],
				     selected_player->name);
			}
			break;
		case 'y':	/* accept offer */
			if (accept_offer_ok) {
				close_offer(dipstatus->pending_offer, ACCEPT);
				if (dipstatus->status == ALLIANCE)
					add_to_cronicle
					    ("%s %s and %s %s concluded an alliance.\n",
					     ranklist[active_player->rank],
					     active_player->name,
					     ranklist[selected_player->rank],
					     selected_player->name);
				else
					add_to_cronicle
					    ("%s %s and %s %s signed a peace treaty.\n",
					     ranklist[active_player->rank],
					     active_player->name,
					     ranklist[selected_player->rank],
					     selected_player->name);
//                                      dipstatus = NULL; /* not needed ? */
			}
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
	mvwprintw(local_win, 3, 2, "tab -- select next piece");
	mvwprintw(local_win, 4, 2, "space -- select next player");
	mvwprintw(local_win, 5, 2,
		  "'c' -- claim the region where noble resides");
	mvwprintw(local_win, 6, 2, "'d' -- diplomacy dialog");
	mvwprintw(local_win, 7, 2, "'f' -- feudal dialog");
	mvwprintw(local_win, 8, 2, "'i' -- show game information");
	mvwprintw(local_win, 9, 2, "'h' -- name a successor");
	mvwprintw(local_win, 10, 2, "'m' -- give money to another player");
	mvwprintw(local_win, 11, 2, "'q' -- quit editor");
	mvwprintw(local_win, 12, 2, "'r' -- give a region to another player");
	mvwprintw(local_win, 13, 2,
		  "'w' -- toggle walkability of current tile");
	mvwprintw(local_win, 14, 2, "'s' -- save game to file");
	mvwprintw(local_win, 15, 2, "'t' -- take money (if dice rolls 6)");
	mvwprintw(local_win, 16, 2, "'u' -- place a soldier on current tile");
	mvwprintw(local_win, 17, 2,
		  "'v' -- toggle between 'move piece' and 'explore map' modes");
	mvwprintw(local_win, 18, 2, "'?' -- show this help");

	mvwprintw(local_win, 23, 2, "To return, press any key");
	get_input(local_win);
	current_screen = MAIN_SCREEN;
}

/**
gameover_dialog() {
	
}
**/

int main()
{
	if (check_termsize()) {
		printf("Your terminal must be at least 24x80. Exiting...\n");
		return 0;
	}

	gameover = 0;
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
	curs_set(FALSE);	// 0

	current_screen = MAIN_SCREEN;
	while (1) {
		switch (current_screen) {
		case MAIN_SCREEN:
			if (world->grid == NULL) {
				int retval = load_game();
				if (retval != 0) {	/* set screen multipliers here!!! */
					destroy_world();
					endwin();
					printf
					    ("Cannot open saved game. Exiting...\n");
					return 0;
				}
				/* check if loaded data are playable */
				char *error_message = NULL;
				retval = validate_game_data(&error_message);
				if (retval != 0) {
					destroy_world();
					endwin();
					printf
					    ("Game not playable: %s Exiting...\n",
					     error_message);
					if (error_message != NULL)
						free(error_message);
					return 0;
				}
			}
			draw_map();
			break;
		case REGIONS_DIALOG:
			regions_dialog();
			break;
		case EDIT_REGION_DIALOG:
			rename_region_dialog();
			break;
		case GIVE_REGION_DIALOG:
			give_region_dialog();
			break;
		case GIVE_MONEY_DIALOG:
			give_money_dialog();
			break;
		case INFORMATION:
			info_dialog();
			break;
		case GAME_OVER:
			info_dialog();
			current_screen = 99;
			break;
		case HEIR_DIALOG:
			successor_dialog();
			break;
		case FEUDAL_DIALOG:
			feudal_dialog();
			break;
		case HOMAGE_DIALOG:
			homage_dialog();
			break;
		case PROMOTE_SOLDIER_DIALOG:
			promote_soldier_dialog();
			break;
		case DIPLOMACY_DIALOG:
			diplomacy_dialog();
			break;
		case HELP_DIALOG:
			help_dialog();
			break;
		case 99:
			destroy_world();
			endwin();
//                              printf ("Thanks for playing!\n");
			return 0;
			break;
		}
	}
//      info_dialog();
//      destroy_world();
/* TODO write to chronicle, delete savefile etc */
	endwin();
	printf("Thanks for playing!\n");
	return 0;
}
