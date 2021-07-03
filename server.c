#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>		/* for isdigit */
#include <unistd.h>
#include <sys/ioctl.h>		/* to get terminal size */
#include "world.h"

#define MAXLINE 1024

#define SETUP_LOOP 0
#define GAME_LOOP 1
#define QUIT -1

#define PROMPT "> "

int stage;

typedef struct {
	unsigned int size;
	char string[MAXLINE];
} buffer_t;

buffer_t *stdin_buffer;

void read_stdin()
{
	int char_received = fgetc(stdin);
	if (stdin_buffer->size == MAXLINE) {
		stdin_buffer->size = 0;
		return;
	}
	stdin_buffer->string[stdin_buffer->size] = char_received;
	stdin_buffer->string[stdin_buffer->size + 1] = 0;
	stdin_buffer->size++;
}

void reset()
{
	destroy_world();
	create_world();
}

void print_about()
{
	dprintf(STDOUT_FILENO, "Feud Server v%s\n", PACKAGE_VERSION);
	return;
}

void print_help_piece()
{
	dprintf(STDOUT_FILENO, "Parameters for 'piece' command:\n");
	dprintf(STDOUT_FILENO,
		" piece add <playerID> <type> <height> <width> - place a new piece type can be %i for %s or %i for %s\n",
		NOBLE, piece_name[NOBLE], SOLDIER, piece_name[SOLDIER]);
	dprintf(STDOUT_FILENO,
		" piece delete <height> <width> - remove a piece at given coordinates\n");
	dprintf(STDOUT_FILENO,
		" piece move <height1> <width1> <height2> <width2> - move a piece\n");
	return;
}

void print_help_player()
{
	dprintf(STDOUT_FILENO, "Parameters for 'player' command:\n");
	dprintf(STDOUT_FILENO, " player add <playerID> - create a new player\n");
	dprintf(STDOUT_FILENO, " player delete <playerID> - delete a player\n");
	dprintf(STDOUT_FILENO,
		" player money <playerID> <amount> - set player money (0-%i)\n",
		MONEY_MAX);
	dprintf(STDOUT_FILENO,
		" player name <playerID> <playerName> - set player name\n");
	dprintf(STDOUT_FILENO,
		" player rank <playerID> <playerRank> - set player rank ([k]ing, [d]uke, [c]ount or [b]aron)\n");
	return;
}

void print_help_region()
{
	dprintf(STDOUT_FILENO, "Parameters for 'region' command:\n");
	dprintf(STDOUT_FILENO, " region add <regionID> - create a new region\n");
	dprintf(STDOUT_FILENO, " region delete <regionID> - delete a region\n");
	dprintf(STDOUT_FILENO,
		" region name <regionID> <regionName> - set or change region name\n");
	dprintf(STDOUT_FILENO,
		" region owner <regionID> <playerID> - set or change region owner (0 for no owner)\n");
	return;
}

void print_help_status()
{
	dprintf(STDOUT_FILENO, "Parameters for 'status' command:\n");
	dprintf(STDOUT_FILENO, " status - print general information (map size, nr. of players, nr. of regions)\n");
	dprintf(STDOUT_FILENO, " status player <playerID> - print player status\n");
	dprintf(STDOUT_FILENO, " status playerlist - print list of players\n");
	dprintf(STDOUT_FILENO, " status region <regionID> - print region info\n");
	dprintf(STDOUT_FILENO, " status regionlist - print list of regions\n");
	return;
}

void print_help_tile()
{
	dprintf(STDOUT_FILENO, "Parameters for 'tile' command:\n");
	dprintf(STDOUT_FILENO,
		" tile region <height> <width> [<regionID>] - add a tile to region (to remove tile from region leave regionID empty)\n");
	dprintf(STDOUT_FILENO,
		" tile walkable <height> <width> <walkability> - set tile walkability (0 for unwalkable or 1 for walkable)\n");
	return;
}

