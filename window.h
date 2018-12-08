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

int current_mode;

int gameover;

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

int draw_map(WINDOW *local_win);

int regions_dialog(WINDOW *local_win);

int rename_region_dialog(WINDOW *local_win);

int give_region_dialog(WINDOW *local_win);

int give_money_dialog(WINDOW *local_win);

int info_dialog(WINDOW *local_win);

int successor_dialog(WINDOW *local_win);

int feudal_dialog(WINDOW *local_win);

int homage_dialog(WINDOW *local_win);

int promote_soldier_dialog(WINDOW *local_win);

int diplomacy_dialog(WINDOW *local_win);

int help_dialog(WINDOW *local_win);

int self_declaration_dialog(WINDOW *local_win);

int editor_start_menu(WINDOW *local_win);

int map_editor(WINDOW *local_win);

int characters_dialog(WINDOW *local_win);

int add_character_dialog(WINDOW *local_win);

int editor_regions_dialog(WINDOW *local_win);

int add_region_dialog(WINDOW *local_win);

int region_to_character(WINDOW *local_win);

int edit_character_dialog(WINDOW *local_win);

int rename_character_dialog(WINDOW *local_win);

int change_character_money_dialog(WINDOW *local_win);

int change_character_dates_dialog(WINDOW *local_win);

int validate_dialog(WINDOW *local_win);

int edit_time_dialog(WINDOW *local_win);

int editor_successor_dialog(WINDOW *local_win);

int editor_homage_dialog(WINDOW *local_win);

int editor_diplomacy_dialog(WINDOW *local_win);

int editor_help_dialog(WINDOW *local_win);

#endif
