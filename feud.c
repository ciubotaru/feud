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

void draw_map(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i, j;
/**
	uint16_t h_multiplier = world->grid->cursor_height / 24;
	uint16_t w_multiplier = world->grid->cursor_width / 48;
**/
	int16_t h_offset = cursor->height - 12;
	int16_t w_offset = cursor->width - 24;

	character_t *character = world->selected_character;
	piece_t *piece = cursor->piece;

	char tile_char = '.';
	int color_nr = 0;
	int character_age_mon = 0;
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
			if (cursor->height == i
			    && cursor->width == j) {
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
	mvwprintw(local_win, 1, 50, "Nr. players: %d", count_characters());
	mvwprintw(local_win, 2, 50, "Map size: %dx%d", world->grid->height,
		  world->grid->width);

	/* character info */
	mvwprintw(local_win, 4, 50, "Hero: %s (%s)", character->name,
		  ranklist[character->rank]);
	character_age_mon =
	    (world->current_time.tm_year - character->birthdate.tm_year) * 12 +
	    (world->current_time.tm_mon - character->birthdate.tm_mon);
	mvwprintw(local_win, 5, 50, "Age: %d year(s) %d month(s)",
		  character_age_mon / 12, character_age_mon % 12);
	mvwprintw(local_win, 6, 50, "Money: %d (#%d)", character->money,
		  character->rank_money);
	mvwprintw(local_win, 7, 50, "Army: %d (#%d)",
		  count_pieces_by_owner(character), character->rank_army);
	mvwprintw(local_win, 8, 50, "Land: %d (#%d)",
		  count_tiles_by_owner(character), character->rank_land);
	mvwprintw(local_win, 9, 50, "Heir: %s",
		  (character->heir != NULL ? character->heir->name : "none"));
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
		  (piece == NULL ? " " : unit_type_list[piece->type]));
	mvwprintw(local_win, 18, 50, "Owned by: %s",
		  (piece == NULL ? " " : piece->owner->name));
	mvwprintw(local_win, 19, 50, "Diplomacy: ");
	if (piece != NULL && piece->owner->id != character->id) {
		dipstatus_t *diplomacy = get_dipstatus(piece->owner, character);
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
		wcolor_set(local_win, 12, NULL);
		for (i = 0; i < (message_len - 1) / 29 + 1; i++) {
			mvwprintw(local_win, 20 + i, 50, "%.*s", 29,
				  &message[29 * i]);
		}
		wcolor_set(local_win, 1, NULL);
	}

	mvwprintw(local_win, 23, 50, "Mode: %s\n", modes[current_mode]);
	mvwprintw(local_win, 23, 65, "Help: '?'");

	int ch;
	ch = get_input(local_win);
	unsigned char result;
	message[0] = '\0';
	switch (ch) {
	case 1065:		//up
		if (current_mode == VIEW) {
			if (cursor->height < world->grid->height - 1) cursor = world->grid->tiles[cursor->height + 1][cursor->width];
		} else {
			if (move_piece(piece, cursor->height + 1,
				   cursor->width) == 0) {
				cursor = world->grid->tiles[cursor->height + 1][cursor->width];
				check_death();
			}
		}
		break;
	case 1066:		//down
		if (current_mode == VIEW) {
			if (cursor->height > 0) cursor = world->grid->tiles[cursor->height - 1][cursor->width];
		} else {
			if (move_piece(piece, cursor->height - 1,
				   cursor->width) == 0) {
				cursor = world->grid->tiles[cursor->height - 1][cursor->width];
				check_death();
			}
		}
		break;
	case 1067:		//right
		if (current_mode == VIEW) {
			if (cursor->width < world->grid->width - 1) cursor = world->grid->tiles[cursor->height][cursor->width + 1];
		} else {
			if (move_piece(piece, cursor->height,
				   cursor->width + 1) == 0) {
				cursor = world->grid->tiles[cursor->height][cursor->width + 1];
				check_death();
			}
		}
		break;
	case 1068:		//left
		if (current_mode == VIEW) {
			if (cursor->width > 0) cursor = world->grid->tiles[cursor->height][cursor->width - 1];
		} else {
			if (move_piece(piece, cursor->height,
				   cursor->width - 1) == 0) {
				cursor = world->grid->tiles[cursor->height][cursor->width - 1];
				check_death();
			}
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
		if (character->next != NULL)
			character = character->next;
		else {
			/* start the next round and change game date */
			character = world->characterlist;
			increment_gametime();
		}
		world->selected_character = character;
		piece = get_noble_by_owner(character);
		cursor = piece->tile;
		current_mode = VIEW;
		break;
	case '\t':		// loop through own pieces
		if (piece != NULL) {
			piece_t *active_piece = next_piece(piece);
			cursor = active_piece->tile;
		}
		break;
	case 'c':		// claim a region
		/* first, switch to noble */
		piece = get_noble_by_owner(character);
		/* set cursor to noble */
		cursor = piece->tile;
		result = claim_region(character, cursor->region);
		switch (result) {
		case 1:	/* claimed from nature */
			add_to_cronicle("%s %s claimed %s.\n",
					ranklist[character->rank], character->name,
					cursor->region->name);
			update_land_ranking();
			break;
		case 2:	/* conquered from enemy */
			add_to_cronicle("%s %s conquered %s.\n",
					ranklist[character->rank], character->name,
					cursor->region->name);
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
	case 'k':		// declare yourself king
		current_screen = SELF_DECLARATION_DIALOG;
		break;
	case 'm':		// give money
		/* check if we have money */
		if (character->money > 0)
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
		if (world->moves_left == 6 && get_money(character) < MONEY_MAX) {
			set_money(character, get_money(character) + 1);
			world->moves_left = 0;
			update_money_ranking();
		}
		break;
	case 'u':		//place a soldier -- dialog
			/**
			 * check if in view mode
			 * check if enough money
			 * check if walkable
			 * check if region is our (defined and claimed)
			 * check if empty
			**/
		if (current_mode != VIEW) break;
		if (character->money < COST_SOLDIER) {
			strcpy(message, "Not enough money.");
			break;
		}
		if (!cursor->walkable) {
			strcpy(message, "Tile not walkable. Choose another tile.");
			break;
		}
		if (piece != NULL) {
			strcpy(message, "Tile not empty. Choose another tile.");
			break;
		}
		if (cursor->region == NULL
			|| cursor->region->owner == NULL
			|| cursor->region->owner->id != world->selected_character->id) {
			strcpy(message, "Not your region. Choose another tile.");
			break;
		}
		add_piece(1, cursor->height,
			  cursor->width, character);
		set_money(character, get_money(character) - COST_SOLDIER);
		update_money_ranking();
		update_army_ranking();
		break;
	case 'v':		// toggle between 'move piece' and 'explore map'
		current_mode = (current_mode + 1) % 2;	/* 0->1, 1->0 */
		if (current_mode == 0) {
			piece =
			    get_noble_by_owner(world->selected_character);
			cursor = piece->tile;
		}
		break;
	case '?':
		current_screen = HELP_DIALOG;
		break;
	default:	// ignore all other keys
		break;
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

	region_t *current_region = world->regionlist;
	character_t *active_character = world->selected_character;

	int nr_regions = count_regions_by_owner(active_character);

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
	 * if selected region is not owned by selected character,
	 * set to first region owned by active character
	**/
	if (selected_region == NULL
	    || selected_region->owner == NULL
	    || selected_region->owner !=
	    world->selected_character) {
		current_region = world->regionlist;
		while (current_region != NULL) {
			if (current_region->owner != NULL
			    && current_region->owner ==
			    world->selected_character) {
				selected_region = current_region;
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
		    && current_region->owner == world->selected_character) {
			if (current_region == selected_region)
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
		    || current_region->owner != world->selected_character) {
			current_region = current_region->next;
			;
			continue;
		}
		if (counter >= section * 10 && counter < section * 10 + 10
		    && counter < nr_regions) {
			if (counter == regionlist_selector)
				wattron(local_win, COLOR_PAIR(26));
			else
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
				    && current_region->owner ==
				    world->selected_character) {
					if (counter == regionlist_selector) {
						selected_region = current_region;
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
				    && current_region->owner ==
				    world->selected_character) {
					if (counter == regionlist_selector) {
						selected_region = current_region;
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
	case 'g':		/* give to another character */
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

	int stage = 0;
	int error = 0;
	region_t *region = selected_region;
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
			change_region_name(name, region);
			sort_region_list();
			current_screen = REGIONS_DIALOG;
			return;
			break;
		}
	}
	return;
}

void give_region_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	character_t *active_character = world->selected_character;
	character_t *selected_character = world->selected_character;
	int characterlist_selector = get_character_order(selected_character);
	region_t *region = selected_region;
	int give_region_ok = 0;

	while (1) {
		give_region_ok = 0;
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s", screens[current_screen]);

		if (selected_character == active_character) {
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
				if (current == active_character)
					wprintw(local_win, " (you)");
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
				selected_character =
				    get_character_by_order(characterlist_selector);
			}
			break;
		case 1066:
			if (characterlist_selector < nr_characters - 1) {
				characterlist_selector++;
				selected_character =
				    get_character_by_order(characterlist_selector);
			}
			break;
		case 10:
			if (give_region_ok) {
				change_region_owner(selected_character, region);
				add_to_cronicle("%s %s granted %s to %s %s.\n",
						ranklist[active_character->rank],
						active_character->name,
						region->name,
						ranklist[selected_character->rank],
						selected_character->name);
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

void give_money_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	int stage = 0;
	int error = 0;
	character_t *active_character = world->selected_character;
	character_t *receiving_character = NULL;
	int characterlist_selector;
	uint16_t money = 0;
	char money_ch[MONEY_MAX_DIGITS + 1] = { 0 };

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
					active_character->money);
			else
				wprintw(local_win,
					"  Type the amount to be given (1-%d), or press Enter to dismiss:\n\n  ",
					active_character->money);
			wgetnstr(local_win, money_ch, MONEY_MAX_DIGITS);
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
			if (money < 1 || money > active_character->money) {
				error = 1;
				break;
			}
			stage = 1;
			error = 0;
			break;
		case 1:
			receiving_character = world->selected_character;
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

				int nr_characters = count_characters();
				int counter = 0;
				int section = 0;
				character_t *current = world->characterlist;
				characterlist_selector =
				    get_character_order(receiving_character);
				while (current != NULL) {
					section = characterlist_selector / 10;
					if (counter >= section * 10
					    && counter < section * 10 + 10
					    && counter < nr_characters) {
						if (counter ==
						    characterlist_selector) {
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
						if (current == world->selected_character)
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
					if (receiving_character->prev) receiving_character = receiving_character->prev;
					break;
				case 1066:
					if (receiving_character->next) receiving_character = receiving_character->next;
					break;
				case 10:
					if (receiving_character != active_character) {
						error = 0;
						stage = 2;
					}
					break;
				case 27:
					current_screen = MAIN_SCREEN;
					return;
					break;
				}
			}
			break;
		case 2:
			if (get_money(receiving_character) + money > MONEY_MAX) {
				set_money(active_character, 
					  get_money(active_character) + get_money(receiving_character) - MONEY_MAX);
				set_money(receiving_character, MONEY_MAX);
			} else {
				set_money(active_character, 
					  get_money(active_character) - money);
				set_money(receiving_character,
					  get_money(receiving_character) + money);
			}
			update_money_ranking();
			current_screen = MAIN_SCREEN;
			return;
			break;
		}
	}
}

void info_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	int section = 0;
	int nr_characters = count_characters();

	while (1) {
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);
		wprintw(local_win, "  To scroll, press up/down keys.\n");
		wprintw(local_win, "  To return, press 'q'.\n\n");

		mvwprintw(local_win, 6, 2, " Nr.");
		mvwprintw(local_win, 6, 7, "Hero's Name");
		mvwprintw(local_win, 6, 44, "Money");
		mvwprintw(local_win, 6, 54, "Army");
		mvwprintw(local_win, 6, 64, "Land");

		int counter = 0;
		character_t *current = world->characterlist;
		while (current != NULL) {
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_characters) {
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
			if (section < (nr_characters - 1) / 10)
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

void successor_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *current;
	character_t *active_character = world->selected_character;
	character_t *heir = active_character->heir;
	character_t *new_heir = world->selected_character;
	int characterlist_selector;

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

		int nr_characters = count_characters();
		int counter = 0;
		int section = 0;
		current = world->characterlist;
		characterlist_selector = get_character_order(new_heir);
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
			if (new_heir->prev) new_heir = new_heir->prev;
			break;
		case 1066:
			if (new_heir->next) new_heir = new_heir->next;
			break;
		case 10:
			if (new_heir != active_character) {
				set_successor(active_character, new_heir);
				heir = new_heir;
			}
			break;
		case 'd':
			heir = NULL;
			set_successor(active_character, heir);
			break;
		case 'q':
			current_screen = MAIN_SCREEN;
			return;
			break;
		}
	}
}

void feudal_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *active_character = world->selected_character;
	character_t *lord = active_character->lord;

	piece_t *current_piece = cursor->piece;
	character_t *selected_character = NULL;
	int characterlist_selector = 0;
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

		int nr_vassals = count_vassals(active_character);

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
		else if (get_money(active_character) < COST_SOLDIER)
			mvwprintw(local_win, 5, 2,
				  "[Can't promote a vassal. Not enough money.]");
		else if (selected_character != NULL
			 && active_character->rank - selected_character->rank <= 1)
			mvwprintw(local_win, 5, 2,
				  "[Can't promote a vassal. He is one rank below you.]");
		else {
			mvwprintw(local_win, 5, 2,
				  "To promote a vassal, press 'p'.");
			promote_vassal_ok = 1;
		}
		if (active_character->rank <= KNIGHT)
			mvwprintw(local_win, 6, 2,
				  "[Can't create vassal. Your rank is too low.]");
		else if (count_regions_by_owner(active_character) < 2)
			mvwprintw(local_win, 6, 2,
				  "[Can't create vassal. Too few regions.]");
		else if (get_money(active_character) < COST_SOLDIER)
			mvwprintw(local_win, 6, 2,
				  "[Can't create vassal. Not enough money.]");
		else if (current_piece == NULL)
			mvwprintw(local_win, 6, 2,
				  "[Can't create vassal. No piece selected.]");
		else if (current_piece->owner->id != active_character->id)
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
		character_t *current = world->characterlist;
		while (current != NULL) {
			section = characterlist_selector / 10;
			if (counter >= section * 10
			    && counter < section * 10 + 10
			    && counter < nr_vassals) {
				if (counter == characterlist_selector) {
					wattron(local_win, COLOR_PAIR(26));
					selected_character = current;
				} else
					wattron(local_win, COLOR_PAIR(1));
				if (current->lord != NULL
				    && current->lord->id == active_character->id) {
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
			if (characterlist_selector > 0)
				characterlist_selector--;
			break;
		case 1066:
			if (characterlist_selector < nr_vassals - 1)
				characterlist_selector++;
			break;
		case 'h':
			if (lord == NULL)
				current_screen = HOMAGE_DIALOG;
			return;
			break;
		case 'f':
			if (selected_character != NULL) {
				selected_character->lord = NULL;
				add_to_cronicle
				    ("%s %s became a sovereign of his lands.\n",
				     ranklist[selected_character->rank],
				     selected_character->name);
				characterlist_selector = 0;
				selected_character = NULL;
			}
			break;
		case 'p':
			if (promote_vassal_ok) {
				set_money(active_character,
					  get_money(active_character) -
					  COST_SOLDIER);
				set_character_rank(selected_character,
						get_character_rank(selected_character)
						+ 1);
				add_to_cronicle
				    ("%s %s bestowed the %s title upon their vassal %s.\n",
				     ranklist[active_character->rank],
				     active_character->name,
				     ranklist[selected_character->rank],
				     selected_character->name);
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

void homage_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *current;
	character_t *active_character = world->selected_character;
	character_t *selected_character = world->selected_character;
	int characterlist_selector;

	int pay_homage_ok = 0;

	while (1) {
		pay_homage_ok = 0;

		curs_set(FALSE);
		noecho();
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		if (selected_character->id == active_character->id)
			mvwprintw(local_win, 2, 2,
				  "[You can't pay homage to yourself]\n");
		else if (selected_character->lord != NULL
			 && selected_character->lord->id == active_character->id)
			mvwprintw(local_win, 2, 2,
				  "[You can't pay homage to your own vassal]\n");
		else if (selected_character->rank <= KNIGHT)
			mvwprintw(local_win, 2, 2,
				  "[You can't pay homage to a knight]\n");
		else {
			mvwprintw(local_win, 2, 2,
				  "To pay homage, press Enter.\n");
			pay_homage_ok = 1;
		}
		mvwprintw(local_win, 3, 2, "To scroll, press up/down keys.\n");
		mvwprintw(local_win, 4, 2, "To return, press 'q'.\n\n");

		int nr_characters = count_characters();
		int counter = 0;
		int section = 0;
		characterlist_selector = get_character_order(selected_character);
		current = world->characterlist;
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
						ranklist[current->rank]);
				wattron(local_win, COLOR_PAIR(1));
			}
			current = current->next;
			counter++;
		}

		int user_move = get_input(local_win);
		switch (user_move) {
		case 1065:
			if (selected_character->prev) selected_character = selected_character->prev;
			break;
		case 1066:
			if (selected_character->next) selected_character = selected_character->next;
			break;
		case 10:
			if (pay_homage_ok) {
				homage(active_character, selected_character);
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

void promote_soldier_dialog(WINDOW *local_win)
{
	/* replace currently selected piece with a new character */
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *active_character = world->selected_character;
	character_t *new_vassal = NULL;
	piece_t *current_piece = cursor->piece;	/* assumed to be our soldier */

	/* set selected_region to our region */
	region_t *current_region = world->regionlist;
	while (current_region != NULL) {
		if (current_region->owner != NULL
		    && current_region->owner == world->selected_character) {
			selected_region = current_region;
			break;
		}
		current_region = current_region->next;
	}

	int stage = 0;
	int error = 0;
	char name[17] = { 0 };
	int nr_regions = count_regions_by_owner(active_character);
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
			if (get_character_by_name(name) == NULL) {
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
				    && current_region->owner ==
				    world->selected_character) {
					if (current_region == selected_region)
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
				    || current_region->owner !=
				    world->selected_character) {
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
						    && current_region->owner ==
						    world->selected_character) {
							if (counter ==
							    regionlist_selector)
							{
								selected_region = current_region;
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
						    && current_region->owner ==
						    world->selected_character) {
							if (counter ==
							    regionlist_selector)
							{
								selected_region = current_region;
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
				new_vassal = add_character(name);
				current_piece->type = NOBLE;
				set_character_rank(new_vassal, KNIGHT);
				current_piece->owner = new_vassal;
				homage(new_vassal, active_character);
				current_region = selected_region;
				change_region_owner(new_vassal, current_region);
				add_to_cronicle
				    ("%s %s granted %s to their new vassal, %s %s.\n",
				     ranklist[active_character->rank],
				     active_character->name, current_region->name,
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

void diplomacy_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *active_character = world->selected_character;
	character_t *selected_character = world->selected_character;
	int characterlist_selector;

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

		if (active_character == selected_character) {
			dipstatus = NULL;
			status = 3;	/* self */
			dipoffer = NULL;
		} else {
			dipstatus =
			    get_dipstatus(active_character, selected_character);
			status = dipstatus->status;
			dipoffer = dipstatus->pending_offer;
		}
		switch (status) {
		case NEUTRAL:
			if (dipoffer != NULL) {
				if (dipoffer->from == selected_character) {
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
			if ((active_character->lord != NULL
			     && active_character->lord == selected_character)
			    || (selected_character->lord != NULL
				&& selected_character->lord == active_character))
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
				if (dipoffer->from == selected_character) {
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

		int nr_characters = count_characters();
		int counter = 0;
		int section = 0;
		character_t *current = world->characterlist;
		dipstatus_t *current_dipstatus = NULL;
		unsigned char current_status = 0;
		dipoffer_t *current_dipoffer = NULL;
		characterlist_selector = get_character_order(selected_character);
		while (current != NULL) {
			if (active_character != current) {
				current_dipstatus =
				    get_dipstatus(active_character, current);
				current_status = current_dipstatus->status;
				current_dipoffer =
				    current_dipstatus->pending_offer;
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
						dipstatuslist[current_status]);
					if (current_dipoffer != NULL)
						wprintw(local_win,
							", %s offer %s",
							dipstatuslist
							[current_dipoffer->
							 offer],
							(current_dipoffer->
							 from ==
							 active_character ? "sent"
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
			if (selected_character->prev) selected_character = selected_character->prev;
			break;
		case 1066:
			if (selected_character->next) selected_character = selected_character->next;
			break;
		case 'a':	/* offer alliance */
			if (offer_alliance_ok) {
				open_offer(active_character, selected_character,
					   ALLIANCE);
				dipstatus =
				    get_dipstatus(active_character,
						  selected_character);
			}
			break;
		case 'n':	/* reject offer */
			if (reject_offer_ok)
				close_offer(dipstatus->pending_offer, REJECT);
			break;
		case 'p':	/* offer peace */
			if (offer_peace_ok) {
				open_offer(active_character, selected_character,
					   NEUTRAL);
				dipstatus =
				    get_dipstatus(active_character,
						  selected_character);
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
				set_diplomacy(active_character, selected_character,
					      WAR);
				add_to_cronicle
				    ("%s %s declared war on %s %s.\n",
				     ranklist[active_character->rank],
				     active_character->name,
				     ranklist[selected_character->rank],
				     selected_character->name);
			}
			break;
		case 'x':	/* quit alliance */
			if (quit_alliance_ok) {
				set_diplomacy(active_character, selected_character,
					      NEUTRAL);
				add_to_cronicle
				    ("%s %s broke their alliance with %s %s.\n",
				     ranklist[active_character->rank],
				     active_character->name,
				     ranklist[selected_character->rank],
				     selected_character->name);
			}
			break;
		case 'y':	/* accept offer */
			if (accept_offer_ok) {
				close_offer(dipstatus->pending_offer, ACCEPT);
				if (dipstatus->status == ALLIANCE)
					add_to_cronicle
					    ("%s %s and %s %s concluded an alliance.\n",
					     ranklist[active_character->rank],
					     active_character->name,
					     ranklist[selected_character->rank],
					     selected_character->name);
				else
					add_to_cronicle
					    ("%s %s and %s %s signed a peace treaty.\n",
					     ranklist[active_character->rank],
					     active_character->name,
					     ranklist[selected_character->rank],
					     selected_character->name);
//                                      dipstatus = NULL; /* not needed ? */
			}
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
	mvwprintw(local_win, 3, 2, "tab -- select next piece");
	mvwprintw(local_win, 4, 2, "space -- select next player");
	mvwprintw(local_win, 5, 2,
		  "'c' -- claim the region where noble resides");
	mvwprintw(local_win, 6, 2, "'d' -- diplomacy dialog");
	mvwprintw(local_win, 7, 2, "'f' -- feudal dialog");
	mvwprintw(local_win, 8, 2, "'i' -- show game information");
	mvwprintw(local_win, 9, 2, "'h' -- name a successor");
	mvwprintw(local_win, 10, 2, "'k' -- become a king");
	mvwprintw(local_win, 11, 2, "'m' -- give money to another player");
	mvwprintw(local_win, 12, 2, "'q' -- quit editor");
	mvwprintw(local_win, 13, 2, "'r' -- give a region to another player");
	mvwprintw(local_win, 14, 2,
		  "'w' -- toggle walkability of current tile");
	mvwprintw(local_win, 15, 2, "'s' -- save game to file");
	mvwprintw(local_win, 16, 2, "'t' -- take money (if dice rolls 6)");
	mvwprintw(local_win, 17, 2, "'u' -- place a soldier on current tile");
	mvwprintw(local_win, 18, 2,
		  "'v' -- toggle between 'move piece' and 'explore map' modes");
	mvwprintw(local_win, 19, 2, "'?' -- show this help");

	mvwprintw(local_win, 23, 2, "To return, press any key");
	get_input(local_win);
	current_screen = MAIN_SCREEN;
}

void self_declaration_dialog(WINDOW *local_win) {
	/**
	 * A player can declare themselves a king if they:
	 * - have enough land (5 regions) for a kingdom
	 * - have no sovereign
	 * - have enough money for coronation (1 for every rank "upgrade")
	 * - are not a king yet
	 **/
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	curs_set(FALSE);
	noecho();

	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);

	unsigned char eligible = 0;
	character_t *active_character = world->selected_character;
	uint16_t nr_regions = count_regions_by_owner(active_character);
	character_t *lord = active_character->lord;
	uint16_t money = get_money(active_character);
	unsigned char rank = get_character_rank(active_character);
	if (rank == KING) {
		mvwprintw(local_win, 2, 2, "You are already a king!");
	}
	else if (lord != NULL) {
		mvwprintw(local_win, 2, 2, "Your sovereign does not endorse this!");
	}
	else if (nr_regions < 1) {
		mvwprintw(local_win, 2, 2, "You don't have enough land to create a kingdom (need 5 regions)!");
	}
	else if (money < KING - rank) {
		mvwprintw(local_win, 2, 2, "You don't have enough money for the coronation!");
	}
	else {
		mvwprintw(local_win, 2, 2, "Press 'y' to declare yourself a king!");
		eligible = 1;
	}
	int user_move = get_input(local_win);
	if (eligible && user_move == 'y') {
		add_to_cronicle
				    ("%s %s declared themselves a king.\n",
				     ranklist[active_character->rank],
				     active_character->name);
		set_character_rank(active_character, KING);
		set_money(active_character, money - KING + rank);
	}
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
	WINDOW *local_win = newwin(25, 80, 0, 0);

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
				piece_t *active_piece = get_noble_by_owner(world->selected_character);
				cursor = active_piece->tile;

			}
			draw_map(local_win);
			break;
		case REGIONS_DIALOG:
			regions_dialog(local_win);
			break;
		case EDIT_REGION_DIALOG:
			rename_region_dialog(local_win);
			break;
		case GIVE_REGION_DIALOG:
			give_region_dialog(local_win);
			break;
		case GIVE_MONEY_DIALOG:
			give_money_dialog(local_win);
			break;
		case INFORMATION:
			info_dialog(local_win);
			break;
		case GAME_OVER:
			info_dialog(local_win);
			current_screen = 99;
			break;
		case HEIR_DIALOG:
			successor_dialog(local_win);
			break;
		case FEUDAL_DIALOG:
			feudal_dialog(local_win);
			break;
		case HOMAGE_DIALOG:
			homage_dialog(local_win);
			break;
		case PROMOTE_SOLDIER_DIALOG:
			promote_soldier_dialog(local_win);
			break;
		case DIPLOMACY_DIALOG:
			diplomacy_dialog(local_win);
			break;
		case HELP_DIALOG:
			help_dialog(local_win);
			break;
		case SELF_DECLARATION_DIALOG:
			self_declaration_dialog(local_win);
			break;
		case 99:
			use_default_colors();
			destroy_world();
			endwin();
//                              printf ("Thanks for playing!\n");
			return 0;
			break;
		default: // return to main screen
			current_screen = MAIN_SCREEN;
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
