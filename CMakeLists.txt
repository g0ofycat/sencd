cmake_minimum_required(VERSION 3.16)
project(sencd LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(sencd-client
	main/client.c
	src/debug/logs.c
	src/server/serverinit/idle.c
	src/commands/client_env.c
	src/commands/utility/clear.c
	src/client/client_core.c
	src/shared/packet.c
	src/sessions/session.c
	src/sessions/session_manager.c
	src/sessions/utility/authentication.c
)

target_include_directories(sencd-client PRIVATE
	src/debug
	src/server/serverinit
	src/commands
	src/commands/utility
	src/client
	src/shared
	src/shared/security
	src/sessions
	src/sessions/utility
)

add_executable(sencd-server
	main/server.c
	src/debug/logs.c
	src/server/server_core.c
	src/server/serverinit/idle.c
	src/commands/server_env.c
	src/commands/utility/clear.c
	src/shared/packet.c
	src/shared/security/encryption.c
	src/sessions/session.c
	src/sessions/session_manager.c
	src/sessions/utility/authentication.c
)

target_include_directories(sencd-server PRIVATE
	src/debug
	src/server
	src/server/serverinit
	src/commands
	src/commands/utility
	src/shared
	src/shared/security
	src/sessions
	src/sessions/utility
)
