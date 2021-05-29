#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <ncurses.h>
#include <ctype.h>		//for isdigit
#include "world.h"

char *modes[] = {
	[MOVE] = "MOVE",
	[VIEW] = "VIEW",
};

int current_screen;

int current_mode = VIEW;

char *screens[] = {
	[START_MENU] = "MENU",
	[MAIN_SCREEN] = "GAME: MAIN SCREEN",
	[GAME_TIME_DIALOG] = "SET GAME TIME",
	[NEW_GAME] = "CREATE NEW GAME",
	[MAP_EDITOR] = "MAP EDITOR",
	[REGIONS_DIALOG] = "REGIONS",
	[ADD_REGION_DIALOG] = "ADD REGION",
	[EDIT_REGION_DIALOG] = "EDIT REGION",
	[GIVE_REGION_DIALOG] = "GIVE REGION",
	[RENAME_REGION_DIALOG] = "RENAME REGION",
	[CHARACTERS_DIALOG] = "HEROES",
	[ADD_CHARACTER_DIALOG] = "ADD HERO",
	[EDIT_CHARACTER_DIALOG] = "EDIT HERO",
	[RENAME_CHARACTER_DIALOG] = "RENAME HERO",
	[CHARACTER_MONEY_DIALOG] = "CHARACTER MONEY",
	[CHARACTER_DATES_DIALOG] = "HERO DATES",
	[REGION_CHARACTER_DIALOG] = "REGION TO HERO",
	[GIVE_MONEY_DIALOG] = "GIVE MONEY",
	[HEIR_DIALOG] = "NAME A HEIR",
	[FEUDAL_DIALOG] = "FEUDAL RELATIONS",
	[HOMAGE_DIALOG] = "PAY HOMAGE",
	[PROMOTE_SOLDIER_DIALOG] = "PROMOTE SOLDIER",
	[DIPLOMACY_DIALOG] = "DIPLOMACY",
	[VALIDATE_DIALOG] = "VALIDATE GAME DATA",
	[HELP_DIALOG] = "HELP",
	[INFORMATION] = "INFORMATION",
	[SELF_DECLARATION_DIALOG] = "BECOME A KING",
	[GAME_OVER] = "GAME OVER",
	[CHOOSE_CHARACTER_DIALOG] = "CHOOSE A HERO"
};

region_t *selected_region = NULL;

char const piece_char[] = {
	[NOBLE] = 'n', /* ? */
	[SOLDIER] = 's',
	[SHIP] = 'S'
};

char const noble_char[] = {
	[KNIGHT] = 'k',
	[BARON] = 'b',
	[COUNT] = 'c',
	[DUKE] = 'd',
	[KING] = 'K'
};

tile_t *cursor = NULL;

int check_termsize()
{
	int retval = 0;
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	if (w.ws_row < 24 || w.ws_col < 80)
		retval = 1;
	return retval;
}

int get_input(WINDOW * window)
{
	int ch = 0, ch2 = 0, input = 0;
	ch = wgetch(window);
	if (ch == 27) {
		ch2 = wgetch(window);
		if (ch2 == '[') {
			input = wgetch(window);
			if (input == 65 || input == 66 || input == 67
			    || input == 68)
				return input + 1000;
		}
		else return tolower(ch2);
	}
	return tolower(ch);
}

int start_menu(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int sf_exists = savefile_exists();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);
	wprintw(local_win, "  [N]ew game\n");
	if (world->grid || sf_exists) wprintw(local_win, "  [L]oad game\n");
	wprintw(local_win, "  [Q]uit\n");

	int ch;
	while (1) {
		ch = tolower(wgetch(local_win));
		switch (ch) {
			case 'l':
				if (world->grid || sf_exists) {
					return MAIN_SCREEN;
				}
				break;
			case 'n':
				return NEW_GAME;
				break;
			case 'q':
				return SHUTDOWN;
				break;
			default:
				break;
		}
	}
}