void print_help(const char *topic)
{
	if (topic == NULL) {
		dprintf(STDOUT_FILENO, "Available commands:\n");
		dprintf(STDOUT_FILENO,
			" board <height> <width> - set game board size\n");
		dprintf(STDOUT_FILENO, " go - start the game\n");
		dprintf(STDOUT_FILENO, " load - load game from file\n");
		dprintf(STDOUT_FILENO, " new - clear everything\n");
		dprintf(STDOUT_FILENO,
			" piece ... - set up pieces (type 'help piece' for more info)\n");
		dprintf(STDOUT_FILENO,
			" ping – pong! (pinging players not implemented yet)\n");
		dprintf(STDOUT_FILENO,
			" player ... - set up a player (type 'help player' for more info)\n");
		dprintf(STDOUT_FILENO, " quit - terminate AI\n");
		dprintf(STDOUT_FILENO,
			" region ... - set up a region (type 'help region' for more info)\n");
		dprintf(STDOUT_FILENO, " save - write current game to file\n");
		dprintf(STDOUT_FILENO,
			" status ... - print print game information (type 'help status' for more info)\n");
		dprintf(STDOUT_FILENO,
			" tile ... - set up a tile (type 'help tile' for more info\n");
		dprintf(STDOUT_FILENO, " turn <playerID> - set player's turn\n");
		dprintf(STDOUT_FILENO, " validate - check game data playability\n");
		dprintf(STDOUT_FILENO, "For details, type 'help [command]'.\n");
		return;
	}
	if (strcmp(topic, "piece") == 0) {
		print_help_piece();
		return;
	}
	if (strcmp(topic, "player") == 0) {
		print_help_player();
		return;
	}
	if (strcmp(topic, "region") == 0) {
		print_help_region();
		return;
	}
	if (strcmp(topic, "status") == 0) {
		print_help_status();
		return;
	}
	if (strcmp(topic, "tile") == 0) {
		print_help_tile();
		return;
	}
	dprintf(STDOUT_FILENO, "%s: no help for this topic\n", topic);
}

void print_print(uint16_t w, uint16_t h) {
	struct winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	ws.ws_row -= 1;
	if (w >= world->grid->width) w = MAX(world->grid->width - ws.ws_col, 0);
	if (h >= world->grid->height) h = MAX(world->grid->height - ws.ws_row, 0);
	uint16_t w_max = MIN(w + ws.ws_col, world->grid->width);
	uint16_t h_max = MIN(h + ws.ws_row, world->grid->height);
	char tile_char = '.';
	int i, j;
	for (i = h_max - 1; i >= h; i--) {
		for (j = w; j < w_max; j++) {
			if (world->grid->tiles[i][j]->piece != NULL) {
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
				case SHIP:	/* ship */
					tile_char = 'S';
					break;
				}
			}
			else if (world->grid->tiles[i][j]->walkable) tile_char = '.';
			else tile_char = '~';
			dprintf(STDOUT_FILENO, "%c", tile_char);
		}
		dprintf(STDOUT_FILENO, "\n");
	}
}

void print_status()
{
	dprintf(STDOUT_FILENO, "Game status:\n");
	if (world == NULL) {
		dprintf(STDOUT_FILENO, " Game world not created\n");
		return;
	}
	if (world->grid == NULL) {
		dprintf(STDOUT_FILENO, " Board not created\n");
		return;
	}
	dprintf(STDOUT_FILENO,
		" Date: %s of year %d\n",
		months[world->current_time.tm_mon],
		world->current_time.tm_year);
	dprintf(STDOUT_FILENO,
		" Map size: %ix%i\n",
		world->grid->width,
		world->grid->height);
	dprintf(STDOUT_FILENO,
		" Regions: %u\n",
		count_regions());
	dprintf(STDOUT_FILENO,
		" Players: %u\n",
		count_characters());
	character_t *turn = world->selected_character;
	dprintf(STDOUT_FILENO,
		"Player to move: %s\n",
		(turn ? turn->name : "none"));
	dprintf(STDOUT_FILENO,
		" Moves left: %i\n",
		world->moves_left);
	return;
}

