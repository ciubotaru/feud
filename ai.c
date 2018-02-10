#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>		/* for isdigit */
#include <unistd.h>
#include "world.h"

#define MAXLINE 1024

int stage = -1;

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
	dprintf(STDOUT_FILENO, " piece add playerID type height width - place a new piece\ntype can be %i for %s or %i for %s\n", NOBLE, unit_type_list[NOBLE], SOLDIER, unit_type_list[SOLDIER]);
	dprintf(STDOUT_FILENO, " piece delete height width - remove a piece at given coordinates\n");
	return;
}

void print_help_player()
{
	dprintf(STDOUT_FILENO, "Parameters for 'player' command:\n");
	dprintf(STDOUT_FILENO, " player add playerID - create a new player\n");
	dprintf(STDOUT_FILENO, " player delete playerID - delete a player\n");
	dprintf(STDOUT_FILENO, " player money playerID amount - set player money (0-%i)\n", MONEY_MAX);
	dprintf(STDOUT_FILENO, " player name playerID playerName - set player name\n");
	dprintf(STDOUT_FILENO, " player rank playerID playerRank - set player rank([k]ing, [d]uke, [c]ount or\n[b]aron\n");
	return;
}

void print_help_region()
{
	dprintf(STDOUT_FILENO, "Parameters for 'region' command:\n");
	dprintf(STDOUT_FILENO, " region add regionID - create a new region\n");
	dprintf(STDOUT_FILENO, " region delete regionID - delete a region\n");
	dprintf(STDOUT_FILENO, " region name regionID regionName - set region name\n");
	dprintf(STDOUT_FILENO, " region owner regionID playerID - set region owner (to clear ownership, write\nplayerID 0)\n");
	return;
}