void draw_map(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	character_t *character = world->selected_character;
	if (!cursor) cursor = get_noble_by_owner(character)->tile;
	piece_t *piece = cursor->piece;

	int i, j;
/**
	uint16_t h_multiplier = world->grid->cursor_height / 24;
	uint16_t w_multiplier = world->grid->cursor_width / 48;
**/
	int16_t h_offset = cursor->height - 12;
	int16_t w_offset = cursor->width - 24;

	char tile_char = '.';
	int color_nr = 0;
	int character_age_mon = 0;
	tile_t *current_tile;
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
				current_tile = NULL;
				color_nr = 1;
				tile_char = ' ';
			} else {
				current_tile = world->grid->tiles[i][j];
				if (current_tile->piece != NULL) {
					color_nr = current_tile->piece->owner->id %
					    6 + 10;
					switch (current_tile->piece->type) {
						case NOBLE:	/* noble */
							tile_char = noble_char[current_tile->piece->owner->rank];
							break;
						case SOLDIER:	/* soldier */
						case SHIP:	/* ship */
							tile_char = piece_char[current_tile->piece->type];
							break;
					}
				}
				else if (current_tile->walkable) {
					if (current_tile->region != NULL
					    && current_tile->region->owner != NULL)
						color_nr = current_tile->region->owner->id % 6 + 10;
					else
						color_nr = 1;
					tile_char = '.';
				}
				else {
					color_nr = 1;	//blue
					tile_char = '~';
				}
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
		  rank_name[character->rank]);
	character_age_mon =
	    (world->current_time.tm_year - character->birthdate.tm_year) * 12 +
	    (world->current_time.tm_mon - character->birthdate.tm_mon);
	mvwprintw(local_win, 5, 50, "Age: %d year(s) %d month(s)",
		  character_age_mon / 12, character_age_mon % 12);
	mvwprintw(local_win, 6, 50, "Money: %d", character->money);
	mvwprintw(local_win, 7, 50, "Army: %d",
		  count_pieces_by_owner(character));
	mvwprintw(local_win, 8, 50, "Land: %d",
		  count_tiles_by_owner(character));
	mvwprintw(local_win, 9, 50, "Heir: %s",
		  (character->heir != NULL ? character->heir->name : "none"));
	mvwprintw(local_win, 10, 50, "Lord: %s",
		  (character->lord != NULL ? character->lord->name : "none"));
	mvwprintw(local_win, 11, 50, "Moves left: %d", world->moves_left);

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
	mvwprintw(local_win, 19, 50, "Diplomacy: ");
	if (piece != NULL && piece->owner->id != character->id) {
		unsigned char status = get_diplomacy(piece->owner, character);
		unsigned char offer = get_offer(piece->owner, character);
		switch (status) {
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
		wprintw(local_win, "%s", dipstatus_name[status]);
		if (offer)
			wprintw(local_win, " *");
		wcolor_set(local_win, 1, NULL);
	}

	int message_len = strlen(world->message);
	if (message_len > 0) {
		wcolor_set(local_win, 12, NULL);
		for (i = 0; i < (message_len - 1) / 29 + 1; i++) {
			mvwprintw(local_win, 20 + i, 50, "%.*s", 29,
				  &world->message[29 * i]);
		}
		wcolor_set(local_win, 1, NULL);
	}

	mvwprintw(local_win, 23, 50, "Mode: %s\n", modes[current_mode]);
	mvwprintw(local_win, 23, 65, "Help: '?'");
	world->message[0] = '\0';
}

int regions_dialog(WINDOW *local_win)
{
	int retval = REGIONS_DIALOG;
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
			"  [You can't give away your last region.]\n");
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

	uint16_t counter = 0;
	uint16_t section = 0;
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
				do {
					selected_region = selected_region->prev;
				} while (selected_region && selected_region->owner != world->selected_character);
			}
			break;
		case 1066:
			if (regionlist_selector < nr_regions - 1) {
				regionlist_selector++;
				do {
					selected_region = selected_region->next;
				} while (selected_region && selected_region->owner != world->selected_character);
			}
			break;
		case 'e':
			retval = EDIT_REGION_DIALOG;
			break;
		case 'g':		/* give to another character */
			retval = GIVE_REGION_DIALOG;
			break;
		case 'q':		/* return to map */
			retval = MAIN_SCREEN;
			break;
		default:
			break;
	}
	return retval;
}

int rename_region_dialog(WINDOW *local_win)
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
				return REGIONS_DIALOG;
				break;
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
			return REGIONS_DIALOG;
			break;
		}
	}
}

