#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("sencd-server: Server for managing clients and rerouting internet traffic.\n- RELEASE: v.1.0.0\n- Add '--help' to this command see a list of all sub-commands\n");
	} else if (strcmp(argv[1], "--help") == 0) {
		printf(
				"usage: sencd-server <options> <argv>?\n\n"
				"options:\n"
				"- CORE:\n"
				"-    start <argv='PORT NUMBER': default='8080'> [Start the Server on an optional port]\n"
				"-    shutdown [Stop the Server, disconnecting all clients if needed]\n"
				"- MANAGEMENT:\n"
				"-    list [List all clients connected to the current server]\n"
				"-    status [Get current diagnostics of the server]\n"
				"-    kick <argv='client_id'> [Kick a client off the server]\n"
				"-    ban <argv='client_id'> [Ban a client off the server, kicking them in the process]\n"
				"-    unban <argv='client_id'> [Unban a client from the server]\n"
				"-    limit <argv='n': default 'inf'> [Limit the server to process a maximum amount of 'n' packages per second]\n"
				"-    setpwd [Set the administrator password, mainly used for remote control access on the client]\n"
			  );
	} else {
		printf("sencd-server: Unknown command\n");
	}
	return 0;
}