void print_status_player(character_t *character) {
	if (character == NULL) {
		dprintf(STDOUT_FILENO,
			"Error: No player with this ID\n");
		return;
	}
	dprintf(STDOUT_FILENO, "ID: %i\n", character->id);
	dprintf(STDOUT_FILENO, "Name: %s\n", character->name);
	dprintf(STDOUT_FILENO, "Rank: %s\n", rank_name[character->rank]);
	dprintf(STDOUT_FILENO, "Born: %s of %i\n", months[character->birthdate.tm_mon], character->birthdate.tm_year);
	uint16_t age_months = 12 * (world->current_time.tm_year - character->birthdate.tm_year) + world->current_time.tm_mon - character->birthdate.tm_mon;
	dprintf(STDOUT_FILENO, "Age: %i year(s) %i month(s)\n", age_months / 12, age_months % 12);
	dprintf(STDOUT_FILENO, "Die: %s of %i\n", months[character->deathdate.tm_mon], character->deathdate.tm_year);
	uint16_t left_months = 12 * (character->deathdate.tm_year - world->current_time.tm_year) + character->deathdate.tm_mon - world->current_time.tm_mon;
	dprintf(STDOUT_FILENO, "Left: %i year(s) %i month(s)\n", left_months / 12, left_months % 12);
	dprintf(STDOUT_FILENO, "Regions: %u (%u tiles)\n", count_regions_by_owner(character), count_tiles_by_owner(character));
	dprintf(STDOUT_FILENO, "Army: %u\n", count_pieces_by_owner(character));
	dprintf(STDOUT_FILENO, "Money: %u\n", get_money(character));
	character_t *lord = get_liege(character);
	dprintf(STDOUT_FILENO, "Lord: %s\n", (lord ? lord->name : "none"));
	dprintf(STDOUT_FILENO, "Vassals: %u\n", count_vassals(character));
	if (world->selected_character == character) dprintf(STDOUT_FILENO,
		"Moves left: %i\n",
		world->moves_left);
}

void print_status_region(region_t *region) {
	if (region == NULL) {
		dprintf(STDOUT_FILENO,
			"Error: No region with this ID\n");
		return;
	}
	dprintf(STDOUT_FILENO, "ID: %i\n", region->id);
	dprintf(STDOUT_FILENO, "Name: %s\n", region->name);
	dprintf(STDOUT_FILENO, "Size: %i\n", region->size);
	dprintf(STDOUT_FILENO, "Owner: %s\n", (region->owner ? region->owner->name : "none"));
}

