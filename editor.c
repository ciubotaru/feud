#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <ctype.h>		/* for isdigit */
//#include <math.h>		/* for log10 */
#include "world.h"

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
	WINDOW *local_win = newwin(25, 80, 0, 0);

	current_screen = MAIN_SCREEN;
	while (1) {
		switch (current_screen) {
		case MAIN_SCREEN:
			current_screen = editor_start_menu(local_win);
			break;
		case NEW_GAME:
			if (new_game_dialog(local_win) == 0) current_screen = MAP_EDITOR;
			else current_screen = MAIN_SCREEN;
			break;
		case MAP_EDITOR:
			if (world->grid == NULL) {
				load_game();
				piece_t *active_piece = get_noble_by_owner(world->selected_character);
				cursor = active_piece->tile;
			}
			current_screen = map_editor(local_win);
			break;
		case CHARACTERS_DIALOG:
			current_screen = characters_dialog(local_win);
			break;
		case ADD_CHARACTER_DIALOG:
			current_screen = add_character_dialog(local_win);
			break;
		case REGIONS_DIALOG:
			current_screen = editor_regions_dialog(local_win);
			break;
		case ADD_REGION_DIALOG:
			current_screen = add_region_dialog(local_win);
			break;
		case REGION_CHARACTER_DIALOG:
			current_screen = region_to_character(local_win);
			break;
		case GAME_TIME_DIALOG:
			current_screen = edit_time_dialog(local_win);
			break;
		case RENAME_REGION_DIALOG:
			current_screen = rename_region_dialog(local_win);
			break;
		case EDIT_CHARACTER_DIALOG:
			current_screen = edit_character_dialog(local_win);
			break;
		case RENAME_CHARACTER_DIALOG:
			current_screen = rename_character_dialog(local_win);
			break;
		case CHARACTER_MONEY_DIALOG:
			current_screen = change_character_money_dialog(local_win);
			break;
		case CHARACTER_DATES_DIALOG:
			current_screen = change_character_dates_dialog(local_win);
			break;
		case HEIR_DIALOG:
			current_screen = editor_successor_dialog(local_win);
			break;
		case FEUDAL_DIALOG:
			current_screen = editor_homage_dialog(local_win);
			break;
		case DIPLOMACY_DIALOG:
			current_screen = editor_diplomacy_dialog(local_win);
			break;
		case VALIDATE_DIALOG:
			current_screen = validate_dialog(local_win);
			break;
		case HELP_DIALOG:
			current_screen = editor_help_dialog(local_win);
			break;
		case SHUTDOWN:
			destroy_world();
			use_default_colors();
			endwin();
			return 0;
			break;
		default: // return to main screen
			current_screen = MAIN_SCREEN;
			break;
		}
	}
}
