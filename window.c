#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include "window.h"
#include "map.h"
#include "file.h"
#include "character.h"
#include "world.h"
#include <ncurses.h>

char *screens[] = {
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
	[GAME_OVER] = "GAME OVER"
};

region_t *selected_region = NULL;

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
	}
	return ch;
}
