#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "world.h"

int gameover = 0;

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

	current_screen = START_MENU;
	while (1) {
		switch (current_screen) {
		case START_MENU:
			current_screen = start_menu(local_win);
			break;
		case NEW_GAME:
			if (new_game_dialog(local_win) == 0) current_screen = MAIN_SCREEN;
			else current_screen = START_MENU;
			break;
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
				retval = validate_game_data();
				if (retval != 0) {
					destroy_world();
					endwin();
					printf
					    ("Game not playable: %s Exiting...\n",
					     world->message);
					return 0;
				}
			}
			draw_map(local_win);
			int ch = get_input(local_win);
			unsigned char result;
			switch (ch) {
				case 1065:		//up
					if (current_mode == VIEW) {
						if (cursor->height < world->grid->height - 1) cursor = world->grid->tiles[cursor->height + 1][cursor->width];
					} else {
						if (move_piece(cursor->piece, cursor->height + 1, cursor->width) == 0) {
							cursor = world->grid->tiles[cursor->height + 1][cursor->width];
							check_death();
						}
					}
					break;
				case 1066:		//down
					if (current_mode == VIEW) {
						if (cursor->height > 0) cursor = world->grid->tiles[cursor->height - 1][cursor->width];
					} else {
						if (move_piece(cursor->piece, cursor->height - 1, cursor->width) == 0) {
							cursor = world->grid->tiles[cursor->height - 1][cursor->width];
							check_death();
						}
					}
					break;
				case 1067:		//right
					if (current_mode == VIEW) {
						if (cursor->width < world->grid->width - 1) cursor = world->grid->tiles[cursor->height][cursor->width + 1];
					} else {
						if (move_piece(cursor->piece, cursor->height, cursor->width + 1) == 0) {
							cursor = world->grid->tiles[cursor->height][cursor->width + 1];
							check_death();
						}
					}
					break;
				case 1068:		//left
					if (current_mode == VIEW) {
						if (cursor->width > 0) cursor = world->grid->tiles[cursor->height][cursor->width - 1];
					} else {
						if (move_piece(cursor->piece, cursor->height, cursor->width - 1) == 0) {
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
					world->moves_left = get_dice();
					if (world->selected_character->next != NULL)
						world->selected_character = world->selected_character->next;
					else {
						/* start the next round and change game date */
						world->selected_character = world->characterlist;
						increment_gametime();
					}
					cursor = get_noble_by_owner(world->selected_character)->tile;
					current_mode = VIEW;
					break;
				case '\t':		// loop through own pieces
					if (cursor->piece != NULL && cursor->piece->owner == world->selected_character) {
						cursor = next_piece(cursor->piece)->tile;
					}
					else
						cursor = get_noble_by_owner(world->selected_character)->tile;
					break;
				case 'c':		// claim a region
					/* set cursor to noble */
					cursor = get_noble_by_owner(world->selected_character)->tile;
					result = claim_region(world->selected_character, cursor->region);
					switch (result) {
						case 1:	/* claimed from nature */
							add_to_chronicle("%s %s claimed %s.\n",
									 rank_name[world->selected_character->rank], world->selected_character->name,
									 cursor->region->name);
							break;
						case 2:	/* conquered from enemy */
							add_to_chronicle("%s %s conquered %s.\n",
									 rank_name[world->selected_character->rank], world->selected_character->name,
									 cursor->region->name);
							check_death();
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
					if (world->selected_character->money > 0)
						current_screen = GIVE_MONEY_DIALOG;
					break;
				case 'q':		// quit
					current_screen = START_MENU;
					break;
				case 'r':		// give region
					current_screen = REGIONS_DIALOG;
					break;
				case 's':		// save game
					save_game();
					break;
				case 't':		// take money (when 6)
					if (world->moves_left == 6 && get_money(world->selected_character) < MONEY_MAX) {
						set_money(world->selected_character, get_money(world->selected_character) + 1);
						world->moves_left = 0;
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
					if (world->selected_character->money < COST_SOLDIER) {
						strcpy(world->message, "Not enough money.");
						break;
					}
					if (!cursor->walkable) {
						strcpy(world->message, "Tile not walkable. Choose another tile.");
						break;
					}
					if (cursor->piece != NULL) {
						strcpy(world->message, "Tile not empty. Choose another tile.");
						break;
					}
					if (cursor->region == NULL
						|| cursor->region->owner == NULL
						|| cursor->region->owner->id != world->selected_character->id) {
						strcpy(world->message, "Not your region. Choose another tile.");
						break;
					}
					add_piece(1, cursor->height, cursor->width, world->selected_character);
					set_money(world->selected_character, get_money(world->selected_character) - COST_SOLDIER);
					break;
				case 'v':		// toggle between 'move piece' and 'explore map'
					current_mode = (current_mode + 1) % 2;	/* 0->1, 1->0 */
					if (current_mode == 0) {
						cursor = get_noble_by_owner(world->selected_character)->tile;
					}
					break;
				case '?':
					current_screen = HELP_DIALOG;
					break;
				default:	// ignore all other keys
					break;
			}
			break;
		case REGIONS_DIALOG:
			current_screen = regions_dialog(local_win);
			break;
		case EDIT_REGION_DIALOG:
			current_screen = rename_region_dialog(local_win);
			break;
		case GIVE_REGION_DIALOG:
			current_screen = give_region_dialog(local_win);
			break;
		case GIVE_MONEY_DIALOG:
			current_screen = give_money_dialog(local_win);
			break;
		case INFORMATION:
			current_screen = info_dialog(local_win);
			break;
		case GAME_OVER:
			info_dialog(local_win);
			delete_savefile();
			rename_logfile();
			current_screen = SHUTDOWN;
			break;
		case HEIR_DIALOG:
			current_screen = successor_dialog(local_win);
			break;
		case FEUDAL_DIALOG:
			current_screen = feudal_dialog(local_win);
			break;
		case HOMAGE_DIALOG:
			current_screen = homage_dialog(local_win);
			break;
		case PROMOTE_SOLDIER_DIALOG:
			current_screen = promote_soldier_dialog(local_win);
			break;
		case DIPLOMACY_DIALOG:
			current_screen = diplomacy_dialog(local_win);
			break;
		case HELP_DIALOG:
			current_screen = help_dialog(local_win);
			break;
		case SELF_DECLARATION_DIALOG:
			current_screen = self_declaration_dialog(local_win);
			break;
		case SHUTDOWN:
			curs_set(TRUE);
			echo();
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
