cmake_minimum_required(VERSION 3.16)
project(sencd LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(sencd-client
	main/client.c
	src/debug/logs.c
	src/client/client_core.c
)

target_include_directories(sencd-client PRIVATE
	src/client
	src/shared
	src/debug
)

add_executable(sencd-server
	main/server.c
	src/debug/logs.c
	src/server/server_core.c
)

target_include_directories(sencd-server PRIVATE
	src/server
	src/shared
	src/debug
)