int give_region_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	character_t *active_character = world->selected_character;
	character_t *selected_character = world->selected_character;
	int characterlist_selector;
	region_t *region = selected_region;
	int give_region_ok = 0;

	uint16_t nr_characters = count_characters();
	uint16_t counter;
	uint16_t section;
	character_t *current;

	while (1) {
		characterlist_selector = get_character_order(selected_character);
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

		counter = 0;
		section = 0;
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
			if (selected_character->prev)
				selected_character = selected_character->prev;
			break;
		case 1066:
			if (selected_character->next)
				selected_character = selected_character->next;
			break;
		case 10:
			if (give_region_ok) {
				change_region_owner(selected_character, region);
				add_to_chronicle("%s %s granted %s to %s %s.\n",
						rank_name[active_character->rank],
						active_character->name,
						region->name,
						rank_name[selected_character->rank],
						selected_character->name);
				return REGIONS_DIALOG;
				break;
			}
			break;
		case 'q':
			return REGIONS_DIALOG;
			break;
		}
	}
}

int give_money_dialog(WINDOW *local_win)
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
				return MAIN_SCREEN;
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

				uint16_t nr_characters = count_characters();
				uint16_t counter = 0;
				uint16_t section = 0;
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
					return MAIN_SCREEN;
					break;
				}
			}
			break;
		case 2:
			error = transfer_money(active_character, receiving_character, money);
			if (error) strcpy(world->message, "Failed to transfer money");
			return MAIN_SCREEN;
			break;
		}
	}
}

int info_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	uint16_t i, j;
	uint16_t section = 0;
	uint16_t nr_characters = count_characters();

	struct rankings {
		character_t *character;
		uint16_t nr_tiles;
		uint16_t land_ranking;
		uint16_t nr_pieces;
		uint16_t army_ranking;
		uint16_t money_ranking;
	};

	struct rankings **characters = malloc(sizeof(struct rankings *) * nr_characters);
	if (!characters) exit(EXIT_FAILURE);
	for (i = 0; i < nr_characters; i++) {
		characters[i] = malloc(sizeof(struct rankings));
		if (!characters[i]) exit(EXIT_FAILURE);
	}

	character_t *current = world->characterlist;
	i = 0;
	while (i < nr_characters && current != NULL) {
		characters[i]->character = current;
		characters[i]->nr_tiles = count_tiles_by_owner(characters[i]->character);
		characters[i]->land_ranking = 1;
		characters[i]->nr_pieces = count_pieces_by_owner(characters[i]->character);
		characters[i]->army_ranking = 1;
		characters[i]->money_ranking = 1;
		current = current->next;
		i++;
	}

	for (i = 0; i < nr_characters; i++) {
		for (j = 0; j < i; j++) {
			if (characters[i]->nr_tiles < characters[j]->nr_tiles) characters[i]->land_ranking++;
			else if (characters[i]->nr_tiles > characters[j]->nr_tiles) characters[j]->land_ranking++;
			if (characters[i]->nr_pieces < characters[j]->nr_pieces) characters[i]->army_ranking++;
			else if (characters[i]->nr_pieces > characters[j]->nr_pieces) characters[j]->army_ranking++;
			if (characters[i]->character->money < characters[j]->character->money) characters[i]->money_ranking++;
			else if (characters[i]->character->money > characters[j]->character->money) characters[j]->money_ranking++;
		}
	}

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

		i = 0;
		while (i < nr_characters) {
			if (i >= section * 10
			    && i < section * 10 + 10) {
				mvwprintw(local_win, 8 + i % 10, 2,
					  "%3d. %s", characters[i]->character->id,
					  characters[i]->character->name);
				mvwprintw(local_win, 8 + i % 10, 42,
					  "%3d (#%d)", characters[i]->character->money,
					  characters[i]->money_ranking);
				mvwprintw(local_win, 8 + i % 10, 52,
					  "%3d (#%d)",
					  characters[i]->nr_pieces,
					  characters[i]->army_ranking);
				mvwprintw(local_win, 8 + i % 10, 62,
					  "%3d (#%d)",
					  characters[i]->nr_tiles,
					  characters[i]->land_ranking);
			}
			i++;
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
			for (i = 0; i < nr_characters; i++) if (characters[i]) free(characters[i]);
			free(characters);
			return MAIN_SCREEN;
			break;
		}
	}
}

int successor_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *current;
	character_t *active_character = world->selected_character;
	character_t *heir = active_character->heir;
	character_t *new_heir = world->selected_character;
	int characterlist_selector;

	uint16_t nr_characters = count_characters();
	uint16_t counter;
	uint16_t section;

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

		counter = 0;
		section = 0;
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
					 && current == heir)
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
				return MAIN_SCREEN;
				break;
		}
	}
}

