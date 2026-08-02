#include "server_env.h"

//--============
// -- COMMANDS
//--============

static SERVER_T *active_server = NULL;
static volatile int shutdown_requested = 0;

static void cmd_shutdown(char **argv) {
	force_logs = 1;
	if (active_server == NULL || !active_server->running) {
		log_msg(WARN_MSG, SERVER_RT, "Server is not running");
		return;
	}

	log_msg(INFO_MSG, SERVER_RT, "Shutdown requested");
	force_logs = 0;
	shutdown_requested = 1;
}

//--============
// -- CONFIG
//--============

static SERVER_COMMAND commands[] = {{"idle", NULL, idle_mode}, {"clear", NULL, cmd_clear}, {"shutdown", NULL, cmd_shutdown}};

//--============
// -- PRIVATE
//--============

static const int command_count = sizeof(commands) / sizeof(commands[0]);

/// @brief exec func mapped to cmd
/// @param *input
static void execute_command(char *input) {
	for (int i = 0; i < command_count; i++) {
		if (strcmp(input, commands[i].command) == 0) {
			commands[i].function(commands[i].argv);
			return;
		}
	}

	force_logs = 1;
	log_msg(ERROR_MSG, SERVER_RT, "Unknown command\n");
	force_logs = 0;
}

/// @brief thread for listening to clients
/// @param *arg
static void *server_listener(void *arg) {
	SERVER_LISTENER_DATA *listener = arg;
	while (listener->server->running) {
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(listener->server->socket, &readfds);
		struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};
		int ready = select(listener->server->socket + 1, &readfds, NULL, NULL, &timeout);

		if (!listener->server->running)
			break;

		if (ready < 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		if (ready == 0)
			continue;

		char client_ip[INET_ADDRSTRLEN];
		int client = server_accept(listener->server, client_ip, sizeof(client_ip));
		if (client < 0)
			continue;

		if (session_manager_connect(
					listener->session_manager,
					SESSION_SERVER,
					client,
					client_ip) == NULL) {
			close(client);
		}
	}

	return NULL;
}

//--============
// -- LOGIC
//--============

void start_server_environment(int argc, char *argv[]) {
	SERVER_T server;
	active_server = &server;
	server_init(&server);
	force_logs = 1;

	uint16_t port = SERVER_DEFAULT_PORT;

	if (argc > 2)
		port = (uint16_t)atoi(argv[2]);

	if (server_start(&server, port) != 0) {
		force_logs = 0;
		return;
	}

	SESSION_MANAGER_T session_manager;
	if (session_manager_init(&session_manager) != 0) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to initialize session manager");
		force_logs = 0;
		server_shutdown(&server);
		pthread_mutex_destroy(&session_manager.lock);
		return;
	}

	if (session_manager_udp_bind(&session_manager, port) != 0) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to start UDP tunnel listener");
		force_logs = 0;
		server_shutdown(&server);
		pthread_mutex_destroy(&session_manager.lock);
		return;
	}

	force_logs = 0;

	pthread_t udp_thread;
	pthread_create(&udp_thread, NULL, session_manager_udp_listener, &session_manager);

	pthread_t tun_sender_thread;
	pthread_create(&tun_sender_thread, NULL, session_manager_tun_sender, &session_manager);

	SERVER_LISTENER_DATA listener_data = {
		.server = &server,
		.session_manager = &session_manager
	};

	pthread_t listener_thread;

	pthread_create(
			&listener_thread,
			NULL,
			server_listener,
			&listener_data
			);

	char input[256];

	while (!shutdown_requested) {
		printf(SHELL_PREFIX);
		fflush(stdout);

		if (fgets(input, sizeof(input), stdin) == NULL)
			break;

		input[strcspn(input, "\n")] = 0;
		execute_command(input);
	}

	server_shutdown(&server);
	pthread_join(listener_thread, NULL);

	close(session_manager.udp_socket);
	session_manager.udp_socket = -1;
	pthread_join(udp_thread, NULL);
	pthread_join(tun_sender_thread, NULL);

	session_manager_disconnect_all(&session_manager);
	pthread_mutex_destroy(&session_manager.lock);
}
