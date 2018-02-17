#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "world.h"

#define STANDBY 0
#define THINK 1
#define GAMEOVER 2
#define MAXLINE 1024
#define PROMPT "> "

int stage = -1;
int side;
player_t *ai_player = NULL;

typedef struct {
	unsigned int size;
	char string[MAXLINE];
} buffer_t;

buffer_t *stdin_buffer;

void read_stdin()
{
	int char_received = fgetc(stdin);
	stdin_buffer->string[stdin_buffer->size] = char_received;
	stdin_buffer->string[stdin_buffer->size + 1] = 0;
	stdin_buffer->size++;
}

void reset()
{
	destroy_world();
	create_world();
}

void print_help_piece()
{
	dprintf(STDOUT_FILENO, "Parameters for 'piece' command:\n");
	dprintf(STDOUT_FILENO,
		" piece add <playerID> <type> <height> <width> - place a new piece type can be %i for %s or %i for %s\n",
		NOBLE, unit_type_list[NOBLE], SOLDIER, unit_type_list[SOLDIER]);
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
		" player rank <playerID> <playerRank> - set player rank([k]ing, [d]uke, [c]ount or [b]aron\n");
	return;
}

void print_help_region()
{
	dprintf(STDOUT_FILENO, "Parameters for 'region' command:\n");
	dprintf(STDOUT_FILENO, " region add <regionID> - create a new region\n");
	dprintf(STDOUT_FILENO, " region delete <regionID> - delete a region\n");
	dprintf(STDOUT_FILENO,
		" region name <regionID> <regionName> - set region name\n");
	dprintf(STDOUT_FILENO,
		" region owner <regionID> <playerID> - set region owner (to clear ownership, write playerID 0)\n");
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

void print_help_turn()
{
	dprintf(STDOUT_FILENO, "Parameters for 'turn' command:\n");
	dprintf(STDOUT_FILENO,
		" turn <playerID> - set player's turn\n");
	return;
}

void print_help(const char *topic)
{
	if (topic == NULL) {
		dprintf(STDOUT_FILENO, "Available commands:\n");
		dprintf(STDOUT_FILENO,
			" board <height> <width> - set game board size\n");
		dprintf(STDOUT_FILENO, " load - load game from file\n");
		dprintf(STDOUT_FILENO, " new - clear everything\n");
		dprintf(STDOUT_FILENO, " ping - check if AI is alive\n");
		dprintf(STDOUT_FILENO,
			" piece ... - set up pieces (type 'help piece' for more info)\n");
		dprintf(STDOUT_FILENO,
			" play <PlayerID> – assign AI to PlayerID\n");
		dprintf(STDOUT_FILENO,
			" player ... - set up a player (type 'help player' for more info)\n");
		dprintf(STDOUT_FILENO, " quit - terminate AI\n");
		dprintf(STDOUT_FILENO,
			" region ... - set up a region (type 'help region' for more info)\n");
		dprintf(STDOUT_FILENO,
			" roll <number> - roll a dice (from 1 to 6)\n");
		dprintf(STDOUT_FILENO, " save - write current game to file\n");
		dprintf(STDOUT_FILENO,
			" tile ... - set up a tile (type 'help tile'\nfor more info\n");
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
	if (strcmp(topic, "tile") == 0) {
		print_help_tile();
		return;
	}
	if (strcmp(topic, "turn") == 0) {
		print_help_turn();
		return;
	}
	dprintf(STDOUT_FILENO, "%s: no help for this topic\n", topic);
}

int set_grid(const uint16_t height, const uint16_t width)
{
	if (world == NULL)
		create_world();
	if (world->grid != NULL) {
		dprintf(STDOUT_FILENO,
			"Error: bad command (board already set, use 'new')\n");
		return 1;
	}
	grid_t *grid = create_grid(height, width);
	if (grid == NULL) {
		dprintf(STDOUT_FILENO,
			"Error: internal error (failed to create grid)\n");
		return 1;
	}
	return 0;
}

void standby()
{
	char command[MAXLINE];
	unsigned char print_prompt = 1;
	while (1) {
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
		if (!strcmp(token, "board")) {
			uint16_t coords[2];
			int i;
			int success = 1;
			for (i = 0; i < 2; i++) {
				token = strtok(NULL, " \n");
				if (!token) {
					print_help("board");
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
				set_grid(coords[0], coords[1]);
				dprintf(STDOUT_FILENO, "ack\n");
			}
			continue;
		}
		if (!strcmp(token, "go")) {
			char *msg = NULL;
			int result = validate_game_data(&msg);
			if (result != 0) {
				dprintf(STDOUT_FILENO, "Error: %s\n", msg);
				free(msg);
				continue;
			}
			if (ai_player == NULL) {
				dprintf(STDOUT_FILENO, "Error: AI not assigned to any side (run 'play' first)\n");
				continue;
			}
			if (world->selected_player != ai_player) {
				dprintf(STDOUT_FILENO, "Error: not AI's turn to play\n");
				continue;
			}
			stage = THINK;
			return;
		}
		if (!strcmp(token, "help")) {
			token = strtok(NULL, " \n");
			print_help(token);
			continue;
		}
		if (!strcmp(token, "load")) {
			load_game();
			dprintf(STDOUT_FILENO, "ack\n");
			continue;
		}
		if (!strcmp(token, "new")) {
			reset();
			dprintf(STDOUT_FILENO, "ack\n");
			continue;
		}
		if (!strcmp(token, "ping")) {
			dprintf(STDOUT_FILENO, "pong\n");
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
				char *player_id_ch = strtok(NULL, " \n");
				if (!player_id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerID missing (type 'help piece' for more info)\n");
					continue;
				}
				uint16_t player_id =
				    (uint16_t) atoi(player_id_ch);
				if (player_id < 1) {
					dprintf(STDOUT_FILENO,
						"Error: bad PlayerID\n");
					continue;
				}
				player_t *player = get_player_by_id(player_id);
				if (player == NULL) {
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
				if (!success || coords[0] >= world->grid->height
				    || coords[1] >= world->grid->width) {
					dprintf(STDOUT_FILENO,
						"Error: invalid piece coordinates (type 'help piece' for more info)\n");
					continue;
				}
				piece_t *piece =
				    add_piece(piece_type, coords[0], coords[1],
					      player);
				if (piece)
					dprintf(STDOUT_FILENO, "ack\n");
				else
					dprintf(STDOUT_FILENO,
						"Error: failed to add piece\n");
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
					dprintf(STDOUT_FILENO, "ack\n");
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
					dprintf(STDOUT_FILENO, "ack\n");
				else
					dprintf(STDOUT_FILENO,
						"Error: illegal move\n");
				continue;
			} else
				dprintf(STDOUT_FILENO,
					"Error: Unknown parameter (type 'help piece' for more info)\n");
			continue;
		}
		if (!strcmp(token, "play")) {
			token = strtok(NULL, " \n");
			if (!token) {
				dprintf(STDOUT_FILENO,
					"Error: Parameter missing\n");
				continue;
			}
			uint16_t id = (uint16_t) atoi(token);
			ai_player = get_player_by_id(id);
			if (ai_player == NULL) {
				dprintf(STDOUT_FILENO,
					"Error: bad PlayerID\n");
				continue;
			}
			dprintf(STDOUT_FILENO, "ack\n");
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
				if (get_player_by_id(id) != NULL) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerID not unique\n");
					continue;
				}
				player_t *player = add_player(id_ch);
				player->id = id;
				dprintf(STDOUT_FILENO, "ack\n");
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
				player_t *player = get_player_by_id(id);
				if (player == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: invalid PlayerID\n");
					continue;
				}
				remove_player(player);
				dprintf(STDOUT_FILENO, "ack\n");
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
				player_t *player = get_player_by_id(id);
				if (player == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: no such player (type 'help player' for more info)\n");
					continue;
				}
				char *money_ch = strtok(NULL, " \n");
				uint16_t money = (uint16_t) atoi(money_ch);
				set_money(player, money);
				dprintf(STDOUT_FILENO, "ack\n");
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
				player_t *player = get_player_by_id(id);
				if (player == NULL) {
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
				if (!strcmp(player->name, name)) {
					dprintf(STDOUT_FILENO, "ack\n");
					continue;
				}
				if (get_player_by_name(name) != NULL) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerName not unique\n");
					continue;
				}
				strcpy(player->name, name);
				dprintf(STDOUT_FILENO, "ack\n");
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
				player_t *player = get_player_by_id(id);
				if (player == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: no such player (type 'help player' for more info)\n");
					continue;
				}
				char *rank_ch = strtok(NULL, " \n");
				int rank = 0;
				int success = 1;
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
				set_player_rank(player, rank);
				dprintf(STDOUT_FILENO, "ack\n");
				continue;
			} else
				dprintf(STDOUT_FILENO,
					"Error: Unknown parameter (type 'help player' for more info)\n");
			continue;
		}
		if (!strcmp(token, "quit")) {
			dprintf(STDOUT_FILENO, "Quitting...\n");
			stage = GAMEOVER;
			return;
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
				dprintf(STDOUT_FILENO, "ack\n");
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
				dprintf(STDOUT_FILENO, "ack\n");
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
						"Error: RegionName missing (type 'help player' for more info)\n");
					continue;
				}
				if (!strcmp(region->name, name)) {
					dprintf(STDOUT_FILENO, "ack\n");
					continue;
				}
				if (get_region_by_name(name) != NULL) {
					dprintf(STDOUT_FILENO,
						"Error: RegionName not unique\n");
					continue;
				}
				strcpy(region->name, name);
				dprintf(STDOUT_FILENO, "ack\n");
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
				char *player_id_ch = strtok(NULL, " \n");
				if (!player_id_ch) {
					dprintf(STDOUT_FILENO,
						"Error: PlayerID missing (type 'help region' for more info)\n");
					continue;
				}
				uint16_t player_id =
				    (uint16_t) atoi(player_id_ch);
				player_t *player = NULL;
				player = get_player_by_id(player_id);
				if (player_id > 0 && player == NULL) {
					dprintf(STDOUT_FILENO,
						"Error: no such player (type 'help region' for more info)\n");
					continue;
				}
				change_region_owner(player, region);
				dprintf(STDOUT_FILENO, "ack\n");
				continue;
			} else
				dprintf(STDOUT_FILENO,
					"Error: Unknown parameter (type 'help region' for more info)\n");
			continue;
		}
		if (!strcmp(token, "roll")) {
			char *roll_ch = strtok(NULL, " \n");
			if (!roll_ch) {
				dprintf(STDOUT_FILENO,
					"Error: Parameter missing\n");
				continue;
			}
			unsigned char roll = (unsigned char)atoi(roll_ch);
			if (roll < 1 || roll > 6) {
				dprintf(STDOUT_FILENO,
					"Error: Bad parameter (must be between 1 and 6)\n");
				continue;
			}
			dprintf(STDOUT_FILENO, "ack\n");
			world->moves_left = roll;
			continue;
		}
		if (!strcmp(token, "save")) {
			char *msg;
			int result = validate_game_data(&msg);
			if (result == 0) {
				dprintf(STDOUT_FILENO, "ack\n");
				save_game();
			}
			else {
				dprintf(STDOUT_FILENO, "Error: Not saving. %s\n", msg);
				free(msg);
			}
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
					dprintf(STDOUT_FILENO, "ack\n");
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
				dprintf(STDOUT_FILENO, "ack\n");
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
				dprintf(STDOUT_FILENO, "ack\n");
				tile->walkable = walkable;
			} else dprintf(STDOUT_FILENO, "Error: Unknown parameter\n");
			continue;
		}
		if (!strcmp(token, "turn")) {
			if (world->playerlist == NULL) {
				dprintf(STDOUT_FILENO, "Error: no player list\n");
				continue;
			}
			char *player_id_ch = strtok(NULL, " \n");
			if (!player_id_ch) {
				dprintf(STDOUT_FILENO, "Error: Parameter missing\n");
				continue;
			}
			uint16_t player_id = (uint16_t) atoi(player_id_ch);
			if (get_player_by_id(player_id) == NULL) {
				dprintf(STDOUT_FILENO, "Error: no such player (type 'help turn' for more info)\n");
				continue;
			}
			dprintf(STDOUT_FILENO, "ack\n");
			world->selected_player = get_player_by_id(player_id);
			continue;
		}
		if (!strcmp(token, "validate")) {
			char *msg = NULL;
			int result = validate_game_data(&msg);
			if (result == 0) dprintf(STDOUT_FILENO, "ack\n");
			else {
				dprintf(STDOUT_FILENO, "Error: %s\n", msg);
				free(msg);
			}
			continue;
		} else
			dprintf(STDOUT_FILENO,
				"Error: Unknown command (type 'help' for more info)\n");
	}
}

