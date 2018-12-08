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

	int current_screen = MAIN_SCREEN;

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
			current_screen = draw_map(local_win);
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
			current_screen = info_dialog(local_win);
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
