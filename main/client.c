// SPDX-License-Identifier: GPL-3.0-only
/*
 *	main/client.c: Main entry point for the client
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/commands/client_env.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("sencd-client: Client for connecting to servers to reroute "
			   "internet traffic.\n- RELEASE: v.1.0.0\n- Add '--help' to this "
			   "command see a list of all sub-commands\n");
	} else if (strcmp(argv[1], "--help") == 0) {
		printf(
			"usage: sencd-client <options> <argv>?\n\n"
			"options:\n"
			"- CORE:\n"
			"-    connect <argv='IP OF SERVER': default: '127.0.0.1'> "
			"<argv='PORT NUMBER': default='8080'> [Connect to a target server "
			"on its port number]\n"
			"-    attach [Connect a client environment to a currently running "
			"daemon]"
			"-    disconnect [Disconnect from the current server, if any]\n"
			"-    setinit <argv='none || startup': default 'none'> [Change "
			"sencd-client's init method]\n-        none: sencd doesn't attempt "
			"to connect to latest disconnected server when the computer "
			"reboots\n-        startup: sencd attempts to connect to the "
			"latest disconnected server on computer reboot\n"
			"- DIAGNOSTICS [ENVIRONMENT ONLY]:\n"
			"-    status [Get the current status of the server]\n"
			"-    ping [Ping the server to check availability]\n"
			"-    info [Display information about the hardware and specifics "
			"of the server]\n"
			"-    stats [Display information based on current server traffic]\n"
			"- SERVER CONTROL [ENVIRONMENT ONLY]:\n"
			"-    sout <argv='n'> [Show the last 'n' messages based on the "
			"server's output]\n"
			"-    rcontrol <argv='SERVER PASSWORD SET USING setpwd'> [Remotely "
			"control the current server, if any]\n");
	} else if (strcmp(argv[1], "connect") == 0) {
		start_client_environment(argc, argv);
	} else {
		printf("sencd-client: Invalid command, 'connect' and 'setinit' are the "
			   "only commands you can run while not in a client environment");
	}

	return 0;
}
