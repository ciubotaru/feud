#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>		/* for isdigit */
#include <unistd.h>
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
		dprintf(STDOUT_FILENO, " quit - terminate AI\n");
		dprintf(STDOUT_FILENO, " save - write current game to file\n");
		dprintf(STDOUT_FILENO, " validate - check game data playability\n");
//		dprintf(STDOUT_FILENO, "For details, type 'help [command]'.\n");
		return;
	}
	if (strcmp(topic, "piece") == 0) {
		print_help_piece();
		return;
	}
	dprintf(STDOUT_FILENO, "%s: no help for this topic\n", topic);
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
			dprintf(STDOUT_FILENO, "ack\n");
			continue;
		}
		if (!strcmp(command, "new")) {
			reset();
			dprintf(STDOUT_FILENO, "ack\n");
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
		if (!strcmp(command, "quit")) {
			dprintf(STDOUT_FILENO, "Quitting...\n");
			stage = -1;
			return;
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
	dprintf(STDOUT_FILENO, "Feud Server v0.0.1\n");
	stdin_buffer = malloc(sizeof(buffer_t));
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