int feudal_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *current;
	character_t *active_character = world->selected_character;
	character_t *lord = active_character->lord;

	piece_t *current_piece = cursor->piece;
	character_t *selected_character = NULL;
	int characterlist_selector = 0;
	int create_vassal_ok = 0;
	int promote_vassal_ok = 0;

	uint16_t counter;
	uint16_t section;

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

		counter = 0;
		section = 0;
		current = world->characterlist;
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
				    && current->lord == active_character) {
					mvwprintw(local_win, 9 + counter % 10,
						  2, "%3d. %s (%s)",
						  current->id, current->name,
						  rank_name[current->rank]);
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
				return HOMAGE_DIALOG;
			break;
		case 'f':
			if (selected_character != NULL) {
				unhomage(selected_character);
				add_to_chronicle
				    ("%s %s became a sovereign of his lands.\n",
				     rank_name[selected_character->rank],
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
				add_to_chronicle
				    ("%s %s bestowed the %s title upon their vassal %s.\n",
				     rank_name[active_character->rank],
				     active_character->name,
				     rank_name[selected_character->rank],
				     selected_character->name);
			}
			break;
		case 'q':
			return MAIN_SCREEN;
			break;
		case 'v':
			if (create_vassal_ok)
				return PROMOTE_SOLDIER_DIALOG;
			break;
		}
	}
}

int homage_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *current;
	character_t *active_character = world->selected_character;
	character_t *selected_character = world->selected_character;
	int characterlist_selector;

	int pay_homage_ok = 0;

	uint16_t nr_characters = count_characters();
	uint16_t counter;
	uint16_t section;

	while (1) {
		pay_homage_ok = 0;

		curs_set(FALSE);
		noecho();
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		if (selected_character == active_character)
			mvwprintw(local_win, 2, 2,
				  "[You can't pay homage to yourself]\n");
		else if (selected_character->lord != NULL
			 && selected_character->lord == active_character)
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

		counter = 0;
		section = 0;
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
				if (current == active_character)
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
			if (selected_character->prev) selected_character = selected_character->prev;
			break;
		case 1066:
			if (selected_character->next) selected_character = selected_character->next;
			break;
		case 10:
			if (pay_homage_ok) {
				homage(active_character, selected_character);
				return FEUDAL_DIALOG;
			}
			break;
		case 'q':
			return FEUDAL_DIALOG;
			break;
		}
	}
}

int promote_soldier_dialog(WINDOW *local_win)
{
	/* replace currently selected piece with a new character */
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *active_character = world->selected_character;
	character_t *new_vassal = NULL;
	piece_t *current_piece = cursor->piece;	/* assumed to be our soldier */
	int regionlist_selector;

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
	uint16_t counter;
	uint16_t section;

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
			if (strlen(name) == 0)
				return FEUDAL_DIALOG;
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

			counter = 0;
			section = 0;
			current_region = world->regionlist;
			regionlist_selector = 0;
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
                    do {
                        selected_region = selected_region->prev;
					} while (selected_region && selected_region->owner != world->selected_character);
                }
				break;
			case 1066:
				if (regionlist_selector < nr_regions - 1) {
					regionlist_selector++;
					do {
                        selected_region = selected_region->next;
					} while (selected_region && selected_region->owner != world->selected_character);
				}
				break;
			case 'q':	/* return to map */
				return FEUDAL_DIALOG;
				break;
			case 10:
				new_vassal = add_character_before(active_character, name);
				current_piece->type = NOBLE;
				set_character_rank(new_vassal, KNIGHT);
				current_piece->owner = new_vassal;
				homage(new_vassal, active_character);
				current_region = selected_region;
				change_region_owner(new_vassal, current_region);
				add_to_chronicle
				    ("%s %s granted %s to their new vassal, %s %s.\n",
				     rank_name[active_character->rank],
				     active_character->name, current_region->name,
				     rank_name[new_vassal->rank],
				     new_vassal->name);
				return FEUDAL_DIALOG;
				break;
			default:
				break;
			}
		}
	}
}

