cmake_minimum_required(VERSION 3.16)
project(sencd LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(sencd-client
	main/client.c
	src/debug/logs.c
	src/server/serverinit/idle.c
	src/client/client_core.c
	src/shared/packet.c
	src/linker/session.c
	src/linker/session_manager.c
)

target_include_directories(sencd-client PRIVATE
	src/debug
	src/server/serverinit
	src/client
	src/shared
	src/linker
)

add_executable(sencd-server
	main/server.c
	src/debug/logs.c
	src/server/server_core.c
	src/server/serverinit/idle.c
	src/commands/server_env.c
	src/shared/packet.c
	src/linker/session.c
	src/linker/session_manager.c
)

target_include_directories(sencd-server PRIVATE
	src/debug
	src/server
	src/server/serverinit
	src/commands
	src/shared
	src/linker
)
