#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h> /* for isdigit */
#include <unistd.h>
//#include "ai.h" /* needed? */
#include "world.h"

#define MAXLINE 1024

typedef struct {
	unsigned int size;
	char string[MAXLINE];
} buffer_t;

buffer_t *stdin_buffer;

int main(int argc, char **argv) {
	dprintf(STDOUT_FILENO, "Feud AI v0.0.1\n");
	FILE *input_stream;
	input_stream = stdin;
	stdin_buffer = malloc(sizeof(buffer_t));
	stdin_buffer->size = 0;

	/* vars for loop */
	char command[MAXLINE] = {0};
	int result = 0;
	int nr_arguments = 0;

	player_t *player = NULL;
	while (1) {
		/* get user input */
		if (!fgets(command, 256, stdin))
			return 0;
		if (command[0] == '\n')
			continue;
		strtok(command, "\n");
		sscanf(command, "%s", command);
		if (!strcmp(command, "quit")) {
			dprintf(STDOUT_FILENO, "Quitting...\n");
			return 0;
		}
		if (!strcmp(command, "ping")) {
			dprintf(STDOUT_FILENO, "pong\n");
		}
		else {
			dprintf(STDOUT_FILENO, "Error: unknown command\n");
		}
	}
	return 0;
}