int diplomacy_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int i;
	character_t *active_character = world->selected_character;
	character_t *selected_character = world->selected_character;
	int characterlist_selector;

	unsigned char offer_alliance_ok, quit_alliance_ok, offer_peace_ok,
	    declare_war_ok, accept_offer_ok, reject_offer_ok, retract_offer_ok;

	unsigned char status;
	unsigned char offer;

	uint16_t nr_characters = count_characters();
	uint16_t counter;
	uint16_t section;
	character_t *current;
	unsigned char current_status;
	unsigned char current_offer;

	while (1) {
		status = NEUTRAL;
		offer = 0;
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
			status = 3;	/* self */
			offer = 0;
		}
		else {
			status = get_diplomacy(active_character, selected_character);
			offer = get_offer(active_character, selected_character);
		}
		switch (status) {
		case NEUTRAL:
			if (offer) {
				if (offer & OFFER_RECEIVED_BIT) {
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
			if (offer) {
				if (offer & OFFER_RECEIVED_BIT) {
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

		counter = 0;
		section = 0;
		current = world->characterlist;
		characterlist_selector = get_character_order(selected_character);
		while (current != NULL) {
			current_status = get_diplomacy(active_character, current);
			current_offer = get_offer(active_character, current);
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
				if (current == active_character)
					wprintw(local_win, "you");
				else {
					wprintw(local_win, "%s",
						dipstatus_name[current_status]);
					if (current_offer)
						wprintw(local_win,
							", %s offer %s",
							dipstatus_name
							[current_status == WAR ? NEUTRAL : ALLIANCE],
							(current_offer & OFFER_SENT_BIT ? "sent"
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
				open_offer(active_character, selected_character);
				status =
				    get_diplomacy(active_character,
						  selected_character);
			}
			break;
		case 'n':	/* reject offer */
			if (reject_offer_ok)
				close_offer(active_character, selected_character, REJECT);
			break;
		case 'p':	/* offer peace */
			if (offer_peace_ok) {
				open_offer(active_character, selected_character);
			}
			break;
		case 'q':	/* return */
			return MAIN_SCREEN;
			break;
		case 'r':	/* retract offer */
			if (retract_offer_ok)
				close_offer(active_character, selected_character, REJECT);
			break;
		case 'w':	/* declare war */
			if (declare_war_ok) {
				set_diplomacy(active_character, selected_character,
					      WAR);
				add_to_chronicle
				    ("%s %s declared war on %s %s.\n",
				     rank_name[active_character->rank],
				     active_character->name,
				     rank_name[selected_character->rank],
				     selected_character->name);
			}
			break;
		case 'x':	/* quit alliance */
			if (quit_alliance_ok) {
				set_diplomacy(active_character, selected_character,
					      NEUTRAL);
				add_to_chronicle
				    ("%s %s broke their alliance with %s %s.\n",
				     rank_name[active_character->rank],
				     active_character->name,
				     rank_name[selected_character->rank],
				     selected_character->name);
			}
			break;
		case 'y':	/* accept offer */
			if (accept_offer_ok) {
				close_offer(active_character, selected_character, ACCEPT);
				if (status == ALLIANCE)
					add_to_chronicle
					    ("%s %s and %s %s concluded an alliance.\n",
					     rank_name[active_character->rank],
					     active_character->name,
					     rank_name[selected_character->rank],
					     selected_character->name);
				else
					add_to_chronicle
					    ("%s %s and %s %s signed a peace treaty.\n",
					     rank_name[active_character->rank],
					     active_character->name,
					     rank_name[selected_character->rank],
					     selected_character->name);
			}
			break;
		}
	}
}

int help_dialog(WINDOW *local_win)
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
	mvwprintw(local_win, 14, 2, "'s' -- save game to file");
	mvwprintw(local_win, 15, 2, "'t' -- take money (if dice rolls 6)");
	mvwprintw(local_win, 16, 2, "'u' -- place a soldier on current tile");
	mvwprintw(local_win, 17, 2,
		  "'v' -- toggle between 'move piece' and 'explore map' modes");
	mvwprintw(local_win, 18, 2, "'?' -- show this help");

	mvwprintw(local_win, 23, 2, "To return, press any key");
	wgetch(local_win);
	return MAIN_SCREEN;
}

int self_declaration_dialog(WINDOW *local_win) {
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
		add_to_chronicle
				    ("%s %s declared themselves a king.\n",
				     rank_name[active_character->rank],
				     active_character->name);
		set_character_rank(active_character, KING);
		set_money(active_character, money - KING + rank);
	}
	return MAIN_SCREEN;
}

int editor_start_menu(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	int sf_exists = savefile_exists();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);
	wprintw(local_win, "  [N]ew game\n");
	if (world->grid || sf_exists) wprintw(local_win, "  [L]oad game\n");
	wprintw(local_win, "  [Q]uit\n");

	int ch;
	while (1) {
		ch = tolower(wgetch(local_win));
		switch (ch) {
			case 'l':
				if (world->grid || sf_exists) {
					return MAP_EDITOR;
				}
				break;
			case 'n':
				return NEW_GAME;
				break;
			case 'q':
				return SHUTDOWN;
				break;
			default:
				break;
		}
	}
}

int map_editor(WINDOW *local_win)
{
	int retval = MAP_EDITOR;
	if (!cursor) cursor = world->grid->tiles[0][0];
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	uint16_t h_multiplier = cursor->height / 24;
	uint16_t w_multiplier = cursor->width / 48;
	int i, j;
	char tile_char = '.';
	int color_nr = 0;
	character_t *character = world->selected_character;
	region_t *region = NULL;
	if (selected_region == NULL && world->regionlist != NULL)
		selected_region = world->regionlist;
	region = selected_region;
	piece_t *piece = cursor->piece;

	tile_t *current_tile = NULL;
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
			current_tile = world->grid->tiles[i][j];
/**
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
**/
			if (current_tile->piece != NULL) {
				color_nr = current_tile->piece->owner->id %
				    6 + 10;
				switch (current_tile->piece->type) {
					case NOBLE:	/* noble */
						tile_char = noble_char[current_tile->piece->owner->rank];
						break;
					case SOLDIER:	/* soldier */
					case SHIP:	/* ship */
						tile_char = piece_char[current_tile->piece->type];
						break;
				}
			} else if (current_tile->walkable) {
				if (current_tile->region != NULL
				    && current_tile->region->owner != NULL)
					color_nr = current_tile->region->owner->id % 6 + 10;
				else
					color_nr = 1;
				tile_char = '.';
			} else {
				color_nr = 1;	/* blue */
				tile_char = '~';
			}
			if (cursor == current_tile) {
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
	mvwprintw(local_win, 23, 65, "Help: '?'");

	int ch;
	ch = get_input(local_win);

	switch (ch) {
	/**
	case 27:
		retval = MAIN_SCREEN;
		break;
	**/
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
		retval = CHARACTERS_DIALOG;
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
		retval = START_MENU;
		break;
	case 'r':
		retval = REGIONS_DIALOG;
		break;
	case 's':
		save_game();
		break;
	case 't':
		retval = GAME_TIME_DIALOG;
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
		retval = VALIDATE_DIALOG;
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
		retval = HELP_DIALOG;
		break;
	default:
		break;
	}
	return retval;
}

int characters_dialog(WINDOW *local_win)
{
	int retval = CHARACTERS_DIALOG;
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

	uint16_t nr_characters = count_characters();
	uint16_t counter = 0;
	uint16_t section = 0;
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
		retval = ADD_CHARACTER_DIALOG;
		break;
	case 'd':
		current = world->selected_character;
		if (world->selected_character->next) world->selected_character = world->selected_character->next;
		else world->selected_character = world->selected_character->prev;
		remove_character(current);
		break;
	case 'e':
		retval = EDIT_CHARACTER_DIALOG;
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
		retval = MAP_EDITOR;
		break;
	default:
		break;
	}
	return retval;
}

int add_character_dialog(WINDOW *local_win)
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
				return CHARACTERS_DIALOG;
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
			curs_set(FALSE);
			noecho();
			return CHARACTERS_DIALOG;
			break;
		}
	}
}

