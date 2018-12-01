#ifndef WINDOW_H
#define WINDOW_H

#define BG_BLACK "\33[40m"
#define BG_WHITE "\33[47m"
#define WHITE "\33[0;97;40m"
#define B_WHITE "\33[1;97;40m"	//bright white
#define RESET "\33[0m"
#define SAVE_CURSOR "\033[s"

#include <stdint.h>
#include <ncurses.h>

enum screenlist {
	MAIN_SCREEN,
	GAME_TIME_DIALOG,
	NEW_GAME,
	MAP_EDITOR,
	REGIONS_DIALOG,
	ADD_REGION_DIALOG,
	EDIT_REGION_DIALOG,
	GIVE_REGION_DIALOG,
	RENAME_REGION_DIALOG,
	CHARACTERS_DIALOG,
	ADD_CHARACTER_DIALOG,
	EDIT_CHARACTER_DIALOG,
	RENAME_CHARACTER_DIALOG,
	CHARACTER_MONEY_DIALOG,
	CHARACTER_DATES_DIALOG,
	REGION_CHARACTER_DIALOG,
	GIVE_MONEY_DIALOG,
	HEIR_DIALOG,
	FEUDAL_DIALOG,
	HOMAGE_DIALOG,
	PROMOTE_SOLDIER_DIALOG,
	DIPLOMACY_DIALOG,
	VALIDATE_DIALOG,
	HELP_DIALOG,
	INFORMATION,
	SELF_DECLARATION_DIALOG,
	GAME_OVER,
	SHUTDOWN
};

extern char *screens[];

struct region *selected_region;

struct tile *cursor;

int current_screen;

int check_termsize();

int get_input();

#endif
