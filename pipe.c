#include "world.h"

ai_connection_t *create_connectionlist()
{
	/* single instance */
	if (connection_list != NULL)
		return connection_list;
	connection_list = malloc(sizeof(ai_connection_t));
	if (connection_list == NULL)
		return NULL;
	return connection_list;
}

void add_connection_details(ai_connection_t * connection, player_t * player)
{
	connection->id = player->id;
	connection->player = player;
	connection->stdin_buffer = malloc(sizeof(buffer_t));
	connection->stdout_buffer = malloc(sizeof(buffer_t));
	connection->next = NULL;
	if (pipe(connection->pipe_to_ai) < 0 || pipe(connection->pipe_from_ai) < 0) {
		debug_msg("pipe error");
		return;
	}
}

void create_pipe(ai_connection_t *connection, pid_t pid) {
	if (pid < 0) {
		debug_msg("fork error");
		return;
	} else if (pid > 0) {	// parent
		connection->pid = pid;
		close(connection->pipe_to_ai[0]); /* close read end */
		close(connection->pipe_from_ai[1]); /* close write end */
		fcntl(connection->pipe_to_ai[1], F_SETFL, O_NONBLOCK); /* non-blocking write to ai */
		fcntl(connection->pipe_from_ai[0], F_SETFL, O_NONBLOCK);
	} else {		// child
		close(connection->pipe_to_ai[1]); /* close write end */
		close(connection->pipe_from_ai[0]); /* close read end */
		fcntl(connection->pipe_from_ai[1], F_SETFL, O_NONBLOCK); /* non-blocking write from ai */
		fflush(stdout);
		/* set write end of to_ai pipe fd to STDIN_FILENO */
		if (connection->pipe_to_ai[0] != STDIN_FILENO) {
			if (dup2(connection->pipe_to_ai[0], STDIN_FILENO) != STDIN_FILENO) {
				debug_msg("dup2 error to stdin");
				return;
			}
			close(connection->pipe_to_ai[0]);
		}

		/* set read end of from_ai pipe fd to STDOUT_FILENO */
		if (connection->pipe_from_ai[1] != STDOUT_FILENO) {
			if (dup2(connection->pipe_from_ai[1], STDOUT_FILENO) !=
			    STDOUT_FILENO) {
				debug_msg("dup2 error to stdout");
				return;
			}
			close(connection->pipe_from_ai[1]);
		}
		if (execl("./client", "client", (char *)0) < 0) {
			debug_msg("execl error");
			return;
		}
	}
}

ai_connection_t *add_connection(player_t * player)
{
	if (connection_list == NULL) {
		connection_list = create_connectionlist();
		add_connection_details(connection_list, player);
		pid_t pid = fork();
		create_pipe(connection_list, pid);
		int fd = find_free_fd();
		connection_list->fd = fd;
		fds[fd].fd = connection_list->pipe_from_ai[0];
		fds[fd].events = POLLIN;
		return connection_list;
	}

	ai_connection_t *current = get_connection_by_player(player);

	if (current != NULL) return current;

	current = connection_list;

	/*fast-forward to the end of list */
	while (current->next != NULL) {
		current = current->next;
	}

	/* now we can add a new variable */
	current->next = malloc(sizeof(ai_connection_t));
	if (!current->next)
		return NULL;
	add_connection_details(current->next, player);
	pid_t pid = fork();
	create_pipe(current->next, pid);
	int fd = find_free_fd();
	current->next->fd = fd;
	fds[fd].fd = current->next->pipe_from_ai[0];
	fds[fd].events = POLLIN;
	return current->next;
}

void remove_connection(ai_connection_t * connection)
{
	if (connection == NULL)
		return;

	free(connection->stdin_buffer);
	free(connection->stdout_buffer);

	close(connection->pipe_to_ai[1]);
	close(connection->pipe_from_ai[0]);

	fds[connection->fd].fd = -1;

	for (ai_connection_t **current = &connection_list; *current; current = &(*current)->next) {
		if (*current == connection) {
			ai_connection_t *next = (*current)->next;
			free(*current);
			*current = next;
			break;
		}
	}
}

int send(ai_connection_t *connection, char *command, int size) {
	if (command[strlen(command) - 1] != '\n') {
		command[strlen(command)] = '\n';
		size++;
	}
	int bytes_sent = write(connection->pipe_to_ai[1],
					  command, size);
	return bytes_sent;
}

ai_connection_t *get_connection_by_player(player_t *player) {
	if (connection_list == NULL) return NULL;
	ai_connection_t *current = connection_list;
	while (current != NULL ) {
		if (current->player == player) return current;
		current = current->next;
	}
	return NULL;
}

int check_connection(ai_connection_t *connection) {
	char line;
	if (connection == NULL) return 0;
	send(connection, "ping", 4);
	if (read(connection->pipe_from_ai[0], &line, 1) <= 0 || line == EOF) {
		remove_connection(connection);
		wait(0);
		return 0;
	}
	else {
		printf("%c\n", line);
		return 1;
	}
}

int find_free_fd() {
	int i = 1;
	while (i < MAXFDS) {
		if (fds[i].fd <= 0) return i;
		i++;
	}
	return -1;
}

void print_connections()
{
	if (connection_list == NULL) {
		printf("Connection list empty.\n");
		return;
	}
	printf("Active connections:\n");
	ai_connection_t *current = connection_list;
	while (current != NULL) {
		printf("%i: %s\n", current->fd, current->player->name);
		current = current->next;
	}
}