int editor_regions_dialog(WINDOW *local_win)
{
	int retval = REGIONS_DIALOG;
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
	uint16_t counter = 0;
	uint16_t section = 0;
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
		if (selected_region->prev)
			selected_region = selected_region->prev;
		break;
	case 1066:
		if (selected_region->next)
			selected_region = selected_region->next;
		break;
	case 'a':
		retval = ADD_REGION_DIALOG;
		break;
	case 'd':
		current = selected_region;
		if (selected_region->next)
			selected_region = selected_region->next;
		else if (selected_region->prev)
			selected_region = selected_region->prev;
		else selected_region = NULL;
		remove_region(current);
		break;
	case 'e':
		retval = RENAME_REGION_DIALOG;
		break;
	case 'o':
		retval = REGION_CHARACTER_DIALOG;
		break;
	case 'q':
		regionlist_selector = 0;
		retval = MAP_EDITOR;
		break;
	}
	return retval;
}

int add_region_dialog(WINDOW *local_win)
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
				return CHARACTERS_DIALOG;
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
			return REGIONS_DIALOG;
			break;
		}
	}
}

int region_to_character(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	region_t *region = selected_region;
	character_t *selected_character;
	if (region->owner) selected_character = region->owner;
	else selected_character = world->selected_character;
	int characterlist_selector;

	while (1) {
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

		uint16_t nr_characters = count_characters();
		uint16_t counter = 0;
		uint16_t section = 0;
		characterlist_selector = get_character_order(selected_character);
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
				    && region->owner == current)
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
				if (selected_character->prev)
					selected_character = selected_character->prev;
				break;
			case 1066:
				if (selected_character->next)
					selected_character = selected_character->next;
				break;
			case 10:
				change_region_owner(selected_character, region);
				return REGIONS_DIALOG;
				break;
			case 'd':
				change_region_owner(NULL, region);
				return REGIONS_DIALOG;
				break;
			case 'q':
				return REGIONS_DIALOG;
				break;
		}
	}
}

