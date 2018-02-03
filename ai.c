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

void standby()
{
	char command[MAXLINE] = { 0 };

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
		strtok(stdin_buffer->string, "\n");	/* remove trailing newline */
		sscanf(stdin_buffer->string, "%s", command);
		stdin_buffer->size = 0;
		if (!strcmp(command, "quit")) {
			dprintf(STDOUT_FILENO, "Quitting...\n");
			stage = -1;
			return;
		}
		if (!strcmp(command, "ping")) {
			dprintf(STDOUT_FILENO, "pong\n");
			continue;
		}
		if (!strcmp(command, "new")) {
			reset();
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