void think()
{
	/* if have enough money and free tile, buy a soldier */
	while (get_money(ai_player) >= COST_SOLDIER) {
		/* find free tile */
		int i, j;
		int done = 0;
		for (i = 0; i < world->grid->height; i++) {
			for (j = 0; j < world->grid->width; j++) {
				if (world->grid->tiles[i][j]->region != NULL && world->grid->tiles[i][j]->region->owner == ai_player && world->grid->tiles[i][j]->piece == NULL && !done) {
					add_piece(1, i, j, ai_player);
					set_money(ai_player, get_money(ai_player) - COST_SOLDIER);
					dprintf(STDOUT_FILENO, "piece add %i 1 %i,%i\n", ai_player->id, i, j);
					done = 1;
				}
			}
		}
		/* break if no free tile */
		if (done == 0) break;
	}

	/* if rolled 6, always choose money. Stupid, but... */
	if (world->moves_left == 6) {
		dprintf(STDOUT_FILENO, "take\n");
		dprintf(STDOUT_FILENO, "done\n");
		world->moves_left = 0;
		ai_player->money++;
		stage = STANDBY;
		return;
	}
	while (world->moves_left > 0) {
/**
		float score = evaluate();
		printf("Evaluation: %f\n", score);
**/
		int nr_ai_pieces = count_pieces_by_owner(ai_player) + 1;
		piece_t *current_piece = world->piecelist;
		int random_nr;
		if (nr_ai_pieces > 1) random_nr = rand() % nr_ai_pieces + 1;
		else random_nr = 1;
		while (current_piece != NULL) {
			if (current_piece->owner == ai_player) {
				random_nr--;
				if (random_nr == 0) break;
			}
			current_piece = current_piece->next;
		}
		uint16_t directions_mask = 0;
		if (is_legal_move(current_piece->height, current_piece->width, current_piece->height + 1, current_piece->width)) directions_mask |= 1;
		if (is_legal_move(current_piece->height, current_piece->width, current_piece->height, current_piece->width + 1)) directions_mask |= (1 << 1);
		if (is_legal_move(current_piece->height, current_piece->width, current_piece->height - 1, current_piece->width)) directions_mask |= (1 << 2);
		if (is_legal_move(current_piece->height, current_piece->width, current_piece->height, current_piece->width - 1)) directions_mask |= (1 << 3);
		uint16_t nr_directions = __builtin_popcount (directions_mask);
		/* if piece is blocked, skip and choose another one (WHAT IF ALL PIECES are blocked???) */
		if (nr_directions == 0) continue;
		random_nr = rand() % nr_directions + 1;
		int bit = 0;
		while (random_nr > 0) {
			if ((directions_mask >> bit) & 1) {
				random_nr--;
			}
			bit++;
		}
		switch (bit) {
			case 1:
				dprintf(STDOUT_FILENO, "piece move %i %i %i %i\n", current_piece->height, current_piece->width, current_piece->height + 1, current_piece->width);
				world->grid->tiles[current_piece->height][current_piece->width]->piece = NULL;
				current_piece->height++;
				world->grid->tiles[current_piece->height][current_piece->width]->piece = current_piece;
				break;
			case 2:
				dprintf(STDOUT_FILENO, "piece move %i %i %i %i\n", current_piece->height, current_piece->width, current_piece->height, current_piece->width + 1);
				world->grid->tiles[current_piece->height][current_piece->width]->piece = NULL;
				current_piece->width++;
				world->grid->tiles[current_piece->height][current_piece->width]->piece = current_piece;
				break;
			case 3:
				dprintf(STDOUT_FILENO, "piece move %i %i %i %i\n", current_piece->height, current_piece->width, current_piece->height - 1, current_piece->width);
				world->grid->tiles[current_piece->height][current_piece->width]->piece = NULL;
				current_piece->height--;
				world->grid->tiles[current_piece->height][current_piece->width]->piece = current_piece;
				break;
			case 4:
				dprintf(STDOUT_FILENO, "piece move %i %i %i %i\n", current_piece->height, current_piece->width, current_piece->height, current_piece->width - 1);
				world->grid->tiles[current_piece->height][current_piece->width]->piece = NULL;
				current_piece->width--;
				world->grid->tiles[current_piece->height][current_piece->width]->piece = current_piece;
				break;
		}
		world->moves_left--;
	}
	dprintf(STDOUT_FILENO, "done\n");
	world->moves_left = 0;
	if (ai_player->next != NULL) world->selected_player = ai_player->next;
	else world->selected_player = world->playerlist;
	stage = STANDBY;
	return;
}

int main(int argc, char **argv)
{
	reset();
	dprintf(STDOUT_FILENO, "Feud AI v0.0.1\n");
	stdin_buffer = malloc(sizeof(buffer_t));
	stdin_buffer->size = 0;
	stage = STANDBY;
	while (1) {
		switch (stage) {
		case STANDBY:
			standby();
			break;
		case THINK:
			think();
			break;
		default:
			return 0;
			break;
		}
	}
	return 0;
}