int edit_character_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int have_lord = 0;

	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	character_t *active_character = world->selected_character;
	/* we can not get into this function if there's no selected_character, but still */
	if (active_character == NULL) {
		return CHARACTERS_DIALOG;
	}
	if (active_character->lord) have_lord = 1;
	while (1) {
		wclear(local_win);
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
		if (have_lord)
				  mvwprintw(local_win, 15, 2, "To unset lord, press 'l'");
		else
				  mvwprintw(local_win, 15, 2, "To set lord, press 'l'");
		mvwprintw(local_win, 16, 2, "To edit diplomacy, press 'd'");
		mvwprintw(local_win, 17, 2, "To return, press 'q'");

		int user_move = get_input(local_win);
		switch (user_move) {
		case '+':
			/* can not increase rank above king */
			if (active_character->rank ==  KING) break;
			/* can not increase rank if it will be equal to lord's rank */
			if (active_character->lord && active_character->rank + 1 == active_character->lord->rank) break;
			active_character->rank++;
			break;
		case '-':
			if (active_character->rank > KNIGHT)
				active_character->rank--;
			break;
		case 'b':
			return CHARACTER_DATES_DIALOG;
			break;
		case 'd':
			return DIPLOMACY_DIALOG;
			break;
		case 'h':
			return HEIR_DIALOG;
			break;
		case 'l':
			if (have_lord) {
				unhomage(active_character);
				have_lord = 0;
			}
			else return FEUDAL_DIALOG;
			break;
		case 'm':
			return CHARACTER_MONEY_DIALOG;
			break;
		case 'q':
			return CHARACTERS_DIALOG;
			break;
		case 'r':
			return RENAME_CHARACTER_DIALOG;
			break;
		}
	}
}

int rename_character_dialog(WINDOW *local_win)
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
			return EDIT_CHARACTER_DIALOG;
		}
		if (get_character_by_name(name) == NULL) {
			strcpy(character->name, name);
			return EDIT_CHARACTER_DIALOG;
		}
		mvwprintw(local_win, 23, 2,
			  "Name not unique. Try another one.");
	}
}

int change_character_money_dialog(WINDOW *local_win)
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
			return EDIT_CHARACTER_DIALOG;
		}
		for (i = 0; i < strlen(money_ch); i++) {
			if (!isdigit(money_ch[i])) {
				error = 1;
			}
		}
		if (!error) {
			money = atoi(money_ch);
			set_money(character, money);
			return EDIT_CHARACTER_DIALOG;
		}
		mvwprintw(local_win, 23, 2, "Error. Try another number.");
	}
}

int change_character_dates_dialog(WINDOW *local_win)
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
	uint16_t deathyear = 0;
	char deathyear_ch[4] = { 0 };
	unsigned char deathmon = 0;
	char deathmon_ch[3] = { 0 };

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
			return EDIT_CHARACTER_DIALOG;
		}
	}
}

int validate_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);

	char *msg;
	int i;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s\n\n", screens[current_screen]);