void print_help(const char *topic)
{
	if (topic == NULL) {
		dprintf(STDOUT_FILENO, "Available commands:\n");
		dprintf(STDOUT_FILENO, " board height width - set game board size\n");
		dprintf(STDOUT_FILENO, " load - load game from file\n");
		dprintf(STDOUT_FILENO, " new - clear everything\n");
		dprintf(STDOUT_FILENO, " ping - check if AI is alive\n");
		dprintf(STDOUT_FILENO, " piece [parameters] - set up pieces (type 'help piece' for more info)\n");
		dprintf(STDOUT_FILENO, " player [parameters] - set up a player (type 'help player' for more info)\n");
		dprintf(STDOUT_FILENO, " quit - terminate AI\n");
		dprintf(STDOUT_FILENO, " region [parameters] - set up a region (type 'help region' for more info)\n");
		dprintf(STDOUT_FILENO, " save - write current game to file\n");
		dprintf(STDOUT_FILENO, "For details, type 'help [command]'.\n");
		return;
	}
	if (strcmp(topic, "board") == 0) {
		dprintf(STDOUT_FILENO,
			"board height width - set game board size\n");
		return;
	}
	if (strcmp(topic, "new") == 0) {
		dprintf(STDOUT_FILENO,
			"new - clear everything and reset game environment\n");
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
	dprintf(STDOUT_FILENO, "%s: no help for this topic\n", topic);
}

int set_grid(const uint16_t height, const uint16_t width) {
	if (world == NULL) create_world();
	if (world->grid != NULL) {
		dprintf(STDOUT_FILENO, "Error: bad command (board already set, use 'new')\n");
		return 1;
	}
	grid_t *grid = create_grid(height, width);
	if (grid == NULL) {
		dprintf(STDOUT_FILENO, "Error: internal error (failed to create grid)\n");
		return 1;
	}
	return 0;
}
/**
int set_grid(const char *command) {
	
}
**/
void standby()
{
	char command[MAXLINE];
	while (1) {
		read_stdin();
		if (stdin_buffer->size == 0
		    || stdin_buffer->string[stdin_buffer->size - 1] != '\n') {
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
					dprintf(STDOUT_FILENO, "Error: bad argument\n");
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
				dprintf(STDOUT_FILENO, "Error: Parameter missing (type 'help piece' for more info)\n");
				continue;
			}
			/* can be 'add', 'delete' or 'move' */
			if (!strcmp(token, "add")) {
				char *player_id_ch = strtok(NULL, " \n");
				if (!player_id_ch) {
					dprintf(STDOUT_FILENO, "Error: PlayerID missing (type 'help piece' for more info)\n");
					continue;
				}
				uint16_t player_id = (uint16_t) atoi(player_id_ch);
				if (player_id < 1) {
					dprintf(STDOUT_FILENO, "Error: bad PlayerID\n");
					continue;
				}
				player_t *player = get_player_by_id(player_id);
				if (player == NULL) {
					dprintf(STDOUT_FILENO, "Error: invalid PlayerID\n");
					continue;
				}
				char *piece_type_ch = strtok(NULL, " \n");
				if (!piece_type_ch) {
					dprintf(STDOUT_FILENO, "Error: PieceType missing (type 'help piece' for more info)\n");
					continue;
				}
				unsigned char piece_type = (unsigned char) atoi(piece_type_ch);
				if (piece_type > 1) {
					dprintf(STDOUT_FILENO, "Error: invalid PieceType (type 'help piece' for more info)\n");
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
					dprintf(STDOUT_FILENO, "Error: invalid piece coordinates (type 'help piece' for more info)\n");
					continue;
				}
				piece_t *piece = add_piece(piece_type, coords[0], coords[1], player);
				if (piece) dprintf(STDOUT_FILENO, "ack\n");
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
				if (!success || coords[0] >= world->grid->height || coords[1] >= world->grid->width) {
					dprintf(STDOUT_FILENO, "Error: invalid piece coordinates (type 'help piece' for more info)\n");
					continue;
				}
				piece_t *piece = world->grid->tiles[coords[0]][coords[1]]->piece;
				if (!piece) dprintf(STDOUT_FILENO, "Error: piece not found\n");
				else {
					dprintf(STDOUT_FILENO, "ack\n");
					remove_piece(piece);
				}
				continue;
			}
			else dprintf(STDOUT_FILENO, "Error: Unknown parameter (type 'help piece' for more info)\n");
			continue;

		}
		if (!strcmp(token, "player")) {
			token = strtok(NULL, " \n");
			if (!token) {
				dprintf(STDOUT_FILENO, "Error: Parameter missing (type 'help player' for more info)\n");
				continue;
			}
			/* can be 'add', 'delete', 'money', 'name' or 'rank' */
			if (!strcmp(token, "add")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO, "Error: PlayerID missing (type 'help player' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				if (id < 1) {
					dprintf(STDOUT_FILENO, "Error: bad PlayerID\n");
					continue;
				}
				if (get_player_by_id(id) != NULL) {
					dprintf(STDOUT_FILENO, "Error: PlayerID not unique\n");
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
					dprintf(STDOUT_FILENO, "Error: PlayerID missing (type 'help player' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				player_t *player = get_player_by_id(id);
				if (player == NULL) {
					dprintf(STDOUT_FILENO, "Error: invalid PlayerID\n");
					continue;
				}
				remove_player(player);
				dprintf(STDOUT_FILENO, "ack\n");
				continue;
			}
			if (!strcmp(token, "money")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO, "Error: PlayerID missing (type 'help player' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				player_t *player = get_player_by_id(id);
				if (player == NULL) {
					dprintf(STDOUT_FILENO, "Error: no such player (type 'help player' for more info)\n");
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
					dprintf(STDOUT_FILENO, "Error: PlayerID missing (type 'help player' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				player_t *player = get_player_by_id(id);
				if (player == NULL) {
					dprintf(STDOUT_FILENO, "Error: no such player (type 'help player' for more info)\n");
					continue;
				}
				char *name = strtok(NULL, " \n");
				if (!name) {
					dprintf(STDOUT_FILENO, "Error: PlayerName missing (type 'help player' for more info)\n");
					continue;
				}
				if (!strcmp(player->name, name)) {
					dprintf(STDOUT_FILENO, "ack\n");
					continue;
				}
				if (get_player_by_name(name) != NULL) {
					dprintf(STDOUT_FILENO, "Error: PlayerName not unique\n");
					continue;
				}
				strcpy(player->name, name);
				dprintf(STDOUT_FILENO, "ack\n");
				continue;
			}
			if (!strcmp(token, "rank")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO, "Error: PlayerID missing (type 'help player' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				player_t *player = get_player_by_id(id);
				if (player == NULL) {
					dprintf(STDOUT_FILENO, "Error: no such player (type 'help player' for more info)\n");
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
						dprintf(STDOUT_FILENO, "Error: invalid rank (type 'help player' for more info)\n");
						success = 0;
						break;
				}
				if (!success) continue;
				set_player_rank(player, rank);
				dprintf(STDOUT_FILENO, "ack\n");
				continue;
			}
			else dprintf(STDOUT_FILENO, "Error: Unknown parameter (type 'help player' for more info)\n");
			continue;
		}
		if (!strcmp(token, "quit")) {
			dprintf(STDOUT_FILENO, "Quitting...\n");
			stage = -1;
			return;
		}
		if (!strcmp(token, "region")) {
			token = strtok(NULL, " \n");
			if (!token) {
				dprintf(STDOUT_FILENO, "Error: Parameter missing (type 'help region' for more info)\n");
				continue;
			}
			/* can be 'add', 'delete', 'name' or 'owner' */
			if (!strcmp(token, "add")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO, "Error: RegionID missing (type 'help region' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				if (get_region_by_id(id) != NULL) {
					dprintf(STDOUT_FILENO, "Error: RegionID not unique\n");
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
					dprintf(STDOUT_FILENO, "Error: RegionID missing (type 'help region' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				region_t *region = get_region_by_id(id);
				if (region == NULL) {
					dprintf(STDOUT_FILENO, "Error: invalid RegionID\n");
					continue;
				}
				remove_region(region);
				dprintf(STDOUT_FILENO, "ack\n");
				continue;
			}
			if (!strcmp(token, "name")) {
				char *id_ch = strtok(NULL, " \n");
				if (!id_ch) {
					dprintf(STDOUT_FILENO, "Error: RegionID missing (type 'help region' for more info)\n");
					continue;
				}
				uint16_t id = (uint16_t) atoi(id_ch);
				region_t *region = get_region_by_id(id);
				if (region == NULL) {
					dprintf(STDOUT_FILENO, "Error: no such region (type 'help region' for more info)\n");
					continue;
				}
				char *name = strtok(NULL, "\n");
				if (!name) {
					dprintf(STDOUT_FILENO, "Error: RegionName missing (type 'help player' for more info)\n");
					continue;
				}
				if (!strcmp(region->name, name)) {
					dprintf(STDOUT_FILENO, "ack\n");
					continue;
				}
				if (get_region_by_name(name) != NULL) {
					dprintf(STDOUT_FILENO, "Error: RegionName not unique\n");
					continue;
				}
				strcpy(region->name, name);
				dprintf(STDOUT_FILENO, "ack\n");
				continue;
			}
			if (!strcmp(token, "owner")) {
				char *region_id_ch = strtok(NULL, " \n");
				if (!region_id_ch) {
					dprintf(STDOUT_FILENO, "Error: RegionID missing (type 'help region' for more info)\n");
					continue;
				}
				uint16_t region_id = (uint16_t) atoi(region_id_ch);
				region_t *region = get_region_by_id(region_id);
				if (region == NULL) {
					dprintf(STDOUT_FILENO, "Error: no such region (type 'help region' for more info)\n");
					continue;
				}
				char *player_id_ch = strtok(NULL, " \n");
				if (!player_id_ch) {
					dprintf(STDOUT_FILENO, "Error: PlayerID missing (type 'help region' for more info)\n");
					continue;
				}
				uint16_t player_id = (uint16_t) atoi(player_id_ch);
				player_t *player = NULL;
				player = get_player_by_id(player_id);
				if (player_id > 0 && player == NULL) {
					dprintf(STDOUT_FILENO, "Error: no such player (type 'help region' for more info)\n");
					continue;
				}
				change_region_owner(player, region);
				dprintf(STDOUT_FILENO, "ack\n");
				continue;
			}
			else dprintf(STDOUT_FILENO, "Error: Unknown parameter (type 'help region' for more info)\n");
			continue;
		}
		if (!strcmp(token, "save")) {
			save_game();
			dprintf(STDOUT_FILENO, "ack\n");
			continue;
		} else
			dprintf(STDOUT_FILENO, "Error: Unknown command (type 'help' for more info)\n");
	}
}

int main(int argc, char **argv)
{
	reset();
	dprintf(STDOUT_FILENO, "Feud AI v0.0.1\n");
	stdin_buffer = malloc(sizeof(buffer_t));
	stdin_buffer->size = 0;
	stage = 0;		/* init stage */
	while (1) {
		switch (stage) {
		case 0:
			standby();
			break;
/**
		case 1:
			think();
			break;
**/
		default:
			return 0;
			break;
		}
	}
	return 0;
}
