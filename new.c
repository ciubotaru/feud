#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>		/* for isdigit */
#include <math.h>		/* for log10 */
#include "world.h"

int new_game_dialog(WINDOW *local_win)
{
	wclear(local_win);
	wattrset(local_win, A_BOLD);
	curs_set(FALSE);
	noecho();

	int i, j;
	for (i = 0; i < (80 - strlen(screens[current_screen])) / 2; i++)
		wprintw(local_win, " ");
	wprintw(local_win, "%s", screens[current_screen]);

	wprintw(local_win,
		"\n\n  You are about to create a new game. This will destroy the old game and\n  overwrite the save file. Press 'y' to confirm.");
	int confirm = tolower(wgetch(local_win));
	if (confirm != 'y') {
		return 1;
	}

	destroy_world();
	create_world();

	wmove(local_win, 2, 0);
	wclrtoeol(local_win);
	wmove(local_win, 3, 0);
	wclrtoeol(local_win);
	wmove(local_win, 2, 2);

	curs_set(TRUE);
	echo();
	wprintw(local_win, "Map height (10-100, default 24): ");
	char h_ch[4];
	int h = 24;
	wgetnstr(local_win, h_ch, 3);
	for (i = 0; i < strlen(h_ch); i++) {
		if (!isdigit(h_ch[i]))
			break;
	}
	if (strlen(h_ch) > 0)
		h = atoi(h_ch);
	if (h == 0)
		h = 24;
	else if (h < 10)
		h = 10;
	else if (h > 100)
		h = 100;
	mvwprintw(local_win, 2, 35, "%3d", h);

	wprintw(local_win, "\n\n  Map width (10-100, default 80): ");
	char w_ch[4];
	int w = 80;
	wgetnstr(local_win, w_ch, 3);
	for (i = 0; i < strlen(w_ch); i++) {
		if (!isdigit(w_ch[i]))
			break;
	}
	if (strlen(w_ch) > 0)
		w = atoi(w_ch);
	if (w == 0)
		w = 80;
	else if (w < 10)
		w = 10;
	else if (w > 100)
		w = 100;
	mvwprintw(local_win, 4, 34, "%3d", w);

	int tiles_total = h * w;
	world->grid = create_grid(h, w);

	wprintw(local_win,
		"\n\n  Percentage of unwalkable terrain (0-50, default 10): ");
	char u_ch[3];
	int unwalkable_perc = 10;
	wgetnstr(local_win, u_ch, 2);
	for (i = 0; i < strlen(u_ch); i++) {
		if (!isdigit(u_ch[i]))
			break;
	}
	if (strlen(u_ch) > 0)
		unwalkable_perc = atoi(u_ch);
	if (unwalkable_perc < 0)
		unwalkable_perc = 0;
	else if (unwalkable_perc > 50)
		unwalkable_perc = 50;
	mvwprintw(local_win, 6, 55, "%2d", unwalkable_perc);

	int tiles_walkable = tiles_total * (100 - unwalkable_perc) / 100;
	if (unwalkable_perc > 0) {
		unsigned char **grid = create_height_grid();
		populate_height_grid(grid);
		blur_height_grid(grid);
		create_contiguous_area(grid, unwalkable_perc);
		delete_height_grid(grid);
		/* recount walkable tiles */
		tiles_walkable = 0;
		for (i = 0; i < world->grid->height; i++) {
			for (j = 0; j < world->grid->width; j++) {
				if (world->grid->tiles[i][j]->walkable) tiles_walkable++;
			}
		}
	}

	wprintw(local_win,
		"\n\n  Size of region (10-%i, default %i): ", MIN(tiles_walkable, 100), MIN(tiles_walkable, 36));
	char r_ch[8];
	int region_size = 36;
	wgetnstr(local_win, r_ch, 3);
	for (i = 0; i < strlen(r_ch); i++) {
		if (!isdigit(r_ch[i]))
			break;
	}
	if (strlen(r_ch) > 0)
		region_size = atoi(r_ch);
	if (region_size < 10)
		region_size = 10;
	else if (region_size > 100)
		region_size = 100;
	mvwprintw(local_win, 8, 38, "%3d", region_size);

	int nr_regions = tiles_walkable / region_size;
	if (nr_regions) {
		create_regions(nr_regions);
		voronoi(nr_regions);
		sort_region_list();
	}
	region_t *current_region = world->regionlist;
	region_t *next_region = NULL;
	while (current_region) {
		next_region = current_region->next;
		if (current_region->size == 0) {
			remove_region(current_region);
			nr_regions--;
		}
		current_region = next_region;
	}

	int p = (nr_regions - 1) / 5 + 1;
	wprintw(local_win, "\n\n  Number of players (1-%i, default %i): ", nr_regions, p);
	char p_ch[8];
	wgetnstr(local_win, p_ch, 7);
	for (i = 0; i < strlen(p_ch); i++) {
		if (!isdigit(p_ch[i]))
			break;
	}
	if (strlen(p_ch) > 0)
		p = atoi(p_ch);
	p = MAX(1, p);
	p = MIN(p, nr_regions);
	wmove(local_win, 10, 40);
	wclrtoeol(local_win);
	mvwprintw(local_win, 10, 40, "%i", p);

	int decimal = (int) floor(log10(p) + 2);
	char *name = malloc(6 + decimal);
	character_t *character = NULL;
	region_t *region = world->regionlist;
	piece_t *piece = NULL;
	int region_size_min = 100;
	tile_t *tile = NULL;
	for (i = 0; i < p; i++) {
		sprintf(name, "Player%0*d", decimal,  i + 1);
		character = add_character(name);
		set_character_rank(character, KING);
		change_region_owner(character, region);
		tile = region_center(region);
		piece = add_piece(NOBLE, tile->height, tile->width, character);
		if (region->size < region_size_min) region_size_min = region->size;
		region = region->next;
	}
	free(name);
	world->selected_character = world->characterlist;

	int s = MIN(3, region_size_min - 1);
	wprintw(local_win, "\n\n  Number of soldiers (0-%i, default %i): ", region_size_min - 1, s);
	char s_ch[8];
	wgetnstr(local_win, s_ch, 7);
	for (i = 0; i < strlen(s_ch); i++) {
		if (!isdigit(s_ch[i]))
			break;
	}
	if (strlen(s_ch) > 0)
		s = atoi(s_ch);
	s = MIN(s, region_size_min - 1);
	wmove(local_win, 12, 40);
	wclrtoeol(local_win);
	mvwprintw(local_win, 12, 40, "%i", s);

	character = world->characterlist;
	while (character) {
		region = get_noble_by_owner(character)->tile->region;
		for (i = 0; i < s; i++) {
			tile = get_empty_tile_in_region(region);
			if (tile) add_piece(SOLDIER, tile->height, tile->width, character);
		}
		character = character->next;
	}

	wprintw(local_win, "\n\n  Starting year (default 0): ");
	char y_ch[8];
	int y = 0;
	wgetnstr(local_win, y_ch, 7);
	for (i = 0; i < strlen(y_ch); i++) {
		if (!isdigit(y_ch[i]))
			break;
	}
	if (strlen(y_ch) > 0)
		y = atoi(y_ch);
	wmove(local_win, 14, 29);
	wclrtoeol(local_win);
	mvwprintw(local_win, 14, 29, "%i", y);

	wprintw(local_win, "\n\n  Starting month (1-12, default 1): ");
	char m_ch[3];
	int m = 0;
	wgetnstr(local_win, m_ch, 2);
	for (i = 0; i < strlen(m_ch); i++) {
		if (!isdigit(m_ch[i]))
			break;
	}
	if (strlen(m_ch) > 0)
		m = atoi(m_ch);
	if (m > 12)
		m = 12;
	if (m > 0)
		m--;
	wmove(local_win, 16, 36);
	wclrtoeol(local_win);
	mvwprintw(local_win, 16, 36, "%s", months[m]);

	world->current_time.tm_year = y;
	world->current_time.tm_mon = m;

	world->moves_left = get_dice();
	save_game();

	wprintw(local_win,
		"\n\n  Map created and saved. To continue, press any key.");
	curs_set(FALSE);
	noecho();
	wgetch(local_win);
	return 0;
}