//	char *error_message = NULL;
	int validation_result = validate_game_data();

	if (validation_result == 0) {
		msg = "All checks passed. Game data are valid.";
//		error_message = malloc(strlen(msg) + 1);
		memcpy(world->message, msg, strlen(msg) + 1);
	}
	wmove(local_win, 10, 0);
	for (i = 0; i < (80 - strlen(world->message)) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, world->message);
//	free(error_message);

	msg = "Press any key to continue.";
	wmove(local_win, 12, 0);
	for (i = 0; i < (80 - strlen(msg)) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", msg);

	wgetch(local_win);
	return MAP_EDITOR;
}

int edit_time_dialog(WINDOW *local_win)
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
	unsigned char mon = 0;
	char mon_ch[3] = { 0 };

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
			return MAP_EDITOR;
			break;
		}
	}
}

int editor_successor_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	character_t *current;
	character_t *active_character = world->selected_character;
	character_t *heir = active_character->heir;
	character_t *new_heir = world->selected_character;
	int characterlist_selector;

	uint16_t nr_characters = count_characters();
	uint16_t counter;
	uint16_t section;

	while (1) {
		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		mvwprintw(local_win, 2, 2, "To pick a hero, press Enter.\n");
		mvwprintw(local_win, 3, 2, "To scroll, press up/down keys.\n");
		mvwprintw(local_win, 4, 2, "To remove heir, press 'd'.\n");
		mvwprintw(local_win, 5, 2, "To return, press 'q'.\n\n");

		counter = 0;
		section = 0;
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
					 && current == heir)
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
				return EDIT_CHARACTER_DIALOG;
				break;
		}
	}
}

int editor_homage_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	character_t *active_character = world->selected_character;
	character_t *lord = world->selected_character;
	int characterlist_selector;

	int set_lord_ok = 0;
	int unset_lord_ok = 0;

	uint16_t nr_characters = count_characters();
	uint16_t counter;
	uint16_t section;
	character_t *current;

	while (1) {
		set_lord_ok = 0;
		unset_lord_ok = 0;

		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		if (lord == active_character)
			mvwprintw(local_win, 2, 2,
				  "[You can't pay homage to yourself]\n");
		else if (lord->lord != NULL
			 && lord->lord == active_character)
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

		counter = 0;
		section = 0;
		current = world->characterlist;
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
				if (current == active_character)
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
					homage(active_character, lord);
					return EDIT_CHARACTER_DIALOG;
				}
				break;
			case 'd':
				if (unset_lord_ok) {
					unhomage(active_character);
					return EDIT_CHARACTER_DIALOG;
				}
				break;
			case 'q':
				return EDIT_CHARACTER_DIALOG;
				break;
		}
	}
}

int editor_diplomacy_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i;
	character_t *active_character = world->selected_character;
	character_t *selected_character = world->selected_character;
	int characterlist_selector;

	unsigned char alliance_ok, neutral_ok, war_ok;
	unsigned char status;

	curs_set(FALSE);
	noecho();

	uint16_t nr_characters = count_characters();
	uint16_t counter;
	uint16_t section;
	character_t *current;
	unsigned char current_status;
	while (1) {
		status = NEUTRAL;
		alliance_ok = 0;	/* a */
		neutral_ok = 0;	/* n */
		war_ok = 0;	/* w */

		wclear(local_win);
		for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
			wprintw(local_win, " ");
		wprintw(local_win, "%s\n\n", screens[current_screen]);

		if (active_character == selected_character)
			status = 3;	/* self */
		else
			status = get_diplomacy(active_character, selected_character);
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

		counter = 0;
		section = 0;
		current = world->characterlist;
		current_status = NEUTRAL;
		characterlist_selector = get_character_order(selected_character);
		while (current != NULL) {
			if (active_character != current) {
				current_status =
				    get_diplomacy(active_character, current);
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
				if (current == active_character)
					wprintw(local_win, "you");
				else {
					wprintw(local_win, "%s",
						dipstatus_name[current_status]);
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
			if (selected_character->prev)
				selected_character = selected_character->prev;
			break;
		case 1066:
			if (selected_character->next)
				selected_character = selected_character->next;
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
			return EDIT_CHARACTER_DIALOG;
			break;
		case 'w':	/* declare war */
			if (war_ok)
				set_diplomacy(active_character, selected_character,
					      WAR);
			break;
		}
	}
}

int editor_help_dialog(WINDOW *local_win)
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
	wgetch(local_win);
	return MAP_EDITOR;
}
