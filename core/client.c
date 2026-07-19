#include <stdio.h>
#include <string.h>

#include "../src/client/core.h"

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("sencd-client: Client for connecting to servers to reroute internet traffic.\n- RELEASE: v.1.0.0\n- Add '--help' to this command see a list of all sub-commands\n");
	} else if (strcmp(argv[1], "--help") == 0) {
		printf(
				"usage: sencd-client <options> <argv>?\n\n"
				"options:\n"
				"- CORE:\n"
				"-    connect <argv='IP OF SERVER'> <argv='PORT NUMBER': default='8080'> [Connect to a target server on its port number]\n"
				"-    disconnect [Disconnect from the current server, if any]\n"
				"-    setinit <argv='none || startup': default 'none'> [Change sencd-client's init method]\n-        none: sencd doesn't attempt to connect to latest disconnected server when the computer reboots\n-        startup: sencd attempts to connect to the latest disconnected server on computer reboot\n"
				"- DIAGNOSTICS:\n"
				"-    status [Get the current status of the server]\n"
				"-    ping [Ping the server to check availability]\n"
				"-    info [Display information about the hardware and specifics of the server]\n"
				"-    stats [Display information based on current server traffic]\n"
				"- SERVER CONTROL:\n"
				"-    sout <argv='n'> [Show the last 'n' messages based on the server's output]\n"
				"-    rcontrol <argv='SERVER PASSWORD SET USING setpwd'> [Remotely control the current server, if any]\n"
			  );
	} else {
		// TODO: actually create a parser for commands. src/commands
		// for server: create env so sub-commands work as normal commands and also change command prefix to smt like "sencd-server >"
		CONNECTION_T server;
		connection_init(&server);
		connection_connect(&server, "127.0.0.1", DEFAULT_PORT);
	}
	return 0;
}
