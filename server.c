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

void print_help(const char *topic)
{
	if (topic == NULL) {
		dprintf(STDOUT_FILENO, "Available commands:\n");
		dprintf(STDOUT_FILENO, 
			" board <height> <width> - set game board size\n");
		dprintf(STDOUT_FILENO, " go - start the game\n");
		dprintf(STDOUT_FILENO, " load - load game from file\n");
		dprintf(STDOUT_FILENO, " new - clear everything\n");
		dprintf(STDOUT_FILENO, " quit - terminate AI\n");
		dprintf(STDOUT_FILENO, " save - write current game to file\n");
		dprintf(STDOUT_FILENO, " validate - check game data playability\n");
//		dprintf(STDOUT_FILENO, "For details, type 'help [command]'.\n");
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
