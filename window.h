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

#define MAIN_SCREEN 0
#define GAME_TIME_DIALOG 1
#define NEW_GAME 2
#define MAP_EDITOR 3
#define REGIONS_DIALOG 4
#define ADD_REGION_DIALOG 5
#define EDIT_REGION_DIALOG 6
#define GIVE_REGION_DIALOG 7
#define RENAME_REGION_DIALOG 8
#define PLAYERS_DIALOG 9
#define ADD_PLAYER_DIALOG 10
#define EDIT_PLAYER_DIALOG 11
#define RENAME_PLAYER_DIALOG 12
#define PLAYER_MONEY_DIALOG 13
#define PLAYER_DATES_DIALOG 14
#define REGION_PLAYER_DIALOG 15
#define GIVE_MONEY_DIALOG 16
#define HEIR_DIALOG 17
#define FEUDAL_DIALOG 18
#define HOMAGE_DIALOG 19
#define PROMOTE_SOLDIER_DIALOG 20
#define DIPLOMACY_DIALOG 21
#define VALIDATE_DIALOG 22
#define HELP_DIALOG 23
#define INFORMATION 24
#define GAME_OVER 25

char *screens[26];

int current_screen;

int check_termsize();

int get_input();

void set_cursor(uint16_t new_height, uint16_t new_width);

#endif
