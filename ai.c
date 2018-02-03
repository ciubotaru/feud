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

void print_help(const char *topic)
{
	if (topic == NULL) {
		dprintf(STDOUT_FILENO, "board height width - set game board size\n");
		dprintf(STDOUT_FILENO, "load - load game from file\n");
		dprintf(STDOUT_FILENO, "new - clear everything\n");
		dprintf(STDOUT_FILENO, "ping - check if AI is alive\n");
		dprintf(STDOUT_FILENO, "quit - terminate AI\n");
		dprintf(STDOUT_FILENO, "save - write current game to file\n");
		dprintf(STDOUT_FILENO, "for details, type 'help [command]'\n");
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
	dprintf(STDOUT_FILENO, "ack\n");
	return 0;
}

void standby()
{
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
		char *command = strdup(stdin_buffer->string);
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
			if (success) set_grid(coords[0], coords[1]);
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
		if (!strcmp(token, "quit")) {
			dprintf(STDOUT_FILENO, "Quitting...\n");
			stage = -1;
			return;
		}
		if (!strcmp(token, "save")) {
			save_game();
			dprintf(STDOUT_FILENO, "ack\n");
			continue;
		} else
			dprintf(STDOUT_FILENO, "Error: Unknown command.\n");
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