void setup_loop()
{
	char command[MAXLINE];
	unsigned char print_prompt = 1;
	while (stage == SETUP_LOOP) {
		if (print_prompt) dprintf(STDOUT_FILENO, "%s", PROMPT);
		print_prompt = 1;
		read_stdin();
		if (stdin_buffer->size == 0
		    || stdin_buffer->string[stdin_buffer->size - 1] != '\n') {
			print_prompt = 0;
			continue;
		}
		if (stdin_buffer->string[0] == '\n') {
			stdin_buffer->size = 0;
			continue;
		}
		strcpy(command, stdin_buffer->string);
		stdin_buffer->size = 0;
		char *token = strtok(command, " \n");	/* remove trailing newline */
		if (!token) continue;
		if (!strcmp(token, "about")) {
			print_about();
			continue;
		}
		if (!strcmp(token, "board")) {
			uint16_t coords[2];
			int i;
			int success = 1;
			for (i = 0; i < 2; i++) {
				token = strtok(NULL, " \n");
				if (!token) {
					dprintf(STDOUT_FILENO,
						"Error: missing argument\n");
					success = 0;
					break;
				}
				coords[i] = (uint16_t) atoi(token);
				if (coords[i] == 0) {
					dprintf(STDOUT_FILENO,
						"Error: bad argument\n");
					success = 0;
					break;
				}
			}
			if (success) {
				if (world == NULL) create_world();
				if (world->grid != NULL) {
					dprintf(STDOUT_FILENO, "Error: bad command (board already set, use 'new')\n");
					continue;
				}
				grid_t *grid = create_grid(coords[0], coords[1]);
				if (grid == NULL) dprintf(STDOUT_FILENO, "Error: internal error (failed to create grid)\n");
				else dprintf(STDOUT_FILENO, "OK\n");
			}
			continue;
		}
		if (!strcmp(token, "go")) {
			dprintf(STDOUT_FILENO, "not implemented yet\n");
			continue;
		}
		if (!strcmp(token, "help")) {
			token = strtok(NULL, " \n");
			print_help(token);
			continue;
		}
		if (!strcmp(token, "load")) {
			load_game();
			dprintf(STDOUT_FILENO, "OK\n");
			continue;
		}
		if (!strcmp(command, "new")) {
			reset();
			dprintf(STDOUT_FILENO, "OK\n");
			continue;
		}
		if (!strcmp(token, "piece")) {
			token = strtok(NULL, " \n");
			if (!token) {
				dprintf(STDOUT_FILENO,
					"Error: Parameter missing (type 'help piece' for more info)\n");
				continue;
			}
			/* can be 'add', 'delete' or 'move' */
			if (!strcmp(token, "add")) {
				char *character_id_ch = strtok(NULL, " \n");
				if (!character_id_ch) {
					dprintf(STDOUT_FILENO,
							"Error: PlayerID missing (type 'help piece' for more info)\n");
					continue;
				}
				uint16_t character_id =
					(uint16_t) atoi(character_id_ch);
				if (character_id < 1) {
					dprintf(STDOUT_FILENO,
							"Error: bad PlayerID\n");
					continue;
				}
				character_t *character = get_character_by_id(character_id);
				if (character == NULL) {
					dprintf(STDOUT_FILENO,
							"Error: invalid PlayerID\n");
					continue;
				}
				char *piece_type_ch = strtok(NULL, " \n");
				if (!piece_type_ch) {
					dprintf(STDOUT_FILENO,
							"Error: PieceType missing (type 'help piece' for more info)\n");
					continue;
				}
				unsigned char piece_type =
					(unsigned char)atoi(piece_type_ch);
				if (piece_type > 1) {
					dprintf(STDOUT_FILENO,
							"Error: invalid PieceType (type 'help piece' for more info)\n");
					continue;
				}
				uint16_t coords[2];
				int i;
				int success = 1;
				for (i = 0; i < 2; i++) {
					token = strtok(NULL, " \n");
					if (!token) {
						success = 0;
						break;
					}
					coords[i] = (uint16_t) atoi(token);
				}
				if (!success || coords[0] >= world->grid->height || coords[1] >= world->grid->width) {
					dprintf(STDOUT_FILENO,
							"Error: invalid piece coordinates (type 'help piece' for more info)\n");
					continue;
				}
				piece_t *piece = add_piece(piece_type, coords[0], coords[1], character);
				if (piece) dprintf(STDOUT_FILENO, "OK\n");
				else dprintf(STDOUT_FILENO, "Error: failed to add piece\n");
				continue;
			}
			if (!strcmp(token, "delete")) {
				uint16_t coords[2];
				int i;
				int success = 1;
				for (i = 0; i < 2; i++) {
					token = strtok(NULL, " \n");
					if (!token) {
						success = 0;
						break;
					}
					coords[i] = (uint16_t) atoi(token);
				}
				if (!success || coords[0] >= world->grid->height
				    || coords[1] >= world->grid->width) {
					dprintf(STDOUT_FILENO,
						"Error: invalid piece coordinates (type 'help piece' for more info)\n");
					continue;
				}
				piece_t *piece =
				    world->grid->tiles[coords[0]][coords[1]]->
				    piece;
				if (!piece)
					dprintf(STDOUT_FILENO,
						"Error: piece not found\n");
				else {
					dprintf(STDOUT_FILENO, "OK\n");
					remove_piece(piece);
				}
				continue;
			}
			if (!strcmp(token, "move")) {
				uint16_t coords[4];
				int i;
				int success = 1;
				for (i = 0; i < 4; i++) {
					token = strtok(NULL, " \n");
					if (!token) {
						success = 0;
						break;
					}
					coords[i] = (uint16_t) atoi(token);
				}
				if (!success || coords[0] >= world->grid->height
				    || coords[1] >= world->grid->width
				    || coords[2] >= world->grid->height
				    || coords[3] >= world->grid->width) {
					dprintf(STDOUT_FILENO,
						"Error: invalid piece coordinates (type 'help piece' for more info)\n");
					continue;
				}
				piece_t *piece =
				    world->grid->tiles[coords[0]][coords[1]]->
				    piece;
				if (!piece) {
					dprintf(STDOUT_FILENO,
						"Error: piece not found\n");
					continue;
				}
				int result =
				    move_piece(piece, coords[2], coords[3]);
				if (result == 0)
					dprintf(STDOUT_FILENO, "OK\n");
				else
					dprintf(STDOUT_FILENO,
						"Error: illegal move\n");
				continue;
			} else
				dprintf(STDOUT_FILENO,
					"Error: Unknown parameter (type 'help piece' for more info)\n");
			continue;
		}
		if (!strcmp(command, "ping")) {
			dprintf(STDOUT_FILENO, "pong\n");
			continue;
		}
		if (!strcmp(token, "player")) {
			token = strtok(NULL, " \n");
			if (!token) {
				dprintf(STDOUT_FILENO,
					"Error: Parameter missing (type 'help player' for more info)\n");
				continue;
			}
			/* can be 'add', 'delete', 'money', 'name' or 'rank' */
			if (!strcmp(token, "add")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerID missing (type 'help player' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				if (id < 1) {
					dprintf(STDOUT_FILENO,
						"Error: bad PlayerID\n");
					continue;
				}
				if (get_character_by_id(id) != NULL) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerID not unique\n");
					continue;
				}
				character_t *character = add_character(id_ch);
				character->id = id;
				dprintf(STDOUT_FILENO, "OK\n");
				continue;
			}
			if (!strcmp(token, "delete")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerID missing (type 'help player' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				character_t *character = get_character_by_id(id);
				if (character == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: invalid PlayerID\n");
					continue;
				}
				remove_character(character);
				dprintf(STDOUT_FILENO, "OK\n");
				continue;
			}
			if (!strcmp(token, "money")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerID missing (type 'help player' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				character_t *character = get_character_by_id(id);
				if (character == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: no such player (type 'help player' for more info)\n");
					continue;
				}
				char *money_ch = strtok(NULL, " \n");
				uint16_t money = 0;
				if (money_ch) money = (uint16_t) atoi(money_ch);
				set_money(character, money);
				dprintf(STDOUT_FILENO, "OK\n");
				continue;
			}
			if (!strcmp(token, "name")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerID missing (type 'help player' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				character_t *character = get_character_by_id(id);
				if (character == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: no such player (type 'help player' for more info)\n");
					continue;
				}
				char *name = strtok(NULL, " \n");
				if (!name) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerName missing (type 'help player' for more info)\n");
					continue;
				}
				if (!strcmp(character->name, name)) {
					dprintf(STDOUT_FILENO, "OK\n");
					continue;
				}
				if (get_character_by_name(name) != NULL) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerName not unique\n");
					continue;
				}
				strcpy(character->name, name);
				dprintf(STDOUT_FILENO, "OK\n");
				continue;
			}
			if (!strcmp(token, "rank")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerID missing (type 'help player' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				character_t *character = get_character_by_id(id);
				if (character == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: no such player (type 'help player' for more info)\n");
					continue;
				}
				char *rank_ch = strtok(NULL, " \n");
				int rank = 0;
				int success = 1;
				if (!rank_ch) {
					dprintf(STDOUT_FILENO,
						"Error: invalid rank (type 'help player' for more info)\n");
					success = 0;
					continue;
				}
				switch (rank_ch[0]) {
				case 'k':
					rank = 4;
					break;
				case 'd':
					rank = 3;
					break;
				case 'c':
					rank = 2;
					break;
				case 'b':
					rank = 1;
					break;
				default:
					dprintf(STDOUT_FILENO,
						"Error: invalid rank (type 'help player' for more info)\n");
					success = 0;
					break;
				}
				if (!success)
					continue;
				set_character_rank(character, rank);
				dprintf(STDOUT_FILENO, "OK\n");
				continue;
			} else
				dprintf(STDOUT_FILENO,
					"Error: Unknown parameter (type 'help player' for more info)\n");
			continue;
		}
		if (!strcmp(command, "print")) {
			if (world->grid == NULL) {
				dprintf(STDOUT_FILENO,
					"Error: board not set\n");
				continue;
			}
			uint16_t coords[2];
			int i;
			for (i = 0; i < 2; i++) {
				token = strtok(NULL, " \n");
				if (!token) coords[i] = 0;
				else coords[i] = (uint16_t) atoi(token);
			}
			print_print(coords[0], coords[1]);
			continue;
		}
		if (!strcmp(command, "quit")) {
			dprintf(STDOUT_FILENO, "Quitting...\n");
			stage = QUIT;
			continue;
		}
		if (!strcmp(token, "region")) {
			token = strtok(NULL, " \n");
			if (!token) {
				dprintf(STDOUT_FILENO,
					"Error: Parameter missing (type 'help region' for more info)\n");
				continue;
			}
			/* can be 'add', 'delete', 'name' or 'owner' */
			if (!strcmp(token, "add")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: RegionID missing (type 'help region' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				if (get_region_by_id(id) != NULL) {
					dprintf(STDOUT_FILENO,
						"Error: RegionID not unique\n");
					continue;
				}
				region_t *region = add_region(id_ch);
				region->id = id;
				dprintf(STDOUT_FILENO, "OK\n");
				continue;
			}
			if (!strcmp(token, "delete")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: RegionID missing (type 'help region' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				region_t *region = get_region_by_id(id);
				if (region == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: invalid RegionID\n");
					continue;
				}
				remove_region(region);
				dprintf(STDOUT_FILENO, "OK\n");
				continue;
			}
			if (!strcmp(token, "name")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: RegionID missing (type 'help region' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				region_t *region = get_region_by_id(id);
				if (region == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: no such region (type 'help region' for more info)\n");
					continue;
				}
				char *name = strtok(NULL, "\n");
				if (!name) {
					dprintf(STDOUT_FILENO,
						"Error: RegionName missing (type 'help region' for more info)\n");
					continue;
				}
				if (!strcmp(region->name, name)) {
					dprintf(STDOUT_FILENO, "OK\n");
					continue;
				}
				if (get_region_by_name(name) != NULL) {
					dprintf(STDOUT_FILENO,
						"Error: RegionName not unique\n");
					continue;
				}
				strcpy(region->name, name);
				dprintf(STDOUT_FILENO, "OK\n");
				continue;
			}
			if (!strcmp(token, "owner")) {
				char *region_id_ch = strtok(NULL, " \n");
				if (!region_id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: RegionID missing (type 'help region' for more info)\n");
					continue;
				}
				uint16_t region_id =
				    (uint16_t) atoi(region_id_ch);
				region_t *region = get_region_by_id(region_id);
				if (region == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: no such region (type 'help region' for more info)\n");
					continue;
				}
				char *character_id_ch = strtok(NULL, " \n");
				if (!character_id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerID missing (type 'help region' for more info)\n");
					continue;
				}
				uint16_t character_id =
				    (uint16_t) atoi(character_id_ch);
				character_t *character = NULL;
				character = get_character_by_id(character_id);
				if (character_id > 0 && character == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: no such player (type 'help region' for more info)\n");
					continue;
				}
				change_region_owner(character, region);
				dprintf(STDOUT_FILENO, "OK\n");
				continue;
			} else
				dprintf(STDOUT_FILENO,
					"Error: Unknown parameter (type 'help region' for more info)\n");
			continue;
		}
		if (!strcmp(token, "save")) {
			int result = validate_game_data();
			if (result == 0) {
				dprintf(STDOUT_FILENO, "OK\n");
				save_game();
			}
			else {
				dprintf(STDOUT_FILENO, "Error: Not saving. %s\n", ((world && strlen(world->message) > 0) ? world->message : "Unknown error"));
			}
			continue;
		}
		if (!strcmp(token, "status")) {
			token = strtok(NULL, " \n");
			if (!token) {
				print_status();
				continue;
			}
			if (!strcmp(token, "player")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerID missing\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				if (id < 1) {
					dprintf(STDOUT_FILENO,
						"Error: bad PlayerID\n");
					continue;
				}
				character_t *character = get_character_by_id(id);
				print_status_player(character);
			}
			else if (!strcmp(token, "playerlist")) {
				if (world == NULL) {
					dprintf(STDOUT_FILENO, "World does not exist\n");
					continue;
				}
				if (world->characterlist == NULL) {
					dprintf(STDOUT_FILENO, "No players\n");
					continue;
				}
				character_t *current = world->characterlist;
				while (current != NULL) {
					printf("%s %u. %s, %s, %u coins, %u soldier(s), %u region(s), %u tile(s)\n", (current == world->selected_character ? "*" : " "), current->id, current->name, rank_name[current->rank], current->money, count_pieces_by_owner(current), count_regions_by_owner(current), count_tiles_by_owner(current));
					current = current->next;
				}
			}
			else if (!strcmp(token, "region")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: RegionID missing\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				if (id < 1) {
					dprintf(STDOUT_FILENO,
						"Error: bad RegionID\n");
					continue;
				}
				region_t *region = get_region_by_id(id);
				print_status_region(region);
			}
			else if (!strcmp(token, "regionlist")) {
				if (world == NULL) {
					dprintf(STDOUT_FILENO, "World does not exist\n");
					continue;
				}
				if (world->regionlist == NULL) {
					dprintf(STDOUT_FILENO, "No regions\n");
					continue;
				}
				region_t *current = world->regionlist;
				while (current != NULL) {
					/* Add nr of tiles and owner */
					printf("%i. %s, %i tile(s), owned by %s\n", current->id, current->name, current->size, (current->owner ? current->owner->name : "none"));
					current = current->next;
				}
			}
			else dprintf(STDOUT_FILENO, "Error: invalid parameter (type 'help status' for more info)\n");
			continue;
		}
		if (!strcmp(token, "tile")) {
			if (world->grid == NULL) {
				dprintf(STDOUT_FILENO,
					"Error: board not set\n");
				continue;
			}
			token = strtok(NULL, " \n");
			if (!token) {
				dprintf(STDOUT_FILENO,
					"Error: Parameter missing (type 'help tile' for more info)\n");
				continue;
			}
			/* can be 'region' or 'walkable' */
			if (!strcmp(token, "region")) {
				uint16_t coords[2];
				int i;
				int success = 1;
				for (i = 0; i < 2; i++) {
					token = strtok(NULL, " \n");
					if (!token) {
						success = 0;
						break;
					}
					coords[i] = (uint16_t) atoi(token);
				}
				if (!success || coords[0] >= world->grid->height
					|| coords[1] >= world->grid->width) {
					dprintf(STDOUT_FILENO,
						"Error: invalid tile coordinates (type 'help tile' for more info)\n");
					continue;
				}
				tile_t *tile = world->grid->tiles[coords[0]][coords[1]];
				char *region_id_ch = strtok(NULL, " \n");
				if (region_id_ch == NULL) {
					dprintf(STDOUT_FILENO, "OK\n");
					change_tile_region(NULL, tile);
					continue;
				}
				if (world->regionlist == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: no region list\n");
					continue;
				}
				uint16_t region_id = (uint16_t) atoi(region_id_ch);
				region_t *region = get_region_by_id(region_id);
				if (region == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: region not found\n");
					continue;
				}
				dprintf(STDOUT_FILENO, "OK\n");
				change_tile_region(region, tile);
				continue;
			}
			if (!strcmp(token, "walkable")) {
				uint16_t coords[2];
				int i;
				int success = 1;
				for (i = 0; i < 2; i++) {
					token = strtok(NULL, " \n");
					if (!token) {
						success = 0;
						break;
					}
					coords[i] = (uint16_t) atoi(token);
				}
				if (!success || coords[0] >= world->grid->height
					|| coords[1] >= world->grid->width) {
					dprintf(STDOUT_FILENO,
						"Error: invalid tile coordinates (type 'help tile' for more info)\n");
					continue;
				}
				tile_t *tile = world->grid->tiles[coords[0]][coords[1]];
				char *walkable_ch = strtok(NULL, " \n");
				if (walkable_ch == NULL) {
					dprintf(STDOUT_FILENO, "Error: Parameter missing  (type 'help tile' for more info)\n");
					continue;
				}
				unsigned char walkable = (unsigned char) atoi(walkable_ch);
				if (walkable > 1) {
					dprintf(STDOUT_FILENO, "Error: Invalid parameter (type 'help tile' for more info)\n");
					continue;
				}
				dprintf(STDOUT_FILENO, "OK\n");
				tile->walkable = walkable;
			} else dprintf(STDOUT_FILENO, "Error: Unknown parameter\n");
			continue;
		}
		if (!strcmp(token, "turn")) {
			if (world->characterlist == NULL) {
				dprintf(STDOUT_FILENO, "Error: no player list\n");
				continue;
			}
			char *character_id_ch = strtok(NULL, " \n");
			if (!character_id_ch) {
				dprintf(STDOUT_FILENO, "Error: Parameter missing\n");
				continue;
			}
			uint16_t character_id = (uint16_t) atoi(character_id_ch);
			if (get_character_by_id(character_id) == NULL) {
//				dprintf(STDOUT_FILENO, "Error: no such player (type 'help turn' for more info)\n");
				dprintf(STDOUT_FILENO, "Error: no such player\n");
				continue;
			}
			dprintf(STDOUT_FILENO, "OK\n");
			world->selected_character = get_character_by_id(character_id);
			continue;
		}
		if (!strcmp(token, "validate")) {
			int result = validate_game_data();
			if (result == 0) dprintf(STDOUT_FILENO, "OK\n");
			else {
				dprintf(STDOUT_FILENO, "Error: %s\n", world->message);
			}
			continue;
		} else
			dprintf(STDOUT_FILENO, "Error: Unknown command.\n");
	}
}

void game_loop()
{
	while (stage == GAME_LOOP) {
		/* we communicate with bots/players here */
		dprintf(STDOUT_FILENO, "game over\n");
		dprintf(STDOUT_FILENO, "clearing gamedata ... OK\n");
		stage = SETUP_LOOP;
		continue;
	}
}

int main(int argc, char **argv)
{
	reset();
	print_about();
	stdin_buffer = malloc(sizeof(buffer_t));
	if (!stdin_buffer) exit(EXIT_FAILURE);
	stdin_buffer->size = 0;
	stage = SETUP_LOOP;
	while (1) {
		switch (stage) {
		case SETUP_LOOP:
			setup_loop();
			break;
		case GAME_LOOP:
			game_loop();
			break;
		default:
			return 0;
		}
	}
	return 0;
}
